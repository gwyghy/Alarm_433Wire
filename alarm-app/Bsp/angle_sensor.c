/********************************************************************************
* 文件名称：	angle_sensor.c
* 作	者：	马如意   
* 当前版本：   	V1.0
* 完成日期：    2014.12.08
* 功能描述: 	完成AD 传感器逻辑层的处理，实现设备初始化、数据读写等操作。
* 历史信息：   
*           	版本信息     完成时间      原作者        注释
*
*       >>>>  在工程中的位置  <<<<
*          	  3-应用层
*             2-协议层
*          √  1-硬件驱动层
*********************************************************************************
* Copyright (c) 2014,天津华宁电子有限公司 All rights reserved.
*********************************************************************************/
/********************************************************************************
* .h头文件
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
* #define宏定义
*********************************************************************************/
#define ANGLE_SENSOR_CALCULATE_TIMES	10//计算平均值的次数
#define	ANGLE_SENOR_VALUE_INVALID		0x9999//无效的角度数值

extern WL_WORK_STATUS eCanWLReportMode;     //无线模块的工作模式


//typedef struct
//{
//	u16 u16SensorValueSample[ANGLE_SENSOR_CALCULATE_TIMES];	//保存采样值，采样个数可设
//	u16 u16SensorWritePtr;	//采样值要保存的位置
//	s16 s16SensorValue;
//}ANGLE_SENSOR_SAMPLE_TYPE;

typedef struct
{
	s16 s16SensorValueSample[ANGLE_SENSOR_CALCULATE_TIMES];	//保存采样值，采样个数可设	MPU6500的原始采样值是浮点数	parry 2021.7.5
	u16 u16SensorWritePtr;	//采样值要保存的位置
	s16 s16SensorValue;
}ANGLE_SENSOR_SAMPLE_TYPE_MPU6500;

/********************************************************************************
* 常量定义
*********************************************************************************/

/********************************************************************************
* 变量定义
*********************************************************************************/
static ANGLE_SENSOR_SAMPLE_TYPE_MPU6500 s_stAngleSensorValueMpu6500X;
static ANGLE_SENSOR_SAMPLE_TYPE_MPU6500 s_stAngleSensorValueMpu6500Y;
static ANGLE_SENSOR_SAMPLE_TYPE_MPU6500 s_stAngleSensorValueMpu6500Z;

static LOGIC_POSEDEV_INF_TYPE s_stAngleDevTypeInf;
static LOGIC_MODIFYPARAM_TYPE s_stAngleModiftParam;

SENSOR_TYPE	s_stSensorType;

static s16 s_s16SensorValueAvgX = 0x00;	//保存最新测量的X轴角度均值，单位是0.01°，校准用	parry 2021.7.5
static s16 s_s16SensorValueAvgY = 0x00;	//保存最新测量的Y轴角度均值，单位是0.01°，校准用
//static u16 s_u16SensorValueModifyZ = 0x00; //对Z轴数据进行补偿，抵消零漂带来的影响
//static s16 s_s16SensorValuePowerOnZ = 0x00; //记录刚上电时Z轴数据，每次上电Z轴角度清零

static u8  s_u8SensorStart = 0x00;  	//MPU6500产生中断信号时开始计时
static u8  s_u8SensorStopTimes = 0x00;  //MPU6500异常次数

//static s16 s_s16SensorValueAvgModifyX = 0x00;	//校准时的X轴角度值	parry 2021.7.5
//static s16 s_s16SensorValueAvgModifyY = 0x00;	//校准时的Y轴角度值
u16 u16CanLeftSendInterval = 0x00;
static u8 u8LeftActiveReportAngleChange = 0x00;

u32 u32CanLeftPrgRecvTimer = 0x00;//接收程序计时器

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
* 函数定义
*********************************************************************************/

/*******************************************************************************************
**函数名称：AngleSensorInit
**函数作用：角度采集任务初始化
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
u8 AngleSensorInit(void) 
{
//	INPUT_VALUE_TYPE InputValue;
//	TIM_TimeBaseInitTypeDef		TIM_Time2Structure;		 //定义定时器结构体
//	NVIC_InitTypeDef			NVIC_InitStructure; 
	
//	memset(&s_stAngleSensorValueX,0x00,sizeof(ANGLE_SENSOR_SAMPLE_TYPE) );
//	memset(&s_stAngleSensorValueY,0x00,sizeof(ANGLE_SENSOR_SAMPLE_TYPE) );
	
	memset(&s_stAngleSensorValueMpu6500X, 0x00, sizeof(ANGLE_SENSOR_SAMPLE_TYPE_MPU6500));
	memset(&s_stAngleSensorValueMpu6500Y, 0x00, sizeof(ANGLE_SENSOR_SAMPLE_TYPE_MPU6500));
	memset(&s_stAngleSensorValueMpu6500Z, 0x00, sizeof(ANGLE_SENSOR_SAMPLE_TYPE_MPU6500));
	  
	/* 读取传感器类型信息 */
