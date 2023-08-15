/********************************************************************************
* 文件名称：	iapupdate.c
* 作	者：	马如意   
* 当前版本：   	V1.0
* 完成日期：    2014.02.08
* 功能描述: 	完成程序升级时，相关硬件的操作。主要包括外扩FLASH、LCD显示等的操作，依据项目实际情况对其中使用的底层函数进行替换。
* 历史信息：   
*           	版本信息     完成时间      原作者        注释
*
*       >>>>  在工程中的位置  <<<<
*          	  3-应用层
*           √ 2-协议层
*             1-硬件驱动层
*********************************************************************************
* Copyright (c) 2014,天津华宁电子有限公司 All rights reserved.
*********************************************************************************/
/********************************************************************************
* .h头文件
*********************************************************************************/
#include "iapupdate.h"
#include "iwdg.h"
#if (BSP_PLATFORM == BSP_PLATFORM_L4)
	#include "l4x_flash.h"
#endif /*BSP_PLATFORM	==	BSP_PLATFORM_L4*/



/********************************************************************************
* 常量定义
*********************************************************************************/

/********************************************************************************
* 变量定义
*********************************************************************************/
/**定义设备类型与存储地址的对应关系***//**对应于不同的模块，此表需要修改***/
static PRG_STORAGE_BASEADDR_TYPE const sDevPrgWriteBase[(u8)(DYK_DEV_TYPE_MAX & 0xFF)]=
{
	{THE_DEV_TYPE,		THE_DEV_PRG_STORAGE_BASEADDR},
	{DYK_DEV_TYPE_MAX,	0x00}
};
/**定义设备类型与程序大小的对应关系***//**对应于不同的模块，此表需要修改***/
static PRG_STORAGE_BASEADDR_TYPE const sDevPrgSize[(u8)(DYK_DEV_TYPE_MAX & 0xFF)]=	
{
	{THE_DEV_TYPE,		THE_DEV_PRG_STORAGE_SIZE},
	{DYK_DEV_TYPE_MAX,	0x00}
};

