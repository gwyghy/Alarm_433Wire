/********************************************************************************
* 文件名称：	iapupdate.h
* 作	者：	马如意   
* 当前版本：   	V1.0
* 完成日期：    2014.02.08
* 功能描述: 	定义iapupdate.h头文件			
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
#ifndef __IAPUPDATE_H__
#define __IAPUPDATE_H__
/********************************************************************************
* .h头文件
*********************************************************************************/
//#include "stm32f4xx.h"  
//#include "stm32f4xx_hal_flash.h"
//#include "stm32f4xx_hal_def.h"
#include "gd32f30x.h"
/*********************************************************************************/

/********************************************************************************
* #define宏定义及类型声明
*********************************************************************************/
/**使用的硬件平台定义****/
#define USE_HAL_LIB
#define	BSP_PLATFORM_M3			0x01
#define	BSP_PLATFORM_M4			0x02
#define	BSP_PLATFORM_M2			0x03
#define	BSP_PLATFORM_LX			0x04//低功耗系列
#define BSP_PLATFORM_L4     	0x05 //低功耗L4系列
#define BSP_PLATFORM_MAX    	0x06 
#define BSP_PLATFORM			BSP_PLATFORM_L4//上述三者之一，其他无效

/**是否使用操作系统定义***/
#define	BSP_OS_USE				0x01
#define BSP_OS_NO_USE			0x02
#define BSP_OS_FLAG				BSP_OS_NO_USE//上述两者之一，其他无效

/**是否使用LCD显示***/
#define	BSP_LCD_USE				0x01
#define BSP_LCD_NO_USE			0x02
#define BSP_LCD_FLAG			BSP_LCD_NO_USE//上述两者之一，其他无效

/**是否使用BEEP***/
#define	BSP_BEEP_USE			0x01
#define BSP_BEEP_NO_USE			0x02
#define BSP_BEEP_FLAG			BSP_BEEP_NO_USE//上述两者之一，其他无效

/**是否使用外扩FLASH标志***/
#define	BSP_PRG_STORAGE_EXFLASH_USE		0x01//使用外扩FLASH
#define	BSP_PRG_STORAGE_INFLASH_USE		0x02//使用CPU内部FLASH
#define BSP_PRG_STORAGE_FLASH_FLAG		BSP_PRG_STORAGE_INFLASH_USE//上述两者之一，其他无效

/**测试标志****/
#define	DEBUG_MODEL_ENABLE		0x01
#define	DEBUG_MODEL_DISABLE		0x02
#define DEBUG_MODEL				DEBUG_MODEL_DISABLE//上述两者之一，其他无效

//通讯协议中，版本信息的结构体定义。总共占用256个字节，64个字。低字节在前，高字节在后。
typedef struct
{
	u32	u32PrgDevType;//程序版本的设备类型
	u32 u32TargetBoardType;//目标板类型
	u32 u32PrgVer;//应用程序的版本号
	u32 u32PrgSize;//应用程序的大小
	u32	u32PrgDate;//应用程序的完成日期
	u32	u32PrgEncryptType;//应用程序的加密算法，0x00为无加密	
	u32 u32PrgWriteBaseaddr;//程序写入的基地址
	u32 u32PrgCrc16;//应用程序的CRC校验值
	u32	reserved[55];//预留信息，52个字
	u32 u32VerInfCrc16;//整个版本信息程序的CRC校验，为CRC16，只占用其中的两个字节
}PROGRAM_VERSION_TYPE;

/**对应于设备类型所对应的外扩FLASH程序存储地址**/
typedef struct
{
	u32 u32DevType;
	u32 u32NorPrgBaseAddr;
}PRG_STORAGE_BASEADDR_TYPE;

