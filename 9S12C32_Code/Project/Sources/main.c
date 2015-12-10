/*
************************************************************************
 ECE 362 - Mini-Project C Source File - Fall 2015
***********************************************************************
	 	   			 		  			 		  		
 Team ID: 22
 Project Name: Bang Bang
 
 Team Members:
   - Team/Doc Leader: Harika Thatukuru       Signature: Harika Thatukuru
   - Software Leader: Abhishek Srikanth      Signature: Abhishek
   - Interface Leader: Tatparya Shankar      Signature: Tatparya Shankar
   - Peripheral Leader: Kanishk Yadav        Signature: Kanishk
 
 Academic Honesty Statement:  In signing above, we hereby certify that we 
 are the individuals who created this HC(S)12 source file and that we have
 not copied the work of any other student (past or present) while completing 
 it. We understand that if we fail to honor this agreement, we will receive 
 a grade of ZERO and be subject to possible disciplinary action.
 
***********************************************************************
 The objective of this Mini-Project is to create a two player shooter 
 that targets ages 8-80. Nerf guns are used, one per player, to score
 as much as they can over a time period of a minute by shooting at 
 allocated targets.
 The targets are adorned with an extensive matrix of LEDs to define which
 player must shoot them. The targets detect a hit using very sensitive 
 piezo-electric vibration sensors. These sensors are converted to digital 
 values to see if a target is hit.
 Players are scored per hit and the winner is announced at the end of the game!
 We hope you enjoy!
***********************************************************************
 List of project-specific success criteria (functionality that will be
 demonstrated):
 1. To implement various peripherals on an extensive scale
   1-a. SPI : 
     1-a-i.   LCD display for prompts and scores
     1-a-ii.  Player ownership information via LEDs
     1-a-iii. Game time matrix vie LEDs
   1-b. RTI : To detect events on pushbuttons used for critical operations
   1-c. TIM : To sample a plethora of targets to detect hits
   1-d. ATD : To measure vibration values and evaluate a hit out of them
 2. To demonstrate successful programming on the microcontroller
   2-a. To start the game on an event and run it for precisely a minute
   2-b. To manage scores of both players for final results
   2-c. To psedo-randomly allocate targets to users ensuring fair distribution
   2-d. To manage all information in a concise and efficient manner using data structures
   2-e. To modularly divide the program into bit-size pieces with easy modifications (Code Scalability)
   2-f. To ensure industry standards are implemented in program
 3. To demonstrate understanding of circuit wiring
   3-a. To create a complex led matrix used to display target allocations and time
   3-b. To connect all interfacing components together to form a coherent circuit
 4. To work in a real life engineering team and gain valuable teaming experience
   4-a. To foster everlasting relationships with team-mates
   4-b. To obtain an over-arching and comprehensive understanding of all underlying concepts relevant to the coursework
 5. To create a complete and wholesome gaming experience for the audience
   5-a. Ensure the game is functional and majorly error free
   5-b. Make game challenging, yet not impossible
***********************************************************************
  Date code started: November 22, 2015
  Update history (add an entry every time a significant change is made):
  Date: Dec  9 Name: Yadav9   Update: Added push button functionality
  Date: Dec  6 Name: Yadav9   Update: ATD condition fix
  Date: Dec  6 Name: srikanth Update: Game LCD Updates set
  Date: Dec  5 Name: srikanth Update: Game-time display code added
  Date: Nov 25 Name: Yadav9   Update: Added function to display target LED
  Date: Nov 25 Name: Yadav9   Update: Added RTI module and TIM module
  Date: Nov 24 Name: Yadav9   Update: Added Module and port initialization()
  Date: Nov 23 Name: srikanth Update: Added function for assigning target to player
  Date: Nov 23 Name: srikanth Update: Added function to be called every  second. Used to update ALL game variables
  Date: Nov 23 Name: srikanth Update: Added function to startGame() and to endGame()
  Date: Nov 23 Name: srikanth Update: Added function to activateTarget() and deactivateTarget()
  Date: Nov 23 Name: srikanth Update: Added function to be called when targetHit()
  Date: Nov 22 Name: srikanth Update: Created LCD functions and basic initialization() block
  Date: Nov 22 Name: srikanth Update: Created "struct TargetData"used to define each target
  Date: Nov 22 Name: srikanth Update: First Commit
  // above is a listing of all major commits
  // All commits may be viewed at "https://github.com/tatparya/PewPew_ECE362/commits/master"
***********************************************************************
*/

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>

