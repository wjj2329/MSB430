//******************************************************************************
//	snake_events.c  (03/29/2016)
//
//  Author:			Paul Roper, Brigham Young University
//  Revisions:		1.0		11/25/2012	RBX430-1
//					1.1		03/24/2016	add_head() & delete_tail loosely coupled
//							03/29/2016	lcd draw head moved to snakelib.c
//
//******************************************************************************
//
#include "msp430.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "snake.h"
#include "snakelib.h"
#include <stdbool.h>

extern volatile uint16 sys_event;			// pending events

volatile enum MODE game_mode;				// 0=idle, 1=play, 2=next
volatile uint16 score;						// current score
volatile uint16 seconds;					// time
volatile uint16 move_cnt;					// snake speed

volatile uint8 level;						// current level (1-4)
uint8 direction;							// current move direction
uint8 head;									// head index into snake array
uint8 tail;									// tail index into snake array
SNAKE snake[MAX_SNAKE];		// snake segments


extern const uint16 snake_text_image[];		// snake text image
extern const uint16 snake1_image[];			// snake image

POINT food[10];
int   position=0;
int   timeforlevel;
bool  create=true;
POINT rocks[4];
//------------------------------------------------------------------------------
//-- move snake event ----------------------------------------------------------
//
void generaterocks()
{
   int i;
   for(i=0; i<4; i++)
   {
	   POINT adding;
	   adding.x=rand()%23;
	   adding.y=rand()%23;
	   rocks[i]=adding;
	   lcd_square(COL(adding.x), ROW(adding.y), 2, 1+FILL);
   }
}

bool checkforrockcollision(int x, int y)
{
	int i=0;
	if(level==1)
	{
		return false;
	}
	for(i=0; i<4; i++)
	{
		if(rocks[i].x==x&&rocks[i].y==y)
		{
			return true;
		}
	}
	return false;
}


void victory()
{
	lcd_clear();
	lcd_backlight(1);
	lcd_wordImage(snake1_image, (159-60)/2, 60, 1);
	imperial_march();
	game_mode=IDLE;
	    while(1)
	    {
	    	if (sys_event & SWITCH_1)
	    	{
	    		seconds=0;//so yea I might need to delete all the variables being used not sure how though.
	    		position=0;
	    		score=0;
	    		return;
	    	}
	    }
}

void cleararray()
{
	int i;
	for(i=0; i<10; i++)
	{
		food[i].x=30;
		food[i].y=30;
	}
	for(i=0; i<4; i++)
	{
		rocks[i].x=30;
		rocks[i].y=30;
	}
}
bool stopduplicates(int col, int row)
{
	int i;
	for(i=0; i<10; i++)
	{
		if(food[i].x==col&&food[i].y==row)
		{
			return true;
		}
	}
	return false;
}
void generateicons(int result, int col, int row)
{
	while(stopduplicates(col, row))
	{
	   col=rand()%23;
	   row=rand()%23;
	}
	if(create)
	{
	    POINT location;
	    location.x=col;//could make this more memory friendly but see what happens for now.
	    location.y=row;
		food[position]=location;
		position++;
			switch(result)
			{
			case 0:
			{
				lcd_diamond(COL(col), ROW(row), 2, 1);
				return;
			}
			case 1:
			{
				lcd_star(COL(col), ROW(row), 2, 1);
				return;
			}
			case 2:
			{
				lcd_circle(COL(col), ROW(row), 2, 1);
				return;
			}
			case 3:
			{
				lcd_square(COL(col), ROW(row), 2, 1);
				return;
			}
			case 4:
			{
				lcd_triangle(COL(col), ROW(row), 2, 1);
				return;
			}
			}
	}
	else
	{
	     //else just make a dummy food vairable in the bottom corner that can never be reached.
		food[position].x=30;
		food[position].y=30;
	}
}
bool checkforselfcollosion()
{
	int i;
	for(i=tail; i<head; i++)
	{
		if(snake[head].xy == snake[i].xy)
		{
			return true;
		}
	}
	return false;
}