typedef  void (*pFunction)(void);
/********************************************************************************
* 常量定义
*********************************************************************************/
/*更新代码前需要设定该地址*/
/*暂定bootloader占用16K内存，应用程序从0x08040000开始至0x0802 0FFF*/
#if (defined(STM32F446xx) || defined(GD32F30X_CL))
	#define IN_FLASH_BOOTLOADER_ADDR		0x08000000//STM32中启动程序起始地址
	#define IN_FLASH_APP_ADDR				0x08004000//STM32中应用程序起始地址

	#define IN_FLASH_ADDRESS_MAX			0x08020FFF//STM32中应用程序在flash中最高地址
	//偏移量地址
	#define IN_CODE_FLASH_SIZE_MAX			(IN_FLASH_ADDRESS_MAX - IN_FLASH_APP_ADDR + 0x01)//STM32中可以容纳的最大代码数																									
	#define VTOR_OFFSET						(IN_FLASH_APP_ADDR - 0x08000000)//应用程序中断向量表偏移地址
	//iapLoadCode内缓冲区大小													
	#define LOAD_CODE_TEMPBUF_MAX_LEN		128//该值最大为65535，而且必须是4的倍数

	//SPI FLASH中的地址及偏移地址设置
	#if ((BSP_PRG_STORAGE_FLASH_FLAG	==	BSP_PRG_STORAGE_INFLASH_USE))
		#define UWB_DEV_PROGRAM_BASE_ADDRESS	0x08021000//uwb的程序存放基地址
		
		#define MY_DEV_TYPE_ADDRESS          	0x0803E000//自身设备类型存放的基地址
		#define APP_WRITE_BASEADDRESS        	(MY_DEV_TYPE_ADDRESS + 4)//应用程序写入的基地址//马如意，2013.08.26增加	
		#define APP_DOWNLOAD_OFFSET_ADDRESS  	(APP_WRITE_BASEADDRESS + 4)//版本信息中程序下载完成标志的编译地址
		#define APP_UPDATE_OFFSET_ADDRESS	   	(APP_DOWNLOAD_OFFSET_ADDRESS + 4)//版本信息中程序立即更新标志的编译地址
		#define	APP_PRGUPDATE_SUCCEED_ADDRESS	(APP_UPDATE_OFFSET_ADDRESS + 4)//版本信息中程序更新成功标志寄存器	
		#define APP_DWNL_NOT_UPDATE_ADDRESS		(APP_PRGUPDATE_SUCCEED_ADDRESS +4)//下载程序完成，未进行更新标志，当此标志位为假时，启动程序可以进行程序的备份。01H：未更新，00H(FF或其他)：已经进行了更新
		#define APP_NOT_DWNL_UPDATE_ADDRESS		(APP_DWNL_NOT_UPDATE_ADDRESS + 4)//未下载程序，但选择立即更新本地。01H：未下载，00H(FF或其他)：已经进行了更新	
		typedef struct
		{
			uint32_t devType;   		//设备类型
			uint32_t appWriteDir; 		//应用程序写入的基地址
			uint32_t dwonLoadAddr;      ////版本信息中程序下载完成标志的编译地址
			uint32_t updateAddr;        //版本信息中程序立即更新标志的编译地址
			uint32_t updateSucceed;     //版本信息中程序更新成功标志寄存器	
			uint32_t dwnlNotUpdate;    //下载程序完成，未进行更新标志，当此标志位为假时，启动程序可以进行程序的备份。01H：未更新，00H(FF或其他)：已经进行了更新
			uint32_t notDwnlUpdate;    //未下载程序，但选择立即更新本地。01H：未下载，00H(FF或其他)：已经进行了更新	
		}sDevParaInfo;

	#elif (BSP_PRG_STORAGE_FLASH_FLAG	== BSP_PRG_STORAGE_EXFLASH_USE)
		#error "The UWB has no exflash"
	#endif

	/**SPI FLAS中程序大小定义***/
	#if ((BSP_PRG_STORAGE_FLASH_FLAG	==	BSP_PRG_STORAGE_INFLASH_USE))
		#define UWB_DEV_PROGRAM_SIZE			0x1CF00//人员定位模块的程序大小(不含版本信息)，115.75K
	#elif (BSP_PRG_STORAGE_FLASH_FLAG	== BSP_PRG_STORAGE_EXFLASH_USE)
		#error "The UWB has no exflash"
	#endif

	/**SPI FLAS中程序大小定义***/
	#if ((BSP_PRG_STORAGE_FLASH_FLAG	==	BSP_PRG_STORAGE_INFLASH_USE))
		#define UWB_DEV_PROGRAM_SIZE			0x1CF00//人员定位模块的程序大小(不含版本信息)，115.75K
	#elif (BSP_PRG_STORAGE_FLASH_FLAG	== BSP_PRG_STORAGE_EXFLASH_USE)
		#error "The UWB Module has no exflash"
	#endif
#endif /*STM32L412xx*/

/**SPI FLAS版本信息大小定义***/
#define DEV_PROGRAM_VERSION_SIZE		0x00100//无论哪种设备，版本信息的总长度，256字节

