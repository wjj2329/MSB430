#include "msp430.h"          // .cdecls C,"msp430.h"
#include "morse2.h"
#include <stdlib.h>
#include <ctype.h>
//#include "lab01.h"
//      Author:  William Jones  I DID THIS CODE!!!!!!!!!!!!!!

#include <stdio.h>
#include <stdint.h>


/*
 * main.c
 */
extern int debounce_count;


extern char* letters[];
extern char* numbers[];
extern void doSPACE();
extern void doDASH();
extern void doDOT();
extern void doEND();
extern void setupstuff();
extern char beep_cnt;					//	            bis.b	#0x10,&P3DIR			; set P3.4 as output (green LED)
extern char  delay_cnt;
extern void TURNONANDOFF();



void docommands(char* commands)
{
	while(*commands!=END)
	{
		if(*commands==DASH)//cmp.b	#DASH,r6

		{
		doDASH();//call do dash
		}
		if(*commands==DOT)//cmp.b	#DOT,r6
		{
		doDOT();//call do dot
		}
		commands++;//mov.b	@r5+,r6
	}
	doEND();//call do end
}
int main(void)
{
					//    .def  main_asm
					//	; start main function vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 WDTCTL=WDT_CTL;		//	main_asm:   mov.w   #WDT_CTL,&WDTCTL        ; set WD timer interval

 IE1 = WDTIE; 	 	//	            mov.b   #WDTIE,&IE1             ; enable WDT interrupt
 P4DIR=0x20;				//	            bis.b   #0x20,&P4DIR            ; set P4.5 as output (speaker)
 P4DIR|=0x40;					//	            bis.b	#0x40,&P4DIR			; set P4.6 as output (red LED)
 P3DIR=0x10;
				//	            clr.w   &beep_cnt               ; clear counters
					//	            clr.w   &delay_cnt
					//
P1SEL&=~0x0f;					//				bic.b  #0x01,&P1SEL          ; select GPIO
P1DIR&=~0x0f;					//				bic.b  #0x01,&P1DIR          ; configure P1.0 as Inputs
P1OUT|=0x0f;					//				bis.b  #0x01,&P1OUT          ; use pull-ups
P1REN|=0x0f;				//					bis.b  #0x01,&P1REN          ; enable pull-ups
P1IES|=0x0f;					//				bis.b  #0x01,&P1IES          ; trigger on high to low transition
P1IE|=0x0f;					//				bis.b  #0x01,&P1IE           ; P1.0 interrupt enabld
P1IFG&=0x0f;					//				bic.b  #0x01,&P1IFG          ; P1.0 IFG cleared
					//				"bis.b	#0x01,r9"  ; sound on
//
//	            mov.w	#WDT_IPS,green_count				; use  as second counter for green LED
//	            jmp	firstloop
__enable_interrupt();// bis.w   #GIE,SR                 ; enable interrupts
setupstuff();


while(1)
{
	char message[18];
	strcpy(message,"HELLO CS 124 WORLD");//.string "HELLO CS 124 WORLD"   	; message
	unsigned int j=0;
	for(j=0; j<18;  j++)
	{
		if(isspace(message[j]))//;if greater then for number; cmp	#0x30,r5
		{
			doSPACE();	//call #doSPACE
			//jmp loopMessage
		}
		else
		if(isalpha(message[j]))//cmp #0x41,r5   ;if great then asci value for A
		{
			char *commands=letters[message[j]-'A'];//sub.w	#'A',r5add.w	r5,r5mov.w	letters(r5),r5
			docommands(commands); //call do Morse
		}
		else
		if(isdigit(message[j]))//cmp #0x30,r5
		{
			char *commands=numbers[message[j]-'0'];//sub.w	#'0',r5add.w	r5,r5mov.w	numbers(r5),r5
			docommands(commands);//call do Morse
		}
	}//jmp letters
}//jpm firstloop
    //return 0;
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1_ISR(void)
{                         // P1_ISR:
   P1IFG &= ~0x0f;        //   bic.b #0x0f,&P1IFG  ; acknowledge
   debounce_count = DEBOUNCE; //   mov.w #DB_CNT,&dcnt ; start debounce
   TURNONANDOFF();
   return 0;                //   reti
}