/* Game oriented definitions */
#define GAMETIME_DATA PTT_PTT0
#define GAMETIME_CLK PTT_PTT1
#define NO_TARGETS 6
#define NO_PLAYER 0
#define PLAYER_A  1
#define PLAYER_B  2

/* ASCII character definitions */
#define CR 0x0D	// ASCII return character
#define LF 0x0A	// ASCII something

/* LCD COMMUNICATION BIT MASKS */
#define RS 0x04		// RS pin mask (PTT[2])
#define RW 0x08		// R/W pin mask (PTT[3])
#define LCDCLK 0x10	// LCD EN/CLK pin mask (PTT[4])

/* LCD INSTRUCTION CHARACTERS */
#define LCDON 0x0F	// LCD initialization command
#define LCDCLR 0x01	// LCD clear display command
#define TWOLINE 0x38// LCD 2-line enable command
#define CURMOV 0xFE	// LCD cursor move instruction
#define LINE1  0x80	// LCD line 1 cursor position
#define LINE2  0xC0	// LCD line 2 cursor position



  void targetInit(Target *myTarget, unsigned char targetMaxScore, int *atd);
void targetHit(int targetNumber);
void activateTarget(int targetNumber, unsigned char player);
void deactivateTarget(int targetNumber);
void oneSecondOver(void);
  void setPlayer(int targetNumber);
  void clock(void);
  void set_leds(void); 
// following are LCD functions
void print_c(char);
void pmsglcd(char str[]);
  void shiftout(char);
  void lcdwait(void);
  void send_byte(char);
  void send_i(char);
  void chgline(char);
  void outchar(char x); 
// for RTI initializations
void startGame(void);
  void gameTimeClock(void);
  void stopGame(void);

unsigned char gameRunning_flag; // true -> game is running
unsigned int player_a_score;
unsigned int player_b_score; 
unsigned int player_a_allocated;
unsigned int player_b_allocated; 
Target target[NO_TARGETS];

unsigned int gameTime;
char leftpb = 0;
char rghtpb = 0;
int counter = 0;
int counter_atd = 0;
//Interrupt RTI
int prevleft = 0;
int prevrght = 0;
int max = 0;

//Interrupt TIM
int tencnt = 0;
int onecnt = 0;
char tenths = 0;
char onesec = 0;
char test = 0;
char test_1 = 0;
char test_2 = 0;
char test_3 = 0;


