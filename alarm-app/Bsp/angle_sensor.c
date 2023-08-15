/********************************************************************************
* �ļ����ƣ�	angle_sensor.c
* ��	�ߣ�	������   
* ��ǰ�汾��   	V1.0
* ������ڣ�    2014.12.08
* ��������: 	���AD �������߼���Ĵ���ʵ���豸��ʼ�������ݶ�д�Ȳ�����
* ��ʷ��Ϣ��   
*           	�汾��Ϣ     ���ʱ��      ԭ����        ע��
*
*       >>>>  �ڹ����е�λ��  <<<<
*          	  3-Ӧ�ò�
*             2-Э���
*          ��  1-Ӳ��������
*********************************************************************************
* Copyright (c) 2014,������������޹�˾ All rights reserved.
*********************************************************************************/
/********************************************************************************
* .hͷ�ļ�
*********************************************************************************/
#include "app_cfg.h"
#include "angle_sensor.h"
#include "logic.h"
#include "mpu6500_driver.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "inv_mpu.h"
#include "math_fun.h"
#include "dmpKey.h"
#include "dmpmap.h"

#include "l4can.h"
#include "can_app.h"
#include "main.h"
/********************************************************************************
* #define�궨��
*********************************************************************************/
#define ANGLE_SENSOR_CALCULATE_TIMES	10//����ƽ��ֵ�Ĵ���
#define	ANGLE_SENOR_VALUE_INVALID		0x9999//��Ч�ĽǶ���ֵ

extern WL_WORK_STATUS eCanWLReportMode;     //����ģ��Ĺ���ģʽ


//typedef struct
//{
//	u16 u16SensorValueSample[ANGLE_SENSOR_CALCULATE_TIMES];	//�������ֵ��������������
//	u16 u16SensorWritePtr;	//����ֵҪ�����λ��
//	s16 s16SensorValue;
//}ANGLE_SENSOR_SAMPLE_TYPE;

typedef struct
{
	s16 s16SensorValueSample[ANGLE_SENSOR_CALCULATE_TIMES];	//�������ֵ��������������	MPU6500��ԭʼ����ֵ�Ǹ�����	parry 2021.7.5
	u16 u16SensorWritePtr;	//����ֵҪ�����λ��
	s16 s16SensorValue;
}ANGLE_SENSOR_SAMPLE_TYPE_MPU6500;

/********************************************************************************
* ��������
*********************************************************************************/

/********************************************************************************
* ��������
*********************************************************************************/
static ANGLE_SENSOR_SAMPLE_TYPE_MPU6500 s_stAngleSensorValueMpu6500X;
static ANGLE_SENSOR_SAMPLE_TYPE_MPU6500 s_stAngleSensorValueMpu6500Y;
static ANGLE_SENSOR_SAMPLE_TYPE_MPU6500 s_stAngleSensorValueMpu6500Z;

static LOGIC_POSEDEV_INF_TYPE s_stAngleDevTypeInf;
static LOGIC_MODIFYPARAM_TYPE s_stAngleModiftParam;

SENSOR_TYPE	s_stSensorType;

static s16 s_s16SensorValueAvgX = 0x00;	//�������²�����X��ǶȾ�ֵ����λ��0.01�㣬У׼��	parry 2021.7.5
static s16 s_s16SensorValueAvgY = 0x00;	//�������²�����Y��ǶȾ�ֵ����λ��0.01�㣬У׼��
//static u16 s_u16SensorValueModifyZ = 0x00; //��Z�����ݽ��в�����������Ư������Ӱ��
//static s16 s_s16SensorValuePowerOnZ = 0x00; //��¼���ϵ�ʱZ�����ݣ�ÿ���ϵ�Z��Ƕ�����

static u8  s_u8SensorStart = 0x00;  	//MPU6500�����ж��ź�ʱ��ʼ��ʱ
static u8  s_u8SensorStopTimes = 0x00;  //MPU6500�쳣����

//static s16 s_s16SensorValueAvgModifyX = 0x00;	//У׼ʱ��X��Ƕ�ֵ	parry 2021.7.5
//static s16 s_s16SensorValueAvgModifyY = 0x00;	//У׼ʱ��Y��Ƕ�ֵ
u16 u16CanLeftSendInterval = 0x00;
static u8 u8LeftActiveReportAngleChange = 0x00;

u32 u32CanLeftPrgRecvTimer = 0x00;//���ճ����ʱ��

u16 u16CanLeftLiuShuiNumb = 0x00;
u16 u16CanLeftSendTimer = 0x00;

u16 u16CanLeftSystemRunStatus = SYSTEM_RUN_STATUS_INIT;
//static LOGIC_POSEDEV_INF_TYPE s_stCanLeftDevTypeInf;

static u16 u16CanLeftSendHeartTimer = 0;

static u16 u16AlarmSendHeartTimer = 0;

static u16 u16WLMSendHeartTimer = 0;

u16 u16LastLeftAngleValue_X = 0;
u16 u16LastLeftAngleValue_Y = 0;

u8  u8LeftAngleFlag = 0;
u8  u8LeftAngleStateFlag = 0;
u32 u32LeftReportRxDevId = 0;
/********************************************************************************
* ��������
*********************************************************************************/

