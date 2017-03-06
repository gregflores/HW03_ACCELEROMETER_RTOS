/*
 * ShortPause.h
 *
 *  Created on: Mar 5, 2017
 *      Author: flore
 */

#ifndef SHORTPAUSE_H_
#define SHORTPAUSE_H_

void ST_init(void);
static void ST_irq(void);
void pauseUS(uint32_t pausTime);

#endif /* SHORTPAUSE_H_ */