typedef  void (*pFunction)(void);
/*定义jumpToApp为指针函数*/
pFunction jumpToApp; 
static u8 u8Buf[LOAD_CODE_TEMPBUF_MAX_LEN] = {0x00};//用于数据计算的临时数组
/********************************************************************************
* 函数定义
*********************************************************************************/
/*******************************************************************************************
**函数名称：IapGetPrgStorageAddr
**函数作用: 获取程序在内/外Flash中的存储地址
**函数参数：无
**注意事项：无
*******************************************************************************************/
u8 IapGetPrgStorageAddr(u32 u32DevType, u32 *pStorageAddr)
{
	u8 u8I = 0x00;
	u8 u8ReturnValue = 0x00;
	
	for(u8I = 0x00; u8I < (u8)(DYK_DEV_TYPE_MAX & 0xFF); u8I++)
	{
		if(sDevPrgWriteBase[u8I].u32DevType == DYK_DEV_TYPE_MAX)
		{
			u8ReturnValue = 0x00;
			break;
		}
		if(sDevPrgWriteBase[u8I].u32DevType == u32DevType)
		{
			*pStorageAddr = sDevPrgWriteBase[u8I].u32NorPrgBaseAddr;
			u8ReturnValue = 0x01;
			break;
		}
			
	}
	
	return u8ReturnValue;
}
/*******************************************************************************************
**函数名称：IapGetPrgStorageAddr
**函数作用: 获取程序在内/外Flash中的程序存储大小
**函数参数：无
**注意事项：无
*******************************************************************************************/
u8 IapGetPrgSize(u32 u32DevType, u32 *pSize)
{
	u8 u8I = 0x00;
	u8 u8ReturnValue = 0x00;
	
	for(u8I = 0x00; u8I < (u8)(DYK_DEV_TYPE_MAX&0xFF); u8I++)
	{
		if(sDevPrgSize[u8I].u32DevType == DYK_DEV_TYPE_MAX)
		{
			u8ReturnValue = 0x00;
			break;
		}
		if(sDevPrgSize[u8I].u32DevType == u32DevType)
		{
			*pSize = sDevPrgSize[u8I].u32NorPrgBaseAddr;
			u8ReturnValue = 0x01;
			break;
		}
			
	}
	
	return u8ReturnValue;
}
/*******************************************************************************************
**函数名称：NVIC_DeInit
**函数作用: NVIC复位
**函数参数：无
**注意事项：无
*******************************************************************************************/
void NVIC_DeInit(void)
{
 #if (BSP_PLATFORM	==	BSP_PLATFORM_M4)
	u32 u32I = 0x00;
	
	/*关闭定时器中断*/
	SysTick->VAL   = 0x00;                                          /* Load the SysTick Counter Value */
	SysTick->CTRL  = 0x00;
	/*清除中断允许位，cortex_m4内核共计81个中断*/
	NVIC->ICER[0] = 0xFFFFFFFF;//写入1：设置中断失能
	NVIC->ICER[1] = 0xFFFFFFFF;
	NVIC->ICER[2] = 0x0001FFFF;
	/*清除中断挂起位*/
	NVIC->ICPR[0] = 0xFFFFFFFF;//写入1：解除中断挂起状态
	NVIC->ICPR[1] = 0xFFFFFFFF;	
	NVIC->ICPR[2] = 0x0001FFFF;
	
	for(u32I = 0; u32I < 81; u32I++)
	{
	 NVIC->IP[u32I] = 0x00;	
	} 
#elif (BSP_PLATFORM	==	BSP_PLATFORM_M3)
	u32 index = 0;
	
	NVIC->ICER[0] = 0xFFFFFFFF;
	NVIC->ICER[1] = 0x000007FF;
	NVIC->ICPR[0] = 0xFFFFFFFF;
	NVIC->ICPR[1] = 0x000007FF;
	
	for(index = 0; index < 0x0B; index++)
	{
	 NVIC->IP[index] = 0x00000000;	
	}  
#elif (BSP_PLATFORM	==	BSP_PLATFORM_LX)
	u32 index = 0;
	
	NVIC->ICER[0] = 0xFFFFFFFF;
	NVIC->ICER[1] = 0x000007FF;
	NVIC->ICPR[0] = 0xFFFFFFFF;
	NVIC->ICPR[1] = 0x000007FF;
	
	for(index = 0; index < 0x0B; index++)
	{
	 NVIC->IP[index] = 0x00000000;	
	}  
#elif (BSP_PLATFORM	==	BSP_PLATFORM_L4)	
	u32 u32I = 0x00;
	
	/*关闭定时器中断*/
	SysTick->VAL   = 0x00;                                          /* Load the SysTick Counter Value */
	SysTick->CTRL  = 0x00;
	/*清除中断允许位，cortex_m4内核共计81个中断*/
	NVIC->ICER[0] = 0xFFFFFFFF;//写入1：设置中断失能
	NVIC->ICER[1] = 0xFFFFFFFF;
	NVIC->ICER[2] = 0x0001FFFF;
	/*清除中断挂起位*/
	NVIC->ICPR[0] = 0xFFFFFFFF;//写入1：解除中断挂起状态
	NVIC->ICPR[1] = 0xFFFFFFFF;	
	NVIC->ICPR[2] = 0x0001FFFF;
	
	for(u32I = 0; u32I < 81; u32I++)
	{
		NVIC->IP[u32I] = 0x00;	
	} 
#else
	#error "Dev type error!"
#endif

}

/*******************************************************************************************
**函数名称：vIapJumpToApp
**函数作用：跳转至APP程序 
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void IapJumpToApp(u32 u32AppAddr)
{
	/*检查栈顶地址是否合法.*/
	if(((*(u32*)u32AppAddr) & 0x2FFE0000) == 0x20000000)
	{ 	
		//NVIC复位
		NVIC_DeInit();
		/*用户代码区第二个字为程序开始地址(复位地址)*/
		jumpToApp = (pFunction)(*(u32*)(u32AppAddr + 4));

		/*初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)*/				
		__set_MSP(*(u32*)u32AppAddr);

		/* Vector Table Relocation in Internal FLASH. */
  		SCB->VTOR = FLASH_BASE | VTOR_OFFSET; 
		//SCB->VTOR = appAddr; 

		/*跳转到APP*/						
		jumpToApp();											
	}
}

