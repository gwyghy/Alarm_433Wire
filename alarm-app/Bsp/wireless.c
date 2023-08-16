/*********************************************************************************************************************************
** 文件名:  wireless.c
** 描　述:  无线管理模块
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
#include "includes.h"
#include "can_app.h"
#include "app_cfg.h"

//无线模块的工作状态
WL_WORK_STATUS eCanWLReportMode = WL_INIT_STATE;

REMOTE_CONTROL_s	SRemoteCtrl={(u16)STEP_IDLE,(u16)-1,(u32)-1,0xffffffff/* 定时器关闭 */,(u32)0};//遥控器控制进程定义

/*
 * 保存取出接收到的一帧无线数据的相关变量
 */
#define WL_RX_BUF_MAX		64		//无线数据接收缓冲区维数
#define WL_INFO_LENGTH		13		//经无线传输的扩展CAN数据字节数

u8 WL_RxBuf[WL_RX_BUF_MAX];			//无线接收缓冲区
u32 WL_RxBufCnt=0;					//有效数据计数
u32 WL_RxBufWpt=0;					//无线接收缓冲区写入指针
u32 WL_RxBufRpt=0;					//无线接收缓冲区读出指针

/*
 * 无线发送缓冲区
 */
#define WL_TX_QUEUE_MAX		64
st_WL_TRANS_ITEM WlTxQueue[WL_TX_QUEUE_MAX];		//无线数据发送缓存区
u16	WL_TxQueueCnt = 0;								//缓存内数据项
u16 WL_TxQueueHead = 0;								//无线发送缓冲区写入指针
u16 WL_TxQueueTail = 0;								//无线发送缓冲区读取指针

//无线发送数据序列号
//s_vuWlTxSn = (s_vuWlTxSn + 1) % WL_TX_SN_MAX;
#define WL_TX_SN_MAX			16
static vu16 s_vuWlTxSn = 0;

// 无线接收信号量定义
static OS_EVENT * WLRxSem;		//RFID接收到数据后的信号量

