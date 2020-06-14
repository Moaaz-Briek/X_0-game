// SpaceInvaders.c	
// Runs on LM4F120/TM4C123
// This project is required for a collegue research.
// I take the spaceinvador code and modified it to make this easy peasy useless game.

// ******* Required Hardware I/O connections*******************
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Blue Nokia 5110
// ---------------
// Signal        (Nokia 5110) LaunchPad pin
// Reset         (RST, pin 1) connected to PA7
// SSI0Fss       (CE,  pin 2) connected to PA3
// Data/Command  (DC,  pin 3) connected to PA6
// SSI0Tx        (Din, pin 4) connected to PA5
// SSI0Clk       (Clk, pin 5) connected to PA2
// 3.3V          (Vcc, pin 6) power
// back light    (BL,  pin 7) not connected, consists of 4 white LEDs which draw ~80mA total
// Ground        (Gnd, pin 8) ground

// Red SparkFun Nokia 5110 (LCD-10168)
// -----------------------------------
// Signal        (Nokia 5110) LaunchPad pin
// 3.3V          (VCC, pin 1) power
// Ground        (GND, pin 2) ground
// SSI0Fss       (SCE, pin 3) connected to PA3
// Reset         (RST, pin 4) connected to PA7
// Data/Command  (D/C, pin 5) connected to PA6
// SSI0Tx        (DN,  pin 6) connected to PA5
// SSI0Clk       (SCLK, pin 7) connected to PA2
// back light    (LED, pin 8) not connected, consists of 4 white LEDs which draw ~80mA total

#include "tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "Random.h"
#include "TExaS.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Timer2_Init(unsigned long period);
void Delay100ms(unsigned long count); // time delay in 0.1 seconds
char checkWin(void);
unsigned long TimerCount;
unsigned long Semaphore;
char position[4][4];
char Winner=0;
char Cursor_position,stepX,stepY,player,game_Type,Game_character,flag;

	void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0       
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0        
}
void PORTE_INIT(void){
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000010;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTE_LOCK_R = 0x4C4F434B;   // 2) unlock PortE
  GPIO_PORTE_CR_R = 0x03;           // allow changes to PE2       
  GPIO_PORTE_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTE_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTE_DIR_R = 0x02;          // 5) PE0 input, PE1 output   
  GPIO_PORTE_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTE_PDR_R= 0x01;          // enable pulldown resistors on PE0       
  GPIO_PORTE_DEN_R = 0x01;          // 7) enable digital pins PE0-PE1
}

void game_Init(char Game_type){
	
	 char i,j;
	 Cursor_position=0; 
	game_Type=Game_type;
	Game_character=(game_Type*game_Type-1);
	flag=1;
	
	 if(Game_type==3)stepX=4,stepY=2;
	 if(Game_type==4)stepX=3,stepY=1;
	 player=0; // 0--> playerX 1 --> playerY 
	 for(i=0;i<Game_type;i++){
		 for(j=0;j<Game_type;j++)
		 position[i][j]=0;
	 }
}
void Draw(){
	char i,j,x,y;
	x=84/game_Type;
	y=48/game_Type;
	
	for(i=0;i<game_Type;i++){
		for(j=0;j<48;j++)Nokia5110_SetPixel(x*i,j);
	  for(j=0;j<84;j++)Nokia5110_SetPixel(j,y*i);
	}
	
	 Nokia5110_SetCursor(0,5);
	 Nokia5110_OutString("Palyer X...");
	
}
char start(){
	char SW1, SW2 ,SW3,choose=3;
	
	   
	Nokia5110_SetCursor(0,0);
	Nokia5110_OutString("3*3  or  4*4");
	Nokia5110_SetCursor(1,2);
	Nokia5110_OutChar('_');
	
	while(1){
		 SW1 = GPIO_PORTF_DATA_R&0x10;     // read PF4 into SW1
     SW2 = GPIO_PORTF_DATA_R&0x01;     // read PF0 into SW2
	   SW3 = GPIO_PORTE_DATA_R&0x01;     // read PE0 into SW3
	
		if(!(SW2)){
		Nokia5110_SetCursor(9,2);
		Nokia5110_OutChar(' ');
		Nokia5110_SetCursor(1,2);
		Nokia5110_OutChar('_');
		
		choose=3;
	}
	if(!(SW1)){
		Nokia5110_SetCursor(1,2);
		Nokia5110_OutChar(' ');
		Nokia5110_SetCursor(9,2);
		Nokia5110_OutChar('_');
		choose=4;
	}
	if(SW3) return choose;
}
	
}
 void set_Cursor() {
	 char posX,posY;
	 if( position[Cursor_position/game_Type][Cursor_position%game_Type]==0){
		        posX=((Cursor_position%game_Type)*stepX);
				    posY=((Cursor_position/game_Type)*stepY);
						position[Cursor_position/game_Type][Cursor_position%game_Type]='_';
						Nokia5110_SetCursor(posX,posY);
			  	  Nokia5110_OutChar('_');
		        Nokia5110_SetCursor(posX,posY);
						}
		 
	 }
void remove_Cursor()
{
		 char posX,posY;
		 if(position[Cursor_position/game_Type][Cursor_position%game_Type]=='_')
				{
			    posX=((Cursor_position%game_Type)*stepX);
				  posY=((Cursor_position/game_Type)*stepY);
					position[Cursor_position/game_Type][Cursor_position%game_Type]=0;
					Nokia5110_SetCursor(posX,posY);
			  	Nokia5110_OutChar(' ');
					
				}
		 
}