/*********************************************************************************************
** 函数名：iapJumpToBoot
** 输　入：bootAddr --- BOOT程序地址  
** 输　出：None
** 功能描述：跳转至bootloader程序 
** 全局变量：None
** 调用模块：None
** 作　者:	马如意
** 日　期： 2013.07.18
*********************************************************************************************/
void IapJumpToBoot(uint32_t u32bootAddr)
{
	/*检查栈顶地址是否合法.*/
	if(((*(uint32_t*)u32bootAddr) & 0x2FFE0000) == 0x20000000)
	{ 	
		__set_FAULTMASK(1);
		NVIC_SystemReset(); //恢复NVIC为复位状态.使中断不再发生.马如意，2013.07.18增加
		//NVIC复位
		//NVIC_DeInit(); //第二种方法有待于验证，后验证成功
		/*用户代码区第二个字为程序开始地址(复位地址)*/
		jumpToApp = (pFunction)(*(uint32_t*)(u32bootAddr + 4));
		

		/*初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)*/				
		__set_MSP(*(uint32_t*)u32bootAddr);

		/* Vector Table Relocation in Internal FLASH. */
		//SCB->VTOR = IN_FLASH_BOOTLOADER_ADDR | VECT_BL_TAB_OFFSET; 
		SCB->VTOR = u32bootAddr; 
		
		/*跳转到Boot*/						
		jumpToApp();											
	}
}
/*******************************************************************************************
**函数名称：InFlashCrc
**函数作用：计算CRC校验
**函数参数：u32SrcAddr:起始地址，u32Len:需要校验数据的长度
**函数输出：u16类型CRC校验值
**注意事项：无
*******************************************************************************************/
/* CRC16 implementation acording to CCITT standards */
u16 const u16Crc16tab[256] = 
{
0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};

u16 InFlashCrc(u32 u32SrcAddr, u32 u32Len)
{
	u32 u32Counter = 0x00;
	u16 u16Crc = 0x00;
	
	for( u32Counter = 0; u32Counter < u32Len; u32Counter++)
	{
		u16Crc = (u16Crc << 8) ^ u16Crc16tab[((u16Crc >> 8) ^ *(u8 *)u32SrcAddr++) & 0x00FF];
	}
	return u16Crc;
}

/*******************************************************************************************
**函数名称：vCrc16Ccitt
**函数作用：计算CRC16校验值
**函数参数：u8Buf:需要校验的数据起始地址，u32Len:需要校验数据的长度，*u16CheckOld:计算后的CRC校验值
**函数输出：无
**注意事项：无
*******************************************************************************************/
void Crc16Ccitt(const u8 *u8Buf, u32 u32Len, u16 *u16CheckOld)
{
	u32 u32Cnt = 0x00;
	u16 u16Crc = *u16CheckOld;
	
	for( u32Cnt = 0; u32Cnt < u32Len; u32Cnt++)
		u16Crc = (u16Crc << 8) ^ u16Crc16tab[((u16Crc >> 8) ^ *(u8 *)u8Buf++) & 0x00FF];
	*u16CheckOld = u16Crc;
}

