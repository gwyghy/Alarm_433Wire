/********************************************************************************
* �ļ����ƣ�	iapupdate.h
* ��	�ߣ�	������   
* ��ǰ�汾��   	V1.0
* ������ڣ�    2014.02.08
* ��������: 	����iapupdate.hͷ�ļ�			
* ��ʷ��Ϣ��   
*           	�汾��Ϣ     ���ʱ��      ԭ����        ע��
*
*       >>>>  �ڹ����е�λ��  <<<<
*          	  3-Ӧ�ò�
*           �� 2-Э���
*             1-Ӳ��������
*********************************************************************************
* Copyright (c) 2014,������������޹�˾ All rights reserved.
*********************************************************************************/
#ifndef __IAPUPDATE_H__
#define __IAPUPDATE_H__
/********************************************************************************
* .hͷ�ļ�
*********************************************************************************/
//#include "stm32f4xx.h"  
//#include "stm32f4xx_hal_flash.h"
//#include "stm32f4xx_hal_def.h"
#include "gd32f30x.h"
/*********************************************************************************/

/********************************************************************************
* #define�궨�弰��������
*********************************************************************************/
/**ʹ�õ�Ӳ��ƽ̨����****/
#define USE_HAL_LIB
#define	BSP_PLATFORM_M3			0x01
#define	BSP_PLATFORM_M4			0x02
#define	BSP_PLATFORM_M2			0x03
#define	BSP_PLATFORM_LX			0x04//�͹���ϵ��
#define BSP_PLATFORM_L4     	0x05 //�͹���L4ϵ��
#define BSP_PLATFORM_MAX    	0x06 
#define BSP_PLATFORM			BSP_PLATFORM_L4//��������֮һ��������Ч

/**�Ƿ�ʹ�ò���ϵͳ����***/
#define	BSP_OS_USE				0x01
#define BSP_OS_NO_USE			0x02
#define BSP_OS_FLAG				BSP_OS_NO_USE//��������֮һ��������Ч

/**�Ƿ�ʹ��LCD��ʾ***/
#define	BSP_LCD_USE				0x01
#define BSP_LCD_NO_USE			0x02
#define BSP_LCD_FLAG			BSP_LCD_NO_USE//��������֮һ��������Ч

/**�Ƿ�ʹ��BEEP***/
#define	BSP_BEEP_USE			0x01
#define BSP_BEEP_NO_USE			0x02
#define BSP_BEEP_FLAG			BSP_BEEP_NO_USE//��������֮һ��������Ч

/**�Ƿ�ʹ������FLASH��־***/
#define	BSP_PRG_STORAGE_EXFLASH_USE		0x01//ʹ������FLASH
#define	BSP_PRG_STORAGE_INFLASH_USE		0x02//ʹ��CPU�ڲ�FLASH
#define BSP_PRG_STORAGE_FLASH_FLAG		BSP_PRG_STORAGE_INFLASH_USE//��������֮һ��������Ч

/**���Ա�־****/
#define	DEBUG_MODEL_ENABLE		0x01
#define	DEBUG_MODEL_DISABLE		0x02
#define DEBUG_MODEL				DEBUG_MODEL_DISABLE//��������֮һ��������Ч

//ͨѶЭ���У��汾��Ϣ�Ľṹ�嶨�塣�ܹ�ռ��256���ֽڣ�64���֡����ֽ���ǰ�����ֽ��ں�
typedef struct
{
	u32	u32PrgDevType;//����汾���豸����
	u32 u32TargetBoardType;//Ŀ�������
	u32 u32PrgVer;//Ӧ�ó���İ汾��
	u32 u32PrgSize;//Ӧ�ó���Ĵ�С
	u32	u32PrgDate;//Ӧ�ó�����������
	u32	u32PrgEncryptType;//Ӧ�ó���ļ����㷨��0x00Ϊ�޼���	
	u32 u32PrgWriteBaseaddr;//����д��Ļ���ַ
	u32 u32PrgCrc16;//Ӧ�ó����CRCУ��ֵ
	u32	reserved[55];//Ԥ����Ϣ��52����
	u32 u32VerInfCrc16;//�����汾��Ϣ�����CRCУ�飬ΪCRC16��ֻռ�����е������ֽ�
}PROGRAM_VERSION_TYPE;

