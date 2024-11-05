/*
 * lcd_SB1602.c
 *
 *  Created on: Oct 10, 2020
 *      Author: omzn
 */

#include "lcd_SB1602.h"

//*****************************************************************************
// initialize
//*****************************************************************************
void initLCD(void) {
	//setting commands for LCD init:
	uint8_t lcd_init_data1[] = { SB1602_COMMAND_BURST, 0x38, 0x39, 0x14, 0x78,
			0x5E, 0x6C };
	//setting commands for LCD display: ?, Display ON/OFF, Clear Display
	uint8_t lcd_init_data2[] = { SB1602_COMMAND_BURST, 0x38, 0b1100, 0x01 };
	//setting commands for LCD display: ?, Return Home?
	uint8_t lcd_init_data3[] = { SB1602_COMMAND_BURST, 0x10, 0x02 };
	// LCD setting
	delay_ms(40); //40ms
	writeDataI2C(I2C3_BASE, SB1602_SLAVE_ADDRESS, lcd_init_data1, 7);
	delay_ms(210); //210ms
	writeDataI2C(I2C3_BASE, SB1602_SLAVE_ADDRESS, lcd_init_data2, 4);
	delay_ms(20); //20ms
	writeDataI2C(I2C3_BASE, SB1602_SLAVE_ADDRESS, lcd_init_data3, 3);
	delay_ms(20); //20ms
}

//*****************************************************************************
// command LCD
//*****************************************************************************
inline void commandLCD(uint8_t cmd) {
	static uint8_t command[2];
	command[0] = SB1602_COMMAND_SINGLE;
	command[1] = cmd;
	writeDataI2C(I2C3_BASE, SB1602_SLAVE_ADDRESS, command, 2);
}

//*****************************************************************************
// set address LCD
//*****************************************************************************
inline void setAddressLCD(uint8_t x, uint8_t y) {
#define COMMAND_SINGLE 0x80
	uint8_t command[] = { COMMAND_SINGLE, 0x80 };

	command[1] = command[1] | ((x - 1) + (y - 1) * 0x40);

	writeDataI2C(I2C3_BASE, SB1602_SLAVE_ADDRESS, command, 2);
}

//*****************************************************************************
// write text LCD
//*****************************************************************************
inline void writeTextLCD(uint8_t *text, uint8_t length) {
	uint8_t command[17];
	command[0] = SB1602_DATA_BURST;
	uint8_t i;
	for (i = 0; i < length; i++) {
		command[i + 1] = text[i];
	}
	writeDataI2C(I2C3_BASE, SB1602_SLAVE_ADDRESS, command, length + 1);
}

//*****************************************************************************
// clear text LCD
//*****************************************************************************
inline void clearTextLCD(uint8_t mode) {
	if (mode == 1 || mode == 3) {
		setAddressLCD(1, 1);
		writeTextLCD("                ", 16);
	}
	if (mode == 2 || mode == 3) {
		setAddressLCD(1, 2);
		writeTextLCD("                ", 16);
	}
}

//*****************************************************************************
// icon LCD
//*****************************************************************************
inline void iconLCD(void) {
	uint8_t command[3];

	//Set IS=1 using 'Function Set'
	command[0] = SB1602_COMMAND_SINGLE;
	command[1] = 0b00111001;
	writeDataI2C(I2C3_BASE, SB1602_SLAVE_ADDRESS, command, 2);
	delay_ms(1);

	//Set Icon Address
	command[1] = 0b01000000;
	writeDataI2C(I2C3_BASE, SB1602_SLAVE_ADDRESS, command, 2);
	delay_ms(1);

	//Write Data To RAM
	command[0] = SB1602_DATA_SINGLE;
	command[1] = 0b000010000;
	writeDataI2C(I2C3_BASE, SB1602_SLAVE_ADDRESS, command, 2);
	delay_ms(1);

	//Restore IS=0
	command[0] = SB1602_COMMAND_SINGLE;
	command[1] = 0b00111000;
	writeDataI2C(I2C3_BASE, SB1602_SLAVE_ADDRESS, command, 2);
}

//*****************************************************************************
// Miscellaneous LCD Functions
//*****************************************************************************
inline void writeCountLCD(uint8_t count) {
	clearTextLCD(1);
	uint8_t d = 0, c = count;
	do {
		d++;
		c /= 10;
	} while (c > 0);
	setAddressLCD(1, 1);
	writeTextLCD("count: ", 7);
	writeTextLCD(itoa(count, d), d);
}

inline void writeTimeLCD(int32_t count, uint8_t line) {

	uint8_t str[11];

	str[0] = count / 360000;
	count -= str[0] * 360000;
	str[1] = str[0] % 10 + '0';
	str[0] = str[0] / 10 + '0';
	str[2] = ':';

	str[3] = count / 6000;
	count -= str[3] * 6000;
	str[4] = str[3] % 10 + '0';
	str[3] = str[3] / 10 + '0';
	str[5] = ':';

	str[6] = count / 100;
	str[7] = str[6] % 10 + '0';
	str[6] = str[6] / 10 + '0';
	str[8] = '.';

	str[9] = count % 100;
	str[10] = str[9] % 10 + '0';
	str[9] = str[9] / 10 + '0';

	clearTextLCD(line);

	setAddressLCD(4, line);

	writeTextLCD(str, 11);

}

