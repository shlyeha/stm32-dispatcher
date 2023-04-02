#ifndef DISPATCHER_H
#define DISPATCHER_H

	#include <stdint.h>
	#include <stdbool.h>
	
	#define MAX_TASK_CNT									12
	#define CREATE_TASK_SUCCESS						0
	#define CREATE_TASK_ERROR							1
	#define TASK_STACK_SIZE								0xff
	#define CRITICAL_LOCK									lock_task(current_task);
	#define CRITICAL_UNLOCK								unlock_task(current_task);

	typedef __attribute__((packed)) struct{
		uint32_t R0;
		uint32_t R1;
		uint32_t R2;
		uint32_t R3;
		uint32_t R12;
		uint32_t LR;
		uint32_t PC;
		uint32_t CPSR;
	} interrupt_stack_frame_typedef;
	
	typedef __attribute__((packed)) struct{
		uint32_t R4;
		uint32_t R5;
		uint32_t R6;
		uint32_t R7;
		uint32_t R8;
		uint32_t R9;
		uint32_t R10;
		uint32_t R11;
		uint32_t SP;	//ptr to scack stored r0-r3, r12, lr, pc, cpsr
	} task_context_typedef;
	
	typedef __attribute__((packed)) struct{
		char * name;
		uint32_t task_number;
		volatile task_context_typedef * context;
		bool critical_lock;
		bool killed;
	} task_typedef;
	
	void dispatcher_init(void);
	int add_task(char * name, void * entry, uint32_t arg);
	void kill_task_if_exists(uint32_t task_number);
	void kill_task_by_name(char * name);

#endif	//DISPATCHER_H
