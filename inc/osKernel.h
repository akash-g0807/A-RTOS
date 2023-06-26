#ifndef _OS_KERNEL_H
#define _OS_KERNEL_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define locked   	 1
#define unlocked   0

extern void lock_mutex(void * mutex);
extern void unlock_mutex(void * mutex);

typedef uint32_t os_stack_t;

bool osKernelLaunch(uint32_t systick_ticks);

void osKernelInit(void);

bool osKernelThreadsInit(void (*handler)(void), os_stack_t *p_stack, uint32_t stack_size);

void osThreadYeild(void);

#endif

