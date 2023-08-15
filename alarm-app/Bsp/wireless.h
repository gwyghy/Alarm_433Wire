/*********************************************************************************************************************************
** 文件名:  wireless.h
** 描　述:  无线管理模块头文件
** 创建人: 	沈万江
** 日　期:  2014-12-26
** 修改人:	
** 日　期:	
**
** 版　本:	V1.0.0.0
** 更新记录:
** 更新记录	：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
**--------------------------------------------------------------------------
**************************Copyright (c) 1998-1999 天津华宁电子技术有限公司技术开发部*************************************************/
#ifndef __WIRELESS_H__
#define __WIRELESS_H__

#include "gd32f30x.h"
#include "protocol.h"

#define	WL_MANAGE_TASK_DELAY_TIME				(10)			// 无线管理任务延时时间 // 10ms
#define WL_SS_ADDR								0xFF			// 服务器无线地址
#define WL_BROADCAST_ADDR						0x00			// 服务器无线地址
// 遥控器控制结构
typedef struct
{
	u16 Step;					//控制步骤
	u16 CtrlSC;					//被控制支架号
	u32 RemoteID;				//遥控器ID
	u32 Timer;					//控制计时器
	u32 CtrlCode;				//控制键码
} REMOTE_CONTROL_s;
// WL发送数据结构
typedef struct
{
	union {
		struct {
			u32 RID:3;				//目标ID(接收方)
			u32 TID:3;				//源ID(发送方)
			u32 FT:10;				//帧类型
			u32 SN:4;				//子帧序列号
			u32 SUM:5;				//总帧数
			u32 SUB:1;				//标明是总帧还是子帧
			u32 ACK:1;				//应答位
			u32 RD:2;				//保留位
		} ID;
		u32 u32Id;
	} u32ID;
	u8  u8FT;					//数据帧类型
	u8  u8Ack;					//是否需要应答
	u8  u8Addr;					//无线数据发送目标地址
	u8  u8WlData[64];			//无线数据内容
	u32 u32Len;					//无线数据发送长度
}st_WL_TRANS_DATA;
typedef struct
{
	st_WL_TRANS_DATA WlTransData;//无线发送数据内容
	u8  u8Prio;					//数据帧的优先级
	u8  u8Count;				//无线数据发送次数
	u16 u16WlTransIntv;			//无线数据发送间隔
	u8	u8Next;					//指向下一个数据结构
	u8  u8Prev;					//指向前一个数据结构
}st_WL_TRANS_ITEM;
// 遥控器状态机
enum
{
	STEP_IDLE=0,				//空闲
	STEP_CHECK_BY_IR,			//红外对码
	STEP_CHECK_BY_WL,			//无线对码
	STEP_CONTROL				//控制
};
/*
 *  无线接收数据处理。以下函数应该在任务中周期执行
 */
void Wl_RxDataProc(void);
/*
 *  无线发送数据处理。以下函数应该在任务中周期执行
 */
void Wl_TxDataProc(void);
/*
 *  插入无线发送缓冲区
 */
u32 InsWlTxBuf(st_WL_TRANS_DATA *stWlData, u16 u16FirstTrs);
/*
 *  对无线进行数据收发进行管理
 */
void WL_MNG_Task(void *p_arg);

/*
 *  获取无线传输涉及的遥控器ID，该值和无线RFID的地址相关；返回值，无线传输涉及的遥控器ID；
 */
u32 GetRemoteIdByWl(void);
/*
 * 保存遥控器ID
 */
u32 SaveSRemoteCtrlInfo(u8 u8Id);

/*
 * 获取新的CAN发送序列号
 */
u32 NewCanTxSn(void);

/*
 * 释放接收到无线数据的信号
 */
void MessageForRfidRcved(void);

#endif /* __WIRELESS_H__ */
/*************************** END OF LINES *****************************************/
