/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "stm32f4xx_hal.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stdint.h"
#include "gd32f30x.h"

#include <ucos_ii.h>
#include "deca_device_api.h"
#include "deca_regs.h"
#include "filtering.h"
#include "math.h"
/* USER CODE END Includes */


/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#define UWB_TAG_MODE   	0
#define UWB_ALARM_MODE  1
#define UWB_LISTEN_MODE 2            //仅仅监听uwb数据

#define DECA_WORK_MODE  UWB_ALARM_MODE

extern u32 g_u32ID;

#define UWB_PERSONNEL_ENABLED   		0x01		//使能UWB人员定位模块
#define BEEP_ALARM_ENABLED				0x02		//使能声光报警器功能
#define GYRO_ANGLE_ENABLED				0x04		//使能角度采集功能
#define WM433_WIRELESS_ENABLED			0x08		//使能433无线模块功能
/* Private defines -----------------------------------------------------------*/

/*引脚名称定义*/
//设备类型识别是否使能声光报警器，UWB人员定位，角度传感器功能
#define ID_GPIO_CLK_ENABLE()		rcu_periph_clock_enable(RCU_GPIOA)
#define	ID0_PORT					RCU_GPIOA
#define	ID0_GPIO					GPIOA
#define ID0_PIN						GPIO_PIN_2

#define	ID1_PORT					RCU_GPIOA
#define	ID1_GPIO					GPIOA
#define ID1_PIN						GPIO_PIN_1

#define	ID2_PORT					RCU_GPIOA
#define	ID2_GPIO					GPIOA
#define ID2_PIN						GPIO_PIN_0

#define	ID3_PORT					RCU_GPIOC
#define	ID3_GPIO					GPIOC
#define ID3_PIN						GPIO_PIN_3

#if 0			//IO预留实际不使用
#define	LED_PORT					RCU_GPIOA
#define	LED_GPIO					GPIOA
#define LED_PIN						GPIO_PIN_3
#endif
/*天线延迟时间*/
#define TX_ANT_DLY                 16440      //接收天线延迟
#define RX_ANT_DLY                 16440      //发送天线延迟

/*MAC 帧字段定义*/
#define FRAME_CTRL_BYTE_FRAME_TYPE_BIT					0x0007    //MAC 帧控制段中帧类型位
#define FRAME_CTRL_BYTE_ACK_BIT							0x0020    //MAC 帧控制段中确认请求位
#define FRAME_CTRL_BYTE_PANID_BIT						0x0040    //MAC 帧控制段中PAN ID位
#define FRAME_CTRL_BYTE_ADDR_LEN_BIT					0x4400    //MAC 帧控制段中地址长度位

/*IEEE 802.15.4	帧类型*/
#define FRAME_BEACON									0x0000		//信标帧
#define FRAME_DATA										0x0001		//数据帧
#define FRAME_ACK										0x0002		//确认帧
#define FRAME_CMD										0x0003		//命令帧
#define FRAME_RESERVED_4								0x0004		//保留帧4
#define FRAME_RESERVED_5								0x0005		//保留帧5
#define FRAME_RESERVED_6								0x0006		//保留帧6
#define FRAME_RESERVED_7								0x0007		//保留帧7


#define ADDR_FOR_BEACON									0xFFFF		//信标帧广播地址
	

/*长度*/
#define ALL_MSG_COMMON_LEN         							10				//通讯帧通用部分长度

#define RX_BUF_LEN            									55
#define RX_BUF_LEN_MAX											1024			//接收缓存区最大长度

#define SHORT_ADDR_ASK_COMMON_LEN								12				//16位地址申请帧通用部分长度


/*数据位索引*/	
#define SOURCE_ADDR_SET_IDX			   							7		//源地址设置索引
#define ALL_MSG_COMMON_DEST_ADDR_IDX							5		//消息帧通用目标地址索引

#define ALL_MSG_SN_IDX                        		2   //帧序列值索引
//#define FINAL_MSG_POLL_TX_TS_IDX              		10  //finally消息中，POLL发送时间戳索引
//#define FINAL_MSG_RESP_RX_TS_IDX              		14  //finally消息中，RESP发送时间戳索引
//#define FINAL_MSG_FINAL_TX_TS_IDX             		18  //finally消息中，FINAL发送时间戳索引
//#define FINAL_MSG_TS_LEN                      		4   //finally消息中，时间戳长度：4个字节

#define FRAME_DEC_TAG_ADDR_IDX									5		//通讯声明帧，标签地址索引
#define FRMAE_DEC_RESP_TAG_ADDR_IDX								5		//通讯声明回复帧，标签地址索引
#define FRMAE_DEC_RESP_ANC_ADDR_IDX								7		//通讯声明回复帧，基站地址索引

