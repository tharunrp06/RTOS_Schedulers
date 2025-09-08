#include "oskernel.h"

//Get yourself familiarize with linked lists for better understanding of this code

#define STACK_SIZE								100
#define NUMBER_OF_THREADS				  3
#define SYS_BUS										16000000

struct tcb{
	uint32_t *stackpt;
	struct tcb *nextpt;
};

typedef struct tcb tcbType;

tcbType *currentpt;
tcbType tcbs[NUMBER_OF_THREADS];
uint32_t tcbstack[NUMBER_OF_THREADS][STACK_SIZE];
uint32_t MILLIS_CONV;

static void osKernelStackInit(int i){
	tcbs[i].stackpt = &tcbstack[i][STACK_SIZE - 16]; 
	/* Here the Stack Pointer is initially stored. Here the stack grows downward i.e. if 200 is the current SP,
	when push is done data is stored in 196 and the sp will be 196. Similarly since 16 registers are available from 
	xPSR to R11 we set the SP as -16 from Size. So registers are saved and further operation will use stack from Stack-16*/
	
	tcbstack[i][STACK_SIZE - 1] = (1U<<24); //Setting the thumb state for ARM as Cortex M4 don't support ARM instructions
	
	//Stack size - 2 is left because next to PSR we will be having PC which we will be setting later according to the task
	
	tcbstack[i][STACK_SIZE - 3] = 0xFFFFFFFD; //LR value. This means that the stack pointer used will be PSP(Process Stack Pointer) in thread mode
	
	/*0xFFFFFFF1 = Handler Mode - MSP
	  0xFFFFFFF9 = Thread Mode  - MSP
	  0xFFFFFFFD = Thread Mode  - PSP*/
	
	//Here after the registers from R0 - R12 will be present. If you want to initialise some values you can or else give 0 for all(not mandatory)
	
	tcbstack[i][STACK_SIZE - 4] = 0;
	tcbstack[i][STACK_SIZE - 5] = 0;
	tcbstack[i][STACK_SIZE - 6] = 0;
	tcbstack[i][STACK_SIZE - 7] = 0;
	tcbstack[i][STACK_SIZE - 8] = 0;
	tcbstack[i][STACK_SIZE - 9] = 0;
	tcbstack[i][STACK_SIZE - 10] = 0;
	tcbstack[i][STACK_SIZE - 11] = 0;
	tcbstack[i][STACK_SIZE - 12] = 0;
	tcbstack[i][STACK_SIZE - 13] = 0;
	tcbstack[i][STACK_SIZE - 14] = 0;
	tcbstack[i][STACK_SIZE - 15] = 0;
	tcbstack[i][STACK_SIZE - 16] = 0;
}

void osKernelAddThread(void (*task0)(void),void (*task1)(void),void (*task2)(void)){
	//Here just 3 task are used. You can use whatever the tasks you want. If you don't know how much tasks, then it will be discussed in the further programs
	
	tcbs[0].nextpt = &tcbs[1]; //The tail gets connected to the head of next task which means we are storing the address of the next task in the current task
	tcbs[1].nextpt = &tcbs[2];
	tcbs[2].nextpt = &tcbs[0];
	
	osKernelStackInit(0);
	tcbstack[0][STACK_SIZE - 2] = (uint32_t)task0; // Setting the PC as the address of the thread to be executed
	
	osKernelStackInit(1);
	tcbstack[1][STACK_SIZE - 2] = (uint32_t)task1;
	
	osKernelStackInit(2);
	tcbstack[2][STACK_SIZE - 2] = (uint32_t)task2;
	
	currentpt = &tcbs[0]; //Setting the currentpt to starting thread
}

void osKernelInit(void){
	MILLIS_CONV = SYS_BUS / 16000; //1 value in Systick corresponds to 1 milli second
} 

void osKernelScheduler(uint32_t quanta){
	//Setup the SysTick Timer. Better to start this block with basic understanding of SysTick Timer
	
	SysTick->CTRL = 0; 
	
	SysTick->LOAD = (quanta * MILLIS_CONV) - 1; //-1 as count starts from 0
	
	SysTick->VAL = 0;
	
	SysTick->CTRL = (1U<<1) | (1U<<2) ; //Setting the clocksource as internal and enabling interrupts. Check Cortex M4 for register
	
	NVIC_SetPriority(SysTick_IRQn,15);
	
	SysTick->CTRL |= (1U<<0) ; //Enable the SysTick
	
	osSchedulerLaunch();
}

void osSchedulerLaunch(void){
	__ASM("LDR R0,=currentpt"); // Load the address of the current pointer to R0
	__ASM("LDR R2,[R0]"); // Load the content pointed by the value present in R0 which is the address of the pointer to the stackpointer
	__ASM("LDR SP,[R2]"); // Load the stack pointer value by dereferencing the pointer to stack pointer
	__ASM("POP {R4-R11}"); // Pop the registers from R4 to R11 first as the other registers will be stored in the top of the stack(by value)
	__ASM("POP {R0-R3}");  
	__ASM("POP {R12}");
	__ASM("ADD SP,SP,4"); //Skip LR and point [R2] to PC
	__ASM("POP {LR}"); //Here PC is stored in LR so that when return is executed the program will return to the address of the thread
	__ASM("ADD SP,SP,4");//Restoring the stack to -0 at the end
	__ASM("CPSIE I");	// Change Program Status to Enable Interrupt
	__ASM("BX LR"); // Branch to LR
  //Here we are initialing the stack for the first task for only one time. Further stack initialization will be done in SysTickIntHandler
}

__attribute__((naked))void SysTick_Handler(void){
	__ASM("CPSID I"); //Chance Program Status to Disable Interrupt
	__ASM("PUSH {R4-R11}"); // Other registers will be automatically pushed when calling interrupt
	__ASM("LDR R0,=currentpt"); // Load the address of the current pointer to R0
	__ASM("LDR R1,[R0]"); // Load the content pointed by the value present in R0 which is the address of the pointer to the stackpointer
	__ASM("STR SP,[R1]"); // Store the stack pointer value to R1 before context switch by dereferencing the pointer to stack pointer
	__ASM("LDR R1,[R1,#4]"); // Move the R1 pointer to the pointer for the next task(nextpt) so that r1 will now hold the address of the next task
	__ASM("STR R1,[R0]");//Store the address it in the current pointer
	__ASM("LDR SP,[R1]");//Load the SP from the value pointed by R1
	__ASM("POP {R4-R11}");//Restore the registers
	__ASM("CPSIE I");
	__ASM("BX LR");
}
	
	