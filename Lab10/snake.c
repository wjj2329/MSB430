//******************************************************************************
//  Lab 09b - Snake (08/07/2015)
//
//  Description:
//
//	"Write a snake game program in C that scores points by eating randomly
//	placed food on the display. There are four levels of play, each with
//	increasing difficulty. The game ends when the snake runs into an obstacle,
//	itself, or time expires. The direction of the snake�s head is turned to
//	horizontal and vertical paths using the push buttons and the body of the
//	snake follows in the head's path. The snake is always moving and the
//	snake moves faster at each level. The tail grows by one segment every
//	time the snake eats a food."
//
//
//  Author:			Paul Roper, Brigham Young University
//  Revisions:		1.0		11/25/2012		RBX430-1
//					1.1		04/12/2013		START_LEVEL
//					1.2		04/06/2015		functions moved to snakelib
//					1.3		08/07/2015		else if (sys_event)
//
//  Built with Code Composer Studio Version: 5.2.0.00090
//******************************************************************************
//******************************************************************************
//
#include "msp430.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "snake.h"
#include "snakelib.h"

volatile uint16 sys_event;					// pending events
extern volatile enum MODE game_mode;		// 0=idle, 1=play, 2=next

//------------------------------------------------------------------------------
//-- main ----------------------------------------------------------------------
//
void main(void)
{
 	srand((unsigned) time(NULL));
	BCSCTL1 = CALBC1_12MHZ;       // Set range
	DCOCTL = CALDCO_12MHZ;        // Set DCO step and modulation
	ERROR2(_SYS, RBX430_init(CLOCK));		// init RBX430-1 board
	ERROR2(_SYS, lcd_init());				// init LCD & interrupts
	ERROR2(_SYS, port1_init());				// init port 1 (switches)
	ERROR2(_SYS, timerB_init());			// init timer B (sound)
	ERROR2(_SYS, watchdog_init(WDT_CTL));	// init watchdog timer
	game_mode = IDLE;						// idle mode
	sys_event = NEW_GAME;					// new game (in idle mode)

	//--------------------------------------------------------------------------
	//	event service routine loop
	//
	while (1)
	{
		// disable interrupts before check sys_event
		_disable_interrupts();

		// if there's something pending, enable interrupts before servicing
		if (sys_event) _enable_interrupt();

		// otherwise, enable interrupts and goto sleep (LPM0)
		else __bis_SR_register(LPM0_bits + GIE);

		//----------------------------------------------------------------------
		//	I'm AWAKE!!!  There must be something to do
		//----------------------------------------------------------------------

		if (sys_event & MOVE_SNAKE)			// move snake event
		{
			sys_event &= ~MOVE_SNAKE;		// clear move event
			MOVE_SNAKE_event();				// move snake
		}
		else if (sys_event & SWITCH_1)		// switch #1 event
		{
			sys_event &= ~SWITCH_1;			// clear switch #1 event
			SWITCH_1_event();				// process switch #1 event
		}
		else if(sys_event & SWITCH_2)
		{
			sys_event &= ~SWITCH_2;
			SWITCH_2_event();
		}
		else if(sys_event & SWITCH_3)
		{
			sys_event &= ~SWITCH_3;
			SWITCH_3_event();
		}
		else if(sys_event & SWITCH_4)
		{
			sys_event &= ~SWITCH_4;
			SWITCH_4_event();
		}

		else if (sys_event & START_LEVEL)	// start level event
		{
			sys_event &= ~START_LEVEL;		// clear start level event
			START_LEVEL_event();			// start game
		}
		else if (sys_event & LCD_UPDATE)	// LCD event
		{
			sys_event &= ~LCD_UPDATE;		// clear LCD event
			LCD_UPDATE_event();				// update LCD
		}
		else if (sys_event & NEW_GAME)		// new game event
		{
			sys_event &= ~NEW_GAME;			// clear new game event
			NEW_GAME_event();				// new game
		}

		else if(sys_event & END_GAME)
		{
			sys_event &= ~END_GAME;
			END_GAME_event();
		}
		else if (sys_event)					// ????
		{
			ERROR2(_USER, _ERR_EVENT);		// unrecognized event
		}
	}
} // end main
