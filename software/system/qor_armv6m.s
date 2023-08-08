.syntax unified
.cpu cortex-m0
.fpu softvfp
.thumb
.code   16
.thumb_func
 
@ The .global directive gives the symbols external linkage.
.global qor_go
.global qor_switch_context
.global qor_sleep

.extern RunPt
.extern qor_scheduler
.extern qor_svc_call

.section    .text.qor_go
.type	qor_go, %function

@   The Cortex-M0 is limited on the thumb number of instructionss
@ the context restore is therefore a bit more complex that M3 or M4 CPU
@  One limitation is the POP instruction that cannot access to the HI registers (registers after R7)

qor_go:
    
    LDR     r2, =RunPt              @ R0 = &RunPt;  // TCB_t**  R0 = &RunPt  // R0 contient l'adresse de la variable 'RunPt', qui est un pointeur vers la structure TCB courante
    LDR     r3, [r2]                @ R1 = *R0;     // TCB_t*   R1 = RunPt // R1 pointe maintenant sur RunPt
    LDR     r0, [r3]                @ R2 = *R1      // On veut la vleur pointée par RunPt (premier élément, donc même adresse) qui correspon à la variable *sp (c'est un pointeur vers la stack)
    
    adds    r0, #32                    @  Discard everything up to r0.
    msr     psp, r0                 @ SP = R2;     // uint32_t SP = R2
                                    @ now we switched to the thread's stack, which we populated before

    movs     r0, #2
    msr     CONTROL, r0             @ ensure we use PSP not MSP


    isb                 @       throw away any prefetched instructions at this point
    pop  {r0-r5}       @ R0 contains the argument, R1 the link register
    mov  lr, r5         @ Update LR
    pop  {r3}           @ Task code
    pop  {r2}           @ Pop and discard XPSR. */
    cpsie i             @ Enable interrupts
    bx   r3             @        /* Finally, jump to the user defined task code. */




@   The fn OSAsm_ThreadSwitch, implemented in os_asm.s, is periodically called by the SchedlTimer (ISR).
@   It preemptively switches to the next thread, that is, it stores the stack of the running
@   thread and restores the stack of the next thread.
@   It calls OS_Schedule to determine which thread is run next and update RunPt.
 

@ EXC_RETURN value to return to Thread mode, while restoring state from PSP.
.equ EXC_RETURN, 0xfffffffd

.section    .text.qor_switch_context
.type	qor_switch_context, %function
qor_switch_context:

        .thumb
        .syntax unified


@ Les registers r0-r3 r12 etc. sont sauvegardés par le processeur
        
        cpsid i
        mrs     r0, psp  @ get the current stack address ()

        ldr     r1, =RunPt
        ldr     r2, [r1]

        @ Subtract room for the registers we are going to store. We have to pre-subtract
        @ and use the incrementing store multiple instruction because the CM0+ doesn't
        @ have the decrementing variant.
        subs    r0, r0, #32
        str r0, [r2]     @                   Save the new top of stack

        @ Save registers on the stack. This has to be done in two stages because
        @ the stmia instruction cannot access the upper registers on the CM0+.
        stmia   r0!, {r4-r7}
        mov     r4, r8
        mov     r5, r9
        mov     r6, r10
        mov     r7, r11
        stmia   r0!, {r4-r7}

        bl  qor_scheduler

        ldr     r0, =RunPt
        ldr     r1, [r0]
        ldr     r0, [r1]   @ R0 is the stack address
        
        @ On est obligé de poper les registres haut d'abord (on n'a pas accès directement à ceux-ci via ldmia)
        adds    r0, r0, #16  @ move to hi regs 
        ldmia   r0!, {r4-r7} @ pop
        mov     r8, r4
        mov     r9, r5
        mov     r10, r6
        mov     r11, r7


        msr     psp, r0      @ new task stack top address
     //   str     r0, [r1]     @                   Save the new top of stack

        subs r0, r0, #32
        ldmia r0!, {r4-r7} 

        // Exit handler. Using a bx to the special EXC_RETURN values causes the
        // processor to perform the exception return behavior.
        ldr     r0, =EXC_RETURN
        cpsie   i
        bx      r0

