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
#define UWB_LISTEN_MODE 2            //��������uwb����

#define DECA_WORK_MODE  UWB_ALARM_MODE

extern u32 g_u32ID;

#define UWB_PERSONNEL_ENABLED   		0x01		//ʹ��UWB��Ա��λģ��
#define BEEP_ALARM_ENABLED				0x02		//ʹ�����ⱨ��������
#define GYRO_ANGLE_ENABLED				0x04		//ʹ�ܽǶȲɼ�����
#define WM433_WIRELESS_ENABLED			0x08		//ʹ��433����ģ�鹦��
/* Private defines -----------------------------------------------------------*/

/*�������ƶ���*/
//�豸����ʶ���Ƿ�ʹ�����ⱨ������UWB��Ա��λ���Ƕȴ���������
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

#if 0			//IOԤ��ʵ�ʲ�ʹ��
#define	LED_PORT					RCU_GPIOA
#define	LED_GPIO					GPIOA
#define LED_PIN						GPIO_PIN_3
#endif
/*�����ӳ�ʱ��*/
#define TX_ANT_DLY                 16440      //���������ӳ�
#define RX_ANT_DLY                 16440      //���������ӳ�

/*MAC ֡�ֶζ���*/
#define FRAME_CTRL_BYTE_FRAME_TYPE_BIT					0x0007    //MAC ֡���ƶ���֡����λ
#define FRAME_CTRL_BYTE_ACK_BIT							0x0020    //MAC ֡���ƶ���ȷ������λ
#define FRAME_CTRL_BYTE_PANID_BIT						0x0040    //MAC ֡���ƶ���PAN IDλ
#define FRAME_CTRL_BYTE_ADDR_LEN_BIT					0x4400    //MAC ֡���ƶ��е�ַ����λ

/*IEEE 802.15.4	֡����*/
#define FRAME_BEACON									0x0000		//�ű�֡
#define FRAME_DATA										0x0001		//����֡
#define FRAME_ACK										0x0002		//ȷ��֡
#define FRAME_CMD										0x0003		//����֡
#define FRAME_RESERVED_4								0x0004		//����֡4
#define FRAME_RESERVED_5								0x0005		//����֡5
#define FRAME_RESERVED_6								0x0006		//����֡6
#define FRAME_RESERVED_7								0x0007		//����֡7


#define ADDR_FOR_BEACON									0xFFFF		//�ű�֡�㲥��ַ
	

/*����*/
#define ALL_MSG_COMMON_LEN         							10				//ͨѶ֡ͨ�ò��ֳ���

#define RX_BUF_LEN            									55
#define RX_BUF_LEN_MAX											1024			//���ջ�������󳤶�

#define SHORT_ADDR_ASK_COMMON_LEN								12				//16λ��ַ����֡ͨ�ò��ֳ���


/*����λ����*/	
#define SOURCE_ADDR_SET_IDX			   							7		//Դ��ַ��������
#define ALL_MSG_COMMON_DEST_ADDR_IDX							5		//��Ϣ֡ͨ��Ŀ���ַ����

#define ALL_MSG_SN_IDX                        		2   //֡����ֵ����
//#define FINAL_MSG_POLL_TX_TS_IDX              		10  //finally��Ϣ�У�POLL����ʱ�������
//#define FINAL_MSG_RESP_RX_TS_IDX              		14  //finally��Ϣ�У�RESP����ʱ�������
//#define FINAL_MSG_FINAL_TX_TS_IDX             		18  //finally��Ϣ�У�FINAL����ʱ�������
//#define FINAL_MSG_TS_LEN                      		4   //finally��Ϣ�У�ʱ������ȣ�4���ֽ�

#define FRAME_DEC_TAG_ADDR_IDX									5		//ͨѶ����֡����ǩ��ַ����
#define FRMAE_DEC_RESP_TAG_ADDR_IDX								5		//ͨѶ�����ظ�֡����ǩ��ַ����
#define FRMAE_DEC_RESP_ANC_ADDR_IDX								7		//ͨѶ�����ظ�֡����վ��ַ����

#define FRAME_POLL_ANC_ADDR_IDX									5		//Poll֡����վ��ַ����
#define FRAME_POLL_TAG_ADDR_IDX									7		//Poll֡����ǩ��ַ����
#define	FRAME_RESP_TAG_ADDR_IDX									5		//Resp֡����ǩ��ַ����
#define FRAME_RESP_ANC_ADDR_IDX									7		//Resp֡����վ��ַ����
#define FRAME_FINAL_TAG_ADDR_IDX								7		//Final֡����ǩ��ַ����
#define FRAME_FINAL_ANC_ADDR_IDX								5		//Fianl֡����վ��ַ����


