/********************************************************************************
* �ļ����ƣ�	logic.c
* ��	�ߣ�	
* ��ǰ�汾��   	V1.0
* ������ڣ�    2014.12.08
* ��������: 	ʵ���豸��ʼ�������ݶ�д�Ȳ�����
* ��ʷ��Ϣ��   
*           	�汾��Ϣ     ���ʱ��      ԭ����        ע��
*
*       >>>>  �ڹ����е�λ��  <<<<
*          	  3-Ӧ�ò�
*           �� 2-Э���
*             1-Ӳ��������
*********************************************************************************
* Copyright (c) 2014,������������޹�˾ All rights reserved.
*********************************************************************************/
/********************************************************************************
* .hͷ�ļ�
*********************************************************************************/
#include <ucos_ii.h>
#include "logic.h"

#include "l4x_flash.h"

/********************************************************************************
* #define�궨��
*********************************************************************************/
/**��������**/
__packed typedef struct
{
	u32 u32YKQPrgSec;//����������

	LOGIC_POSEDEV_INF_TYPE sPoseDevInf;//�豸��Ϣ
	LOGIC_MODIFYPARAM_TYPE sModiftParam;//������Ϣ
	
	u16 u16CalibaratedFlag;//У׼��־λ��0xAA55ΪУ׼�ɹ���־
	u16 u16Crc;//CRCУ��ֵ
}LOGIC_PARAM_TYPE;

/**��������**/
__packed typedef struct
{
	u32 u32YKQPrgSec;//����������

	u32 u32PrgSystemRunStatus;//���ϵͳ����״̬
	
	u16 u16WorkMode;//������ʽ
	u16 u16ReportInterval;//�Ƕ��ϱ�������з���������0.01��Ϊ��λ)

	s16 u16AngleValueX;//x�Ƕ�ֵ���з���������0.01��Ϊ��λ)
	s16 u16AngleValueY;//y�Ƕ�ֵ���з���������0.01��Ϊ��λ)
	s16 u16AngleValueZ;//z�Ƕ�ֵ���з���������0.01��Ϊ��λ)
	
	u16 u16Resverd;//Ԥ��
	u16 u16Crc;//CRCУ��ֵ
}LOGIC_RUNINF_TYPE;

/********************************************************************************
* ��������
*********************************************************************************/
//����������Ĵ˲���������У��������ָ�ΪĬ��ֵ����Ҫ�ԽǶȴ������ٴ�У׼��
#define	ANGLE_PRG_SEC_NUMBER	0x16070413
#define ANGLE_CALIBRATED_FLAG	0xAA55//У׼��־


/**Ĭ�ϲ���ֵ**********/
static LOGIC_PARAM_TYPE	const s_stLogicParamDefaultDTQJ = 
{
	ANGLE_PRG_SEC_NUMBER,
	/**�豸���Ͷ���**/
	{
		0x00,
		ACCR_CHIP_SCA100T,
		ANGLE_COMM_CAN1_2,
		ANGLE_TYPE_X_Y,
		18000,
		9000
	},

	/**������������**/
	{
		0x03,
		{0,	9000,	-9000,	0x00},
		{0,	8900,	56636, 	0x00},
		
		0x03,
		{0,	9000,	-9000,	0x00},
		{0,	8900,	56636,	0x00},
	},
	
	0xAA55,
	0x00//CRCУ��ֵ���ڽ�����д��ʱ�������¼���CRCУ��ֵ
	
};

/**Ĭ�ϲ���ֵ**********/
static LOGIC_RUNINF_TYPE const s_stLogicRunInfDefault = 
{
	ANGLE_PRG_SEC_NUMBER,

	SYSTEM_RUN_STATUS_INIT,
	
	ANGLE_REPORT_MODE,
	500,

	0x0000,
	0x0000,
	
	0x00,
	0x00//CRCУ��ֵ���ڽ�����д��ʱ�������¼���CRCУ��ֵ
	
};
/********************************************************************************
* ��������
*********************************************************************************/
static LOGIC_PARAM_TYPE		s_stLogicParamSystem;		//ϵͳ���й�����ʹ�õ�Ĭ�ϲ���
static LOGIC_RUNINF_TYPE	s_stLogicRunInfSystem;		//ϵͳ���й�����ʹ�õ����в���

