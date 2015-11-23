#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */

#define NO_TARGETS 6
#define NO_PLAYER 0
#define PLAYER_A  1
#define PLAYER_B  2

void initializations(void);
  void targetInit(Target *myTarget, unsigned char targetMaxScore);
void targetHit(int targetNumber);
void activateTarget(int targetNumber, unsigned char player);
void deactivateTarget(int targetNumber);
void oneSecondOver(void);
  void setPlayer(int targetNumber);
// following are LCD functions
void print_c(char);
void pmsglcd(char str[]);
  void shiftout(char);
  void lcdwait(void);
  void send_byte(char);
  void send_i(char);
  void chgline(char);
// for RTI initializations
void startGame(void);
  void stopGame(void);

unsigned char gameRunning_flag; // true -> game is running
unsigned int player_a_score;
unsigned int player_b_score; 
unsigned int player_a_allocated;
unsigned int player_b_allocated; 
Target target[NO_TARGETS];

unsigned int gameTime;

void main(void) 
{
  DisableInterrupts
  initializations();
	EnableInterrupts;
  for(;;) 
  {
    _FEED_COP(); // feeds the dog 
                 // TODO (for watchdog timer @Kanishk)                                           

    if(gameRunning_flag != 0) 
    {
      
    }
    
  }
}

void initializations() 
{
  /* Set the PLL speed (bus clock = 24 MHz) */
  CLKSEL = CLKSEL & 0x80; // disengage PLL from system
  PLLCTL = PLLCTL | 0x40; // turn on PLL
  SYNR = 0x02;            // set PLL multiplier
  REFDV = 0;              // set PLL divider
  while (!(CRGFLG & 0x08)){  }
  CLKSEL = CLKSEL | 0x80; // engage PLL
  
  
  /*
    @Kanishk : Add code here
  */
  
  // targetInit(Target *myTarget, unsigned char targetMaxScore)  
  targetInit( &(target[0]), 10 ); // target 1
  targetInit( &(target[1]),  9 ); // target 2
  targetInit( &(target[2]),  7 ); // target 3
  targetInit( &(target[3]),  5 ); // target 4
  targetInit( &(target[4]),  3 ); // target 5
  targetInit( &(target[5]),  3 ); // target 6
  
  // Variable initializations
  gameRunning_flag = 0;
}


/*
***********************************************************************
  deactivateTarget: To pick a target and set a player to it
***********************************************************************
*/
void deactivateTarget(int targetNumber) 
{
  target[targetNumber].player = NO_PLAYER;
  // @Kanishk : set LED display port OFF
}

/*
***********************************************************************
  setPlayer: sets the player for a target that has no player assigned
***********************************************************************
*/
void setPlayer(int targetNumber) 
{
  // assign target to player who has been allocated fewer shots
  if(player_a_allocated < player_b_allocated) 
  {
    target[targetNumber].player = PLAYER_A;
  }
  else
  {
    target[targetNumber].player = PLAYER_B;
  }
}

/*
***********************************************************************
  oneSecondOver: Called after every second gets over
                 Reduces time in a target. 
                 If target is for no player 
                  then setPlayer for that target
  
  @Kanishk : This function does not set score                  
***********************************************************************
*/
void oneSecondOver() 
{
  int i;
  for(i = 0; i < NO_TARGETS; ++i)
  {
    if(target[i].player != NO_PLAYER) 
    {
      --target[i].time_as_player;
      if(target[i].time_as_player <= 0) 
      {    
        deactivateTarget(i);
      }             
    } 
    else
    {
      if(target[i].time_as_player <= 0)
      {
        setPlayer(i);
        target[i].time_as_player = 5; // each target lasts 5 seconds
      } 
      else 
      {
        --target[i].time_as_player;
      }
      
    }
  }
  --gameTime;
  if(gameTime == 0) 
  {
    stopGame();
  }
}

/*
***********************************************************************
  activateTarget: To pick a target and set a player to it
***********************************************************************
*/
void activateTarget(int targetNumber, unsigned char player) 
{
  target[targetNumber].player = player;
  // @Kanishk : set LED display port ON
}

