/********************************************************************************
* �ļ����ƣ�	logic.h
* ��	�ߣ�	������   
* ��ǰ�汾��   	V1.0
* ������ڣ�    2014.12.08
* ��������: 	����logic.hͷ�ļ�
* ��ʷ��Ϣ��   
*           	�汾��Ϣ     ���ʱ��      ԭ����        ע��
*
*       >>>>  �ڹ����е�λ��  <<<<
*          	  3-Ӧ�ò�
*          ��  2-Э���
*             1-Ӳ��������
*********************************************************************************
* Copyright (c) 2014,������������޹�˾ All rights reserved.
*********************************************************************************/
#ifndef _LOGIC_H_
#define _LOGIC_H_
/********************************************************************************
* .hͷ�ļ�
*********************************************************************************/
#include "logic.h"
//#include "angle_sensor.h"
//#include "iwdg.h"

#include "beep_app.h"
#include "led_app.h"
/********************************************************************************
* #define�궨��
*********************************************************************************/
/********FLASH�ڴ��ַ����*****/
#define PARAM_1_START_ADDR			0x0803F000//�����������ʼ��ַ
#define PARAM_2_START_ADDR			0x0803F800//�����������ʼ��ַ

#define ANGLE_MODIFY_DEGREE_MAX		181//�����Ƕȵ����ֵ

#define ANGLE_HEART_BYTE0			'P'
#define ANGLE_HEART_BYTE1			'O'
#define ANGLE_HEART_BYTE2			'S'	

#define SUB_DEVICE_NUM			0x03
/*************һЩ�ṹ�嶨��**************/
/**�Ƿ�Ӧ����***/
enum
{
	NO_ACK = 0,
	ACK = 1
};

/**���֡���Ͷ���****/
enum
{
	COMBIN_FRAME = 0,//���֡
	SUB_FRAME = 1//��֡
	
};

/**֡���ȶ���**/
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

/**����Ŀ��ʹ����չ֡��ʽ***/
__packed typedef struct
{
	__packed union
	{
		__packed struct
		{
			unsigned int	RxID:3;				//���շ�ID
			unsigned int	TxID:3;				//���ͷ�ID
			unsigned int	FrameType:10;		//֡���	
			unsigned int	LiuShuiNumb:4;		//��ˮ���кţ�0x0-0xfѭ����ACK=1Ӧ��֡����ˮ�����ñ�Ӧ��֡����ˮ
			unsigned int	Sum:5;		//��֡��Ż�
			unsigned int	Sub:1;		//���֡/��֡��0�����֡����֡��1����֡
			unsigned int	ACK:1;				//Ӧ���־��1:Ӧ��0:����Ӧ��			
			unsigned int	Reservd:2;			//Ԥ���������ڴ�����:01,����:00	
			unsigned int	NoUsed:3;			//����������
		} ID;//֡ID
		u32 u32Id;//֡ID
	} u32ID;	//֡ID
	u8	u8DT[8];			//֡����
	u16	u16DLC;				//֡����
} stFRAME;

