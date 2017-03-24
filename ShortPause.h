/* Texas A&M University
** Electronics Systems Engineering Technology
** ENTC-489 Embedded Real Time Software Development
** Author: Mike Willey
** File: shortpause.h
** ---------
** RTOS Support for MSP432 short timer (uS resolution)
*/

#ifndef SHORTPAUSE_H_
#define SHORTPAUSE_H_

void ST_init(void);
void pauseUS(uint32_t pauseTime);

#endif /* SHORTPAUSE_H_ */