/********************************************************************************
* ��������
*********************************************************************************/
//�ӿں�������
/**������д��صĽӿں���**/
static u32 LogicSetDevTypeInfProc(void *pData);
static u32 LogicSetModifyParamNumbProc(void *pData);
static u32 LogicSetModifyParamProc(void *pData);
static u32 LogicSetCalibrateFlagProc(void *pData);
static u32 LogicSaveParamMsgProc(void *pData);

static u32 LogicGetDevTypeInfProc(void *pData);
static u32 LogicGetModifyParamNumbProc(void *pData);
static u32 LogicGetModifyParamProc(void *pData);
static u32 LogicGetModifyParamAllProc(void *pData);
static u32 LogicGetCalibrateFlagProc(void *pData);

//LOGIC API �ӿں���ָ������
u32 (*LogicParamApiProcFuncs[LOGIC_PARAM_MSG_MAX])(void *pData) =
{
	/**������д��صĽӿں���**/
	LogicSetDevTypeInfProc,	
	LogicSetModifyParamNumbProc,
	LogicSetModifyParamProc,
	LogicSetCalibrateFlagProc,
	LogicSaveParamMsgProc,

	LogicGetDevTypeInfProc,
	LogicGetModifyParamNumbProc,
	LogicGetModifyParamProc,
	LogicGetModifyParamAllProc,
	LogicGetCalibrateFlagProc
};

/**������Ϣ��д��صĽӿں���**/
static u32 LogicSetSystemStatusProc(void *pData);
static u32 LogicSetWorkModeProc(void *pData);
static u32 LogicSetReportIntervalProc(void *pData);
static u32 LogicSetAngleValueXProc(void *pData);
static u32 LogicSetAngleValueYProc(void *pData);
static u32 LogicSetAngleValueZProc(void *pData);

static u32 LogicGetSystemStatusProc(void *pData);
static u32 LogicGetWorkModeProc(void *pData);
static u32 LogicGetReportIntervalProc(void *pData);
static u32 LogicGetAngleValueXProc(void *pData);
static u32 LogicGetAngleValueYProc(void *pData);
static u32 LogicGetAngleValueZProc(void *pData);

//LOGIC API �ӿں���ָ������
u32 (*LogicRunInfApiProcFuncs[LOGIC_RUNINF_MSG_MAX])(void *pData) =
{
	/**���в�����д��صĽӿں���**/
	LogicSetSystemStatusProc,
	LogicSetWorkModeProc,
	LogicSetReportIntervalProc,
	LogicSetAngleValueXProc,
	LogicSetAngleValueYProc,
	LogicSetAngleValueZProc,

	LogicGetSystemStatusProc,
	LogicGetWorkModeProc,
	LogicGetReportIntervalProc,
	LogicGetAngleValueXProc,
	LogicGetAngleValueYProc,	
	LogicGetAngleValueZProc,	
};
/*******************************************************************************************
**�������ƣ�crc16_ccitt
**�������ã����������CRCУ��
**������������
**ע�������
*******************************************************************************************/
/* CRC16 implementation acording to CCITT standards */
static const u16 crc16tab[256] = {
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
	0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
	0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
	0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
	0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
	0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
	0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
	0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
	0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
	0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
	0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
	0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
	0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
	0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
	0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
	0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
	0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
	0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
	0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
	0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
	0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
	0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
	0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
	0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
	0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
	0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
	0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
	0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
	0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
	0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
	0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
	0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};

void crc16_ccitt(const u8 *buf, u32 len, u16 *check_old)
{
	u32 counter;
	u16 crc = *check_old;
	for( counter = 0; counter < len; counter++)
		crc = (crc << 8) ^ crc16tab[((crc >> 8) ^ *(u8 *)buf++)&0x00FF];
	*check_old = crc;
}