/*******************************************************************************************
**函数名称：vDelay
**函数作用：简单的延时(非精确)
**函数参数：u32Count延时的时间参数
**函数输出：无
**注意事项：无
*******************************************************************************************/
void Delay(u32 u32Count)
{
	u32 u32I = 0x00, u32Y = 0x00;
	
	for (u32I = 0; u32I < 500 ; ++u32I)
	{
		for (u32Y = 0; u32Y < u32Count; ++u32Y)
			;
	}
}
 /*******************************************************************************************
**函数名称：InFlashReadBuf
**函数作用：读取FLASH中特定长度的数据
**函数参数：pBuffer：读取地址,u32Addr:需要操作的起始地址，u16NumByteToRead：字节数 
**函数输出：操作是否成功。SUCCESS:读取成功(0);ERROR_UP_OVERFLOW:地址超过最大地址，溢出(0x01)
**注意事项：无
*******************************************************************************************/
u8 InFlashReadBuf(u8 *pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	u16 u16Counter = 0x00;
	
	for(u16Counter = 0; u16Counter < NumByteToRead; u16Counter++)
	{
		if((NumByteToRead - u16Counter) >= 0x04)//以32位进行读取
		{
			*(u32 *)(pBuffer + u16Counter) = *(u32 *)(ReadAddr + u16Counter);
			u16Counter += 0x03;
		}
		else	
		{
			*(pBuffer + u16Counter) = *(u8 *)(ReadAddr + u16Counter);
		}
	}

	return 0x01;
}
/*******************************************************************************************
**函数名称：IapExFlashCrc
**函数作用：计算从 srcAddr开始长度为len的数据的crc
**函数参数：u32SrcAddr:外部FLASH内数据首地址,u32Len:外部FLASH内数据长度
**函数输出：u16类型CRC校验值
**注意事项：无
*******************************************************************************************/
u16 IapExFlashCrc(u32 u32SrcAddr, u32 u32Len,u16 *pCrc)
{
	u16 u16i = 0x00;
	u16 u16Crc = *pCrc;
	u16 u16CopyLen = 0x00;
	u32 u32ExFlashAppAddr = u32SrcAddr;
	
	while (u32Len > 0)
	{
		/*判断每次拷贝数据的大小*/	
		u16CopyLen = (u32Len > LOAD_CODE_TEMPBUF_MAX_LEN) ? LOAD_CODE_TEMPBUF_MAX_LEN : u32Len;			
		
		/*读取每次拷贝的数据*/
		#if (BSP_PRG_STORAGE_FLASH_FLAG	==	BSP_PRG_STORAGE_EXFLASH_USE)
			W25QXX_ReadBuffer(u8Buf, u32ExFlashAppAddr, u16CopyLen);	
		#elif (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_INFLASH_USE)
			InFlashReadBuf(u8Buf, u32ExFlashAppAddr, u16CopyLen);	
		#else
			return 0x00;
		#endif
		u32ExFlashAppAddr += u16CopyLen;
		
		/*将数据进行CRC校验*/
		for (u16i = 0; u16i < u16CopyLen; u16i++)
		{
			u16Crc = (u16Crc << 8) ^ u16Crc16tab[((u16Crc >> 8) ^ u8Buf[u16i]) & 0x00FF];	
		}
		/*剩余数据的长度*/
		 if (u16CopyLen != u32Len)
		{
			u32Len = u32Len - LOAD_CODE_TEMPBUF_MAX_LEN;
		}
		else
		{
			return u16Crc;
		}
		#ifdef IWDG_ENABLED
			IWDG_Feed();
		#endif
	} 
	return 0;
}

/*******************************************************************************************
**函数名称：IapLoadCode
**函数作用:	将外部FLASH内升级数据写入STM32 
**函数参数：u32destAddr:STM32内部FLASH地址,u32SrcAddr:外部FLASH内数据首地址,u32Len:外部FLASH内数据长度
**函数输出：操作是否成功。SUCCESS:操作成功(0x00);ERROR_UP_OVERFLOW:地址超过最大地址，溢出(0x01)；ERROR_DATA_NUM:读取字节数错误(0x04)
**注意事项：无
*******************************************************************************************/
u8 IapLoadCode(u32 u32DestAddr, const u32 u32SrcAddr, u32 u32Len)
{
//	u16 u16I = 0x00; 
	u16 u16CopyLen = 0x00;
//	uint64_t  tmp_data = 0x00;
	u32 u32ExFlashAppAddr = u32SrcAddr;
	u32 u32InFlashAppAddr = u32DestAddr;
	u32 u32FlashLen = IN_CODE_FLASH_SIZE_MAX;

	/*边界判断*/
	if (u32Len > u32FlashLen)
	{
		return ERROR_UP_OVERFLOW;
	}
	else
	{
		while (u32Len > 0)
		{
			/*判断每次拷贝数据的大小*/	
			u16CopyLen = (u32Len > LOAD_CODE_TEMPBUF_MAX_LEN) ? LOAD_CODE_TEMPBUF_MAX_LEN : u32Len;			

			/*读取每次拷贝的数据*/
			#if (BSP_PRG_STORAGE_FLASH_FLAG	== BSP_PRG_STORAGE_EXFLASH_USE)
				W25QXX_ReadBuffer(u8Buf, u32ExFlashAppAddr, u16CopyLen);		
			#elif (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_INFLASH_USE)
				InFlashReadBuf(u8Buf, u32ExFlashAppAddr, u16CopyLen);	
			#else
				return 0x00;
			#endif
		
			/*读取每次拷贝的数据*/
			u32ExFlashAppAddr += u16CopyLen;
			
			/*将数据写入STM32的内部FLASH*/
			InFlashWriteBuf(u8Buf, u32InFlashAppAddr, u16CopyLen);
			u32InFlashAppAddr += u16CopyLen;
			/*剩余数据的长度*/
			if (u16CopyLen != u32Len)
			{
				u32Len = u32Len - LOAD_CODE_TEMPBUF_MAX_LEN;
			}
			else
			{
				return SUCCESS;
			}
		}
	} 
	return ERROR_DATA_NUM;
} 


