/********************************************************************************
* 文件名称：	CanIap.c
* 作	者：	马如意   
* 当前版本：   	V1.0
* 完成日期：    2015.01.28
* 功能描述: 	完成程序更新功能(传输总线为CAN)。含本架更新、左右邻架、全面更新。本文件隶属于CAN总线处理过程。
* 历史信息：   
*           	版本信息     完成时间      原作者        注释
*
*       >>>>  在工程中的位置  <<<<
*          	  3-应用层
*           √ 2-协议层
*          	  1-硬件驱动层
*********************************************************************************
* Copyright (c) 2014,天津华宁电子有限公司 All rights reserved.
*********************************************************************************/
/********************************************************************************
* .h头文件
*********************************************************************************/
#include "l4x_flash.h"
#include "iapupdate.h"
#include "CanIap.h"
#include "ucos_ii.h"
/********************************************************************************
* #define宏定义
*********************************************************************************/
#define CANIAP_OS_USE_FLAG	1//是否使用OS系统标志
#if CANIAP_OS_USE_FLAG > 0
	#define TICK_TIME			1
#endif
/********************************************************************************
* 常量定义
*********************************************************************************/
#define DOWNLOAD_PROGRAM_FILTER     		0x1FFFF					//传输程序时的过滤字，马如意，2013.07.22增加
#define TRANS_PROG_TIMES					0x02//每一帧数据发送的次数
/********************************************************************************
* 变量定义
*********************************************************************************/
u32 u32RecvBaseAddr,u32RecvOffsetAddr;//基地址,偏移地址
u32 u32RecvTotalPacketNumb;//总包数
u32 u32NowRecvPacketNumb;//当前包数
u32 u32NowRecvPacketNumbBackup;//当前包数的备份
u8 	u8NowRecvPacketNumbTimes;//当前包数接收的次数
u32 u32RecvProgLength;
u16 u16BackupScId;//备份的本架架号

u16 u16BackupScMinid;//备份的最小架号
u16 u16BackupScMaxid;//备份的最小架号
u8 u8BackupLeftErr;//备份的左邻架通讯异常
u8 u8BackupRightErr;//备份的右邻架通讯异常
u32 u32DevType;//u32DevType在传输程序时为程序类型;

/********************************************************************************
* 函数定义
*********************************************************************************/
/*******************************************************************************************
**函数名称：CanRecvProgProc
**函数作用：收到传输的程序帧时的处理
**函数参数：1)RxFrame，所接收的数据帧；2)u16From,16位整型，收到数据帧的方向(左/右)
**函数输出：无
**注意事项：无
** 作　者:	马如意
** 日　期： 2013.07.18
*******************************************************************************************/
void CanIapInint(void)
{
	//传输程序相关变量赋初值
	u32RecvBaseAddr = 0x00;
	u32RecvOffsetAddr = 0x00;//基地址,偏移地址
	u32RecvTotalPacketNumb = 0x00;//总包数
	u32NowRecvPacketNumb = 0x00;//当前包数
	u32NowRecvPacketNumbBackup = 0x01;//总是从第一包开始收
	u32RecvProgLength = 0x00;
	u32DevType = NONE_DEV_TYPE;
	u16BackupScId = 0x00;//备份的本架架号
	u16BackupScMinid = 0x00;//备份的最小架号
	u16BackupScMaxid = 0x00;//备份的最小架号
	u8BackupLeftErr = 0x00;//备份的左邻架通讯异常
	u8BackupRightErr = 0x00;//备份的右邻架通讯异常
}
/*******************************************************************************************
**函数名称：CanIapTimeDlyUs
**函数作用：延时函数(非精确延时，取决于系统时钟)
**函数参数：u32us:延时的时间
**函数输出：无
**注意事项：无
** 作　者:	马如意
** 日　期： 2013.07.18
*******************************************************************************************/
//#if (CANIAP_OS_USE_FLAG == 0)//是否使用了操作系统
	u32 CanIapTimeDlyUs(u32 u32us)
	{
		u32 u32i,u32j;

		for (u32i = 0; u32i < u32us; u32i++)
		{
			for(u32j = 0; u32j < 10; u32j++)
			/* Nothing to do */;
		}
		
		return 0x01;
	}
