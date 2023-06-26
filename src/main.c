#include <stm32f767xx.h>
#include "osKernel.h"

#define RCC_GPIOB_EN 1<<1

#define MODER0_OUT 1 << 0
#define LED1_ON 1 << 0

#define MODER7_OUT 1<<14
#define LED2_ON 1 << 7

#define MODER14_OUT 1<<28
#define LED3_ON 1<<14

volatile uint32_t tick;
volatile uint32_t _tick;

unsigned int output_mutex = unlocked;

void GPIO_Init(void);
void User_LED1_on(void);
void User_LED2_on(void);
void User_LED3_on(void);
void User_LED1_off(void);
void User_LED2_off(void);
void User_LED3_off(void);

int count1=0;
int count2=0;
int count3=0;

#define Green_LED() __asm("SVC #0")
#define Blue_LED() __asm("SVC #1")
#define Red_LED() __asm("SVC #2")
#define LED_Off() __asm("SVC #3")

void Task1(void);
void Task2(void);
void Task3(void);

int main(void){
	GPIO_Init();
	
	static os_stack_t stack1[128];
	static os_stack_t stack2[128];
	static os_stack_t stack3[128];

	osKernelInit();

	osKernelThreadsInit(&Task1, stack1, 128);
	osKernelThreadsInit(&Task2, stack2, 128);
	osKernelThreadsInit(&Task3, stack3, 128);
	
	/* Context switch every second: */
	osKernelLaunch(1000);
	

}

void GPIO_Init(void){
	RCC->AHB1ENR |= RCC_GPIOB_EN;
	GPIOB->MODER |= MODER0_OUT;
	GPIOB->MODER |= MODER7_OUT;
	GPIOB->MODER |= MODER14_OUT;
}


void User_LED1_on(void){
	GPIOB->ODR |= LED1_ON;

}

void User_LED2_on(void){
	GPIOB->ODR |= LED2_ON;

}

void User_LED3_on(void){
	GPIOB->ODR |= LED3_ON;

}

void User_LED1_off(void){
	GPIOB->ODR = 0U;

}

void User_LED2_off(void){
	GPIOB->ODR = 0U;

}

void User_LED3_off(void){
	GPIOB->ODR = 0U;

}

void SVC_Handler(void)
{
  __asm(
    ".global SVC_Handler_Main\n"
    "TST lr, #4\n"
    "ITE EQ\n"
    "MRSEQ r0, MSP\n"
    "MRSNE r0, PSP\n"
    "B SVC_Handler_Main\n"
  ) ;
}

void SVC_Handler_Main( unsigned int *svc_args )
{
  unsigned int svc_number;

  /*
  * Stack contains:
  * r0, r1, r2, r3, r12, r14, the return address and xPSR
  * First argument (r0) is svc_args[0]
  */
  svc_number = ( ( char * )svc_args[ 6 ] )[ -2 ] ;
  switch( svc_number )
  {
    case 0:  
			GPIOB->ODR |= LED1_ON;
	break;
	case 1:
			GPIOB->ODR |= LED2_ON;
    break;
	case 2:
			GPIOB->ODR |= LED3_ON;
	break;
	case 3:
			GPIOB->ODR = 0U;
	break;
    default:    /* unknown SVC */
    break;
  }
}

static void delay(volatile uint32_t time)
{
	while (time > 0)
		time--;
}

void Task1(void)
{
	while (1) {
		lock_mutex(&output_mutex);
		GPIOB->ODR ^= LED1_ON;
		unlock_mutex(&output_mutex);
	
		delay(1000000);
		osThreadYeild();
		
	}
}

void Task2(void)
{
	while (1) {
		lock_mutex(&output_mutex);
		GPIOB->ODR ^= LED2_ON;
		unlock_mutex(&output_mutex);

		delay(500000);
			
	}
}

void Task3(void)
{
	while (1) {
		lock_mutex(&output_mutex);
		GPIOB->ODR ^= LED3_ON;
		unlock_mutex(&output_mutex);
	
		delay(100000);

	}
}