/*******************************************************************************************
**函数名称：IapWriteCodeLen
**函数作用: 写入升级代码的长度
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void IapWriteCodeLen(u32 u32Addr, u32 u32Len)
{
	/*将数据写入目标地址*/
	#if (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_EXFLASH_USE)
		W25QXX_WriteBuffer((u8 *)&u32Len, u32Addr, 0x04);
	#elif (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_INFLASH_USE)
		InFlashWriteBuf((u8 *)&u32Len, u32Addr, sizeof(u32Len));
	#else
		return ;
	#endif
}

/*******************************************************************************************
**函数名称：IapReadCodeLen
**函数作用：读出需要升级程序的大小
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
u32 IapReadCodeLen(u32 u32Addr)
{
	u32 u32Len = 0x00;
	
	/*从目标地址读出所需的数据至缓存*/
	#if (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_EXFLASH_USE)
		W25QXX_ReadBuffer((u8 *)&u32Len, u32Addr, 0x04);
	#elif (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_INFLASH_USE)
		InFlashReadBuf((u8 *)&u32Len, u32Addr, 0x04);
	#else
		return 0x00;
	#endif
	return u32Len;
}

/*******************************************************************************************
**函数名称：IapWriteCrc
**函数作用：将CRC写入到外部FLASH
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void IapWriteCrc(u32 u32Addr, u32 u32Crc)
{
	/*将拆分后的数据写入目标地址*/
	#if (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_EXFLASH_USE)
		W25QXX_WriteBuffer((u8 *)&u32Crc, u32Addr, 0x04);
	#elif (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_INFLASH_USE)
		InFlashWriteBuf((u8 *)&u32Crc, u32Addr, sizeof(u32Crc));
	#else
		return ;
	#endif	
}

/*******************************************************************************************
**函数名称：IapReadCrc
**函数作用：读出外部FLASH的CRC
**函数参数：无
**函数输出：CRC校验值，为u16类型。
**注意事项：无
*******************************************************************************************/
u32 IapReadCrc(u32 u32Addr)
{
	u32 u32Crc = 0x00;

	/*从目标地址读出所需的数据至缓存*/
	#if (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_EXFLASH_USE)	
		W25QXX_ReadBuffer((u8 *)&u32Crc, u32Addr, 0x04);
	#elif (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_INFLASH_USE)
		InFlashReadBuf((u8 *)&u32Crc, u32Addr, 0x04);
	#else
		return 0x00;
	#endif
	return u32Crc;
}

/*******************************************************************************************
**函数名称：IapReadFlag
**函数作用：读取升级标志状态
**函数参数：u32Addr:需要操作的标志位地址 
**函数输出：操作是否成功。UCCESS:标志设置成功(0x00);ERROR_UP_OVERFLOW:地址超过最大地址，溢出(0x01)
**注意事项：无
*******************************************************************************************/
u32 IapReadFlag(u32 u32Addr)
{
 	/*将目标地址上的标志读出*/
 	u32 u32FlagStats = 0x00;
	
	#if (BSP_PRG_STORAGE_FLASH_FLAG	== BSP_PRG_STORAGE_EXFLASH_USE)		
 		W25QXX_ReadBuffer((u8 *)&u32FlagStats, u32Addr, 0x04);
	#elif (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_INFLASH_USE)
		InFlashReadBuf((u8 *)&u32FlagStats, u32Addr, 0x04);
	#else
		return 0x00;
	#endif
	
 	return u32FlagStats;
}

/*******************************************************************************************
**函数名称：IapFlagSet
**函数作用：置位升级标志 
**函数参数：u32Addr:需要操作的标志位地址 
**函数输出：操作是否成功。UCCESS:标志设置成功(0x00);ERROR_UP_OVERFLOW:地址超过最大地址，溢出(0x01)
**注意事项：无
*******************************************************************************************/
u32 IapFlagSet(u32 u32Addr)
{
	/*将目标地址上的标志位置位*/
 	u8 Flag = 0x01;
	
	#if (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_EXFLASH_USE)	
 		return W25QXX_WriteBuffer((u8 *)&Flag, u32Addr, 0x04);
	#elif (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_INFLASH_USE)
		return InFlashWriteBuf((u8 *)&Flag, u32Addr, 0x04);
	#else
		return 0x00;	
	#endif
}

