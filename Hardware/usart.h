#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"

void uart_init(u32 bound);
void USART1_IRQHandler(void);

#endif
