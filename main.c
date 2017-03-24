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

/* Example/Board Header files */
#include "Board.h"
#include <stdio.h>


#include <grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"

Graphics_Context g_sContext;

/* ADC sample count */
#define ADC_SAMPLE_COUNT  (10)

/*Task Specific defines */
#define TASKSTACKSIZE     (768)

Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

Task_Struct task1Struct;
Char task1Stack[TASKSTACKSIZE];

Task_Struct task2Struct;
Char task2Stack[TASKSTACKSIZE];

/* ADC conversion result variables */
uint16_t adcValue0;
uint16_t adcValue1;
uint16_t adcValue2;

void drawTitle(void);

/*
 *  ======== taskFxn0 ========
 *  Open an ADC instance and get a sampling result from a one-shot conversion.
 */
Void taskFxn0(UArg arg0, UArg arg1)
{
    Crystalfontz128x128_Init();
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

    /* Initializes graphics context */
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
    drawTitle();
	ADC_Handle   adc;
	ADC_Params   params;
	int_fast16_t res;

	ADC_Params_init(&params);
	adc = ADC_open(Board_ADC14, &params);
	while(1)
	{
		Task_sleep(200);

	    char string[8];



		if (adc == NULL) {
			System_abort("Error initializing ADC channel 0\n");
		}


		/* Blocking mode conversion */
		res = ADC_convert(adc, &adcValue0);

		if (res == ADC_STATUS_SUCCESS) {
			System_printf("x: %i\n", adcValue0);
		    sprintf(string, "X: %5d", adcValue0);
		    Graphics_drawStringCentered(&g_sContext,
		                                    (int8_t *)string,
		                                    8,
		                                    64,
		                                    50,
		                                    OPAQUE_TEXT);
		}
		else {
			System_printf("ADC channel 14 convert failed\n");
		}

		//ADC_close(adc);

		System_flush();
	}
}

/*
 *  ======== taskFxn1 ========
 *  Open a ADC handle and get a array of sampling results after
 *  calling several conversions.
 */
/*Void taskFxn1(UArg arg0, UArg arg1)
{
	while(1)
	{
		Task_sleep(200);
		ADC_Handle   adc;
		ADC_Params   params;
		int_fast16_t res;

		ADC_Params_init(&params);
		adc = ADC_open(Board_ADC13, &params);

		if (adc == NULL) {
			System_abort("Error initializing ADC channel 1\n");
		}



		res = ADC_convert(adc, &adcValue1);

		if (res == ADC_STATUS_SUCCESS) {
			System_printf("y: %i\n", adcValue1);
		}
		else {
			System_printf("ADC channel 13 convert failed\n");
		}

		System_flush();

		ADC_close(adc);
	}
}

Void taskFxn2(UArg arg0, UArg arg1)
{
	while(1)
	{
		Task_sleep(200);
		ADC_Handle   adc;
		ADC_Params   params;
		int_fast16_t res;

		ADC_Params_init(&params);
		adc = ADC_open(Board_ADC11, &params);

		if (adc == NULL) {
			System_abort("Error initializing ADC channel 1\n");
		}



		res = ADC_convert(adc, &adcValue0);

		if (res == ADC_STATUS_SUCCESS) {
			System_printf("z: %i\n\n\n\n", adcValue0);
		}
		else {
			System_printf("ADC channel 11 convert failed\n");
		}

		ADC_close(adc);

		System_flush();
	}
}
*/
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
    Task_construct(&task0Struct, (Task_FuncPtr)taskFxn0, &taskParams, NULL);
/*
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task1Stack;
    Task_construct(&task1Struct, (Task_FuncPtr)taskFxn1, &taskParams, NULL);

    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task2Stack;
    Task_construct(&task2Struct, (Task_FuncPtr)taskFxn2, &taskParams, NULL);
*/
    System_printf("Starting the ADC Single Channel example\nSystem provider is "
        "set to SysMin.  Halt the target to view any SysMin contents in ROV.\n");

    /* SysMin will only print to the console when you call flush or exit */
    System_flush();




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
