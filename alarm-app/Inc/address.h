#ifndef __ADDRESS_H
#define __ADDRESS_H

#include "main.h"
#include "flash.h"
#include "usart.h"

typedef struct
{
	uint8_t  DWT_Mode;      	//模块工作模式：0--标签，其它--基站
	uint32_t ChipID;					//32位芯片ID
	uint16_t ShortAddr;				//16位分配地址
	uint8_t  ShortAddrFlag;		//16位ID标识位：0--未分配16位ID，1--已分配16位ID
}DWT_AddrStatus_s;

typedef struct
{
	uint8_t DWT_Mode;
	uint32_t ChipID;
}DWT_AddrSta_s;

void Tag_AskFor16BitAddr(void);
void Tag_EnsureFor16BitAddr(void);
void CLE_AskFor16BitAddr(uint8_t *pdata);
void CLE_EnsureFor16BitAddr(uint8_t *pdata);
uint16_t GetFreeAddr(void);




#endif
