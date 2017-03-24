/*
 * Texas A&M University
 * 	Electronic Systems Engineering Technology
 * 	ENTC-489 Embedded Real Time Software Development
 * 	Author: Gregorio Flores & Adrian Morales
 * 	File: main.c
 * 	--------
 *	Main program for HW03_ACCELEROMETER_RTOS
 *	This file takes care of all the functionality for the assignment.
 *	This includes the button interrupt tasks, the lcd update task, and the adc convert tasks.
 *	The main function takes care of all the initialization for the MSP432.
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
#include <math.h>
/* LCD and Graphic Library files */
#include <grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
/* Initializes the graphics context */
Graphics_Context g_sContext;

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

/* ADC Converted g values */
float adcFloat0;
float adcFloat1;
float adcFloat2;

/* Switch/Button1 variables */
uint8_t s1Flag;
float s1Float0;
float s1Float1;
float s1Float2;
float s1V;
float s1A;
float s1B;

/* Switch/Button2 variables */
uint8_t s2Flag;
float s2Float0;
float s2Float1;
float s2Float2;
float s2V;
float s2A;
float s2B;

/* Function Definitions */
void drawTitle(void);

/*
 *  ======== gpioButtonFxn0 ========
 *  This is the interrupt for the first button. When the button is initially pressed
 *  there is a check on the flag. Initially the flag is 0 and will capture the current
 *  accelerometer values. An LED also toggles on to show the user the press was successful.
 *  The flag is also set to the next state. The function also checks if the flag is set to
 *  2. If it is, it will increment the flag to the next state and toggle the LED off.
 *  This is done to clear the screen in the lcd task.
 */
void gpioButtonFxn0(unsigned int index)
{
	if(s1Flag == 0)
	{
		s1Flag = 1;
		s1Float0 = adcFloat0;
		s1Float1 = adcFloat1;
		s1Float2 = adcFloat2;
		GPIO_toggle(Board_LED0);
	}
	else if(s1Flag == 2)
	{
		s1Flag = 3;
		GPIO_toggle(Board_LED0);
	}
} /* gpioButtonFxn */

/*
 *  ======== gpioButtonFxn1 ========
 *  This is the interrupt for the second button. It functions in the same way as the first.
 */
void gpioButtonFxn1(unsigned int index)
{
	if(s2Flag == 0)
	{
		s2Flag = 1;
		/* Clear the GPIO interrupt and toggle an LED */
		s2Float0 = adcFloat0;
		s2Float1 = adcFloat1;
		s2Float2 = adcFloat2;
		GPIO_toggle(Board_LED1);
	}
	else if(s2Flag == 2)
	{
		s2Flag = 3;
		GPIO_toggle(Board_LED1);
	}
} /* gpioButtonFxn */