/*******************************************************************************************
**函数名称：u32IapFlagClear
**函数作用：清除升级标志
**函数参数：u32Addr:需要操作的标志位地址 
**函数输出：操作是否成功。SUCCESS:标志设置成功(0x00);ERROR_UP_OVERFLOW:地址超过最大地址，溢出(0x01)
**注意事项：无
*******************************************************************************************/
u32 IapFlagClear(u32 u32Addr)
{
	/*将目标地址上的标志位清除*/
 	FlagStatus Flag = RESET;

	#if (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_EXFLASH_USE)	
 		return W25QXX_WriteBuffer((u8*)&Flag, u32Addr, 0x04);
	#elif (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_INFLASH_USE)
		return InFlashWriteBuf((u8 *)&Flag, u32Addr, 0x04); 
	#else
		return 0x00;
	#endif	
}

/*******************************************************************************************
**函数名称：u8IapReadBuf
**函数作用：读取FLASH中特定长度的数据，并写入到指定位置
**函数参数：pBuffer：写入地址,u32Addr:需要操作的起始地址，u16NumByteToRead：字节数 
**函数输出：操作是否成功。SUCCESS:读取成功(0);ERROR_UP_OVERFLOW:地址超过最大地址，溢出(0x01)
**注意事项：无
*******************************************************************************************/
u8 IapReadBuf(u8 *pBuffer, u32 u32ReadAddr, u16 u16NumByteToRead)
{
 	u8 u8Flag = 0x00;

	#if (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_EXFLASH_USE)	
		u8Flag = W25QXX_ReadBuffer(pBuffer,u32ReadAddr,u16NumByteToRead);
	#elif (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_INFLASH_USE)
		u8Flag = InFlashReadBuf(pBuffer, u32ReadAddr, u16NumByteToRead);
	#else
		u8Flag =  0x00;
	#endif
 	return u8Flag;
}

/*******************************************************************************************
**函数名称：IapWriteBuf
**函数作用：将特定长度的数据写入到flash中指定位置
**函数参数：pBuffer：数据读取地址，u32Addr:需要操作的起始地址，u16NumByteToRead：字节数  
**函数输出：操作是否成功。SUCCESS:写入成功(0x00);ERROR_UP_OVERFLOW:地址超过最大地址，溢出(0x01)
**注意事项：u16NumByteToRead:最大写入字节范围为64K。
*******************************************************************************************/
u8 IapWriteBuf(u8 *pBuffer, u32 u32ReadAddr, u16 u16NumByteToRead)
{
 	u8 u8Flag = 0x00;
	 
	#if (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_EXFLASH_USE)
		u8Flag = W25QXX_WriteBuffer(pBuffer,u32ReadAddr,u16NumByteToRead);
	#elif (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_INFLASH_USE)
		u8Flag = InFlashWriteBuf(pBuffer, u32ReadAddr, u16NumByteToRead);
	#else
		u8Flag =  0x00;
	#endif	
 	return u8Flag;
}

/*******************************************************************************************
**函数名称：IapEraserBulk
**函数作用：删除一个块
**函数参数：u32BulkAddr:需要操作的起始地址
**函数输出：操作是否成功。SUCCESS:操作成功(0x00);ERROR_UP_OVERFLOW:地址超过最大地址，溢出(0x01)
**注意事项：一个块默认为64K.
		  外扩块实际为64K.
		  内部:1xx以页为单位，小容量中容量为1K，大容量互联型为2K。
			   4xx以扇区为单位,为16K~128K
*******************************************************************************************/
u8 IapEraserBulk(u32 u32BulkAddr)
{
	u8 u8Flag = 0x00;
	
	#if (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_EXFLASH_USE)
		u8Flag = W25QXX_EraseBulk(u32BulkAddr);
	#elif (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_INFLASH_USE)
		InFlashErasePage(u32BulkAddr, u32BulkAddr + THE_DEV_PRG_BULK_SIZE - 0x01);
	#else
		u8Flag = 0x00;
	#endif
	
	return u8Flag;
}