/**��Ӧ���豸��������Ӧ������FLASH����洢��ַ**/
typedef struct
{
	u32 u32DevType;
	u32 u32NorPrgBaseAddr;
}PRG_STORAGE_BASEADDR_TYPE;

typedef  void (*pFunction)(void);
/********************************************************************************
* ��������
*********************************************************************************/
/*���´���ǰ��Ҫ�趨�õ�ַ*/
/*�ݶ�bootloaderռ��16K�ڴ棬Ӧ�ó����0x08040000��ʼ��0x0802 0FFF*/
#if (defined(STM32F446xx) || defined(GD32F30X_CL))
	#define IN_FLASH_BOOTLOADER_ADDR		0x08000000//STM32������������ʼ��ַ
	#define IN_FLASH_APP_ADDR				0x08004000//STM32��Ӧ�ó�����ʼ��ַ

	#define IN_FLASH_ADDRESS_MAX			0x08020FFF//STM32��Ӧ�ó�����flash����ߵ�ַ
	//ƫ������ַ
	#define IN_CODE_FLASH_SIZE_MAX			(IN_FLASH_ADDRESS_MAX - IN_FLASH_APP_ADDR + 0x01)//STM32�п������ɵ���������																									
	#define VTOR_OFFSET						(IN_FLASH_APP_ADDR - 0x08000000)//Ӧ�ó����ж�������ƫ�Ƶ�ַ
	//iapLoadCode�ڻ�������С													
	#define LOAD_CODE_TEMPBUF_MAX_LEN		128//��ֵ���Ϊ65535�����ұ�����4�ı���

	//SPI FLASH�еĵ�ַ��ƫ�Ƶ�ַ����
	#if ((BSP_PRG_STORAGE_FLASH_FLAG	==	BSP_PRG_STORAGE_INFLASH_USE))
		#define UWB_DEV_PROGRAM_BASE_ADDRESS	0x08021000//uwb�ĳ����Ż���ַ
		
		#define MY_DEV_TYPE_ADDRESS          	0x0803E000//�����豸���ʹ�ŵĻ���ַ
		#define APP_WRITE_BASEADDRESS        	(MY_DEV_TYPE_ADDRESS + 4)//Ӧ�ó���д��Ļ���ַ//�����⣬2013.08.26����	
		#define APP_DOWNLOAD_OFFSET_ADDRESS  	(APP_WRITE_BASEADDRESS + 4)//�汾��Ϣ�г���������ɱ�־�ı����ַ
		#define APP_UPDATE_OFFSET_ADDRESS	   	(APP_DOWNLOAD_OFFSET_ADDRESS + 4)//�汾��Ϣ�г����������±�־�ı����ַ
		#define	APP_PRGUPDATE_SUCCEED_ADDRESS	(APP_UPDATE_OFFSET_ADDRESS + 4)//�汾��Ϣ�г�����³ɹ���־�Ĵ���	
		#define APP_DWNL_NOT_UPDATE_ADDRESS		(APP_PRGUPDATE_SUCCEED_ADDRESS +4)//���س�����ɣ�δ���и��±�־�����˱�־λΪ��ʱ������������Խ��г���ı��ݡ�01H��δ���£�00H(FF������)���Ѿ������˸���
		#define APP_NOT_DWNL_UPDATE_ADDRESS		(APP_DWNL_NOT_UPDATE_ADDRESS + 4)//δ���س��򣬵�ѡ���������±��ء�01H��δ���أ�00H(FF������)���Ѿ������˸���	
		typedef struct
		{
			uint32_t devType;   		//�豸����
			uint32_t appWriteDir; 		//Ӧ�ó���д��Ļ���ַ
			uint32_t dwonLoadAddr;      ////�汾��Ϣ�г���������ɱ�־�ı����ַ
			uint32_t updateAddr;        //�汾��Ϣ�г����������±�־�ı����ַ
			uint32_t updateSucceed;     //�汾��Ϣ�г�����³ɹ���־�Ĵ���	
			uint32_t dwnlNotUpdate;    //���س�����ɣ�δ���и��±�־�����˱�־λΪ��ʱ������������Խ��г���ı��ݡ�01H��δ���£�00H(FF������)���Ѿ������˸���
			uint32_t notDwnlUpdate;    //δ���س��򣬵�ѡ���������±��ء�01H��δ���أ�00H(FF������)���Ѿ������˸���	
		}sDevParaInfo;

	#elif (BSP_PRG_STORAGE_FLASH_FLAG	== BSP_PRG_STORAGE_EXFLASH_USE)
		#error "The UWB has no exflash"
	#endif

	/**SPI FLAS�г����С����***/
	#if ((BSP_PRG_STORAGE_FLASH_FLAG	==	BSP_PRG_STORAGE_INFLASH_USE))
		#define UWB_DEV_PROGRAM_SIZE			0x1CF00//��Ա��λģ��ĳ����С(�����汾��Ϣ)��115.75K
	#elif (BSP_PRG_STORAGE_FLASH_FLAG	== BSP_PRG_STORAGE_EXFLASH_USE)
		#error "The UWB has no exflash"
	#endif

	/**SPI FLAS�г����С����***/
	#if ((BSP_PRG_STORAGE_FLASH_FLAG	==	BSP_PRG_STORAGE_INFLASH_USE))
		#define UWB_DEV_PROGRAM_SIZE			0x1CF00//��Ա��λģ��ĳ����С(�����汾��Ϣ)��115.75K
	#elif (BSP_PRG_STORAGE_FLASH_FLAG	== BSP_PRG_STORAGE_EXFLASH_USE)
		#error "The UWB Module has no exflash"
	#endif
