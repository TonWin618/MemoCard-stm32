#include <stm32f10x.h>
#include "StoreSystem.h"
#include "Flash.h"
#include "BlueTooth.h"

#define SecurityRegister		0x003000		//安全寄存器3

#define Addr_LastStoreAddr 	0x003000 		//存储3字节地址	存储的内容为最后一次存储地址
#define Addr_CardTotal			0x003003		//存储2字节整数	Card总数
#define Addr_DayStatus			0x003005		//存储1字节整数 0表示今日未开始，1表示今日进行中，2表示今日已完成
#define Addr_DayCeiling 	 	0x003006 		//存储2字节整数	每日上限,也是CardPool中地址个数
#define Addr_DayFinish 		0x003008 		//存储2字节整数	当前完成
#define Addr_CardIndex		0x00300A		//存储2字节整数 当前在卡片池中序号
#define Addr_CardDeletedTotal  0x00300C	//存储2字节整数 存储内容为删除卡片总数
#define Addr_CardFamilierTotal 0x00300E //存储2字节整数 存储内容为熟悉卡片总数
#define Addr_CardNewTotal			 0x003010 //存储2字节整数 存储内容为新加入卡片总数



#define LimitStoreAddr			0x6FFFFFF		//Card存储截止地址
#define CardPoolAddr				0x000000 		//对象池，大小一个扇区，结束地址0x000FFF
#define StartStoreAddr			0x001000		//Card存储开始地址
#define CardPoolMaxIndex    1365				//对象池能存储卡片地址的最大个数


void CalculatePriority(struct Card* pCard);
uint16_t Store_GetCardIndex(void);
uint32_t Store_GetCardAddr(void);

union CardDataFlow //将Card结构体和相同大小的数组形成联合，方便在Flash中读写卡片
{
	uint8_t cardDataFlow[CARD_SIZE];
	struct Card cardData;
};
union CardDataFlow dataFlow;

uint8_t Rx_buffer[4096];//接收缓存，用于与Flash通信
uint8_t Tx_buffer[4096];//发送缓存，用于与Flash通信

struct Card currentCard;//当前卡片，供外部读取

//下一轮调用
void Store_TestInit()
{
	Flash_EraseSector(CardPoolAddr);
	Flash_EraseSector(0x002000);
	Store_ChangeSecurity(Addr_DayFinish,0,2);
	Store_ChangeSecurity(Addr_CardIndex,0,2);
	Store_ChangeSecurity(Addr_DayStatus,0,1);
}
//全部初始化
void Store_TestInitAll()
{
	Flash_EraseSector(StartStoreAddr);
	Flash_EraseSector(CardPoolAddr);
	Flash_EraseSecurity(SecurityRegister);
	//								最后一次存储地址	总数			状态	上限				完成				序号
	u8 setting[18] ={0x00,0x10,0x00,	0x00,0x00,	0x00,	0x00,0x00,	0x00,0x00, 0x00,0x00};
	Flash_WriteSecurity(setting,SecurityRegister,18);
}

//将蓝牙传输的卡片信息存储到Flash中
void Store_WriteCardToFlash(struct Card card)
{
	//存储卡片
	u8 temp[3];
	u32 addr;
	Flash_ReadSecurity(temp,Addr_LastStoreAddr,3);
	addr = ((u32)temp[0]<<16) | ((u32)temp[1]<<8) |((u32)temp[2]);
	dataFlow.cardData=card;
	Flash_WriteBuffer(dataFlow.cardDataFlow,addr,CARD_SIZE);	
	//改变安全寄存器中值
	Store_ChangeSecurity(Addr_CardTotal, Store_GetCardTotal()+1, 2);
	addr += CARD_SIZE;
	Store_ChangeSecurity(Addr_LastStoreAddr, addr, 3);
	//输出完成
	BlueTooth_Print("FINISH");
}
//将蓝牙传输的卡片信息直接覆盖Flash中已删除的卡片
void Store_CoverCardToFlash(struct Card card)
{
	uint16_t index;
	uint32_t addr;
	for(index=0;index<Store_GetCardTotal();index++)
	{
		addr = StartStoreAddr + index * CARD_SIZE;
		Flash_ReadBuffer(dataFlow.cardDataFlow,addr,CARD_SIZE);
		if(dataFlow.cardData.status.isDeleted == 1)
		{
			break;
		}
	}
	dataFlow.cardData = card;
	Flash_WriteBuffer(dataFlow.cardDataFlow,addr,CARD_SIZE);
	
}

