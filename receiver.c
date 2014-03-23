/*
 * Receiver.c
 *
 * Created: 05.03.2014 10:37:32
 *  Author: TOPRIS3
 */ 

#include "sam4s16c.h"

void RC_InitReceiver(uint32_t baudrate)
{
	uint32_t CD;
	CD=(12000000/baudrate*16);		//BaudRate=MCK/(CD*16), MCK=12000000, BaudRate=19200 => CD=0x0027;
	
	UART1->UART_BRGR= CD;	
}
void RC_StartReceiver(void)
{
	uint32_t uart_pmc;
	
	uart_pmc		= PMC->PMC_PCSR0;
	PMC->PMC_PCER0	= uart_pmc|(0x00000100);	// Enable UART0 Clock
	PMC->PMC_PCDR0	= 0x00000000;
	
	UART1->UART_CR	= 0x00000090;	// Enable Receiver	
}
void RC_StopReceiver(void)
{
	uint32_t uart_pmc;
	
	uart_pmc		= PMC->PMC_PCSR0;
	PMC->PMC_PCER0	= uart_pmc&(0xFFFFFEFF);	// Disable UART0 Clock
	PMC->PMC_PCDR0	= uart_pmc|(0x00000100);
	
	UART1->UART_CR	= 0x000000A0;	// Disables Receiver	
}