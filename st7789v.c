/**
 * @file st7789v.c
 * @author Renato Freitas (freitas-renato@outlook.com)
 * @brief This file contains all functions implementations for ST7789V LCD driver
 * @version 0.1
 * @date 2021-03-24
 * 
 * @note It uses the LCD_IO format from STMicroelectronics Nucleo and Discovery boards
 * @see 
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "st7789v.h"



/**
 * @brief Initialization routine. Sets interface to RGB.
 * 
 */
void st7789_Init(void) {
    LCD_IO_Init();

    //* CASET set display width
    const uint8_t caset[4] = {
        0x00, 0x00,
        (ST7789_LCD_WIDTH - 1) >> 8, (ST7789_LCD_WIDTH - 1) & 0xFF
    };

    //* RASET set display height
    const uint8_t raset[4] = {
        0x00, 0x00,
        (ST7789_LCD_HEIGHT - 1) >> 8, (ST7789_LCD_HEIGHT - 1) & 0xFF
    };

    //* Initialization sequence
    const st7789_command_t initSequence[] = {
        // Sleep
        {ST7789_CMD_SLPIN, 10, 0, NULL},                    // Sleep
        {ST7789_CMD_SWRESET, 200, 0, NULL},                 // Reset
        {ST7789_CMD_SLPOUT, 120, 0, NULL},                  // Sleep out

        {ST7789_CMD_CMD2EN, 100, 0, NULL},

        {ST7789_CMD_MADCTL, 0, 1, ( uint8_t *)"\x00"},      // Page / column address order
        {ST7789_CMD_COLMOD, 0, 1, ( uint8_t *)"\x55"},      // 16 bit RGB mode

        // //* ADD vsync, hsync
        // {ST7789_CMD_RGBCTRL, 0, 3, (uint8_t *)"\x42\x08\x3c"},

        {ST7789_CMD_INVON, 0, 0, NULL},                     // Inversion on
        {ST7789_CMD_CASET, 0, 4, ( uint8_t *)caset},        // Set width
        {ST7789_CMD_RASET, 0, 4, ( uint8_t *)raset},        // Set height

        // Porch setting
        {ST7789_CMD_PORCTRL, 0, 5, ( uint8_t *)"\x0c\x0c\x00\x33\x33"},
        // Set VGH to 12.54V and VGL to -9.6V
        {ST7789_CMD_GCTRL, 0, 1, ( uint8_t *)"\x35"},
        // Set VCOM to 1.475V
        {ST7789_CMD_VCOMS, 0, 1, ( uint8_t *)"\x1f"},
        // Enable VDV/VRH control
        {ST7789_CMD_VDVVRHEN, 0, 1, ( uint8_t *)"\x01"},

        // LCM control
        {ST7789_CMD_LCMCTRL, 0, 1, ( uint8_t *)"\x2c"},
        // VAP(GVDD) = 4.45+(vcom+vcom offset+vdv)
        {ST7789_CMD_VRHS, 0, 1, ( uint8_t *)"\x12"},
        // VDV = 0V
        {ST7789_CMD_VDVSET, 0, 1, ( uint8_t *)"\x20"},
        // AVDD=6.8V, AVCL=-4.8V, VDDS=2.3V
        {ST7789_CMD_PWCTRL1, 0, 2, ( uint8_t *)"\xa4\xa1"},
        //  60 fps
        {ST7789_CMD_FRCTR2, 0, 1, ( uint8_t *)"\x0f"},
        // Gama 2.2
        {ST7789_CMD_GAMSET, 0, 1, (uint8_t *)"\x01"},
        // Gama curve
        {ST7789_CMD_PVGAMCTRL, 0, 14, ( uint8_t *)"\xd0\x08\x11\x08\x0c\x15\x39\x33\x50\x36\x13\x14\x29\x2d"},
        {ST7789_CMD_NVGAMCTRL, 0, 14, ( uint8_t *)"\xd0\x08\x10\x08\x06\x06\x39\x44\x51\x0b\x16\x14\x2f\x31"},
        
        {ST7789_CMDLIST_END, 0, 0, NULL}                   // End of commands
    };

    st7789_RunCommands(initSequence);

    LCD_IO_Delay(10);
    st7789_Clear(ST7789_BLACK);

    const st7789_command_t initSequence2[] = {
        {ST7789_CMD_RGBCTRL, 0, 3, (uint8_t *)"\x42\x08\x3c"},  // HSYNC = 0x3C, VSYNC = 0x80
        {ST7789_CMD_RAMCTRL, 0, 2, (uint8_t*)"\x11\xc2"},       // RAMCTRL Select RGB interface
        {ST7789_CMD_DISPON, 100, 0, NULL},                      // Display on
        {ST7789_CMD_SLPOUT, 100, 0, NULL},                      // Sleep out
        
        {ST7789_CMD_RAMWR, 50, 0, NULL},                        // Begin GRAM write
        {ST7789_CMDLIST_END, 0, 0, NULL},                       // End of commands
    };

    st7789_RunCommands(initSequence2);
}