//存放程序时相关的偏移地址定义
#define PROG_DEVTYPE_OFFSET_ADDRESS		0x0000//版本信息中设备类型的偏移地址
#define PROG_TARGETYPE_OFFSET_ADDRESS	0x0004//版本信息中目标板类型的偏移地址
#define PROG_VERSION_OFFSET_ADDRESS		0x0008//版本信息中版本号的偏移地址
#define PROG_LENGTH_OFFSET_ADDRESS		0x000C//版本信息中程序大小的偏移地址

#define PROG_DATE_OFFSET_ADDRESS		0x0010//版本信息中程序完成日期的偏移地址
#define PROG_ENCRPYT_OFFSET_ADDRESS		0x0014//版本信息中程序加密算法的偏移地址

#define PROG_WRITE_BASE_ADDRESS	   		0x0018//版本信息中程序写入首地址的偏移
#define PROG_CRCL_OFFSET_ADDRESS	   	0x001C//版本信息中程序CRC校验的低字节
#define PROG_CRCH_OFFSET_ADDRESS	   	0x001D//版本信息中程序CRC校校验的高字节

#define PROG_MESS_OFFSET_ADDRESS	   	0x0020//版本信息中程序修订内容的偏移地址

#define PROG_VER_CRCL_OFFSET_ADDRESS	0x00FC//版本信息整个字段CRC校验低字节的编译地址
#define PROG_VER_CRCH_OFFSET_ADDRESS	0x00FD//版本信息整个字段CRC校验高字节的编译地址

#define PROG_CODE_OFFSET_ADDRESS	  	0x00100//程序代码的偏移地址

//一些标志
#define RROG_DOWNLOAD_FLAG				(0x01)//程序已经下载完成标志
#define RROG_UPDATE_FLAG				(0x01)//程序需要立即更新标志
#define PROG_NOT_DOWNLOAD_UPDATE_FLAG	(0x01)//未进行下载程序但需要立即更新标志

#define PROG_UPDATE_SUCCEED_FLAG		(0x01)//程序更新成功标志
#define PROG_UPDATE_FAIL_FLAG			(0x02)//程序更新失败标志

/**通讯协议中所定义的设备类型****/
#define NONE_DEV_TYPE					(0x00000000)//设备类型无效
#define SC_DEV_TYPE						(0x00100A00)//支架控制器_电液控
#define EMVD_DEV_TYPE					(0x00100A01)//电磁阀_电液控
#define HUB_DEV_TYPE					(0x00100A02)//HUB_电液控
#define WL_DEV_TYPE 					(0x00100A03)//无线模块_电液控
#define CXB_DEV_TYPE					(0x00100A04)//程序棒_电液控
#define ANGLE_DEV_TYPE					(0x00100A05)//角度_电液控
#define SS_DEV_TYPE						(0x00100A06)//支架服务器_电液控
#define UWB_DEV_TYPE 					(0x00100A07)//uwb定位模块_电液控
#define YKQ_DEV_TYPE 					(0x00100A08)//遥控器_电液控
#define KEY_DEV_TYPE         	    	(0x00100A09)//键盘模块_电液控
#define WIRELESS_DEV_TYPE         	   	(0x00100A0A)//无线模块_电液控
#define HEIGHT_DEV_TYPE 				(0x00100A0B)//高度传感器_电液控
#define ALARM_DEV_TYPE 					(0x00100A0C)//声光报警器_电液控

#define ALARM_ANGLE_HEIGHT_DEV_TYPE 	(0x00100A0D)//高度传感器+角度组合传感器_电液控
#define ALARM_UWB_ANGLE_DEV_TYPE 		(0x00100A0E)//声光报警器+人员定位+角度组合传感器_电液控

#define DYK_DEV_TYPE_MAX				(0x00100A1F)//设备类型的最大值

/**目标板设备类型定义****/
#define TAGET_51_MCU					(0x00000001)
#define TAGET_PIC_MCU					(0x00000002)
#define TAGET_STM32F1_MCU				(0x00000003)
#define TAGET_STM32F2_MCU				(0x00000004)
#define TAGET_STM32F4_MCU				(0x00000005)
#define TAGET_STM32L1_MCU				(0x00000006)
#define TAGET_STM32L4_MCU				(0x00000007)
#define TAGET_STM32F1_HIGH_MCU			(0x00000008)	//微感专用	2020.12.10 parry