int main(void){
	
	 unsigned long SW1,SW2,SW3;

	TExaS_Init(SSI0_Real_Nokia5110_Scope);  // set system clock to 80 MHz
	 Random_Init(1);
 	 Nokia5110_Init();
	 Nokia5110_ClearBuffer();
	 Nokia5110_DisplayBuffer();      
	 PortF_Init();
	 PORTE_INIT();
	 Nokia5110_DisplayBuffer();
	 Nokia5110_Clear();
	 game_Init(start());  // choose Game type  3*3 or 4*4 
	 Nokia5110_Clear();
	 Delay100ms(1);
	 Draw();
	 set_Cursor();
	 Nokia5110_SetCursor(0,0);
	
	 while(1){
		 SW1 = GPIO_PORTF_DATA_R&0x10;     // read PF4 into SW1
     SW2 = GPIO_PORTF_DATA_R&0x01;     // read PF0 into SW2
		 SW3 = GPIO_PORTE_DATA_R&0x01;     // read PE0 into SW3
		
	
			
		 if(!(SW1))
			{
				
        remove_Cursor();				
				
				Cursor_position++;
				if(Cursor_position>Game_character)Cursor_position=Game_character;
    	   while(!(GPIO_PORTF_DATA_R&0x10));
				 set_Cursor();
			}
				
			if(!(SW2))
			{
				remove_Cursor();
				
				Cursor_position--;
				if(Cursor_position<0)Cursor_position=0;
				while(!(GPIO_PORTF_DATA_R&0x01));
				set_Cursor();
			}
			
			if((SW3))
				{
						while(GPIO_PORTE_DATA_R&0x01);
						if(!(player))
					{
						if(position[Cursor_position/game_Type][Cursor_position%game_Type]=='_')
						{
							Nokia5110_OutChar('X');
							position[Cursor_position/game_Type][Cursor_position%game_Type]='X';
							Cursor_position++;
							if(Cursor_position>Game_character)Cursor_position=Game_character;
							Nokia5110_SetCursor(0,5);
							Nokia5110_OutString("Palyer O..."); 
							set_Cursor();
							player^=1;
						}
					
					}
					
				else{
					if(position[Cursor_position/game_Type][Cursor_position%game_Type]=='_')
						{
							Nokia5110_OutChar('O');
							position[Cursor_position/game_Type][Cursor_position%game_Type]='O';
							Cursor_position++;
							if(Cursor_position>Game_character)Cursor_position=Game_character;
							Nokia5110_SetCursor(0,5);						
							Nokia5110_OutString("Palyer X...");
							set_Cursor();
							player^=1;
						}
				}
				
				Winner=checkWin();
			}
			
			if(Winner){
				if(Winner=='x') 
				{
					if(flag){
					Nokia5110_Clear();
					Nokia5110_SetCursor(0,2);
					Nokia5110_OutString("player X Win");
					Delay100ms(2);
					Nokia5110_Clear();
					flag=0;
									}
				}
				if(Winner=='o') {
					if(flag){
					Nokia5110_Clear();
					Nokia5110_SetCursor(0,2);
					Nokia5110_OutString("player Y Win");
					Delay100ms(2);
					Nokia5110_Clear();
					flag=0;
					}
				}
				if(Winner=='q') {
					if(flag){
					Nokia5110_Clear();
					Nokia5110_SetCursor(0,2);
					Nokia5110_OutString("No one win");
					Delay100ms(2);
					Nokia5110_Clear();
					flag=0;
					}
				}
				
			}
		}
}


char checkWin(void)
{
	char i,j,raw_counter_X = 0, raw_counter_Y = 0, coloum_counter_X = 0, coloum_counter_Y = 0, diagonal_counter_X = 0, diagonal_counter_Y = 0;
	char other_diagonal_counter_X = 0, other_diagonal_counter_Y = 0, Full = 1;

	for(i = 0; i <game_Type; i++)
	{
		raw_counter_X = 0, raw_counter_Y = 0, coloum_counter_X = 0, coloum_counter_Y = 0;
		
		for(j = 0; j <game_Type; j++)
		{
			if(position[i][j] == 'X')raw_counter_X++;  // counter check x in rows
			if(position[i][j] == 'O')raw_counter_Y++;	// counter check o in rows
		  if(position[j][i] == 'X')coloum_counter_X++;  // counter check x in cols
		  if(position[j][i] == 'O')coloum_counter_Y++;	// counter check o in cols
		  if(position[i][j] == 0) Full=0;			
		}

		if(position[i][i] == 'X')diagonal_counter_X++;  // counter check x in diagonal
		if(position[i][i] == 'O')diagonal_counter_Y++;  // counter check o in diagonal

		if(position[i][game_Type - 1 - i] == 'X')other_diagonal_counter_X++; // counter check x in other diagonal
		if(position[i][game_Type - 1 - i] == 'O')other_diagonal_counter_Y++; // counter check o in other diagonal

		if(raw_counter_X ==game_Type || coloum_counter_X ==game_Type || diagonal_counter_X ==game_Type || other_diagonal_counter_X ==game_Type)return 'x';
		if(raw_counter_Y ==game_Type || coloum_counter_Y ==game_Type || diagonal_counter_Y ==game_Type || other_diagonal_counter_Y == game_Type)return 'o';
				
	}
	
	if(Full) return 'q';
	return 0;
}

void Delay100ms(unsigned long count){unsigned long volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}