#include <stm32f10x.h>
#include "StoreSystem.h"
#include "Flash.h"
#include "BlueTooth.h"

#define SecurityRegister		0x003000		//��ȫ�Ĵ���3

#define Addr_LastStoreAddr 	0x003000 		//�洢3�ֽڵ�ַ	�洢������Ϊ���һ�δ洢��ַ
#define Addr_CardTotal			0x003003		//�洢2�ֽ�����	Card����
#define Addr_DayStatus			0x003005		//�洢1�ֽ����� 0��ʾ����δ��ʼ��1��ʾ���ս����У�2��ʾ���������
#define Addr_DayCeiling 	 	0x003006 		//�洢2�ֽ�����	ÿ������,Ҳ��CardPool�е�ַ����
#define Addr_DayFinish 		0x003008 		//�洢2�ֽ�����	��ǰ���
#define Addr_CardIndex		0x00300A		//�洢2�ֽ����� ��ǰ�ڿ�Ƭ�������
#define Addr_CardDeletedTotal  0x00300C	//�洢2�ֽ����� �洢����Ϊɾ����Ƭ����
#define Addr_CardFamilierTotal 0x00300E //�洢2�ֽ����� �洢����Ϊ��Ϥ��Ƭ����
#define Addr_CardNewTotal			 0x003010 //�洢2�ֽ����� �洢����Ϊ�¼��뿨Ƭ����



#define LimitStoreAddr			0x6FFFFFF		//Card�洢��ֹ��ַ
#define CardPoolAddr				0x000000 		//����أ���Сһ��������������ַ0x000FFF
#define StartStoreAddr			0x001000		//Card�洢��ʼ��ַ
#define CardPoolMaxIndex    1365				//������ܴ洢��Ƭ��ַ��������


void CalculatePriority(struct Card* pCard);
uint16_t Store_GetCardIndex(void);
uint32_t Store_GetCardAddr(void);

union CardDataFlow //��Card�ṹ�����ͬ��С�������γ����ϣ�������Flash�ж�д��Ƭ
{
	uint8_t cardDataFlow[CARD_SIZE];
	struct Card cardData;
};
union CardDataFlow dataFlow;

uint8_t Rx_buffer[4096];//���ջ��棬������Flashͨ��
uint8_t Tx_buffer[4096];//���ͻ��棬������Flashͨ��

struct Card currentCard;//��ǰ��Ƭ�����ⲿ��ȡ

//��һ�ֵ���
void Store_TestInit()
{
	Flash_EraseSector(CardPoolAddr);
	Flash_EraseSector(0x002000);
	Store_ChangeSecurity(Addr_DayFinish,0,2);
	Store_ChangeSecurity(Addr_CardIndex,0,2);
	Store_ChangeSecurity(Addr_DayStatus,0,1);
}
//ȫ����ʼ��
void Store_TestInitAll()
{
	Flash_EraseSector(StartStoreAddr);
	Flash_EraseSector(CardPoolAddr);
	Flash_EraseSecurity(SecurityRegister);
	//								���һ�δ洢��ַ	����			״̬	����				���				���
	u8 setting[18] ={0x00,0x10,0x00,	0x00,0x00,	0x00,	0x00,0x00,	0x00,0x00, 0x00,0x00};
	Flash_WriteSecurity(setting,SecurityRegister,18);
}

//����������Ŀ�Ƭ��Ϣ�洢��Flash��
void Store_WriteCardToFlash(struct Card card)
{
	//�洢��Ƭ
	u8 temp[3];
	u32 addr;
	Flash_ReadSecurity(temp,Addr_LastStoreAddr,3);
	addr = ((u32)temp[0]<<16) | ((u32)temp[1]<<8) |((u32)temp[2]);
	dataFlow.cardData=card;
	Flash_WriteBuffer(dataFlow.cardDataFlow,addr,CARD_SIZE);	
	//�ı䰲ȫ�Ĵ�����ֵ
	Store_ChangeSecurity(Addr_CardTotal, Store_GetCardTotal()+1, 2);
	addr += CARD_SIZE;
	Store_ChangeSecurity(Addr_LastStoreAddr, addr, 3);
	//������
	BlueTooth_Print("FINISH");
}
//����������Ŀ�Ƭ��Ϣֱ�Ӹ���Flash����ɾ���Ŀ�Ƭ
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

