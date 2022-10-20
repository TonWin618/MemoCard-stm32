#include "stm32f10x.h"
#include "delay.h"
#include "Flash.h"
#include "Key.h"
#include "lcd_init.h"
#include "lcd.h"
#include "BlueTooth.h"
#include "StoreSystem.h"
#include "OperatingSystem.h"

extern KeyNum key;//��ǰ���µİ�����ֵ
extern OperatingStatus status;//��ǰ�����״̬
extern uint8_t bleIsConnected;

//����ʱ����ִ�еĳ�ʼ������
void init()
{
	delay_init();
	
	LCD_Init();
	Flash_Init();
	BlueTooth_Init();
	Flash_Wakeup();
	Key_Init(); 
}

int main(void)
{
	init();//��ʼ��
	Action_DeveloperMode();
	//Action_LockMain();//������������
	while(1)
	{
		if(key != KEY_UNDO)
		{
			switch(key)
			{
				case KEY_UP:
					switch(status)
					{
						case Lock_Main:
							Action_BlueToothMain();
							break;
						case Card_Back:
							Action_LockMain();
							break;
						case Card_Face:
							Action_LockMain();
							break;
						case Card_End:
							Action_LockMain();
							break;
						default:
							break;
					}
					break;

				case KEY_DOWN:
					switch(status)
					{
						case Lock_Main:
							Action_CardFace(0);
							break;
						case Card_Back:
							Action_CardFace(2);
							break;
						case BlueTooth_Main:
							Action_LockMain();
						default:
							break;
					}
					break;

				case KEY_LEFT:
					switch(status)
					{
						case Card_Back:
							Action_CardFace(1);
							break;
						case BlueTooth_Transmit:
							Action_BlueToothMain();
							break;
						case Lock_Main:
							Action_DeveloperMode();
							break;
						case Settings:
							Action_LockMain();
							break;
						default:
							break;
					}
					break;
					
				case KEY_RIGHT:
					switch(status)
					{
						case Card_Back:
							Action_CardFace(3);
							break;
						case BlueTooth_Transmit:
							Action_BlueToothTransmit();
							break;
						case Lock_Main:
							Action_Settings();
							break;
						case DeveloperMode:
							Action_LockMain();
							break;
						default:
							break;
					}
					break;
				case KEY_MIDDLE:
					switch(status)
					{
						case Card_Face:
							Action_CardBack();
							break;
						case BlueTooth_Main:
							Action_BlueToothTransmit();
							break;
						case DeveloperMode:
							Store_TestInitAll();
							break;
						default:
							break;
					}
					break;
				case KEY_SET:
					Action_CardFace(4);
					break;
				case KEY_RESET:
					Action_CardFace(5);
					break;
				default:
					break;
			}
			key = KEY_UNDO;
		}
	}
}
