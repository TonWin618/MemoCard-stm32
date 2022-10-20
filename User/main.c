#include "stm32f10x.h"
#include "delay.h"
#include "Flash.h"
#include "Key.h"
#include "lcd_init.h"
#include "lcd.h"
#include "BlueTooth.h"
#include "StoreSystem.h"
#include "OperatingSystem.h"

extern KeyNum key;//当前按下的按键键值
extern OperatingStatus status;//当前界面的状态
extern uint8_t bleIsConnected;

//启动时所需执行的初始化函数
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
	init();//初始化
	Action_DeveloperMode();
	//Action_LockMain();//进入锁屏界面
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
