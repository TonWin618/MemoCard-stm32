#include <stm32f10x.h>
#include "OperatingSystem.h"
#include "StoreSystem.h"
#include "delay.h"
#include "Key.h"
#include "lcd.h"
#include "lcd_init.h"
#include "Flash.h"
#include "BlueTooth.h"
#include "ShowChinese.h"
#include "pic.h"

OperatingStatus status = Lock_Main;//Ĭ��״̬Ϊ����������

extern KeyNum key;//����Key�еļ�ֵ
extern uint8_t k;
extern struct Card currentCard;//����StoreSystem�е�currentCard
uint8_t bleIsConnected;//�����Ƿ�����

typedef enum{
	SET_MAIN,SET_NEXTROUND0,SET_NEXTROUND1,SET_DAYCEILING0,SET_DAYCEILING1
}SetStatus;
SetStatus setStatus;//���ý���״̬

void Set_DAYCEILING0(void);
void Set_DAYCEILING1(void);
void Set_NEXTROUND0(void);
void Set_NEXTROUND1(void);

//����������
void Action_LockMain()
{
	LCD_Fill(0,0,LCD_W,LCD_H,CYAN);
	LCD_ShowString(8,8,"HELLO",DARKGRAY,LIGHTGRAY,48,1);
	LCD_ShowString(8,50,"TONWIN",WHITE,LIGHTGRAY,48,1);
	LCD_ShowString(10,138,"CARD",WHITE,LIGHTGRAY,32,1);
	Draw_Triangle(80,143,WHITE,3);
	LCD_ShowString(262,8,"BLE",WHITE,LIGHTGRAY,32,1);
	Draw_Triangle(230,13,WHITE,2);
	LCD_ShowString(262,138,"SET",WHITE,LIGHTGRAY,32,1);
	Draw_Triangle(230,143,WHITE,0);
	status =Lock_Main;
}
//��Ƭ����չʾ����
void Action_CardFace(uint8_t command)
{
	if(Store_GetDayCeiling()==0)
	{
		return;
	}
	status = Card_Face;
	//����һ�Ž��еĲ���
	switch(command)
	{
		//��LockMain����
		case 0:
		//��δ��ʼ����г�ʼ��
			if(Store_GetDayStatus() == 0)
			{
				Store_CardPoolInit();
			}
			else
			{
				Store_CardReload();
			}
			break;
		//���������
		case 1:
			Store_ChangeCard(1);
			Store_SwitchCard();
			break;
		//���¼�����
		case 2:
			Store_ChangeCard(2);
			Store_SwitchCard();
			break;
		//���Ҽ�����
		case 3:
			Store_ChangeCard(3);
			Store_SwitchCard();
			break;
		//��SET������
		case 4:
			Store_ChangeCard(4);
			Store_SwitchCard();
			break;
		//��RESET������
		case 5:
			Store_ChangeCard(5);
			Store_SwitchCard();
			break;
		default:
			break;
	}
	//�ж��Ƿ���������
	if(Store_GetDayStatus() == 2)
	{
		Action_CardEnd();
		status = Card_End;
		return;
	}
	//�Ϸ������������Ϸ�������ʾ
	uint16_t countFinished;
	uint16_t countCeiling;
	countFinished = Store_GetDayFinish();
	countCeiling = Store_GetDayCeiling();
	uint16_t FinishWidth = LCD_W*countFinished/countCeiling;
	LCD_Fill(0, 0, FinishWidth+1, 30, CYAN);
	LCD_Fill(FinishWidth+1, 0, LCD_W, 30, GRAY);
	LCD_Fill(0, 30, LCD_W, LCD_H, LIGHTGRAY);
	LCD_ShowIntNum(230,4,countFinished,3,DARKGRAY,GRAY,24);
	LCD_ShowChar(270,1,'/',DARKGRAY,GRAY,24,1);
	LCD_ShowIntNum(280,4,countCeiling,3,DARKGRAY,GRAY,24);
	//չʾ��������
	//���ݳ��ȷֲ�ѡ�������С
	LCD_ShowString_Center(40,currentCard.face,DARKGRAY,LIGHTGRAY,48,1);
	//չʾ�����·�֪��/ģ��/��������
	uint16_t count_all = currentCard.count_know+currentCard.count_hazy+currentCard.count_forget;
	LCD_Fill(0,115,280*currentCard.count_know/count_all+1,134,CYAN);
	LCD_ShowIntNum(280*currentCard.count_know/count_all,111,currentCard.count_know,2,DARKGRAY,LIGHTGRAY,24);
	LCD_Fill(0,134,280*currentCard.count_hazy/count_all+1,153,YELLOW);
	LCD_ShowIntNum(280*currentCard.count_hazy/count_all,131,currentCard.count_hazy,2,DARKGRAY,LIGHTGRAY,24);
	LCD_Fill(0,153,280*currentCard.count_forget/count_all+1,172,RED);
	LCD_ShowIntNum(280*currentCard.count_forget/count_all,153,currentCard.count_forget,2,DARKGRAY,LIGHTGRAY,24);
	//״̬�л�
	
}

