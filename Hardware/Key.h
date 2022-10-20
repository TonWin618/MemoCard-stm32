#ifndef __KEY_H
#define __KEY_H

typedef enum
{
	KEY_UP = 0, KEY_DOWN = 1, KEY_LEFT = 2, KEY_RIGHT = 3, KEY_MIDDLE = 4, KEY_SET = 5, KEY_RESET = 6, KEY_UNDO = 7
}KeyNum;//按键所对应键值

void Key_Init(void);
void TIM2_IRQHandler(void);

#endif
