/*
 * ShortPause.c
 *
 *  Created on: Mar 5, 2017
 *      Author: flore
 */






/* sysbios include */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/hal/Timer.h>
#include <ti/sysbios/gates/GateMutexPri.h>

#include "ShortPause.h"

#define SHORT_TIMER_INDEX 3
#define TIMER_CLOCK_FREQ 32768

/* short pause semaphore declarations */
static Semaphore_Handle shortTimerSem;
static Semaphore_Struct shortTimerSemStruct;
static Semaphore_Params semParams;

/* short timer declarations */
static Timer_Struct shortTimerStruct;
static Timer_Params timerParams;
static Timer_Handle shortTimerHand;

/* short timer mutex declarations */
static GateMutexPri_Struct shortTimerMtxStruct;
static GateMutexPri_Handle shortTimerMtxHand;
static GateMutexPri_Params mtxParams;

void ST_init(void)
{
	/* create the short timer mutex instance */
	GateMutexPri_Params_init(&mtxParams);
	GateMutexPri_construct(&shortTimerMtxStruct, &mtxParams);
	shortTimerMtxHand = GateMutexPri_handle(&shortTimerMtxStruct);

	/* create the short timer sempahore instance */
	Semaphore_Params_init(&semParams);
	semParams.mode = Semaphore_Mode_BINARY;
	Semaphore_construct(&shortTimerSemStruct, 1, &semParams);
	shortTimerSem = Semaphore_handle(&shortTimerSemStruct);

	/* create the timer instance */
	Timer_Params_init(&timerParams);
	timerParams.periodType = Timer_PeriodType_MICROSECS;
	timerParams.runMode = Timer_RunMode_CONTINUOUS;
	timerParams.startMode = Timer_StartMode_USER;
	timerParams.extFreq.hi = 0;
	timerParams.extFreq.lo = TIMER_CLOCK_FREQ;
	Timer_construct(&shortTimerStruct, SHORT_TIMER_INDEX, (Timer_FuncPtr)ST_irq, &timerParams, NULL);
	shortTimerHand = Timer_handle(&shortTimerStruct);
}

static void ST_irq(void)
{
	/* stop the timer because eit will interrupt again if we don't */
	Timer_stop(shortTimerHand);
	Semaphore_post(shortTimerSem);
}

void pauseUS(uint32_t pauseTime)
{
	uint64_t pauseCount;
	IArg mutexKey;

	/* this fucntion has side effects (using a signle hardware timer)
	 * so use a mutex to protect it */
	mutexKey = GateMutexPri_enter(shortTimerMtxHand);

	/*scale for a timer clocked at TIMER_CLOCK_FREQ */
	pauseCount = ((uint64_t)pauseTime * (uint64_t)TIMER_CLOCK_FREQ) / (uint64_t)1000000L;
	/* make sure the bound of the count are supported by the timer */
	if (pauseCount > 0xffff)
	{
		pauseCount = 0xffff;
	}
	if (pauseCount == 0)
	{
		pauseCount = 1;
	}

	/* set the count and start the timer */
	Timer_setPeriod(shortTimerHand, (uint32_t)pauseCount);
	Timer_start(shortTimerHand);

	/* wait for the timeout */
	Semaphore_pend(shortTimerSem, BIOS_WAIT_FOREVER);

	/* free the mutex */
	GateMutexPri_leave(shortTimerMtxHand, mutexKey);
}
