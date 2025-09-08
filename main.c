#include "oskernel.h"

volatile uint32_t ptask0,ptask1,ptask2;

void task0(void){
	while(1){
		ptask0++;
	}
}

void task1(void){
	while(1){
		ptask1++;
	}
}

void task2(void){
	while(1){
		ptask2++;
	}
}

int main(){
	osKernelInit();
	osKernelAddThread(&task0,&task1,&task2);
	osKernelScheduler(10);
	return 1;
}