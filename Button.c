#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

void PauseButton_Init(){
	GPIO_PORTE_DIR_R &= ~0x02;    	
  GPIO_PORTE_AFSEL_R &= ~0x02;  
  GPIO_PORTE_DEN_R |= 0x02;   
  GPIO_PORTE_PCTL_R &= ~(0xF << 4);
	GPIO_PORTE_AMSEL_R &= ~0x02; 
	GPIO_PORTE_IS_R &= ~0x02;     
	GPIO_PORTE_IBE_R &= ~0x02;
	GPIO_PORTE_IEV_R &= ~0x02;  
	GPIO_PORTE_ICR_R = 0x02;      
	GPIO_PORTE_IM_R |= 0x02;  
	NVIC_PRI1_R = (NVIC_PRI1_R&0xFFFFFF0F)|0x000000E0; 
	NVIC_EN0_R = 0x00000010;      		
}

void RestartButton_Init(){
	GPIO_PORTE_DIR_R &= ~0x01;    	
  GPIO_PORTE_AFSEL_R &= ~0x01;  
  GPIO_PORTE_DEN_R |= 0x01;   
  GPIO_PORTE_PCTL_R &= ~0xF;
	GPIO_PORTE_AMSEL_R &= ~0x01; 
	GPIO_PORTE_IS_R &= ~0x01;     
	GPIO_PORTE_IBE_R &= ~0x01;
	GPIO_PORTE_IEV_R &= ~0x01;  
	GPIO_PORTE_ICR_R = 0x01;      
	GPIO_PORTE_IM_R |= 0x01;   		
}

uint8_t restartFlag; 
uint8_t pauseFlag; 
void GPIOPortE_Handler(void){
	if(GPIO_PORTE_RIS_R & 0x01){ //Interrupt from restart
		GPIO_PORTE_ICR_R |= 0x01;
		restartFlag = 1; 
	}
		
	if(GPIO_PORTE_RIS_R & 0x02){ //Interrupt from pause
	GPIO_PORTE_ICR_R |= 0x02; 
	//Handle pausing
	pauseFlag ^= 1; 
	}
}