int val = 0;
int tempMax = 0;
char valStr[] = "***";
char maxStr[] = "***";
int itr = 0;
						
						
void initializations() 
{
  /* Set the PLL speed (bus clock = 24 MHz) */
  CLKSEL = CLKSEL & 0x80; // disengage PLL from system
  PLLCTL = PLLCTL | 0x40; // turn on PLL
  SYNR = 0x02;            // set PLL multiplier
  REFDV = 0;              // set PLL divider
  while (!(CRGFLG & 0x08)){  }
  CLKSEL = CLKSEL | 0x80; // engage PLL
  
  
 
  /* Disable watchdog timer (COPCTL register) */
   COPCTL = 0x40   ; // COP off; RTI and COP stopped in BDM-mode

 /* Initialize asynchronous serial port (SCI) for 9600 baud, interrupts off initially */
   SCIBDH =  0x00; //set baud rate to 9600
   SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx) 
   SCICR1 =  0x00; //$9C = 156
   SCICR2 =  0x0C; //initialize SCI for program-driven operation
   DDRB   =  0x10; //set PB4 for output mode
   PORTB  =  0x10; //assert DTR pin on COM port
     
     /*
   Initialize SPI for baud rate of 6 Mbs, MSB first
   (note that R/S, R/W', and LCD clk are on different PTT pins)
 */
   SPICR1 = 0x50;   //SPI control registers
   SPICR2 = 0x00;
   SPIBR_SPR0 = 1; //baud rate selection bits
   SPIBR_SPR1 = 0;  
   SPIBR_SPR2 = 0;
   SPIBR_SPPR0 = 0; //baud rate preselection bits
   SPIBR_SPPR1 = 0;
   SPIBR_SPPR2 = 0;

 /* Initialize digital I/O port pins */
    DDRAD = 0x00;
    DDRT = 0xFF;
    DDRM_DDRM3 = 1; //Initializing registers for LCD module
    DDRM_DDRM4 = 1;
    DDRM_DDRM5 = 1;
    ATDDIEN = 0xFF; // 0011_1111 input enable 3F  
    PTT_PTT0 = 0; //PORT 0
    PTT_PTT1 = 0; //PORT 1
    PTT_PTT3 = 0;
    PTT_PTT2 = 0;
 /*
    Initialize the LCD
      - pull LCDCLK high (idle)
      - pull R/W' low (write state)
      - turn on LCD (LCDON instruction)
      - enable two-line mode (TWOLINE instruction)
      - clear LCD (LCDCLR instruction)
      - wait for 2ms so that the LCD can wake up    
 */
    PTT_PTT6 = 1; //LCDCLK HIGH
    PTT_PTT5 = 0;  //R/W' LOW
    send_i(LCDON);   //TURN ON LCD
    send_i(TWOLINE);   // ENABLE 2 LINE MODE
    send_i(LCDCLR);     //CLEAR LCD
    lcdwait();          //WAIT FOR LCD TO WAKE UP
    
  /*
   Initialize the RTI for an 2.048 ms interrupt rate
  */
  RTICTL = 0x1F; //2.048 ms interrupt rate
  CRGINT = 0x80;  //enable CRG block
  
  /*
    Initialize TIM Ch 7 (TC7) for periodic interrupts every 10.0 ms 
    - Enable timer subsystem                        
     - Set channel 7 for output compare
     - Set appropriate pre-scale factor and enable counter reset after OC7
     - Set up channel 7 to generate 10 ms interrupt rate
     - Initially disable TIM Ch 7 interrupts                                                                                                                         
  */                                                                                                                         
  TSCR1 = 0x80; //enables timer subsystem
  TIOS = 0x80;
  TSCR2 = 0x0C;
  TC7 =  1500;
  TIE = 0x80;
  // TIE_C7I = 0;
    
    
  // ATD                                                                                                                       
  ATDCTL2 = 0x80; //CONTROL REG 2
  ATDCTL3 = 0x30; //3 TO ENABLE 6 CONVRESIONS
  ATDCTL4 = 0x85; //4
  ATDCTL2_AFFC = 1;
  
  // targetInit(Target *myTarget, unsigned char targetMaxScore)  
  targetInit( &(target[0]), 10 , &ATDDR0H ); // target 1
  targetInit( &(target[1]),  9 , &ATDDR1H ); // target 2
  targetInit( &(target[2]),  7 , &ATDDR2H); // target 3
  targetInit( &(target[3]),  5 , &ATDDR3H); // target 4
  targetInit( &(target[4]),  3 , &ATDDR4H); // target 5
  targetInit( &(target[5]),  3 , &ATDDR5H); // target 6
  
  // Variable initializations
  gameRunning_flag = 0;
}