//	InputGetValue(INPUT_1, &InputValue);	
//	if(InputValue == INPUT_LOW)		//输入为低时，表示倾角传感器型号为GUD90(C)，采用的是6轴传感器。在GUD90(C)上INPUT_1接地，读回低电平。	parry 2021.7.5
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
	TIM_TimeBaseStructInit(&TIM_Time2Structure);		//初始化定时器2结构体
	TIM_DeInit(TIM2);									//设置TIM2默认模式

	/* TIM2 Periph clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	TIM_Time2Structure.TIM_Period = 14999;									// 设置初始值,定时7.5ms中断
	TIM_Time2Structure.TIM_Prescaler = 41;									// 设置定时器2的分频值，时钟为2MHz
	TIM_Time2Structure.TIM_ClockDivision=TIM_CKD_DIV1;						// 设置时钟分割
	TIM_Time2Structure.TIM_CounterMode=TIM_CounterMode_Up;					// TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_Time2Structure);							// 按结构体的值，初始化定时器2
	TIM_InternalClockConfig(TIM2);											// 设置TIM2的时钟为内部时钟
	TIM2->CNT = 0x0000;														// 定时器2计数器清零
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);								// 使能TIM2溢出中断源
	TIM_ARRPreloadConfig(TIM2, ENABLE);										// 使能自动重装
	TIM_Cmd(TIM2, DISABLE);													// 禁止TIM2

    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;							// 使能或者失能指定的IRQ通道 TIM2全局中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0f;			// 不使用中断优先级嵌套。因为SysTick的中断优先级为0x0f
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;	//1;				// 设置成员 NVIC_IRQChannel中的从优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;							// 使能 NVIC_IRQChannel
	NVIC_Init(&NVIC_InitStructure);    

	TIM_Cmd(TIM2, ENABLE);													// 使能TIM2
#endif
	return 0x01;
}
#if 0
//定时器2溢出中断函数
void TIM2_IRQHandler(void)
{	  
	OS_CPU_SR  cpu_sr;
	static u16 times;
//	static u16 times1;
	OS_ENTER_CRITICAL();                         /* Tell uC/OS-II that we are starting an ISR          */
	OSIntEnter();
	OS_EXIT_CRITICAL();

	if(TIM_GetITStatus(TIM2,TIM_IT_Update) == SET)							// 判断是否发生定时器溢出中断
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);							// 定时器溢出中断待处理标记清零
		TIM_ClearFlag(TIM2, TIM_FLAG_Update);								// 与上句功能相同
		
		times ++;
		s_u8SensorStopTimes ++;		//40ms
		if((s_u8SensorStart > 0) && (s_u8SensorStopTimes > 50))	//MPU6500开始工作后，连续2s都读不到角度，重启
			//执行跳转
			IapJumpToBoot(IN_FLASH_BOOTLOADER_ADDR);
		
//		if(times > 3299)	//3299对应130s。每130s补偿-0.5°，可以使6台整机测试13.5h后，Z轴漂移在±8.6°，也就是±0.64°/h
//		if(times > 2828)	//第二次系统测试时，3299会导致13小时后正漂30°，降为2828后，纠正提升1/7。3299会在13小时纠正180°，实际需要纠正210°，2828会纠正210°
//		{
//			times = 0;
//			s_u16SensorValueModifyZ++;
//			s_u16SensorValueModifyZ %= 720;  //增加360°等于没变
//		}
//		if((times > 300) && (s_s16SensorValuePowerOnZ == 0))	 //300对应12s延时，上电12s后Z轴数据稳定下来	
//		{
//			s_s16SensorValuePowerOnZ = s_stAngleSensorValueMpu6500Z.s16SensorValue;
//		}
	}

	OSIntExit();
}
#endif
//MPU6500引脚中断
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
* 函数名称：FilterAndSum 
* 功能描述：数据去掉一个最大值和最小值，求余下数据的和
* 入口参数：adc：数据；length：数据个数
* 出口参数：无
* 使用说明：无
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
* 函数名称：FilterAndSum 
* 功能描述：数据去掉一个最大值和最小值，求余下数据的和
* 入口参数：adc：数据；length：数据个数
* 出口参数：无
* 使用说明：无
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
**函数名称：CalibrateXYFunction
**函数作用：校准X、Y角度值
**函数参数：无
**函数输出：无
**注意事项：无
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
//		for(j=0; j < ANGLE_SENSOR_CALCULATE_TIMES; j++)//X轴和Y轴各读20次
//		{
//			RecX[j] = ReadLtc1865Value(X_CHANNEL);
//			RecY[j]  = ReadLtc1865Value(Y_CHANNEL);
//		}

