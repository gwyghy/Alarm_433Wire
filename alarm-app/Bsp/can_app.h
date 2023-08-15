/*
*********************************************************************************************************
*	                                            UWB
*                           Commuication Controller For The Mining Industry
*
*                                  (c) Copyright 1994-2013  HNDZ 
*                       All rights reserved.  Protected by international copyright laws.
*
*
*    File    : can_app.h
*    Module  : can driver
*    Version : V1.0
*    History :
*   -----------------
*             can�ײ�����
*              Version  Date           By            Note
*              v1.0     2013-09-09     xxx
*
*********************************************************************************************************
*/

#ifndef __CAN_APP_H__
#define __CAN_APP_H__

#include "l4can.h"
#include "can_bus.h"
#include "string.h"
#include "protocol.h"
#include "can_buf.h"
#include "led_app.h"

/****************************************************************************************************
  CAN  version define
 ***************************************************************************************************/
#define CAN_POSITION_MODE 			0
#define CAN_MONITOR_MODE 			1

#define CAN_TASK_MODE		CAN_POSITION_MODE

#define UWB_ID  					7
#define CAN_DEV_ID  				0

#define ANGLE_ID  					5

#define WL_RID						3
/****************************************************************************************************
//���ⱨ����ض���
 ***************************************************************************************************/
extern u16 LightFlag;

extern u8 RGrayScale;				//��ɫ�Ҷ�ֵ
extern u8 GGrayScale;				//��ɫ�Ҷ�ֵ
extern u8 BGrayScale;				//��ɫ�Ҷ�ֵ

extern u8 LED_NUM;		//���ݵ�������ѡ��ͬ������ʽ
extern u8 WaterFlow_Type;		//LED��ˮ��ģʽ

extern uint16_t Breathestep_R;
extern uint16_t Breathestep_G;
extern uint16_t Breathestep_B;

#define FRM_ID_BEFORE_ALARM		0x00000107				// Ԥ��֡ID
#define FRM_ID_ACTION_ALARM		0x00000147				// ����ִ�б���֡ID

#define AGING_TEST_BYTE			0xFE					//��Ӧ���ϻ���װ���Ƶ�������LED����ʱ���ϻ�LED
#define POWER_TEST_BYTE			0xFD					//��Ӧ�ڳ�Ʒ������Ե�������������
#define LED_TEST_BYTE			0xFC					//��Ӧ�ڳ�Ʒ������Ե������LED�Ƿ�����

#define BL_TIMEOUT_VALUE		200						//ͨѶ��ʱʱ�䣬�ղ���������������ֹͣ���ⱨ��״̬

//UWB��λ��ض���
#define REQ_CONFIG_DELAY	500
#define UWB_HEART_DELAY		4000			//�������ݷ��ͼ��
#define UWB_MODEL			0x31

#define BEEP_LIGHT_MODEL	0x30			//���ⱨ���豸����
/****************************************************************************************************
 UWB MODULE STATE
 ***************************************************************************************************/
enum
{
	UWB_INIT = 0,				//UWB��λ��ʼ��δ����״̬
	UWB_NORMAL					//������ɣ���������
};

enum
{
	UWB_CONFIG_REQ = 0,				//��������
	UWB_INFO_REPORT,				//1����Ա��λ��Ϣ�ϱ�
	REQ_UWB_INFO,					//2����ѯ��Ա��λģ��ı�ǩ����
	SET_UWB_PARA,					//3��������Ա��λģ�鹤������
	 //ID_BEFORE_ALARM,				//4
	 //ID_ACTION_ALARM,				//5
	UWB_RESET = 6,					//6����λ��Ա��λģ������ⱨ����
	UWB_HEART,
	 
	BEEP_LIGHT_HEART = 100,
	WIRELESS_MOUDLE_HEART,
};
typedef enum
{
    WL_NORMAL_STATE = 0,
    WL_INIT_STATE = 1,
}WL_WORK_STATUS;

typedef union {
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
} CanHeadID;
 /****************************************************************************************************
uwb dev of info
 ***************************************************************************************************/
typedef struct
{
	uint16_t standNum;   //֧�ܿ��������
	union {
		struct {
			uint8_t workType:1; //����ģʽ
			uint8_t distType:1;  //��������
			uint8_t reserve:2;   //����
			uint8_t rate:4;      //���书��
		}sUwbSet;
		uint8_t uPara;
	}uwbPara;
	//		uint8_t devType;
	uint8_t interval;		//ʱ����
	uint16_t extent;		//��ⷶΧ
	uint16_t height;		//֧�ܸ߶�
					
}sUWB_RUN_INFO;				//���в�����Ϣ
 

typedef struct
{
	uint8_t peoplenum; //����
	union {
		struct {
			uint8_t tagState:4; //��ǩ״̬
			uint8_t author:4;    //Ȩ��
		}sTag;
		uint8_t tag;
	}tagInfo;
	uint16_t tagId;       //��ǩid
	uint16_t tagDist;     //��ǩ����
	uint8_t  reserve;
	uint8_t  crc;
}sTag_Info;
/****************************************************************************************************
CAN TX MESSAGE 	STATE
 ***************************************************************************************************/
enum									//���ͽ������ 
{
	CAN_TRS_FAIL = 0,
	CAN_TRS_SUCCESS
};


/****************************************************************************************************
CAN TX MESSAGE 	TYPE ENUM
 ***************************************************************************************************/
enum									//��������
{
	CAN_ASK = 0,
	CAN_ACK
};

/****************************************************************************************************
 CAN rx and tx task  stacks
 ***************************************************************************************************/
#define CANTX_TASK_SIZE                        384u
#define CANRX_TASK_SIZE                        384u

//���ⱨ������ر���
extern u32 BeepLightRxTimeoutTimer;		//���ⱨ����ͨѶ״̬�жϳ�ʱ��ʱ������ʱ�����������״̬����ֹ����״̬ͣ������
extern u16 BeepCaseFlag;				// ��������־
extern u16 u16BeepOffTimer;				//��������Ъ��ʱ��xms��
extern u16 u16BeepOnTimer;				//�������澯��ʱ��xms��

extern u16 u16LightOffTimer;			//LED��Ъ��ʱ��xms��
extern u16 u16LightOnTimer;				//LED�澯��ʱ��xms��

extern sUWB_RUN_INFO 	gUwbRunPara;
extern sCAN_BUFFER 		gUwbCanTxBuf;

void WaitUwbStartSem(void);
void CanAppInit(void);

uint16_t GetUwbId(void);
uint16_t GetWLMId(void);

int32_t WriteReportInfo(uint8_t tagNum, uint16_t tagId, uint16_t dist, uint16_t state, uint8_t right);
int32_t CheckTagState(uint16_t dist);
uint16_t GetUwbReportTime(void);


sCAN_FRAME SendAngleData(uint32_t func, uint8_t dest, uint8_t *data, uint8_t size);
#endif /*__CAN_APP_H__*/


