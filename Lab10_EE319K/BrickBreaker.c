// SpaceInvaders.c
// Runs on TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the ECE319K Lab 10

// Last Modified: 1/2/2023 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php

// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// buttons connected to PE0-PE3
// 32*R resistor DAC bit 0 on PB0 (least significant bit)
// 16*R resistor DAC bit 1 on PB1
// 8*R resistor DAC bit 2 on PB2 
// 4*R resistor DAC bit 3 on PB3
// 2*R resistor DAC bit 4 on PB4
// 1*R resistor DAC bit 5 on PB5 (most significant bit)
// LED on PD1
// LED on PD0


#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/ST7735.h"
#include "Random.h"
#include "TExaS.h"
#include "../inc/ADC.h"
#include "../inc/DAC.h"
#include "Images.h"
#include "../inc/wave.h"
#include "Timer1.h"
#include "../inc/Timer1A.h"
#include "../inc/Timer4A.h"
#include "../inc/Button.h"
#define BRICKS 20
extern void Timer1A_Init(void(*task)(void), uint32_t period, uint32_t priority);
extern void Timer1A_Stop(void);
extern void Timer2A_Init(void(*task)(void), uint32_t period, uint32_t priority); 
extern void Timer2A_Stop(void); 
extern uint8_t pauseFlag; 
extern uint8_t restartFlag; 
typedef enum {False, True} bool; 
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
uint32_t time; 
uint8_t executionCount; 
uint8_t resetBall; 
uint8_t shootStage; 
uint8_t brickCount; 
uint8_t impbrickCount;
uint8_t widenEffect; 
uint8_t noBoundEffect; 
uint8_t widenTimer; 
uint8_t noBoundTimer; 

//STRUCTS--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 struct paddle{
	int16_t x;
	int16_t y;  
	uint8_t w, h; 
	int8_t lives;
	uint16_t score;
  const uint16_t *image;
	bool moved; 
};
typedef struct paddle paddle_t; 

//WHEN DEFINING THE Y CORDINATE DESIRED, Top Left is defined by pixel location + 12
struct brick{
	uint8_t x;
	uint8_t y;  
	int8_t w; 
	int8_t h; 
	int8_t strength; //Wood - 2, Purple - 3, Orange - 4, White - 5, Impenetrable - 127
	int8_t type; //Wood - 2, Purple - 3, Orange - 4, White - 5, Impenetrable - 127
	uint8_t id; 
	uint8_t pUp; //0 - none, 1 - plusLife, 2 - Widen, 3 - No Bounds
	const uint16_t *(*image[6]);
};
typedef struct brick brick_t; 

struct ball{
	int16_t x;
	int16_t y; 
	uint8_t w; //In pixels
	uint16_t h; 
	int8_t xvel; 
	int8_t yvel; 
	const uint16_t *image;
	bool moved; 
};
typedef struct ball ball_t;

struct power{
	int16_t x; 
	int16_t y;
	int8_t w, h; 
	int8_t yvel; 
	uint8_t active; 
	const uint16_t *image; 
};
typedef struct power power_t; 
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//OBJECT INITIALIZATIONS--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
paddle_t UserPaddle = {48, 159, 32, 4, 1, 0, paddle, False};
ball_t ball = {59, 155, 10, 10, 0, 0, brickball, False};  
power_t WidenP = {0, 0, 20, 20, 1, 0, PlusLen};
power_t ExtraLiveP = {0, 0, 20, 20, 1, 0, ExtraLife}; 
power_t NoBoundP = {0, 0, 20, 20, 1, 0, NoBound}; 

