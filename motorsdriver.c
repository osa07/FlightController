/*
 * MotorsDriver.c
 *
 * Created: 05.03.2014 10:24:25
 *  Author: TOPRIS3
 */ 

#include "sam4s16c.h"

const uint32_t TAILSERVOPERIOD=0x00000928;		// PWM period=20ms
#define TAILSERVODEFDCYCLE	0x000000B0;

void MD_InitMotors(void)
{
	uint32_t pwm_pmc;
	//PA11 - PWMH0 = 
	//PA12 - PWMH1 = 
	//PA13 - PWMH2 = 
	//PA14 - PWMH3 = for Tail Servo	
	
	pwm_pmc				= PMC->PMC_PCSR0;
	PMC->PMC_PCER0		= pwm_pmc|(0x80000000);			//Enable PWM clock
	
	PWM->PWM_CH_NUM[3].PWM_CMR	= 0x0000020A;			// PWM clock=MCK/1024
	PWM->PWM_CH_NUM[3].PWM_CPRD	= TAILSERVOPERIOD;	
	PWM->PWM_CH_NUM[3].PWM_CDTY = TAILSERVODEFDCYCLE;	// PWM duty cycle=7,5% 
	PWM->PWM_ENA				= 0x00000008;			// enable PWM3 output
}

void MD_UpdateFL(int speed)
{
	
}
void MD_UpdateFR(int speed)
{
	
}
void MD_UpdateTail(int speed)
{
	
}
void MD_UpdateTailServo(uint8_t newpos)
{
	// min:    1ms(5%)=0x75
	// middle: 1,5ms(7,5%)=0xB0
	// max:	   2ms(10%)=0xEA;
	if ((newpos>=0x75)&&(newpos<=0xEA))
		PWM->PWM_CH_NUM[3].PWM_CDTYUPD=newpos;	
	else
		PWM->PWM_CH_NUM[3].PWM_CDTYUPD=TAILSERVODEFDCYCLE;	
}