/* Texas A&M University
** Electronics Systems Engineering Technology
** ENTC-489 Embedded Real Time Software Development
** Author: Mike Willey
** File: shortpause.c
** ---------
** RTOS Support for MSP432 short timer (uS resolution)
*/

/* standard includes */
#include <stdlib.h>
#include <stdbool.h>

/* sysbios includes */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/hal/Timer.h>
#include <ti/sysbios/gates/GateMutexPri.h>

#include "shortpause.h"

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

/* function prototype for static functions */
static void ST_irq(void);

/** Short timer initialize.  Uses the Timer A timers supported
 *  by the TI RTOS drivers.  TimerA 3 is the one that is used
 *  set by the SHORT_TIMER_INDEX.  It has been verified that
 *  the HAL library sets timerA to clock off of ACLK which is
 *  currently set to 32768.  Refer to documentation on
 *  module ti.sysbios.hal.Timer.  This makes the resolution
 *  of the short timers about 30uS.
 */
void ST_init(void)
{
	/* create the short timer mutex instance */
	GateMutexPri_Params_init(&mtxParams);
	GateMutexPri_construct(&shortTimerMtxStruct,&mtxParams);
	shortTimerMtxHand = GateMutexPri_handle(&shortTimerMtxStruct);

	/* create the short timer semaphore instance */
	Semaphore_Params_init(&semParams);
	semParams.mode = Semaphore_Mode_BINARY;
	Semaphore_construct(&shortTimerSemStruct,1,&semParams);
	shortTimerSem = Semaphore_handle(&shortTimerSemStruct);

	/* create the timer instance */
	Timer_Params_init(&timerParams);
	timerParams.periodType = Timer_PeriodType_MICROSECS;
	timerParams.runMode = Timer_RunMode_CONTINUOUS;
	timerParams.startMode = Timer_StartMode_USER;
	timerParams.extFreq.hi = 0;
	timerParams.extFreq.lo = TIMER_CLOCK_FREQ;
	Timer_construct(&shortTimerStruct,SHORT_TIMER_INDEX,(Timer_FuncPtr)ST_irq,&timerParams,NULL);
	shortTimerHand = Timer_handle(&shortTimerStruct);
}

/** Short Timer IRQ handler.  Function that is called when the
 *  short timer expires.
 */
static void ST_irq(void)
{
	/* stop the timer because it will interrupt again if we don't */
	Timer_stop(shortTimerHand);
	Semaphore_post(shortTimerSem);
}

/** Pause for microsecond increments.  Function that will
 *  pend the calling task for the specified number of microsecond.
 *  See ST_init() for details on the granularity of this timer.
 *  This function uses a mutex to protect against multiple tasks
 *  using the timer at the same time.
 */
void pauseUS(uint32_t pauseTime)
{
	uint64_t pauseCount;
	IArg mutexKey;

	/* this function has side effects (using a single hardware timer)
	 * so use a mutex to protect it */
	mutexKey = GateMutexPri_enter(shortTimerMtxHand);

	/* scale for a timer clocked at TIMER_CLOCK_FREQ */
	pauseCount = ((uint64_t)pauseTime * (uint64_t)TIMER_CLOCK_FREQ) / (uint64_t)1000000L;
	/* make sure the bounds of the count are supported by the timer */
	if (pauseCount > 0xffff)
	{
		pauseCount = 0xffff;
	}
	if (pauseCount == 0)
	{
		pauseCount = 1;
	}
	Semaphore_pend(shortTimerSem,BIOS_NO_WAIT);

	/* set the count and start the timer */
	Timer_setPeriod(shortTimerHand,(uint32_t)pauseCount);
	Timer_start(shortTimerHand);

	/* wait for the timeout */
	Semaphore_pend(shortTimerSem,BIOS_WAIT_FOREVER);

	/* free the mutex */
	GateMutexPri_leave(shortTimerMtxHand,mutexKey);
}
