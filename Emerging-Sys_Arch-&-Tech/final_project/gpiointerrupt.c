/*
 * Copyright (c) 2015-2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== gpiointerrupt.c ========
 */
#include <stdint.h>
#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/Power.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/Timer.h>

/* Driver configuration */
#include "ti_drivers_config.h"

#define DISPLAY(x)  UART_write(uart, &output, x);

// I2C Global Variables
static const struct {
    uint8_t address;
    uint8_t resultReg;
    char *id;
} sensors[3] = {
                {0x48, 0x0000, "11X"},
                {0x49, 0x0000, "116"},
                {0x41, 0x0001, "006"}
};
uint8_t          txBuffer[1];
uint8_t          rxBuffer[2];

// UART Global Variables
char   output[64];
int    bytesToSend;

// Driver Handles - Global variables
I2C_Handle i2c;
I2C_Transaction  i2cTransaction;
UART_Handle uart;
Timer_Handle timer0;

// global variables
// temperature variables
int setTemp;
int16_t currentTemp;
int heat;
int seconds;

// timer variables
int timerCnt = 0;
volatile unsigned char TimerFlag = 0;
// button flags
int upFlag = 0;
int downFlag = 0;

// Task scheduler
typedef struct task {
    int state;  // current state of task
    unsigned long period; // Task rate period
    unsigned long elapsedTime;  // How much time has passed since last tick
    int (*TickFct)(int);  // Call task tick
} task;

task tasks[2];

const unsigned char tasksNum = 2;
const unsigned long tasksPeriodGCD = 100;
const unsigned long periodReadTemp = 100;
const unsigned long periodCheckButtons = 200;
const unsigned long periodSetHeat = 500;

enum CB_States { SM1_SMStart, Temp_Change };
int TickFct_CheckButtons(int state);

enum SH_States { SM2_SMStart, SH_s1 };
int TickFct_SetHeat(int state);

int16_t readTemp(void)
{
    int j;
    int16_t temperature = 0;
    i2cTransaction.readCount = 2;
    if (I2C_transfer(i2c, &i2cTransaction))
    {
        /*
        * Extract degrees C from the received data;
        * see TMP sensor datasheet
        */
        temperature = (rxBuffer[0] << 8) | (rxBuffer[1]);
        temperature *= 0.0078125;
        /*
        * If the MSB is set '1', then we have a 2's complement
        * negative value which needs to be sign extended
        */
        if (rxBuffer[0] & 0x80)
        {
            temperature |= 0xF000;
        }
    }
    else
    {
        DISPLAY(snprintf(output, 64, "Error reading temperature sensor (%d)\n\r", i2cTransaction.status))
        DISPLAY(snprintf(output, 64, "Please power cycle your board by unplugging USB and plugging back in.\n\r"))
    }
    return temperature;
}

void timerCallback(Timer_Handle myHandle, int_fast16_t status){
    unsigned char i;
    for ( i = 0; i < tasksNum; ++i ) {
        if ( tasks[i].elapsedTime >= tasks[i].period ) {
            tasks[i].state = tasks[i].TickFct( tasks[i].state );
            tasks[i].elapsedTime = 0;
        }
        tasks[i].elapsedTime += tasksPeriodGCD;
    }

    TimerFlag = 1;
}

// initialize Timer
void initTimer(void){
    Timer_Params params;

    // init the driver
    Timer_init();

    // Configure the driver
    Timer_Params_init(&params);
    params.period = 1000000; // 0.1 seconds
    params.periodUnits = Timer_PERIOD_US;
    params.timerMode = Timer_CONTINUOUS_CALLBACK;
    params.timerCallback = timerCallback;

    // Open the driver
    timer0 = Timer_open(CONFIG_TIMER_0, &params);

    if(timer0 == NULL){
        // Failed to initialized timer
        while(1){}
    }

    if(Timer_start(timer0) == Timer_STATUS_ERROR){
        // Failed to start timer
        while(1){}
    }
}

// initialize UART
void initUART(void){
    UART_Params uartParams;

    // Init the driver
    UART_init();

    // Configure the driver
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.baudRate = 115200;

    // Open the driver
    uart = UART_open(CONFIG_UART_0, &uartParams);

    if(uart == NULL){
        // UART_open() failed
        while(1);
    }
}

// Make sure you call initUART() before calling this function.
void initI2C(void)
{
    int8_t i, found;
    I2C_Params i2cParams;
    DISPLAY(snprintf(output, 64, "Initializing I2C Driver - "))

    // Init the driver
    I2C_init();

    // Configure the driver
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2cParams.transferMode = I2C_MODE_BLOCKING;

    // Open the driver
    i2c = I2C_open(CONFIG_I2C_0, &i2cParams);

    if (i2c == NULL){
        DISPLAY(snprintf(output, 64, "Failed\n\r"))
        while (1);
    }
    DISPLAY(snprintf(output, 32, "Passed\n\r"))
    // Boards were shipped with different sensors.
    // Welcome to the world of embedded systems.
    // Try to determine which sensor we have.
    // Scan through the possible sensor addresses
    /* Common I2C transaction setup */
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 0;
    found = false;

    for (i=0; i<3; ++i){
        i2cTransaction.slaveAddress = sensors[i].address;
        txBuffer[0] = sensors[i].resultReg;
        DISPLAY(snprintf(output, 64, "Is this %s? ", sensors[i].id))

        if (I2C_transfer(i2c, &i2cTransaction)) {
            DISPLAY(snprintf(output, 64, "Found\n\r"))
            found = true;
            break;
        }
            DISPLAY(snprintf(output, 64, "No\n\r"))
    }

    if(found) {
        DISPLAY(snprintf(output, 64, "Detected TMP%s I2C address:%x\n\r", sensors[i].id, i2cTransaction.slaveAddress))
    } else {
        DISPLAY(snprintf(output, 64, "Temperature sensor not found, contact professor\n\r"))
    }
}