/*
 *  ======== lcdFxn0 ========
 *  This is the LCD RTOS task. It initializes the LCD using the RTOS modified functions.
 *  It then draws the initial labels Accel:, S1, and S2.
 *  The task begins drawing the X,Y, and Z, accelerometer data.
 *  There are then checks for the button flags that determine whether or not the values are
 *  locked in and drawn in their respective position. Ther is also a check to clear these values
 *  when the button is pressed again. I am pretty sure I injured my finger typing this out.
 */
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
    /*
     * The start of the display updating. The update takes place every 500 ms.
     * Here the adcFloat values are converted to strings with up to 3
     * decimal places. These strings are then written to the LCD. They are all left oriented.
     */
    while(1)
    {
    	Task_sleep(100);
	    char string[8];
	    sprintf(string, "X: %.3f", adcFloat0);
	    Graphics_drawStringCentered(&g_sContext,
	                                    (int8_t *)string,
	                                    8,
	                                    28,
	                                    20,
	                                    OPAQUE_TEXT);
	    sprintf(string, "Y: %.3f", adcFloat1);
	    Graphics_drawStringCentered(&g_sContext,
	                                    (int8_t *)string,
	                                    8,
	                                    28,
	                                    30,
	                                    OPAQUE_TEXT);
	    sprintf(string, "Z: %.3f", adcFloat2);
	    Graphics_drawStringCentered(&g_sContext,
	                                    (int8_t *)string,
	                                    8,
	                                    28,
	                                    40,
	                                    OPAQUE_TEXT);
	    /*
	     * Here the button flag is checked and if set to 1, the vector and angle conversion takes place on the captured values.
	     * This is then converted to a string and printed to the LCD.
	     */
	    if(s1Flag == 1)
	    {
	    	s1V = sqrt(s1Float0*s1Float0 + s1Float1*s1Float1 + s1Float2*s1Float2);
	    	s1A = atan2(s1Float1, s1Float0);
	    	s1B = atan2(s1Float2, s1Float1);
		    char string[8];
		    sprintf(string, "v: %.3f", s1V);
		    Graphics_drawStringCentered(&g_sContext,
		                                    (int8_t *)string,
		                                    8,
		                                    28,
		                                    60,
		                                    OPAQUE_TEXT);
		    sprintf(string, "a: %.3f", s1A);
		    Graphics_drawStringCentered(&g_sContext,
		                                    (int8_t *)string,
		                                    8,
		                                    28,
		                                    70,
		                                    OPAQUE_TEXT);
		    sprintf(string, "b: %.3f", s1B);
		    Graphics_drawStringCentered(&g_sContext,
		                                    (int8_t *)string,
		                                    8,
		                                    28,
		                                    80,
		                                    OPAQUE_TEXT);
		    s1Flag = 2;
	    }
	    /*
	     * Here there is a check if the button has been pressed a second time. If so The lines where
	     * the V, A, and B, values were printed is cleared.
	     */
	    else if (s1Flag == 3)
	    {
	        Graphics_drawStringCentered(&g_sContext,
	                                        "        ",
	                                        AUTO_STRING_LENGTH,
	                                        28,
	                                        60,
	                                        OPAQUE_TEXT);
	        Graphics_drawStringCentered(&g_sContext,
	                                        "        ",
	                                        AUTO_STRING_LENGTH,
	                                        28,
	                                        70,
	                                        OPAQUE_TEXT);
	        Graphics_drawStringCentered(&g_sContext,
	                                        "        ",
	                                        AUTO_STRING_LENGTH,
	                                        28,
	                                        80,
	                                        OPAQUE_TEXT);
	        s1Flag = 0;
	    }
	    if(s2Flag == 1)
	    {
	    	s2V = sqrt(s2Float0*s2Float0 + s2Float1*s2Float1 + s2Float2*s2Float2);
	    	s2A = atan2(s2Float1, s2Float0);
	    	s2B = atan2(s2Float2, s2Float1);
		    char string[8];
		    sprintf(string, "v: %.3f", s2V);
		    Graphics_drawStringCentered(&g_sContext,
		                                    (int8_t *)string,
		                                    8,
		                                    28,
		                                    100,
		                                    OPAQUE_TEXT);
		    sprintf(string, "a: %.3f", s2A);
		    Graphics_drawStringCentered(&g_sContext,
		                                    (int8_t *)string,
		                                    8,
		                                    28,
		                                    110,
		                                    OPAQUE_TEXT);
		    sprintf(string, "b: %.3f", s2B);
		    Graphics_drawStringCentered(&g_sContext,
		                                    (int8_t *)string,
		                                    8,
		                                    28,
		                                    120,
		                                    OPAQUE_TEXT);
		    s2Flag = 2;
	    }
	    else if (s2Flag == 3)
	    {
	        Graphics_drawStringCentered(&g_sContext,
	                                        "        ",
	                                        AUTO_STRING_LENGTH,
	                                        28,
	                                        100,
	                                        OPAQUE_TEXT);
	        Graphics_drawStringCentered(&g_sContext,
	                                        "        ",
	                                        AUTO_STRING_LENGTH,
	                                        28,
	                                        110,
	                                        OPAQUE_TEXT);
	        Graphics_drawStringCentered(&g_sContext,
	                                        "        ",
	                                        AUTO_STRING_LENGTH,
	                                        28,
	                                        120,
	                                        OPAQUE_TEXT);
	        s2Flag = 0;
	    }
    }
} /* lcdFxn0 */

/*
 *  ======== taskFxn0 ========
 *  This task takes care of the ADC initialization and the conversions.
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

	/*
	 * The adc conversions are done at a rate of 5Hz
	 */
	while(1)
	{
		Task_sleep(40);


	    switch(arg0)
	    {
	    case Board_ADC14:
			/* Blocking mode conversion */
			res = ADC_convert(adc, &adcValue0);

			if (res == ADC_STATUS_SUCCESS) {
				adcFloat0 = 0.000231*(float)adcValue0 - 2.5;
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
} /* taskFxn0 */


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

    /* install Button callback */
    GPIO_setCallback(EDUMKII_B1, gpioButtonFxn0);

    /* Enable interrupts */
    GPIO_enableInt(EDUMKII_B1);

    /* install Button callback */
    GPIO_setCallback(EDUMKII_B2, gpioButtonFxn1);

    /* Enable interrupts */
    GPIO_enableInt(EDUMKII_B2);

    BIOS_start();

    return (0);
} /* main */

/*
 *  ======== drawTitle ========
 */
void drawTitle()
{
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext,
                                    "Accel:",
                                    AUTO_STRING_LENGTH,
                                    20,
                                    10,
                                    OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext,
                                    "S1",
                                    AUTO_STRING_LENGTH,
                                    8,
                                    50,
                                    OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext,
                                     "S2",
                                     AUTO_STRING_LENGTH,
                                     8,
                                     90,
                                     OPAQUE_TEXT);
}/* drawTitle */
