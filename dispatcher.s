					area |.text|, code, readonly
					export SysTick_Handler
					export dispatcher_start
						
					import Task_table
					import Context_table
					import MAX_TASK_COUNT
					import INTERRUPT_STACK_FRAME_SIZE
					import CONTEXT_SIZE
					import TASK_SIZE
					import current_task

SysTick_Handler		proc
						; r0-r3, r12, lr, pc, spcr are now in current task stack
						; now store current task context
						mov32 r0, current_task
						ldr r0, [r0]
						
						; check if task is critically locked, then don't change task, just exit
						mov32 r1, TASK_SIZE
						ldr r1, [r1]
						mul r1, r1, r0
						add r1, #0x0c
						mov32 r2, Task_table
						add r2, r2, r1
						ldrb r2, [r2]
						mov32 r1, #0x01
						cmp r1, r2
						beq exit_systick_handler
						
						mov32 r1, CONTEXT_SIZE
						ldr r1, [r1]
						mul r1, r1, r0
						mov32 r2, Context_table
						add r2, r1
						
						str r4, [r2]
						str r5, [r2, #0x04]
						str r6, [r2, #0x08]
						str r7, [r2, #0x0c]
						str r8, [r2, #0x10]
						str r9, [r2, #0x14]
						str r10, [r2, #0x18]
						str r11, [r2, #0x1c]
						str sp, [r2, #0x20]
						
						; now search not-null task_number in task table
						mov32 r1, TASK_SIZE
						ldr r1, [r1]
						mov32 r2, Task_table
						mov32 r3, MAX_TASK_COUNT
						ldr r3, [r3]
						
next_task				add r0, #1
						cmp r0, r3
						moveq r0, #0
						cmp r0, #0
						beq serve_task
						mul r4, r1, r0
						add r4, r2
						ldr r4, [r4, #0x04]
						cmp r4, #0
						beq next_task
							
serve_task				; now select task context and execute the task
						mov32 r1, current_task
						str r0, [r1]
						mov32 r1, CONTEXT_SIZE
						ldr r1, [r1]
						mul r1, r1, r0
						mov32 r2, Context_table
						add r2, r1
						
						ldr r4, [r2]
						ldr r5, [r2, #0x04]
						ldr r6, [r2, #0x08]
						ldr r7, [r2, #0x0c]
						ldr r8, [r2, #0x10]
						ldr r9, [r2, #0x14]
						ldr r10, [r2, #0x18]
						ldr r11, [r2, #0x1c]
						ldr sp, [r2, #0x20]
						isb
						
exit_systick_handler	bx lr
					endp
						
dispatcher_start	proc
						mov r0, #0
						mov32 r1, CONTEXT_SIZE
						ldr r1, [r1]
						mul r1, r1, r0
						mov32 r2, Context_table
						add r2, r1
						
						ldr r4, [r2]
						ldr r5, [r2, #0x04]
						ldr r6, [r2, #0x08]
						ldr r7, [r2, #0x0c]
						ldr r8, [r2, #0x10]
						ldr r9, [r2, #0x14]
						ldr r10, [r2, #0x18]
						ldr r11, [r2, #0x1c]
						ldr sp, [r2, #0x20]
						isb
						mov r0, sp
						add r0, #0x18
						ldr lr, [r0]
						add r0, #0x04
						mov sp, r0
						
						bx lr
					endp


					align
					end