void main(void) 
{
  DisableInterrupts
  initializations();
  EnableInterrupts;
	
  send_i(LCDCLR);
  chgline(LINE1);
  pmsglcd("BangBang sez yo");
  chgline(LINE2);
  pmsglcd("Strt 2 play gem");
	
  for(;;) 
  {
    //_FEED_COP(); // feeds the dog 
    // TODO (for watchdog timer @Kanishk) 
    if(leftpb)           //PUSH left push button to start the game
    {
      leftpb = 0;
      
      max = 0;
      
      counter = 0;   // Counts to 100 for a second
     // PTT_PTT1 = 1;  // Light up the LED  (game started!)
      startGame();   // GAME STARTS
    }
    
    if( rghtpb )
    {      
      rghtpb = 0;

      send_i(LCDCLR);
      chgline(LINE1);
      pmsglcd("BangBang sez yo");
      chgline(LINE2);
      pmsglcd("Strt 2 play gem");

      //PTT_PTT0 = 0;
      //PTT_PTT1 = 0;    
      gameRunning_flag = 0;
    }
    
    if(gameRunning_flag != 0) 
    {
      if(tenths) 
      {
        tenths = 0;   
        //PTT_PTT0 = 0;////////////////////////////////////////////////////
        if(counter == 100) 
        {
          // PTT_PTT0 = 1;  //////////////////////////////////////
          oneSecondOver();
          counter = 0;
        }       
        counter++;
        ATDCTL5 = 0x10;     //perform ATD Conversion
        while(ATDSTAT0_SCF != 1){};
/*      for(counter_atd = 0; counter_atd < NO_TARGETS; counter_atd++) 
        {
          if(target[counter_atd].player != NO_PLAYER) 
          {
            if(*target[counter_atd].atd_address > 255) 
            {
              target[counter_atd].score = target[counter_atd].maxScore;
              targetHit(counter_atd);
            }
          }
      }
*/      
        if(target[0].player == PLAYER_A || target[0].player == PLAYER_B)
        {
  
          if(ATDDR0H > 20) 
          {
            target[0].score = 10;
            targetHit(0);
          }
        }
        
        if(target[1].player == PLAYER_A || target[1].player == PLAYER_B)
        {
          
          if(ATDDR1H > 20) 
          {
            target[1].score = 10;
            targetHit(1);
          }
        }
        if(target[2].player == PLAYER_A || target[2].player == PLAYER_B)
        {
          if(ATDDR2H > 20) 
          {
            target[2].score = 10;
            targetHit(2);
          }
        }
        if(target[3].player == PLAYER_A || target[3].player == PLAYER_B)
        {
          if(ATDDR3H > 20) 
          {
            target[3].score = 10;
            targetHit(3);
          }
        }
        if(target[4].player == PLAYER_A || target[4].player == PLAYER_B)
        {
          if(ATDDR4H > 20) 
          {
            target[4].score = 10;
            targetHit(4);
          }
        }
        if(target[5].player == PLAYER_A || target[5].player == PLAYER_B)
        {
          if(ATDDR5H > 20) 
          {
            target[5].score = 10;
            targetHit(5);
          }
        }
       }
      
    }
  }
}


 /*
 ***********************************************************************
 RTI interrupt service routine: RTI_ISR
   Initialized for 2.048 ms interrupt rate
   Samples state of pushbuttons (PAD7 = left, PAD6 = right)
   If change in state from "high" to "low" detected, set pushbutton flag
      leftpb (for PAD7 H -> L), rghtpb (for PAD6 H -> L)
      Recall that pushbuttons are momentary contact closures to ground
 ***********************************************************************
 */
interrupt 7 void RTI_ISR(void)
{
  // clear RTI interrupt flag
  CRGFLG = CRGFLG | 0x80;
  if(PORTAD0_PTAD6 == 0 && prevrght == 1)
  {
  	rghtpb = 1;
  }
      
  if(PORTAD0_PTAD7  == 0 && prevleft == 1)
  {
    leftpb = 1;
    //  PTT_PTT0 = 1; //PORT 0
    //  PTT_PTT1 = 1;
  }
      
  prevleft = PORTAD0_PTAD7 ;
  prevrght = PORTAD0_PTAD6;

 }
 
  /************************************************************************                      
   TIM interrupt service routine
   Initialized for 10.0 ms interrupt rate
   Uses variable "tencnt" to track if one-tenth second has accumulated
      and sets "tenths" flag
                         
   Uses variable "onecnt" to track if one second has accumulated and
      sets "onesec" flag                                                                                             
 ;***********************************************************************
 */
interrupt 15 void TIM_ISR(void)
{
   // clear TIM CH 7 interrupt flag
   TFLG1 = TFLG1 | 0x80;
   tencnt = tencnt + 1; //update tenct flag
   //onecnt = onecnt + 1; //update one count flag
           
   if(tencnt == 10)
   {  //reset tenct flag when it reaches 10
     tencnt = 0;
     tenths = 1;     
   }
}

/*
***********************************************************************
  deactivateTarget: To pick a target and set a player to it
***********************************************************************
*/
void deactivateTarget(int targetNumber) 
{
  
  target[targetNumber].player = NO_PLAYER;
  set_leds();

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
    activateTarget(targetNumber, PLAYER_A);    
    player_a_allocated += target[targetNumber].maxScore;
  }
  else
  {
    activateTarget(targetNumber, PLAYER_B);
    player_b_allocated += target[targetNumber].maxScore;
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
    --target[i].time_as_player;

    if(target[i].time_as_player <= 0 || target[i].player == NO_PLAYER)
    {
      setPlayer(i);
      target[i].time_as_player = 5; // each target lasts 5 seconds
    }
  }
  
  --gameTime;
  if(gameTime % 10 == 0)
  {
    GAMETIME_DATA = 0;
    gameTimeClock();
  }
  if(gameTime == 0) 
  {
   // PTT_PTT1 = 0;
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
  set_leds();
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
}                  


