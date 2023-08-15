/********************************************************************************
* 文件名称：	logic.h
* 作	者：	马如意   
* 当前版本：   	V1.0
* 完成日期：    2014.12.08
* 功能描述: 	定义logic.h头文件
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
#ifndef _LOGIC_H_
#define _LOGIC_H_
/********************************************************************************
* .h头文件
*********************************************************************************/
#include "logic.h"
//#include "angle_sensor.h"
//#include "iwdg.h"

#include "beep_app.h"
#include "led_app.h"
/********************************************************************************
* #define宏定义
*********************************************************************************/
/********FLASH内存地址划分*****/
#define PARAM_1_START_ADDR			0x0803F000//参数区域的起始地址
#define PARAM_2_START_ADDR			0x0803F800//参数区域的起始地址

#define ANGLE_MODIFY_DEGREE_MAX		181//修正角度的最大值

#define ANGLE_HEART_BYTE0			'P'
#define ANGLE_HEART_BYTE1			'O'
#define ANGLE_HEART_BYTE2			'S'	

#define SUB_DEVICE_NUM			0x03
/*************一些结构体定义**************/
/**是否应答定义***/
enum
{
	NO_ACK = 0,
	ACK = 1
};

/**组合帧类型定义****/
enum
{
	COMBIN_FRAME = 0,//组合帧
	SUB_FRAME = 1//子帧
	
};

/**帧长度定义**/
typedef enum
{
	CAN_LENGTH_0 = 0x00,
	CAN_LENGTH_1 = 0x01,
	CAN_LENGTH_2 = 0x02,
	CAN_LENGTH_3 = 0x03,

	CAN_LENGTH_4 = 0x04,
	CAN_LENGTH_5 = 0x05,
	CAN_LENGTH_6 = 0x06,
	CAN_LENGTH_7 = 0x07,
	CAN_LENGTH_8 = 0x08	
}STFRAME_LENGTH_TYPE;

/**本项目中使用扩展帧格式***/
__packed typedef struct
{
	__packed union
	{
		__packed struct
		{
			unsigned int	RxID:3;				//接收方ID
			unsigned int	TxID:3;				//发送方ID
			unsigned int	FrameType:10;		//帧类别	
			unsigned int	LiuShuiNumb:4;		//流水序列号，0x0-0xf循环，ACK=1应答帧的流水，采用被应答帧的流水
			unsigned int	Sum:5;		//子帧序号或
			unsigned int	Sub:1;		//组合帧/子帧，0：组合帧总数帧：1：子帧
			unsigned int	ACK:1;				//应答标志，1:应答，0:无需应答			
			unsigned int	Reservd:2;			//预留区域。用于传程序:01,其他:00	
			unsigned int	NoUsed:3;			//不存在区域
		} ID;//帧ID
		u32 u32Id;//帧ID
	} u32ID;	//帧ID
	u8	u8DT[8];			//帧数据
	u16	u16DLC;				//帧长度
} stFRAME;

/**与倾角传感器，CAN帧类型定义*****/
typedef enum
{
	ANGLE_HEART = 0,
	ANGLE_REPORT_VALUE = 0x100,//角度上报帧
	ANGLE_CHECK_VALUE = 0x101,//角度查询帧

	ANGLE_SET_DEV_TYPE = 0x001,//设置设备类型
	ANGLE_GET_DEV_TYPE = 0x002,//获取设备类型	
	ANGLE_GET_DEV_TYPE_ACK = 0x003,//获取设备类型应答		
		
	ANGLE_SET_MODIFY_PARAM_NUMB = 0x004,//设置出厂修正值总数
	
	ANGLE_GET_MODIFY_PARAM_NUMB = 0x005,//回读出厂修正值总数
	ANGLE_GET_MODIFY_PARAM_NUMB_ACK = 0x006,//回读出厂修正值总数应答
	
	ANGLE_SET_MODIFY_VALUE = 0x007,//设置出厂修正值

	ANGLE_GET_MODIFY_VALUE = 0x008,//回读出厂修正值
	ANGLE_GET_MODIFY_VALUE_ACK = 0x009,//回读出厂修正值应答

	ANGLE_MODIFY_PARAM_SAVE = 0x00A,//出厂修正值参数保存
	
	ANGLE_SET_WORK_PARAM = 0x105,//设置工作参数
	
	ANGLE_GET_WORK_PARAM = 0x106,//回读工作参数
	ANGLE_GET_WORK_PARAM_ACK = 0x107,//回读工作参数应答

	ANGLE_PRG_DOWNLOAD_FLAG = 0x3FC,//下载程序完成标志上报帧
	ANGLE_PRG_UPDATE_FLAG = 0x3FD,//更新程序完成标志上报帧
	ANGLE_READ_VERSION = 0x3FE,//读取版本号
	ANGLE_READ_VERSION_ACK = 0x3FF,//回读版本号应答
	
	ANGLE_CANFRAME_TYPE_MAX = 0x400//帧类型的最大值
}ANGLE_CANFRAME_TYPE;

/**角度上报方式***/
enum
{
	ANGLE_CHECK_MODE = 0x01,	//主机查询上报模式
	ANGLE_REPORT_MODE = 0x02	//主动立即上报模式
};

/**逻辑层信息处理所使用的参数消息定义***/
typedef enum
{
	LOGIC_SET_DEVTYPE_INF = 0x00,	//设置设备类型信息
	LOGIC_SET_MODIFY_PARAM_NUMB,	//设置修正值参数个数
	LOGIC_SET_MODIFY_PARAM,			//设置修正值
	LOGIC_SET_CALIBRATE_FLAG,		//设置修正标志
	
	LOGIC_SAVE_PARAM_MSG,		//保存参数

	LOGIC_GET_DEVTYPE_INF,		//设置设备类型信息
	LOGIC_GET_MODIFY_PARAM_NUMB,//获取修正值参数个数	
	LOGIC_GET_MODIFY_PARAM,		//获取修正值
	LOGIC_GET_MODIFY_PARAM_ALL,	//获取修正参数的所有数值
	LOGIC_GET_CALIBRATE_FLAG,	//获取修正标志

	LOGIC_PARAM_MSG_MAX//逻辑层处理消息的最大值
}LOGIC_PARAM_MSG_TYPE;

