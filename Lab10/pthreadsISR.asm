;    threadsISR.asm    08/07/2015
;*******************************************************************************
;  STATE        Task Control Block        Stacks (malloc'd)
;                          ______                                       T0 Stack
;  Running tcbs[0].thread | xxxx-+------------------------------------->|      |
;                 .stack  | xxxx-+-------------------------------       |      |
;                 .block  | 0000 |                               \      |      |
;                 .retval |_0000_|                       T1 Stack \     |      |
;  Ready   tcbs[1].thread | xxxx-+---------------------->|      |  \    |      |
;                 .stack  | xxxx-+------------------     |      |   \   |      |
;                 .block  | 0000 |                  \    |      |    -->|(exit)|
;                 .retval |_0000_|        T2 Stack   --->|r4-r15|       |------|
;  Blocked tcbs[2].thread | xxxx-+------->|      |       |  SR  |
;                 .stack  | xxxx-+---     |      |       |  PC  |
;                 .block  |(sem) |   \    |      |       |(exit)|
;                 .retval |_0000_|    --->|r4-r15|       |------|
;  Free    tcbs[3].thread | 0000 |        |  SR  |
;                 .stack  | ---- |        |  PC  |
;                 .block  | ---- |        |(exit)|
;                 .retval |_----_|        |------|
;
;*******************************************************************************

            .cdecls C,"msp430.h"            ; MSP430
            .cdecls C,"pthreads.h"          ; threads header

            .def    TA_isr
            .ref    ctid
            .ref    tcbs

tcb_thread  .equ    (tcbs + 0)
tcb_stack   .equ    (tcbs + 2)
tcb_block   .equ    (tcbs + 4)

; Code Section ------------------------------------------------------------

            .text                           ; beginning of executable code
; Timer A ISR -------------------------------------------------------------
TA_isr:     bic.w   #TAIFG|TAIE,&TACTL      ; acknowledge & disable TA interrupt
;
; >>>>>> 1. Save current context on stack
; >>>>>> 2. Save SP in task control block
; >>>>>> 3. Find next non-blocked thread tcb
; >>>>>> 4. If all threads blocked, enable interrupts in LPM0
; >>>>>> 5. Set new SP
; >>>>>> 6. Load context from stack
;
            bis.w    #TAIE,&TACTL           ; enable TA interrupts
            reti


; Interrupt Vector --------------------------------------------------------
            .sect   ".int08"                ; timer A section
            .word   TA_isr                  ; timer A isr
            .end
