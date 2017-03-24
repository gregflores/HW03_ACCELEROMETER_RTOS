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
#include <math.h>

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

uint8_t s1Flag;
float s1Float0;
float s1Float1;
float s1Float2;
float s1V;
float s1A;
float s1B;

uint8_t s2Flag;
float s2Float0;
float s2Float1;
float s2Float2;
float s2V;
float s2A;
float s2B;

void drawTitle(void);

void gpioButtonFxn0(unsigned int index)
{
	if(s1Flag == 0)
	{
		s1Flag = 1;
		/* Clear the GPIO interrupt and toggle an LED */
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
}

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
}

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
}

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
}