/*******************************************************************************************
**函数名称：IapEraserSector
**函数作用：删除一个扇区
**函数参数：u32BulkAddr:需要操作的起始地址
**函数输出：操作是否成功。SUCCESS:操作成功(0x00);ERROR_UP_OVERFLOW:地址超过最大地址，溢出(0x01)
**注意事项：无
*******************************************************************************************/
u8 IapEraserSector(u32 u32SectorAddr)
{
	u8 u8Flag = 0x00;

	#if (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_EXFLASH_USE)
		u8Flag = W25QXX_EraseSector(u32SectorAddr);
	#elif (BSP_PRG_STORAGE_FLASH_FLAG == BSP_PRG_STORAGE_INFLASH_USE)
		InFlashErasePage(u32SectorAddr, u32SectorAddr + THE_DEV_PRG_SECTOR_SIZE - 0x01);
	#else
		u8Flag = 0x00;
	#endif	
	
	return u8Flag;
}

/*******************************************************************************************
**函数名称：void UpdateEndProcess(uint32_t flag)
**函数作用：更新完成后的处理函数
**函数参数：flag 更新成功与否
**函数输出：无
**注意事项：无
*******************************************************************************************/
void UpdateEndProcess(uint32_t flag)
{
	#if (BSP_PLATFORM == BSP_PLATFORM_LX)
//		u8 u8Buf[0x20];
		sDevParaInfo tmp;
	#else
		u32 u32Temp = 0x00;	
	#endif
	
	#if (BSP_PLATFORM == BSP_PLATFORM_LX)
//		IapReadBuf(u8Buf, MY_DEV_TYPE_ADDRESS, sizeof(u8Buf));
		IapReadBuf(u8Buf, MY_DEV_TYPE_ADDRESS, sizeof(sDevParaInfo));
		IapEraserSector(MY_DEV_TYPE_ADDRESS);//擦除块
	tmp.updateAddr = 0x00;      //立即更新标志
	tmp.updateSucceed = flag;   //更新失败标志
	if(tmp.notDwnlUpdate == PROG_NOT_DOWNLOAD_UPDATE_FLAG)
		tmp.notDwnlUpdate = 0x00;
	IapWriteBuf(u8Buf, MY_DEV_TYPE_ADDRESS, sizeof(sDevParaInfo));	
//		u8Buf[(APP_UPDATE_OFFSET_ADDRESS&0xFF)] = 0x00;//立即更新标志
//		u8Buf[(APP_PRGUPDATE_SUCCEED_ADDRESS&0xFF)] = flag;//更新失败标志
//		if(u8Buf[(APP_NOT_DWNL_UPDATE_ADDRESS&0xFF)] == PROG_NOT_DOWNLOAD_UPDATE_FLAG)
//			u8Buf[(APP_NOT_DWNL_UPDATE_ADDRESS&0xFF)] = 0x00;
//		IapWriteBuf(u8Buf, MY_DEV_TYPE_ADDRESS, sizeof(u8Buf));	
	#else
		IapFlagClear(APP_UPDATE_OFFSET_ADDRESS);
		/**写入程序更新成功标志**/
		u32Temp = flag;
		IapWriteBuf((u8 *)&u32Temp, APP_PRGUPDATE_SUCCEED_ADDRESS, 0x04);
		
		if(IapReadFlag(APP_NOT_DWNL_UPDATE_ADDRESS) == PROG_NOT_DOWNLOAD_UPDATE_FLAG)
			IapFlagClear(APP_NOT_DWNL_UPDATE_ADDRESS);	
	#endif
}

/*******************************************************************************************
**函数名称：LcdClear
**函数作用：清屏函数
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
#if (BSP_LCD_FLAG == BSP_LCD_USE)
	void IapLcdClear(void)
	{
		LcdClr(0);
	}
#endif

/*******************************************************************************************
**函数名称：ShowStr
**函数作用：显示字符串
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
#if (BSP_LCD_FLAG == BSP_LCD_USE)
	void IapShowStr(u16 u16Inv, const u8 *cu8Str, u16 u16Row, u16 u16Col)
	{
		ShowStr16b(u16Inv, cu8Str, u16Row, u16Col);
	}
#endif


