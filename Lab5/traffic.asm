;*******************************************************************************
;   Lab 5b - traffic.asm
;
;   Description:  1. Turn the large green LED and small red LED on and
;                    delay 20 seconds while checking for orange LED.
;                    (If orange LED is on and 10 seconds has expired, immediately
;                    skip to next step.)
;                 2. Turn large green LED off and yellow LED on for 5 seconds.
;                 3. Turn yellow LED off and large red LED on.
;                 4. If orange LED is on, turn small red LED off and small green
;                    LED on.  After 5 seconds, toggle small green LED on and off
;                    for 6 seconds at 1 second intervals.  Finish by toggling
;                    small green LED on and off for 4 seconds at 1/5 second
;                    intervals.
;                    Else, turn large red LED on for 5 seconds.
;                 5. Repeat the stoplight cycle.
;
;   I  William Jones certify this to be my source code and not obtained from any student, past
;   or current.
;
;*******************************************************************************
;                            MSP430F2274
;                  .-----------------------------.
;            SW1-->|P1.0^                    P2.0|<->LCD_DB0
;            SW2-->|P1.1^                    P2.1|<->LCD_DB1
;            SW3-->|P1.2^                    P2.2|<->LCD_DB2
;            SW4-->|P1.3^                    P2.3|<->LCD_DB3
;       ADXL_INT-->|P1.4                     P2.4|<->LCD_DB4
;        AUX INT-->|P1.5                     P2.5|<->LCD_DB5
;        SERVO_1<--|P1.6 (TA1)               P2.6|<->LCD_DB6
;        SERVO_2<--|P1.7 (TA2)               P2.7|<->LCD_DB7
;                  |                             |
;         LCD_A0<--|P3.0                     P4.0|-->LED_1 (Green)
;        i2c_SDA<->|P3.1 (UCB0SDA)     (TB1) P4.1|-->LED_2 (Orange) / SERVO_3
;        i2c_SCL<--|P3.2 (UCB0SCL)     (TB2) P4.2|-->LED_3 (Yellow) / SERVO_4
;         LCD_RW<--|P3.3                     P4.3|-->LED_4 (Red)
;   TX/LED_5 (G)<--|P3.4 (UCA0TXD)     (TB1) P4.4|-->LCD_BL
;             RX-->|P3.5 (UCA0RXD)     (TB2) P4.5|-->SPEAKER
;           RPOT-->|P3.6 (A6)          (A15) P4.6|-->LED 6 (R)
;           LPOT-->|P3.7 (A7)                P4.7|-->LCD_E
;                  '-----------------------------'
;
;*******************************************************************************
;*******************************************************************************
            .cdecls  C,LIST,"msp430.h"      ; MSP430
            .def RESET

        .asg   "bis.b #0x08,&P4OUT",RED_ON
        .asg   "bic.b #0x08,&P4OUT",RED_OFF
        .asg   "xor.b #0x08,&P4OUT",RED_TOGGLE
        .asg   "bit.b #0x08,&P4OUT",RED_TEST

        .asg   "bis.b #0x04,&P4OUT",YELLOW_ON
        .asg   "bic.b #0x04,&P4OUT",YELLOW_OFF
        .asg   "xor.b #0x04,&P4OUT",YELLOW_TOGGLE
        .asg   "bit.b #0x04,&P4OUT",YELLOW_TEST

        .asg   "bis.b #0x02,&P4OUT",ORANGE_ON
        .asg   "bic.b #0x02,&P4OUT",ORANGE_OFF
        .asg   "xor.b #0x02,&P4OUT",ORANGE_TOGGLE
        .asg   "bit.b #0x02,&P4OUT",ORANGE_TEST
        .asg   "bit.b #0x02,&P4OUT",ORANGE_TEST

        .asg   "bis.b #0x01,&P4OUT",GREEN_ON
        .asg   "bic.b #0x01,&P4OUT",GREEN_OFF
        .asg   "xor.b #0x01,&P4OUT",GREEN_TOGGLE
        .asg   "bit.b #0x01,&P4OUT",GREEN_TEST

        .asg   "bis.b #0x40,&P4OUT",RED2_ON
        .asg   "bic.b #0x40,&P4OUT",RED2_OFF
        .asg   "xor.b #0x40,&P4OUT",RED2_TOGGLE
        .asg   "bit.b #0x40,&P4OUT",RED2_TEST

        .asg   "bis.b #0x10,&P3OUT",GREEN2_ON
        .asg   "bic.b #0x10,&P3OUT",GREEN2_OFF
        .asg   "xor.b #0x10,&P3OUT",GREEN2_TOGGLE
        .asg   "bit.b #0x10,&P3OUT",GREEN2_TEST