//��Ƭ����չʾ����
void Action_CardBack()
{
	uint16_t countFinished;
	uint16_t countCeiling;
	countFinished = Store_GetDayFinish();
	countCeiling = Store_GetDayCeiling();
	uint16_t FinishWidth = LCD_W*countFinished/countCeiling;
	LCD_Fill(0, 0, FinishWidth+1, 30, CYAN);
	LCD_Fill(FinishWidth+1, 0, LCD_W, 30, GRAY);
	LCD_Fill(0, 30, LCD_W, LCD_H, LIGHTGRAY);
	LCD_ShowIntNum(230,4,countFinished,3,DARKGRAY,GRAY,24);
	LCD_ShowChar(270,1,'/',DARKGRAY,GRAY,24,1);
	LCD_ShowIntNum(280,4,countCeiling,3,DARKGRAY,GRAY,24);
	LCD_Fill(0,30,LCD_W,LCD_H,LIGHTGRAY);
	
	LCD_Fill(0,140,107,172,CYAN);
	LCD_ShowString(21,140,"know",LIGHTGRAY,LIGHTGRAY,32,1);
	LCD_Fill(107,140,213,172,YELLOW);
	LCD_ShowString(127,140,"hazy",LIGHTGRAY,LIGHTGRAY,32,1);
	LCD_Fill(213,140,320,172,RED);
	LCD_ShowString(218,140,"forget",LIGHTGRAY,LIGHTGRAY,32,1);
	ShowMixString(30,40,currentCard.back, DARKGRAY);
	status = Card_Back;
}

//��Ƭ��������
void Action_CardEnd()
{
	LCD_Fill(0, 0, LCD_W, LCD_H, LIGHTGRAY);
	LCD_ShowString_Center(50,"END",DARKGRAY,WHITE,32,0);
	status = Card_End;
}

//����������
void Action_BlueToothMain()
{
	LCD_Fill(0,0,LCD_W,LCD_H,BLUE);
	bleIsConnected = BlueTooth_IsConnected();
	LCD_ShowString(140,30,"BLUETOOTH<",WHITE,BLUE,32,0);
	//�����жϲ���ʾ�жϽ������Ļ��
	if(bleIsConnected == 1)
	{
		LCD_Fill(140, 85, 320, 132, BLUE);
		LCD_ShowString(140,85,"Connected",WHITE,BLUE,32,0);
	}
	else
	{
		LCD_Fill(140, 85, 320, 132, BLUE);
		LCD_ShowString(140,85,"Unconnected",WHITE,BLUE,32,0);
	}
	LCD_ShowIcon(10,22,128,128,gImage_ble,WHITE);
	//��Բ�����봫��
	status = BlueTooth_Main;
}