/**����Ǵ�������CAN֡���Ͷ���*****/
typedef enum
{
	ANGLE_HEART = 0,
	ANGLE_REPORT_VALUE = 0x100,//�Ƕ��ϱ�֡
	ANGLE_CHECK_VALUE = 0x101,//�ǶȲ�ѯ֡

	ANGLE_SET_DEV_TYPE = 0x001,//�����豸����
	ANGLE_GET_DEV_TYPE = 0x002,//��ȡ�豸����	
	ANGLE_GET_DEV_TYPE_ACK = 0x003,//��ȡ�豸����Ӧ��		
		
	ANGLE_SET_MODIFY_PARAM_NUMB = 0x004,//���ó�������ֵ����
	
	ANGLE_GET_MODIFY_PARAM_NUMB = 0x005,//�ض���������ֵ����
	ANGLE_GET_MODIFY_PARAM_NUMB_ACK = 0x006,//�ض���������ֵ����Ӧ��
	
	ANGLE_SET_MODIFY_VALUE = 0x007,//���ó�������ֵ

	ANGLE_GET_MODIFY_VALUE = 0x008,//�ض���������ֵ
	ANGLE_GET_MODIFY_VALUE_ACK = 0x009,//�ض���������ֵӦ��

	ANGLE_MODIFY_PARAM_SAVE = 0x00A,//��������ֵ��������
	
	ANGLE_SET_WORK_PARAM = 0x105,//���ù�������
	
	ANGLE_GET_WORK_PARAM = 0x106,//�ض���������
	ANGLE_GET_WORK_PARAM_ACK = 0x107,//�ض���������Ӧ��

	ANGLE_PRG_DOWNLOAD_FLAG = 0x3FC,//���س�����ɱ�־�ϱ�֡
	ANGLE_PRG_UPDATE_FLAG = 0x3FD,//���³�����ɱ�־�ϱ�֡
	ANGLE_READ_VERSION = 0x3FE,//��ȡ�汾��
	ANGLE_READ_VERSION_ACK = 0x3FF,//�ض��汾��Ӧ��
	
	ANGLE_CANFRAME_TYPE_MAX = 0x400//֡���͵����ֵ
}ANGLE_CANFRAME_TYPE;

/**�Ƕ��ϱ���ʽ***/
enum
{
	ANGLE_CHECK_MODE = 0x01,	//������ѯ�ϱ�ģʽ
	ANGLE_REPORT_MODE = 0x02	//���������ϱ�ģʽ
};

/**�߼�����Ϣ������ʹ�õĲ�����Ϣ����***/
typedef enum
{
	LOGIC_SET_DEVTYPE_INF = 0x00,	//�����豸������Ϣ
	LOGIC_SET_MODIFY_PARAM_NUMB,	//��������ֵ��������
	LOGIC_SET_MODIFY_PARAM,			//��������ֵ
	LOGIC_SET_CALIBRATE_FLAG,		//����������־
	
	LOGIC_SAVE_PARAM_MSG,		//�������

	LOGIC_GET_DEVTYPE_INF,		//�����豸������Ϣ
	LOGIC_GET_MODIFY_PARAM_NUMB,//��ȡ����ֵ��������	
	LOGIC_GET_MODIFY_PARAM,		//��ȡ����ֵ
	LOGIC_GET_MODIFY_PARAM_ALL,	//��ȡ����������������ֵ
	LOGIC_GET_CALIBRATE_FLAG,	//��ȡ������־

	LOGIC_PARAM_MSG_MAX//�߼��㴦����Ϣ�����ֵ
}LOGIC_PARAM_MSG_TYPE;

/**�߼�����Ϣ������ʹ�õ�������Ϣ����***/
typedef enum
{
	LOGIC_SET_SYSTEM_RUN_STATUS = 0x00,//����ϵͳ����״̬
	LOGIC_SET_WORK_MODE,		//���ù���ģʽ
	LOGIC_SET_REPORT_INTERVAL,	//���ýǶ��ϱ����
	LOGIC_SET_ANGLEVALUE_X,		//���ýǶ��ϱ���ֵ
	LOGIC_SET_ANGLEVALUE_Y,		//���ýǶ��ϱ���ֵ
	LOGIC_SET_ANGLEVALUE_Z,		//���ýǶ��ϱ���ֵ

	LOGIC_GET_SYSTEM_RUN_STATUS,//����ϵͳ����״̬	
	LOGIC_GET_WORK_MODE,		//��ȡ����ģʽ
	LOGIC_GET_REPORT_INTERVAL,	//��ȡ�Ƕ��ϱ����
	LOGIC_GET_ANGLEVALUE_X,		//��ȡ�Ƕ��ϱ���ֵ
	LOGIC_GET_ANGLEVALUE_Y,		//��ȡ�Ƕ��ϱ���ֵ
	LOGIC_GET_ANGLEVALUE_Z,		//��ȡ�Ƕ��ϱ���ֵ
	
	LOGIC_RUNINF_MSG_MAX		//�߼��㴦����Ϣ�����ֵ
}LOGIC_RUNINF_MSG_TYPE;