//通过蓝牙连接通信来建立卡片
void Store_SetCard()
{
	struct Card card;
	BlueTooth_Print("Card Face:");
	BlueTooth_Scanf(card.face,sizeof(card.face));
	BlueTooth_Print("Card Back:");
	BlueTooth_Scanf(card.back,sizeof(card.back));
	card.count_know=0;
	card.count_hazy=0;
	card.count_forget=0;
	card.priority=0;
	card.status.isDeleted=0;
	card.status.isFamiliar=0;
	card.status.isNew=1;
	card.status.keep=0;
	Store_WriteCardToFlash(card);
}


//只对当前展示在CardBack的卡片进行操作，改变其信息
//command值：1.know++ 2,Fuzzy++ 3.Forget++ 4.Delete = 1 5.Familiar = 1
void Store_ChangeCard(uint8_t command)
{
	uint32_t sectorAddr;
	uint32_t changeIndex;
	uint32_t i,j,k;
	
	uint8_t* pRx_Buffer = Rx_buffer;
	uint8_t* pTx_Buffer = Tx_buffer;
	
	uint8_t temp[3];
	uint32_t addr = CardPoolAddr + (Store_GetCardIndex()-1) * 3;
	Flash_ReadBuffer(temp, addr, 3);
	addr = ((u32)temp[0]<<16) | ((u32)temp[1]<<8) | ((u32)temp[2]);
	
	sectorAddr = addr & 0xFFF000;
	changeIndex = addr & 0x000FFF;
	
	Flash_ReadBuffer(Rx_buffer,sectorAddr,4096);
	Flash_EraseSector(sectorAddr);
	
	for(i=0;i<changeIndex;i++)
	{
		*pTx_Buffer++=*pRx_Buffer++;
	}
	
	dataFlow.cardData = currentCard;
	
	switch(command)
	{
		case 1:
			dataFlow.cardData.count_know += 1;
			if(dataFlow.cardData.status.isNew == 1)
				dataFlow.cardData.status.isNew = 0;
			CalculatePriority(&dataFlow.cardData);
			break;
		case 2:
			dataFlow.cardData.count_hazy += 1;
			if(dataFlow.cardData.status.isNew == 1)
				dataFlow.cardData.status.isNew = 0;
			CalculatePriority(&dataFlow.cardData);
			break;
		case 3:
			dataFlow.cardData.count_forget += 1;
			if(dataFlow.cardData.status.isNew == 1)
				dataFlow.cardData.status.isNew = 0;
			CalculatePriority(&dataFlow.cardData);
			break;
		case 4:
			dataFlow.cardData.status.isFamiliar = 1;
			Store_ChangeSecurity(Addr_CardTotal,Store_GetCardTotal()-1,2);
			break;
		case 5:
			dataFlow.cardData.status.isDeleted = 1;
			Store_ChangeSecurity(Addr_CardTotal,Store_GetCardTotal()-1,2);
			break;
		default:
			break;
	}
	
	for(j=0;j<CARD_SIZE;j++)
	{
		*pTx_Buffer++ = dataFlow.cardDataFlow[j];
		pRx_Buffer++;
	}
	
	for(k=i+j;k<4096;k++)
	{
		*pTx_Buffer++=*pRx_Buffer++;
	}
	
	Flash_WriteBuffer(Tx_buffer,sectorAddr,4096);
}

//改变安全寄存器信息
//addr:需更改数据在安全存储器的地址
//data:传入的数据
//num:传入数据的长度，单位为字节，取值为1-3
void Store_ChangeSecurity(uint32_t addr, uint32_t data, uint8_t num)
{
	//暂时保存安全寄存器中数据
	uint8_t buffer[256];
	Flash_ReadSecurity(buffer,SecurityRegister,256);
	
	//num只会在1-3之间取值
	if(num == 1)
	{
		buffer[addr-SecurityRegister] = (data & 0xFF);
	}
	else if(num == 2)
	{
		buffer[addr-SecurityRegister] = (data & 0xFF00) >> 8;
		buffer[addr-SecurityRegister+1] = (data & 0xFF);
	}
	else if(num == 3)
	{
		buffer[addr-SecurityRegister] = (data & 0xFF0000) >> 16;
		buffer[addr-SecurityRegister+1] = (data & 0xFF00) >> 8;
		buffer[addr-SecurityRegister+2] = (data & 0xFF);
	}
	else
	{
		return;
	}
	//重新擦写
	Flash_EraseSecurity(SecurityRegister);
	Flash_WriteSecurity(buffer,SecurityRegister,256);
}