/*******************************************************************************************
**�������ƣ�AngleSensorInit
**�������ã��ǶȲɼ������ʼ��
**������������
**�����������
**ע�������
*******************************************************************************************/
u8 AngleSensorInit(void) 
{
//	INPUT_VALUE_TYPE InputValue;
//	TIM_TimeBaseInitTypeDef		TIM_Time2Structure;		 //���嶨ʱ���ṹ��
//	NVIC_InitTypeDef			NVIC_InitStructure; 
	
//	memset(&s_stAngleSensorValueX,0x00,sizeof(ANGLE_SENSOR_SAMPLE_TYPE) );
//	memset(&s_stAngleSensorValueY,0x00,sizeof(ANGLE_SENSOR_SAMPLE_TYPE) );
	
	memset(&s_stAngleSensorValueMpu6500X, 0x00, sizeof(ANGLE_SENSOR_SAMPLE_TYPE_MPU6500));
	memset(&s_stAngleSensorValueMpu6500Y, 0x00, sizeof(ANGLE_SENSOR_SAMPLE_TYPE_MPU6500));
	memset(&s_stAngleSensorValueMpu6500Z, 0x00, sizeof(ANGLE_SENSOR_SAMPLE_TYPE_MPU6500));
	  
	/* ��ȡ������������Ϣ */
//	InputGetValue(INPUT_1, &InputValue);	
//	if(InputValue == INPUT_LOW)		//����Ϊ��ʱ����ʾ��Ǵ������ͺ�ΪGUD90(C)�����õ���6�ᴫ��������GUD90(C)��INPUT_1�ӵأ����ص͵�ƽ��	parry 2021.7.5
	{
		s_stSensorType = SENSOR_AXIS_6;
	}
	
	LogicParamApi(LOGIC_GET_DEVTYPE_INF, &s_stAngleDevTypeInf);

	LogicParamApi(LOGIC_GET_MODIFY_PARAM_ALL, &s_stAngleModiftParam);
	
//	if(u16CanLeftReportMode == ANGLE_REPORT_MODE)
//	{
		LogicRunInfApi(LOGIC_GET_REPORT_INTERVAL, &u16CanLeftSendInterval);
		if(u16CanLeftSendInterval < CANLEFT_REPORT_TIME_MIN)
			u16CanLeftSendInterval = CANLEFT_REPORT_TIME_MIN;
//	}
		
	u16CanLeftLiuShuiNumb = 0x00;
	u16CanLeftSendTimer = 0x00;	
	u32CanLeftPrgRecvTimer = 0x00;

	u8LeftActiveReportAngleChange = 5;
	
#if 0
	TIM_TimeBaseStructInit(&TIM_Time2Structure);		//��ʼ����ʱ��2�ṹ��
	TIM_DeInit(TIM2);									//����TIM2Ĭ��ģʽ

	/* TIM2 Periph clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	TIM_Time2Structure.TIM_Period = 14999;									// ���ó�ʼֵ,��ʱ7.5ms�ж�
	TIM_Time2Structure.TIM_Prescaler = 41;									// ���ö�ʱ��2�ķ�Ƶֵ��ʱ��Ϊ2MHz
	TIM_Time2Structure.TIM_ClockDivision=TIM_CKD_DIV1;						// ����ʱ�ӷָ�
	TIM_Time2Structure.TIM_CounterMode=TIM_CounterMode_Up;					// TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_Time2Structure);							// ���ṹ���ֵ����ʼ����ʱ��2
	TIM_InternalClockConfig(TIM2);											// ����TIM2��ʱ��Ϊ�ڲ�ʱ��
	TIM2->CNT = 0x0000;														// ��ʱ��2����������
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);								// ʹ��TIM2����ж�Դ
	TIM_ARRPreloadConfig(TIM2, ENABLE);										// ʹ���Զ���װ
	TIM_Cmd(TIM2, DISABLE);													// ��ֹTIM2

    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;							// ʹ�ܻ���ʧ��ָ����IRQͨ�� TIM2ȫ���ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0f;			// ��ʹ���ж����ȼ�Ƕ�ס���ΪSysTick���ж����ȼ�Ϊ0x0f
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;	//1;				// ���ó�Ա NVIC_IRQChannel�еĴ����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;							// ʹ�� NVIC_IRQChannel
	NVIC_Init(&NVIC_InitStructure);    

	TIM_Cmd(TIM2, ENABLE);													// ʹ��TIM2
#endif
	return 0x01;
}
#if 0
//��ʱ��2����жϺ���
void TIM2_IRQHandler(void)
{	  
	OS_CPU_SR  cpu_sr;
	static u16 times;
//	static u16 times1;
	OS_ENTER_CRITICAL();                         /* Tell uC/OS-II that we are starting an ISR          */
	OSIntEnter();
	OS_EXIT_CRITICAL();

	if(TIM_GetITStatus(TIM2,TIM_IT_Update) == SET)							// �ж��Ƿ�����ʱ������ж�
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);							// ��ʱ������жϴ�����������
		TIM_ClearFlag(TIM2, TIM_FLAG_Update);								// ���Ͼ书����ͬ
		
		times ++;
		s_u8SensorStopTimes ++;		//40ms
		if((s_u8SensorStart > 0) && (s_u8SensorStopTimes > 50))	//MPU6500��ʼ����������2s���������Ƕȣ�����
			//ִ����ת
			IapJumpToBoot(IN_FLASH_BOOTLOADER_ADDR);
		
//		if(times > 3299)	//3299��Ӧ130s��ÿ130s����-0.5�㣬����ʹ6̨��������13.5h��Z��Ư���ڡ�8.6�㣬Ҳ���ǡ�0.64��/h
//		if(times > 2828)	//�ڶ���ϵͳ����ʱ��3299�ᵼ��13Сʱ����Ư30�㣬��Ϊ2828�󣬾�������1/7��3299����13Сʱ����180�㣬ʵ����Ҫ����210�㣬2828�����210��
//		{
//			times = 0;
//			s_u16SensorValueModifyZ++;
//			s_u16SensorValueModifyZ %= 720;  //����360�����û��
//		}
//		if((times > 300) && (s_s16SensorValuePowerOnZ == 0))	 //300��Ӧ12s��ʱ���ϵ�12s��Z�������ȶ�����	
//		{
//			s_s16SensorValuePowerOnZ = s_stAngleSensorValueMpu6500Z.s16SensorValue;
//		}
	}

	OSIntExit();
}
#endif
//MPU6500�����ж�
void MPU6500_IRQHandler(void)
{
	/* USER CODE BEGIN EXTI_IRQn */
	OSIntEnter();
	/* USER CODE END EXTI_IRQn */
	/* EXTI line interrupt detected */
	#if STM32 
	if(__HAL_GPIO_EXTI_GET_IT(MPU6500_INT_PIN) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(MPU6500_INT_PIN);
	}
	#endif
	if (RESET != exti_interrupt_flag_get(MPU6500_EXTI_LINE))
	{
		MPU_EXTI_flag = 0xff;
    }
	exti_interrupt_flag_clear(MPU6500_EXTI_LINE);

	/* USER CODE BEGIN EXTI_IRQn 1 */
	OSIntExit();
	/* USER CODE END EXTI_IRQn 1 */
}

/******************************************************************************************
* �������ƣ�FilterAndSum 
* ��������������ȥ��һ�����ֵ����Сֵ�����������ݵĺ�
* ��ڲ�����adc�����ݣ�length�����ݸ���
* ���ڲ�������
* ʹ��˵������
*******************************************************************************************/	
u32 FilterAndSum(u16 *adc, u8 length)
{
    u32 sum = 0;
	u32 max = *adc;
	u32 min = *adc;
	u8 i = 0;
	
	for (i = 0; i < length; i++)
	{
	    if (*(adc + i) > max)   
		     max = *(adc + i);
		if (*(adc + i) < min)   
		     min = *(adc + i);
	    sum += *(adc + i);
	}
	
	sum = (sum - max - min);// (sum- max - min)/(length-2)
	
	return sum;
}

/******************************************************************************************
* �������ƣ�FilterAndSum 
* ��������������ȥ��һ�����ֵ����Сֵ�����������ݵĺ�
* ��ڲ�����adc�����ݣ�length�����ݸ���
* ���ڲ�������
* ʹ��˵������
*******************************************************************************************/	
s32 FilterAndSumMpu6500(s16 *adc, u8 length)
{
    s32 sum = 0;
	s32 max = *adc;
	s32 min = *adc;
	u8 i = 0;
	
	for (i = 0; i < length; i++)
	{
	    if (*(adc + i) > max)   
		     max = *(adc + i);
		if (*(adc + i) < min)   
		     min = *(adc + i);
	    sum += *(adc + i);
	}
	sum = (sum - max - min);		// (sum- max - min)/(length-2)

	return sum;
}

