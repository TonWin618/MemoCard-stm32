//��ģ�����ڶ�ȡFlashоƬ����¼�ĺ��ֲ���ʾ��LCD����
//32:���ִ�С��ռ32*32����
//130:һ�����ֵ������͵����ܴ�С��ռ130�ֽ�
//128:�����С��ռ128�ֽ�
//2:������С��ռ2�ֽ�
//0x80:��ѯ��ʼλ�ã����ִ洢�ȵ����ٺ���

#include "stm32f10x.h"
#include "lcd.h"
#include "Flash.h"
#include "lcd_init.h"

#define GBKBurnAddr 					0x700000
#define GBKCharacterTotal 		3755

typedef struct
{
	u8 msk[128];
	u8 index[2];
}Chinese;
//��ʾ��������
void ShowCharacter(u16 x, u16 y, u8 *s, u16 color)
{
	uint8_t i,j,x0,sizey;
	Chinese chinese;
	x0=x;
	sizey=32;
	u32 addr = 0x80;
	while(addr<GBKCharacterTotal*130)
	{
		Flash_ReadBuffer(chinese.index,addr+GBKBurnAddr,2);
		if(chinese.index[0]==*(s))
		{
			if(chinese.index[1]==*(s+1))
			{
				Flash_ReadBuffer(chinese.msk,addr+GBKBurnAddr-128,128);
				break;
			}
		}
		addr+=130;
	}

	LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
	for(i=0;i<32*4;i++)
	{
		for(j=0;j<8;j++)
		{
			if(chinese.msk[i]&(0x01<<j))
				LCD_DrawPoint(x,y,color);
			x++;
			if((x-x0)==sizey)
			{
				x=x0;
				y++;
				break;
			}
		}
	}
}

//��ʾ�����ַ���
void ShowChinese(u16 x, u16 y, u8 *s, u16 color)
{
	uint8_t x0,y0;
	x0=x;
	y0=y;
	while(*s!='\0')
	{
		ShowCharacter(x, y, s, color);
		x+=32;
		y=y0;
		x0+=32;
		s+=2;
	}
}

//���ĺͷ��Ż����ʾ
void ShowMixString(u16 x, u16 y, u8 *s, u16 color)
{
	uint8_t x0,y0;
	x0=x;
	y0=y;
	while(*s!='\0')
	{
		if(*s<0x7F)
		{
			LCD_ShowChar(x,y,*s,color,color,32,1);
			x+=16;
			if(x>280)
			{
			x=x0;
			y0+=32;
			}
			y=y0;
			s+=1;
		}
		else
		{
			ShowCharacter(x, y, s, color);
			x+=32;
			if(x>280)
			{
			x = x0;
			y0+=32;
			}
			y=y0;
			s+=2;
		}
	}
}
//������ʾ�����ַ�
void ShowMixString_Center(u16 y, u8 *s, u16 color)
{
	uint8_t i;
	for(i=0;*s!='\0';i++);
	ShowMixString((LCD_W-i*32)/2, y, s, color);
}
