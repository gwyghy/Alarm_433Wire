/********************************************************************************
* �ļ�����	l4x_flash.c
* ���ߣ�	zty
* �汾��   	V1.0
* ���ڣ�    2015.04.24
* ��������:  l4 flash����

* �޸�˵����   
*
*       >>>>  �ڹ����е�λ��  <<<<
*             3-Ӧ�ò�
*             2-Э���
*         ��   1-Ӳ��������
*********************************************************************************
* @copy
* <h2><center>&copy; COPYRIGHT ������������޹�˾ �з����� �����</center></h2>
*********************************************************************************/
/********************************************************************************
* .hͷ�ļ�
*********************************************************************************/
#include "iapupdate.h"
#if (BSP_PLATFORM == BSP_PLATFORM_L4)
#include "l4x_flash.h"

/********************************************************************************
* #define�궨��
*********************************************************************************/


/**********************************************************************************************************
*	�� �� ��: FLASH_Unlock()
*	����˵��: unlock the main FMC operation
*	��    ��: none
*	�� �� ֵ: none
**********************************************************************************************************/
void FLASH_Unlock(void)
{
	#if STM32 
	HAL_FLASH_Unlock();
	#endif
	fmc_unlock();
}

/**********************************************************************************************************
*	�� �� ��: FLASH_Lock()
*	����˵��: lock the main FMC operation
*	��    ��: none
*	�� �� ֵ: none
**********************************************************************************************************/
void FLASH_Lock(void)
{
	#if STM32 
	HAL_FLASH_Lock();
	#endif
	fmc_lock();
}


/**********************************************************************************************************
*	�� �� ��: IrUsartRxWtrRtrGet
*	����˵��: ��ȡIrUsart�Ľ��ջ�����дָ��
*	��    ��: u8Data:���ݵ�ָ��
*	�� �� ֵ: 0x00:��������Ҫ����0x01:��������Ҫ����
**********************************************************************************************************/
//u32 IrUsartRxWtrRtrGet(u32 *u32Data)
//{
//	*u32Data = s_u32IrUsartRxWrtPtr;
//	
//	return 0x01;
//}
void FLASH_SetLatency(uint32_t FLASH_Latency)
{
	#if STM32 
	__HAL_FLASH_SET_LATENCY(FLASH_Latency);
	#endif
	fmc_wscnt_set(FLASH_Latency);

}

#if STM32 
	uint32_t GetPage(uint32_t Addr)
	{
	  uint32_t page = 0;
	  
	  if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
	  {
		/* Bank 1 */
		page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
	  }
	  else
	  {
		/* Bank 2 */
		page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
	  }
	  
	  return page;
	}
#endif

fmc_state_enum FLASH_ErasePage(uint32_t Page_Address)
{
	#if STM32 
	FLASH_EraseInitTypeDef        EraseInitStruct;
	uint32_t PAGEError = 0;
	
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks       = FLASH_BANK_1;
	EraseInitStruct.Page        = GetPage(Page_Address);
	EraseInitStruct.NbPages     = 1;

	return HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
	#endif	
	
	return fmc_page_erase(Page_Address);

}


/*******************************************************************************************
**�������ƣ�InFlashErasePage
**�������ã���������ʼҳ��ʼ��flash���һҳ ��	��������ʼ������ʼ��flash���һ����
**����������u32StartPageAddr����Ҫ��������ʼҳ�ĵ�ַ
**�����������
**ע�������
*******************************************************************************************/
void InFlashErasePage(u32 u32StartPageAddr, u32 u32EndPageAddr)
{
	while (u32StartPageAddr <= u32EndPageAddr)
	{
		FLASH_ErasePage(u32StartPageAddr);
		u32StartPageAddr += 0x800;			//FLASH_PAGE_SIZE;//����ҳ��С2K 
	}
}