//OS_STK  wl_manage_task_stk[WL_MANAGE_TASK_STK_SIZE];		//开辟任务堆栈
/***********************************************************************************************
** 函 数 名：	WlVarInit()
** 功能描述：	无线管理任务中用到的相关变量的初始化
** 输　  入：	无
** 输　  出：	无
** 作　  者：	沈万江
** 日　  期：	2015.9.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
************************************************************************************************/
static void WlVarInit(void)
{
	u32 i;
	
	for (i = 0; i < WL_TX_QUEUE_MAX; i++)
	{
		WlTxQueue[i].u8Next = 0xff;						//无线数据发送缓存区
		WlTxQueue[i].u8Prev = 0xff;
	}
	WL_TxQueueCnt = 0;								//缓存内数据项
	WL_TxQueueHead = 0;								//无线发送缓冲区写入指针
	WL_TxQueueTail = 0;								//无线发送缓冲区读取指针
}
/*****************************************************************
** 函数名：MessageForRfidRcved
** 输　入：无
** 输　出：无
** 功能描述：RFID数据接收后的回调函数，发出信号量
******************************************************************/
void MessageForRfidRcved(void)
{
	OSSemPost(WLRxSem);	//RFID接收到数据
}
/*****************************************************************
** 函数名：IsMessageForRfidRcved
** 输　入：无
** 输　出：TRUE：接收到信号量；FALSE：没有接收到信号量
** 功能描述：判定是否接收到RFID收到数据的信号量
******************************************************************/
u32 IsMessageForRfidRcved(void)
{
	u8 err;
	 
	OSSemPend(WLRxSem, 0/* WL_MANAGE_TASK_DELAY_TIME/TICK_TIME */, &err);//等待信号量 
	
	if(err == OS_ERR_NONE)								//接收到信号量
		return (TRUE);
	else
		return (FALSE);
}
/***********************************************************************************************
** 函 数 名：	WL_MNG_Task
** 功能描述：	对无线模块的数据收发以及状态进行管理
** 输　  入：	无
** 输　  出：	无
** 作　  者：	沈万江
** 日　  期：	2015.9.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
************************************************************************************************/
void WL_MNG_Task(void *p_arg)
{
	// 创建无线接收信号量
	WLRxSem = OSSemCreate(0);	
	// 设置RFID接收数据后发送信号量
	SetRcvedBackCallFunc((RCVED_BACK_CALL_FUNC)MessageForRfidRcved);
	// 无线管理任务中用到的相关变量的初始化
	WlVarInit();
	//RFID硬件初始化
	RFID_Init();
	while(1)
	{
		/* 
		 * 等待信号量
		 */
		(void)IsMessageForRfidRcved();
		/*
		 * 无线接收数据处理,以下函数应该在任务中周期执行
		 */
		Wl_RxDataProc();
		//Wl_TxDataProc();

		//执行周期10ms
		//OSTimeDly(WL_MANAGE_TASK_DELAY_TIME/TICK_TIME);
	}
}
/***********************************************************************************************
** 函 数 名：	SaveSRemoteCtrlInfo
** 功能描述：	保存遥控器相关信息
** 输　  入：	无
** 输　  出：	无
** 作　  者：	沈万江
** 日　  期：	2015.9.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 SaveSRemoteCtrlInfo(u8 u8Id)
{
	SRemoteCtrl.RemoteID = u8Id;
	
	return TRUE;
}
/***********************************************************************************************
** 函 数 名：	NewCanTxSn
** 功能描述：	获取CAN数据新的发送流水
** 输　  入：	无
** 输　  出：	无
** 作　  者：	沈万江
** 日　  期：	2015.9.26
** 版	本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日	期	  姓	名					描	  述
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 NewCanTxSn(void)
{
	s_vuWlTxSn = (s_vuWlTxSn + 1) % WL_TX_SN_MAX;
	
	return (s_vuWlTxSn);
}
/***********************************************************************************************
** 函 数 名：	Wl_RxDataProc
** 功能描述：	无线接收数据处理。以下函数应该在任务中周期执行。
** 输　  入：	无
** 输　  出：	无
** 作　  者：	沈万江
** 日　  期：	2015.9.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**					2016-2-3	沈万江			修改帧ID = 6时，添加保存遥控器ID的函数
************************************************************************************************/
can_trasnmit_message_struct TxCan;
extern u8  u8RssiSignalInf;   //信号强度
u8 LiuShuiNumb;
void Wl_RxDataProc(void)
{
	u32	i,j,k,m;
	sCanFrame RxWlCan;
	u8 buf[WL_INFO_LENGTH];
	static u32 RFID_StateInquireIntv = 0;
//	static u32 RFID_Sleep = 0;
	
	i = WL_ReceiveData((unsigned char *)buf, CC1101_RX_FIFO_SIZE);

	if (!i)
	{
		RFID_StateInquireIntv++;
		if (RFID_StateInquireIntv >= 5)		//100ms =5*WL_RX_TASK_INTERVAL
		{
			RFID_StateInquireIntv = 0;
//#if 0
			//获取RDID状态
			j = GetRfidCurStatus();
			switch(j&0xf0)
			{
				case CC1101_STATE_IDLE:
					SetRfidSRX();				//设置RFID进入接收状态
				break;
				case CC1101_STATE_RX:
				case CC1101_STATE_TX:
				case CC1101_STATE_FSTXON:
				case CC1101_STATE_CALIBRATE:
				case CC1101_STATE_SETTLING:
				break;
				case CC1101_STATE_RX_OVERFLOW:
					RfidStrobe(CC1101_SFRX);	//刷新 RX FIFO buffer.
					RfidStrobe(CC1101_SRX);		//进入接收状态	
				break;
				case CC1101_STATE_TX_UNDERFLOW:
					RfidStrobe(CC1101_SFTX);    //刷新 TX FIFO buffer.
					RfidStrobe(CC1101_SRX);		//进入接收状态	
				break;
			}
//#endif
#if 0
			if (RFID_Sleep == 0)
			{
				RFID_Sleep = 1;
				SetRFChipSleepMode();
			}
#endif
		}
	}else
	{
		RFID_StateInquireIntv = 0;
//		RFID_Sleep = 0;
	}
	
	if (i < 5)
		return;
	memmove(WL_RxBuf, buf, i);
	
// 	// 缓冲区数据足够
// 	if (i < (WL_RxBuf[4]+5))			//帧的数据长度
// 		return; 

	RxWlCan.u32ID.u32Id = 0;
	for (i = 0; i < 4; i++)
	{
		RxWlCan.u32ID.u32Id += (u32)WL_RxBuf[i]<<(i<<3);
	}
	/*
	 * 接收到无线数据后，根据FT判断处理方式；如果是总线数据经由无线发出，如果是遥控器数据通过CAN发送到SC
	 * 根据数据帧类型将SN序列号添加到CAN ID中
	 */
	switch (RxWlCan.u32ID.ID.FT)
	{
		case FT_SC_TO_WL_DBUS:					// 2			// SC 发给无线的总线数据，需要应答
			i = 0;
			k = 0;
			j = 0;
			m = 0;
			RxWlCan.u32ID.ID.ACK = eACK;
			RxWlCan.u32ID.ID.FT = FT_WL_TO_SC_DBUS;
			RxWlCan.u32ID.ID.RD = 0;
			RxWlCan.u32ID.ID.RID = SC_RID;
			RxWlCan.u32ID.ID.TID = WL_RID;
			m = WL_RxBuf[4] - (WL_RxBuf[4] / 8) * 8;	//数据中不够8字节的剩余字节数
			i = WL_RxBuf[4] / 8;				// 数据中完整包个数
			if(i)
			{
				if(!m)
				{
					if(i==1)
					{
						RxWlCan.u32ID.ID.SN = NewCanTxSn();
						RxWlCan.u32ID.ID.SUB = 0;
						RxWlCan.u32ID.ID.SUM = 0;		//没有子帧
						RxWlCan.u16DLC = 8;
						for(j=0;j<RxWlCan.u16DLC;j++)
						{
							RxWlCan.u8Data[j] = WL_RxBuf[j+5];
						}
						
						TxCan.tx_efid = RxWlCan.u32ID.u32Id;
						TxCan.tx_ff = CAN_FF_EXTENDED;
						TxCan.tx_dlen = RxWlCan.u16DLC;
						for(j = 0; j < RxWlCan.u16DLC; j++)
						{
							TxCan.tx_data[j] = WL_RxBuf[j+5];
						}
						can_message_transmit(CAN0, &TxCan);
						//InsCanTrsQueue(&RxWlCan, TRUE);	
					}
					else
					{
						RxWlCan.u32ID.ID.SN = NewCanTxSn();
						RxWlCan.u32ID.ID.SUB = 0;
						RxWlCan.u32ID.ID.SUM = i-1;		//i子帧
						RxWlCan.u16DLC = 8;
						for(j=0;j<RxWlCan.u16DLC;j++)
						{
							RxWlCan.u8Data[j] = WL_RxBuf[j+5];
						}
						TxCan.tx_efid = RxWlCan.u32ID.u32Id;
						TxCan.tx_ff = CAN_FF_EXTENDED;
						TxCan.tx_dlen = RxWlCan.u16DLC;
						for(j=0;j<RxWlCan.u16DLC;j++)
						{
							TxCan.tx_data[j] = WL_RxBuf[j+5];
						}
						can_message_transmit(CAN0, &TxCan);
						//InsCanTrsQueue(&RxWlCan, TRUE);	
						i--;
						for(k=0;k<i;k++)
						{
							RxWlCan.u32ID.ID.SN = NewCanTxSn();
							RxWlCan.u32ID.ID.SUB = 1;
							RxWlCan.u32ID.ID.SUM = k;		//第k子帧
							RxWlCan.u16DLC = 8;
							for(j=0;j<RxWlCan.u16DLC;j++)
							{
								RxWlCan.u8Data[j] = WL_RxBuf[j+5+8+8*k];
							}
						
						TxCan.tx_efid = RxWlCan.u32ID.u32Id;
						TxCan.tx_ff = CAN_FF_EXTENDED;
						TxCan.tx_dlen = RxWlCan.u16DLC;
						for(j=0;j<RxWlCan.u16DLC;j++)
						{
							TxCan.tx_data[j] = WL_RxBuf[j+5];
						}
						can_message_transmit(CAN0, &TxCan);
							//InsCanTrsQueue(&RxWlCan, TRUE);	
						}
					}
				}
				else								// if(m)
				{
					/*
					 * 数据包首帧处理
					 */
					RxWlCan.u32ID.ID.SN = NewCanTxSn();
					RxWlCan.u32ID.ID.SUB = 0;
					RxWlCan.u32ID.ID.SUM = i;		//i+1子帧
					RxWlCan.u16DLC = 8;
					for(j=0;j<RxWlCan.u16DLC;j++)
					{
						RxWlCan.u8Data[j] = WL_RxBuf[j+5];
					}
						TxCan.tx_efid = RxWlCan.u32ID.u32Id;
						TxCan.tx_ff = CAN_FF_EXTENDED;
						TxCan.tx_dlen = RxWlCan.u16DLC;
						for(j=0;j<RxWlCan.u16DLC;j++)
						{
							TxCan.tx_data[j] = WL_RxBuf[j+5];
						}
						can_message_transmit(CAN0, &TxCan);
					//InsCanTrsQueue(&RxWlCan, TRUE);
					i--;							//完整包个数-1
					
					/*
					 * 数据包完整子帧处理
					 */
					while(i)
					{
						RxWlCan.u32ID.ID.SN = NewCanTxSn();
						RxWlCan.u32ID.ID.SUB = 1;
						RxWlCan.u32ID.ID.SUM = k;		//第k子帧
						RxWlCan.u16DLC = 8;
						for(j=0;j<RxWlCan.u16DLC;j++)
						{
							RxWlCan.u8Data[j] = WL_RxBuf[j+5+8+8*k];
						}
						
						TxCan.tx_efid = RxWlCan.u32ID.u32Id;
						TxCan.tx_ff = CAN_FF_EXTENDED;
						TxCan.tx_dlen = RxWlCan.u16DLC;
						for(j=0;j<RxWlCan.u16DLC;j++)
						{
							TxCan.tx_data[j] = WL_RxBuf[j+5];
						}
						can_message_transmit(CAN0, &TxCan);
						//InsCanTrsQueue(&RxWlCan, TRUE);
						k++;
						i--;
					}
					/*
					 * 数据包末帧不为整帧数据的处理
					 */
					if(!i)
					{
						RxWlCan.u32ID.ID.SN = NewCanTxSn();
						RxWlCan.u32ID.ID.SUB = 1;
						RxWlCan.u32ID.ID.SUM = k;		//第k子帧
						RxWlCan.u16DLC = m;
						for(j=0;j<RxWlCan.u16DLC;j++)
						{
							RxWlCan.u8Data[j] = WL_RxBuf[j+5+8+8*k];
						}
						TxCan.tx_efid = RxWlCan.u32ID.u32Id;
						TxCan.tx_ff = CAN_FF_EXTENDED;
						TxCan.tx_dlen = RxWlCan.u16DLC;
						for(j=0;j<RxWlCan.u16DLC;j++)
						{
							TxCan.tx_data[j] = WL_RxBuf[j+5];
						}
						can_message_transmit(CAN0, &TxCan);
						//InsCanTrsQueue(&RxWlCan, TRUE);						
					}
				}
			}else
			{
				RxWlCan.u32ID.ID.SN = NewCanTxSn();
				RxWlCan.u32ID.ID.SUB = 0;
				RxWlCan.u32ID.ID.SUM = 0;	//没有子帧，此时应为不够一帧can数据
				RxWlCan.u16DLC = m;
				for(j=0;j<RxWlCan.u16DLC;j++)
				{
					RxWlCan.u8Data[j] = WL_RxBuf[j+5];
				}
						TxCan.tx_efid = RxWlCan.u32ID.u32Id;
						TxCan.tx_ff = CAN_FF_EXTENDED;
						TxCan.tx_dlen = RxWlCan.u16DLC;
						for(j=0;j<RxWlCan.u16DLC;j++)
						{
							TxCan.tx_data[j] = WL_RxBuf[j+5];
						}
				can_message_transmit(CAN0, &TxCan);
				//InsCanTrsQueue(&RxWlCan, TRUE);				
			}
			
		break;
		case FT_WL_TO_SC_DBUS:					// 3			// 无线 发给 SC 的总线数据，需要应答
			
		break;
		case FT_WL_TO_SC_BEAT:
		case FT_WL_TO_SC_RF_MATCH:				// 6			// 无线 发给 SC 的无线对码
// 			if(RxWlCan.u32ID.ID.ACK == eNOACK)
// 			{
// 				i = NewCanTxSn();
// 				RxWlCan.u32ID.u32Id |= 0x000f0000 & (i << 16);
// 				
// 				RxWlCan.u16DLC = WL_RxBuf[4];
// 			
// 				for (i = 0; i < RxWlCan.u16DLC; i++)
// 				{
// 					RxWlCan.u8Data[i] = WL_RxBuf[i+5];
// 				}
// 				InsCanTrsQueue(&RxWlCan, TRUE);
// 			}
// 			if ((((u16)RxWlCan.u8Data[4]<<8)|RxWlCan.u8Data[3]) == GET_PT_U16(PT_SER_SC_NO))	//支架架号匹配
// 			{
// 				//SaveSRemoteCtrlInfo((u8)((u16)(RxWlCan.u8Data[3] & 0xff) | (((u16)RxWlCan.u8Data[4] << 8) & 0xff00)));
// 				SaveSRemoteCtrlInfo((u8)RxWlCan.u8Data[5]);
// 			}
// 		break;
		case FT_SC_TO_WL_RF_MATCH_RST:			// 7			// 无线与红外接收器发给 SC 的无线对码结果，需要应答
		case FT_WL_TO_SC_CTL_DATA:				// 10			// 遥控器 发给 SC 的无线控制数据，需要应答
		case FT_WL_TO_SC_CTL_DATA_SQN:			// 12			// //无线 发给 SC 的控制数据(按键连续按下)
		case FT_WL_TO_SC_CTL_DATA_LIFT:			// 13			// 无线 发给 SC 的控制数据(按键抬起)
		case FT_WL_TO_SC_DISCONNECT:			// 15			// 无线 发给 SC 解除对码
		case FT_SC_TO_WL_AUTO_PRESS_OPEN_RECEPT:			
			if(RxWlCan.u32ID.ID.ACK == eNOACK)
			{
				i = NewCanTxSn();
				RxWlCan.u32ID.u32Id |= 0x000f0000 & (i << 16);
				RxWlCan.u16DLC = WL_RxBuf[4];
			
				for (i = 0; i < RxWlCan.u16DLC; i++)
				{
					RxWlCan.u8Data[i] = WL_RxBuf[i+5];
				}
				TxCan.tx_efid = RxWlCan.u32ID.u32Id;
				TxCan.tx_ff = CAN_FF_EXTENDED;
				TxCan.tx_dlen = RxWlCan.u16DLC;
				for(j=0;j<RxWlCan.u16DLC;j++)
				{
					TxCan.tx_data[j] = WL_RxBuf[j+5];
				}
				can_message_transmit(CAN0, &TxCan);
				//InsCanTrsQueue(&RxWlCan, TRUE);
				/*判断是否是解除对码，并且与码遥控器编号一致，防止其它编号的遥控器解除对码*/
//				if ((RxWlCan.u32ID.ID.FT == FT_WL_TO_SC_DISCONNECT) 
//					&& (RxWlCan.u8Data[5] == SRemoteCtrl.RemoteID) 
//					&& ((((u16)RxWlCan.u8Data[4]<<8)|RxWlCan.u8Data[3]) == GET_PT_U16(PT_SER_SC_NO)))	//支架架号匹配
//				{
//					SetLedState(LED_GREEN,FALSE,0x00);
//					SetLedState(LED_RED,FALSE,0x00);
//				}
			}
		break;	
		case  FT_SC_TO_WL_AUTO_PRESS_CLOSE_RECEPT:    //21
			if(RxWlCan.u32ID.ID.ACK == eNOACK)
			{
				i = NewCanTxSn();
				RxWlCan.u32ID.u32Id |= 0x000f0000 & (i << 16);
				RxWlCan.u32ID.ID.RID =3;
				RxWlCan.u32ID.ID.TID =0;
				RxWlCan.u32ID.ID.FT  = 22;
				RxWlCan.u32ID.ID.SUM = LiuShuiNumb;	
				LiuShuiNumb	++;
				LiuShuiNumb %= 0x0f;
				RxWlCan.u16DLC = WL_RxBuf[4];
						
				for (i = 0; i < RxWlCan.u16DLC; i++)
				{
					RxWlCan.u8Data[i] = WL_RxBuf[i+5];
				}
				RxWlCan.u8Data[2]  = u8RssiSignalInf; 
				CanRcvWlSendProc(&RxWlCan,RxWlCan.u8Data[0]);
			}
			
		break;		
		default:break;
	}
}
/***********************************************************************************************
** 函 数 名：	NewWlTrsItem
** 功能描述：	申请一个无线发送队列项
** 输　  入：	void
** 输　  出：	无
** 作　  者：	沈万江
** 日　  期：	2015.9.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
************************************************************************************************/
static u16 NewWlTrsItem(void)
{
	u16 i;

	for (i = 0; i < WL_TX_QUEUE_MAX; i++)
		if (WlTxQueue[i].u8Next == 0xff && WlTxQueue[i].u8Prev == 0xff)
		{
			WlTxQueue[i].u8Prev = 0x00;
			WlTxQueue[i].u8Next = 0x00;
			return(i);
		}
	return(0xff);													//全部占用
}
/***********************************************************************************************
** 函 数 名：	DeleteWlTrsItem
** 功能描述：	释放一个无线发送队列项
** 输　  入：	su16Index,要删除的项号；
** 输　  出：	无
** 作　  者：	沈万江
** 日　  期：	2015.9.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
************************************************************************************************/
static void DeleteWlTrsItem(u16 u16Index)
{
	if (u16Index < WL_TX_QUEUE_MAX)
	{
		WlTxQueue[u16Index].u8Prev = 0xff;
		WlTxQueue[u16Index].u8Next = 0xff;
	}
}