/*******************************************************************************************
**�������ƣ�LogicParamSaveProc
**�������ã�ϵͳ��������
**������������
**�����������
**ע�������
*******************************************************************************************/
void LogicParamSaveProc(void)
{
	u16 u16Temp = 0x00;
	u8 *p = (u8 *)&(s_stLogicParamSystem.u32YKQPrgSec);
	
	uint8_t tmp_buf[4];
	uint8_t tmp_over = 0;
	uint16_t count = 0;
	uint16_t i;
	u32 u32WriteAddr = 0;
	
	crc16_ccitt((u8 *)&s_stLogicParamSystem, sizeof(LOGIC_PARAM_TYPE) - 0x02, &u16Temp);
	s_stLogicParamSystem.u16Crc = u16Temp;
	//��Ϣ�洢
	FLASH_SetLatency(THE_DEV_FLASH_Latency);//120 MHz< HCLK <= 168 MHz
	FLASH_Unlock();
	__disable_fault_irq();	
	FLASH_ErasePage(PARAM_1_START_ADDR);
	p = (u8 *)&(s_stLogicParamSystem.u32YKQPrgSec);

	count = (sizeof(LOGIC_PARAM_TYPE)) / 4;
	tmp_over = (sizeof(LOGIC_PARAM_TYPE)) % 4;
	for(i = 0; i < count; i ++)
	{     
		#if STM32 
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, PARAM_1_START_ADDR + u32WriteAddr, *(uint64_t *)p) == HAL_OK)
		#endif
		if (fmc_word_program(PARAM_1_START_ADDR + u32WriteAddr, *(uint32_t *)p) == FMC_READY)
		{
			u32WriteAddr += 4;
			p += 4;
		}
	}
	if(tmp_over)
	{
		memset(tmp_buf, 0xff, 4);
		memcpy(tmp_buf, p, tmp_over);
		#if STM32 
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, PARAM_1_START_ADDR + u32WriteAddr, *(uint64_t *)tmp_buf) == HAL_OK)
		#endif
		if (fmc_word_program(PARAM_1_START_ADDR + u32WriteAddr, *(uint32_t *)tmp_buf) == FMC_READY)
		{    
			;
		}
	}
	
	FLASH_Lock();
	__enable_fault_irq();

}

/*******************************************************************************************
**�������ƣ�LogicRunInfSaveProc
**�������ã������������
**������������
**�����������
**ע�������
*******************************************************************************************/
void LogicRunInfSaveProc(void)
{
	u16 u16Temp = 0x00;
	u8 *p = (u8 *)&(s_stLogicRunInfSystem.u32YKQPrgSec);
	
	uint8_t tmp_buf[4];
	uint8_t tmp_over = 0;
	uint16_t count = 0;
	uint16_t i;
	u32 u32WriteAddr = 0;
	
	crc16_ccitt((u8 *)&s_stLogicRunInfSystem, sizeof(LOGIC_RUNINF_TYPE) - 0x02, &u16Temp);
	s_stLogicRunInfSystem.u16Crc = u16Temp;
	//��Ϣ�洢
	FLASH_SetLatency(THE_DEV_FLASH_Latency);//120 MHz< HCLK <= 168 MHz
	FLASH_Unlock();
	__disable_fault_irq();	
	FLASH_ErasePage(PARAM_2_START_ADDR);
	p = (u8 *)&(s_stLogicRunInfSystem.u32YKQPrgSec);

	count = (sizeof(LOGIC_RUNINF_TYPE)) / 4;			//4 byte once
	tmp_over = (sizeof(LOGIC_RUNINF_TYPE)) % 4;			//����4byte
	for(i = 0; i < count; i ++)
	{     
		#if STM32 
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, PARAM_2_START_ADDR + u32WriteAddr, *(uint64_t *)p) == HAL_OK)
		#endif
		if (fmc_word_program(PARAM_2_START_ADDR + u32WriteAddr, *(uint32_t *)p) == FMC_READY)
		{    
			u32WriteAddr += 4;
			p += 4;
		}
	}
	if(tmp_over)
	{
		memset(tmp_buf, 0xff, 4);
		memcpy(tmp_buf, p, tmp_over);
		#if STM32 
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, PARAM_2_START_ADDR + u32WriteAddr, *(uint64_t *)tmp_buf) == HAL_OK)
		#endif
		if (fmc_word_program(PARAM_2_START_ADDR + u32WriteAddr, *(uint32_t *)tmp_buf) == FMC_READY)
		{    
			;
		}
	}

	FLASH_Lock();
	__enable_fault_irq();
	
}

/*******************************************************************************
* Function Name  : GetLockCode
* Description    : ���ü�����
* Input          : None
* Output         : None
* Return         : None
* Attention		 : һ���ڳ���ʼ��ʱ�����,��Ҫ�Ѽ���ģʽ���ӻ�������Ľ��˺���
*******************************************************************************/
void GetLockCode(void)
{
	u32 u32Lock_Code;
	u8 u32CpuID[0x03];
	
	/* ��ȡCPUΨһID */
	u32CpuID[0] = *(vu32*)(0x1ffff7e8);
	u32CpuID[1] = *(vu32*)(0x1ffff7ec);
	u32CpuID[2] = *(vu32*)(0x1ffff7f0);
	/* �����㷨,�ܼ򵥵ļ����㷨 */
	/* ��γ���Ƚ����ױ�����࣬�����Ҫ�����ӵļ��ܷ�ʽ��Ӧ�þ������Ⱪ¶IDλ�úͼ��ܹ�ʽ */
	u32Lock_Code = (u32CpuID[0] >> 1) + (u32CpuID[1] >> 2) + (u32CpuID[2] >> 3);

	srand(u32Lock_Code);

	OSTimeDly(u32Lock_Code & 0xFF);
}

