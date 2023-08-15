/*********************************************************************************************************************************
** 文件名:  protocol.h
** 描　述:  数据协议
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

#ifndef __PROTOCOL_H__
#define	__PROTOCOL_H__

#ifndef STM32F10x
typedef int32_t  s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef const int32_t sc32;  /*!< Read Only */
typedef const int16_t sc16;  /*!< Read Only */
typedef const int8_t sc8;   /*!< Read Only */

typedef __IO int32_t  vs32;
typedef __IO int16_t  vs16;
typedef __IO int8_t   vs8;

typedef __I int32_t vsc32;  /*!< Read Only */
typedef __I int16_t vsc16;  /*!< Read Only */
typedef __I int8_t vsc8;   /*!< Read Only */

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef const uint32_t uc32;  /*!< Read Only */
typedef const uint16_t uc16;  /*!< Read Only */
typedef const uint8_t uc8;   /*!< Read Only */

typedef __IO uint32_t  vu32;
typedef __IO uint16_t vu16;
typedef __IO uint8_t  vu8;

typedef __I uint32_t vuc32;  /*!< Read Only */
typedef __I uint16_t vuc16;  /*!< Read Only */
typedef __I uint8_t vuc8;   /*!< Read Only */

#endif 

/***************************************头文件包含****************************************************************/
//#include "stm32f10x.h"
/***************************************宏定义****************************************************************/
#define TRUE								1
#define FALSE								0
#define ENABLED								1
#define DISABLED							0
//CAN通信帧ID中帧类型-表示该数据帧是什么命令
//IR红外协议帧结构
typedef struct{
	u32 Type:4;
	u32 ScNoLSB3:3;
	u32 Sign1:1;
	u32 ScNoMSB6:6;
	u32 Dir:1;
	u32 Sign2:1;
	u32 Result:1;
	u32 ACK:1;
	u32 RemoteID:4;
	u32 ScNoIncDir:1;
	u32 Sign3:1;
} IR_INFO_s;
//IR红外协议帧联合
typedef union{
	IR_INFO_s	sIrInfo;
	u8	u8IrInfo[3];
} IR_INFO_u;

//无线对码成功与否识别位
#define WL_MATCH_RESULT_MASK	0x01
#define WL_MATCH_OK				0x01
#define WL_MATCH_FAILURE		0x00
//遥控器类型识别位
#define MODEL_MASK				0x80
#define MODEL_FULL				0x00
#define MODEL_SIMPLE			0x80

#define	WL_SYNC					(u16)0x0800	//字节	2			无线同步字
#define	PT_SER_SC_NO			(u16)0x01f0		//字节	2			支架编号	本地/整体	服务		

#define FT_TYPE_MAX							        22			//帧类型个数，不包含更新程序相关

#define FT_WL_TO_SC_BEAT					        0			//无线 发给 SC 的心跳数据

#define FT_WL_TO_SC_IR_REPORT				        1			//无线 发给 SC 的红外上报数据
#define FT_SC_TO_WL_DBUS					        2			// SC 发给无线的总线数据，需要应答
#define FT_WL_TO_SC_DBUS					        3			//无线 发给 SC 的总线数据，需要应答
#define FT_WL_TO_SC_IR_MATCH				        4			//无线 发给 SC 的红外对码
#define FT_SC_TO_WL_IR_MATCH_RST			        5			//SC 发给无线的红外对码结果，需要应答

#define FT_WL_TO_SC_RF_MATCH				        6			//无线 发给 SC 的无线对码
#define FT_SC_TO_WL_RF_MATCH_RST			        7			//无线与红外接收器发给 SC 的无线对码结果，需要应答
#define FT_SC_TO_WL_MATCH_SC_GROUP_INFO		        8			// SC发送给WL，对码架发送成组信息
#define FT_SC_TO_WL_UC_SC_GROUP_INFO		        9			// SC发送给WL，被控架发送成组信息
#define FT_WL_TO_SC_CTL_DATA				        10			//无线 发给 SC 的控制数据
#define FT_SC_TO_WL_CTL_DATA_RECEPT			        11			// SC 发送给 WL 控制是否接收，需要应答
#define FT_WL_TO_SC_CTL_DATA_SQN			        12			//无线 发给 SC 的控制数据(按键连续按下)
#define FT_WL_TO_SC_CTL_DATA_LIFT			        13			//无线 发给 SC 的控制数据(按键抬起)
#define FT_SC_TO_WL_DISCONNECT				        14			// SC 发送给 WL 解除对码
#define FT_WL_TO_SC_DISCONNECT				        15			//无线 发给 SC 解除对码
#define FT_SC_TO_WL_RESET_PAR				        16			// SC 发送给 WL 重设参数
#define FT_SC_TO_WL_LIFT_RECEPT				        17			// SC 发送给 WL 按键抬起是否接受
#define FT_WL_TO_SC_AUTO_PRESS_OPEN                 18          // WL 发送给 SC 设置自动补压开启
#define FT_SC_TO_WL_AUTO_PRESS_OPEN_RECEPT          19          // SC 发送给 WL 设置自动补压开启是否接收