//#endif
/*******************************************************************************************
**函数名称：CanRecvProgProc
**函数作用：收到传输的程序帧时的处理
**函数参数：1)RxFrame，所接收的数据帧；2)u16From,16位整型，收到数据帧的方向(左/右)
**函数输出：无
**注意事项：无
** 作　者:	马如意
** 日　期： 2013.07.18
*******************************************************************************************/
void CanRecvProgProc(CXB_CAN_FRAME_TYPE *ExCan)
{
	u16 u16DstAddr;
	static u16 u16CrcCalculate = 0x00;
	static u16 u16Crc = 0x00;
	u32 u32Addr = 0x00;
	u32 u32Temp;
//	#if CANIAP_OS_USE_FLAG//是否使用了操作系统
//		OS_TCB pdata;
//	#endif
	u64 u64Data = 0x00;
	
	u16DstAddr = ExCan->u32ID.ID.RID;						//接收ID
	u32NowRecvPacketNumb = ExCan->u32ID.ID.PACKET_NUMB;		//20位
	
	if((ExCan->u32ID.ID.TD_FTYPE != CXB_FRAME_DOWNLOAD_PRG_VERSION) && (u32NowRecvPacketNumb != 0x01))
	{
		if((u32NowRecvPacketNumb - u32NowRecvPacketNumbBackup) > 1)
		{
			u32Temp = 0x01;
			return;
		}
	}
	
	if(u32NowRecvPacketNumb)//不是第0帧时数据的处理
	{
		if(u32NowRecvPacketNumb == u32NowRecvPacketNumbBackup)//接收到上一帧的第二帧数据
			u8NowRecvPacketNumbTimes++;
		/**当收到版本信息最后一帧，此时接收备份为第一帧，此种情况下无需处理**/
		else if((u8NowRecvPacketNumbTimes == 0x01) && (u32NowRecvPacketNumbBackup == 0x01) && (u32NowRecvPacketNumb == 0x20))
			u8NowRecvPacketNumbTimes++;
		else//接收到新的一帧时，需判断上一帧是否接收了n次
		{
			u8NowRecvPacketNumbTimes = 0x01;//新的一帧收到了一次

		}
	}	
	
	u32NowRecvPacketNumbBackup = u32NowRecvPacketNumb;
	
	if(u16DstAddr == (THE_DEV_TYPE & 0xFF))                             //目标ID(接收方)与设备类型一致
	{
		if(ExCan->u32ID.ID.TD_FTYPE == CXB_FRAME_DOWNLOAD_PRG_VERSION)//版本信息
		{
			if((u32NowRecvPacketNumb == 0x01) && (u8NowRecvPacketNumbTimes == 0x01))//第一帧收到第一次时时读取设备类型并擦除所有的块
			{
				u32DevType = (ExCan->u8DT[0x00] | (ExCan->u8DT[0x01] << 8) | (ExCan->u8DT[0x02] << 16) | (ExCan->u8DT[0x03] << 24));
//				if((IapGetPrgStorageAddr(u32DevType, &u32RecvBaseAddr)  == 0) || (IapGetPrgSize(u32DevType, &u32RecvOffsetAddr) == 0))
//				{
//					u32DevType = 0x00;
//					return ;
//				}
				

				if(u32DevType == ALARM_UWB_ANGLE_DEV_TYPE)		//更新应用程序
				{
					u32RecvBaseAddr = THE_DEV_PRG_STORAGE_BASEADDR;
					u32RecvOffsetAddr = THE_DEV_PRG_STORAGE_SIZE;
				}
				else
				{
					u32DevType = 0x00;
					return ;
				}
				
				__disable_fault_irq();
				FLASH_SetLatency(THE_DEV_FLASH_Latency);
				FLASH_Unlock();	
				/**擦除此部分的所有块**/
				for(u32Addr = u32RecvBaseAddr; u32Addr < (u32RecvBaseAddr + u32RecvOffsetAddr); u32Addr += THE_DEV_PRG_SECTOR_SIZE)
				{
					IapEraserSector(u32Addr);//擦除块
				}

				__enable_fault_irq();		

				u32RecvOffsetAddr = 0x00;
				u8NowRecvPacketNumbTimes = 0x01;
				if( sizeof(PROGRAM_VERSION_TYPE) % 0x08)
					u32Temp = sizeof(PROGRAM_VERSION_TYPE) / 0x08 + 0x01;
				else
					u32Temp = sizeof(PROGRAM_VERSION_TYPE) / 0x08;
				u32RecvTotalPacketNumb = u32Temp;
				
			}
			if(u32NowRecvPacketNumb == 0x02)//第二帧时读取程序大小并计算总包数
			{
				u32RecvProgLength =  (ExCan->u8DT[0x04] | (ExCan->u8DT[0x05] << 8));
				u32RecvProgLength |= ((ExCan->u8DT[0x06] << 16) | (ExCan->u8DT[0x07] << 24));
			}
			//仅存储1~4包
			if((u32NowRecvPacketNumb <= 0x04) && (u8NowRecvPacketNumbTimes == 0x01))
			{
				if(u32NowRecvPacketNumb == 0X04)
					u32NowRecvPacketNumb = 0x04;
				/*判断写入地址是否在程序存储范围内*/
				__disable_fault_irq();
				FLASH_SetLatency(THE_DEV_FLASH_Latency);
				FLASH_Unlock();		
			
				u32Addr = u32RecvBaseAddr + (u32NowRecvPacketNumb - 0x01) * 0x08;			
				if((u32Addr >= THE_DEV_PRG_STORAGE_BASEADDR) && (u32Addr < MY_DEV_TYPE_ADDRESS))
					IapWriteBuf(&(ExCan->u8DT[0x00]), u32Addr, (u8)(ExCan->u16DLC));//数据		

				__enable_fault_irq();						
			}
			if(u32NowRecvPacketNumb == u32RecvTotalPacketNumb)//所有的版本信息发送完毕
			{
				u32RecvOffsetAddr = PROG_CODE_OFFSET_ADDRESS;//偏移地址
				u32NowRecvPacketNumb = 0x01;//即将收到的包数
				u32NowRecvPacketNumbBackup = u32NowRecvPacketNumb;
				if( u32RecvProgLength % 0x08)
					u32Temp = u32RecvProgLength / 0x08 + 0x01;
				else
					u32Temp = u32RecvProgLength / 0x08;
				u32RecvTotalPacketNumb = u32Temp;	
			}			
		}
		else
		{
			/**当收到程序代码的时候***/
			if(u32NowRecvPacketNumb && u32RecvTotalPacketNumb && (u32NowRecvPacketNumb < u32RecvTotalPacketNumb))
			{
				u32Addr = (u32NowRecvPacketNumb - 0x01) * 0x08;
				u32Addr += u32RecvBaseAddr + PROG_CODE_OFFSET_ADDRESS;
				if((u32Addr >= THE_DEV_PRG_STORAGE_BASEADDR) && (u32Addr < MY_DEV_TYPE_ADDRESS) && (u8NowRecvPacketNumbTimes == 0x01))
				{
					__disable_fault_irq();
					FLASH_SetLatency(THE_DEV_FLASH_Latency);
					FLASH_Unlock();	
					
					IapWriteBuf(&(ExCan->u8DT[0x00]), u32Addr, (u8)(ExCan->u16DLC));//程序
		
					__enable_fault_irq();		
				}
			}		
			else//最后一帧数据的处理
			{			
				/*排除非正常进入的条件*/					
				if(!(u32NowRecvPacketNumb && u32RecvTotalPacketNumb && (u32NowRecvPacketNumb == u32RecvTotalPacketNumb)))
				{			
					return;
				}

				u32Addr = (u32NowRecvPacketNumb - 0x01) * 0x08;
				u32Addr += u32RecvBaseAddr + PROG_CODE_OFFSET_ADDRESS;
				if((u32Addr >= THE_DEV_PRG_STORAGE_BASEADDR) && (u32Addr < MY_DEV_TYPE_ADDRESS) && (u8NowRecvPacketNumbTimes == 0x01))
				{
					__disable_fault_irq();
					FLASH_SetLatency(THE_DEV_FLASH_Latency);
					FLASH_Unlock();		
					
					IapWriteBuf(&(ExCan->u8DT[0x00]), u32Addr, (u8)(ExCan->u16DLC));//程序
	
					__enable_fault_irq();		
				}
				
				#if CANIAP_OS_USE_FLAG//是否使用了操作系统
					OSTimeDly(100 / TICK_TIME);//延时100ms
				#else
					CanIapTimeDlyUs(1000);
				#endif
				
				u16CrcCalculate = 0x00;
				for(u32Addr = u32RecvBaseAddr + PROG_CODE_OFFSET_ADDRESS; u32Addr < (u32RecvBaseAddr + PROG_CODE_OFFSET_ADDRESS + (u32NowRecvPacketNumb - 0x01) * 0x08 + (u8)(ExCan->u16DLC)); u32Addr += 0x20000)
				{
					if(((u32RecvBaseAddr + PROG_CODE_OFFSET_ADDRESS + (u32NowRecvPacketNumb - 0x01) * 0x08 + (u8)(ExCan->u16DLC)) - u32Addr) >= 0x20000)
						u16CrcCalculate = IapExFlashCrc(u32Addr, 0x20000, &u16CrcCalculate);//临时屏蔽	
					else
						u16CrcCalculate = IapExFlashCrc(u32Addr, (u32RecvBaseAddr + PROG_CODE_OFFSET_ADDRESS + (u32NowRecvPacketNumb - 0x01) * 0x08 + (u8)(ExCan->u16DLC)-u32Addr), &u16CrcCalculate);//临时屏蔽
					#if CANIAP_OS_USE_FLAG//是否使用了操作系统
						OSTimeDly(100/TICK_TIME);//延时1500ms，以便调度看门狗任务
					#else
						CanIapTimeDlyUs(1000);
					#endif
				}

				u16Crc = 0x00;
				IapReadBuf((u8 *)&u16Crc, u32RecvBaseAddr + PROG_CRCL_OFFSET_ADDRESS, 0x02);
				if(u16CrcCalculate == u16Crc)		//校验成功
				{
					u16CrcCalculate = 0x00;
					u16CrcCalculate = IapExFlashCrc(u32RecvBaseAddr + PROG_DEVTYPE_OFFSET_ADDRESS, DEV_PROGRAM_VERSION_SIZE - 0x04, &u16CrcCalculate);//计算CRC校验
					u32Temp = (u32)u16CrcCalculate;
					/*判断写入地址是否在程序存储范围内*/
					u32Addr = u32RecvBaseAddr + PROG_VER_CRCL_OFFSET_ADDRESS;			
					if((u32Addr >= THE_DEV_PRG_STORAGE_BASEADDR) && (u32Addr < MY_DEV_TYPE_ADDRESS) && (u8NowRecvPacketNumbTimes == 0x01))	
					{					
						u64Data = u32Temp;
						u64Data <<= 32;
						IapReadBuf((u8 *)&u32Temp,  u32RecvBaseAddr + PROG_VER_CRCL_OFFSET_ADDRESS - 0x04, 0x04);	
						u64Data |= u32Temp;
						__disable_fault_irq();
						FLASH_SetLatency(THE_DEV_FLASH_Latency);
						FLASH_Unlock();	
						IapWriteBuf((u8 *)&u64Data, u32RecvBaseAddr + PROG_VER_CRCL_OFFSET_ADDRESS - 0x04, 0x08);
						FLASH_Lock();	
						__enable_fault_irq();
					}
				#if (THE_DEV_TYPE == SC_DEV_TYPE)
					#ifndef DYK_SS
						if (u32DevType == SC_DEV_TYPE)
					#else
						if(u32DevType == SS_DEV_TYPE)
					#endif
				#else
					if(u32DevType == ALARM_UWB_ANGLE_DEV_TYPE)//设备类型相符，写入相关标识，立即更新
				#endif
					{
						__disable_fault_irq();
						FLASH_SetLatency(THE_DEV_FLASH_Latency);
						FLASH_Unlock();
						
						//写入设备类型字
						IapEraserSector(MY_DEV_TYPE_ADDRESS);//擦除一个扇区
						
						/* 程序启动首地址*/
						IapReadBuf((u8 *)&u32Temp, u32RecvBaseAddr + PROG_WRITE_BASE_ADDRESS, 0x04);						
						u64Data = (u64)u32Temp;
						u64Data <<= 32;
						u64Data |= (u64)u32DevType;
						/**写入设备类型以及启动首地址***/
						IapWriteBuf((u8 *)&u64Data, MY_DEV_TYPE_ADDRESS, 0x08);

						/*判断写入地址是否在程序存储范围内*/
						u32Addr = APP_UPDATE_OFFSET_ADDRESS;
						if((u32Addr >= MY_DEV_TYPE_ADDRESS) && (u32Addr < (APP_NOT_DWNL_UPDATE_ADDRESS + 0x04)))
						{
							u64Data = RROG_UPDATE_FLAG;//写入立即更新标志
							u64Data <<= 32;
						}
						u32Addr = APP_DOWNLOAD_OFFSET_ADDRESS;
						if((u32Addr >= MY_DEV_TYPE_ADDRESS) && (u32Addr < (APP_NOT_DWNL_UPDATE_ADDRESS + 0x04)))
						{							
							u64Data |= RROG_DOWNLOAD_FLAG;//写入下载完成标志
							IapWriteBuf((u8 *)&u64Data, APP_DOWNLOAD_OFFSET_ADDRESS, 0x08);
						}
						
						FLASH_Lock();
						__enable_fault_irq();
						
						#if CANIAP_OS_USE_FLAG//是否使用了操作系统
							OSTimeDly(100 / TICK_TIME);//延时100ms，以便将数据顺利发出
						#else
							CanIapTimeDlyUs(1000);						
						#endif
						
						//执行跳转
						IapJumpToBoot(IN_FLASH_BOOTLOADER_ADDR);
					}
				}
				else //校验不正确
				{
					__disable_fault_irq();
					FLASH_Lock();	
					__enable_fault_irq();
					u32DevType = NONE_DEV_TYPE;	
					u32RecvProgLength = 0x00;
					u32NowRecvPacketNumb = 0x00;
					u32RecvTotalPacketNumb = 0x00;
					u32RecvOffsetAddr = 0x00;
					u32RecvBaseAddr = 0x00;						
				}
				
				#if CANIAP_OS_USE_FLAG//是否使用了操作系统
//					/**恢复不必要的任务**/
//					OSTaskQuery(WL_MANAGE_TASK_PRIO, &pdata);			//查询处理任务是否挂起
//					if(pdata.OSTCBStat == OS_STAT_SUSPEND)
//					{
//						OSTaskResume(WL_MANAGE_TASK_PRIO);				//唤醒任务
//					}	
//					
//					/**恢复不必要的任务**/
//					OSTaskQuery(INFRARED_COM_TASK_PRIO, &pdata);			//查询处理任务是否挂起
//					if(pdata.OSTCBStat == OS_STAT_SUSPEND)
//					{
//						OSTaskResume(INFRARED_COM_TASK_PRIO);				//唤醒任务
//					}	
//					
//					/**恢复不必要的任务**/
//					OSTaskQuery(LOGIC_TASK_PRIO, &pdata);			//查询处理任务是否挂起
//					if(pdata.OSTCBStat == OS_STAT_SUSPEND)
//					{
//						OSTaskResume(LOGIC_TASK_PRIO);				//唤醒任务
//					}	

//					/**恢复不必要的任务**/
//					OSTaskQuery(CAN_RCV_TASK_PRIO, &pdata);			//查询处理任务是否挂起
//					if(pdata.OSTCBStat == OS_STAT_SUSPEND)
//					{
//						OSTaskResume(CAN_RCV_TASK_PRIO);				//唤醒任务
//					}
//					
//					/**恢复不必要的任务**/
//					OSTaskQuery(CAN_TASK_PRIO, &pdata);			//查询处理任务是否挂起
//					if(pdata.OSTCBStat == OS_STAT_SUSPEND)
//					{
//						OSTaskResume(CAN_TASK_PRIO);				//唤醒任务
//					}
				#endif
			 }
		}
	}			
}


