/********************************************************************************
* 文件名称：	l4x_flash.h
* 作	者：	zty   
* 当前版本：   	V1.0
* 完成日期：    2015.04.24
* 功能描述: 	定义bsp.h头文件
* 历史信息：   
*           	版本信息     完成时间      原作者        注释
*
*       >>>>  在工程中的位置  <<<<
*             3-应用层
*          	  2-协议层
*          √  1-硬件驱动层
*********************************************************************************
* Copyright (c) 2014,天津华宁电子有限公司 All rights reserved.
*********************************************************************************/
#ifndef __L4X_FLASH_H__ 
#define __L4X_FLASH_H__

/********************************************************************************
* .h头文件
*********************************************************************************/
#include "string.h"
//#include "stm32f4xx.h"  
//#include "stm32f4xx_hal_flash.h"
//#include "stm32f4xx_hal_def.h"
#include "gd32f30x.h"
#if STM32 
#define L4_FLASH_TEST
#endif
/********************************************************************************
* #define宏定义
*********************************************************************************/

enum {
	L4_FLASH_OK = 0,
	L4_FLASH_ERASE_ERR = 0X01,
	L4_FLSAH_WR_ERR
};


/********************************************************************************
* 函数声明
*********************************************************************************/
void FLASH_Lock(void);
void FLASH_Unlock(void);
void FLASH_SetLatency(uint32_t FLASH_Latency);
fmc_state_enum FLASH_ErasePage(uint32_t Page_Address);
u8 InFlashWriteBuf(u8* pBuffer, u32 u32WriteAddr, u16 NumByteToRead);
void InFlashErasePage(u32 u32StartPageAddr, u32 u32EndPageAddr);
u8 EraseInFlashWriteBuf(u8* pBuffer, u32 u32WriteAddr, u16 NumByteToRead);

#ifdef L4_FLASH_TEST
void FlashTestProcess(uint32_t addr);
#endif /*L4_FLASH_TEST*/
#endif //__L4X_FLASH_H__


