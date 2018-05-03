/*
 * morse2.h
 *
 *  Created on: Jul 16, 2016
 *      Author: wjj2329
 */

#ifndef MORSE2_H_
#define MORSE2_H_


#define myCLOCK    1200000                 // 1.2 Mhz clock
#define WDT_CTL      WDT_MDLY_0_5           // ; WD: Timer, SMCLK, 0.5 ms
#define WDT_CPI        600//                     ; WDT Clocks Per Interrupt (@1 Mhz)
#define WDT_IPS    (myCLOCK/WDT_CPI)//        ; WDT Interrupts Per Second
#define STACK       1536 //0x0600       ; top of stack

//; Morse Code equates ----------------------------------------------------------
#define END   0    //  .equ
#define DOT   1   //.equ
#define DASH  2 //.equ
#define ELEMENT  (WDT_IPS*240/1000)   // .equ           ; (WDT_IPS * 6 / WPM) / 5
#define DEBOUNCE 10  //.equ


#endif /* MORSE2_H_ */