//�����������
void Action_BlueToothTransmit()
{
	if(bleIsConnected == 0)
	{
		return;
	}
	status = BlueTooth_Transmit;
	LCD_Fill(140, 85, 320, 172, BLUE);
	LCD_ShowString(140,85,"Transmit",WHITE,BLUE,32,0);
	Store_SetCard();
	Draw_Triangle(140,120,WHITE,0);
	LCD_ShowString(204,120,"NEXT",WHITE,WHITE,32,1);
	Draw_Triangle(140,144,WHITE,1);
	LCD_ShowString(204,144,"BACK",WHITE,WHITE,32,1);
}

//������������
void Action_BlueToothEnd()
{
	//���δ���ͳ��
	status = BlueTooth_End;
}

//���ý���
void Action_Settings()
{
	setStatus = SET_NEXTROUND0;
	uint16_t value = Store_GetDayCeiling();
	//����������
	LCD_Fill(0,0,LCD_W,LCD_H,BLACK);
	LCD_ShowString_Center(3,"SETTINGS",WHITE,BLACK,32,1);
	LCD_DrawLine(0,40,LCD_W,40,BRIGHTRED);
	//NextRound����
	LCD_Fill(80,60,240,96,BRIGHTRED);
	LCD_ShowString_Center(61,"NextRound",WHITE,BLACK,32,1);
	//DayCeiling����
	LCD_ShowString_Center(100,"DayCeiling",WHITE,BLACK,32,1);
	Draw_Triangle(95,142,WHITE,1);
	LCD_Fill(125,138,195,170,WHITE);
	Draw_Triangle(200,142,WHITE,0);
	LCD_ShowIntNum(128,138,Store_GetDayCeiling(),4,BLACK,BLACK,32);
	
	while(1)
	{
		switch(key)
		{
			case KEY_MIDDLE:
				switch(setStatus)
				{
					case SET_DAYCEILING0:
						setStatus =	SET_DAYCEILING1;//����Ķ���ֵ
						break;
					case SET_NEXTROUND0:
						Store_TestInit();
						setStatus =	SET_NEXTROUND1;//�������ٴγ�ʼ��
						break;
					default:
						break;
				}
				break;
			case KEY_UP:
				switch(setStatus)
				{
					case SET_DAYCEILING1:
						if(Store_GetDayStatus() != 1)
						{
							Store_ChangeDayCeiling(value);
						}
							
					case SET_DAYCEILING0:
						Set_DAYCEILING0();
						Set_NEXTROUND1();
						setStatus = SET_NEXTROUND0;
						break;
					default:
						break;
				}
				break;
			case KEY_DOWN:
				switch(setStatus)
				{
					case SET_NEXTROUND1:
					case SET_NEXTROUND0:
						Set_NEXTROUND0();
						Set_DAYCEILING1();
						setStatus = SET_DAYCEILING0;
						break;
					default:
						break;
				}
				break;
			case KEY_LEFT:
				switch(setStatus)
				{
					case SET_DAYCEILING1:
						if(value > 1)
							value--;
						LCD_Fill(125,138,195,170,BRIGHTRED);
						LCD_ShowIntNum(128,138,value,4,WHITE,BLACK,32);
						delay_ms(300);
						break;
					default:
						Action_LockMain();
						status = Lock_Main;
						return;
				}
				break;
			case KEY_RIGHT:
				switch(setStatus)
				{
					case SET_DAYCEILING1:
						if(value < Store_GetCardTotal())
							value++;
						LCD_Fill(125,138,195,170,BRIGHTRED);
						LCD_ShowIntNum(128,138,value,4,WHITE,BLACK,32);
						delay_ms(300);
						break;
					default:
						break;
				}
				break;
			default:
						break;
		}
	}
}
//δѡ��ÿ������
void Set_DAYCEILING0()
{
	LCD_Fill(125,138,195,170,WHITE);
	LCD_ShowIntNum(128,138,Store_GetDayCeiling(),4,BLACK,BLACK,32);
}
//ѡ��ÿ������
void Set_DAYCEILING1()
{
	LCD_Fill(125,138,195,170,BRIGHTRED);
	LCD_ShowIntNum(128,138,Store_GetDayCeiling(),4,WHITE,BLACK,32);
}
//δѡ����һ��
void Set_NEXTROUND0()
{
	LCD_Fill(80,60,240,96,WHITE);
	LCD_ShowString_Center(61,"NextRound",BLACK,BLACK,32,1);
}
//ѡ����һ��
void Set_NEXTROUND1()
{
	LCD_Fill(80,60,240,96,BRIGHTRED);
	LCD_ShowString_Center(61,"NextRound",WHITE,BLACK,32,1);
}
//������ģʽ
void Action_DeveloperMode()
{
	LCD_Fill(0,0,LCD_W,LCD_H,BLACK);
	while(1){
	
	LCD_ShowIntNum(30,120,key,3,WHITE,DARKGRAY,24);
	LCD_ShowIntNum(0,120,k,3,WHITE,DARKGRAY,24);
	}
//	LCD_Fill(0,0,LCD_W,LCD_H,BLACK);
//	LCD_ShowIntNum(0,0,Store_GetDayStatus(),3,WHITE,DARKGRAY,24);
//	LCD_ShowIntNum(96,0,Store_GetDayCeiling(),5,WHITE,DARKGRAY,24);
//	LCD_ShowIntNum(240,0,Store_GetDayFinish(),5,WHITE,DARKGRAY,24);
//	
//	LCD_ShowIntNum(0,30,Store_GetCardTotal(),5,WHITE,DARKGRAY,24);
//	LCD_ShowIntNum(96,30,Store_GetCardIndex(),5,WHITE,DARKGRAY,24);
//	
//	u8 temp[3];
//	Flash_ReadSecurity(temp,0x003000,3);
//	LCD_ShowIntNum(0,60,temp[0],3,WHITE,DARKGRAY,24);
//	LCD_ShowIntNum(60,60,temp[1],3,WHITE,DARKGRAY,24);
//	LCD_ShowIntNum(120,60,temp[2],3,WHITE,DARKGRAY,24);
//	
//	u8 face[32];
//	Flash_ReadBuffer(face,0x001000,32);
//	LCD_ShowString(0,90,face,WHITE,DARKGRAY,24,0);
//	Flash_ReadBuffer(face,0x001068,32);
//	LCD_ShowString(60,90,face,WHITE,DARKGRAY,24,0);
//	Flash_ReadBuffer(face,0x0010D0,32);
//	LCD_ShowString(120,90,face,WHITE,DARKGRAY,24,0);
//	Flash_ReadBuffer(face,0x001138,32);
//	LCD_ShowString(180,90,face,WHITE,DARKGRAY,24,0);
//	
//	u8 num[1];
//	Flash_ReadBuffer(num,0x000002,1);
//	LCD_ShowIntNum(0,120,*num,3,WHITE,DARKGRAY,24);
//	Flash_ReadBuffer(num,0x000005,1);
//	LCD_ShowIntNum(60,120,*num,3,WHITE,DARKGRAY,24);
//	Flash_ReadBuffer(num,0x000008,1);
//	LCD_ShowIntNum(120,120,*num,3,WHITE,DARKGRAY,24);
//	Flash_ReadBuffer(num,0x00000B,1);
//	LCD_ShowIntNum(180,120,*num,3,WHITE,DARKGRAY,24);
//	
//	status = DeveloperMode;
}

//�����غ��½��ش�������BluetoothMain����ʵʱ��ʾ�����Ƿ�����
void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line11) != RESET)
	{
		if(status == BlueTooth_Main)
		{
			if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11))
			{
				LCD_Fill(140, 85, 320, 132, BLUE);
				LCD_ShowString(140,85,"Connected",WHITE,BLUE,32,0);
				bleIsConnected = 1;
			}
			else
			{
				LCD_Fill(140, 85, 320, 132, BLUE);
				LCD_ShowString(140,85,"Unconnected",WHITE,BLUE,32,0);
				bleIsConnected = 0;
			}
		}
		if(status == BlueTooth_Transmit)
		{
			Action_BlueToothMain();
		}
		EXTI_ClearITPendingBit(EXTI_Line11);
	}
	
}
