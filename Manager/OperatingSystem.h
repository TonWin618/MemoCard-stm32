#ifndef __OPERATINGSYSTEM_H
#define __OPERATINGSYSTEM_H

#include <stm32f10x.h>

typedef enum{
	Lock_Main=0,Card_Face=1,Card_Back=2,Card_End=3,BlueTooth_Main=4,BlueTooth_Transmit=5,BlueTooth_End=6,Settings=7,DeveloperMode=8,Initialize=9
}OperatingStatus;//²Ù×÷½çÃæ×´Ì¬

void Action_LockMain(void);
void Action_CardFace(uint8_t command);
void Action_CardBack(void);
void Action_CardEnd(void);
void Action_BlueToothMain(void);
void Action_BlueToothTransmit(void);
void Action_BlueToothEnd(void);
void Action_Settings(void);
void Action_DeveloperMode(void);
void EXTI9_5_IRQHandler(void);

#endif