//ͨ����������ͨ����������Ƭ
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


//ֻ�Ե�ǰչʾ��CardBack�Ŀ�Ƭ���в������ı�����Ϣ
//commandֵ��1.know++ 2,Fuzzy++ 3.Forget++ 4.Delete = 1 5.Familiar = 1
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

//�ı䰲ȫ�Ĵ�����Ϣ
//addr:����������ڰ�ȫ�洢���ĵ�ַ
//data:���������
//num:�������ݵĳ��ȣ���λΪ�ֽڣ�ȡֵΪ1-3
void Store_ChangeSecurity(uint32_t addr, uint32_t data, uint8_t num)
{
	//��ʱ���氲ȫ�Ĵ���������
	uint8_t buffer[256];
	Flash_ReadSecurity(buffer,SecurityRegister,256);
	
	//numֻ����1-3֮��ȡֵ
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
	//���²�д
	Flash_EraseSecurity(SecurityRegister);
	Flash_WriteSecurity(buffer,SecurityRegister,256);
}

//��ȡ��ȫ�Ĵ����д洢��ÿ��������
uint16_t Store_GetDayCeiling()
{
	uint16_t value;
	uint8_t temp[2];
	Flash_ReadSecurity(temp,Addr_DayCeiling,2);
	value = temp[1]	+	((uint16_t)temp[0]<<8);
	return value;
}

//��ȡ��ȫ�Ĵ����д洢�ĵ��������
uint16_t Store_GetDayFinish()
{
	uint16_t value;
	uint8_t temp[2];
	Flash_ReadSecurity(temp,Addr_DayFinish,2);
	value = temp[1]	+	((uint16_t)temp[0]<<8);
	return value;
}

//��ȡ��ȫ�Ĵ����д洢�ĵ�ǰ���
uint16_t Store_GetCardIndex()
{
	uint16_t value;
	uint8_t temp[2];
	Flash_ReadSecurity(temp, Addr_CardIndex, 2);
	value = temp[1]	+	((uint16_t)temp[0] << 8);
	return value;
}

//���ݰ�ȫ�Ĵ����еĵ�ǰ��Ż�ȡ����Flash�еĵ�ַ
uint32_t Store_GetCardAddr()
{
	uint8_t temp[3];
	uint32_t addr = CardPoolAddr + Store_GetCardIndex() * 3;
	Flash_ReadBuffer(temp, addr, 3);
	addr = ((u32)temp[0]<<16) | ((u32)temp[1]<<8) | ((u32)temp[2]);
	return addr;
}

//��ȡ��ȫ�Ĵ����д洢�Ŀ�Ƭ����
uint16_t Store_GetCardTotal()
{
	uint16_t value;
	uint8_t temp[2];
	Flash_ReadSecurity(temp, Addr_CardTotal, 2);
	value = temp[1]	+	((uint16_t)temp[0] << 8);
	return value;
}
//��ȡ��ȫ�Ĵ����洢�ĵ���״̬
uint8_t Store_GetDayStatus()
{
	uint8_t value;
	uint8_t temp[1];
	Flash_ReadSecurity(temp, Addr_DayStatus, 1);
	value = temp[0];
	return value;
}
//��ȡɾ����Ƭ������
uint16_t Store_GetCardDeletedTotal()
{
	uint16_t value;
	uint8_t temp[2];
	Flash_ReadSecurity(temp, Addr_CardDeletedTotal, 2);
	value = temp[1]	+	((uint16_t)temp[0] << 8);
	return value;
}
//��ȡ��Ϥ��Ƭ������
uint16_t Store_GetCardFamilierTotal()
{
	uint16_t value;
	uint8_t temp[2];
	Flash_ReadSecurity(temp, Addr_CardFamilierTotal, 2);
	value = temp[1]	+	((uint16_t)temp[0] << 8);
	return value;
}
//��ȡ�¼��뿨Ƭ������
uint16_t Store_GetCardNewTotal()
{
	uint16_t value;
	uint8_t temp[2];
	Flash_ReadSecurity(temp, Addr_CardNewTotal, 2);
	value = temp[1]	+	((uint16_t)temp[0] << 8);
	return value;
}

