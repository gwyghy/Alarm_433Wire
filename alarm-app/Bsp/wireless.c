/*********************************************************************************************************************************
** �ļ���:  wireless.c
** �衡��:  ���߹���ģ��
** ������: 	����
** �ա���:  2014-12-26
** �޸���:	
** �ա���:	
**
** �桡��:	V1.0.0.0
** ���¼�¼:
** ���¼�¼	��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
**--------------------------------------------------------------------------
**************************Copyright (c) 1998-1999 ��������Ӽ������޹�˾����������*************************************************/
#include "includes.h"
#include "can_app.h"
#include "app_cfg.h"

//����ģ��Ĺ���״̬
WL_WORK_STATUS eCanWLReportMode = WL_INIT_STATE;

REMOTE_CONTROL_s	SRemoteCtrl={(u16)STEP_IDLE,(u16)-1,(u32)-1,0xffffffff/* ��ʱ���ر� */,(u32)0};//ң�������ƽ��̶���

/*
 * ����ȡ�����յ���һ֡�������ݵ���ر���
 */
#define WL_RX_BUF_MAX		64		//�������ݽ��ջ�����ά��
#define WL_INFO_LENGTH		13		//�����ߴ������չCAN�����ֽ���

u8 WL_RxBuf[WL_RX_BUF_MAX];			//���߽��ջ�����
u32 WL_RxBufCnt=0;					//��Ч���ݼ���
u32 WL_RxBufWpt=0;					//���߽��ջ�����д��ָ��
u32 WL_RxBufRpt=0;					//���߽��ջ���������ָ��

/*
 * ���߷��ͻ�����
 */
#define WL_TX_QUEUE_MAX		64
st_WL_TRANS_ITEM WlTxQueue[WL_TX_QUEUE_MAX];		//�������ݷ��ͻ�����
u16	WL_TxQueueCnt = 0;								//������������
u16 WL_TxQueueHead = 0;								//���߷��ͻ�����д��ָ��
u16 WL_TxQueueTail = 0;								//���߷��ͻ�������ȡָ��

//���߷����������к�
//s_vuWlTxSn = (s_vuWlTxSn + 1) % WL_TX_SN_MAX;
#define WL_TX_SN_MAX			16
static vu16 s_vuWlTxSn = 0;

// ���߽����ź�������
static OS_EVENT * WLRxSem;		//RFID���յ����ݺ���ź���

//OS_STK  wl_manage_task_stk[WL_MANAGE_TASK_STK_SIZE];		//���������ջ
/***********************************************************************************************
** �� �� ����	WlVarInit()
** ����������	���߹����������õ�����ر����ĳ�ʼ��
** �䡡  �룺	��
** �䡡  ����	��
** ����  �ߣ�	����
** �ա�  �ڣ�	2015.9.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
static void WlVarInit(void)
{
	u32 i;
	
	for (i = 0; i < WL_TX_QUEUE_MAX; i++)
	{
		WlTxQueue[i].u8Next = 0xff;						//�������ݷ��ͻ�����
		WlTxQueue[i].u8Prev = 0xff;
	}
	WL_TxQueueCnt = 0;								//������������
	WL_TxQueueHead = 0;								//���߷��ͻ�����д��ָ��
	WL_TxQueueTail = 0;								//���߷��ͻ�������ȡָ��
}
/*****************************************************************
** ��������MessageForRfidRcved
** �䡡�룺��
** �䡡������
** ����������RFID���ݽ��պ�Ļص������������ź���
******************************************************************/
void MessageForRfidRcved(void)
{
	OSSemPost(WLRxSem);	//RFID���յ�����
}
/*****************************************************************
** ��������IsMessageForRfidRcved
** �䡡�룺��
** �䡡����TRUE�����յ��ź�����FALSE��û�н��յ��ź���
** �����������ж��Ƿ���յ�RFID�յ����ݵ��ź���
******************************************************************/
u32 IsMessageForRfidRcved(void)
{
	u8 err;
	 
	OSSemPend(WLRxSem, 0/* WL_MANAGE_TASK_DELAY_TIME/TICK_TIME */, &err);//�ȴ��ź��� 
	
	if(err == OS_ERR_NONE)								//���յ��ź���
		return (TRUE);
	else
		return (FALSE);
}
/***********************************************************************************************
** �� �� ����	WL_MNG_Task
** ����������	������ģ��������շ��Լ�״̬���й���
** �䡡  �룺	��
** �䡡  ����	��
** ����  �ߣ�	����
** �ա�  �ڣ�	2015.9.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
void WL_MNG_Task(void *p_arg)
{
	// �������߽����ź���
	WLRxSem = OSSemCreate(0);	
	// ����RFID�������ݺ����ź���
	SetRcvedBackCallFunc((RCVED_BACK_CALL_FUNC)MessageForRfidRcved);
	// ���߹����������õ�����ر����ĳ�ʼ��
	WlVarInit();
	//RFIDӲ����ʼ��
	RFID_Init();
	while(1)
	{
		/* 
		 * �ȴ��ź���
		 */
		(void)IsMessageForRfidRcved();
		/*
		 * ���߽������ݴ���,���º���Ӧ��������������ִ��
		 */
		Wl_RxDataProc();
		//Wl_TxDataProc();

		//ִ������10ms
		//OSTimeDly(WL_MANAGE_TASK_DELAY_TIME/TICK_TIME);
	}
}
/***********************************************************************************************
** �� �� ����	SaveSRemoteCtrlInfo
** ����������	����ң���������Ϣ
** �䡡  �룺	��
** �䡡  ����	��
** ����  �ߣ�	����
** �ա�  �ڣ�	2015.9.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 SaveSRemoteCtrlInfo(u8 u8Id)
{
	SRemoteCtrl.RemoteID = u8Id;
	
	return TRUE;
}
/***********************************************************************************************
** �� �� ����	NewCanTxSn
** ����������	��ȡCAN�����µķ�����ˮ
** �䡡  �룺	��
** �䡡  ����	��
** ����  �ߣ�	����
** �ա�  �ڣ�	2015.9.26
** ��	����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��	��	  ��	��					��	  ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 NewCanTxSn(void)
{
	s_vuWlTxSn = (s_vuWlTxSn + 1) % WL_TX_SN_MAX;
	
	return (s_vuWlTxSn);
}
/***********************************************************************************************
** �� �� ����	Wl_RxDataProc
** ����������	���߽������ݴ������º���Ӧ��������������ִ�С�
** �䡡  �룺	��
** �䡡  ����	��
** ����  �ߣ�	����
** �ա�  �ڣ�	2015.9.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**					2016-2-3	����			�޸�֡ID = 6ʱ����ӱ���ң����ID�ĺ���
************************************************************************************************/
can_trasnmit_message_struct TxCan;
extern u8  u8RssiSignalInf;   //�ź�ǿ��
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
			//��ȡRDID״̬
			j = GetRfidCurStatus();
			switch(j&0xf0)
			{
				case CC1101_STATE_IDLE:
					SetRfidSRX();				//����RFID�������״̬
				break;
				case CC1101_STATE_RX:
				case CC1101_STATE_TX:
				case CC1101_STATE_FSTXON:
				case CC1101_STATE_CALIBRATE:
				case CC1101_STATE_SETTLING:
				break;
				case CC1101_STATE_RX_OVERFLOW:
					RfidStrobe(CC1101_SFRX);	//ˢ�� RX FIFO buffer.
					RfidStrobe(CC1101_SRX);		//�������״̬	
				break;
				case CC1101_STATE_TX_UNDERFLOW:
					RfidStrobe(CC1101_SFTX);    //ˢ�� TX FIFO buffer.
					RfidStrobe(CC1101_SRX);		//�������״̬	
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
	
// 	// �����������㹻
// 	if (i < (WL_RxBuf[4]+5))			//֡�����ݳ���
// 		return; 

	RxWlCan.u32ID.u32Id = 0;
	for (i = 0; i < 4; i++)
	{
		RxWlCan.u32ID.u32Id += (u32)WL_RxBuf[i]<<(i<<3);
	}
	/*
	 * ���յ��������ݺ󣬸���FT�жϴ���ʽ��������������ݾ������߷����������ң��������ͨ��CAN���͵�SC
	 * ��������֡���ͽ�SN���к���ӵ�CAN ID��
	 */
	switch (RxWlCan.u32ID.ID.FT)
	{
		case FT_SC_TO_WL_DBUS:					// 2			// SC �������ߵ��������ݣ���ҪӦ��
			i = 0;
			k = 0;
			j = 0;
			m = 0;
			RxWlCan.u32ID.ID.ACK = eACK;
			RxWlCan.u32ID.ID.FT = FT_WL_TO_SC_DBUS;
			RxWlCan.u32ID.ID.RD = 0;
			RxWlCan.u32ID.ID.RID = SC_RID;
			RxWlCan.u32ID.ID.TID = WL_RID;
			m = WL_RxBuf[4] - (WL_RxBuf[4] / 8) * 8;	//�����в���8�ֽڵ�ʣ���ֽ���
			i = WL_RxBuf[4] / 8;				// ����������������
			if(i)
			{
				if(!m)
				{
					if(i==1)
					{
						RxWlCan.u32ID.ID.SN = NewCanTxSn();
						RxWlCan.u32ID.ID.SUB = 0;
						RxWlCan.u32ID.ID.SUM = 0;		//û����֡
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
						RxWlCan.u32ID.ID.SUM = i-1;		//i��֡
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
							RxWlCan.u32ID.ID.SUM = k;		//��k��֡
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
					 * ���ݰ���֡����
					 */
					RxWlCan.u32ID.ID.SN = NewCanTxSn();
					RxWlCan.u32ID.ID.SUB = 0;
					RxWlCan.u32ID.ID.SUM = i;		//i+1��֡
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
					i--;							//����������-1
					
					/*
					 * ���ݰ�������֡����
					 */
					while(i)
					{
						RxWlCan.u32ID.ID.SN = NewCanTxSn();
						RxWlCan.u32ID.ID.SUB = 1;
						RxWlCan.u32ID.ID.SUM = k;		//��k��֡
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
					 * ���ݰ�ĩ֡��Ϊ��֡���ݵĴ���
					 */
					if(!i)
					{
						RxWlCan.u32ID.ID.SN = NewCanTxSn();
						RxWlCan.u32ID.ID.SUB = 1;
						RxWlCan.u32ID.ID.SUM = k;		//��k��֡
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
				RxWlCan.u32ID.ID.SUM = 0;	//û����֡����ʱӦΪ����һ֡can����
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
		case FT_WL_TO_SC_DBUS:					// 3			// ���� ���� SC ���������ݣ���ҪӦ��
			
		break;
		case FT_WL_TO_SC_BEAT:
		case FT_WL_TO_SC_RF_MATCH:				// 6			// ���� ���� SC �����߶���
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
// 			if ((((u16)RxWlCan.u8Data[4]<<8)|RxWlCan.u8Data[3]) == GET_PT_U16(PT_SER_SC_NO))	//֧�ܼܺ�ƥ��
// 			{
// 				//SaveSRemoteCtrlInfo((u8)((u16)(RxWlCan.u8Data[3] & 0xff) | (((u16)RxWlCan.u8Data[4] << 8) & 0xff00)));
// 				SaveSRemoteCtrlInfo((u8)RxWlCan.u8Data[5]);
// 			}
// 		break;
		case FT_SC_TO_WL_RF_MATCH_RST:			// 7			// ������������������ SC �����߶���������ҪӦ��
		case FT_WL_TO_SC_CTL_DATA:				// 10			// ң���� ���� SC �����߿������ݣ���ҪӦ��
		case FT_WL_TO_SC_CTL_DATA_SQN:			// 12			// //���� ���� SC �Ŀ�������(������������)
		case FT_WL_TO_SC_CTL_DATA_LIFT:			// 13			// ���� ���� SC �Ŀ�������(����̧��)
		case FT_WL_TO_SC_DISCONNECT:			// 15			// ���� ���� SC �������
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
				/*�ж��Ƿ��ǽ�����룬��������ң�������һ�£���ֹ������ŵ�ң�����������*/
//				if ((RxWlCan.u32ID.ID.FT == FT_WL_TO_SC_DISCONNECT) 
//					&& (RxWlCan.u8Data[5] == SRemoteCtrl.RemoteID) 
//					&& ((((u16)RxWlCan.u8Data[4]<<8)|RxWlCan.u8Data[3]) == GET_PT_U16(PT_SER_SC_NO)))	//֧�ܼܺ�ƥ��
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
** �� �� ����	NewWlTrsItem
** ����������	����һ�����߷��Ͷ�����
** �䡡  �룺	void
** �䡡  ����	��
** ����  �ߣ�	����
** �ա�  �ڣ�	2015.9.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
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
	return(0xff);													//ȫ��ռ��
}
/***********************************************************************************************
** �� �� ����	DeleteWlTrsItem
** ����������	�ͷ�һ�����߷��Ͷ�����
** �䡡  �룺	su16Index,Ҫɾ������ţ�
** �䡡  ����	��
** ����  �ߣ�	����
** �ա�  �ڣ�	2015.9.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
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
** �� �� ����	InsWlTxBuf()
** ����������	�������߷��ͻ�����
** �䡡  �룺	stWlData,Ҫͨ�����߷��͵����ݣ�u16FirstTrs = TRUE�������������ݣ� = FALSE��ת���������ݣ�
** �䡡  ����	��
** ����  �ߣ�	����
** �ա�  �ڣ�	2015.9.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 InsWlTxBuf(st_WL_TRANS_DATA *stWlData, u16 u16FirstTrs)
{
	u8 u8Prio;
	u16 i,j,k;
	u16 RetVal=FALSE;												//������
#if OS_CRITICAL_METHOD == 3                      					/* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
	
	/*
	 *	����֡�����ȼ�����
	 */
//	u8Prio = TRS_FRAME_PRIO_LOWEST;									//Ĭ�Ϸ������ȼ����
	if(stWlData->u8FT == FT_WL_TO_SC_CTL_DATA)						//���߿����������ȼ��θ�
	{
//		u8Prio = TRS_FRAME_PRIO_HIGH;
	}
	/*
	 *	��������
	 */
	OS_ENTER_CRITICAL();											//��ȫ���жϣ�
	if ((i = NewWlTrsItem()) != 0xff)									//����һ��������
	{
		if(WL_TxQueueCnt < WL_TX_QUEUE_MAX)
		{
			WlTxQueue[i].WlTransData = *stWlData;					//�°汾C������ṹ��ֵ���ṹ
		
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
			
			//�������ȼ�������ȴ���������
			k = WL_TxQueueHead;										//New head = i
			for (j = 0; j <  WL_TxQueueCnt; j++)
				if (WlTxQueue[k].u8Prio > u8Prio)						//��С�����ȼ���
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
// 			if (u32CanTrsTimer  == TIMER_CLOSED || u32CanTrsTimer  == TIMER_EXPIRED)	//���ͼ�ʱ����
// 				SetCanTrsInt(CAN1, ENABLED);						//ʹ�ܷ����ж�
			OSSemPost(WLRxSem);	//�ͷ��ź�����֪ͨ��Ҫ����������
			RetVal = TRUE;											//�ɹ�
		}
		else
			DeleteWlTrsItem(i);										//�ͷ�
	}
	//��ȫ���жϣ�
	OS_EXIT_CRITICAL();
	return(RetVal);
}
/***********************************************************************************************
** �� �� ����	DelWLTrsQueueFirstItem()
** ����������	ɾ��can�����������
** �䡡  �룺	stWlData,Ҫͨ�����߷��͵����ݣ�u16FirstTrs = TRUE�������������ݣ� = FALSE��ת���������ݣ�
** �䡡  ����	��
** ����  �ߣ�	����
** �ա�  �ڣ�	2015.9.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** ע�����	���øú���ʱ����Ҫ���ǹ����������⡣����п����ڶ��������ʹ�ã����뿼�Ǽӷ���ͻ�ź�����
**				ͬʱ���ж��У�����п���ʹ�ã�Ӧ�ô����ж��У�����ж�״̬��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
void DelWLTrsQueueFirstItem(void)
{
	u16 i;
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif

	//��ȫ���жϣ�
	OS_ENTER_CRITICAL();
	if (WL_TxQueueCnt)
	{
		WL_TxQueueCnt --;
		i= WL_TxQueueHead ;
		if (i < WL_TX_QUEUE_MAX)
		{
			WL_TxQueueHead  = WlTxQueue[WL_TxQueueHead].u8Next;	//New head = Old head's Next
			DeleteWlTrsItem(i);								//�ͷ�
		}
	}
	//��ȫ���жϣ�
	OS_EXIT_CRITICAL();

// 	if (!u16CanTrsCnt);
// 		StateLed(0x00, LED_COM_TRS);
}
/***********************************************************************************************
** �� �� ����	Wl_TxDataProc()
** ����������	���߷������ݴ��������߹��������������Ե���
** �䡡  �룺	void
** �䡡  ����	void
** ����  �ߣ�	����
** �ա�  �ڣ�	2015.9.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
void Wl_TxDataProc(void)
{
	st_WL_TRANS_DATA stWlTran;
	u16 i;
	
	while (1)
	{
		if (WL_TxQueueCnt!= 0)									//���Ͷ��в�Ϊ��
		{
			stWlTran = WlTxQueue[WL_TxQueueHead].WlTransData;	//����������Ϣ

			i = WlTxQueue[WL_TxQueueHead].u16WlTransIntv;
			if (WlTxQueue[WL_TxQueueHead].u8Count == 0)		//���ʹ���Ϊ0����ɾ��������������¿�ʼ�������̡�
			{
				//InsCanSentQueue(&TrsCan, CAN_TRS_TIME_OVER);
				//DelCanTrsQueue();
				continue;
			}
			WlTxQueue[WL_TxQueueHead].u8Count--;			//���ʹ���-1

			if ((WlTxQueue[WL_TxQueueHead].u8Count == 0) && (stWlTran.u8Ack == eNOACK))	//û��Ҫ��Ӧ���֡�����ͼ������㣬����ɾ����
			{
				i = 0;
			}
			//û�з��ͼ�ʱ���������£����ʹ���Ϊ0����ɾ���������
			//���ִ�����ת����ACK֡������ȡ�����Է��ص�Ӧ��������ױȽϲ�ƥ�䣬���ǣ���������Ҳ���ᵼ�����⡣
			if ((i == 0x0000) || (i == 0xffff))
			{
				if (WlTxQueue[WL_TxQueueHead].u8Count == 0)
				{
					//InsCanSentQueue(&TrsCan, CAN_TRS_TIME_OK);
					DelWLTrsQueueFirstItem();
				}//else
//					WlTxQueue[WL_TxQueueHead].u8Prio = TRS_FRAME_PRIO_HIGHEST;		//�������ȼ������
				//SetCanTrsInt(CAN1, (u16CanTrsCnt != 0) ? ENABLED : DISABLED);	//ʧ�ܷ����ж�
			}
			else														//���ͼ�ʱ������ڵ�����£������ͼ�ʱ������رշ����ж�
			{
				//u32CanTrsTimer = i;		//i+1;	// GetIntervalAndTrsCountByIMD�������Ѳ�ȡͬ������		//ȷ�����ͼ��>=i
				//SetCanTrsInt(CAN1, DISABLED);							//ʧ�ܷ����ж�
//				WlTxQueue[WL_TxQueueHead].u8Prio = TRS_FRAME_PRIO_HIGHEST;		//�������ȼ������
			}
			WL_SendData(WlTxQueue[WL_TxQueueHead].WlTransData.u8Addr, WlTxQueue[WL_TxQueueHead].WlTransData.u8WlData,WlTxQueue[WL_TxQueueHead].WlTransData.u32Len);
//			SetLedState(LED_CAN_TRANS,TRUE,3);
		}
		break;
	}
}
/***********************************************************************************************
** �� �� ����	GetRemoteIdByWl()
** ����������	��ȡ���ߴ����漰��ң����ID����ֵ������RFID�ĵ�ַ���
** �䡡  �룺	��
** �䡡  ����	���ߴ����漰��ң����ID
** ����  �ߣ�	����
** �ա�  �ڣ�	2015.9.26
** ��    ����	V1.0.0
** ���¼�¼��
** ���¼�¼��
** 					��    ��      ��    ��                    ��      ��
** 					==========  =============  ========================================
**
************************************************************************************************/
u32 GetRemoteIdByWl(void)
{
	return(255-SRemoteCtrl.RemoteID);			//ң����ID
}

/*************************** END OF LINES *****************************************/