/*******************************************************************************************
**�������ƣ�LogicInit
**�������ã�������ʼ��
**������������
**�����������
**ע�������
*******************************************************************************************/
u32 LogicInit(void)
{
	u16 u16Temp = 0x00;
	u32 *p = (u32 *)&(s_stLogicParamSystem.u32YKQPrgSec);
	u32 u32ReturnValue = 0x00;

	/**��ȡ�߼�����**/
	for(u16Temp = 0x00; u16Temp < sizeof(LOGIC_PARAM_TYPE); u16Temp += 4)
	{
		*p = *(u32 *)(PARAM_1_START_ADDR + u16Temp);
		p++;
	}
	
	u16Temp = 0x00;
	crc16_ccitt((u8 *)&s_stLogicParamSystem, sizeof(LOGIC_PARAM_TYPE) - 0x02, &u16Temp);

	if((s_stLogicParamSystem.u32YKQPrgSec == ANGLE_PRG_SEC_NUMBER) && (u16Temp == s_stLogicParamSystem.u16Crc))
	{
		u32ReturnValue = 0x01;
	}	
	else//��Ҫ�ָ�Ĭ�ϲ���������Ĭ�ϲ������д洢��
	{
		memmove(&s_stLogicParamSystem, &s_stLogicParamDefaultDTQJ, sizeof(LOGIC_PARAM_TYPE));
		LogicParamSaveProc();
		u32ReturnValue = 0x00;
	}

	/**��ȡ������Ϣ**/
	p = (u32 *)&(s_stLogicRunInfSystem.u32YKQPrgSec);
	for(u16Temp = 0x00; u16Temp < sizeof(LOGIC_PARAM_TYPE); u16Temp += 4)
	{
		*p = *(u32 *)(PARAM_2_START_ADDR + u16Temp);
		p++;
	}
	
	u16Temp = 0x00;
	crc16_ccitt((u8 *)&s_stLogicRunInfSystem, sizeof(LOGIC_RUNINF_TYPE) - 0x02, &u16Temp);
	if((s_stLogicRunInfSystem.u32YKQPrgSec == ANGLE_PRG_SEC_NUMBER) && (u16Temp == s_stLogicRunInfSystem.u16Crc))
	{
		u32ReturnValue = 0x01;
	}
	else//��Ҫ�ָ�Ĭ�ϲ���������Ĭ�ϲ������д洢��
	{
		memmove(&s_stLogicRunInfSystem, &s_stLogicRunInfDefault, sizeof(LOGIC_PARAM_TYPE));
		LogicRunInfSaveProc();
		u32ReturnValue = 0x00;
	}

	return u32ReturnValue;
}

