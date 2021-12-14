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

/* For usleep() */
#include <unistd.h>
#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>

/* Driver configuration */
#include "ti_drivers_config.h"

/* Timer files */
#include <ti/drivers/Timer.h>


/* Timer */

volatile unsigned char TimerFlag=0;


volatile unsigned char buttonpress=0;

void timerCallback(Timer_Handle myHandle, int_fast16_t status)
{

}

void initTimer(void)
{
    Timer_Handle timer0;
    Timer_Params params;
    Timer_init();
    Timer_Params_init(&params);
    params.period = 500000;
    params.periodUnits = Timer_PERIOD_US;
    params.timerMode = Timer_CONTINUOUS_CALLBACK;
    params.timerCallback = timerCallback;
    timer0 = Timer_open(CONFIG_TIMER_0, &params);
    if (timer0 == NULL) {
        /* Failed to initialized timer */
        while (1) {}
    }
    if (Timer_start(timer0) == Timer_STATUS_ERROR) {
        /* Failed to start timer */
        while (1) {}
    }
}

/* Set State machines for SOS */
enum MC_SOS_States {MC_SMStart, MC_S, MC_O, MC_O2, MC_S2, MC_K, MC_BREAK, MC_BREAK2} MC_SOS_State, MC_OK_State;

/* State Machine for SOS */
void MC_SOS_SM()
{
    switch(MC_SOS_State) {
    /* Starting State Machine*/
        //case MC_SMStart:
            // Sets starting state machine to SOS
          //  MC_SOS_State = MC_S;
           // break;
        // First S in SOS
        case MC_S:
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON); // Red on
            usleep(500000);
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF); // Red off
            usleep(500000);
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON); // Red on
            usleep(500000);
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF); // Red off
            usleep(500000);
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON); // Red on
            usleep(500000);
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF); // Red off
            usleep(1500000);
            MC_SOS_State = MC_O;
            break;
        /* OK State Machine */
        case MC_O:
            GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_ON); // Green on
            usleep(1500000);
            GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF); // Green off
            usleep(500000);
            GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_ON); // Green on
            usleep(1500000);
            GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF); // Green off
            usleep(500000);
            GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_ON); // Green on
            usleep(1500000);
            GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF); // Green off
            usleep(1500000);
            MC_SOS_State = MC_S2;
            break;

        // Second S in SOS
        case MC_S2:
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON); // Red on
            usleep(500000);
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF); // Red off
            usleep(500000);
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON); // Red on
            usleep(500000);
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF); // Red off
            usleep(500000);
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON); // Red on
            usleep(500000);
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF); // Red off
            MC_SOS_State = MC_BREAK;
            break;

        // Break in-between words
        case MC_BREAK:
            // Turn LEDs off for 3500 MS
            GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF); // Green off
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF); // Red off
            usleep(3500000);
            MC_SOS_State = MC_S;
            break;

        /* Default State Machine set to Starting State Machine */
        //default:
            //MC_SOS_State = MC_SMStart;
            //break;
    }
    switch(MC_OK_State) {
        // O in OK
        case MC_O2:
            GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_ON); // Green on
            usleep(1500000);
            GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF); // Green off
            usleep(500000);
            GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_ON); // Green on
            usleep(1500000);
            GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF); // Green off
            usleep(500000);
            GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_ON); // Green on
            usleep(1500000);
            GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF); // Green off
            usleep(1500000);
            MC_OK_State = MC_K;
            break;
            // K in OK
        case MC_K:
            GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_ON); // Green on
            usleep(1500000);
            GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF); // Green off
            usleep(500000);
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON); // Red on
            usleep(500000);
            GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_ON); // Green on
            MC_OK_State = MC_BREAK2;
            break;
        // Break in-between words
        case MC_BREAK2:
            // Turn LEDs off for 3500 MS
            GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF); // Green off
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF); // Red off
            usleep(3500000);
            MC_OK_State = MC_O2;
            break;
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
    // Set Machine State to SOS Morse code
    MC_SOS_State = MC_S;

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
    // Set Machine State to OK Morse code
    MC_OK_State = MC_O2;

}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    /* Call driver init functions */
    GPIO_init();

    /* Configure the LED and button pins */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_LED_1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_BUTTON_0, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);


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

    // Program starts with SOS
    MC_SOS_State = MC_S;

    // While the program is running
    while(1) {
        // Run State Machine
        MC_SOS_SM();


        //return (NULL);

    }


}