#define FT_WL_TO_SC_AUTO_PRESS_CLOSE                20          // WL 发送给 SC 设置自动补压关闭
#define FT_SC_TO_WL_AUTO_PRESS_CLOSE_RECEPT         21          // SC 发送给 WL 设置自动补压关闭是否接收
#define FT_SC_TO_WL_RESET_PAR_WL			        22			// SC 发送给 WL 设置无线红外接收模块的参数

//#define FT_SC_TO_WL_RESET_PAR_WL			30			// SC 发送给 WL 设置无线红外接收模块的参数
//#define FT_SC_TO_WL_MATCH_END				31			// SC 发送给 WL 对码结束
#define FT_WL_TO_SC_RCV_PROGRAM				        0x3FE		// WL 发送给 SC，接收程序上报帧
#define FT_WL_TO_SC_UPDATE_PROGRAM			        0x3FD		// WL 发送给 SC，更新程序上报帧

//CAN数据帧字节长度
#define FRM_DLC_ONE_BYTE					1			
#define FRM_DLC_TWO_BYTE					2			
#define FRM_DLC_THREE_BYTE					3			
#define FRM_DLC_FOUR_BYTE					4			
#define FRM_DLC_FIVE_BYTE					5			
#define FRM_DLC_SIX_BYTE					6			
#define FRM_DLC_SENVEN_BYTE					7			
#define FRM_DLC_EIGHT_BYTE					8			

//红外通信数据的掩码
#define MASK_FLAG							(0x80)        // 获取标志位的掩码
#define MASK_SC_L							0x70
#define MASK_FT								0x0F
#define MASK_RD2							0x80
#define MASK_DIR							0x40
#define MASK_SC_H							0x3F
#define MASK_RD1							0xD0
#define MASK_ID								0x3D
#define MASK_ACK							0x02
#define MASK_RST							0x01
//获取对应字段的移位数
#define BITS_FLAG							(7)        // 获取标志位移位数
#define BITS_SC_L							(4)
#define BITS_FT								(0)
#define BITS_RD2							(7)
#define BITS_DIR							(6)
#define BITS_SC_H							(0)
#define BITS_RD1							(6)
#define BITS_ID								(2)
#define BITS_ACK							(1)
#define BITS_RST							(0)

//定时器相关宏定义
#define TIMER_EXPIRED	(u32)0x0				//计时结束
#define TIMER_DONE		(u32)0xfffffffe			//计时结束已处理
#define TIMER_CLOSED	(u32)0xffffffff			//计时器关闭

//定义错误状态
#define SUCCESS 0
#define ERROR_1 1
#define ERROR_2 2
#define ERROR_3 3
#define ERROR_4 4
#define ERROR_5 5
#define ERROR_6 6
#define ERROR_7 7
#define ERROR_8 8
#define ERROR_9 9
#define ERROR_10 10

#define ERROR_UP_OVERFLOW     ERROR_1
#define ERROR_DOWN_OVERFLOW   ERROR_2
#define ERROR_OUTOFFRAME      ERROR_3
#define ERROR_OVERTIME		  ERROR_4
#define ERROR_DATA_NUM		  ERROR_5

/***************************************枚举类型定义****************************************************************/
//使能或失能标志
enum{
	eDISABLED=0,				//(u16)0		//禁止
	eENABLED					//(u16)1		//允许
};
//应答标志
enum{
	eNOACK=0,
	eACK
};
// 数据发送方向
enum{
	eDirectWL = 0,				// 手持设备或者红外发射器->无线红外接收端
	eDirectHS					// 无线红外接收端 ->手持设备或者红外发射器
};
/***************************************数据结构定义****************************************************************/
////协议帧ID结构定义
//typedef struct{
//	u32 RID:3;						//目标ID(接收方)
//	u32 TID:3;						//源ID(发送方)
//	u32 FT:10;						//帧类型
//	u32 SN:4;						//子帧序列号
//	u32 SUM:5;						//总帧数
//	u32 SUB:1;						//标明是总帧还是子帧
//	u32 ACK:1;						//应答位
//	u32 RD:2;						//保留位			
//} sCanFrameId;
//CAN通信帧帧格式
typedef struct {
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
	u8 u8Data[8];
	u16 u16DLC;
}sCanFrame;
//红外通信帧格式
typedef struct{
	union{
		struct{
			u32 RST:1;				// 对码结果
			u32 ACK:1;				// 应答位
			u32 ID:4;				// 设备ID
			u32 RD1:2;				// 保留位
			u32 SC_H:6;				// 控制架号的高6位
			u32 DIR:1;				// 数据发送方向
			u32 RD2:1;				// 保留位
			u32 FT:4;				// 动作类型
			u32 SC_L:3;				// 控制架号的低3位
			u32 FLAG:1;				// 标志位，判断一帧数据的起始位
		}CUSTOM_DATA;
		u32 u32CustomData;
	}u32IRDATA;	
	u8 u8DLC;
}sIrFrame;

#define GET_CRYP_KEY()	(0x00)		// 获取密钥

#endif

/*********************************天津华宁电子有限公司*************************************************************/