/*******************************************************************************************
**�������ƣ�CalibrateXYFunction
**�������ã�У׼X��Y�Ƕ�ֵ
**������������
**�����������
**ע�������
*******************************************************************************************/
u32 AngleSensorCalibrateXYFunction(LOGIC_SET_ANGLEPARAM_TYPE *pData)
{
	u32 SampleVADX = 0;
	u32 SampleVADY = 0;
//	u8 j = 0;
//	u16 RecX[20] = {0};
//	u16 RecY[20] = {0};

	if((pData->u8AnleXYflag != ANGLE_TYPE_X) && (pData->u8AnleXYflag != ANGLE_TYPE_Y))
		return 0x00;
	
//	if(s_stSensorType == SENSOR_AXIS_2)
//	{	
//		for(j=0; j < ANGLE_SENSOR_CALCULATE_TIMES; j++)//X���Y�����20��
//		{
//			RecX[j] = ReadLtc1865Value(X_CHANNEL);
//			RecY[j]  = ReadLtc1865Value(Y_CHANNEL);
//		}

//		SampleVADX = FilterAndSum (RecX, ANGLE_SENSOR_CALCULATE_TIMES);           //ȥ�����ֵ����Сֵȡƽ��ֵ
//		SampleVADY = FilterAndSum (RecY, ANGLE_SENSOR_CALCULATE_TIMES);

//		SampleVADX /= (ANGLE_SENSOR_CALCULATE_TIMES-0x02);
//		SampleVADY /= (ANGLE_SENSOR_CALCULATE_TIMES-0x02);
//	}
//	else 
	if(s_stSensorType == SENSOR_AXIS_6)	//6�ᴫ������ȡ���ľ��ǽǶ�ֵ���˴���ȡ���Ѿ���ƽ��ֵ���ʲ�����ƽ��
	{
		SampleVADX = s_s16SensorValueAvgX;		//6�ᴫ���������ĽǶ�ֵ�����ɸ������ṹ��LOGIC_SET_ANGLEPARAM_TYPE��u16AngleAdValueΪ�޷������Σ�����ʱ����ǿ��ת��Ϊs16
		SampleVADY = s_s16SensorValueAvgY;
    }		

	if(pData->u8AnleXYflag == ANGLE_TYPE_X)
		pData->u16AngleAdValue = SampleVADX;
	else
		pData->u16AngleAdValue = SampleVADY;
	
//	s_s16SensorValueAvgModifyX = s_s16SensorValueAvgX;
//	s_s16SensorValueAvgModifyY = s_s16SensorValueAvgY;

	return 0x01;
	
}

