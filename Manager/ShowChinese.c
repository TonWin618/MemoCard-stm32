//本模块用于读取Flash芯片中烧录的汉字并显示在LCD屏上
//32:汉字大小，占32*32像素
//130:一个汉字的索引和点阵总大小，占130字节
//128:点阵大小，占128字节
//2:索引大小，占2字节
//0x80:查询起始位置，汉字存储先点阵再汉字

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
//显示单个汉字
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

//显示中文字符串
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

//中文和符号混合显示
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
//居中显示中文字符
void ShowMixString_Center(u16 y, u8 *s, u16 color)
{
	uint8_t i;
	for(i=0;*s!='\0';i++);
	ShowMixString((LCD_W-i*32)/2, y, s, color);
}
