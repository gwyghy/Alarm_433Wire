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
*             can底层驱动
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
//声光报警相关定义
 ***************************************************************************************************/
extern u16 LightFlag;

extern u8 RGrayScale;				//红色灰度值
extern u8 GGrayScale;				//绿色灰度值
extern u8 BGrayScale;				//蓝色灰度值

extern u8 LED_NUM;		//根据灯珠数量选择不同驱动方式
extern u8 WaterFlow_Type;		//LED流水灯模式

extern uint16_t Breathestep_R;
extern uint16_t Breathestep_G;
extern uint16_t Breathestep_B;

#define FRM_ID_BEFORE_ALARM		0x00000107				// 预警帧ID
#define FRM_ID_ACTION_ALARM		0x00000147				// 动作执行报警帧ID

#define AGING_TEST_BYTE			0xFE					//适应于老化工装控制点亮所有LED，长时间老化LED
#define POWER_TEST_BYTE			0xFD					//适应于成品检验测试单板最大电流控制
#define LED_TEST_BYTE			0xFC					//适应于成品检验测试单板各个LED是否正常

#define BL_TIMEOUT_VALUE		200						//通讯超时时间，收不到控制命令主动停止声光报警状态

//UWB定位相关定义
#define REQ_CONFIG_DELAY	500
#define UWB_HEART_DELAY		4000			//心跳数据发送间隔
#define UWB_MODEL			0x31

#define BEEP_LIGHT_MODEL	0x30			//声光报警设备类型
/****************************************************************************************************
 UWB MODULE STATE
 ***************************************************************************************************/
enum
{
	UWB_INIT = 0,				//UWB定位初始化未配置状态
	UWB_NORMAL					//配置完成，正常运行
};

enum
{
	UWB_CONFIG_REQ = 0,				//参数申请
	UWB_INFO_REPORT,				//1：人员定位信息上报
	REQ_UWB_INFO,					//2：查询人员定位模块的标签距离
	SET_UWB_PARA,					//3：设置人员定位模块工作参数
	 //ID_BEFORE_ALARM,				//4
	 //ID_ACTION_ALARM,				//5
	UWB_RESET = 6,					//6：复位人员定位模块或声光报警器
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
} CanHeadID;
 /****************************************************************************************************
uwb dev of info
 ***************************************************************************************************/
typedef struct
{
	uint16_t standNum;   //支架控制器编号
	union {
		struct {
			uint8_t workType:1; //工作模式
			uint8_t distType:1;  //距离类型
			uint8_t reserve:2;   //保留
			uint8_t rate:4;      //发射功率
		}sUwbSet;
		uint8_t uPara;
	}uwbPara;
	//		uint8_t devType;
	uint8_t interval;		//时间间隔
	uint16_t extent;		//检测范围
	uint16_t height;		//支架高度
					
}sUWB_RUN_INFO;				//运行参数信息
 

typedef struct
{
	uint8_t peoplenum; //人数
	union {
		struct {
			uint8_t tagState:4; //标签状态
			uint8_t author:4;    //权限
		}sTag;
		uint8_t tag;
	}tagInfo;
	uint16_t tagId;       //标签id
	uint16_t tagDist;     //标签距离
	uint8_t  reserve;
	uint8_t  crc;
}sTag_Info;
/****************************************************************************************************
CAN TX MESSAGE 	STATE
 ***************************************************************************************************/
enum									//发送结果定义 
{
	CAN_TRS_FAIL = 0,
	CAN_TRS_SUCCESS
};


/****************************************************************************************************
CAN TX MESSAGE 	TYPE ENUM
 ***************************************************************************************************/
enum									//发送类型
{
	CAN_ASK = 0,
	CAN_ACK
};

/****************************************************************************************************
 CAN rx and tx task  stacks
 ***************************************************************************************************/
#define CANTX_TASK_SIZE                        384u
#define CANRX_TASK_SIZE                        384u

//声光报警器相关变量
extern u32 BeepLightRxTimeoutTimer;		//声光报警器通讯状态中断超时定时器，超时立即清除报警状态，防止声光状态停不下来
extern u16 BeepCaseFlag;				// 蜂鸣器标志
extern u16 u16BeepOffTimer;				//蜂鸣器间歇计时（xms）
extern u16 u16BeepOnTimer;				//蜂鸣器告警计时（xms）

extern u16 u16LightOffTimer;			//LED间歇计时（xms）
extern u16 u16LightOnTimer;				//LED告警计时（xms）

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