#define TAGET_GD32F3_MCU				(0x00000009)

/**对应程序中自身设备所对应的写入地址以及空间大小等的定义****/
#define THE_DEV_TYPE					UWB_DEV_TYPE
#define THE_TARGET_TYPE					TAGET_GD32F3_MCU//设备的目标板类型
#define	THE_DEV_PRG_STORAGE_BASEADDR	UWB_DEV_PROGRAM_BASE_ADDRESS
#define	THE_DEV_PRG_STORAGE_SIZE		UWB_DEV_PROGRAM_SIZE

#if (BSP_PRG_STORAGE_FLASH_FLAG	== BSP_PRG_STORAGE_EXFLASH_USE)
	#error "The UWB Module has no exflash"//扇区定义。外扩时，==外部扇区大小；内部时，==内部页大小
#elif (BSP_PRG_STORAGE_FLASH_FLAG	==	BSP_PRG_STORAGE_INFLASH_USE)
	#define THE_DEV_PRG_SECTOR_SIZE		0x800//FLASH_PAGE_SIZE//扇区定义。外扩时，==外部扇区大小；内部时，==内部页大小
#endif

#define THE_DEV_PRG_BULK_SIZE			0x10000//块定义。64K

#define THE_PRG_STORAGE_BASEADDRESS_MAX	MY_DEV_TYPE_ADDRESS/**设备程序存储的最大地址，超过此值，判定无效***/

/**所使用的晶振频率对应的Latency***/
#if STM32 
#define	THE_DEV_FLASH_Latency			FLASH_LATENCY_4  //等待周期
#define	THE_DEV_CPU_VOLTAGE				VoltageRange_3/*!<Device operating range: 1.5V */
#endif
#define	THE_DEV_FLASH_Latency			WS_WSCNT_2  //等待周期
#define	THE_DEV_CPU_VOLTAGE				VoltageRange_3/*!<Device operating range: 1.5V */
/********************************************************************************
* 全局变量声明
*********************************************************************************/

/********************************************************************************
* 函数声明
*********************************************************************************/
u8 IapGetPrgStorageAddr(u32 u32DevType, u32 *pStorageAddr);
u8 IapGetPrgSize(u32 u32DevType, u32 *pSize);
u16 InFlashCrc(u32 u32SrcAddr, u32 u32Len);
void Crc16Ccitt(const u8 *u8Buf, u32 u32Len, u16 *u16CheckOld);
void Delay(u32 u32Count);
#if (BSP_PLATFORM	==	BSP_PLATFORM_M3)
	void InFlashErasePage(u32 u32StartSectorAddr,u32 u32EndSectorAddr);
#elif (BSP_PLATFORM	==	BSP_PLATFORM_M4	)
	void InFlashEraseSector(u32 u32StartSectorAddr ,u32 u32EndSectorAddr,uint8_t VoltageRange);
#elif (BSP_PLATFORM	==	BSP_PLATFORM_LX)	
	void InFlashErasePage(u32 u32StartSectorAddr,u32 u32EndSectorAddr);
#endif
u16 IapExFlashCrc(u32 u32SrcAddr, u32 u32Len, u16 *pCrc);
u8 IapLoadCode(u32 u32DestAddr, const u32 u32SrcAddr, u32 u32Len);
void IapJumpToApp(u32 u32AppAddr);
void IapJumpToBoot(uint32_t u32bootAddr);

void IapWriteCodeLen(u32 u32Addr, u32 u32Len);
u32 IapReadCodeLen(u32 u32Addr);

void IapWriteCrc(u32 u32Addr, u32 u32Crc);
u32 IapReadCrc(u32 u32Addr);

u32 IapReadFlag(u32 u32Addr);
u32 IapFlagSet(u32 u32Addr);
u32 IapFlagClear(u32 u32Addr);

u8 IapReadBuf(u8 *pBuffer, u32 u32ReadAddr, u16 u16NumByteToRead);
u8 IapWriteBuf(u8 *pBuffer, u32 u32ReadAddr, u16 u16NumByteToRead);
u8 IapEraserBulk(u32 u32BulkAddr);
u8 IapEraserSector(u32 u32SectorAddr);

void IapLcdClear(void);
void IapShowStr(u16 u16Inv, const u8 *cu8Str, u16 u16Row, u16 u16Col);

void IapDevInit(void);

#endif
