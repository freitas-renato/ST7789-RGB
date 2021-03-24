# ST7789-RGB

Library designed to initialize displays with a ST7789 driver using RGB interface. Tested on ST7789V / ST7789VW.

Checkout the [datasheet](https://www.newhavendisplay.com/appnotes/datasheets/LCDs/ST7789V.pdf) for detailed information about all registers address and pin setup.


# Pin Configuration

## Interface Selection

First of all, we should select the **4-line 8bit serial Interface I**, seting IM pins accordingly:

| IM3 | IM2 | IM1 | IM0 |
| --- | --- | --- | --- |
| 0   | 1   | 1   | 0   |


## LTDC Configuration

This library was tested using LTDC display driver from STMicroelectronics, available on some STM32 families, alongside a TouchGFX project to design screens and drive.

We have to configure the LTDC parameters based on the driver's datasheet. The LTDC clock must be around 17 MHz for this parameters work, otherwise you will need to calculate based on datasheet's page 114. 

| Parameter | Value |
| ----      | ---   |
| Horizontal Sync Width | 30 pixels |
| Horizontal Backporch  | 30 pixels |
| Active Width*         | 240 pixels|
| Horizontal Frontporch | 60 pixels |
| Vertical Sync Width   | 4 lines   |
| Vertial Backporch     | 4 lines   |
| Active Heigth*        | 320 pixels|
| Vertical Frontporch   | 8 lines   |

> \* you should fill active width and heigth with the size of your own display


| Signal Polarity | Value |
| ----                                  | ---          |
| Horizontal Sync Polarity              | Active Low   |
| Vertical Sync Polarity                | Active Low   |
| Not Data Enable Polarity Polarity     | Active Low   |
| Pixel Clock Polarity                  | Normal Input | 


Also, connect data pins D0 - D17 to the MCU:



| ST7789 Pin | D0  | D1 | D2 | D3 | D4 | D5 | D6 | D7 | D8 | D9 | D10 | D11 | D12 | D13 | D14 | D15 | D16 | D17 |
| ---  | --- | --- | --- | --- | --- | --- |--- | --- | --- |--- | --- | --- | --- |--- | --- | --- |  --- | --- |
| LTDC Pin   | N/C | B0 | B1 | B2 | B3 | B4 | G1 | G2 | G3 | G4 | G5 | N/C | R0 | R1 | R2 | R3 | R4 | R5 |

> This table shows the connection for RGB565 (16 bits) interface. If you want to use RGB666 (18 bits), just connect them all in order. 

Finally, connect VSYNC, HSYNC, ENABLE and DOTCLK to respective LTDC signals.

## SPI Configuration

For the initialization routine we will use SPI on Half-Duplex Master Mode, 8-bit data size, MSB first, CPOL Low, CPHA 1 Edge. The baudrate tested was 42.5 MBits/s, but it should work with lower values.

Pinout:

| ST7789 Pin | MCU |
| ---       | --- |
| SDA       | SPI_MOSI |
| DCX       | SPI_CLK  |
| CSX       | Any GPIO pin | 
| WRX       | Any GPIO pin |

Both CSX and WRX will be controlled via software.

Connect RDX to VDD or GND, since it will not be used (only used by 8080 interface), and let SDO open.

Finally, connect RESX to a GPIO pin because it will also be controlled via software.

# Library Configuration

Configure your display dimensions on `st7789v.h`.

```c
#define ST7789_LCD_WIDTH     240
#define ST7789_LCD_HEIGHT    320
```


You will need to implement some auxiliary functions externally, used to write commands and data to the display.

```c
extern void     LCD_IO_Init(void);                                  // SPI bus init and LCD RESX pin toggle
extern void     LCD_IO_WriteCommand(uint8_t command);               // SPI write command
extern void     LCD_IO_WriteData(uint8_t *data, uint8_t length);    // SPI write data
extern uint16_t LCD_IO_ReadData(void);                              // SPI read data
extern void     LCD_IO_Delay(uint32_t delay);                       // Basic MCU delay
```


## LCD_IO_Init


```c
/**
 * @brief Initializes display SPI bus and toggles RESX for hardware reset.
 * 
 */
void LCD_IO_Init(void) {
  //* CSX pin  toggle
  CS_IDLE();         // Set CS pin HIGH
  LCD_IO_Delay(20);  // Reset pulse time
  CS_ACTIVE();       // Set CS pin LOW
  LCD_IO_Delay(20);  // Reset pulse time

  //* RESX pin routine
  RESX_IDLE();      // Set RESX pin HIGH
  LCD_IO_Delay(50); // Reset pulse time
  RESX_ACTIVE();    // Set RESX pin LOW
  LCD_IO_Delay(50); // Reset pulse time
  RESX_IDLE();      // Set RESX pin HIGH
  LCD_IO_Delay(50); // Reset pulse time
}
```

## LCD_IO_WriteCommand

```c
/**
  * @brief  Writes to the display using D/CX pin, selecting Command option.
  */
void LCD_IO_WriteCommand(uint8_t command) {
  DC_COMMAND();   // Set WRX pin LOW to indicate Command
  CS_ACTIVE();    // Set CS pin LOW
  
  // SPI 1-byte transmission example, change this to your SPI transmit function
  HAL_SPI_Transmit(&hspi4, &command, 1, 500);
  
  CS_IDLE();      // Set CS pin HIGH
}
```

## LCD_IO_WriteData

```c
/**
  * @brief  Writes to the display using D/CX pin, selecting Data option.
  * 
  */
void LCD_IO_WriteData(uint8_t *data, uint8_t length) {
  if (length > 0) {
    DC_DATA();      // Set WRX pin HIGH to indicate Data
    CS_ACTIVE();    // Set CS pin LOW
    
    // SPI 1-byte transmission example, change this to your SPI transmit function
    HAL_SPI_Transmit(&hspi4, data, length, 500);
  }

  CS_IDLE();        // Set CS pin HIGH
}
```

## LCD_IO_Delay

```c
/**
  * @brief  Simple MCU delay in ms.
  * @param  Delay in ms.
  */
void LCD_IO_Delay(uint32_t Delay) {
  HAL_Delay(Delay);  // MCU Delay function
}
```

## LCD_IO_ReadData

> To Do



# Using 
Now, all you need to do is call `st7789_Init()` to send initialization parameters to the display and initialize your LTDC interface and let it deal with sending data to the display. Luckily, TouchGFX does this automatically on the background for you 😉