enum
{
	SYSTEM_RUN_STATUS_NORMAL = 0x00,	
	SYSTEM_RUN_STATUS_INIT = 0x01,
	SYSTEM_RUN_STATUS_ADJUST  = 0x02,
	SYSTEM_RUN_STATUS_UPPRG  = 0x03	
};

/**���ٶ�оƬö��***/
enum
{
	ACCR_CHIP_SCA100T = 0x01,
	ACCR_CHIP_ADXL203 = 0x02
};

/**�Ƕȴ�����ͨѶ�˿�ö��***/
enum
{
	ANGLE_COMM_CAN1 = 0x01,
	ANGLE_COMM_CAN2 = 0x02,	
	ANGLE_COMM_CAN1_2 = ANGLE_COMM_CAN1 | ANGLE_COMM_CAN2,			
};

/*�Ƕȴ��������ݲɼ����������ö��***/
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

/**���ò������ֵ����������****/
typedef struct
{
	u16 u16AngleParamNumbX;
	u16 u16AngleParamNumbY;//���������ֵ
}LOGIC_SET_PARAMNUMB_TYPE;

/**���ýǶȲ�������������****/
__packed typedef struct
{
	u8 u8RservedFlag;
	u8 u8AnleXYflag;
	s16 s16AngleValue;//�Ƕ�ֵ
	u16 u16AngleAdValue;//AD��������ֵ
	u16 u16StoragePosition;//�����洢��λ��	
}LOGIC_SET_ANGLEPARAM_TYPE;

/**��������ֵ��������**/
__packed typedef struct
{
	u32 u32ModifyX_ParamNumb;//X�Ƕ����ò����ĸ���
	s16 s16ModifyX_AngleValue[ANGLE_MODIFY_DEGREE_MAX];//X�����Ƕ�ֵ����0.01��Ϊ��λ����������)
	u16 u16ModifyX_AdValue[ANGLE_MODIFY_DEGREE_MAX];//X����ADֵ��X AD����������ֵ,0��У��)  �Զ�̬��ǣ��˴���ŵ��ǲ����Ƕ�ֵ����0.01��Ϊ��λ

	u32 u32ModifyY_ParamNumb;//Y�Ƕ����ò����ĸ���
	s16 s16ModifyY_AngleValue[ANGLE_MODIFY_DEGREE_MAX];//Y�����Ƕ�ֵ����0.01��Ϊ��λ����������)
	u16 u16ModifyY_AdValue[ANGLE_MODIFY_DEGREE_MAX];//Y����ADֵ��X AD����������ֵ,0��У��)  �Զ�̬��ǣ��˴���ŵ��ǲ����Ƕ�ֵ����0.01��Ϊ��λ
}LOGIC_MODIFYPARAM_TYPE;

/**�豸���ͽṹ��**/
__packed typedef struct
{
	u8 u8Resverd;//Ԥ��
	u8 u8AccrChipType;//�Ƕȴ�����оƬ����
	u8 u8DevCommType;//�豸ͨѶ�˿�����
	u8 u8DevSampleXyType;//�ɼ�X�ᡢY�ᶨ��
	u16 u16PoseXRange;//X�������Χ,��u16PoseXRange
	u16 u16PoseYRange;//Y�������Χ,��u16PoseXRange
}LOGIC_POSEDEV_INF_TYPE;
/********************************************************************************
* ��������
*********************************************************************************/
#define ANGLE_VALUE_INVALID					0x9999//��Ч�ĽǶ���ֵ

/********************************************************************************
* ȫ�ֱ�������
*********************************************************************************/

/********************************************************************************
* #define�궨��
*********************************************************************************/

#include "iapupdate.h"
#include "caniap.h"

/********************************************************************************
* ��������
*********************************************************************************/
u32 LogicInit(void);
u32 LogicParamApi(LOGIC_PARAM_MSG_TYPE sMsg, void *pData);
u32 LogicRunInfApi(LOGIC_RUNINF_MSG_TYPE sMsg, void *pData);

#endif