bool checkforintersection(int x,  int y)
{
	int i;
	for(i=0; i<10; i++)
	{
		if(food[i].x==x&&food[i].y==y)
		{
			//generate new food
			position=i;
			generateicons(rand()%5, rand()%23, rand()%23);
			beep(); blink();
			score+=level;//increase snake size
			return true;
			//delete food from map.
		}
	}
	return false;
}
void MOVE_SNAKE_event(void)
{
	if (level > 0)
	{
		add_head(&head, &direction);		// add head
		bool delete=true;
		if(checkforselfcollosion())
		{
			 END_GAME_event();
		}
		if(checkforrockcollision(snake[head].point.x,  snake[head].point.y))
		{
			 END_GAME_event();
		}
		if(checkforintersection(snake[head].point.x,  snake[head].point.y))
		 {
			 delete=false;
		 }
		if(score==12&&level==1)
		 {
			timeforlevel=30;
			move_cnt = WDT_MOVE2;				// level 2, speed 2
			NEXT_LEVEL_event();
		 }
		 if(score==32&&level==2)
		 {
			 timeforlevel=45;
			 move_cnt = WDT_MOVE3;				// level 3, speed 3
			 NEXT_LEVEL_event();
		 }
		 if(score==62&&level==3)
		 {
			 timeforlevel=60;					//level 4, speed 4
			 move_cnt=WDT_MOVE4;
			 NEXT_LEVEL_event();
		 }
		 if(score==102&&level==4)
		 {
			 victory();
		 }
		 if(seconds>=timeforlevel)
		 {
			 END_GAME_event();
		 }
		if(delete)
		{
		delete_tail(&tail);
		}// delete tail
	}
	return;
} // end MOVE_SNAKE_event



//------------------------------------------------------------------------------
//-- new game event ------------------------------------------------------------
//
void NEW_GAME_event(void)
{
	lcd_clear();							// clear lcd
	lcd_backlight(1);						// turn on backlight
	int s=0;

	switch (game_mode)
	{
	case IDLE:
	{
		lcd_cursor(0, 3);
		lcd_printf("%s","Up");
		lcd_cursor(40, 3);
		lcd_printf("%s","Down");
		lcd_cursor(90, 3);
		lcd_printf("%s","Left");
		lcd_cursor(130, 3);
		lcd_printf("%s","Right");
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		// ***demo only***
		//lcd_wordImage(snake1_image, (159-60)/2, 60, 1);
		//lcd_wordImage(snake_text_image, (159-111)/2, 20, 1);
		//lcd_diamond(COL(16), ROW(20), 2, 1);
		//lcd_star(COL(17), ROW(20), 2, 1);
		//lcd_circle(COL(18), ROW(20), 2, 1);
		//lcd_square(COL(19), ROW(20), 2, 1);
		//lcd_triangle(COL(23), ROW(23), 2, 1);
		position=0;
		create=true;
		for(s=0; s<10; s++)
		{
			int result=rand()%5;
			int col=rand()%23;
			int row=rand()%23;
			generateicons(result,col,row);

		}
		timeforlevel=30;
		score = 2;
		move_cnt = WDT_MOVE1;				// level 2, speed 2
		level = 1;

		break;
	}
		// ***demo only***
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	default:
		break;
	}

	new_snake(score, RIGHT);
	sys_event = START_LEVEL;
	return;
} // end NEW_GAME_event


//------------------------------------------------------------------------------
//-- start level event -----------------------------------------------------------
//
void START_LEVEL_event(void)
{
	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	//	Add code here to setup playing board for next level
	//	Draw snake, foods, reset timer, set level, move_cnt etc
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	game_mode = PLAY;						// start level
	seconds = 0;							// restart timer
	return;
} // end START_LEVEL_event


//------------------------------------------------------------------------------
//-- next level event -----------------------------------------------------------
//
void NEXT_LEVEL_event(void)
{
	int i;
	level++;
	cleararray();
	lcd_clear();							// clear lcd
	blink();
	charge();
	while(1)
	{
	  if (sys_event & SWITCH_1)
	    {
	    break;
     	}
	}
	generaterocks();
	if(level==2)
	{
		position=0;
		for(i=0; i<10; i++)
		{
			generateicons(rand()%5, rand()%23, rand()%23);
		}
		create=false;
	}
	if(level==3)
	{
		create=true;
		position=0;
		generateicons(rand()%5, rand()%23, rand()%23);
	}
	if(level==4)
	{
		generateicons(rand()%5, rand()%23, rand()%23);
	}
	seconds=0;//so yea I might need to delete all the variables being used not sure how though.
	position=0;
	lcd_cursor(0, 3);
	lcd_printf("%s","Up");
	lcd_cursor(40, 3);
	lcd_printf("%s","Down");
	lcd_cursor(90, 3);
	lcd_printf("%s","Left");
	lcd_cursor(130, 3);
	lcd_printf("%s","Right");
	return;
} // end NEXT_LEVEL_event