//�������ȼ�
//pCard:������Ŀ�Ƭ�ĵ�ַ
void CalculatePriority(struct Card* pCard)
{
	(*pCard).priority =  (*pCard).count_hazy + 2 * (*pCard).count_forget - (*pCard).count_know;
}

//�л���Ƭ
void Store_SwitchCard()
{
	Flash_ReadBuffer(dataFlow.cardDataFlow,Store_GetCardAddr(),CARD_SIZE);
	currentCard = dataFlow.cardData;
	Store_ChangeSecurity(Addr_DayFinish,Store_GetDayFinish()+1,2);//�������+1
	Store_ChangeSecurity(Addr_CardIndex,Store_GetCardIndex()+1,2);//��Ƭ���+1
	//�жϵ����Ƿ������
	if(Store_GetDayCeiling() == Store_GetDayFinish())
	{
		Store_ChangeSecurity(Addr_DayStatus,2,1);
		return;
	}
}
//��ʼ������
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
	struct SortNode sortNodes[dayCeiling];//����ش�СΪÿ������
	
	uint32_t cardAddr;
	struct Card card;
	
	for(readIndex=0; (readIndex < Store_GetCardTotal()) && (poolIndex < dayCeiling); readIndex++)//�ӵ�һ�ſ�Ƭ��ʼ����
	{
		cardAddr = StartStoreAddr + readIndex * CARD_SIZE;
		Flash_ReadBuffer(dataFlow.cardDataFlow, cardAddr, CARD_SIZE);
		card = dataFlow.cardData;
		
		if(card.status.isDeleted == 0 && card.status.isFamiliar == 0 )//����Ƭ��ɾ����������֪������
		{
			if(card.status.isNew == 1)//����ƬΪ����ֱ�Ӽ��뿨Ƭ��
			{
				u8 temp[3];
				temp[0] = (cardAddr & 0xFF0000)>>16;
				temp[1] = (cardAddr & 0xFF00)>>8;
				temp[2] = cardAddr & 0xFF;
				Flash_WriteBuffer(temp, CardPoolAddr + poolIndex * 3, 3);
				poolIndex++;
			}
			else//�������򽫿�Ƭ��ȡ��ź����ȼ�����SortNodes��
			{
				sortNodes[priorityIndex].index = readIndex;
				sortNodes[priorityIndex].priority = card.priority;
				priorityIndex++;
			}
		}
	}
	
	//��SortNodes�еĽڵ�������ȼ����򣬸ߵ���ǰ
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
	//��ǰn����Ƭ��ַ���뿨Ƭ�أ�n = DayCeiling - poolIndex
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
	
	//��ȡ�����е�һ�ſ�Ƭ
	Flash_ReadBuffer(dataFlow.cardDataFlow,Store_GetCardAddr(),CARD_SIZE);
	currentCard = dataFlow.cardData;
	Store_ChangeSecurity(Addr_CardIndex,Store_GetCardIndex()+1,2);
	Store_ChangeSecurity(Addr_DayStatus,1,1);
	
}

//���������ؿ�Ƭ
void Store_CardReload()
{
	uint8_t temp[3];
	uint32_t addr = CardPoolAddr + (Store_GetCardIndex()-1) * 3;
	Flash_ReadBuffer(temp, addr, 3);
	addr = ((u32)temp[0]<<16) | ((u32)temp[1]<<8) | ((u32)temp[2]);
	Flash_ReadBuffer(dataFlow.cardDataFlow,addr,CARD_SIZE);
	currentCard = dataFlow.cardData;
}
//�ͷű����ɾ�����ڴ�
void Store_MemoryRelease()
{
	//�ͷ���ɾ����Ƭ��ռ�ڴ�
}
//�ı�ÿ������ֵ
void Store_ChangeDayCeiling(uint16_t value)
{
	if(value > Store_GetCardTotal())
	{
		value = Store_GetCardTotal();
	}
	Store_ChangeSecurity(Addr_DayCeiling,value,2);
}