#define FRAME_POLL_ANC_ADDR_IDX									5		//Poll帧，基站地址索引
#define FRAME_POLL_TAG_ADDR_IDX									7		//Poll帧，标签地址索引
#define	FRAME_RESP_TAG_ADDR_IDX									5		//Resp帧，标签地址索引
#define FRAME_RESP_ANC_ADDR_IDX									7		//Resp帧，基站地址索引
#define FRAME_FINAL_TAG_ADDR_IDX								7		//Final帧，标签地址索引
#define FRAME_FINAL_ANC_ADDR_IDX								5		//Fianl帧，基站地址索引


#define FRAME_32BIT_ADDR_TYPE_IDX								11	//32Bit地址数据帧，帧类型索引

#define BEACON_FRAME_TYPE_IDX									7	//信标帧，帧子类型索引


/*延时时间及超时时间*/
#define FRAME_COMM_DEC_TX_TO_RESP_RX_DLY_UUS			150			//通讯声明帧发送完成到接收回复帧延迟时间
//#define FRAME_COMM_DEC_RX_TO_RESP_TX_DLY_UUS			300			//通讯声明帧接收完成到发送回复帧延迟时间
#define FRAME_COMM_DEC_RESP_RX_TIMEOUT_UUS				500			//通讯声明帧回复接收超时时间

/*标签部分*/
//#define POLL_TX_TO_RESP_RX_DLY_UUS          150		//POLL发送完成到开始接收RESP延迟时间
//#define RESP_RX_TIMEOUT_UUS                 2700	//RESP接收超时时间
//#define RESP_RX_TO_FINAL_TX_DLY_UUS         3100	//RESP接收完成到考试发送FINAL延迟时间

#define POLL_TX_TO_RESP_RX_DLY_UUS          150		//POLL发送完成到开始接收RESP延迟时间
#define RESP_RX_TIMEOUT_UUS                 2700	//RESP接收超时时间
#define RESP_RX_TO_FINAL_TX_DLY_UUS         300		//RESP接收完成到考试发送FINAL延迟时间   最小可设为260

/*基站部分*/
//#define POLL_RX_TIMEOUT_UUS				  1000	//POLL接收超时时间
//#define POLL_RX_TO_RESP_TX_DLY_UUS          2600  //POLL接收完成到开始发送RESP延迟时间
//#define RESP_TX_TO_FINAL_RX_DLY_UUS         500   //RESP发送完成到开始接收FINAL延迟时间
//#define FINAL_RX_TIMEOUT_UUS                4300  //FINAL接收超时时间

#define POLL_RX_TIMEOUT_UUS									1000	//POLL接收超时时间

//#define RESP_TX_TO_FINAL_RX_DLY_UUS         150   //RESP发送完成到开始接收FINAL延迟时间
//#define FINAL_RX_TIMEOUT_UUS                700  	//FINAL接收超时时间

#define LISTEN_CHANNEL_TIME									200		//信道监听时间200us

#define SPEED_OF_LIGHT                     					299702547  //光速

#define ASK_ADDR_DELAY_MS									100

#define ADDR_16BIT_MAX_NUM									255			//16Bit地址分配最大值--256个地址，该值最大为65535-3（0xFFFC）:0xFFFF--广播帧，0xFFFE--无可分配地址，0xFFFD--CLE地址（该值固定）

#define TAG_NUM_MAX											5

#define TAG_COMM_TO_ANC_TIME_MS								5		//标签和基站单次测距时间，作为随机回退时间单位使用
#define TAG_COMM_TO_ALL_ANC_TIME_MS							100	//标签和所有基站的测距通讯时间

#define SYSTEM_COMM_CYCLE_MS								1000	//人员定位系统通讯周期(面内)
#define SYSTEM_COMM_CYCLE_LP_MS								30000	//人员定位系统通讯周期(面外)
#define TAG_EXIT_WORKSPACE_JUDGE_TIME_MS					5000	//人员离面判定时间

/*工作面内的基站数量*/
#define	ANC_NUM												255	//基站的数量

/*一个通讯周期内，标签与基站的数量对应关系*/
#define TAG_COMM_TO_ANC_NUM_MAX							7		//标签在一个通讯周期内通讯的基站数



#define __DEBUG

#ifdef __DEBUG
	#define Debug(format,...)			printf("File: "__FILE__", Line: %d, "format,__LINE__,##__VA_ARGS__)			//打印报文信息，测试版本功能
#else
	#define Debug(format,...)

#endif

/* USER CODE END Private defines */

//void WriteToMsg(uint8_t *pMsg, uint8_t *pdata, uint8_t MsgOffset, uint8_t DataLength);
uint64_t GetRxTimeStamp_u64(void);


u32 Get_DevID(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