/*
***********************************************************************
  targetInit: Initializes the target to a maxScore and sets current score
              as 0 and sets player number to none
***********************************************************************
*/
void targetInit(Target *myTarget, unsigned char targetMaxScore, int *atd)
{
  myTarget->maxScore       = targetMaxScore;
  myTarget->score          = 0;  
  myTarget->player         = NO_PLAYER; // Defined above
  myTarget->time_as_player = 0;
  myTarget->atd_address    = atd; 
}                  

/*
***********************************************************************
  gameTimeClock : single clock for shift register for game Time display
***********************************************************************
*/
void gameTimeClock()
{
  GAMETIME_CLK = 1;
  GAMETIME_CLK = 0;
  GAMETIME_CLK = 1;
}

/*
***********************************************************************
  startGame: Starts the game by setting corresponding flags and carrying
             out other relevant actions
***********************************************************************
*/
void startGame(void) 
{ 
  send_i(LCDCLR);
  chgline(LINE1);
  pmsglcd("Best o' luck!");
  chgline(LINE2);
  pmsglcd("Have fun!");

  gameRunning_flag = 1;
  player_a_score = 0;
  player_b_score = 0;
  player_a_allocated = 0;
  player_b_allocated = 0;
  gameTime = 60; // 60 seconds
  for(itr = 0; itr < NO_TARGETS; ++itr)
  {
    setPlayer(itr);
  }
  // following is logic to initialize time led's to all ON
  GAMETIME_DATA = 1;
  for(itr = 0; itr < 6; ++itr)
  {
    gameTimeClock();
  }
  GAMETIME_DATA = 0;
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
  for(itr = 0; itr < NO_TARGETS; itr++)
  {
    target[itr].player = NO_PLAYER; 
  }
  set_leds();
  send_i(LCDCLR);
  chgline(LINE1);
  pmsglcd("Player A: ");
  print_c( (char)((player_a_score / 100 % 10) + 48) );
  print_c( (char)((player_a_score / 10 % 10) + 48) );
  print_c( (char)((player_a_score % 10) + 48) );
  chgline(LINE2);
  pmsglcd("Player B: ");
  print_c( (char)((player_b_score / 100 % 10) + 48) );
  print_c( (char)((player_b_score / 10 % 10) + 48) );
  print_c( (char)((player_b_score % 10) + 48) );
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
   int itr = 30;
   // read the SPTEF bit, continue if bit is 1
   while(!SPISR_SPTEF)
   {          
   }
   // write data to SPI data register
   SPIDR = x;               
   // wait for 30 cycles for SPI data to shift out
   while(itr) 
   {             
     itr--;
   } 
}

/*
***********************************************************************
  lcdwait: Delay for approx 2 ms
***********************************************************************
*/
void lcdwait()
{
  int itr = 5000;
  while( itr )
  {
    itr--;
  } 
}

/*
*********************************************************************** 
  send_byte: writes character x to the LCD
***********************************************************************
*/
void send_byte(char x)
{     
     // shift out character
     shiftout(x);      
     // pulse LCD clock line low->high->low
     PTT_PTT6 = 1;     
     PTT_PTT6 = 0;
     PTT_PTT6 = 1;
     // wait 2 ms for LCD to process data
     lcdwait();   
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
  PTT_PTT4 = 0;
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
  PTT_PTT4 = 1;
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

 /*
***********************************************************************
  clock: clock
***********************************************************************
*/ 
void clock() 
{
  PTT_PTT3 = 1;
  PTT_PTT3 = 0;
  PTT_PTT3 = 1;  
}
 /*
***********************************************************************
  set_leds: set LEDS
***********************************************************************
*/ 
void set_leds() 
{
  int i; 
  for(i = NO_TARGETS-1; i >= 0;i--) 
  {
    if(target[i].player == PLAYER_A)
    {
      PTT_PTT2 = 1;
    } 
    else
    {
      PTT_PTT2 = 0;
    }
    clock();
     if(target[i].player == PLAYER_B)
    {
      PTT_PTT2 = 1;
    } 
    else
    {
      PTT_PTT2 = 0;
    }
    clock();      
  }  
}