/*******************************************************************************************
**�������ƣ�AngleSensorSampleXYZProc
**�������ã���ȡMPU6500����DMP�����X�ᡢY�ᡢZ����б�Ƕ�
**������������
**�����������
**ע�������
*******************************************************************************************/
u32 AngleSensorSampleXYZProc(void)			//parry 2021.7.5
{
	u16 u16Min = 0xFF, u16Max = 0xFF, u16I = 0xFF;
	s16 s16AdMin = 0x00, s16AdMax = 0x00;
	s16 s16AngleMin = 0x00, s16AngleMax = 0x00;

	s8 result;
	float pitch, roll, yaw;
	short accel[3], gyro[3];
	s32 s32Sum = 0x00;
	s16	s16Angle;
	s32	s32Angle = 0x00;

	if(((s_stAngleDevTypeInf.u8DevSampleXyType) & ANGLE_TYPE_X_Y) != ANGLE_TYPE_X_Y)
		return 0x00;

	if(MPU_EXTI_flag)	//MPU6500���������ʱ���Ų����ж� �����жϺ�������λ�ı�š�MPU6500�����жϵ�������20ms���Ƕ������������10ms(ANGLESENSOR_TASK_TIME_SET),�������MPU6500��FIFO���
	{
		MPU_EXTI_flag = 0;
		result = MPU6500_dmp_get_euler_angle(accel, gyro, &pitch, &roll, &yaw);		//���6��ԭʼ�����Լ�DMP�ں�������3���Ƕ�
//		if(result == -6)	//dmp��Ҫ8�����Ҳ����ȶ���������ϵ�ʱ��
//		{
//			while(1)
//			{
//				result = MPU6500_dmp_get_euler_angle(accel,gyro,&pitch,&roll,&yaw);	//���6��ԭʼ�����Լ�DMP�ں�������3���Ƕ�
//				if(!result) 
//					break;
//			}
//		}

		if(result == 0)	//��MPU6500�ж�ȡ���Ƕ�
		{
			if(!s_u8SensorStart) 	//MPU6500��ʼ����
				s_u8SensorStart = 1;
			/***********************����X�Ƕ�ֵ************************************************************************************/
			s_stAngleSensorValueMpu6500X.s16SensorValueSample[s_stAngleSensorValueMpu6500X.u16SensorWritePtr] = (s16)(roll * 100);	//���Ƕȵ�λ��1��ת��Ϊ0.01��
			s_stAngleSensorValueMpu6500X.u16SensorWritePtr++;
			if(s_stAngleSensorValueMpu6500X.u16SensorWritePtr == ANGLE_SENSOR_CALCULATE_TIMES)			//�ﵽ����ƽ��ֵ�Ĵ���:10
			{
				if(s_u8SensorStopTimes)	//MPU6500�쳣��������
					s_u8SensorStopTimes = 0; 

				s_stAngleSensorValueMpu6500X.u16SensorWritePtr = 0x00;

				s32Sum = FilterAndSumMpu6500(&(s_stAngleSensorValueMpu6500X.s16SensorValueSample[0x00]), ANGLE_SENSOR_CALCULATE_TIMES);//(ANGLE_SENSOR_CALCULATE_TIMES-2)���ݵ�ƽ��ֵ
				
				s16Angle = (s16)(s32Sum / (ANGLE_SENSOR_CALCULATE_TIMES - 0x02)); 
				
				if((abs(s16Angle)) > s_stAngleDevTypeInf.u16PoseXRange)		//����������Χ֮��
				{
					s_stAngleSensorValueMpu6500X.s16SensorValue = ANGLE_SENOR_VALUE_INVALID;			
				}
				else
				{
					s_s16SensorValueAvgX = s16Angle;	//�궨ǰ�Ĳ���ֵ���յ��궨ָ��ʱ����ֵ���浽u16AngleAdValue
					
					/**��������Ƕ�ֵ***/
					if(s16Angle >= (s16)(s_stAngleModiftParam.u16ModifyX_AdValue[0x00]))  //ˮƽ�Ƕ�ֵ
					{
						if((s_stAngleModiftParam.u32ModifyX_ParamNumb != 0x01))
						{	
							for(u16I = 0x00; u16I < ((s_stAngleModiftParam.u32ModifyX_ParamNumb + 1) / 2); u16I++)//���Ҵ��ڵķ�Χ��ǰ�벿�ִ�ŵ���0�������Ƕȣ�0�㡢+x�㡢+y��...-x�㡢-y��...��u32ModifyX_ParamNumbΪ����
							{
								if(s16Angle >= (s16)(s_stAngleModiftParam.u16ModifyX_AdValue[u16I]))
								{
									s16AdMin = (s16)s_stAngleModiftParam.u16ModifyX_AdValue[u16I];
									s16AngleMin = s_stAngleModiftParam.s16ModifyX_AngleValue[u16I];
									if((u16I + 0x01) >= ((s_stAngleModiftParam.u32ModifyX_ParamNumb + 1) / 2))
										break;

									if(s16Angle <= (s16)(s_stAngleModiftParam.u16ModifyX_AdValue[u16I + 0x01]))
									{
										u16Min = u16I;
										u16Max = u16I + 0x01;
										s16AdMax = (s16)s_stAngleModiftParam.u16ModifyX_AdValue[u16I + 0x01];
										s16AngleMax = s_stAngleModiftParam.s16ModifyX_AngleValue[u16I + 0x01];
										break;
									}
								}
							}
						}
						else
						{
							u16Min = 0x00;
							s16AdMin = (s16)s_stAngleModiftParam.u16ModifyX_AdValue[0x00];		
						}
						/**����Ƕ�ֵ***/
						if((u16Min == 0x00) && (s_stAngleModiftParam.u32ModifyX_ParamNumb == 0x01))		//ֻ��һ���궨�㣺��ȥ���궨ֵ����
						{
							s16Angle -= s16AdMin;
							
							if(s16Angle <= s_stAngleDevTypeInf.u16PoseXRange)
								s_stAngleSensorValueMpu6500X.s16SensorValue = s16Angle;
							else
								s_stAngleSensorValueMpu6500X.s16SensorValue = s_stAngleDevTypeInf.u16PoseXRange;
						}
						else if(u16Min < ((s_stAngleModiftParam.u32ModifyX_ParamNumb + 1) / 2))		//�ж���궨�㣬�Ҳ���ֵ���������궨��֮�䣺��Ϊ������֮�������Եģ�����������
						{
							s32Angle = (s16AngleMax - s16AngleMin) * (s16Angle - s16AdMin);
							s32Angle /= (s16AdMax - s16AdMin);
							s32Angle += s16AngleMin;
							s_stAngleSensorValueMpu6500X.s16SensorValue = (s16)s32Angle;
						}
						else//�ж���궨�㣬�ҳ����궨ֵ�����Χ�����ݱ궨ֵ�����ֵб�ʽ��м��㣬���������ܳ���Ĭ�ϵ��豸�ɲ����Ƕȵ����ֵ
						{
							if( (s16Angle < s_stAngleDevTypeInf.u16PoseXRange) && (s16Angle >= (s16)(s_stAngleModiftParam.u16ModifyX_AdValue[0x00])))
							{
								s16AdMax = s_stAngleModiftParam.u16ModifyX_AdValue[s_stAngleModiftParam.u32ModifyX_ParamNumb/0x02];
								s16AdMin = s_stAngleModiftParam.u16ModifyX_AdValue[s_stAngleModiftParam.u32ModifyX_ParamNumb/0x02 - 0x01];
								s16AngleMax = s_stAngleModiftParam.s16ModifyX_AngleValue[s_stAngleModiftParam.u32ModifyX_ParamNumb/0x02];
								s16AngleMin = s_stAngleModiftParam.s16ModifyX_AngleValue[s_stAngleModiftParam.u32ModifyX_ParamNumb/0x02 - 0x01];
								
								s32Angle = (s16AngleMax - s16AngleMin) * (s16Angle - s16AdMin);
								s32Angle /= (s16AdMax - s16AdMin);
								s32Angle += s16AngleMin;
								s_stAngleSensorValueMpu6500X.s16SensorValue = (s16)s32Angle;
							}
							if(s_stAngleSensorValueMpu6500X.s16SensorValue >= s_stAngleDevTypeInf.u16PoseXRange)
								s_stAngleSensorValueMpu6500X.s16SensorValue = s_stAngleDevTypeInf.u16PoseXRange;
						}
					}
					/**���㸺��Ƕ�ֵ**/
					else if(s16Angle < (s16)(s_stAngleModiftParam.u16ModifyX_AdValue[0x00]))  //ˮƽ�Ƕ�ֵ
					{
						if((s_stAngleModiftParam.u32ModifyX_ParamNumb != 0x01))
						{	
							for(u16I = ((s_stAngleModiftParam.u32ModifyX_ParamNumb + 1)/2); u16I < s_stAngleModiftParam.u32ModifyX_ParamNumb; u16I++)//���Ҵ��ڵķ�Χ����벿�ִ�ŵ��Ǹ���Ƕȣ�0�㡢+x�㡢+y��...-x�㡢-y��...��u32ModifyX_ParamNumbΪ����
							{
								if(s16Angle >= (s16)(s_stAngleModiftParam.u16ModifyX_AdValue[u16I]))
								{
									s16AdMin = (s16)s_stAngleModiftParam.u16ModifyX_AdValue[u16I];
									s16AngleMin = s_stAngleModiftParam.s16ModifyX_AngleValue[u16I];
									if((u16I - 0x01) < ((s_stAngleModiftParam.u32ModifyX_ParamNumb + 1) / 2))
										u16Max = 0x00;
									else
										u16Max = (u16I - 0x01);
									if(s16Angle <= (s16)(s_stAngleModiftParam.u16ModifyX_AdValue[u16Max]))
									{
										u16Min = u16I;
										s16AdMax = (s16)s_stAngleModiftParam.u16ModifyX_AdValue[u16Max];
										s16AngleMax = s_stAngleModiftParam.s16ModifyX_AngleValue[u16Max];
										break;
									}
								}
							}
						}
						else
						{
							u16Max = 0x00;
							s16AdMax = (s16)s_stAngleModiftParam.u16ModifyX_AdValue[0x00];		
						}			
						/**����Ƕ�ֵ***/
						if((u16Max == 0x00) && (s_stAngleModiftParam.u32ModifyX_ParamNumb == 0x01))		//ֻ��һ���궨�㣺��ȥ���궨ֵ����
						{
							s16Angle -= s16AdMax;

							if(s16Angle >= (s_stAngleDevTypeInf.u16PoseXRange * -1))
								s_stAngleSensorValueMpu6500X.s16SensorValue = s16Angle;
							else
								s_stAngleSensorValueMpu6500X.s16SensorValue = (s_stAngleDevTypeInf.u16PoseXRange * -1);
						}
						else if((((u16Max >= ((s_stAngleModiftParam.u32ModifyX_ParamNumb + 1)/2))) && (u16Max < s_stAngleModiftParam.u32ModifyX_ParamNumb)) || (u16Max == 0x00))	//�ж���궨�㣬�Ҳ���ֵ���������궨��֮�䣺��Ϊ������֮�������Եģ�����������
						{						
							s32Angle = (s16AngleMin - s16AngleMax) * (s16Angle - s16AdMax);
							s32Angle /= (s16AdMin - s16AdMax);
							s32Angle += s16AngleMax;
							s_stAngleSensorValueMpu6500X.s16SensorValue = (s16)s32Angle;					
						}
						else//�ж���궨�㣬�ҳ����궨ֵ�����Χ�����ݱ궨ֵ�����ֵб�ʽ��м��㣬���������ܳ���Ĭ�ϵ��豸�ɲ����Ƕȵ����ֵ
						{
							if( (s16Angle >= (s_stAngleDevTypeInf.u16PoseXRange * -1)) && (s16Angle <= (s16)(s_stAngleModiftParam.u16ModifyX_AdValue[0x00])))
							{
								s16AdMax = s_stAngleModiftParam.u16ModifyX_AdValue[s_stAngleModiftParam.u32ModifyX_ParamNumb - 0x02];
								s16AdMin = s_stAngleModiftParam.u16ModifyX_AdValue[s_stAngleModiftParam.u32ModifyX_ParamNumb - 0x01];
								s16AngleMax = s_stAngleModiftParam.s16ModifyX_AngleValue[s_stAngleModiftParam.u32ModifyX_ParamNumb - 0x02];
								s16AngleMin = s_stAngleModiftParam.s16ModifyX_AngleValue[s_stAngleModiftParam.u32ModifyX_ParamNumb - 0x01];
														
								s32Angle = (s16AngleMin - s16AngleMax) * (s16Angle - s16AdMax);
								s32Angle /= (s16AdMin - s16AdMax);
								s32Angle += s16AngleMax;
								s_stAngleSensorValueMpu6500X.s16SensorValue = (s16)s32Angle;
							}
							
							if(s_stAngleSensorValueMpu6500X.s16SensorValue < (s_stAngleDevTypeInf.u16PoseXRange * -1))
								s_stAngleSensorValueMpu6500X.s16SensorValue = (s_stAngleDevTypeInf.u16PoseXRange * -1);
						}			
					}
					else
					{
						s_stAngleSensorValueMpu6500X.s16SensorValue = ANGLE_SENOR_VALUE_INVALID;	
					}
				}
				LogicRunInfApi(LOGIC_SET_ANGLEVALUE_X, &s_stAngleSensorValueMpu6500X.s16SensorValue);
			}
			
			/***********************����Y�Ƕ�ֵ************************************************************************************/
			s_stAngleSensorValueMpu6500Y.s16SensorValueSample[s_stAngleSensorValueMpu6500Y.u16SensorWritePtr] = (s16)(pitch * 100);	//���Ƕȵ�λ��1��ת��Ϊ0.01��
			s_stAngleSensorValueMpu6500Y.u16SensorWritePtr++;
			if(s_stAngleSensorValueMpu6500Y.u16SensorWritePtr == ANGLE_SENSOR_CALCULATE_TIMES)
			{
				if(s_u8SensorStopTimes)	//MPU6500�쳣��������
					s_u8SensorStopTimes = 0; 

				s_stAngleSensorValueMpu6500Y.u16SensorWritePtr = 0x00;

				s32Sum = FilterAndSumMpu6500(&(s_stAngleSensorValueMpu6500Y.s16SensorValueSample[0x00]), ANGLE_SENSOR_CALCULATE_TIMES);//(ANGLE_SENSOR_CALCULATE_TIMES-2)���ݵ�ƽ��ֵ
				
				s16Angle = (s16)(s32Sum / (ANGLE_SENSOR_CALCULATE_TIMES - 0x02)); 
				
				if((abs(s16Angle)) > s_stAngleDevTypeInf.u16PoseYRange)		//����������Χ֮��
				{
					s_stAngleSensorValueMpu6500Y.s16SensorValue = ANGLE_SENOR_VALUE_INVALID;			
				}
				else
				{
					s_s16SensorValueAvgY = s16Angle ;	//�궨ǰ�Ĳ���ֵ���յ��궨ָ��ʱ����ֵ���浽u16AngleAdValue������Ϊ��GUD90(B)��Y������һ��
					
					/**��������Ƕ�ֵ***/
					if(s16Angle >= (s16)(s_stAngleModiftParam.u16ModifyY_AdValue[0x00]))  //ˮƽ�Ƕ�ֵ
					{
						if((s_stAngleModiftParam.u32ModifyY_ParamNumb != 0x01))
						{	
							for(u16I = 0x00; u16I < ((s_stAngleModiftParam.u32ModifyY_ParamNumb + 1) / 2); u16I++)//���Ҵ��ڵķ�Χ��ǰ�벿�ִ�ŵ���0�������Ƕȣ�0�㡢+x�㡢+y��...-x�㡢-y��...��u32ModifyY_ParamNumbΪ����
							{
								if(s16Angle >= (s16)(s_stAngleModiftParam.u16ModifyY_AdValue[u16I]))
								{
									s16AdMin = (s16)s_stAngleModiftParam.u16ModifyY_AdValue[u16I];
									s16AngleMin = s_stAngleModiftParam.s16ModifyY_AngleValue[u16I];
									if((u16I + 0x01) >= ((s_stAngleModiftParam.u32ModifyY_ParamNumb + 1) / 2))
										break;

									if(s16Angle <= (s16)(s_stAngleModiftParam.u16ModifyY_AdValue[u16I + 0x01]))
									{
										u16Min = u16I;
										u16Max = u16I + 0x01;
										s16AdMax = (s16)s_stAngleModiftParam.u16ModifyY_AdValue[u16I + 0x01];
										s16AngleMax = s_stAngleModiftParam.s16ModifyY_AngleValue[u16I + 0x01];
										break;
									}
								}
							}
						}
						else
						{
							u16Min = 0x00;
							s16AdMin = (s16)s_stAngleModiftParam.u16ModifyY_AdValue[0x00];		
						}
						/**����Ƕ�ֵ***/
						if((u16Min == 0x00) && (s_stAngleModiftParam.u32ModifyY_ParamNumb == 0x01))		//ֻ��һ���궨�㣺��ȥ���궨ֵ����
						{
							s16Angle -= s16AdMin;
							
							if(s16Angle <= s_stAngleDevTypeInf.u16PoseYRange)
								s_stAngleSensorValueMpu6500Y.s16SensorValue = s16Angle;	
							else
								s_stAngleSensorValueMpu6500Y.s16SensorValue = s_stAngleDevTypeInf.u16PoseYRange;
						}
						else if(u16Min < ((s_stAngleModiftParam.u32ModifyY_ParamNumb + 1)/2))		//�ж���궨�㣬�Ҳ���ֵ���������궨��֮�䣺��Ϊ������֮�������Եģ�����������
						{
							s32Angle = (s16AngleMax - s16AngleMin) * (s16Angle - s16AdMin);
							s32Angle /= (s16AdMax - s16AdMin);
							s32Angle += s16AngleMin;

							if((s16)s32Angle <= s_stAngleDevTypeInf.u16PoseYRange)
								s_stAngleSensorValueMpu6500Y.s16SensorValue = (s16)s32Angle;	
							else
								s_stAngleSensorValueMpu6500Y.s16SensorValue = s_stAngleDevTypeInf.u16PoseYRange;
						}
						else//�ж���궨�㣬�ҳ����궨ֵ�����Χ�����ݱ궨ֵ�����ֵб�ʽ��м��㣬���������ܳ���Ĭ�ϵ��豸�ɲ����Ƕȵ����ֵ+2
						{
							if( (s16Angle < s_stAngleDevTypeInf.u16PoseYRange) && (s16Angle >= (s16)(s_stAngleModiftParam.u16ModifyY_AdValue[0x00])))
							{
								s16AdMax = s_stAngleModiftParam.u16ModifyY_AdValue[s_stAngleModiftParam.u32ModifyY_ParamNumb/0x02];
								s16AdMin = s_stAngleModiftParam.u16ModifyY_AdValue[s_stAngleModiftParam.u32ModifyY_ParamNumb/0x02 - 0x01];
								s16AngleMax = s_stAngleModiftParam.s16ModifyY_AngleValue[s_stAngleModiftParam.u32ModifyY_ParamNumb/0x02];
								s16AngleMin = s_stAngleModiftParam.s16ModifyY_AngleValue[s_stAngleModiftParam.u32ModifyY_ParamNumb/0x02 - 0x01];
								
								s32Angle = (s16AngleMax - s16AngleMin) * (s16Angle - s16AdMin);
								s32Angle /= (s16AdMax - s16AdMin);
								s32Angle += s16AngleMin;
								s_stAngleSensorValueMpu6500Y.s16SensorValue = (s16)s32Angle;	
							}
							if(s_stAngleSensorValueMpu6500Y.s16SensorValue >= s_stAngleDevTypeInf.u16PoseYRange)
							{
								if(s_stAngleSensorValueMpu6500Y.s16SensorValue <= (s_stAngleDevTypeInf.u16PoseYRange + 500))
									s_stAngleSensorValueMpu6500Y.s16SensorValue = s_stAngleDevTypeInf.u16PoseYRange;	//���޲�����5��ʱ�������Ƕȴ���
								else
									s_stAngleSensorValueMpu6500Y.s16SensorValue = ANGLE_SENOR_VALUE_INVALID;
							}
						}
					}
					/**���㸺��Ƕ�ֵ**/
					else if(s16Angle < (s16)(s_stAngleModiftParam.u16ModifyY_AdValue[0x00]))  //ˮƽ�Ƕ�ֵ
					{
						if((s_stAngleModiftParam.u32ModifyY_ParamNumb != 0x01))
						{	
							for(u16I = ((s_stAngleModiftParam.u32ModifyY_ParamNumb + 1) / 2); u16I < s_stAngleModiftParam.u32ModifyY_ParamNumb; u16I++)//���Ҵ��ڵķ�Χ����벿�ִ�ŵ��Ǹ���Ƕȣ�0�㡢+x�㡢+y��...-x�㡢-y��...��u32ModifyY_ParamNumbΪ����
							{
								if(s16Angle >= (s16)(s_stAngleModiftParam.u16ModifyY_AdValue[u16I]))
								{
									s16AdMin = (s16)s_stAngleModiftParam.u16ModifyY_AdValue[u16I];
									s16AngleMin = s_stAngleModiftParam.s16ModifyY_AngleValue[u16I];
									if((u16I - 0x01) < ((s_stAngleModiftParam.u32ModifyY_ParamNumb + 1) / 2))
										u16Max = 0x00;
									else
										u16Max = (u16I - 0x01);
									if(s16Angle <= (s16)(s_stAngleModiftParam.u16ModifyY_AdValue[u16Max]))
									{
										u16Min = u16I;
										s16AdMax = (s16)s_stAngleModiftParam.u16ModifyY_AdValue[u16Max];
										s16AngleMax = s_stAngleModiftParam.s16ModifyY_AngleValue[u16Max];
										break;
									}
								}
							}
						}
						else
						{
							u16Max = 0x00;
							s16AdMax = (s16)s_stAngleModiftParam.u16ModifyY_AdValue[0x00];		
						}			
						/**����Ƕ�ֵ***/
						if((u16Max == 0x00) && (s_stAngleModiftParam.u32ModifyY_ParamNumb == 0x01))		//ֻ��һ���궨�㣺��ȥ���궨ֵ����
						{
							s16Angle -= s16AdMax;

							if(s16Angle >= (s_stAngleDevTypeInf.u16PoseYRange * -1))
								s_stAngleSensorValueMpu6500Y.s16SensorValue = s16Angle;	
							else
								s_stAngleSensorValueMpu6500Y.s16SensorValue = (s_stAngleDevTypeInf.u16PoseYRange * -1);
						}
						else if((((u16Max >= ((s_stAngleModiftParam.u32ModifyY_ParamNumb + 1)/2))) && (u16Max < s_stAngleModiftParam.u32ModifyY_ParamNumb)) || (u16Max == 0x00))	//�ж���궨�㣬�Ҳ���ֵ���������궨��֮�䣺��Ϊ������֮�������Եģ�����������
						{						
							s32Angle = (s16AngleMin - s16AngleMax) * (s16Angle - s16AdMax);
							s32Angle /= (s16AdMin - s16AdMax);
							s32Angle += s16AngleMax;

							if((s16)s32Angle >= (s_stAngleDevTypeInf.u16PoseYRange * -1))
								s_stAngleSensorValueMpu6500Y.s16SensorValue = (s16)s32Angle;	
							else
								s_stAngleSensorValueMpu6500Y.s16SensorValue = (s_stAngleDevTypeInf.u16PoseYRange * -1);
						}
						else//�ж���궨�㣬�ҳ����궨ֵ�����Χ�����ݱ궨ֵ�����ֵб�ʽ��м��㣬���������ܳ���Ĭ�ϵ��豸�ɲ����Ƕȵ����ֵ-2
						{
							if( (s16Angle >= ( - s_stAngleDevTypeInf.u16PoseYRange)) && (s16Angle <= (s16)(s_stAngleModiftParam.u16ModifyY_AdValue[0x00])))
							{
								s16AdMax = s_stAngleModiftParam.u16ModifyY_AdValue[s_stAngleModiftParam.u32ModifyY_ParamNumb - 0x02];
								s16AdMin = s_stAngleModiftParam.u16ModifyY_AdValue[s_stAngleModiftParam.u32ModifyY_ParamNumb - 0x01];
								s16AngleMax = s_stAngleModiftParam.s16ModifyY_AngleValue[s_stAngleModiftParam.u32ModifyY_ParamNumb - 0x02];
								s16AngleMin = s_stAngleModiftParam.s16ModifyY_AngleValue[s_stAngleModiftParam.u32ModifyY_ParamNumb - 0x01];
														
								s32Angle = (s16AngleMin - s16AngleMax) * (s16Angle - s16AdMax);
								s32Angle /= (s16AdMin - s16AdMax);
								s32Angle += s16AngleMax;
								s_stAngleSensorValueMpu6500Y.s16SensorValue = (s16)s32Angle;
							}
							
							if(s_stAngleSensorValueMpu6500Y.s16SensorValue < (s_stAngleDevTypeInf.u16PoseYRange * -1))
							{
								if(s_stAngleSensorValueMpu6500Y.s16SensorValue >= (s_stAngleDevTypeInf.u16PoseYRange * -1 - 500))
									s_stAngleSensorValueMpu6500Y.s16SensorValue = s_stAngleDevTypeInf.u16PoseYRange * -1;		//���޲�����5��ʱ�������Ƕȴ���
								else
									s_stAngleSensorValueMpu6500Y.s16SensorValue = ANGLE_SENOR_VALUE_INVALID;
							}
						}
					}
					else
					{
						s_stAngleSensorValueMpu6500Y.s16SensorValue = ANGLE_SENOR_VALUE_INVALID;	
					}
				}
				LogicRunInfApi(LOGIC_SET_ANGLEVALUE_Y, &s_stAngleSensorValueMpu6500Y.s16SensorValue);
			}
			//Z����Ƕ�ֵ������
#if 0
			/***********************����Z�Ƕ�ֵ************************************************************************************/
			s_stAngleSensorValueMpu6500Z.s16SensorValueSample[s_stAngleSensorValueMpu6500Z.u16SensorWritePtr] = (s16)(yaw * 100);	//���Ƕȵ�λ��1��ת��Ϊ0.01��
			s_stAngleSensorValueMpu6500Z.u16SensorWritePtr++;
			if(s_stAngleSensorValueMpu6500Z.u16SensorWritePtr == ANGLE_SENSOR_CALCULATE_TIMES)
			{
				if(s_u8SensorStopTimes)	//MPU6500�쳣��������
					s_u8SensorStopTimes = 0; 

				s_stAngleSensorValueMpu6500Z.u16SensorWritePtr = 0x00;
				s32Sum = FilterAndSumMpu6500(&(s_stAngleSensorValueMpu6500Z.s16SensorValueSample[0x00]), ANGLE_SENSOR_CALCULATE_TIMES);//(ANGLE_SENSOR_CALCULATE_TIMES-2)���ݵ�ƽ��ֵ
				s32Sum /= (ANGLE_SENSOR_CALCULATE_TIMES - 0x02);
//				s32Sum -= (s_u16SensorValueModifyZ * 50);	//��Ư����Z��Ƕ�ÿ116.1s����0.5�㣬�˴�������Ư
//				if(s32Sum < -18000) s32Sum += 36000;	//ŷ���Ǵ���180������-180�㣬�ڴ˴�������
//				s16Angle = (s16)s32Sum - s_s16SensorValuePowerOnZ;	//�ϵ�ʱZ��Ƕ�����
						
				s16Angle = (s16)s32Sum;
				s_stAngleSensorValueMpu6500Z.s16SensorValue = s16Angle;
				LogicRunInfApi(LOGIC_SET_ANGLEVALUE_Z, &s_stAngleSensorValueMpu6500Z.s16SensorValue);
			}
#endif
		}
	}
	
	return 0x01;
}