void st7789_Reset(void) {
    LCD_IO_Init();
}


/**
 * @brief Turns display ON
 * 
 */
void st7789_DisplayOn(void) {
    LCD_IO_WriteCommand(ST7789_CMD_DISPON);
}

/**
 * @brief Turns display OFF
 * 
 */
void st7789_DisplayOff(void) {
    LCD_IO_WriteCommand(ST7789_CMD_DISPOFF);
}

/**
 * @brief Run ST7789V Command using SPI
 * 
 * @param command 
 */
void st7789_RunCommand(const st7789_command_t *command) {
    //* Send command
    LCD_IO_WriteCommand(command->command);

    LCD_IO_WriteData(command->data, command->dataSize);

    if (command->waitMs > 0) {
        LCD_IO_Delay(command->waitMs);
    }
}

/**
 * @brief Run command predefined command sequence
 * 
 * @param sequence 
 */
void st7789_RunCommands(const st7789_command_t *sequence) {
    while (sequence->command != ST7789_CMDLIST_END) {
        st7789_RunCommand(sequence);
        sequence++;
    }
}

/**
 * @brief Sets Display RAM window for pixel write
 * 
 * @param xStart  Horizontal start position
 * @param yStart  Horizontal end position
 * @param xEnd    Vertical start position
 * @param yEnd    Vertical end position
 */
void st7789_SetWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd) {
    uint8_t caset[4];
    uint8_t raset[4];

    caset[0] = (uint8_t)(xStart >> 8);
    caset[1] = (uint8_t)(xStart & 0xFF);
    caset[2] = (uint8_t)(xEnd >> 8);
    caset[3] = (uint8_t)(xEnd & 0xFF);

    raset[0] = (uint8_t)(yStart >> 8);
    raset[1] = (uint8_t)(yStart & 0xFF);
    raset[2] = (uint8_t)(yEnd >> 8);
    raset[3] = (uint8_t)(yEnd & 0xFF);


    st7789_command_t sequence[] = {
        {ST7789_CMD_CASET, 0, 4, caset},
        {ST7789_CMD_RASET, 0, 4, raset},
        {ST7789_CMDLIST_END, 0, 0, NULL},
    };

    st7789_RunCommands(sequence);

    //* Start RAM WRITE command
    const st7789_command_t ram_wr = {ST7789_CMD_RAMWR, 0, 0, NULL};
    st7789_RunCommand(&ram_wr);
}

/**
 * @brief Fill Rectangular area
 * 
 * @param color     16bit color code (RGB 565)
 * @param startX 
 * @param startY 
 * @param width 
 * @param height 
 */
void st7789_FillArea(uint16_t color, uint16_t startX, uint16_t startY, uint16_t width, uint16_t height) {
    uint8_t hi = (color >> 8) & 0xFF;
    uint8_t lo = (uint8_t)color;

    //* Set window based on (x,y)
    st7789_SetWindow(startX, startY, startX + width - 1, startY + height - 1);

    //* Write color to the selected window
    //! Check this
    for (double i = 0; i < width * height; i++) {
        LCD_IO_WriteData(&hi, 1);
        LCD_IO_WriteData(&lo, 1);
    }
}

void st7789_Clear(uint16_t color) {
    st7789_FillArea(color, 0, 0, ST7789_LCD_WIDTH, ST7789_LCD_HEIGHT);
}
