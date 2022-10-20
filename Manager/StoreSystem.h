#ifndef __STORESYSTEM_H
#define __STORESYSTEM_H
#include <stm32f10x.h>

#define CARD_SIZE sizeof(struct Card) //一张卡片所占字节大小

struct Status
{
	uint8_t isDeleted: 1;			//标记删除
	uint8_t isNew: 1;					//标记新加入
	uint8_t isFamiliar: 1;		//标记熟悉
	uint8_t keep: 5;					//保留位
};

struct Card
{
	uint8_t face[32];					//正面内容
	uint8_t back[64];					//反面内容
	struct Status status;			//状态
	int8_t priority;					//优先级
	uint16_t count_know;			//知道次数
	uint16_t count_hazy;			//模糊次数
	uint16_t count_forget;		//遗忘次数
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