/**逻辑层信息处理所使用的运行消息定义***/
typedef enum
{
	LOGIC_SET_SYSTEM_RUN_STATUS = 0x00,//设置系统运行状态
	LOGIC_SET_WORK_MODE,		//设置工作模式
	LOGIC_SET_REPORT_INTERVAL,	//设置角度上报间隔
	LOGIC_SET_ANGLEVALUE_X,		//设置角度上报数值
	LOGIC_SET_ANGLEVALUE_Y,		//设置角度上报数值
	LOGIC_SET_ANGLEVALUE_Z,		//设置角度上报数值

	LOGIC_GET_SYSTEM_RUN_STATUS,//设置系统运行状态	
	LOGIC_GET_WORK_MODE,		//获取工作模式
	LOGIC_GET_REPORT_INTERVAL,	//获取角度上报间隔
	LOGIC_GET_ANGLEVALUE_X,		//获取角度上报数值
	LOGIC_GET_ANGLEVALUE_Y,		//获取角度上报数值
	LOGIC_GET_ANGLEVALUE_Z,		//获取角度上报数值
	
	LOGIC_RUNINF_MSG_MAX		//逻辑层处理消息的最大值
}LOGIC_RUNINF_MSG_TYPE;

enum
{
	SYSTEM_RUN_STATUS_NORMAL = 0x00,	
	SYSTEM_RUN_STATUS_INIT = 0x01,
	SYSTEM_RUN_STATUS_ADJUST  = 0x02,
	SYSTEM_RUN_STATUS_UPPRG  = 0x03	
};

/**加速度芯片枚举***/
enum
{
	ACCR_CHIP_SCA100T = 0x01,
	ACCR_CHIP_ADXL203 = 0x02
};

/**角度传感器通讯端口枚举***/
enum
{
	ANGLE_COMM_CAN1 = 0x01,
	ANGLE_COMM_CAN2 = 0x02,	
	ANGLE_COMM_CAN1_2 = ANGLE_COMM_CAN1 | ANGLE_COMM_CAN2,			
};

/*角度传感器数据采集轴向的类型枚举***/
enum
{
	ANGLE_TYPE_X = 0x01,
	ANGLE_TYPE_Y = 0x02,
	ANGLE_TYPE_X_Y =  ANGLE_TYPE_X | ANGLE_TYPE_Y,
};

enum
{
	ANGLE_SUB_TYPE = 0,
	HEIGHT_SUB_TYPE = 1
};

/**设置参数最大值的数据类型****/
typedef struct
{
	u16 u16AngleParamNumbX;
	u16 u16AngleParamNumbY;//参数的最大值
}LOGIC_SET_PARAMNUMB_TYPE;

/**设置角度参数的数据类型****/
__packed typedef struct
{
	u8 u8RservedFlag;
	u8 u8AnleXYflag;
	s16 s16AngleValue;//角度值
	u16 u16AngleAdValue;//AD采样的数值
	u16 u16StoragePosition;//参数存储的位置	
}LOGIC_SET_ANGLEPARAM_TYPE;

/**出厂修正值参数类型**/
__packed typedef struct
{
	u32 u32ModifyX_ParamNumb;//X角度设置参数的个数
	s16 s16ModifyX_AngleValue[ANGLE_MODIFY_DEGREE_MAX];//X修正角度值（以0.01度为单位，区分正负)
	u16 u16ModifyX_AdValue[ANGLE_MODIFY_DEGREE_MAX];//X修正AD值（X AD采样的修正值,0°校验)  对动态倾角，此处存放的是测量角度值，以0.01度为单位

	u32 u32ModifyY_ParamNumb;//Y角度设置参数的个数
	s16 s16ModifyY_AngleValue[ANGLE_MODIFY_DEGREE_MAX];//Y修正角度值（以0.01度为单位，区分正负)
	u16 u16ModifyY_AdValue[ANGLE_MODIFY_DEGREE_MAX];//Y修正AD值（X AD采样的修正值,0°校验)  对动态倾角，此处存放的是测量角度值，以0.01度为单位
}LOGIC_MODIFYPARAM_TYPE;

/**设备类型结构体**/
__packed typedef struct
{
	u8 u8Resverd;//预留
	u8 u8AccrChipType;//角度传感器芯片类型
	u8 u8DevCommType;//设备通讯端口类型
	u8 u8DevSampleXyType;//采集X轴、Y轴定义
	u16 u16PoseXRange;//X轴测量范围,±u16PoseXRange
	u16 u16PoseYRange;//Y轴测量范围,±u16PoseXRange
}LOGIC_POSEDEV_INF_TYPE;
/********************************************************************************
* 常量定义
*********************************************************************************/
#define ANGLE_VALUE_INVALID					0x9999//无效的角度数值

/********************************************************************************
* 全局变量声明
*********************************************************************************/

/********************************************************************************
* #define宏定义
*********************************************************************************/

#include "iapupdate.h"
#include "caniap.h"

/********************************************************************************
* 函数声明
*********************************************************************************/
u32 LogicInit(void);
u32 LogicParamApi(LOGIC_PARAM_MSG_TYPE sMsg, void *pData);
u32 LogicRunInfApi(LOGIC_RUNINF_MSG_TYPE sMsg, void *pData);

#endif
