#ifndef __OS_KERNEL_H__
#define __OS_KERNEL_H__

#include "stm32f401xc.h"
#include <stdint.h>

void osSchedulerLaunch(void);
void osKernelScheduler(uint32_t quanta);
void osKernelInit(void);
void osKernelAddThread(void (*task0)(void),void (*task1)(void),void (*task2)(void));

#endif