/*******************************************************************************************
**�������ƣ�CanLeftPeriodProc
**�������ã���CAN�����Դ������
**������������
**�����������
**ע�������
*******************************************************************************************/
void CanLeftPeriodProc(u32 u32TimeDelay)
{
	u16 u16VauleTempX, u16VauleTempY;
	sCAN_FRAME SendFrame;
	stFRAME SendFrame1;
	u32 Temp = 0x00;
	
	u16CanLeftSendTimer += u32TimeDelay;
	if(u16CanLeftSendTimer % 10)	//���������(���³��򡢲�νӿ�)������u16CanRightSendTimer��λ��Ϊ0��������ᵼ�½Ƕ����ݷ�����ȥ
		u16CanLeftSendTimer -= (u16CanLeftSendTimer % 10);
	if(u16CanLeftSendTimer % 100 == 0) 
	{
		LogicRunInfApi(LOGIC_GET_SYSTEM_RUN_STATUS, &Temp);
		if(Temp != SYSTEM_RUN_STATUS_UPPRG)
		{
			SendFrame1.u32ID.ID.RxID = (u8)(HUB_DEV_TYPE & 0xFF);
			SendFrame1.u32ID.ID.TxID = (u8)(ANGLE_DEV_TYPE & 0xFF);
			SendFrame1.u32ID.ID.FrameType = ANGLE_REPORT_VALUE;			//�Ƕ��ϱ�֡
			SendFrame1.u32ID.ID.Sum = 0;
			SendFrame1.u32ID.ID.Sub = 0;
			SendFrame1.u32ID.ID.ACK = NO_ACK;
			SendFrame1.u32ID.ID.Reservd = 0;
			SendFrame1.u32ID.ID.NoUsed = 0;
			
			SendFrame1.u16DLC = CAN_LENGTH_6;			//�Ƕ��ϱ�֡6���ֽ�

			SendFrame1.u8DT[0x00] = 0x00;		//ԭʼ���ͣ�ת������Ϊ00H��
			SendFrame1.u8DT[0x01] = SUB_DEVICE_NUM;		//������̬�Ƕȴ�������
			LogicRunInfApi(LOGIC_GET_ANGLEVALUE_X, &u16VauleTempX);
			SendFrame1.u8DT[0x02] = (u8)(u16VauleTempX & 0xFF);
			SendFrame1.u8DT[0x03] = (u8)((u16VauleTempX & 0xFF00) >> 8);
			
			LogicRunInfApi(LOGIC_GET_ANGLEVALUE_Y, &u16VauleTempY);
			SendFrame1.u8DT[0x04] = (u8)(u16VauleTempY & 0xFF);
			SendFrame1.u8DT[0x05] = (u8)((u16VauleTempY & 0xFF00) >> 8);	

			//���仯ֵ�Ƿ񳬹������ϱ�ֵ
			if(u8LeftActiveReportAngleChange != 0 && (((abs(u16LastLeftAngleValue_X - u16VauleTempX) / 10) > u8LeftActiveReportAngleChange)
													|| (abs((u16LastLeftAngleValue_Y - u16VauleTempY) / 10) > u8LeftActiveReportAngleChange)))
			{
				u16LastLeftAngleValue_X = u16VauleTempX;
				u16LastLeftAngleValue_Y = u16VauleTempY;

				u8LeftAngleFlag = 1;
			}
			//�ﵽ�����ϱ��������ϱ�ʱ��������
			if ((u8LeftAngleFlag == 1)\
			|| (u16CanLeftSendTimer >= u16CanLeftSendInterval))			//�ﵽ״̬�仯����ﵽ�ϱ�����ʱ��Ҫ�����ϱ�״̬
			{
				u16CanLeftSendTimer = 0x00;		
				u8LeftAngleFlag = 0;
				
				SendFrame1.u32ID.ID.LiuShuiNumb = u16CanLeftLiuShuiNumb;
				u16CanLeftLiuShuiNumb ++;
				u16CanLeftLiuShuiNumb %= STFRAME_LS_MAX; 
				SendFrame.Stdid = SendFrame1.u32ID.u32Id;
				SendFrame.DLC = SendFrame1.u16DLC;
				memcpy(SendFrame.Data, SendFrame1.u8DT, SendFrame1.u16DLC);
				STM32_CAN_Write(0, SendFrame, CAN_FF_EXTENDED);
			}
		}
	}
}
/*******************************************************************************************
**�������ƣ� CanTransAngleHeartBeatProc
**�������ã� ���ͽǶ������ϱ�����
**������������
**�����������
**ע�������
*******************************************************************************************/
sCAN_FRAME CanTransAngleHeartBeatProc(u8 systemRunStatus)
{
	stFRAME SendFrame;
	sCAN_FRAME ret;
	
	/***���Ϳ�������ʶ�������֡**/
	SendFrame.u32ID.ID.RxID = (u8)(SC_DEV_TYPE & 0xFF);
	SendFrame.u32ID.ID.TxID = (u8)(ANGLE_DEV_TYPE & 0xFF);
	SendFrame.u32ID.ID.FrameType = ANGLE_HEART;
	SendFrame.u32ID.ID.Sum = 0;
	SendFrame.u32ID.ID.Sub = 0;
	SendFrame.u32ID.ID.ACK = NO_ACK;
	SendFrame.u32ID.ID.Reservd = 0;
	SendFrame.u32ID.ID.NoUsed = 0;
	
	SendFrame.u16DLC = CAN_LENGTH_8;

	ret.Data[0x00] = ANGLE_HEART_BYTE0;
	ret.Data[0x01] = ANGLE_HEART_BYTE1;
	ret.Data[0x02] = ANGLE_HEART_BYTE2;
	ret.Data[0x03] = 0x3a;				//�๦�������еĶ�������豸�����ϱ���������
	ret.Data[0x04] = systemRunStatus;
	ret.Data[0x05] = VERSION_1;
	ret.Data[0x06] = VERSION_2;
	ret.Data[0x07] = VERSION_3;
	
	ret.Stdid = SendFrame.u32ID.u32Id;
	ret.DLC = SendFrame.u16DLC;
	
	return ret;
}