//读取安全寄存器中存储的每日上限数
uint16_t Store_GetDayCeiling()
{
	uint16_t value;
	uint8_t temp[2];
	Flash_ReadSecurity(temp,Addr_DayCeiling,2);
	value = temp[1]	+	((uint16_t)temp[0]<<8);
	return value;
}

//读取安全寄存器中存储的当日完成数
uint16_t Store_GetDayFinish()
{
	uint16_t value;
	uint8_t temp[2];
	Flash_ReadSecurity(temp,Addr_DayFinish,2);
	value = temp[1]	+	((uint16_t)temp[0]<<8);
	return value;
}

//读取安全寄存器中存储的当前序号
uint16_t Store_GetCardIndex()
{
	uint16_t value;
	uint8_t temp[2];
	Flash_ReadSecurity(temp, Addr_CardIndex, 2);
	value = temp[1]	+	((uint16_t)temp[0] << 8);
	return value;
}

//根据安全寄存器中的当前序号获取其在Flash中的地址
uint32_t Store_GetCardAddr()
{
	uint8_t temp[3];
	uint32_t addr = CardPoolAddr + Store_GetCardIndex() * 3;
	Flash_ReadBuffer(temp, addr, 3);
	addr = ((u32)temp[0]<<16) | ((u32)temp[1]<<8) | ((u32)temp[2]);
	return addr;
}

//读取安全寄存器中存储的卡片总数
uint16_t Store_GetCardTotal()
{
	uint16_t value;
	uint8_t temp[2];
	Flash_ReadSecurity(temp, Addr_CardTotal, 2);
	value = temp[1]	+	((uint16_t)temp[0] << 8);
	return value;
}
//读取安全寄存器存储的当日状态
uint8_t Store_GetDayStatus()
{
	uint8_t value;
	uint8_t temp[1];
	Flash_ReadSecurity(temp, Addr_DayStatus, 1);
	value = temp[0];
	return value;
}
//获取删除卡片的数量
uint16_t Store_GetCardDeletedTotal()
{
	uint16_t value;
	uint8_t temp[2];
	Flash_ReadSecurity(temp, Addr_CardDeletedTotal, 2);
	value = temp[1]	+	((uint16_t)temp[0] << 8);
	return value;
}
//获取熟悉卡片的数量
uint16_t Store_GetCardFamilierTotal()
{
	uint16_t value;
	uint8_t temp[2];
	Flash_ReadSecurity(temp, Addr_CardFamilierTotal, 2);
	value = temp[1]	+	((uint16_t)temp[0] << 8);
	return value;
}
//获取新加入卡片的数量
uint16_t Store_GetCardNewTotal()
{
	uint16_t value;
	uint8_t temp[2];
	Flash_ReadSecurity(temp, Addr_CardNewTotal, 2);
	value = temp[1]	+	((uint16_t)temp[0] << 8);
	return value;
}

//计算优先级
//pCard:被计算的卡片的地址
void CalculatePriority(struct Card* pCard)
{
	(*pCard).priority =  (*pCard).count_hazy + 2 * (*pCard).count_forget - (*pCard).count_know;
}

