#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */

/* Game oriented definitions */
#define NO_TARGETS 6
#define NO_PLAYER 0
#define PLAYER_A  1
#define PLAYER_B  2

/* ASCII character definitions */
#define CR 0x0D	// ASCII return character   

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

//Interrupt TIM
int tencnt = 0;
int onecnt = 0;
char tenths = 0;
char onesec = 0;
char test = 0;
char test_1 = 0;
char test_2 = 0;
char test_3 = 0;
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
    

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    
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
  for(;;) 
  {
    //_FEED_COP(); // feeds the dog 
                 // TODO (for watchdog timer @Kanishk) 
                 
    if(leftpb)           //PUSH left push button to start the game
    {
      leftpb = 0;
      counter = 0;
      outchar('C');
      PTT_PTT1 = 1;//Light up the LED  (to check if push button is working)
      startGame();   //GAME STARTS
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
        
        for(counter_atd = 0; counter_atd < NO_TARGETS; counter_atd++) 
        {
              
              if(ATDDR5H > 128) {
              PTT_PTT0 = 1;
              }
              
          if(target[counter_atd].player != NO_PLAYER) 
          {
            test = ATDDR0H;      ///////////////////////////////////////////////////////
            test_1 = (test % 10);
            test_2 = (test/10) %10;
            test_3 = (test/100) %10;  //////////////////////////////////////////////////
            outchar('C');
            outchar(':');
            outchar(test_3);
            outchar(test_2);
            outchar(test_1);
            outchar(CR);
           
            
            target[counter_atd].score = (target[counter_atd].maxScore *(*(target[counter_atd].atd_address)))/255;
            targetHit(counter_atd);
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
           
          
       if(PORTAD0_PTAD6 == 0 && prevrght == 1){
         rghtpb = 1;
          ;
        
       }
      
       if(PORTAD0_PTAD7 == 0 && prevleft == 1){
         leftpb = 1;
        //  PTT_PTT0 = 1; //PORT 0
         //  PTT_PTT1 = 1;
        
       }
      
       prevleft = PORTAD0_PTAD7;
       prevrght = PORTAD0_PTAD6;
      ;

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
  if(target[targetNumber].time_as_player < 2) 
  {
    target[targetNumber].time_as_player = 2;          //to off the target momentarily when target switches from one player to another
  } 
  // @Kanishk : set LED display port OFF
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
    PTT_PTT1 = 0;
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
  // @Kanishk: Decide if want to flash color of player on port
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