//Wood - 2, Purple - 3, Orange - 4, White - 5, Impenetrable - 127
const uint16_t *WhiteBricks[6] = {DestroyedBrick, WhiteBrick1, WhiteBrick2, WhiteBrick3, WhiteBrick4, WhiteBrick5};
const uint16_t *OrangeBricks[6] = {DestroyedBrick, OrangeBrick1, OrangeBrick2, OrangeBrick3, OrangeBrick4, DestroyedBrick};
const uint16_t *PurpleBricks[6] = {DestroyedBrick, PurpleBrick1, PurpleBrick2, PurpleBrick3, DestroyedBrick, DestroyedBrick};
const uint16_t *WoodBricks[6] = {DestroyedBrick, woodBrick1, woodBrick2, DestroyedBrick, DestroyedBrick, DestroyedBrick};
const uint16_t *ImpenetrableBricks[6] = {ImpenetrableBrick, ImpenetrableBrick, ImpenetrableBrick, ImpenetrableBrick, ImpenetrableBrick, ImpenetrableBrick};

//Row 0
brick_t b0 = {6, 22, 26, 12, 2, 2,  0, 0, WoodBricks};
brick_t b1 = {38, 22, 26, 12, 4, 4, 1, 0, OrangeBricks};
brick_t b2 = {64, 22, 26, 12, 4, 4, 2, 2, OrangeBricks};
brick_t b3 = {95, 22, 26, 12, 2, 2, 3, 0, WoodBricks};
//Row 1
brick_t b4 = {6, 34, 26, 12, 4, 4,  4, 0, OrangeBricks};
brick_t b5 = {38, 34, 26, 12, 2, 2, 5, 0, WoodBricks};
brick_t b6 = {64, 34, 26, 12, 2, 2, 6, 0, WoodBricks};
brick_t b7 = {95, 34, 26, 12, 4, 4, 7, 1, OrangeBricks};
//Row 2
brick_t b8 = {6, 46, 26, 12, 127, 127, 8, 0, ImpenetrableBricks};
brick_t b9 = {38, 46, 26, 12, 5, 5, 9, 2, WhiteBricks};
brick_t b10 = {64, 46, 26, 12, 5, 5, 10, 0, WhiteBricks};
brick_t b11 = {95, 46, 26, 12, 127, 127, 11, 0, ImpenetrableBricks};
//Row 3
brick_t b12 = {6, 58, 26, 12, 3, 3, 12, 0, PurpleBricks};
brick_t b13 = {38, 58, 26, 12, 3, 3, 13, 3, PurpleBricks};
brick_t b14 = {64, 58, 26, 12, 3, 3, 14, 0, PurpleBricks};
brick_t b15 = {95, 58, 26, 12, 3, 3, 15, 0, PurpleBricks};
//Row 4
brick_t b16 = {6, 70, 26, 12, 127, 127, 16, 0, ImpenetrableBricks};
brick_t b17 = {38, 70, 26, 12, 2, 2, 17, 0, WoodBricks};
brick_t b18 = {64, 70, 26, 12, 2, 2, 18, 1, WoodBricks};
brick_t b19 = {95, 70, 26, 12, 127, 127, 19, 0, ImpenetrableBricks};
//0 - none, 1 - plusLife, 2 - Widen, 3 - No Bounds
brick_t *brickField[BRICKS] = {&b0, &b1, &b2, &b3, &b4, &b5, &b6, &b7, &b8, &b9, &b10, &b11, &b12, &b13, &b14, &b15,& b16, &b17, &b18, &b19}; 
static int8_t brickId = -1; //Flag to check if brick has been hit: -1 corresponds to no bricks, any other number corresponds to the actual brick
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	

//MISC--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void static DelayMs(uint32_t n){
  volatile uint32_t time;
  while(n){
    time = 6665;  // 1msec, tuned at 80 MHz
    while(time){
      time--;
    }
    n--;
  }
}

void Button_Init(void){ 
	SYSCTL_RCGCGPIO_R  |= 0x10;
	volatile int delay = 0;
	delay = delay + 1;
	delay = delay +1;
	GPIO_PORTE_DIR_R &= ~0x0F;
	GPIO_PORTE_DEN_R |= 0x0F;
}

