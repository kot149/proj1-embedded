/*
 * lcd_SB1602.h
 *
 *  Created on: 2015/01/15
 */

#ifndef LCD_SB1602_H_BAK_
#define LCD_SB1602_H_BAK_

#include "inc/hw_memmap.h"
#include "my_i2c.h"
#include "my_util.h"

//*****************************************************************************
//                      [Co:RS:0:0:0:0:0:0]
// RS = 0 : Command
// RS = 1 : Data
// Co = 0 : Burst
// Co = 1 : Single
//*****************************************************************************
#define SB1602_DATA_SINGLE    0xC0
#define SB1602_DATA_BURST     0x40
#define SB1602_COMMAND_SINGLE 0x80
#define SB1602_COMMAND_BURST  0x00

#define SB1602_SLAVE_ADDRESS 0x3E

//*****************************************************************************
// initialize
//*****************************************************************************
void initLCD(void);
//*****************************************************************************
// command LCD
//*****************************************************************************
inline void commandLCD(uint8_t cmd);
//*****************************************************************************
// set address LCD
//*****************************************************************************
inline void setAddressLCD(uint8_t x, uint8_t y);
//*****************************************************************************
// write text LCD
//*****************************************************************************
inline void writeTextLCD(uint8_t *text, uint8_t length);

//*****************************************************************************
// clear text LCD
//*****************************************************************************
inline void clearTextLCD(uint8_t mode);

#endif /* LCD_SB1602_H_BAK_ */