#define FRAME_32BIT_ADDR_TYPE_IDX								11	//32Bit��ַ����֡��֡��������

#define BEACON_FRAME_TYPE_IDX									7	//�ű�֡��֡����������


/*��ʱʱ�估��ʱʱ��*/
#define FRAME_COMM_DEC_TX_TO_RESP_RX_DLY_UUS			150			//ͨѶ����֡������ɵ����ջظ�֡�ӳ�ʱ��
//#define FRAME_COMM_DEC_RX_TO_RESP_TX_DLY_UUS			300			//ͨѶ����֡������ɵ����ͻظ�֡�ӳ�ʱ��
#define FRAME_COMM_DEC_RESP_RX_TIMEOUT_UUS				500			//ͨѶ����֡�ظ����ճ�ʱʱ��

/*��ǩ����*/
//#define POLL_TX_TO_RESP_RX_DLY_UUS          150		//POLL������ɵ���ʼ����RESP�ӳ�ʱ��
//#define RESP_RX_TIMEOUT_UUS                 2700	//RESP���ճ�ʱʱ��
//#define RESP_RX_TO_FINAL_TX_DLY_UUS         3100	//RESP������ɵ����Է���FINAL�ӳ�ʱ��

#define POLL_TX_TO_RESP_RX_DLY_UUS          150		//POLL������ɵ���ʼ����RESP�ӳ�ʱ��
#define RESP_RX_TIMEOUT_UUS                 2700	//RESP���ճ�ʱʱ��
#define RESP_RX_TO_FINAL_TX_DLY_UUS         300		//RESP������ɵ����Է���FINAL�ӳ�ʱ��   ��С����Ϊ260

/*��վ����*/
//#define POLL_RX_TIMEOUT_UUS				  1000	//POLL���ճ�ʱʱ��
//#define POLL_RX_TO_RESP_TX_DLY_UUS          2600  //POLL������ɵ���ʼ����RESP�ӳ�ʱ��
//#define RESP_TX_TO_FINAL_RX_DLY_UUS         500   //RESP������ɵ���ʼ����FINAL�ӳ�ʱ��
//#define FINAL_RX_TIMEOUT_UUS                4300  //FINAL���ճ�ʱʱ��

#define POLL_RX_TIMEOUT_UUS									1000	//POLL���ճ�ʱʱ��

//#define RESP_TX_TO_FINAL_RX_DLY_UUS         150   //RESP������ɵ���ʼ����FINAL�ӳ�ʱ��
//#define FINAL_RX_TIMEOUT_UUS                700  	//FINAL���ճ�ʱʱ��

#define LISTEN_CHANNEL_TIME									200		//�ŵ�����ʱ��200us

#define SPEED_OF_LIGHT                     					299702547  //����

#define ASK_ADDR_DELAY_MS									100

#define ADDR_16BIT_MAX_NUM									255			//16Bit��ַ�������ֵ--256����ַ����ֵ���Ϊ65535-3��0xFFFC��:0xFFFF--�㲥֡��0xFFFE--�޿ɷ����ַ��0xFFFD--CLE��ַ����ֵ�̶���

#define TAG_NUM_MAX											5

#define TAG_COMM_TO_ANC_TIME_MS								5		//��ǩ�ͻ�վ���β��ʱ�䣬��Ϊ�������ʱ�䵥λʹ��
#define TAG_COMM_TO_ALL_ANC_TIME_MS							100	//��ǩ�����л�վ�Ĳ��ͨѶʱ��

#define SYSTEM_COMM_CYCLE_MS								1000	//��Ա��λϵͳͨѶ����(����)
#define SYSTEM_COMM_CYCLE_LP_MS								30000	//��Ա��λϵͳͨѶ����(����)
#define TAG_EXIT_WORKSPACE_JUDGE_TIME_MS					5000	//��Ա�����ж�ʱ��

/*�������ڵĻ�վ����*/
#define	ANC_NUM												255	//��վ������

/*һ��ͨѶ�����ڣ���ǩ���վ��������Ӧ��ϵ*/
#define TAG_COMM_TO_ANC_NUM_MAX							7		//��ǩ��һ��ͨѶ������ͨѶ�Ļ�վ��



#define __DEBUG

#ifdef __DEBUG
	#define Debug(format,...)			printf("File: "__FILE__", Line: %d, "format,__LINE__,##__VA_ARGS__)			//��ӡ������Ϣ�����԰汾����
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
