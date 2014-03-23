/*
 * MotorsDriver.h
 *
 * Created: 05.03.2014 10:28:29
 *  Author: TOPRIS3
 */ 


#ifndef MOTORSDRIVER_H_
#define MOTORSDRIVER_H_

void MD_InitMotors(void);
void MD_UpdateFL(int speed);
void MD_UpdateFR(int speed);
void MD_UpdateTail(int speed);
void MD_UpdateTailServo(uint8_t newpos);


#endif /* MOTORSDRIVER_H_ */