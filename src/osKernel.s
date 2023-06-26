		AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
		EXTERN  os_curr_task
		EXTERN  os_next_task
		EXPORT	PendSV_Handler
		EXPORT  osSchedulerLaunch
		EXPORT 	lock_mutex
		EXPORT 	unlock_mutex
			
		MACRO
		WAIT_FOR_UPDATE
		WFI   
		MEND
		
		MACRO
		SIGNAL_UPDATE          ; No software signalling operation
		MEND

locked    EQU 	1
unlocked  EQU 	0


PendSV_Handler          ;save r0,r1,r2,r3,r12,lr,pc,psr      
    CPSID     I                    ; Disable interrupts
    PUSH      {R4-R11}             ; Save the context of the current thread
    LDR       R0, =os_curr_task
    LDR       R1, [R0]             ; R1 = currentThread (R1 = &currentThread) 
    STR       SP, [R1]             ; Save SP into the currentThread's TCB
    LDR       R0, =os_next_task      ; R0 = &nextThread
    LDR       R1, [R0]             ; R1 = nextThread (R1 = &nextThread)
    LDR       SP, [R1]             ; Load SP from nextThread's TCB
    POP       {R4-R11}             ; Restore the context of the next thread
    CPSIE     I                    ; Enable interrupts
    BX        LR                   ; Return from interrupt
	

osSchedulerLaunch
    LDR       R0, =os_curr_task   ; R0 = &currentThread
    LDR       R2 , [R0]            ; R2 = currentThread (R0 = &currentThread)
    LDR       SP,  [R2]            ; Load SP from currentThread's TCB
    POP       {R4-R11}             ; Restore the context of the next thread
    POP       {R0-R3}              ; Pop R0-R3 from the stack
    POP       {R12}                ; Pop R12 from the stack
    ADD       SP, SP, #4           ; Pop the LR from the stack
    POP       {LR}                 ; Pop the PC from the stack
    CPSIE     I                    ; Enable interrupts
    BX        LR                   ; Return from interrupt

lock_mutex PROC
    LDR     r1, =locked
1   LDREX   r2, [r0]
    CMP     r2, r1        ; Test if mutex is locked or unlocked
        BEQ     %f2           ; If locked - wait for it to be released, from 2
    STREXNE r2, r1, [r0]  ; Not locked, attempt to lock it
    CMPNE   r2, #1        ; Check if Store-Exclusive failed
    BEQ     %b1           ; Failed - retry from 1
    ; Lock acquired
    DMB                   ; Required before accessing protected resource
    BX      lr

2   ; Take appropriate action while waiting for mutex to become unlocked
    WAIT_FOR_UPDATE
    B       %b1           ; Retry from 1
    ENDP

unlock_mutex PROC
    LDR     r1, =unlocked
    DMB                   ; Required before releasing protected resource
    STR     r1, [r0]      ; Unlock mutex
    SIGNAL_UPDATE       
    BX      lr
    ENDP



    ALIGN
    END