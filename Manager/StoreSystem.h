#ifndef __STORESYSTEM_H
#define __STORESYSTEM_H
#include <stm32f10x.h>

#define CARD_SIZE sizeof(struct Card) //һ�ſ�Ƭ��ռ�ֽڴ�С

struct Status
{
	uint8_t isDeleted: 1;			//���ɾ��
	uint8_t isNew: 1;					//����¼���
	uint8_t isFamiliar: 1;		//�����Ϥ
	uint8_t keep: 5;					//����λ
};

struct Card
{
	uint8_t face[32];					//��������
	uint8_t back[64];					//��������
	struct Status status;			//״̬
	int8_t priority;					//���ȼ�
	uint16_t count_know;			//֪������
	uint16_t count_hazy;			//ģ������
	uint16_t count_forget;		//��������
};

void Store_TestInit(void);
void Store_TestInitAll(void);
void Store_SetCard(void);
void Store_SwitchCard(void);
void Store_CardPoolInit(void);
void Store_CardReload(void);
void Store_ChangeCard(uint8_t command);
void Store_ChangeSecurity(uint32_t addr, uint32_t data, uint8_t num);
uint8_t Store_GetDayStatus(void);
uint16_t Store_GetDayCeiling(void);
uint16_t Store_GetDayFinish(void);
uint16_t Store_GetCardTotal(void);
uint16_t Store_GetCardIndex(void);
uint16_t Store_GetCardDeletedTotal(void);
uint16_t Store_GetCardFamilierTotal(void);
uint16_t Store_GetCardNewTotal(void);


void Store_ChangeDayCeiling(uint16_t value);
void Store_MemoryRelease(void);

#endif
