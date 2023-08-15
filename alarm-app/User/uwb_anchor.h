#ifndef __UWB_ANCHOR_H__
#define __UWB_ANCHOR_H__


#include "stdint.h"
#include "stdbool.h"
#include "can_app.h"
#include "deca_device_api.h"
#include "deca_regs.h"
//#include "filtering.h"
#include "zlist.h"


#define DW_TIME_OVER        							0x10000000000


#define ANCHOR_TX_ANT_DLY                 16440      //���������ӳ�
#define ANCHOR_RX_ANT_DLY                 16440      //���������ӳ�

#define ANCHOR_PAN_ID 										0xDECA					//��վPAN IDֵ

#define ANCHOR_ADDR_OFF						7		//��վ��ַ��֡�е�ƫ����
#define ENCHOR_SOURECE_ADDR_OFF				5

#define ANC_RX_LEN_MAX										64

#define ANC_DATA_HEAD_SIZE                10

#define ANC_TX_FRAME_DATA_LEN             3

#define ANC_COMM_ACK_LEN                  ANC_DATA_HEAD_SIZE+12 
#define ANC_TX_FRAME_LEN                  ANC_DATA_HEAD_SIZE+ ANC_TX_FRAME_DATA_LEN +2

#define BEACON_DELAY_BASIC                300
#define BEACON_DELAY_SETP                 450
#define BEACON_GROUP_NUM                  20


typedef struct
{
	uint64_t t1;
	uint64_t t2;
	uint64_t t3;
	uint64_t t4;
	uint64_t t5;
	uint64_t t6;
}sLocTime;

typedef struct
{
	uint64_t t2;
	uint64_t t3;
}sTagTime;



typedef enum{
	UWB_WAIT_STATE = 0,  //
	UWB_RX_BEA_STATE,
	UWB_RES_BEA_STATE,
	UWB_RX_POLL_STATE,
	UWB_RES_POLL_STATE,
	UWB_RX_FINAL_STATE,
}UWB_DEV_STATE;

#define FRAME_CTRL_BYTE_ADDR_LEN_BIT						0x4400    //MAC ֡���ƶ��е�ַ����λ
/*��λ֡��ʽ*/
//#define FINAL_MSG_TS_LEN                      		5  //finally��Ϣ�У�ʱ������ȣ�5���ֽ�
//#define FINAL_MSG_POLL_TX_TS_IDX              		10  //finally��Ϣ�У�POLL����ʱ�������
//#define FINAL_MSG_RESP_RX_TS_IDX              		(FINAL_MSG_POLL_TX_TS_IDX + FINAL_MSG_TS_LEN)  //finally��Ϣ�У�RESP����ʱ�������
//#define FINAL_MSG_FINAL_TX_TS_IDX             		(FINAL_MSG_RESP_RX_TS_IDX +  FINAL_MSG_TS_LEN) //finally��Ϣ�У�FINAL����ʱ�������

#define MSG_TIME_LEN                      		     5  //ʱ�������
#define BEACON_ACK_T2_DIR              		       ANC_DATA_HEAD_SIZE  //�ű�Ӧ��֡t2����λ��
#define BEACON_ACK_T3_DIR              		       (BEACON_ACK_T2_DIR + MSG_TIME_LEN)  //�ű�Ӧ��֡t3����λ��



#define ANC_LOC_TAG_MAX									10	//��վ��ͬʱ���ɵı�ǩ��
#define ANC_TOF_REC_MAX									10		//��ǩ��һ��ͨѶ��������ͬһ��վ��ͨѶ����
#define ANC_TOF_MIN_DATA                10

typedef struct
{
	uint16_t lengId;
	uint32_t count;
}sTimeData;


typedef struct
{
	uint16_t tagId;                     //��¼�ı�ǩid
	uint16_t dist[ANC_TOF_REC_MAX];     //�ñ�ǩ���վ�ľ���buf
	uint16_t realDist;                  //����ȥ����ľ���
	uint16_t heartCout;           			//��������
	uint16_t oldHeartCout;           		//�ɵ���������
	uint8_t  commCnt;							      //�������ڱ�ǩ��ͬһ��վ��ͨѶ����
	uint8_t  wr;                        //д��ǩid
	uint8_t  mark;
	uint8_t  maxPtr;                      //���ֵָ��
	uint8_t  minPtr;	                   //��Сֵָ��
	uint8_t  errCount;                   //���ߴ������
	uint8_t  right;                      //Ȩ��
	uint8_t  FisrtDistErr;
	uint16_t ErrDist;
}sDistNote;

enum{
	UWB_RX_STATE = 0,
	UWB_TX_STATE
};

typedef struct
{
	uint16_t devNum;            //�豸���
	uint16_t devState;
	uint16_t uwbStata;          //uwb ״̬
	uint16_t currDevId;         //��ǰ���ж�λ�ı�ǩid��
	uint32_t locCount;          //��λ�ɹ�����
	uint32_t locErrNum;         //��λ�������
	uint32_t beaconNum;         //�����ű�֡����
	uint32_t txBeaNum;          //�����ű�֡Ӧ��֡����
	uint32_t rxPollNum;         //��������֡poll����
	uint32_t txPollNum;         //����pollӦ��֡����
	uint32_t rxFinalNum;        //����final֡����
//	uint32_t sendNum;           //���͵�֡����
//	uint32_t dataNumErr;        //�������ݴ������
	uint32_t idErr;
	uint32_t waitErr;           //�ȴ��������
	uint32_t elseErr;            //��֪������
	uint32_t dataErr;            //�������ݴ���
//	uint16_t maxTagId;           //��֪������ı�ǩid
//	uint16_t maxDist;            //��֪��������
	sDistNote *maxP;
}sAncDevInfo;

typedef struct
{
	double	 NoiseFigure;
	double   FP_Power;
	double   RX_Power;	
}sFrameQuality;




// ����һ����������
LIST_DEFINE(Dist, sDistNote, ANC_LOC_TAG_MAX)
#ifdef __UWB_ANCHOR_C__
	LIST_DEFINE_FUNC(Dist, sDistNote, ANC_LOC_TAG_MAX)
#endif /*__UWB_ANCHOR_C__*/






void FinalMsgGetTS_64(const uint8_t *ts_field, uint64_t *ts);
uint64_t GetSysTimeStamp_u64(void);

void UwbAnchorTask(void *pdata);
sDistList_N * GetDistBufPoint(void);
void DistBufDataCount(sDistNote * data);
void DelDistBufData(sDistList_N * before, sDistList_N * del);

void UwbAnchorIrqTask(void *pdata);
void RestartRxProcess(void);
void AnchorIdInit(void);
#endif /*__UWB_ANCHOR_H__*/