/*
 *  ======== gpioButtonFxn0 ========
 *  Callback function for the GPIO interrupt on CONFIG_GPIO_BUTTON_0.
 *
 *  Note: GPIO interrupts are cleared prior to invoking callbacks.
 */
void gpioButtonFxn0(uint_least8_t index)
{
    // Add 1 to up temp button and turn down temperature
    upFlag += 1;

}

/*
 *  ======== gpioButtonFxn1 ========
 *  Callback function for the GPIO interrupt on CONFIG_GPIO_BUTTON_1.
 *  This may not be used for all boards.
 *
 *  Note: GPIO interrupts are cleared prior to invoking callbacks.
 */
void gpioButtonFxn1(uint_least8_t index)
{
    // Add 1 to down temp button to turn down temperature
    downFlag += 1;
}

// Check buttons tick
int TickFct_CheckButtons(int state){
    // every 200 ms check the button flags
    switch(state){
        case SM2_SMStart: // Second state machine
            state = Temp_Change; // state is temp_change
            break;
        case Temp_Change:
            state = Temp_Change;
            break;
        default: // Default
            state = SM1_SMStart;
            break;
    }

    switch(state){
        case Temp_Change: // Temp Change
            if(upFlag > 0 && setTemp < 99){
              setTemp += 1; // Change temperature plus 1
            }
            if(downFlag > 0 && setTemp > 0){
              setTemp -= 1; // Change temperature minus 1
            }
            // Reset up and down flags
            upFlag = 0;
            downFlag = 0;
            break;
    }
    return state;
}

// Set heat tick
int TickFct_SetHeat(int state){
    switch(state){ // transitions
        case SM2_SMStart: // Starting second state machine
            state = SH_s1;
            break;
        case SH_s1:
            state = SH_s1;
            break;
        default: // Default start to State Machine
            state = SM2_SMStart;
            break;
    }

    switch(state){ // state activities
        case SH_s1:
            // update the led depending on current temperature versus set temperature
            if(setTemp > currentTemp){ // If the set temperature is greater than the current temperature
                heat = 1; // Set heat to 1
                GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON); // Turn red LED on
            } else { // If set temperature is not greater than current temperature or else
                heat = 0; // Set heat to 0
                GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF); // Turn red LED off
            }
            break;
    }
    return state;
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    /* Call driver init functions */
    GPIO_init();
    initUART();
    initI2C();
    initTimer();


    /* Configure the LED and button pins */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_BUTTON_0, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);
    //GPIO_setConfig(CONFIG_GPIO_TMP_EN, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_HIGH);

    /* Turn on user LED */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    /* Install Button callback */
    GPIO_setCallback(CONFIG_GPIO_BUTTON_0, gpioButtonFxn0);

    /* Enable interrupts */
    GPIO_enableInt(CONFIG_GPIO_BUTTON_0);

    /*
     *  If more than one input pin is available for your device, interrupts
     *  will be enabled on CONFIG_GPIO_BUTTON1.
     */
    if (CONFIG_GPIO_BUTTON_0 != CONFIG_GPIO_BUTTON_1) {
        /* Configure BUTTON1 pin */
        GPIO_setConfig(CONFIG_GPIO_BUTTON_1, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);

        /* Install Button callback */
        GPIO_setCallback(CONFIG_GPIO_BUTTON_1, gpioButtonFxn1);
        GPIO_enableInt(CONFIG_GPIO_BUTTON_1);
    }

    unsigned long seconds; //  Create seconds variable
    unsigned char i = 0;

    tasks[i].state = SM1_SMStart; // Start with first State Machine
    tasks[i].period = periodCheckButtons; // Check buttons
    tasks[i].elapsedTime = tasks[i].period; // Task period
    tasks[i].TickFct = &TickFct_CheckButtons; // Check Buttons tick
    ++i;
    tasks[i].state = SM2_SMStart; // Start second State Machine
    tasks[i].period = periodSetHeat; // Set task for period heat
    tasks[i].elapsedTime = tasks[i].period; // Elapsed time with the time period
    tasks[i].TickFct = &TickFct_SetHeat; // Tick setheat

    heat = 0; // Set heat to 0
    seconds = 0; // Set seconds to 0
    setTemp = 28; // Set the temperature to 28 on setTemp

    // Loop forever
    while(1){
        // read current temp
        currentTemp = readTemp();

        // timer
        while(!TimerFlag){}
        TimerFlag = 0;
        seconds++;
        // Output the current temperature, the set temperature, heat number, and time in seconds that the program has been running.
        DISPLAY(snprintf(output, 64, "<%02d,%02d,%d,%04d>\n\r", currentTemp, setTemp, heat, seconds))
    }

    return (NULL);
}