//------------------------------------------------------------------------------
//-- end game event -------------------------------------------------------------
//
void END_GAME_event(void)
{
	lcd_clear();							// clear lcd
	blink();
	rasberry();
    game_mode=IDLE;
    lcd_cursor((score < 100) ? 35 : 145, 144);
    lcd_printf("\b\t%s", "SCORE ");
    lcd_cursor((score < 100) ? 105 : 145, 144);
    lcd_printf("\b\t%d", score-2);

    while(1)
    {
    	if (sys_event & SWITCH_1)
    	{
    		seconds=0;//so yea I might need to delete all the variables being used not sure how though.
    		position=0;
    		return;
    	}
    }
} // end END_GAME_event


//------------------------------------------------------------------------------
//-- switch #1 event -----------------------------------------------------------
//
void SWITCH_1_event(void)
{
	switch (game_mode)
	{
		case IDLE:
			sys_event |= NEW_GAME;
			break;

		case PLAY:
			switch (direction)
			{
				case UP:
					//if ((level != 2) || (snake[head].point.x < X_MAX))
					{
					direction = RIGHT;
					sys_event |= MOVE_SNAKE;
					}
					break;
				case DOWN:
					//if ((level != 2) || (snake[head].point.x < X_MAX))
					{
						direction = RIGHT;
						sys_event |= MOVE_SNAKE;
					}
					break;
				case RIGHT:
					break;// ignore if going right
				case LEFT:					// ignore if going left
					break;
			}
			break;

		case NEXT:
			sys_event |= NEW_GAME;
			break;
	}
	return;
} // end SWITCH_1_event


//------------------------------------------------------------------------------
//-- switch #2 event -----------------------------------------------------------
//
void SWITCH_2_event(void)
{
	switch (direction)
	{
		case UP:
				//if ((level != 2) || (snake[head].point.x < X_MAX))
				{
					direction = LEFT;
					sys_event |= MOVE_SNAKE;
				}
				break;
		case DOWN:
				//if ((level != 2) || (snake[head].point.x < X_MAX))
				{
					direction = LEFT;
					sys_event |= MOVE_SNAKE;
				}
				break;
		case RIGHT:
				break;// ignore if going right
		case LEFT:					// ignore if going left
				break;

		}
		return;
} // end SWITCH_2_event


//------------------------------------------------------------------------------
//-- switch #3 event -----------------------------------------------------------
//
void SWITCH_3_event(void)
{
	switch (direction)
	{
		case UP:
				break;
		case DOWN:
				break;
		case RIGHT:
		{
		direction = DOWN;
		sys_event |= MOVE_SNAKE;
		}
				break;
		case LEFT:
		{
		direction = DOWN;
		sys_event |= MOVE_SNAKE;
		}
				break;
		}
		return;
} // end SWITCH_3_event


//------------------------------------------------------------------------------
//-- switch #4 event -----------------------------------------------------------
//
void SWITCH_4_event(void)
{
	switch (direction)
		{
			case UP:
					break;
			case DOWN:
					break;
			case RIGHT:
			direction = UP;
			sys_event |= MOVE_SNAKE;
			break;

			case LEFT:
			direction = UP;
			sys_event |= MOVE_SNAKE;
			break;
		}
} // end SWITCH_4_event


//------------------------------------------------------------------------------
//-- update LCD event -----------------------------------------------------------
//
void LCD_UPDATE_event(void)
{
	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	//	Add code here to handle LCD_UPDATE event
	lcd_cursor((score < 100) ? 75 : 145, 144);
	lcd_printf("\b\t%d", timeforlevel-seconds);
	lcd_cursor((score < 100) ? 35 : 145, 144);
	lcd_printf("\b\t%d", score-2);


	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
} // end LCD_UPDATE_event
