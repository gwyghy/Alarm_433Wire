/*********************************************************************************************************************************
** �ļ���:  wireless.h
** �衡��:  ���߹���ģ��ͷ�ļ�
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
#ifndef __WIRELESS_H__
#define __WIRELESS_H__

#include "gd32f30x.h"
#include "protocol.h"

#define	WL_MANAGE_TASK_DELAY_TIME				(10)			// ���߹���������ʱʱ�� // 10ms
#define WL_SS_ADDR								0xFF			// ���������ߵ�ַ
#define WL_BROADCAST_ADDR						0x00			// ���������ߵ�ַ
// ң�������ƽṹ
typedef struct
{
	u16 Step;					//���Ʋ���
	u16 CtrlSC;					//������֧�ܺ�
	u32 RemoteID;				//ң����ID
	u32 Timer;					//���Ƽ�ʱ��
	u32 CtrlCode;				//���Ƽ���
} REMOTE_CONTROL_s;
// WL�������ݽṹ
typedef struct
{
	union {
		struct {
			u32 RID:3;				//Ŀ��ID(���շ�)
			u32 TID:3;				//ԴID(���ͷ�)
			u32 FT:10;				//֡����
			u32 SN:4;				//��֡���к�
			u32 SUM:5;				//��֡��
			u32 SUB:1;				//��������֡������֡
			u32 ACK:1;				//Ӧ��λ
			u32 RD:2;				//����λ
		} ID;
		u32 u32Id;
	} u32ID;
	u8  u8FT;					//����֡����
	u8  u8Ack;					//�Ƿ���ҪӦ��
	u8  u8Addr;					//�������ݷ���Ŀ���ַ
	u8  u8WlData[64];			//������������
	u32 u32Len;					//�������ݷ��ͳ���
}st_WL_TRANS_DATA;
typedef struct
{
	st_WL_TRANS_DATA WlTransData;//���߷�����������
	u8  u8Prio;					//����֡�����ȼ�
	u8  u8Count;				//�������ݷ��ʹ���
	u16 u16WlTransIntv;			//�������ݷ��ͼ��
	u8	u8Next;					//ָ����һ�����ݽṹ
	u8  u8Prev;					//ָ��ǰһ�����ݽṹ
}st_WL_TRANS_ITEM;
// ң����״̬��
enum
{
	STEP_IDLE=0,				//����
	STEP_CHECK_BY_IR,			//�������
	STEP_CHECK_BY_WL,			//���߶���
	STEP_CONTROL				//����
};
/*
 *  ���߽������ݴ������º���Ӧ��������������ִ��
 */
void Wl_RxDataProc(void);
/*
 *  ���߷������ݴ������º���Ӧ��������������ִ��
 */
void Wl_TxDataProc(void);
/*
 *  �������߷��ͻ�����
 */
u32 InsWlTxBuf(st_WL_TRANS_DATA *stWlData, u16 u16FirstTrs);
/*
 *  �����߽��������շ����й���
 */
void WL_MNG_Task(void *p_arg);

/*
 *  ��ȡ���ߴ����漰��ң����ID����ֵ������RFID�ĵ�ַ��أ�����ֵ�����ߴ����漰��ң����ID��
 */
u32 GetRemoteIdByWl(void);
/*
 * ����ң����ID
 */
u32 SaveSRemoteCtrlInfo(u8 u8Id);

/*
 * ��ȡ�µ�CAN�������к�
 */
u32 NewCanTxSn(void);

/*
 * �ͷŽ��յ��������ݵ��ź�
 */
void MessageForRfidRcved(void);

#endif /* __WIRELESS_H__ */
/*************************** END OF LINES *****************************************/