/***********************************************************************************************
** 函 数 名：	InsWlTxBuf()
** 功能描述：	插入无线发送缓冲区
** 输　  入：	stWlData,要通过无线发送的内容；u16FirstTrs = TRUE，主动发送数据； = FALSE，转发无线数据；
** 输　  出：	无
** 作　  者：	沈万江
** 日　  期：	2015.9.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 InsWlTxBuf(st_WL_TRANS_DATA *stWlData, u16 u16FirstTrs)
{
	u8 u8Prio;
	u16 i,j,k;
	u16 RetVal=FALSE;												//队列满
#if OS_CRITICAL_METHOD == 3                      					/* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
	
	/*
	 *	数据帧的优先级定义
	 */
//	u8Prio = TRS_FRAME_PRIO_LOWEST;									//默认发送优先级最低
	if(stWlData->u8FT == FT_WL_TO_SC_CTL_DATA)						//无线控制数据优先级次高
	{
//		u8Prio = TRS_FRAME_PRIO_HIGH;
	}
	/*
	 *	插入数据
	 */
	OS_ENTER_CRITICAL();											//关全局中断？
	if ((i = NewWlTrsItem()) != 0xff)									//申请一个空闲项
	{
		if(WL_TxQueueCnt < WL_TX_QUEUE_MAX)
		{
			WlTxQueue[i].WlTransData = *stWlData;					//新版本C中允许结构赋值给结构
		
			if (u16FirstTrs  == TRUE)
			{
				//GetIntervalAndTrsCountByFT(&(WlTxQueue[i].u16WlTransIntv), &(WlTxQueue[i].u8Count), (u16)stWlData->u8FT, (u16)stWlData->u8Ack);
			}
			else
			{
				WlTxQueue[i].u8Count = 1;
				WlTxQueue[i].u16WlTransIntv = 0;
			}
			WlTxQueue[i].u8Prio = u8Prio;
			
			//根据优先级，加入等待发送链表
			k = WL_TxQueueHead;										//New head = i
			for (j = 0; j <  WL_TxQueueCnt; j++)
				if (WlTxQueue[k].u8Prio > u8Prio)						//数小，优先级大
					break;
				else
					k = WlTxQueue[k].u8Next;
			if (!j && !WL_TxQueueCnt )
			{
				WL_TxQueueHead  = i;									//New head = i
				WL_TxQueueTail  = i;									//New tail = i
			}
			else if (!j)
			{
				WlTxQueue[i].u8Next = WL_TxQueueHead;					//i's Next = Old head
				WlTxQueue[WL_TxQueueHead].u8Prev = i;					//Old head's Prev = i
				WL_TxQueueHead  = i;									//New head = i
			}
			else if (j == WL_TxQueueCnt )
			{
				WlTxQueue[WL_TxQueueTail].u8Next = i;					//u16CanTrsEnd's Next = i
				WlTxQueue[i].u8Prev = WL_TxQueueTail ;					//i's Prev = u16CanTrsEnd
				WL_TxQueueTail  = i;									//New end = i
			}
			else
			{
				WlTxQueue[WlTxQueue[k].u8Prev].u8Next = i;				//(j's Prev)'s Next = i
				WlTxQueue[i].u8Prev = WlTxQueue[k].u8Prev;				//i's Prev = j's Prev
				WlTxQueue[i].u8Next = k;								//i's Next = j
				WlTxQueue[k].u8Prev = i;								//j's Prev = i
			}
			WL_TxQueueCnt ++;
// 			if (u32CanTrsTimer  == TIMER_CLOSED || u32CanTrsTimer  == TIMER_EXPIRED)	//发送计时结束
// 				SetCanTrsInt(CAN1, ENABLED);						//使能发送中断
			OSSemPost(WLRxSem);	//释放信号量，通知有要待发的数据
			RetVal = TRUE;											//成功
		}
		else
			DeleteWlTrsItem(i);										//释放
	}
	//开全局中断？
	OS_EXIT_CRITICAL();
	return(RetVal);
}
/***********************************************************************************************
** 函 数 名：	DelWLTrsQueueFirstItem()
** 功能描述：	删除can发送链表首项。
** 输　  入：	stWlData,要通过无线发送的内容；u16FirstTrs = TRUE，主动发送数据； = FALSE，转发无线数据；
** 输　  出：	无
** 作　  者：	沈万江
** 日　  期：	2015.9.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 注意事项：	调用该函数时，需要考虑规避重入的问题。如果有可能在多个任务中使用，必须考虑加防冲突信号量；
**				同时，中断中，如果有可能使用，应该处于中断中，或关中断状态。
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
************************************************************************************************/
void DelWLTrsQueueFirstItem(void)
{
	u16 i;
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif

	//关全局中断？
	OS_ENTER_CRITICAL();
	if (WL_TxQueueCnt)
	{
		WL_TxQueueCnt --;
		i= WL_TxQueueHead ;
		if (i < WL_TX_QUEUE_MAX)
		{
			WL_TxQueueHead  = WlTxQueue[WL_TxQueueHead].u8Next;	//New head = Old head's Next
			DeleteWlTrsItem(i);								//释放
		}
	}
	//开全局中断？
	OS_EXIT_CRITICAL();

// 	if (!u16CanTrsCnt);
// 		StateLed(0x00, LED_COM_TRS);
}
/***********************************************************************************************
** 函 数 名：	Wl_TxDataProc()
** 功能描述：	无线发送数据处理，在无线管理任务中周期性调用
** 输　  入：	void
** 输　  出：	void
** 作　  者：	沈万江
** 日　  期：	2015.9.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
************************************************************************************************/
void Wl_TxDataProc(void)
{
	st_WL_TRANS_DATA stWlTran;
	u16 i;
	
	while (1)
	{
		if (WL_TxQueueCnt!= 0)									//发送队列不为空
		{
			stWlTran = WlTxQueue[WL_TxQueueHead].WlTransData;	//保留待发信息

			i = WlTxQueue[WL_TxQueueHead].u16WlTransIntv;
			if (WlTxQueue[WL_TxQueueHead].u8Count == 0)		//发送次数为0，则删除队列首项，且重新开始发送流程。
			{
				//InsCanSentQueue(&TrsCan, CAN_TRS_TIME_OVER);
				//DelCanTrsQueue();
				continue;
			}
			WlTxQueue[WL_TxQueueHead].u8Count--;			//发送次数-1

			if ((WlTxQueue[WL_TxQueueHead].u8Count == 0) && (stWlTran.u8Ack == eNOACK))	//没有要求应答的帧，发送计数到零，立即删除。
			{
				i = 0;
			}
			//没有发送计时间隔的情况下，发送次数为0，则删除队列首项。
			//这种处理导致转发的ACK帧被立即取消，对方回的应答与队列首比较不匹配，但是，这样处理也不会导致问题。
			if ((i == 0x0000) || (i == 0xffff))
			{
				if (WlTxQueue[WL_TxQueueHead].u8Count == 0)
				{
					//InsCanSentQueue(&TrsCan, CAN_TRS_TIME_OK);
					DelWLTrsQueueFirstItem();
				}//else
//					WlTxQueue[WL_TxQueueHead].u8Prio = TRS_FRAME_PRIO_HIGHEST;		//升级优先级至最高
				//SetCanTrsInt(CAN1, (u16CanTrsCnt != 0) ? ENABLED : DISABLED);	//失能发送中断
			}
			else														//发送计时间隔存在的情况下，开发送计时间隔，关闭发送中断
			{
				//u32CanTrsTimer = i;		//i+1;	// GetIntervalAndTrsCountByIMD函数中已采取同样动作		//确保发送间隔>=i
				//SetCanTrsInt(CAN1, DISABLED);							//失能发送中断
//				WlTxQueue[WL_TxQueueHead].u8Prio = TRS_FRAME_PRIO_HIGHEST;		//升级优先级至最高
			}
			WL_SendData(WlTxQueue[WL_TxQueueHead].WlTransData.u8Addr, WlTxQueue[WL_TxQueueHead].WlTransData.u8WlData,WlTxQueue[WL_TxQueueHead].WlTransData.u32Len);
//			SetLedState(LED_CAN_TRANS,TRUE,3);
		}
		break;
	}
}
/***********************************************************************************************
** 函 数 名：	GetRemoteIdByWl()
** 功能描述：	获取无线传输涉及的遥控器ID，该值和无线RFID的地址相关
** 输　  入：	无
** 输　  出：	无线传输涉及的遥控器ID
** 作　  者：	沈万江
** 日　  期：	2015.9.26
** 版    本：	V1.0.0
** 更新记录：
** 更新记录：
** 					日    期      姓    名                    描      述
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 GetRemoteIdByWl(void)
{
	return(255-SRemoteCtrl.RemoteID);			//遥控器ID
}

/*************************** END OF LINES *****************************************/