/*******************************************************************************************
**�������ƣ�LogicSetDevTypeInfProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicSetDevTypeInfProc(void *pData)
{
	if(((((LOGIC_POSEDEV_INF_TYPE *)pData)->u8AccrChipType) < ACCR_CHIP_SCA100T) || ((((LOGIC_POSEDEV_INF_TYPE *)pData)->u8AccrChipType) > ACCR_CHIP_ADXL203))
		return 0x00;	
	if(((((LOGIC_POSEDEV_INF_TYPE *)pData)->u8DevCommType) < ANGLE_COMM_CAN1) || ((((LOGIC_POSEDEV_INF_TYPE *)pData)->u8DevCommType) > ANGLE_COMM_CAN1_2))
		return 0x00;
	if(((((LOGIC_POSEDEV_INF_TYPE *)pData)->u8DevSampleXyType) < ANGLE_TYPE_X) || ((((LOGIC_POSEDEV_INF_TYPE *)pData)->u8DevCommType) > ANGLE_TYPE_X_Y))
		return 0x00;	

	s_stLogicParamSystem.sPoseDevInf.u8Resverd = 0x00;
	s_stLogicParamSystem.sPoseDevInf.u8AccrChipType = ((LOGIC_POSEDEV_INF_TYPE *)pData)->u8AccrChipType;
	s_stLogicParamSystem.sPoseDevInf.u8DevCommType = ((LOGIC_POSEDEV_INF_TYPE *)pData)->u8DevCommType;
	s_stLogicParamSystem.sPoseDevInf.u8DevSampleXyType = ((LOGIC_POSEDEV_INF_TYPE *)pData)->u8DevSampleXyType;
	s_stLogicParamSystem.sPoseDevInf.u16PoseXRange =  ((LOGIC_POSEDEV_INF_TYPE *)pData)->u16PoseXRange;
	s_stLogicParamSystem.sPoseDevInf.u16PoseYRange =  ((LOGIC_POSEDEV_INF_TYPE *)pData)->u16PoseYRange;	

	return 0x01;	
}

/*******************************************************************************************
**�������ƣ�LogicSetModifyParamNumbProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicSetModifyParamNumbProc(void *pData)
{
	if( ((LOGIC_SET_PARAMNUMB_TYPE *)pData)->u16AngleParamNumbX == 0x00)
		return 0x00;
	if( ((LOGIC_SET_PARAMNUMB_TYPE *)pData)->u16AngleParamNumbY == 0x00)
		return 0x00;
	if( ((LOGIC_SET_PARAMNUMB_TYPE *)pData)->u16AngleParamNumbX > ANGLE_MODIFY_DEGREE_MAX)
		return 0x00;
	if(((LOGIC_SET_PARAMNUMB_TYPE *)pData)->u16AngleParamNumbY > ANGLE_MODIFY_DEGREE_MAX)
		return 0x00;
	
	s_stLogicParamSystem.sModiftParam.u32ModifyX_ParamNumb = (u32)(((LOGIC_SET_PARAMNUMB_TYPE *)pData)->u16AngleParamNumbX);

	s_stLogicParamSystem.sModiftParam.u32ModifyY_ParamNumb = (u32)(((LOGIC_SET_PARAMNUMB_TYPE *)pData)->u16AngleParamNumbY);

	return 0x01;
}
/*******************************************************************************************
**�������ƣ�LogicSetModifyParamProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicSetModifyParamProc(void *pData)
{
	if((((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u8AnleXYflag != ANGLE_TYPE_X) && (((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u8AnleXYflag != ANGLE_TYPE_Y))
		return 0x00;
	if(((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u16StoragePosition >= ANGLE_MODIFY_DEGREE_MAX)
		return 0x00;

	if(((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u8AnleXYflag == ANGLE_TYPE_X)
	{
		if(((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u16StoragePosition >= s_stLogicParamSystem.sModiftParam.u32ModifyX_ParamNumb)
			return 0x00;
		s_stLogicParamSystem.sModiftParam.s16ModifyX_AngleValue[((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u16StoragePosition] = ((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->s16AngleValue;
		s_stLogicParamSystem.sModiftParam.u16ModifyX_AdValue[((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u16StoragePosition] = ((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u16AngleAdValue;		
	}
	else
	{
		if(((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u16StoragePosition >= s_stLogicParamSystem.sModiftParam.u32ModifyY_ParamNumb)
			return 0x00;
		s_stLogicParamSystem.sModiftParam.s16ModifyY_AngleValue[((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u16StoragePosition] = ((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->s16AngleValue;
		s_stLogicParamSystem.sModiftParam.u16ModifyY_AdValue[((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u16StoragePosition] = ((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u16AngleAdValue;	
	}
	
	return 0x01;
}

/*******************************************************************************************
**�������ƣ�LogicParamApiProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicSetCalibrateFlagProc(void *pData)
{
	s_stLogicParamSystem.u16CalibaratedFlag = *(u16 *)pData;

	return 0x01;
}

/*******************************************************************************************
**�������ƣ�LogicParamApiProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicSaveParamMsgProc(void *pData)
{
	LogicParamSaveProc();

	return 0x01;
}

/*******************************************************************************************
**�������ƣ�LogicParamApiProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicGetDevTypeInfProc(void *pData)
{
	((LOGIC_POSEDEV_INF_TYPE *)pData)->u8Resverd = 0x00;
	((LOGIC_POSEDEV_INF_TYPE *)pData)->u8AccrChipType = s_stLogicParamSystem.sPoseDevInf.u8AccrChipType;
	((LOGIC_POSEDEV_INF_TYPE *)pData)->u8DevCommType = s_stLogicParamSystem.sPoseDevInf.u8DevCommType;
	((LOGIC_POSEDEV_INF_TYPE *)pData)->u8DevSampleXyType = s_stLogicParamSystem.sPoseDevInf.u8DevSampleXyType;
	((LOGIC_POSEDEV_INF_TYPE *)pData)->u16PoseXRange = s_stLogicParamSystem.sPoseDevInf.u16PoseXRange;
	((LOGIC_POSEDEV_INF_TYPE *)pData)->u16PoseYRange = s_stLogicParamSystem.sPoseDevInf.u16PoseYRange;	

	return 0x01;	
}

/*******************************************************************************************
**�������ƣ�LogicParamApiProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicGetModifyParamNumbProc(void *pData)
{
		
	((LOGIC_SET_PARAMNUMB_TYPE *)pData)->u16AngleParamNumbX = s_stLogicParamSystem.sModiftParam.u32ModifyX_ParamNumb;

	((LOGIC_SET_PARAMNUMB_TYPE *)pData)->u16AngleParamNumbY = s_stLogicParamSystem.sModiftParam.u32ModifyY_ParamNumb;

	return 0x01;

}

/*******************************************************************************************
**�������ƣ�LogicGetModifyParamProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicGetModifyParamProc(void *pData)
{
	if((((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u8AnleXYflag != ANGLE_TYPE_X) && (((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u8AnleXYflag != ANGLE_TYPE_Y))
		return 0x00;
	if(((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u16StoragePosition >= ANGLE_MODIFY_DEGREE_MAX)
		return 0x00;

	if(((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u8AnleXYflag == ANGLE_TYPE_X)
	{
		if(((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u16StoragePosition >= s_stLogicParamSystem.sModiftParam.u32ModifyX_ParamNumb)
			return 0x00;
		((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->s16AngleValue = s_stLogicParamSystem.sModiftParam.s16ModifyX_AngleValue[((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u16StoragePosition];
		((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u16AngleAdValue = s_stLogicParamSystem.sModiftParam.u16ModifyX_AdValue[((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u16StoragePosition];
	}
	else
	{
		if(((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u16StoragePosition >= s_stLogicParamSystem.sModiftParam.u32ModifyY_ParamNumb)
			return 0x00;
		((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->s16AngleValue = s_stLogicParamSystem.sModiftParam.s16ModifyY_AngleValue[((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u16StoragePosition];
		((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u16AngleAdValue = s_stLogicParamSystem.sModiftParam.u16ModifyY_AdValue[((LOGIC_SET_ANGLEPARAM_TYPE *)pData)->u16StoragePosition];
	}
	
	return 0x01;

}

/*******************************************************************************************
**�������ƣ�LogicGetModifyParamAllProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicGetModifyParamAllProc(void *pData)
{
	memmove((LOGIC_MODIFYPARAM_TYPE *)pData, (const void *)&(s_stLogicParamSystem.sModiftParam.u32ModifyX_ParamNumb), sizeof(LOGIC_MODIFYPARAM_TYPE));
	
	return 0x01;
}
/*******************************************************************************************
**�������ƣ�LogicGetCalibrateFlagProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicGetCalibrateFlagProc(void *pData)
{
	*(u16 *)pData = s_stLogicParamSystem.u16CalibaratedFlag;

	return 0x01;
}

/**���в����ӿں�������***/
/*******************************************************************************************
**�������ƣ�LogicParamApiProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicSetSystemStatusProc(void *pData)
{
	s_stLogicRunInfSystem.u32PrgSystemRunStatus = *(u32 *)pData;

	if(*(u32 *)pData == SYSTEM_RUN_STATUS_ADJUST)
	{
//		CanLeftSetReportMode(ANGLE_CHECK_MODE);	
		/*************************��can
		CanRightSetReportMode(ANGLE_CHECK_MODE);
		*****************************/
	}
	return 0x01;
}