#endif /*STM32L412xx*/

/**SPI FLAS�汾��Ϣ��С����***/
#define DEV_PROGRAM_VERSION_SIZE		0x00100//���������豸���汾��Ϣ���ܳ��ȣ�256�ֽ�

//��ų���ʱ��ص�ƫ�Ƶ�ַ����
#define PROG_DEVTYPE_OFFSET_ADDRESS		0x0000//�汾��Ϣ���豸���͵�ƫ�Ƶ�ַ
#define PROG_TARGETYPE_OFFSET_ADDRESS	0x0004//�汾��Ϣ��Ŀ������͵�ƫ�Ƶ�ַ
#define PROG_VERSION_OFFSET_ADDRESS		0x0008//�汾��Ϣ�а汾�ŵ�ƫ�Ƶ�ַ
#define PROG_LENGTH_OFFSET_ADDRESS		0x000C//�汾��Ϣ�г����С��ƫ�Ƶ�ַ

#define PROG_DATE_OFFSET_ADDRESS		0x0010//�汾��Ϣ�г���������ڵ�ƫ�Ƶ�ַ
#define PROG_ENCRPYT_OFFSET_ADDRESS		0x0014//�汾��Ϣ�г�������㷨��ƫ�Ƶ�ַ

#define PROG_WRITE_BASE_ADDRESS	   		0x0018//�汾��Ϣ�г���д���׵�ַ��ƫ��
#define PROG_CRCL_OFFSET_ADDRESS	   	0x001C//�汾��Ϣ�г���CRCУ��ĵ��ֽ�
#define PROG_CRCH_OFFSET_ADDRESS	   	0x001D//�汾��Ϣ�г���CRCУУ��ĸ��ֽ�

#define PROG_MESS_OFFSET_ADDRESS	   	0x0020//�汾��Ϣ�г����޶����ݵ�ƫ�Ƶ�ַ

#define PROG_VER_CRCL_OFFSET_ADDRESS	0x00FC//�汾��Ϣ�����ֶ�CRCУ����ֽڵı����ַ
#define PROG_VER_CRCH_OFFSET_ADDRESS	0x00FD//�汾��Ϣ�����ֶ�CRCУ����ֽڵı����ַ

#define PROG_CODE_OFFSET_ADDRESS	  	0x00100//��������ƫ�Ƶ�ַ

//һЩ��־
#define RROG_DOWNLOAD_FLAG				(0x01)//�����Ѿ�������ɱ�־
#define RROG_UPDATE_FLAG				(0x01)//������Ҫ�������±�־
#define PROG_NOT_DOWNLOAD_UPDATE_FLAG	(0x01)//δ�������س�����Ҫ�������±�־

#define PROG_UPDATE_SUCCEED_FLAG		(0x01)//������³ɹ���־
#define PROG_UPDATE_FAIL_FLAG			(0x02)//�������ʧ�ܱ�־

