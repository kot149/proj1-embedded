/*
  LCD
    @omzn  2020/10/20
*/

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "inc/hw_i2c.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_qei.h"
#include "inc/hw_timer.h"
#include "driverlib/rom.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/pwm.h"
#include "driverlib/timer.h"

#include "driverlib/qei.h"
#include "utils/uartstdio.h"
#include "utils/uartstdio.c" //strong-arm method

#include "periphConf.h" //generated by PinMux

#include "my_util.h"
#include "my_i2c.h"

#include "buzzer.h"
#include "lcd_SB1602.h"

//*****************************************************************************
// Global variables
//*****************************************************************************
uint8_t face1[]={'(',0xF3,0xA5,'w',0xA5,0xF4,')'};
uint8_t face2[]={'(',0xF4,0xA5,'w',0xA5,0xF3,')'};
uint8_t msg1[]= {0xBC,0xAC,0xB7,0xB0,0xDD,'!'};
uint8_t msg2[]= {0xBC,0xAE,0xCE,0xDE,'-',0xDD};

uint8_t button_pressed = 0;
uint8_t button_press_count = 0;
uint32_t time = 0;
uint8_t timerEnabled = 0;
uint32_t led_base = ~LED_ALL;

//*****************************************************************************
// General functions
//*****************************************************************************
void clearTextLCD(uint8_t mode){
	if(mode == 1 || mode == 3){
		setAddressLCD(1, 1);
		writeTextLCD("                ", 16);
	}
	if(mode == 2 || mode == 3){
		setAddressLCD(1, 2);
		writeTextLCD("                ", 16);
	}
}

void writeCountLCD(uint8_t count){
	clearTextLCD(1);
	uint8_t d = 0, c = count;
	do {
		d++;
		c /= 10;
	} while(c > 0);
	setAddressLCD(1, 1);
	writeTextLCD("count: ", 7);
	writeTextLCD(itoa(count, d), d);
}

void writeTimeLCD(int32_t count, uint8_t line){

	uint8_t str[11];

	str[0] = count / 360000;
	count -= str[0]*360000;
	str[1] = str[0] % 10 + '0';
	str[0] = str[0] / 10 + '0';
	str[2] = ':';

	str[3] = count / 6000;
	count -= str[3]*6000;
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

//*****************************************************************************
//
// This function sets up UART0 to be used for a console to display information
// as the example is running.
//
//*****************************************************************************
void initConsole(void) {
  // Enable GPIO port A which is used for UART0 pins.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  // Configure the pin muxing for UART0 functions on port A0 and A1.
  // This step is not necessary if your part does not support pin muxing.
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  // Enable UART0 so that we can configure the clock.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  // Use the internal 16MHz oscillator as the UART clock source.
  UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
  // Select the alternate (UART) function for these pins.
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  // Initialize the UART for console I/O.
  UARTStdioConfig(0, 9600, 16000000);
}

void initInterruptPins(void) {
	GPIOIntClear(GPIO_PORTF_BASE, INT_ALL_BUTTONS);

	GPIOIntRegister(GPIO_PORTF_BASE, SW1PinIntHandler);

	GPIOIntTypeSet(GPIO_PORTF_BASE, INT_ALL_BUTTONS, GPIO_BOTH_EDGES);
}

//*****************************************************************************
// Event handers
//*****************************************************************************

void SysTickIntHandler(void) {
	static uint32_t longpress_count = 0;

	if(button_pressed){
		longpress_count++;
		if(longpress_count >= 100){
			longpress_count = 0;
			time = 0;
			writeTimeLCD(0, 1);
			clearTextLCD(2);
			button_pressed = 0;
			GPIOPinWrite(GPIO_PORTF_BASE, LED_BLUE, 0);
			led_base = LED_BLUE;
			timerEnabled = 0;
			//GPIOIntTypeSet(GPIO_PORTF_BASE, INT_ALL_BUTTONS, GPIO_FALLING_EDGE);
		}
	} else longpress_count = 0;

	if(timerEnabled){
		time++;
		writeTimeLCD(time, 1);

		if(time % 50 == 0){
			led_base = ~led_base;
			GPIOPinWrite(GPIO_PORTF_BASE, LED_BLUE, led_base);
		}
	}
}

void SW1PinIntHandler(void) {

	GPIOIntDisable(GPIO_PORTF_BASE,INT_ALL_BUTTONS);
	GPIOIntClear(GPIO_PORTF_BASE, INT_ALL_BUTTONS);

	if(!button_pressed){
		button_pressed = 1;

		//GPIOIntTypeSet(GPIO_PORTF_BASE, INT_ALL_BUTTONS, GPIO_RISING_EDGE);
	} else {
		button_pressed = 0;

		if(timerEnabled){
			if(time) writeTimeLCD(time, 2);
			//timerEnabled = 0;
			//GPIOPinWrite(GPIO_PORTF_BASE, LED_BLUE, 0);
		} else {
			timerEnabled = 1;
		}

		//GPIOIntTypeSet(GPIO_PORTF_BASE, INT_ALL_BUTTONS, GPIO_FALLING_EDGE);
	}

	GPIOIntEnable(GPIO_PORTF_BASE,INT_ALL_BUTTONS);
}

int main(void) {
  // Set the clocking to run directly from the crystal.
  ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
		     SYSCTL_XTAL_16MHZ);
  // Set up ports hardware (see periphConf.c)
  PortFunctionInit();

  // Set up interrupts (you can specify GPIO interrupt initialization here)
  initInterruptPins();

  // Initialize console
  initConsole();
  UARTprintf("LCD example\n");

  // Initialize buzzer
  initBuzzer();

  // Initialize two I2C Masters
  initI2C(I2C3_BASE);
  // Initialize LCD module
  initLCD();

  SysTickPeriodSet(SysCtlClockGet() / 100);
  SysTickEnable();
  SysTickIntRegister(SysTickIntHandler);
  SysTickIntEnable();

  GPIOIntEnable(GPIO_PORTF_BASE,INT_ALL_BUTTONS);
  //writeCountLCD(0);
  //writeTimeLCD(367823);
  writeTimeLCD(0, 1);

  while(1);
}