COUNT       .equ    0
COUNT2      .equ    28
COUNT3      .equ    112
COUNT4      .equ    5
COUNT5      .equ    1


;------------------------------------------------------------------------------
            .text                           ; beginning of executable code
            .retain                         ; Override ELF conditional linking
;-------------------------------------------------------------------------------
RESET:

		  ORANGE_OFF
       	  YELLOW_OFF
       	  GREEN_OFF
       	  GREEN2_OFF
       	  RED_OFF
       	  RED2_ON
		  mov.w   #__STACK_END,SP         ; init stack pointer
          mov.w   #WDTPW+WDTHOLD,&WDTCTL  ; stop WDT
          bis.b   #0x08,&P4DIR            ; set P4.3 as output
          bis.b  #0x4f,&P4DIR             ; set P4.0-3,6 as output
          bis.b  #0x10,&P3DIR             ; set P3.4 as output


       	  ; switch setup:
          bic.b  #0x0f,&P1DIR          ; set P1.0 as input
          bis.b  #0x0f,&P1OUT          ; select pull-up
          bis.b  #0x0f,&P1REN          ; enable pull-up on P1.0

          bic.b  #0x0f,&P1SEL          ; select GPIO
          bic.b  #0x0f,&P1DIR          ; configure P1.0 as inputs
          bis.b  #0x0f,&P1OUT          ; use pull-up
          bis.b  #0x0f,&P1REN          ; enable pull-up
          bis.b  #0x0f,&P1IES          ; trigger on high to low transition
          bis.b  #0x0f,&P1IE           ; P1.0 interrupt enabled
          bic.b  #0x0f,&P1IFG          ; P1.0 IFG cleared

          bis.w  #GIE,SR               ; enable general interrupts


mainloop:

          	GREEN_ON						;green light on for twenty seconds
 		    mov.w   #COUNT,r15              ; use R15 as delay counter
            mov.w   #COUNT3,r14
 		    call #delayloop
            GREEN_OFF
            GREEN2_OFF
			YELLOW_ON						;Yellow on for five seconds
			mov.w   #COUNT,r15              ; use R15 as delay counter
            mov.w   #COUNT2,r14
            call #delayloop
		    mov.w   #COUNT, r15
            YELLOW_OFF

            RED_ON						;Red on for five seconds
            mov.w   #COUNT,r15              ; use R15 as delay counter
            mov.w   #COUNT2,r14
			ORANGE_TEST   ;if off it will go where the jeq wants it to go.
			jne pedwalk
            call #delayloop
            mov.w   #COUNT,r15
            RED_OFF
            jmp  mainloop



; delayloop suproutine for 5 seconds


delayloop:
			push r14
			push r15
delayloop1:
			sub.w   #1,r15                  ;    delay over? 1
            jne     delayloop1
            mov.w   #COUNT,r15
delayloop2:
			sub.w   #1,r14
            jne delayloop1
            pop r14
            pop r15
            ret

P1_ISR:
          bic.b  #0x0f,&P1IFG          ; clear P1.0 Interrupt Flag
          ;bis.b  #0x01,&P4OUT          ; turn on green LED
xor.b   #0x20,&P4OUT            ; y, use 50% PWM
          reti


toggle_slow: push r14
			 push r15
			 mov.w   #COUNT,r15              ; use R15 as delay counter
             mov.w   #COUNT4,r14
             call #delayloop
             GREEN2_TOGGLE
             pop r14
             pop r15
             ret


toggle_fast: push r14
			 push r15
			 mov.w   #COUNT,r15              ; use R15 as delay counter
           	 mov.w   #COUNT5,r14
             call #delayloop
             GREEN2_TOGGLE
             pop r14
             pop r15
             ret

pedwalk:
			RED2_OFF
			GREEN2_ON
            mov.w   #COUNT,r15              ; use R15 as delay counter
            mov.w   #COUNT2,r14
            call #delayloop				;first five seconds

          	call #toggle_slow
          	call #toggle_slow
          	call #toggle_slow
          	call #toggle_slow
          	call #toggle_slow
          	call #toggle_slow
          	call #toggle_slow
          	call #toggle_slow

            ;second one
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            call #toggle_fast
            RED2_ON
            ORANGE_OFF
			ret

          .sect  ".int02"              ; P1 interrupt vector
          .word  P1_ISR
;-------------------------------------------------------------------------------
;           Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect 	.stack

;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .word   RESET                   ; start address
            .end