/**ͨѶЭ������������豸����****/
#define NONE_DEV_TYPE					(0x00000000)//�豸������Ч
#define SC_DEV_TYPE						(0x00100A00)//֧�ܿ�����_��Һ��
#define EMVD_DEV_TYPE					(0x00100A01)//��ŷ�_��Һ��
#define HUB_DEV_TYPE					(0x00100A02)//HUB_��Һ��
#define WL_DEV_TYPE 					(0x00100A03)//����ģ��_��Һ��
#define CXB_DEV_TYPE					(0x00100A04)//�����_��Һ��
#define ANGLE_DEV_TYPE					(0x00100A05)//�Ƕ�_��Һ��
#define SS_DEV_TYPE						(0x00100A06)//֧�ܷ�����_��Һ��
#define UWB_DEV_TYPE 					(0x00100A07)//uwb��λģ��_��Һ��
#define YKQ_DEV_TYPE 					(0x00100A08)//ң����_��Һ��
#define KEY_DEV_TYPE         	    	(0x00100A09)//����ģ��_��Һ��
#define WIRELESS_DEV_TYPE         	   	(0x00100A0A)//����ģ��_��Һ��
#define HEIGHT_DEV_TYPE 				(0x00100A0B)//�߶ȴ�����_��Һ��
#define ALARM_DEV_TYPE 					(0x00100A0C)//���ⱨ����_��Һ��

#define ALARM_ANGLE_HEIGHT_DEV_TYPE 	(0x00100A0D)//�߶ȴ�����+�Ƕ���ϴ�����_��Һ��
#define ALARM_UWB_ANGLE_DEV_TYPE 		(0x00100A0E)//���ⱨ����+��Ա��λ+�Ƕ���ϴ�����_��Һ��

#define DYK_DEV_TYPE_MAX				(0x00100A1F)//�豸���͵����ֵ

/**Ŀ����豸���Ͷ���****/
#define TAGET_51_MCU					(0x00000001)
#define TAGET_PIC_MCU					(0x00000002)
#define TAGET_STM32F1_MCU				(0x00000003)
#define TAGET_STM32F2_MCU				(0x00000004)
#define TAGET_STM32F4_MCU				(0x00000005)
#define TAGET_STM32L1_MCU				(0x00000006)
#define TAGET_STM32L4_MCU				(0x00000007)
#define TAGET_STM32F1_HIGH_MCU			(0x00000008)	//΢��ר��	2020.12.10 parry

#define TAGET_GD32F3_MCU				(0x00000009)

/**��Ӧ�����������豸����Ӧ��д���ַ�Լ��ռ��С�ȵĶ���****/
#define THE_DEV_TYPE					UWB_DEV_TYPE
#define THE_TARGET_TYPE					TAGET_GD32F3_MCU//�豸��Ŀ�������
#define	THE_DEV_PRG_STORAGE_BASEADDR	UWB_DEV_PROGRAM_BASE_ADDRESS
#define	THE_DEV_PRG_STORAGE_SIZE		UWB_DEV_PROGRAM_SIZE

#if (BSP_PRG_STORAGE_FLASH_FLAG	== BSP_PRG_STORAGE_EXFLASH_USE)
	#error "The UWB Module has no exflash"//�������塣����ʱ��==�ⲿ������С���ڲ�ʱ��==�ڲ�ҳ��С
#elif (BSP_PRG_STORAGE_FLASH_FLAG	==	BSP_PRG_STORAGE_INFLASH_USE)
	#define THE_DEV_PRG_SECTOR_SIZE		0x800//FLASH_PAGE_SIZE//�������塣����ʱ��==�ⲿ������С���ڲ�ʱ��==�ڲ�ҳ��С
#endif

#define THE_DEV_PRG_BULK_SIZE			0x10000//�鶨�塣64K

#define THE_PRG_STORAGE_BASEADDRESS_MAX	MY_DEV_TYPE_ADDRESS/**�豸����洢������ַ��������ֵ���ж���Ч***/

/**��ʹ�õľ���Ƶ�ʶ�Ӧ��Latency***/
#if STM32 
#define	THE_DEV_FLASH_Latency			FLASH_LATENCY_4  //�ȴ�����
#define	THE_DEV_CPU_VOLTAGE				VoltageRange_3/*!<Device operating range: 1.5V */
#endif
#define	THE_DEV_FLASH_Latency			WS_WSCNT_2  //�ȴ�����
#define	THE_DEV_CPU_VOLTAGE				VoltageRange_3/*!<Device operating range: 1.5V */
/********************************************************************************
* ȫ�ֱ�������
*********************************************************************************/

/********************************************************************************
* ��������
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
