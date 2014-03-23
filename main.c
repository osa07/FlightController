/*
 * ControllerBoard.c
 *
 * Created: 05.03.2014 09:56:24
 *  Author: TOPRIS3
 */ 
#include "sam4s16c.h"
#include "InertialBoard.h"
#include "MotorsDriver.h"
#include "Receiver.h"

/**
 * \brief Application entry point.
 *
 * \return Unused (ANSI-C compatibility).
 */

#define DISABLEWATCHDOG 0x00008000;

void UART1_Handler(void)
{
	uint32_t rx_yaw, newpos;
	// receiver interrupt - from here it will be interesting :)
	
	rx_yaw=UART1->UART_RHR;
	if ((rx_yaw&0x000000F0)==0x00000000)
		newpos=(rx_yaw&0x0000000F)<<0x04;
	else{
		if((rx_yaw&0x000000F0)==0x00000010){
			newpos|=(rx_yaw&0x0000000F);
			
			
			/* newpos=0    -> Left =  0x75
			   newpos=0x7F -> Midle = 0xB0                                                                    
			   newpos=0xFF -> Right = 0xEA
			*/
			newpos=newpos*458/1000+117;	// 117 is the offset of the range 117 - 234 (without offset=0-117) 0,458 is the scale factor between the 0-255 and the 0-117 ranges
			MD_UpdateTailServo(newpos);
		}
		else{
			// ignore for the moment
			newpos=0x00000000;
		}
	}
	
	
}
void PIOA_Handler(void)
{
	// button timer interrupt
	
	uint8_t servopositions[0x03]={0x0000007A,0x000000B0,0x000000E5};
	static uint8_t index=0x00;
	uint32_t reg;
	
	reg=PIOA->PIO_ISR;
	reg=(PIOC->PIO_ODSR & 0x00000400);
	if(reg==0x0000400)
		PIOC->PIO_CODR=0x00000400;					// switch LED0 off
	else
		PIOC->PIO_SODR=0x00000400;					// switch LED0 on
	
	if(index>0x02){
		index=0x00;
	}
	MD_UpdateTailServo(servopositions[index]);		
	index++;
}
void TC0_Handler(void)
{
	static uint32_t reg;
	
	reg=TC0->TC_CHANNEL[0].TC_SR;	
	TC0->TC_CHANNEL[0].TC_RC	= 0x00004E20;	// set for 20ms
	
	reg=(PIOC->PIO_ODSR & 0x00000400);
	if(reg==0x0000400)
		PIOC->PIO_CODR=0x00000400;					// switch LED0 off
	else
		PIOC->PIO_SODR=0x00000400;					// switch LED0 on
}
int main(void)
{
    uint32_t temp;
	
	SystemInit();		/* Initialize the SAM system  - system frequency */
	WDT->WDT_MR=DISABLEWATCHDOG;
	
	/* Initialize Pins
	
	J1:
	1 - PA3 - TWD0
	2 - PA4 - TWCK0
	3 - PB2 - UART Rx
	4 - PB3 - UART Tx
	5 - PA31 - Interrupt from IMU-3000
	
	J2:
	-
	
	J3:
	2 - PA11 - PWMH0
	4 - PA18 - Inertialboard power supply control
	
	J4:
	6 - PA13 - PWMH2 =
	7 - PA12 - PWMH1 =
	8 - PA14 - PWMH3 = for Tail Servo */
	
	// Initialize LED0 - PC10
	PIOC->PIO_PER=0x00000400;
	PIOC->PIO_OER=0x00000400;
	PIOC->PIO_SODR=0x00000400;					// switch LED0 off
	PIOC->PIO_PUDR=0x00000400;					// disable PULL-UP on pin
	PIOC->PIO_OWER=0x00020400;
	
	// Initialize PWM3 - PA14
	PIOA->PIO_IDR=0x00004000;					// PA14 is PWM3
	PIOA->PIO_ABCDSR[0]=0x00004000;				// Peripheral B
	PIOA->PIO_ABCDSR[1]=0x00000000;
	PIOA->PIO_PDR=0x000004000;
	PIOA->PIO_PUER=0x00004000;
	
	// Initialize Button - PA5
	PIOA->PIO_IDR|=0x00000020;					// disable interrupt for PA5
	PMC->PMC_PCER0|=0x00000800;					// enable PIOA clock
	PIOA->PIO_PER|=0x00000020;					// PA5 controlled by PIO
	PIOA->PIO_ODR|=0x00000020;					// disables output on PA5
	PIOA->PIO_PPDDR=0xFFFFFFFF;					// disables all pull-downs
	PIOA->PIO_PUER|=0x00000020;					// enable pull up on PA5
	
	PIOA->PIO_IFSCER=0x00000020;				// enable debounce filter on PA5
	PIOA->PIO_SCDR=0x00000682;					// 200ms debounce filter
	PIOA->PIO_IFER=0x00000020;					// enable glitch filter
	
	PIOA->PIO_AIMER=0x00000020;					// enable additional interrupt
	PIOA->PIO_REHLSR=0x00000020;				// rising edge
	PIOA->PIO_ESR=0x00000020;					// edge driven
	PIOA->PIO_LSR=0x00000000;					
	temp=PIOA->PIO_ISR;							// clear interrupt requests 
	PIOA->PIO_IER=0x00000020;					// enable interrupt for PA5
	
	// Initialize counter module
	PMC->PMC_PCER0				|=0x00800000;	// Enable TC0 clock
	TC0->TC_CHANNEL[0].TC_CMR	|= 0x0000C003;	// timer clock=MCK/128, Capture mode disable, WAVESEL=10
	TC0->TC_CHANNEL[0].TC_CCR	= 0x00000005;	// enables the clock
	TC0->TC_CHANNEL[0].TC_RC	= 0x00004E20;	// set for 20ms
	//TC0->TC_CHANNEL[0].TC_IDR	=0xFFFFFFFF;
	TC0->TC_CHANNEL[0].TC_IER	= 0x00000010;	// enable RC compare interrupt
	
	NVIC_EnableIRQ(TC0_IRQn);					// enable TC0 interrupt
	NVIC_EnableIRQ(PIOA_IRQn);					// enable PIOA interrupt
	
	MD_InitMotors();			// Initialize the PWMs, switch main motors OFF, position tail servo in the middle
	//IB_InitBoard();			// initialize the inertial board
	//RC_InitReceiver(19200);
	//RC_StartReceiver();
	//NVIC_EnableIRQ(9);
	// init PDC for UART Rx
	
    while (1) 
    {
		uint32_t temp;
		temp=0x00000000;
		do 
		{
			do 
			{
				temp++;
			} while (temp<1000);
			temp=0;
		} while (temp!=1);
    }
}