/************************************************************************************************
* ������������������
 * �����������
 * ����ʱ�䣺
 * �������ߣ�
*************************************************************************************************/
sCAN_FRAME SendHeartData(uint32_t func, uint8_t dest, uint8_t *data, uint8_t size)
{
	CanHeadID  tmp;
	sCAN_FRAME ret;
//	uint8_t    buf[12];

	memset(&tmp, 0x00, sizeof(tmp));
	memset(&ret, 0x00, sizeof(ret));
	switch (func)
	{
		//���ⱨ��������
		case BEEP_LIGHT_HEART:
			tmp.ID.TID = UWB_ID;
			memcpy(ret.Data, "ALM", 3);
			ret.Data[3] = BEEP_LIGHT_MODEL;          //ָ��Ϊ���ⱨ��������ģ��
			ret.Data[4] = 0;
			ret.Data[5] = VERSION_1;   //�汾��
			ret.Data[6] = VERSION_2;   //�汾��
			ret.Data[7] = VERSION_3;   //�汾��
			ret.DLC = 8;
			ret.Stdid = tmp.u32Id;
			break;
		//����ģ������
		case WIRELESS_MOUDLE_HEART:
			tmp.ID.TID = WL_RID;//3
			memcpy(ret.Data, "WLM", 3);
			ret.Data[3] = 0x31;                 //ָ��Ϊ��������ģ��
            ret.Data[4] = eCanWLReportMode;     //0:����״̬��1��ʼ��״̬��2���³���״̬
			ret.Data[5] = VERSION_1;            //�汾��
			ret.Data[6] = VERSION_2;            //�汾��
			ret.Data[7] = VERSION_3;            //�汾��
			ret.DLC = 8;
			ret.Stdid = tmp.u32Id;
			break;

		default:
			break;
	}
	return ret;
}
/*******************************************************************************************
**�������ƣ�MPU6500Init
**�������ã���ʼ������
**������������
**�����������
**ע�������
*******************************************************************************************/
void MPU6500Init(void)
{
	MPU6500_Port_EXIT_Init();	//MPU6050���ж�ͨ����ʼ��
	InitMPU6050();				//��ʼ��MPU6050
	MPU6500_DMP_Init(); 		//MPU6050��dmp��ʼ��
}

