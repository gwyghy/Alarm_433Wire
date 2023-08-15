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


#define ANCHOR_TX_ANT_DLY                 16440      //发送天线延迟
#define ANCHOR_RX_ANT_DLY                 16440      //接收天线延迟

#define ANCHOR_PAN_ID 										0xDECA					//基站PAN ID值

#define ANCHOR_ADDR_OFF						7		//基站地址在帧中的偏移量
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

#define FRAME_CTRL_BYTE_ADDR_LEN_BIT						0x4400    //MAC 帧控制段中地址长度位
/*定位帧格式*/
//#define FINAL_MSG_TS_LEN                      		5  //finally消息中，时间戳长度：5个字节
//#define FINAL_MSG_POLL_TX_TS_IDX              		10  //finally消息中，POLL发送时间戳索引
//#define FINAL_MSG_RESP_RX_TS_IDX              		(FINAL_MSG_POLL_TX_TS_IDX + FINAL_MSG_TS_LEN)  //finally消息中，RESP发送时间戳索引
//#define FINAL_MSG_FINAL_TX_TS_IDX             		(FINAL_MSG_RESP_RX_TS_IDX +  FINAL_MSG_TS_LEN) //finally消息中，FINAL发送时间戳索引

#define MSG_TIME_LEN                      		     5  //时间戳长度
#define BEACON_ACK_T2_DIR              		       ANC_DATA_HEAD_SIZE  //信标应答帧t2数据位置
#define BEACON_ACK_T3_DIR              		       (BEACON_ACK_T2_DIR + MSG_TIME_LEN)  //信标应答帧t3数据位置



#define ANC_LOC_TAG_MAX									10	//基站能同时容纳的标签量
#define ANC_TOF_REC_MAX									10		//标签在一个通讯周期内与同一基站的通讯次数
#define ANC_TOF_MIN_DATA                10

typedef struct
{
	uint16_t lengId;
	uint32_t count;
}sTimeData;


typedef struct
{
	uint16_t tagId;                     //记录的标签id
	uint16_t dist[ANC_TOF_REC_MAX];     //该标签离基站的距离buf
	uint16_t realDist;                  //经过去抖后的距离
	uint16_t heartCout;           			//心跳计数
	uint16_t oldHeartCout;           		//旧的心跳计数
	uint8_t  commCnt;							      //单周期内标签与同一基站的通讯次数
	uint8_t  wr;                        //写标签id
	uint8_t  mark;
	uint8_t  maxPtr;                      //最大值指针
	uint8_t  minPtr;	                   //最小值指针
	uint8_t  errCount;                   //离线错误计数
	uint8_t  right;                      //权限
	uint8_t  FisrtDistErr;
	uint16_t ErrDist;
}sDistNote;

enum{
	UWB_RX_STATE = 0,
	UWB_TX_STATE
};

typedef struct
{
	uint16_t devNum;            //设备编号
	uint16_t devState;
	uint16_t uwbStata;          //uwb 状态
	uint16_t currDevId;         //当前进行定位的标签id号
	uint32_t locCount;          //定位成功次数
	uint32_t locErrNum;         //定位错误次数
	uint32_t beaconNum;         //接收信标帧个数
	uint32_t txBeaNum;          //发送信标帧应答帧个数
	uint32_t rxPollNum;         //接收数据帧poll个数
	uint32_t txPollNum;         //发送poll应答帧个数
	uint32_t rxFinalNum;        //接收final帧个数
//	uint32_t sendNum;           //发送的帧个数
//	uint32_t dataNumErr;        //接收数据错误个数
	uint32_t idErr;
	uint32_t waitErr;           //等待错误个数
	uint32_t elseErr;            //不知名错误
	uint32_t dataErr;            //计算数据错误
//	uint16_t maxTagId;           //已知最大距离的标签id
//	uint16_t maxDist;            //已知的最大距离
	sDistNote *maxP;
}sAncDevInfo;

typedef struct
{
	double	 NoiseFigure;
	double   FP_Power;
	double   RX_Power;	
}sFrameQuality;




// 声明一级缓存链表
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