uint32_t Button_In(void){ 
  return GPIO_PORTE_DATA_R & 0x0F; // replace this line
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//ADC SAMPLING--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Timer3A_Init(uint32_t period, uint32_t priority){
  volatile uint32_t delay;
  SYSCTL_RCGCTIMER_R |= 0x08;   // 0) activate TIMER3
  delay = SYSCTL_RCGCTIMER_R;         // user function
  TIMER3_CTL_R = 0x00000000;    // 1) disable timer2A during setup
  TIMER3_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER3_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER3_TAILR_R = period-1;    // 4) reload value
  TIMER3_TAPR_R = 0;            // 5) bus clock resolution
  TIMER3_ICR_R = 0x00000001;    // 6) clear timer2A timeout flag
  TIMER3_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI8_R = (NVIC_PRI8_R&0x00FFFFFF)|(priority<<29); // priority  
// interrupts enabled in the main program after all devices initialized
// vector number 39, interrupt number 23
  NVIC_EN1_R = 1<<(35-32);      // 9) enable IRQ 35 in NVIC
  TIMER3_CTL_R = 0x00000001;    // 10) enable timer3A
}

void Timer3A_Stop(void){
  NVIC_DIS1_R = 1<<(35-32);   // 9) disable interrupt 35 in NVIC
  TIMER3_CTL_R = 0x00000000;  // 10) disable timer3
}

static uint32_t ADCMail;
void Timer3A_Handler(void){
  TIMER3_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER2A timeout
	if(pauseFlag == 0){
	ADCMail = ADC_In();
	executionCount++;
	if(executionCount == 10){
		executionCount = 0; 
		time++; 
		if(widenEffect){
			widenTimer--;
			if(widenTimer == 0){
				widenEffect = 0; 
				UserPaddle.w = 32;
				UserPaddle.image = paddle; 
				ST7735_FillRect(UserPaddle.x + UserPaddle.w, UserPaddle.y - UserPaddle.h + 1, UserPaddle.w + 10, UserPaddle.h, ST7735_WHITE); 
			}
		}
		if(noBoundEffect){
			noBoundTimer--;
			if(noBoundTimer == 0)
				noBoundEffect = 0; 
		}
	}
}
}

