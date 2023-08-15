/********************************************************************************
* �ļ����ƣ�	l4x_flash.h
* ��	�ߣ�	zty   
* ��ǰ�汾��   	V1.0
* ������ڣ�    2015.04.24
* ��������: 	����bsp.hͷ�ļ�
* ��ʷ��Ϣ��   
*           	�汾��Ϣ     ���ʱ��      ԭ����        ע��
*
*       >>>>  �ڹ����е�λ��  <<<<
*             3-Ӧ�ò�
*          	  2-Э���
*          ��  1-Ӳ��������
*********************************************************************************
* Copyright (c) 2014,������������޹�˾ All rights reserved.
*********************************************************************************/
#ifndef __L4X_FLASH_H__ 
#define __L4X_FLASH_H__

/********************************************************************************
* .hͷ�ļ�
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
* #define�궨��
*********************************************************************************/

enum {
	L4_FLASH_OK = 0,
	L4_FLASH_ERASE_ERR = 0X01,
	L4_FLSAH_WR_ERR
};


/********************************************************************************
* ��������
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