/*******************************************************************************************
**�������ƣ�AngleSensor_Task
**�������ã��źŲɼ�������������
**������������
**�����������
**ע�������
*******************************************************************************************/
void AngleSensor_Task(void *p_arg) 
{
	u32 u32Sum;
	
	LogicInit();			//Logic ������ʼ��

	MPU6500Init();			//MPU6050���ж�ͨ����ʼ������ʼ��MPU6050���в�����MPU6050��dmp��ʼ��

	AngleSensorInit();		//�ǶȲɼ������ʼ��
	
	u32Sum = SYSTEM_RUN_STATUS_NORMAL;		//�ǶȲɼ�����״̬��ʼ��
	LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);
	
	OSTimeDly(200);						//��ʱ200mS
	
	while(1)
	{
		OSTimeDly(ANGLESENSOR_TASK_TIME_SET);	//������10ms

		AngleSensorSampleXYZProc();				//�źŲɼ�����
	}
}

/*******************************************************************************************
**�������ƣ�AngleSensorMNG_Task
**�������ã�������״̬���ڹ�������
**������������
**�����������
**ע�������
*******************************************************************************************/
void AngleSensorMNG_Task(void *p_arg) 
{
	OS_TCB pdata;
	u32 Temp = 0x00;
    
	STM32_CAN_Write(0, SendHeartData(WIRELESS_MOUDLE_HEART, 0, NULL, 0), CAN_FF_EXTENDED); //����ģ���ϵ���������3�Σ�ÿ�μ��1s
    OSTimeDly(1000);
    STM32_CAN_Write(0, SendHeartData(WIRELESS_MOUDLE_HEART, 0, NULL, 0), CAN_FF_EXTENDED); //����ģ���ϵ���������3�Σ�ÿ�μ��1s
    OSTimeDly(1000);
    STM32_CAN_Write(0, SendHeartData(WIRELESS_MOUDLE_HEART, 0, NULL, 0), CAN_FF_EXTENDED); //����ģ���ϵ���������3�Σ�ÿ�μ��1s
	OSTimeDly(ANGLESENSOR_POWERUP_TIME_SET);						//��ʱ9S������������ȴ���3S��Լ12S�нǶ�ֵ��ʾ
	while(1)
	{
		OSTimeDly(ANGLESENSOR_TASK_TIME_SET);	//������10ms
		//���ڳ������״̬�£���û���յ��������֡
		if(u32CanLeftPrgRecvTimer == 0x00)
		{
			{
				u16AlarmSendHeartTimer++;								//���ⱨ��������֡��ʱ
				//���ⱨ��������֡
				if(u16AlarmSendHeartTimer >= ALARM_HEARTTIMERMAX)	// == ��Ϊ >= �����������ʱ����������������
				{
					u16AlarmSendHeartTimer = 0;
					STM32_CAN_Write(0, SendHeartData(BEEP_LIGHT_HEART, 0, NULL, 0), CAN_FF_EXTENDED); // д���ⱨ��������֡
				}
			}
			//ʹ�ܽǶȲɼ�����
			if ((Get_DevID() & GYRO_ANGLE_ENABLED) == GYRO_ANGLE_ENABLED)
			{
				CanLeftPeriodProc(ANGLESENSOR_TASK_TIME_SET);

				u16CanLeftSendHeartTimer++;								//�Ƕȴ���������֡��ʱ
				// �Ƕȴ���������֡
				if(u16CanLeftSendHeartTimer >= CANLEFT_HEARTTIMERMAX)	// == ��Ϊ >= �����������ʱ����������������
				{
					u16CanLeftSendHeartTimer = 0;
					/***���Ϳ�������ʶ�������֡**/
					STM32_CAN_Write(0, CanTransAngleHeartBeatProc(Temp), CAN_FF_EXTENDED); // д�Ƕȴ���������֡
				}
			}
			//ʹ��433����ģ�鹦��
			if ((Get_DevID() & WM433_WIRELESS_ENABLED) == WM433_WIRELESS_ENABLED)
			{
                u16WLMSendHeartTimer ++;								//����ģ������֡��ʱ
                //����ģ������֡
                if(u16WLMSendHeartTimer >= WLM_HEARTTIMERMAX)	// == ��Ϊ >= �����������ʱ����������������
                {
                    u16WLMSendHeartTimer = 0;
                    STM32_CAN_Write(0, SendHeartData(WIRELESS_MOUDLE_HEART, 0, NULL, 0), CAN_FF_EXTENDED); // д����ģ������֡
                }
			}
		}
		
		if(u32CanLeftPrgRecvTimer > ANGLESENSOR_TASK_TIME_SET)			//�������״̬��ʱ
			u32CanLeftPrgRecvTimer -= ANGLESENSOR_TASK_TIME_SET;
		else
		{
			u32CanLeftPrgRecvTimer = 0x00;
			LogicRunInfApi(LOGIC_GET_SYSTEM_RUN_STATUS, &Temp);
			if(Temp == SYSTEM_RUN_STATUS_UPPRG)
			{
				Temp = SYSTEM_RUN_STATUS_NORMAL;
				LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &Temp);
			}
		}
		//�ȴ����ճ���ʱ����֮ǰ���ָ��ǶȲɼ��ͽǶ����ݹ�����
		if(u32CanLeftPrgRecvTimer == ANGLESENSOR_TASK_TIME_SET)
		{
			//ʹ�ܽǶȲɼ�����
			if ((Get_DevID() & GYRO_ANGLE_ENABLED) == GYRO_ANGLE_ENABLED)
			{
				/**�ָ���Ҫ������**/
				OSTaskQuery(ANGLE_SENSOR_TASK_PRIO, &pdata);			//��ѯ���������Ƿ����
				if(pdata.OSTCBStat == OS_STAT_SUSPEND)
				{
					OSTaskResume(ANGLE_SENSOR_TASK_PRIO);				//�ָ�����
				}
			}
			//ʹ����Ա��λ����
			if ((Get_DevID() & UWB_PERSONNEL_ENABLED) == UWB_PERSONNEL_ENABLED)
			{
				/**�ָ���Ҫ������**/
				OSTaskQuery(UWBAPP_TASK_PRIO, &pdata);			//��ѯ���������Ƿ����
				if(pdata.OSTCBStat == OS_STAT_SUSPEND)
				{
					OSTaskResume(UWBAPP_TASK_PRIO);				//�ָ�����
				}
				/**�ָ���Ҫ������**/
				OSTaskQuery(CANTX_TASK_PRIO, &pdata);			//��ѯ���������Ƿ����	
				if(pdata.OSTCBStat == OS_STAT_SUSPEND)
				{
					OSTaskResume(CANTX_TASK_PRIO);				//�ָ�����
				}
			}
			//ʹ��433����ģ�鹦��
			if ((Get_DevID() & WM433_WIRELESS_ENABLED) == WM433_WIRELESS_ENABLED)
			{
				/**�ָ���Ҫ������**/
				OSTaskQuery(WL_MANAGE_TASK_PRIO, &pdata);			//��ѯ���������Ƿ����	
				if(pdata.OSTCBStat == OS_STAT_SUSPEND)
				{	
					OSTaskResume(WL_MANAGE_TASK_PRIO);
				}
			}
		}
	}
}