/*******************************************************************************************
**�������ƣ�LogicSetWorkModeProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicSetWorkModeProc(void *pData)
{
	s_stLogicRunInfSystem.u16WorkMode = *(u16*)pData;
	LogicRunInfSaveProc();
//	CanLeftSetReportMode(*(u16*)pData);
	/*************************��can
	CanRightSetReportMode(*(u16*)pData);
	*****************************/
	return 0x01;
}

/*******************************************************************************************
**�������ƣ�LogicSetReportIntervalProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicSetReportIntervalProc(void *pData)
{
	s_stLogicRunInfSystem.u16ReportInterval = *(u16 *)pData;
	LogicRunInfSaveProc();
//	CanLeftSetSendInterval(*(u16*)pData );
	/*************************��can
	CanRightSetSendInterval(*(u16*)pData );	
	*****************************/
	return 0x01;
}

/*******************************************************************************************
**�������ƣ�LogicSetAngleValueXProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicSetAngleValueXProc(void *pData)
{
	s_stLogicRunInfSystem.u16AngleValueX = *(u16 *)pData;

	return 0x01;
}

/*******************************************************************************************
**�������ƣ�LogicSetAngleValueYProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicSetAngleValueYProc(void *pData)
{
	s_stLogicRunInfSystem.u16AngleValueY = *(u16 *)pData;

	return 0x01;
}

/*******************************************************************************************
**�������ƣ�LogicSetAngleValueZProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicSetAngleValueZProc(void *pData)
{
	s_stLogicRunInfSystem.u16AngleValueZ = *(u16 *)pData;

	return 0x01;
}

/*******************************************************************************************
**�������ƣ�LogicGetSystemStatusProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicGetSystemStatusProc(void *pData)
{
	*(u32 *)pData = s_stLogicRunInfSystem.u32PrgSystemRunStatus;

	return 0x01;
}

/*******************************************************************************************
**�������ƣ�LogicGetWorkModeProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicGetWorkModeProc(void *pData)
{
	*(u16*)pData = s_stLogicRunInfSystem.u16WorkMode;

	return 0x01;
}

/*******************************************************************************************
**�������ƣ�LogicGetReportIntervalProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicGetReportIntervalProc(void *pData)
{
	*(u16 *)pData = s_stLogicRunInfSystem.u16ReportInterval;

	return 0x01;
}

/*******************************************************************************************
**�������ƣ�LogicGetAngleValueXProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicGetAngleValueXProc(void *pData)
{
	*(u16 *)pData = s_stLogicRunInfSystem.u16AngleValueX;

	return 0x01;
}

/*******************************************************************************************
**�������ƣ�LogicGetAngleValueYProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicGetAngleValueYProc(void *pData)
{
	*(u16 *)pData = s_stLogicRunInfSystem.u16AngleValueY;

	return 0x01;
}

/*******************************************************************************************
**�������ƣ�LogicGetAngleValueZProc
**�������ã��߼�����Ϣ�����б�
**������������
**�����������
**ע�������
*******************************************************************************************/
static u32 LogicGetAngleValueZProc(void *pData)
{
	*(u16 *)pData = s_stLogicRunInfSystem.u16AngleValueZ;

	return 0x01;
}