/*
***********************************************************************
  targetHit: Called when a target has been hit
             The function will distribute score and reset to 0
***********************************************************************
*/
void targetHit(int targetNumber)
{    
  if(target[targetNumber].player == PLAYER_A) 
  {
    player_a_score += target[targetNumber].score;     
  } 
  else if(target[targetNumber].player == PLAYER_B) 
  {
    player_b_score += target[targetNumber].score;      
  }
  
  target[targetNumber].score = 0; 
  deactivateTarget(targetNumber);     
  // @Kanishk: Decide if want to flash color of player on port
}                  


/*
***********************************************************************
  targetInit: Initializes the target to a maxScore and sets current score
              as 0 and sets player number to none
***********************************************************************
*/
void targetInit(Target *myTarget, unsigned char targetMaxScore)
{
  myTarget->maxScore       = targetMaxScore;
  myTarget->score          = 0;  
  myTarget->player         = NO_PLAYER; // Defined above
  myTarget->time_as_player = 0;
}                  


/*
***********************************************************************
  startGame: Starts the game by setting corresponding flags and carrying
             out other relevant actions
***********************************************************************
*/
void startGame(void) 
{
  gameRunning_flag = 1;
  player_a_score = 0;
  player_b_score = 0;
  player_a_allocated = 0;
  player_b_allocated = 0;
  gameTime = 60; // 60 seconds
}

/*
***********************************************************************
  stopGame: Stops the game by setting corresponding flags and carrying
            out other relevant actions
***********************************************************************
*/
void stopGame(void) 
{
  gameRunning_flag = 0;
}

/*
***********************************************************************
  shiftout: Transmits the character x to external shift 
            register using the SPI.  It should shift MSB first.  
             
            MISO = PM[4]
            SCK  = PM[5]
***********************************************************************
*/ 
void shiftout(char x)
{       
  int ctr;
  while(!SPISR_SPTEF) {} // read the SPTEF bit, continue if bit is 1
  SPIDR = x; // write data to SPI data register
                                    
  // wait for 30 cycles for SPI data to shift out 
  for(ctr = 0; ctr < 30; ++ctr) {;}   
}

/*
***********************************************************************
  lcdwait: Delay for approx 2 ms
***********************************************************************
*/
void lcdwait()
{
  int k = 5000;
  while(k-- > 0)
  {;}
}

/*
*********************************************************************** 
  send_byte: writes character x to the LCD
***********************************************************************
*/
void send_byte(char x)
{     
  shiftout(x);  // shift out character
  PTT_PTT4 = 1;
  PTT_PTT4 = 0; // pulse LCD clock line low->high->low
  PTT_PTT4 = 1;
  lcdwait();    // wait 2 ms for LCD to process data
}

/*
***********************************************************************
  send_i: Sends instruction byte x to LCD  
***********************************************************************
*/
void send_i(char x)
{
  // set the register select line low (instruction data)
  // send byte
  PTT_PTT2 = 0;
  send_byte(x);  
}

/*
***********************************************************************
  chgline: Move LCD cursor to position x
  NOTE: Cursor positions are encoded in the LINE1/LINE2 variables
***********************************************************************
*/
void chgline(char x)
{
  send_i(CURMOV);
  send_byte(x);
}

/*
***********************************************************************
  print_c: Print (single) character x on LCD            
***********************************************************************
*/ 
void print_c(char x)
{
  PTT_PTT2 = 1;
  send_byte(x);
}

/*
***********************************************************************
  pmsglcd: print character string str[] on LCD
***********************************************************************
*/
void pmsglcd(char str[])
{
  int i = 0;
  while(str[i] != '\0') 
  {
    print_c(str[i]);
    i += 1;
  }
}

/*
***********************************************************************
 Character I/O Library Routines for 9S12C32 (for debugging only)
***********************************************************************
 Name:         inchar
 Description:  inputs ASCII character from SCI serial port and returns it
 Example:      char ch1 = inchar();
***********************************************************************
*/
char inchar(void) 
{
  /* receives character from the terminal channel */
  while (!(SCISR1 & 0x20)); /* wait for input */
  return SCIDRL;
}

/*
***********************************************************************
 Name:         outchar
 Description:  outputs ASCII character x to SCI serial port
 Example:      outchar('x');
***********************************************************************
*/
void outchar(char x) 
{
  /* sends a character to the terminal channel */
  while (!(SCISR1 & 0x80));  /* wait for output buffer empty */
  SCIDRL = x;
}
