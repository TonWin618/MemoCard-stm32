#include "stm32f10x.h"
#include "Key.h"

#define KEY_TOTAL 7	//按键总数

uint8_t key = KEY_UNDO;		//默认状态
uint8_t k = 7;
uint8_t kLast = 7;
uint8_t kTime = 0;

void Key_Init()
{
	//配置定时器中断，5ms进入一次中断
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_InternalClockConfig(TIM2);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 10000 - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 -1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	
	//配置NVIC，2抢占优先级，1从优先级
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM2, ENABLE);
	
	//初始化五向按键模块所接的7个引脚
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU          ;
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

//TIM2中断函数
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)//检查是否发生中断
	{
		uint8_t i;
		k = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5) + GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_6)*2 + GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7)*4;
		if(k == KEY_UNDO)
		{
			kTime = 0;
			kLast = KEY_UNDO;
		}
		else
		{
			if(k==kLast)
			{
				kTime++;
			}
			else
			{
				kTime = 1;
				kLast = k;
			}
		}
		
		if(kTime == 4)
		{
			key = k;
			kTime = 0;
		}
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);//清除中断标志位
	}
}