//切换卡片
void Store_SwitchCard()
{
	Flash_ReadBuffer(dataFlow.cardDataFlow,Store_GetCardAddr(),CARD_SIZE);
	currentCard = dataFlow.cardData;
	Store_ChangeSecurity(Addr_DayFinish,Store_GetDayFinish()+1,2);//当日完成+1
	Store_ChangeSecurity(Addr_CardIndex,Store_GetCardIndex()+1,2);//卡片序号+1
	//判断当日是否已完成
	if(Store_GetDayCeiling() == Store_GetDayFinish())
	{
		Store_ChangeSecurity(Addr_DayStatus,2,1);
		return;
	}
}
//初始化卡池
void Store_CardPoolInit()
{
	if(Store_GetCardTotal()==0)
	{
		return;
	}
	uint16_t readIndex;
	uint16_t poolIndex=0;
	uint16_t priorityIndex=0;
	uint16_t dayCeiling = Store_GetDayCeiling();
	
	struct SortNode
	{
		uint16_t index;
		int8_t priority;
	};
	struct SortNode sortNodes[dayCeiling];//排序池大小为每日上限
	
	uint32_t cardAddr;
	struct Card card;
	
	for(readIndex=0; (readIndex < Store_GetCardTotal()) && (poolIndex < dayCeiling); readIndex++)//从第一张卡片开始遍历
	{
		cardAddr = StartStoreAddr + readIndex * CARD_SIZE;
		Flash_ReadBuffer(dataFlow.cardDataFlow, cardAddr, CARD_SIZE);
		card = dataFlow.cardData;
		
		if(card.status.isDeleted == 0 && card.status.isFamiliar == 0 )//若卡片已删除或者已熟知则跳过
		{
			if(card.status.isNew == 1)//若卡片为新则直接加入卡片池
			{
				u8 temp[3];
				temp[0] = (cardAddr & 0xFF0000)>>16;
				temp[1] = (cardAddr & 0xFF00)>>8;
				temp[2] = cardAddr & 0xFF;
				Flash_WriteBuffer(temp, CardPoolAddr + poolIndex * 3, 3);
				poolIndex++;
			}
			else//若其他则将卡片读取序号和优先级存入SortNodes中
			{
				sortNodes[priorityIndex].index = readIndex;
				sortNodes[priorityIndex].priority = card.priority;
				priorityIndex++;
			}
		}
	}
	
	//对SortNodes中的节点进行优先级排序，高的在前
	uint8_t i,j;
	struct SortNode tempNode;
	for(i=0; i<priorityIndex-1; i++)
	{
		for(j=0;j+1<priorityIndex-i;j++)
		{
			if(sortNodes[j].priority< sortNodes[j+1].priority)
			{
				tempNode = sortNodes[j];
				sortNodes[j] = sortNodes[j+1];
				sortNodes[j+1] = tempNode;
			}
		}
	}
	
	uint16_t remain = dayCeiling - poolIndex;
	//将前n个卡片地址存入卡片池，n = DayCeiling - poolIndex
	for(i=0; i<priorityIndex && i<remain; i++)
	{
		cardAddr = StartStoreAddr + sortNodes[i].index * CARD_SIZE;
		u8 temp[3];
		temp[0] = (cardAddr & 0xFF0000)>>16;
		temp[1] = (cardAddr & 0xFF00)>>8;
		temp[2] = (cardAddr & 0xFF);
		Flash_WriteBuffer(temp, CardPoolAddr + poolIndex * 3, 3);
		poolIndex++;
	}
	
	//读取卡池中第一张卡片
	Flash_ReadBuffer(dataFlow.cardDataFlow,Store_GetCardAddr(),CARD_SIZE);
	currentCard = dataFlow.cardData;
	Store_ChangeSecurity(Addr_CardIndex,Store_GetCardIndex()+1,2);
	Store_ChangeSecurity(Addr_DayStatus,1,1);
	
}

//开机后重载卡片
void Store_CardReload()
{
	uint8_t temp[3];
	uint32_t addr = CardPoolAddr + (Store_GetCardIndex()-1) * 3;
	Flash_ReadBuffer(temp, addr, 3);
	addr = ((u32)temp[0]<<16) | ((u32)temp[1]<<8) | ((u32)temp[2]);
	Flash_ReadBuffer(dataFlow.cardDataFlow,addr,CARD_SIZE);
	currentCard = dataFlow.cardData;
}
//释放被标记删除的内存
void Store_MemoryRelease()
{
	//释放已删除卡片所占内存
}
//改变每日上限值
void Store_ChangeDayCeiling(uint16_t value)
{
	if(value > Store_GetCardTotal())
	{
		value = Store_GetCardTotal();
	}
	Store_ChangeSecurity(Addr_DayCeiling,value,2);
}
