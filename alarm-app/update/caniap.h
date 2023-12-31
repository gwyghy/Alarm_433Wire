/********************************************************************************
* 文件名称：	CanIap.h
* 作	者：	马如意   
* 当前版本：   	V1.0
* 完成日期：    2015.01.28
* 功能描述: 	定义caniap.h头文件
* 历史信息：   
*           	版本信息     完成时间      原作者        注释
*
*       >>>>  在工程中的位置  <<<<
*          	  3-应用层
*          √  2-协议层
*             1-硬件驱动层
*********************************************************************************
* Copyright (c) 2014,天津华宁电子有限公司 All rights reserved.
*********************************************************************************/
#ifndef _CAN_IAP_H_
#define _CAN_IAP_H_
/********************************************************************************
* .h头文件
*********************************************************************************/
//#include "stm32f4xx.h"  
#include "gd32f30x.h"
/********************************************************************************
* #define宏定义
*********************************************************************************/

/********************************************************************************
* 常量定义
*********************************************************************************/
//CXB CAN数据帧结构
typedef __packed struct
{
	__packed union
	{
		__packed struct
		{
			unsigned int	RID:3;					//接收方ID
			unsigned int	TD_FTYPE:3;				//发送方ID或帧类型
			unsigned int	PACKET_NUMB:20;		    //包ID
			unsigned int	ACK:1;					//要求确认位
			unsigned int	RD:2;					//包含协议中的IMD（8）、RD（2）、和NC（3）
			unsigned int 	RESVERD:3;				//预留信息
		}ID;
		u32		u32Id;
	} u32ID; 
	u8	u8DT[8];					//数据字节
	u16	u16DLC;						//数据字节数
} CXB_CAN_FRAME_TYPE;


/**帧类型定义(当为传输程序时)***/
enum
{
	CXB_FRAME_DOWNLOAD_PRG_REQ = 0x00,//请求下载程序
	CXB_FRAME_DOWNLOAD_PRG_VERSION = 0x01,//下载版本信息
	CXB_FRAME_DOWNLOAD_PRG_CODE	 = 0x02,//下载程序代码

	CXB_FRAME_BACKUP_PRG_REQ	 = 0x03,//请求备份程序
	CXB_FRAME_BACKUP_PRG_VERSION	 = 0x04,//备份版本信息	
	CXB_FRAME_BACKUP_PRG_CODE	 = 0x05,//备份程序代码

	CXB_FRAME_PRG_VERSION_CRC	=	0x06,//版本信息的校验帧
	CXB_FRAME_PRG_CODE_CRC	= 0x07,//程序代码的校验帧	

	FRAME_TYPE_MAX		= 0x08//帧类型的最大值
};

/**RD类型定义(2个bit)****/
typedef enum
{
	RD_GLOBAL_FRAME = 0x00,		//普通帧
	RD_PRG_FRAME = 0x01,		//传递程序帧	
	RD_RESVERD1 = 0x02,			//预留1
	RD_RESVERD2 = 0x03,			//预留2	
	RD_MAX = 0x04,				//最大值，无效	
}STFRAME_RD_TYPE;

/**进行程序更新时的方向***/
typedef enum
{
	CANIAP_LEFTWORD,//左架
	CANIAP_RIGHTWORD	,//右项
	CANIAP_LEFTRIGHTWORD,//左右项	
	CANIAP_EXCANWORD,//外围CAN
}CANIAP_TRANSWAY_TYPE;
/********************************************************************************
* 全局变量声明
*********************************************************************************/

/********************************************************************************
* 函数声明
*********************************************************************************/
void CanIapInint(void);

//当本设备不为为支架控制器时，进行如下处理
void CanRecvProgProc(CXB_CAN_FRAME_TYPE *ExCan);
	
#endif
