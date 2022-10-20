#ifndef   __BlueTooth_H
#define   __BlueTooth_H

void BlueTooth_Init(void);
void BlueTooth_Print(uint8_t* content);
void BlueTooth_Scanf(uint8_t* TX_Buffer,uint16_t lenth);
uint8_t BlueTooth_IsConnected(void);

#endif
