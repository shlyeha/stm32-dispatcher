#include "dispatcher.h"


volatile task_typedef Task_table[MAX_TASK_CNT];
volatile task_context_typedef Context_table[MAX_TASK_CNT];
static volatile uint32_t Task_stack[MAX_TASK_CNT][TASK_STACK_SIZE] __attribute__((aligned(8))) ;
volatile uint32_t current_task;

const uint32_t MAX_TASK_COUNT = MAX_TASK_CNT;
const uint32_t INTERRUPT_STACK_FRAME_SIZE = sizeof(interrupt_stack_frame_typedef);
const uint32_t CONTEXT_SIZE = sizeof(task_context_typedef);
const uint32_t TASK_SIZE = sizeof(task_typedef);

void lock_task(volatile uint32_t table_number){
	Task_table[table_number].critical_lock = true;
}

void unlock_task(volatile uint32_t table_number){
	Task_table[table_number].critical_lock = false;
}

void stub_process(){
	Task_table[current_task].task_number = 0;
	while(1){
		__asm("wfi");
	}
}

void clear_context(volatile task_context_typedef * context){
	context->R4 = 0;
	context->R5 = 0;
	context->R6 = 0;
	context->R7 = 0;
	context->R8 = 0;
	context->R9 = 0;
	context->R10 = 0;
	context->R11 = 0;
}

void clear_interrupt_stack_frame(interrupt_stack_frame_typedef * frame){
	frame->R0 = 0;
	frame->R1 = 0;
	frame->R2 = 0;
	frame->R3 = 0;
	frame->R12 = 0;
	frame->LR = (uint32_t)&stub_process;
	frame->CPSR = 0x01000000;
}

void clear_sp(uint32_t task_number){
	volatile task_context_typedef * task_context;
	task_context = &Context_table[task_number];
	task_context->SP = (uint32_t)(&Task_stack[task_number][TASK_STACK_SIZE]) - INTERRUPT_STACK_FRAME_SIZE;
	
	interrupt_stack_frame_typedef * frame;
	frame = (interrupt_stack_frame_typedef *)task_context->SP;
	clear_interrupt_stack_frame(frame);
}



int add_task(char * name, void * entry, uint32_t arg){
	volatile task_typedef * task;
	volatile task_context_typedef * task_context;
	interrupt_stack_frame_typedef * frame;
	
	for (int i = 0; i < MAX_TASK_CNT; i++){
		task = &Task_table[i];
		if (task->task_number == 0){
			CRITICAL_LOCK
			
			task->task_number = i + 1;
			task->name = name;
			task->critical_lock = false;
			task->killed = false;
			
			task_context = task->context;
			clear_context(task->context);
			clear_sp(i);
			
			frame = (interrupt_stack_frame_typedef *)(task_context->SP);
			frame->PC = (uint32_t)entry;
			frame->R0 = arg;
			
			CRITICAL_UNLOCK
			return CREATE_TASK_SUCCESS;
		}
	}
	
	return CREATE_TASK_ERROR;
}

void remove_task(uint32_t table_number){
	volatile task_typedef * task;
	volatile task_context_typedef * task_context;
	interrupt_stack_frame_typedef * frame;
	
	task = &Task_table[table_number];
	
	CRITICAL_LOCK
	task_context = task->context;
	frame = (interrupt_stack_frame_typedef *)(task_context->SP);
	clear_interrupt_stack_frame(frame);
	clear_context(task->context);
	clear_sp(table_number);
	task->name = "";
	task->task_number = 0;
	task->critical_lock = false;
	task->killed = false;
	CRITICAL_UNLOCK
}

void kill_task_if_exists(uint32_t task_number){
	if (task_number > 1){
		for (int i = 0; i < MAX_TASK_CNT; i++){
			if (Task_table[i].task_number == task_number){
				Task_table[i].task_number = 0;
				Task_table[i].killed = true;
				break;
			}
		}
	}
}



void kill_task_by_name(char * name){
	CRITICAL_LOCK
	for (int i = 0; i < MAX_TASK_CNT; i++){
		if (strcmp(Task_table[i].name, name) == 0){
			kill_task_if_exists(Task_table[i].task_number);
			break;			
		}
	}
	CRITICAL_UNLOCK
}

void idle_process(){
	volatile task_typedef * task;
	
	while(1){
		for (uint32_t i = 0; i < MAX_TASK_CNT; i++){
			task = &Task_table[i];
			if (task->killed){
				remove_task(i);
			}
		}
	}
}

void init_tables(){
	volatile task_typedef * task;
	volatile task_context_typedef * task_context;
	
	for (int i = 0; i < MAX_TASK_CNT; i++){
		task = &Task_table[i];
		task_context = &Context_table[i];
		
		task->context = task_context;
		task->task_number = 0;
		task->name = "";
		
		for (int j = 0; j < TASK_STACK_SIZE; j++){
			Task_stack[i][j] = 0;
		}
	}
}

void dispatcher_init(void){
	current_task = 0;
	init_tables();
	add_task("System idle process", idle_process, 0);
}