/*******************************************************************************************
**�������ƣ�LogicParamApi
**�������ã�LOGIC��Ϣ����
**������������
**�����������
**ע�������
*******************************************************************************************/
u32 LogicParamApi(LOGIC_PARAM_MSG_TYPE sMsg, void *pData)
{
	u32 u32ReturnFlag = 0x00;
	
	#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
			OS_CPU_SR  cpu_sr = 0;
	#endif
	
	OS_ENTER_CRITICAL(); 
	
	if(LogicParamApiProcFuncs[sMsg])
	{
		u32ReturnFlag = (*LogicParamApiProcFuncs[sMsg])(pData);
	}
	else
	{
		u32ReturnFlag = 0x00;
	}
	
	OS_EXIT_CRITICAL();	
	
	return u32ReturnFlag;
}

/*******************************************************************************************
**�������ƣ�LogicRunInfApi
**�������ã�LOGIC��Ϣ����
**������������
**�����������
**ע�������
*******************************************************************************************/
u32 LogicRunInfApi(LOGIC_RUNINF_MSG_TYPE sMsg, void *pData)
{
	u32 u32ReturnFlag = 0x00;
	
	#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
		OS_CPU_SR  cpu_sr = 0;
	#endif
	
	OS_ENTER_CRITICAL(); 
	
	if(LogicRunInfApiProcFuncs[sMsg])
	{
		u32ReturnFlag = (*LogicRunInfApiProcFuncs[sMsg])(pData);
	}
	else
	{
		u32ReturnFlag = 0x00;
	}
	
	OS_EXIT_CRITICAL();	
	
	return u32ReturnFlag;
}