#if STM32 
/*******************************************************************************************
**�������ƣ�InFlashWriteBuf
**�������ã�д�ڲ�FLASH������
**����������u32StartPageAddr����Ҫ��������ʼҳ�ĵ�ַ
**�����������
**ע�������
*******************************************************************************************/
u8 EraseInFlashWriteBuf(u8* pBuffer, u32 u32WriteAddr, u16 NumByteToRead)
{
	uint8_t tmp_buf[8];
	uint8_t tmp_over = 0;
	uint16_t count = 0;
	uint16_t  i;
	
	//д��ַ��� δ��
	#if STM32 
	HAL_FLASH_Unlock();
	#endif
	fmc_unlock();
	
	if(FLASH_ErasePage(GetPage(u32WriteAddr)) != FMC_READY)
		return L4_FLASH_ERASE_ERR;
	
	tmp_over = NumByteToRead % 4;
	count = NumByteToRead / 4;

	for(i = 0; i < count; i ++)
	{     
		#if STM32 
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, u32WriteAddr, *(uint64_t *)pBuffer) == FMC_READY)
		#endif
		if (fmc_word_program(u32WriteAddr, *(uint32_t *)pBuffer) == FMC_READY)
		{    
			u32WriteAddr += 4;
			pBuffer += 4;
		}
		else
			return L4_FLSAH_WR_ERR;
	}
	if(tmp_over)
	{
		memset(tmp_buf, 0xff, 4);
		memcpy(tmp_buf, pBuffer, tmp_over);
		#if STM32 
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, u32WriteAddr, *(uint64_t *)tmp_buf) != FMC_READY)
		#endif
		if (fmc_word_program(u32WriteAddr, *(uint32_t *)tmp_buf) == FMC_READY)
		{    
			return L4_FLSAH_WR_ERR;
		}
	}
	
	#if STM32 
	HAL_FLASH_Lock();
	#endif
	fmc_unlock();
	return L4_FLASH_OK;

}
#endif

/*******************************************************************************************
**�������ƣ�InFlashWriteBuf
**�������ã�д�ڲ�FLASH������
**����������u32StartPageAddr����Ҫ��������ʼҳ�ĵ�ַ
**�����������
**ע�������
*******************************************************************************************/
u8 InFlashWriteBuf(u8* pBuffer, u32 u32WriteAddr, u16 NumByteToRead)
{
	uint8_t tmp_buf[4];
	uint8_t tmp_over = 0;
	uint16_t count = 0;
	uint16_t  i;
	
	//д��ַ��� δ��
//	HAL_FLASH_Unlock();

	tmp_over = NumByteToRead % 4;
	count = NumByteToRead / 4;

	for(i = 0; i < count; i ++)
	{
		#if STM32 
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, u32WriteAddr, *(uint64_t *)pBuffer) == FMC_READY)
		#endif
		if (fmc_word_program(u32WriteAddr, *(uint32_t *)pBuffer) == FMC_READY)
		{    
			u32WriteAddr += 4;
			pBuffer += 4;
		}
		else
			return L4_FLSAH_WR_ERR;
	}
	if(tmp_over)
	{
		memset(tmp_buf, 0xff, 4);
		memcpy(tmp_buf, pBuffer, tmp_over);
		#if STM32 
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, u32WriteAddr, *(uint64_t *)tmp_buf) != FMC_READY)
		#endif
		if (fmc_word_program(u32WriteAddr, *(uint32_t *)tmp_buf) == FMC_READY)
		{    
			return L4_FLSAH_WR_ERR;
		}
	}
	
// HAL_FLASH_Lock();
	return L4_FLASH_OK;
	
}


#ifdef L4_FLASH_TEST
uint32_t gFlashData[4];
uint32_t gFlashWData[4];
void FlashTestProcess(uint32_t addr)
{
	FLASH_Unlock();
	
	FLASH_ErasePage(addr);
	InFlashReadBuf((uint8_t *)gFlashData, addr, sizeof(gFlashData));
	gFlashWData[0] = 0x11223344;
	gFlashWData[1] = 0x55667788;
	gFlashWData[2] = 0x99001122;
	gFlashWData[3] = 0x33445566;
	
//	HAL_FLASH_Program(FLASH_TYPEPROGRAM_FAST, addr, *(uint64_t *)gFlashWData);
	
	InFlashWriteBuf((u8*)gFlashWData, addr, 1);
	
	InFlashReadBuf((uint8_t *)gFlashData, addr, sizeof(gFlashData));
	
	gFlashWData[0] = 0;
	gFlashWData[1] = 0x55667788;
	
	InFlashWriteBuf((u8*)gFlashWData, addr, 4);	
	InFlashReadBuf((uint8_t *)gFlashData, addr, sizeof(gFlashData));
	FLASH_Lock();
}


#endif /*L4_FLASH_TEST*/

//HAL_StatusTypeDef HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data)
//{
//	;
//}

#endif /*BSP_PLATFORM == BSP_PLATFORM_L4*/

