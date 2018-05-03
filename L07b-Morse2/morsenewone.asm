	.title	"morse.asm"
;*******************************************************************************
;     Project:  morse.asm
;      Author:  William Jones  I DID THIS CODE!!!!!!!!!!!!!!
;
; Description:  Outputs a message in Morse Code using a LED and a transducer
;               (speaker).  The watchdog is configured as an interval timer.
;               The watchdog interrupt service routine (ISR) toggles the green
;               LED every second and pulse width modulates (PWM) the speaker
;               such that a tone is produced.
;
;	Morse code is composed of dashes and dots:
;
;        1. A dot is equal to an element of time.
;        2. One dash is equal to three dots.
;        3. The space between parts of the letter is equal to one dot.
;        4. The space between two letters is equal to three dots.
;        5. The space between two words is equal to seven dots.
;
;    5 WPM = 60 sec / (5 * 50) elements = 240 milliseconds per element.
;    element = (WDT_IPS * 6 / WPM) / 5
;
;******************************************************************************

		.cdecls C,"msp430.h"
		.cdecls C,"morse2.h"

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

		.asg	"bis.b	#0x01,r9",SOUND_ON
		.asg	"bic.b	#0x01,r9",SOUND_OFF
		.asg	"xor.b	#0x01,r9",SOUND_TOGGLE
		.asg	"bit.b	#0x01,r9",SOUND_TEST

; System equates --------------------------------------------------------------


; External references ---------------------------------------------------------
            .ref    numbers                 ; codes for 0-9
            .ref    letters                 ; codes for A-Z

; Global variables ------------------------------------------------------------

            .bss    beep_cnt,2              ; beeper flag
            .bss    delay_cnt,2             ; delay flag
            .bss	debounce_count,2		; debounce counter
			.bss	green_count,2
			.def   debounce_count

			.def   doSPACE
			.def   doDOT
			.def   doDASH
			.def   doEND
			.def   setupstuff
			.def   TURNONANDOFF

; Program section -------------------------------------------------------------
            .text                           ; program section
message:    .string "HELLO CS 124 WORLD"   	; message
            .byte   0
            .align  2                       ; align on word boundary



firstloop:
			mov.w	#message,r14

loopMessage:
			mov.b	@r14+,r5
			cmp #0x41,r5   ;if great then asci value for A
			jge	isLetter
			cmp	#0x30,r5   ;if greater then for number
			jge	isNumber
			cmp #0x20,r5   ;if equal for space
			jz	isSpace
			jmp firstloop

isLetter:
			sub.w	#'A',r5
			add.w	r5,r5
			mov.w	letters(r5),r5
			jmp morseConvert
			;mov.b  #address, r4

isNumber:
			sub.w	#'0',r5
			add.w	r5,r5
			mov.w	numbers(r5),r5
			jmp morseConvert
isSpace:
			call #doSPACE
			jmp loopMessage
morseConvert:
			mov.b	@r5+,r6
			cmp.b	#DOT,r6
			jne	dash
			call	#doDOT
			jmp	morseConvert
dash:
			cmp.b	#DASH,r6
			jne	ending
			call	#doDASH
			jmp morseConvert
ending:
			call #doEND
            ret


doDASH:
		push r15
        mov.w   #ELEMENT*3,r15          ; output DASH
        call    #beep
        mov.w   #ELEMENT,r15            ; delay 1 element
		call    #delay
		pop r15
		ret

doDOT:
		push r15
		mov.w	#ELEMENT,r15
		call	#beep
		mov.w	#ELEMENT,r15		; delay 1 element
		call	#delay
		pop r15
		ret

doEND:
		push r15
        mov.w   #ELEMENT*3,r15
		call    #delay
		pop r15
		ret

doSPACE:
		push r15
		mov.w	#ELEMENT*7,r15
		call	#delay
		pop r15
		ret

; beep (r15) ticks subroutine -------------------------------------------------
beep:       mov.w   r15,&beep_cnt           ; start beep
beep02:     tst.w   &beep_cnt               ; beep finished?
            jne   beep02                  ; n
            ret                             ; y

setupstuff:	SOUND_ON
	        ; bis.w   #GIE,SR                 ; enable interrupts
	         mov.w	#WDT_IPS,green_count				; use  as second counter for green LED
	         ret

; delay (r15) ticks subroutine ------------------------------------------------
delay:      mov.w   r15,&delay_cnt          ; start delay

delay02:    tst.w   &delay_cnt              ; delay done?
              jne   delay02                 ; n
            ret                             ; y

; Switch 1 ISR for any buttons pressed
;SW1_ISR:
;			bic.b	#0x0f,&P1IFG			; reset interrupt flag
;			mov.w	#DEBOUNCE,&debounce_count	; debounce
 ;           xor.b   #0x20,&P4DIR             ; y, use 50% PWM
	;		reti

; Watchdog Timer ISR ----------------------------------------------------------
WDT_ISR:
			tst.w   &beep_cnt               ; beep on?
            jeq     WDT_02                  ; n
            RED2_ON
            dec.w   &beep_cnt               ; y, decrement count
            xor.b   #0x20,&P4OUT            ; beep using 50% PWM

WDT_02:
			tst.w   &delay_cnt              ; delay?
            jeq   Deb                ; n
            dec.w   &delay_cnt              ; y, decrement count
            RED2_OFF

Deb:
Checklight:
			dec.w	green_count
			jeq WDT_03

           tst.w   debounce_count      ; debouncing?
             jeq   WDT_30                ; n

; debounce switches & process
           dec.w   debounce_count       ; y, decrement count, done?
           jne   WDT_30                ; n
           push    r15                   ; y
           mov.b   &P1IN,r15             ; read switches
           and.b   #0x0f,r15
           xor.b   #0x0f,r15             ; any switches?
           jeq   WDT_20                ; n
		   xor.b   #0x20,&P4OUT



WDT_20:    pop     r15

WDT_30:    reti

WDT_03:
		GREEN2_TOGGLE
		mov	#WDT_IPS,green_count
		jmp WDT_ISR

TURNONANDOFF:
			xor.b   #0x20,&P4DIR             ; y, use 50% PWM
			ret
; Interrupt Vectors -----------------------------------------------------------
            .sect   ".int10"                ; Watchdog Vector
            .word   WDT_ISR                 ; Watchdog ISR

            ;.sect	".int02"
            ;.word	SW1_ISR



            .end
