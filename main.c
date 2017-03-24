/*
 * Copyright (c) 2016, Texas Instruments Incorporated
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
 *  ======== adcsinglechannel.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* Driver Header files */
#include <ti/drivers/ADC.h>
#include <ti/drivers/GPIO.h>
/* Example/Board Header files */
#include "Board.h"
#include <stdio.h>


#include <grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"

Graphics_Context g_sContext;

/* ADC sample count */
#define ADC_SAMPLE_COUNT  (10)

/*Task Specific defines */
#define TASKSTACKSIZE     (1024)

Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

Task_Struct task1Struct;
Char task1Stack[TASKSTACKSIZE];

Task_Struct task2Struct;
Char task2Stack[TASKSTACKSIZE];

Task_Struct task3Struct;
Char task3Stack[TASKSTACKSIZE];
/* ADC conversion result variables */
uint16_t adcValue0;
uint16_t adcValue1;
uint16_t adcValue2;
float adcFloat0;
float adcFloat1;
float adcFloat2;


void drawTitle(void);


void lcdFxn0(UArg arg0, UArg arg1)
{
    Crystalfontz128x128_Init();
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

    /* Initializes graphics context */
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
    drawTitle();
    while(1)
    {
    	GPIO_toggle(Board_LED1);
    	Task_sleep(100);
	    char string[8];
	    sprintf(string, "X: %.3f", adcFloat0);
	    Graphics_drawStringCentered(&g_sContext,
	                                    (int8_t *)string,
	                                    8,
	                                    64,
	                                    50,
	                                    OPAQUE_TEXT);
	    sprintf(string, "Y: %.3f", adcFloat1);
	    Graphics_drawStringCentered(&g_sContext,
	                                    (int8_t *)string,
	                                    8,
	                                    64,
	                                    60,
	                                    OPAQUE_TEXT);
	    sprintf(string, "Z: %.3f", adcFloat2);
	    Graphics_drawStringCentered(&g_sContext,
	                                    (int8_t *)string,
	                                    8,
	                                    64,
	                                    70,
	                                    OPAQUE_TEXT);
    }
}

/*
 *  ======== taskFxn0 ========
 *  Open an ADC instance and get a sampling result from a one-shot conversion.
 */
Void taskFxn0(UArg arg0, UArg arg1)
{

	ADC_Handle   adc;
	ADC_Params   params;
	int_fast16_t res;

	ADC_Params_init(&params);
	adc = ADC_open(arg0, &params);
	if (adc == NULL) {
		System_abort("Error initializing ADC channel\n");
	}
	while(1)
	{
		Task_sleep(200);


	    switch(arg0)
	    {
	    case Board_ADC14:
			/* Blocking mode conversion */
			res = ADC_convert(adc, &adcValue0);

			if (res == ADC_STATUS_SUCCESS) {
				adcFloat0 = 0.000231*(float)adcValue0 - 2.5;
				System_printf("x: %f\n", adcFloat0);
			}
			else {
				System_printf("ADC X convert failed\n");
			}
	    	break;

	    case Board_ADC13:
			/* Blocking mode conversion */
			res = ADC_convert(adc, &adcValue1);

			if (res == ADC_STATUS_SUCCESS) {
				adcFloat1 = 0.000231*(float)adcValue1 - 2.5;
				System_printf("y: %f\n", adcFloat1);
			}
			else {
				System_printf("ADC Y convert failed\n");
			}
	    	break;

	    case Board_ADC11:
			/* Blocking mode conversion */
			res = ADC_convert(adc, &adcValue2);

			if (res == ADC_STATUS_SUCCESS) {
				adcFloat2 = 0.000231*(float)adcValue2 - 2.5;
				System_printf("z: %f\n", adcFloat2);
			}
			else {
				System_printf("ADC Z convert failed\n");
			}
	    	break;

	    default:
	    	System_printf("You Dun Goofed\n");
	    	break;
	    }
		System_flush();
	}
}


/*
 *  ======== main ========
 */
int main(void)
{
    Task_Params taskParams;

    /* Call board init functions */
    Board_initGeneral();
    Board_initADC();
    Board_initGPIO();
    Board_initSPI();



    /* Create tasks */
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    Task_construct(&task0Struct, (Task_FuncPtr)lcdFxn0, &taskParams, NULL);

    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task1Stack;
    taskParams.arg0 = Board_ADC14;
    Task_construct(&task1Struct, (Task_FuncPtr)taskFxn0, &taskParams, NULL);

    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task2Stack;
    taskParams.arg0 = Board_ADC13;
    Task_construct(&task2Struct, (Task_FuncPtr)taskFxn0, &taskParams, NULL);

    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task3Stack;
    taskParams.arg0 = Board_ADC11;
    Task_construct(&task3Struct, (Task_FuncPtr)taskFxn0, &taskParams, NULL);

    System_printf("Starting the ADC Single Channel example\nSystem provider is "
        "set to SysMin.  Halt the target to view any SysMin contents in ROV.\n");

    /* SysMin will only print to the console when you call flush or exit */
    System_flush();
    GPIO_write(Board_LED0, Board_LED_OFF);



    BIOS_start();

    return (0);
}

void drawTitle()
{
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext,
                                    "Accelerometer:",
                                    AUTO_STRING_LENGTH,
                                    64,
                                    30,
                                    OPAQUE_TEXT);

}
