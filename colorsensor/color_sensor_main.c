/*
  Color sensor
    @omzn  2020/10/20
*/

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "inc/hw_i2c.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_qei.h"
#include "inc/hw_timer.h"
#include "inc/hw_types.h"
#include "driverlib/qei.h"
#include "utils/uartstdio.c" //strong-arm method
#include "utils/uartstdio.h"

#include "periphConf.h" //generated by PinMux

#include "my_i2c.h"
#include "my_util.h"

#include "buzzer.h"
#include "colorSensor_TCS3472.h"
#include "lcd_SB1602.h"

uint16_t read_data_clear = 0, read_data_red = 0, read_data_blue = 0, read_data_green = 0;
uint16_t low_th = 0x0000, high_th = 0x2000;

void clearTextLCD(uint8_t mode){
    if(mode == 1 || mode == 3){
        setAddressLCD(0, 0);
        writeTextLCD("                ", 16);
    }
    if(mode == 2 || mode == 3){
        setAddressLCD(0, 1);
        writeTextLCD("                ", 16);
    }
}

void LCDprint(uint8_t *str){
    #define MAX_LENGTH 16
    uint8_t tmp[MAX_LENGTH];
    uint8_t i;
    uint8_t flag = 0;

    setAddressLCD(0, 0);
    for(i = 0; i < MAX_LENGTH; i++){
        if(str[i] == '\0'){
            flag = 1;
            while(i < MAX_LENGTH) tmp[i++] = ' ';
            break;
        }
        tmp[i] = str[i];
    }
    writeTextLCD(tmp, 16);

    setAddressLCD(0, 1);
    for(i = MAX_LENGTH; i < MAX_LENGTH*2; i++){
        if(str[i] == '\0' || flag){
            while(i < MAX_LENGTH*2) tmp[i++ - MAX_LENGTH] = ' ';
            break;
        }
        tmp[i - MAX_LENGTH] = str[i];
    }
    writeTextLCD(tmp, 16);
}

float average(uint16_t *data, uint8_t num){
    float sum = 0;
    uint8_t i;
    for(i = 0; i < num; i++) sum += data[i];

    return sum / num;
}

float variance(uint16_t *data, uint8_t num){
    float sum = 0;
    uint8_t i;
    for(i = 0; i < num; i++) sum += pow(data[i], 2);

    return sum / num - pow(average(data, num), 2);
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
    // Init SW1 Interrupt
    GPIOIntClear(GPIO_PORTF_BASE, INT_ALL_BUTTONS);
    GPIOIntRegister(GPIO_PORTF_BASE, SW1PinIntHandler);
    GPIOIntTypeSet(GPIO_PORTF_BASE, INT_ALL_BUTTONS, GPIO_FALLING_EDGE);

    // Init Interrupt for Color Sensor
    GPIOIntRegister(GPIO_PORTB_BASE,ColorSensorIntHandler);
    // Make pins 0 low level triggered interrupts.
    GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_INT_PIN_0, GPIO_LOW_LEVEL);
    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_INT_PIN_0,
    GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

//*****************************************************************************
// Event handers
//*****************************************************************************

void SysTickIntHandler(void) {
  static uint32_t led_color = LED_ALL;
  static uint32_t tick_count = 0;
  if (tick_count % 16 == 0) {
    led_color = ~led_color;
    GPIOPinWrite(GPIO_PORTF_BASE, LED_CYAN, led_color);
    tick_count = 0;
  }
  tick_count++;
}

void SW1PinIntHandler(void) {
    GPIOIntDisable(GPIO_PORTF_BASE, INT_ALL_BUTTONS);
    GPIOIntClear(GPIO_PORTF_BASE, INT_ALL_BUTTONS);

    uint8_t i = 0;
    uint16_t data[10];

    LCDprint("Reading data ");
    for(i = 0; i < 10; i++){
        setAddressLCD(13, 0);
        writeTextLCD(itoa(i+1, 2), 2);

        data[i] = read16ColorSensor(CDATAL_REG);
        delay_ms(154);
    }

    setIntThresholdColorSensor(0, (uint16_t)(average(data, 10) + sqrt(variance(data, 10)) * 3));

    LCDprint("Done.");

    GPIOIntEnable(GPIO_PORTB_BASE, GPIO_INT_PIN_0);
}

void ColorSensorIntHandler(void){
    GPIOIntDisable(GPIO_PORTB_BASE, GPIO_INT_PIN_0);
    GPIOIntClear(GPIO_PORTB_BASE, GPIO_INT_PIN_0);
    clearIntColorSensor();

    clearTextLCD(3);

    setAddressLCD(1, 0);
    writeTextLCD("R:", 2);
    writeTextLCD(itoh(read16ColorSensor(RDATAL_REG), 4), 4);
    writeTextLCD(" ", 1);

    writeTextLCD("B:", 2);
    writeTextLCD(itoh(read16ColorSensor(BDATAL_REG), 4), 4);

    setAddressLCD(1, 1);
    writeTextLCD("G:", 2);
    writeTextLCD(itoh(read16ColorSensor(GDATAL_REG), 4), 4);
    writeTextLCD(" ", 1);

    writeTextLCD("C:", 2);
    writeTextLCD(itoh(read16ColorSensor(CDATAL_REG), 4), 4);


    GPIOIntEnable(GPIO_PORTB_BASE, INT_ALL_BUTTONS);
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
  UARTprintf("Color sensor\n");

  // Initialize buzzer
  initBuzzer();

  // Initialize two I2C Masters
  initI2C(I2C3_BASE);
  // Initialize LCD module
  initLCD();
  // Initialize color sensor module
  initColorSensor(INTEGRATIONTIME_154MS, GAIN_16X);
  clearIntColorSensor();

  GPIOIntEnable(GPIO_PORTF_BASE, INT_ALL_BUTTONS);

  LCDprint("Push SW1 to start adjustment...");

  SysTickPeriodSet(SysCtlClockGet() / SYSTICKS_PER_SEC);
  SysTickEnable();
  SysTickIntRegister(SysTickIntHandler);
  SysTickIntEnable();

  while(1);
}