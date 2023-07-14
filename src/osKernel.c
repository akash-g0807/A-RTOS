#include "osKernel.h"
#include <stm32f767xx.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define NUM_THREADS 3

#define BUS_CLOCK_FREQ 16000000
#define SYSPRI3         (*((volatile uint32_t *)0xE000ED20))
#define INTCTRL         (*((volatile uint32_t *)0xE000ED04))

uint32_t MILLIS_PRESCALER;

void osSchedulerLaunch(void);

typedef enum {
	OS_TASK_STATUS_IDLE,
	OS_TASK_STATUS_ACTIVE
} os_task_status_t;

typedef struct {
    uint32_t stackPtr;
    os_task_status_t status;
    void (*handler)(void);

} tcb_t;

static struct {
	tcb_t threads[NUM_THREADS];
	volatile uint32_t current_task;
	uint32_t size;
} thread_table;


volatile tcb_t *os_curr_task;
volatile tcb_t *os_next_task;

static void task_finished(void)
{
	/* This function is called when some task handler returns. */
	volatile uint32_t i = 0;
	while (1)
		i++;
}

void osKernelInit(void)
{
		memset(&thread_table, 0, sizeof(thread_table));
    MILLIS_PRESCALER = BUS_CLOCK_FREQ/1000;
}

bool osKernelThreadsInit(void (*handler)(void), os_stack_t *p_stack, uint32_t stack_size){
	if (thread_table.size >= NUM_THREADS)
		return false;	// No more tasks can be created

	/* Initialize the task structure and set SP to the top of the stack
	   minus 16 words (64 bytes) to leave space for storing 16 registers: */
	tcb_t *p_task = &thread_table.threads[thread_table.size];
	p_task->handler = handler;
	p_task->stackPtr = (uint32_t)(p_stack + stack_size - 16);
	p_task->status = OS_TASK_STATUS_IDLE;

	/* Initialize the stack: */
	p_stack[stack_size-1] = 0x01000000;  // xPSR
	p_stack[stack_size-2] = (uint32_t)handler;  // PC
	p_stack[stack_size-3] = (uint32_t)task_finished;  // LR

	thread_table.size++;

	return true;

}


bool osKernelLaunch(uint32_t systick_ticks)
{
	NVIC_SetPriority(PendSV_IRQn, 0xff); /* Lowest possible priority */
	NVIC_SetPriority(SysTick_IRQn, 0x00); /* Highest possible priority */

	SysTick->CTRL = 0;
	SysTick->VAL = 0;
	SysTick->LOAD = (systick_ticks*MILLIS_PRESCALER) -1;
	SYSPRI3 =(SYSPRI3&0x00FFFFFF)|0xE0000000; // priority 7
	SysTick->CTRL =0x00000007;

	/*
	uint32_t ret_val = SysTick_Config(systick_ticks);
	if (ret_val != 0)
		return false;
	*/
	os_curr_task = &thread_table.threads[thread_table.current_task];
	
	os_curr_task->handler();
	osSchedulerLaunch();

	return true;
}

void SysTick_Handler(void)
{
	os_curr_task = &thread_table.threads[thread_table.current_task];
	os_curr_task->status = OS_TASK_STATUS_IDLE;

	/* Select next task: */
	thread_table.current_task++;
	if (thread_table.current_task >= thread_table.size)
		thread_table.current_task = 0;

	os_next_task = &thread_table.threads[thread_table.current_task];
	os_next_task->status = OS_TASK_STATUS_ACTIVE;

	/* Trigger PendSV which performs the actual context switch: */
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void osThreadYeild(void){
	
	SysTick->VAL = 0;
	/* Trigger an systick interrupt by setting bit to high*/
	INTCTRL = 0x04000000;
	
}

