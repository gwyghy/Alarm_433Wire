/********************************************************************************
* 文件名：	l4x_flash.c
* 作者：	zty
* 版本：   	V1.0
* 日期：    2015.04.24
* 功能描述:  l4 flash驱动

* 修改说明：   
*
*       >>>>  在工程中的位置  <<<<
*             3-应用层
*             2-协议层
*         √   1-硬件驱动层
*********************************************************************************
* @copy
* <h2><center>&copy; COPYRIGHT 天津华宁电子有限公司 研发中心 软件部</center></h2>
*********************************************************************************/
/********************************************************************************
* .h头文件
*********************************************************************************/
#include "iapupdate.h"
#if (BSP_PLATFORM == BSP_PLATFORM_L4)
#include "l4x_flash.h"

/********************************************************************************
* #define宏定义
*********************************************************************************/


/**********************************************************************************************************
*	函 数 名: FLASH_Unlock()
*	功能说明: unlock the main FMC operation
*	形    参: none
*	返 回 值: none
**********************************************************************************************************/
void FLASH_Unlock(void)
{
	#if STM32 
	HAL_FLASH_Unlock();
	#endif
	fmc_unlock();
}

/**********************************************************************************************************
*	函 数 名: FLASH_Lock()
*	功能说明: lock the main FMC operation
*	形    参: none
*	返 回 值: none
**********************************************************************************************************/
void FLASH_Lock(void)
{
	#if STM32 
	HAL_FLASH_Lock();
	#endif
	fmc_lock();
}


/**********************************************************************************************************
*	函 数 名: IrUsartRxWtrRtrGet
*	功能说明: 读取IrUsart的接收缓冲区写指针
*	形    参: u8Data:数据的指针
*	返 回 值: 0x00:无数据需要处理，0x01:有数据需要处理
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
**函数名称：InFlashErasePage
**函数作用：擦除从起始页开始至flash最后一页 或	擦除从起始扇区开始至flash最后一扇区
**函数参数：u32StartPageAddr：需要擦除的起始页的地址
**函数输出：无
**注意事项：无
*******************************************************************************************/
void InFlashErasePage(u32 u32StartPageAddr, u32 u32EndPageAddr)
{
	while (u32StartPageAddr <= u32EndPageAddr)
	{
		FLASH_ErasePage(u32StartPageAddr);
		u32StartPageAddr += 0x800;			//FLASH_PAGE_SIZE;//闪存页大小2K 
	}
}

#if STM32 
/*******************************************************************************************
**函数名称：InFlashWriteBuf
**函数作用：写内部FLASH的数据
**函数参数：u32StartPageAddr：需要擦除的起始页的地址
**函数输出：无
**注意事项：无
*******************************************************************************************/
u8 EraseInFlashWriteBuf(u8* pBuffer, u32 u32WriteAddr, u16 NumByteToRead)
{
	uint8_t tmp_buf[8];
	uint8_t tmp_over = 0;
	uint16_t count = 0;
	uint16_t  i;
	
	//写地址检测 未做
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
**函数名称：InFlashWriteBuf
**函数作用：写内部FLASH的数据
**函数参数：u32StartPageAddr：需要擦除的起始页的地址
**函数输出：无
**注意事项：无
*******************************************************************************************/
u8 InFlashWriteBuf(u8* pBuffer, u32 u32WriteAddr, u16 NumByteToRead)
{
	uint8_t tmp_buf[4];
	uint8_t tmp_over = 0;
	uint16_t count = 0;
	uint16_t  i;
	
	//写地址检测 未做
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