//		SampleVADX = FilterAndSum (RecX, ANGLE_SENSOR_CALCULATE_TIMES);           //去掉最大值和最小值取平均值
//		SampleVADY = FilterAndSum (RecY, ANGLE_SENSOR_CALCULATE_TIMES);

//		SampleVADX /= (ANGLE_SENSOR_CALCULATE_TIMES-0x02);
//		SampleVADY /= (ANGLE_SENSOR_CALCULATE_TIMES-0x02);
//	}
//	else 
	if(s_stSensorType == SENSOR_AXIS_6)	//6轴传感器读取到的就是角度值；此处读取的已经是平均值，故不再求平均
	{
		SampleVADX = s_s16SensorValueAvgX;		//6轴传感器读到的角度值可正可负，但结构体LOGIC_SET_ANGLEPARAM_TYPE中u16AngleAdValue为无符号整形，补偿时须先强制转换为s16
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
**函数名称：AngleSensorSampleXYZProc
**函数作用：读取MPU6500经过DMP处理的X轴、Y轴、Z轴倾斜角度
**函数参数：无
**函数输出：无
**注意事项：无
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

	if(MPU_EXTI_flag)	//MPU6500有数据输出时引脚产生中断 ，在中断函数中置位改标号。MPU6500产生中断的周期是20ms，角度任务的周期是10ms(ANGLESENSOR_TASK_TIME_SET),不会造成MPU6500的FIFO溢出
	{
		MPU_EXTI_flag = 0;
		result = MPU6500_dmp_get_euler_angle(accel, gyro, &pitch, &roll, &yaw);		//获得6轴原始数据以及DMP融合运算后的3个角度
//		if(result == -6)	//dmp需要8秒左右才能稳定输出，刚上电时，
//		{
//			while(1)
//			{
//				result = MPU6500_dmp_get_euler_angle(accel,gyro,&pitch,&roll,&yaw);	//获得6轴原始数据以及DMP融合运算后的3个角度
//				if(!result) 
//					break;
//			}
//		}

		if(result == 0)	//从MPU6500中读取到角度
		{
			if(!s_u8SensorStart) 	//MPU6500开始工作
				s_u8SensorStart = 1;
			/***********************计算X角度值************************************************************************************/
			s_stAngleSensorValueMpu6500X.s16SensorValueSample[s_stAngleSensorValueMpu6500X.u16SensorWritePtr] = (s16)(roll * 100);	//将角度单位由1°转换为0.01°
			s_stAngleSensorValueMpu6500X.u16SensorWritePtr++;
			if(s_stAngleSensorValueMpu6500X.u16SensorWritePtr == ANGLE_SENSOR_CALCULATE_TIMES)			//达到计算平均值的次数:10
			{
				if(s_u8SensorStopTimes)	//MPU6500异常次数清零
					s_u8SensorStopTimes = 0; 

				s_stAngleSensorValueMpu6500X.u16SensorWritePtr = 0x00;

				s32Sum = FilterAndSumMpu6500(&(s_stAngleSensorValueMpu6500X.s16SensorValueSample[0x00]), ANGLE_SENSOR_CALCULATE_TIMES);//(ANGLE_SENSOR_CALCULATE_TIMES-2)数据的平均值
				
				s16Angle = (s16)(s32Sum / (ANGLE_SENSOR_CALCULATE_TIMES - 0x02)); 
				
				if((abs(s16Angle)) > s_stAngleDevTypeInf.u16PoseXRange)		//不在正常范围之内
				{
					s_stAngleSensorValueMpu6500X.s16SensorValue = ANGLE_SENOR_VALUE_INVALID;			
				}
				else
				{
					s_s16SensorValueAvgX = s16Angle;	//标定前的测量值；收到标定指令时将该值保存到u16AngleAdValue
					
					/**计算正向角度值***/
					if(s16Angle >= (s16)(s_stAngleModiftParam.u16ModifyX_AdValue[0x00]))  //水平角度值
					{
						if((s_stAngleModiftParam.u32ModifyX_ParamNumb != 0x01))
						{	
							for(u16I = 0x00; u16I < ((s_stAngleModiftParam.u32ModifyX_ParamNumb + 1) / 2); u16I++)//查找处于的范围，前半部分存放的是0°和正向角度：0°、+x°、+y°...-x°、-y°...，u32ModifyX_ParamNumb为奇数
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
						/**计算角度值***/
						if((u16Min == 0x00) && (s_stAngleModiftParam.u32ModifyX_ParamNumb == 0x01))		//只有一个标定点：减去零点标定值即可
						{
							s16Angle -= s16AdMin;
							
							if(s16Angle <= s_stAngleDevTypeInf.u16PoseXRange)
								s_stAngleSensorValueMpu6500X.s16SensorValue = s16Angle;
							else
								s_stAngleSensorValueMpu6500X.s16SensorValue = s_stAngleDevTypeInf.u16PoseXRange;
						}
						else if(u16Min < ((s_stAngleModiftParam.u32ModifyX_ParamNumb + 1) / 2))		//有多个标定点，且测量值处于两个标定点之间：认为这两点之间是线性的，按比例运算
						{
							s32Angle = (s16AngleMax - s16AngleMin) * (s16Angle - s16AdMin);
							s32Angle /= (s16AdMax - s16AdMin);
							s32Angle += s16AngleMin;
							s_stAngleSensorValueMpu6500X.s16SensorValue = (s16)s32Angle;
						}
						else//有多个标定点，且超过标定值的最大范围，依据标定值的最大值斜率进行计算，计算结果不能超过默认的设备可测量角度的最大值
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
					/**计算负向角度值**/
					else if(s16Angle < (s16)(s_stAngleModiftParam.u16ModifyX_AdValue[0x00]))  //水平角度值
					{
						if((s_stAngleModiftParam.u32ModifyX_ParamNumb != 0x01))
						{	
							for(u16I = ((s_stAngleModiftParam.u32ModifyX_ParamNumb + 1)/2); u16I < s_stAngleModiftParam.u32ModifyX_ParamNumb; u16I++)//查找处于的范围，后半部分存放的是负向角度：0°、+x°、+y°...-x°、-y°...，u32ModifyX_ParamNumb为奇数
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
						/**计算角度值***/
						if((u16Max == 0x00) && (s_stAngleModiftParam.u32ModifyX_ParamNumb == 0x01))		//只有一个标定点：减去零点标定值即可
						{
							s16Angle -= s16AdMax;

							if(s16Angle >= (s_stAngleDevTypeInf.u16PoseXRange * -1))
								s_stAngleSensorValueMpu6500X.s16SensorValue = s16Angle;
							else
								s_stAngleSensorValueMpu6500X.s16SensorValue = (s_stAngleDevTypeInf.u16PoseXRange * -1);
						}
						else if((((u16Max >= ((s_stAngleModiftParam.u32ModifyX_ParamNumb + 1)/2))) && (u16Max < s_stAngleModiftParam.u32ModifyX_ParamNumb)) || (u16Max == 0x00))	//有多个标定点，且测量值处于两个标定点之间：认为这两点之间是线性的，按比例运算
						{						
							s32Angle = (s16AngleMin - s16AngleMax) * (s16Angle - s16AdMax);
							s32Angle /= (s16AdMin - s16AdMax);
							s32Angle += s16AngleMax;
							s_stAngleSensorValueMpu6500X.s16SensorValue = (s16)s32Angle;					
						}
						else//有多个标定点，且超过标定值的最大范围，依据标定值的最大值斜率进行计算，计算结果不能超过默认的设备可测量角度的最大值
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
			
			/***********************计算Y角度值************************************************************************************/
			s_stAngleSensorValueMpu6500Y.s16SensorValueSample[s_stAngleSensorValueMpu6500Y.u16SensorWritePtr] = (s16)(pitch * 100);	//将角度单位由1°转换为0.01°
			s_stAngleSensorValueMpu6500Y.u16SensorWritePtr++;
			if(s_stAngleSensorValueMpu6500Y.u16SensorWritePtr == ANGLE_SENSOR_CALCULATE_TIMES)
			{
				if(s_u8SensorStopTimes)	//MPU6500异常次数清零
					s_u8SensorStopTimes = 0; 

				s_stAngleSensorValueMpu6500Y.u16SensorWritePtr = 0x00;

				s32Sum = FilterAndSumMpu6500(&(s_stAngleSensorValueMpu6500Y.s16SensorValueSample[0x00]), ANGLE_SENSOR_CALCULATE_TIMES);//(ANGLE_SENSOR_CALCULATE_TIMES-2)数据的平均值
				
				s16Angle = (s16)(s32Sum / (ANGLE_SENSOR_CALCULATE_TIMES - 0x02)); 
				
				if((abs(s16Angle)) > s_stAngleDevTypeInf.u16PoseYRange)		//不在正常范围之内
				{
					s_stAngleSensorValueMpu6500Y.s16SensorValue = ANGLE_SENOR_VALUE_INVALID;			
				}
				else
				{
					s_s16SensorValueAvgY = s16Angle ;	//标定前的测量值；收到标定指令时将该值保存到u16AngleAdValue；调整为与GUD90(B)的Y轴正向一致
					
					/**计算正向角度值***/
					if(s16Angle >= (s16)(s_stAngleModiftParam.u16ModifyY_AdValue[0x00]))  //水平角度值
					{
						if((s_stAngleModiftParam.u32ModifyY_ParamNumb != 0x01))
						{	
							for(u16I = 0x00; u16I < ((s_stAngleModiftParam.u32ModifyY_ParamNumb + 1) / 2); u16I++)//查找处于的范围，前半部分存放的是0°和正向角度：0°、+x°、+y°...-x°、-y°...，u32ModifyY_ParamNumb为奇数
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
						/**计算角度值***/
						if((u16Min == 0x00) && (s_stAngleModiftParam.u32ModifyY_ParamNumb == 0x01))		//只有一个标定点：减去零点标定值即可
						{
							s16Angle -= s16AdMin;
							
							if(s16Angle <= s_stAngleDevTypeInf.u16PoseYRange)
								s_stAngleSensorValueMpu6500Y.s16SensorValue = s16Angle;	
							else
								s_stAngleSensorValueMpu6500Y.s16SensorValue = s_stAngleDevTypeInf.u16PoseYRange;
						}
						else if(u16Min < ((s_stAngleModiftParam.u32ModifyY_ParamNumb + 1)/2))		//有多个标定点，且测量值处于两个标定点之间：认为这两点之间是线性的，按比例运算
						{
							s32Angle = (s16AngleMax - s16AngleMin) * (s16Angle - s16AdMin);
							s32Angle /= (s16AdMax - s16AdMin);
							s32Angle += s16AngleMin;

							if((s16)s32Angle <= s_stAngleDevTypeInf.u16PoseYRange)
								s_stAngleSensorValueMpu6500Y.s16SensorValue = (s16)s32Angle;	
							else
								s_stAngleSensorValueMpu6500Y.s16SensorValue = s_stAngleDevTypeInf.u16PoseYRange;
						}
						else//有多个标定点，且超过标定值的最大范围，依据标定值的最大值斜率进行计算，计算结果不能超过默认的设备可测量角度的最大值+2
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
									s_stAngleSensorValueMpu6500Y.s16SensorValue = s_stAngleDevTypeInf.u16PoseYRange;	//超限不大于5°时都按最大角度处理
								else
									s_stAngleSensorValueMpu6500Y.s16SensorValue = ANGLE_SENOR_VALUE_INVALID;
							}
						}
					}
					/**计算负向角度值**/
					else if(s16Angle < (s16)(s_stAngleModiftParam.u16ModifyY_AdValue[0x00]))  //水平角度值
					{
						if((s_stAngleModiftParam.u32ModifyY_ParamNumb != 0x01))
						{	
							for(u16I = ((s_stAngleModiftParam.u32ModifyY_ParamNumb + 1) / 2); u16I < s_stAngleModiftParam.u32ModifyY_ParamNumb; u16I++)//查找处于的范围，后半部分存放的是负向角度：0°、+x°、+y°...-x°、-y°...，u32ModifyY_ParamNumb为奇数
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
						/**计算角度值***/
						if((u16Max == 0x00) && (s_stAngleModiftParam.u32ModifyY_ParamNumb == 0x01))		//只有一个标定点：减去零点标定值即可
						{
							s16Angle -= s16AdMax;

							if(s16Angle >= (s_stAngleDevTypeInf.u16PoseYRange * -1))
								s_stAngleSensorValueMpu6500Y.s16SensorValue = s16Angle;	
							else
								s_stAngleSensorValueMpu6500Y.s16SensorValue = (s_stAngleDevTypeInf.u16PoseYRange * -1);
						}
						else if((((u16Max >= ((s_stAngleModiftParam.u32ModifyY_ParamNumb + 1)/2))) && (u16Max < s_stAngleModiftParam.u32ModifyY_ParamNumb)) || (u16Max == 0x00))	//有多个标定点，且测量值处于两个标定点之间：认为这两点之间是线性的，按比例运算
						{						
							s32Angle = (s16AngleMin - s16AngleMax) * (s16Angle - s16AdMax);
							s32Angle /= (s16AdMin - s16AdMax);
							s32Angle += s16AngleMax;

							if((s16)s32Angle >= (s_stAngleDevTypeInf.u16PoseYRange * -1))
								s_stAngleSensorValueMpu6500Y.s16SensorValue = (s16)s32Angle;	
							else
								s_stAngleSensorValueMpu6500Y.s16SensorValue = (s_stAngleDevTypeInf.u16PoseYRange * -1);
						}
						else//有多个标定点，且超过标定值的最大范围，依据标定值的最大值斜率进行计算，计算结果不能超过默认的设备可测量角度的最大值-2
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
									s_stAngleSensorValueMpu6500Y.s16SensorValue = s_stAngleDevTypeInf.u16PoseYRange * -1;		//超限不大于5°时都按最大角度处理
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
			//Z轴向角度值不计算
#if 0
			/***********************计算Z角度值************************************************************************************/
			s_stAngleSensorValueMpu6500Z.s16SensorValueSample[s_stAngleSensorValueMpu6500Z.u16SensorWritePtr] = (s16)(yaw * 100);	//将角度单位由1°转换为0.01°
			s_stAngleSensorValueMpu6500Z.u16SensorWritePtr++;
			if(s_stAngleSensorValueMpu6500Z.u16SensorWritePtr == ANGLE_SENSOR_CALCULATE_TIMES)
			{
				if(s_u8SensorStopTimes)	//MPU6500异常次数清零
					s_u8SensorStopTimes = 0; 

				s_stAngleSensorValueMpu6500Z.u16SensorWritePtr = 0x00;
				s32Sum = FilterAndSumMpu6500(&(s_stAngleSensorValueMpu6500Z.s16SensorValueSample[0x00]), ANGLE_SENSOR_CALCULATE_TIMES);//(ANGLE_SENSOR_CALCULATE_TIMES-2)数据的平均值
				s32Sum /= (ANGLE_SENSOR_CALCULATE_TIMES - 0x02);
//				s32Sum -= (s_u16SensorValueModifyZ * 50);	//零漂导致Z轴角度每116.1s增加0.5°，此处消除零漂
//				if(s32Sum < -18000) s32Sum += 36000;	//欧拉角大于180°后会变成-180°，在此处做补偿
//				s16Angle = (s16)s32Sum - s_s16SensorValuePowerOnZ;	//上电时Z轴角度清零
						
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
**函数名称：CanLeftPeriodProc
**函数作用：左CAN周期性处理过程
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void CanLeftPeriodProc(u32 u32TimeDelay)
{
	u16 u16VauleTempX, u16VauleTempY;
	sCAN_FRAME SendFrame;
	stFRAME SendFrame1;
	u32 Temp = 0x00;
	
	u16CanLeftSendTimer += u32TimeDelay;
	if(u16CanLeftSendTimer % 10)	//特殊情况下(更新程序、插拔接口)，存在u16CanRightSendTimer个位不为0的情况，会导致角度数据发不出去
		u16CanLeftSendTimer -= (u16CanLeftSendTimer % 10);
	if(u16CanLeftSendTimer % 100 == 0) 
	{
		LogicRunInfApi(LOGIC_GET_SYSTEM_RUN_STATUS, &Temp);
		if(Temp != SYSTEM_RUN_STATUS_UPPRG)
		{
			SendFrame1.u32ID.ID.RxID = (u8)(HUB_DEV_TYPE & 0xFF);
			SendFrame1.u32ID.ID.TxID = (u8)(ANGLE_DEV_TYPE & 0xFF);
			SendFrame1.u32ID.ID.FrameType = ANGLE_REPORT_VALUE;			//角度上报帧
			SendFrame1.u32ID.ID.Sum = 0;
			SendFrame1.u32ID.ID.Sub = 0;
			SendFrame1.u32ID.ID.ACK = NO_ACK;
			SendFrame1.u32ID.ID.Reservd = 0;
			SendFrame1.u32ID.ID.NoUsed = 0;
			
			SendFrame1.u16DLC = CAN_LENGTH_6;			//角度上报帧6个字节

			SendFrame1.u8DT[0x00] = 0x00;		//原始发送，转发次数为00H。
			SendFrame1.u8DT[0x01] = SUB_DEVICE_NUM;		//顶梁动态角度传感器。
			LogicRunInfApi(LOGIC_GET_ANGLEVALUE_X, &u16VauleTempX);
			SendFrame1.u8DT[0x02] = (u8)(u16VauleTempX & 0xFF);
			SendFrame1.u8DT[0x03] = (u8)((u16VauleTempX & 0xFF00) >> 8);
			
			LogicRunInfApi(LOGIC_GET_ANGLEVALUE_Y, &u16VauleTempY);
			SendFrame1.u8DT[0x04] = (u8)(u16VauleTempY & 0xFF);
			SendFrame1.u8DT[0x05] = (u8)((u16VauleTempY & 0xFF00) >> 8);	

			//检查变化值是否超过主动上报值
			if(u8LeftActiveReportAngleChange != 0 && (((abs(u16LastLeftAngleValue_X - u16VauleTempX) / 10) > u8LeftActiveReportAngleChange)
													|| (abs((u16LastLeftAngleValue_Y - u16VauleTempY) / 10) > u8LeftActiveReportAngleChange)))
			{
				u16LastLeftAngleValue_X = u16VauleTempX;
				u16LastLeftAngleValue_Y = u16VauleTempY;

				u8LeftAngleFlag = 1;
			}
			//达到主动上报或周期上报时间间隔条件
			if ((u8LeftAngleFlag == 1)\
			|| (u16CanLeftSendTimer >= u16CanLeftSendInterval))			//达到状态变化量或达到上报周期时需要主动上报状态
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
**函数名称： CanTransAngleHeartBeatProc
**函数作用： 发送角度心跳上报数据
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
sCAN_FRAME CanTransAngleHeartBeatProc(u8 systemRunStatus)
{
	stFRAME SendFrame;
	sCAN_FRAME ret;
	
	/***发送控制器能识别的心跳帧**/
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
	ret.Data[0x03] = 0x3a;				//多功能声光中的顶梁倾角设备类型上报心跳数据
	ret.Data[0x04] = systemRunStatus;
	ret.Data[0x05] = VERSION_1;
	ret.Data[0x06] = VERSION_2;
	ret.Data[0x07] = VERSION_3;
	
	ret.Stdid = SendFrame.u32ID.u32Id;
	ret.DLC = SendFrame.u16DLC;
	
	return ret;
}

/************************************************************************************************
* 功能描述：发送心跳
 * 输入参数：无
 * 创建时间：
 * 创建作者：
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
		//声光报警器心跳
		case BEEP_LIGHT_HEART:
			tmp.ID.TID = UWB_ID;
			memcpy(ret.Data, "ALM", 3);
			ret.Data[3] = BEEP_LIGHT_MODEL;          //指定为声光报警器心跳模块
			ret.Data[4] = 0;
			ret.Data[5] = VERSION_1;   //版本号
			ret.Data[6] = VERSION_2;   //版本号
			ret.Data[7] = VERSION_3;   //版本号
			ret.DLC = 8;
			ret.Stdid = tmp.u32Id;
			break;
		//无线模块心跳
		case WIRELESS_MOUDLE_HEART:
			tmp.ID.TID = WL_RID;//3
			memcpy(ret.Data, "WLM", 3);
			ret.Data[3] = 0x31;                 //指定为无线心跳模块
            ret.Data[4] = eCanWLReportMode;     //0:正常状态，1初始化状态，2更新程序状态
			ret.Data[5] = VERSION_1;            //版本号
			ret.Data[6] = VERSION_2;            //版本号
			ret.Data[7] = VERSION_3;            //版本号
			ret.DLC = 8;
			ret.Stdid = tmp.u32Id;
			break;

		default:
			break;
	}
	return ret;
}
/*******************************************************************************************
**函数名称：MPU6500Init
**函数作用：初始化外设
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void MPU6500Init(void)
{
	MPU6500_Port_EXIT_Init();	//MPU6050的中断通道初始化
	InitMPU6050();				//初始化MPU6050
	MPU6500_DMP_Init(); 		//MPU6050的dmp初始化
}

/*******************************************************************************************
**函数名称：AngleSensor_Task
**函数作用：信号采集及计算主任务
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void AngleSensor_Task(void *p_arg) 
{
	u32 u32Sum;
	
	LogicInit();			//Logic 参数初始化

	MPU6500Init();			//MPU6050的中断通道初始化，初始化MPU6050运行参数，MPU6050的dmp初始化

	AngleSensorInit();		//角度采集任务初始化
	
	u32Sum = SYSTEM_RUN_STATUS_NORMAL;		//角度采集任务状态初始化
	LogicRunInfApi(LOGIC_SET_SYSTEM_RUN_STATUS, &u32Sum);
	
	OSTimeDly(200);						//延时200mS
	
	while(1)
	{
		OSTimeDly(ANGLESENSOR_TASK_TIME_SET);	//任务间隔10ms

		AngleSensorSampleXYZProc();				//信号采集计算
	}
}

/*******************************************************************************************
**函数名称：AngleSensorMNG_Task
**函数作用：传感器状态周期管理任务
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void AngleSensorMNG_Task(void *p_arg) 
{
	OS_TCB pdata;
	u32 Temp = 0x00;
    
	STM32_CAN_Write(0, SendHeartData(WIRELESS_MOUDLE_HEART, 0, NULL, 0), CAN_FF_EXTENDED); //无线模块上电心跳，发3次，每次间隔1s
    OSTimeDly(1000);
    STM32_CAN_Write(0, SendHeartData(WIRELESS_MOUDLE_HEART, 0, NULL, 0), CAN_FF_EXTENDED); //无线模块上电心跳，发3次，每次间隔1s
    OSTimeDly(1000);
    STM32_CAN_Write(0, SendHeartData(WIRELESS_MOUDLE_HEART, 0, NULL, 0), CAN_FF_EXTENDED); //无线模块上电心跳，发3次，每次间隔1s
	OSTimeDly(ANGLESENSOR_POWERUP_TIME_SET);						//延时9S加上引导程序等待的3S，约12S有角度值显示
	while(1)
	{
		OSTimeDly(ANGLESENSOR_TASK_TIME_SET);	//任务间隔10ms
		//不在程序更新状态下，即没有收到程序更新帧
		if(u32CanLeftPrgRecvTimer == 0x00)
		{
			{
				u16AlarmSendHeartTimer++;								//声光报警器心跳帧定时
				//声光报警器心跳帧
				if(u16AlarmSendHeartTimer >= ALARM_HEARTTIMERMAX)	// == 改为 >= ，避免出现有时发不出心跳的现象
				{
					u16AlarmSendHeartTimer = 0;
					STM32_CAN_Write(0, SendHeartData(BEEP_LIGHT_HEART, 0, NULL, 0), CAN_FF_EXTENDED); // 写声光报警器心跳帧
				}
			}
			//使能角度采集功能
			if ((Get_DevID() & GYRO_ANGLE_ENABLED) == GYRO_ANGLE_ENABLED)
			{
				CanLeftPeriodProc(ANGLESENSOR_TASK_TIME_SET);

				u16CanLeftSendHeartTimer++;								//角度传感器心跳帧定时
				// 角度传感器心跳帧
				if(u16CanLeftSendHeartTimer >= CANLEFT_HEARTTIMERMAX)	// == 改为 >= ，避免出现有时发不出心跳的现象
				{
					u16CanLeftSendHeartTimer = 0;
					/***发送控制器能识别的心跳帧**/
					STM32_CAN_Write(0, CanTransAngleHeartBeatProc(Temp), CAN_FF_EXTENDED); // 写角度传感器心跳帧
				}
			}
			//使能433无线模块功能
			if ((Get_DevID() & WM433_WIRELESS_ENABLED) == WM433_WIRELESS_ENABLED)
			{
                u16WLMSendHeartTimer ++;								//无线模块心跳帧定时
                //无线模块心跳帧
                if(u16WLMSendHeartTimer >= WLM_HEARTTIMERMAX)	// == 改为 >= ，避免出现有时发不出心跳的现象
                {
                    u16WLMSendHeartTimer = 0;
                    STM32_CAN_Write(0, SendHeartData(WIRELESS_MOUDLE_HEART, 0, NULL, 0), CAN_FF_EXTENDED); // 写无线模块心跳帧
                }
			}
		}
		
		if(u32CanLeftPrgRecvTimer > ANGLESENSOR_TASK_TIME_SET)			//程序接收状态计时
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
		//等待接收程序超时结束之前，恢复角度采集和角度数据管理功能
		if(u32CanLeftPrgRecvTimer == ANGLESENSOR_TASK_TIME_SET)
		{
			//使能角度采集功能
			if ((Get_DevID() & GYRO_ANGLE_ENABLED) == GYRO_ANGLE_ENABLED)
			{
				/**恢复必要的任务**/
				OSTaskQuery(ANGLE_SENSOR_TASK_PRIO, &pdata);			//查询处理任务是否挂起
				if(pdata.OSTCBStat == OS_STAT_SUSPEND)
				{
					OSTaskResume(ANGLE_SENSOR_TASK_PRIO);				//恢复任务
				}
			}
			//使能人员定位功能
			if ((Get_DevID() & UWB_PERSONNEL_ENABLED) == UWB_PERSONNEL_ENABLED)
			{
				/**恢复必要的任务**/
				OSTaskQuery(UWBAPP_TASK_PRIO, &pdata);			//查询处理任务是否挂起
				if(pdata.OSTCBStat == OS_STAT_SUSPEND)
				{
					OSTaskResume(UWBAPP_TASK_PRIO);				//恢复任务
				}
				/**恢复必要的任务**/
				OSTaskQuery(CANTX_TASK_PRIO, &pdata);			//查询处理任务是否挂起	
				if(pdata.OSTCBStat == OS_STAT_SUSPEND)
				{
					OSTaskResume(CANTX_TASK_PRIO);				//恢复任务
				}
			}
			//使能433无线模块功能
			if ((Get_DevID() & WM433_WIRELESS_ENABLED) == WM433_WIRELESS_ENABLED)
			{
				/**恢复必要的任务**/
				OSTaskQuery(WL_MANAGE_TASK_PRIO, &pdata);			//查询处理任务是否挂起	
				if(pdata.OSTCBStat == OS_STAT_SUSPEND)
				{	
					OSTaskResume(WL_MANAGE_TASK_PRIO);
				}
			}
		}
	}
}