uint32_t Convert(uint32_t x){
	
	uint32_t pixelLoc = x/32; 
	pixelLoc -= 127; //Comment this line for simulation
	if(pixelLoc >= 95 && widenEffect == 0)
		return 95; 
	else if(pixelLoc >= 63 && widenEffect == 1)
		return 63;
	else if(pixelLoc < 16)
		return 0; 
	else
		return pixelLoc - 16;	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//LANGUAGE--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
typedef enum {English, Spanish} Language_t;
Language_t myLanguage = English;
typedef enum {HELLO, GOODBYE, LANGUAGE, LIVES, TIME, SCORE, WIN, LOSE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Lives_English[] = "Lives:"; 
const char Lives_Spanish[] = "Vidas:"; 
const char Time_English[] = "Timer: "; 
const char Time_Spanish[] = "Tiempo:"; 
const char Score_Enlish[] = "Score: "; 
const char Score_Spanish[] = "Marca: ";  
const char Win_English[] = "You Win!";
const char Win_Spanish[] = "Tu Ganas!";
const char Lose_English[] = "You Lose!";
const char Lose_Spanish[] = "Tu Pierdes!"; 
const char *Phrases[8][2]={
  {Hello_English,Hello_Spanish},
  {Goodbye_English,Goodbye_Spanish},
  {Language_English,Language_Spanish},
	{Lives_English, Lives_Spanish},
	{Time_English, Time_Spanish},
	{Score_Enlish, Score_Spanish},
	{Win_English, Win_Spanish},
	{Lose_English, Lose_Spanish}};

void LangSelect(){
	uint32_t button = 0;
	while(1){ //Left Button = 0x04, Right Button = 0x08 
		button = Button_In(); 
		if(button == 0x04 || button == 0x08)
			break; 
	}
	myLanguage = (button == 0x04)? English:Spanish; 
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	
	
//DRAW FUNCTIONS---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void DrawBall(ball_t b){
	ST7735_DrawBitmap(b.x, b.y, b.image, b.w, b.h); 
}

void DrawPaddle(paddle_t p){
	ST7735_DrawBitmap(p.x, p.y, p.image, p.w, p.h); 
}

void DrawBrick(brick_t b){
	if(b.strength != 127)
		ST7735_DrawBitmap(b.x, b.y, (*b.image)[b.strength], b.w, b.h); 
	else
		ST7735_DrawBitmap(b.x, b.y, (*b.image)[0], b.w, b.h);
}

void DrawPower(power_t p){
	ST7735_DrawBitmap(p.x, p.y, p.image, p.w, p.h); 
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	
//COLLISION FUNCTIONS--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void brickCollision(){
	for(uint8_t i = 0; i < BRICKS; i++){
		
		if(brickField[i]->strength != 0)
		{	//TOP COLLISION
			playsound(hit); 
			if((ball.x + ball.w >=  brickField[i]->x  && ball.x <= brickField[i]->x + brickField[i]->w) && ball.y ==  brickField[i]->y - brickField[i]->h){
				ball.yvel *= -1; 
				if(brickField[i]->strength != 127){
					brickField[i]->strength--; 
				}
				brickId = brickField[i]->id;
				break;
		}
			//BOTTOM COLLISION
			if(ball.y - ball.w == brickField[i]->y && ball.y > brickField[i]->y && ball.x + ball.w >= brickField[i]->x && ball.x <= brickField[i]->x + brickField[i]->w){
				playsound(hit); 
				ball.yvel *= -1; 
				if(brickField[i]->strength != 127){
					brickField[i]->strength--;
				}
				brickId = brickField[i]->id;
				break; 
		}
			//SIDE COLLISIONS
			if(ball.y >= brickField[i]->y - brickField[i]->h && ball.y - ball.h <= brickField[i]->y){ //Check to see if ball.y is in range 
					if((ball.x + ball.w == brickField[i]->x && ball.xvel > 0)|| (ball.x == brickField[i]->x + brickField[i]->w && ball.xvel < 0)){ //Check to see if collision with side
						playsound(hit); 
						ball.xvel *= -1;
						if(brickField[i]->strength != 127){
							brickField[i]->strength--;
						}
						brickId = brickField[i]->id;
						break; 
					}
			}
		}
	}
	if(brickField[brickId]->strength == 0){
		UserPaddle.score += (brickField[brickId]->type * 1000) / time;  
		brickCount--; 
		//0 - none, 1 - plusLife, 2 - Widen, 3 - No Bounds
		if(!ExtraLiveP.active || !WidenP.active || !NoBoundP.active){
			if(brickField[brickId]->pUp == 1){
					ExtraLiveP.active = 1; 
					ExtraLiveP.x = 64 - ExtraLiveP.w/2; 
					ExtraLiveP.y = 80 + ExtraLiveP.h/2; 
			}
			if(brickField[brickId]->pUp == 2){
					WidenP.active = 1; 
					WidenP.x = 64 - WidenP.w/2; 
					WidenP.y = 80 + WidenP.h/2; 
			}
			if(brickField[brickId]->pUp == 3){
					NoBoundP.active = 1; 
					NoBoundP.x = 64 - NoBoundP.w/2; 
					NoBoundP.y = 80 + NoBoundP.h/2;
			}
		}
	}
}

void paddleCollision(){
	//TOP PADDLE COLLISION
	if((ball.x + ball.w >= UserPaddle.x && ball.x <= UserPaddle.x + UserPaddle.w) && ball.y >=  UserPaddle.y - UserPaddle.h && shootStage == 0){
		ball.yvel *= -1; 
		if((ball.x + ball.w >= UserPaddle.x && ball.x + ball.w/2 <= UserPaddle.x + UserPaddle.w/2))
			ball.xvel = -1;
		if((ball.x + ball.w >= UserPaddle.x && ball.x + ball.w/2 >= UserPaddle.x + UserPaddle.w/2))
			ball.xvel = 1;
	}
	
	//SIDE PADDLE COLLISION
	if((ball.y <= UserPaddle.y) && (ball.y > UserPaddle.y - UserPaddle.h) && (((ball.x == UserPaddle.x + UserPaddle.w) && ball.xvel == -1) || ((ball.x + ball.w == UserPaddle.x) && ball.xvel == 1)))
		ball.xvel *= -1; 
	
	//LOST BALL COLLISIOn
	if(ball.y == UserPaddle.y && noBoundEffect == 0 && shootStage == 0){
		playsound(lose); 
		ball.xvel = ball.yvel = 0; 
		UserPaddle.lives--; 
		if(UserPaddle.lives > 0)
			resetBall = 1; 
		else{
			Timer1A_Stop();
			Timer3A_Stop(); 
		}
	}
}

void wallCollision(){
	//Wall Collisions 
	if(ball.x < 0)
		ball.x = 0; 
	
	if(ball.x + ball.w > 127)
		ball.x = 127 - ball.w; 
	
	if(ball.x + ball.w >= 127 || ball.x <= 0 ){
		ball.xvel *= -1; 
		playsound(hit); 
	}
	
	if(ball.y >= 159 || ball.y - ball.h <= 10){
		ball.yvel *= -1; 
		playsound(hit); 
	}
}

void powerUpCollision(){
	if(WidenP.x + WidenP.w >= UserPaddle.x && WidenP.x <= UserPaddle.x + UserPaddle.w && WidenP.y >= UserPaddle.y - UserPaddle.h && WidenP.active){
		widenEffect = 1;
		widenTimer = 2; 
		WidenP.active = 0; 
		UserPaddle.w = 64; 
		UserPaddle.image  = LongPaddle; 
		ST7735_FillRect(WidenP.x, WidenP.y - WidenP.h, WidenP.w, WidenP.h, ST7735_WHITE); 
	}
	if(WidenP.y >= UserPaddle.y && WidenP.active){
		WidenP.active = 0; 
		ST7735_FillRect(WidenP.x, WidenP.y - WidenP.h, WidenP.w, WidenP.h, ST7735_WHITE);
	}
	
	if(NoBoundP.x + NoBoundP.w >= UserPaddle.x && NoBoundP.x <= UserPaddle.x + UserPaddle.w && NoBoundP.y >= UserPaddle.y - UserPaddle.h && NoBoundP.active){ //Incomplete
		noBoundEffect = 1; 
		noBoundTimer = 5; 
		NoBoundP.active = 0; 
		ST7735_FillRect(NoBoundP.x, NoBoundP.y - NoBoundP.h, NoBoundP.w, NoBoundP.h, ST7735_WHITE); 
	}
	if(NoBoundP.y >= UserPaddle.y && NoBoundP.active){
		NoBoundP.active = 0; 
		ST7735_FillRect(NoBoundP.x, NoBoundP.y - NoBoundP.h, NoBoundP.w, NoBoundP.h, ST7735_WHITE); 
	}
	
	if(ExtraLiveP.x + ExtraLiveP.w >= UserPaddle.x && ExtraLiveP.x <= UserPaddle.x + UserPaddle.w && ExtraLiveP.y >= UserPaddle.y - UserPaddle.h && ExtraLiveP.active){ //Complete
		ExtraLiveP.active = 0; 
		UserPaddle.lives++; 
		ST7735_FillRect(ExtraLiveP.x, ExtraLiveP.y - ExtraLiveP.h, ExtraLiveP.w,ExtraLiveP.h, ST7735_WHITE); 
	}
	if(ExtraLiveP.y >= UserPaddle.y && ExtraLiveP.active){
		ExtraLiveP.active = 0;  
		ST7735_FillRect(ExtraLiveP.x, ExtraLiveP.y - ExtraLiveP.h, ExtraLiveP.w,ExtraLiveP.h, ST7735_WHITE); 
	}	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//MOVEMENT FUNCTIONS--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Move(void){
	if(pauseFlag == 0){
		uint32_t newPaddlePos = Convert(ADCMail); 
		
	//Paddle Movement -> TO BE MOVED TO DIFFERENT ISR
	if(UserPaddle.x < newPaddlePos){ 
		UserPaddle.x += 1;
		UserPaddle.moved = True; 
	}
	if(UserPaddle.x > newPaddlePos){
		UserPaddle.x -= 1; 
		UserPaddle.moved = True; 
	}
	
	//Ball Movement
	ball.x += ball.xvel; 
	ball.y += ball.yvel;
	
	if(WidenP.active)
		WidenP.y += WidenP.yvel; 
	if(ExtraLiveP.active)
		ExtraLiveP.y += ExtraLiveP.yvel; 
	if(NoBoundP.active)
		NoBoundP.y += NoBoundP.yvel; 
	
	powerUpCollision(); 
	
	//Wall Collisions
	wallCollision(); 
	
	//Paddle Collisions
	paddleCollision();
	
	//Brick Collisions
	brickCollision(); 
	
	
	ball.moved = True; 
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//GAMEPLAY MANAGEMENT--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void resetBallpos(){
	ST7735_SetCursor(19, 0); 
	ST7735_OutUDec(UserPaddle.lives);
	if(resetBall == 1){
			ST7735_FillRect(ball.x, ball.y - ball.h, ball.w,ball.h, ST7735_WHITE); 
			ball.x = (UserPaddle.x + UserPaddle.w/2) - ball.w/2; 
			ball.y = UserPaddle.y - UserPaddle.h; 
			ball.moved = True; 
			DrawBall(ball);
			resetBall = 0; 
			shootStage = 1; 
		}
}

void isShootStage(){
	while(shootStage == 1 && restartFlag == 0) {
		ST7735_DrawBitmap((UserPaddle.x + UserPaddle.w/2) - ball.w/2, UserPaddle.y - UserPaddle.h, ball.image, ball.w, ball.h); //Ball Follows Paddle
		DrawPaddle(UserPaddle);
		
		ball.x = (UserPaddle.x + UserPaddle.w/2) - ball.w/2;
		ball.y = UserPaddle.y - UserPaddle.h; 
		
		ST7735_SetCursor(7,0); 
		ST7735_OutUDec(time); 
		
		uint32_t button = Button_In(); 
		if(button == 0x04){ //Shoot Left
			shootStage = 0; 
			ball.yvel = -1; 
			ball.xvel = -1; 
			ball.moved = True; 
		}
		if(button == 0x08){ //Shot Right
			shootStage = 0; 
			ball.yvel = -1; 
			ball.xvel = 1; 
			ball.moved = True; 
		}
	}
}

void isBallMoved(){
	if(ball.moved){
		DrawBall(ball); 
		ball.moved = False; 
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//MAIN GAMEPLAY--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int main(){
restart: 
	DisableInterrupts();
	
	//Physical Initializations
	pauseFlag = 0; 
	restartFlag = 0;
	widenEffect = 0;
	noBoundEffect = 0; 
	shootStage = 1; 
	time = 0; 
	resetBall = 0;  
	TExaS_Init(NONE);
	Random_Init(13);
	Output_Init();
	ADC_Init(); 
	DAC_Init(); 
	Button_Init(); 
	brickCount = 0; 
	impbrickCount = 0; 
	for(uint8_t i = 0; i < BRICKS; i++){
		if(brickField[i]->type == 127)
			impbrickCount++; 
		else
			brickCount++; 
	}
	
	//Game Object Intializations
	ball.x = 59;
	ball.y = 155;
	UserPaddle.x = 48;
	UserPaddle.y = 159; 
	UserPaddle.lives = 3;
	UserPaddle.score = 0; 
	
	//Timer Initializations
	Timer1A_Init(&Move, 80000000/35, 3); //35 for real world, 200 for sim
	Timer3A_Init(80000000/10, 2); 
	
	//Edge-Interrupts
	PauseButton_Init();
	RestartButton_Init(); 
	
	//Title Initializations
	ST7735_FillScreen(ST7735_BLACK); 
	ST7735_DrawBitmap(2,48, Title, 123,45);
	ST7735_DrawBitmap(10, 92, ChooseYourLang, 110, 37); 
	ST7735_DrawBitmap(17, 122, LB, 26, 14);
	ST7735_DrawBitmap(82, 122, RB, 29, 14);
	ST7735_DrawBitmap(13, 142, USAFlag, 32, 16);
	ST7735_DrawBitmap(79, 142, MexicoFlag, 32, 16); 
	
	//Language Initializations
	LangSelect(); 
	DelayMs(500); 
	
	//Game Screen Initiallization 
	ST7735_FillScreen(ST7735_WHITE);
	ST7735_FillRect(0, 0, 128, 10, ST7735_BLACK);
	ST7735_SetCursor(0, 0); 
	ST7735_OutString(Phrases[TIME][myLanguage]); 
	ST7735_OutUDec(time); 
	ST7735_SetCursor(13,0); 
	ST7735_OutString(Phrases[LIVES][myLanguage]); 
	ST7735_OutUDec(UserPaddle.lives); 
	
	//Draw Ball and Paddle
	DrawBall(ball); 
	DrawPaddle(UserPaddle); 
	
	//Give Original Health Back To All Bricks and Draw Them
	for(uint8_t i = 0; i < BRICKS; i++){
		brickField[i]->strength = brickField[i]->type; 
		DrawBrick(*brickField[i]); 
	}

	EnableInterrupts(); 
	
	while(1){
		
		while(pauseFlag == 1 && restartFlag == 0); 
		
		if(restartFlag == 1){
			DisableInterrupts();
			ST7735_FillScreen(ST7735_BLACK); 
			goto restart; 
		}
		
		//Text Based Game Elements
		ST7735_SetCursor(7,0); 
		ST7735_OutUDec(time); 
		ST7735_SetCursor(19, 0); 
		ST7735_OutUDec(UserPaddle.lives); 
		
		//In-progress Gameplay
		if(UserPaddle.lives > 0){
				//Initial Game State/Lost Life 
				resetBallpos(); 
				isShootStage(); 
				
				//Movement Based Game Elements
				if(WidenP.active){
					DrawPower(WidenP);
					isBallMoved(); 
				}
				if(ExtraLiveP.active){
					DrawPower(ExtraLiveP);
					isBallMoved(); 
				}
				if(NoBoundP.active){
					DrawPower(NoBoundP); 
					isBallMoved(); 
				}
				isBallMoved(); 
				
				if(brickId != -1){
					DrawBrick(*brickField[brickId]);
					brickId = -1; 
				}
				
				if(UserPaddle.moved){
					DrawPaddle(UserPaddle); 
					UserPaddle.moved = False; 
				}
			}
		
			
		//End-Game
		if(UserPaddle.lives == 0 || brickCount == 0){
			ST7735_FillScreen(ST7735_BLACK);
			
			if(UserPaddle.lives == 0){
				//Handle Lose Screen 
				ST7735_SetCursor(6,6);
				ST7735_OutString(Phrases[LOSE][myLanguage]); 
				while(restartFlag == 0); 
			}
			
			if(brickCount == 0){
				//Handle win screen and display score
				playsound(victory); 
				ST7735_SetCursor(6,6);
				ST7735_OutString(Phrases[WIN][myLanguage]);
				ST7735_SetCursor(6,9);
				ST7735_OutString(Phrases[SCORE][myLanguage]);
				ST7735_OutUDec(UserPaddle.score); 
				while(restartFlag == 0); 
			}
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
