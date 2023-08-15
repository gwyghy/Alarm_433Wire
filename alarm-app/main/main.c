/**
  ******************************************************************************
  * 文件名称: main.c
  * 作    者: 
	* 当前版本：V1.0
	* 完成日期：
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

#include "main.h"
#include "app_cfg.h"
#include "l4spi.h"

#include "port.h"
#include "sleep.h"

#include "uwb_app.h"
#include "dw1000_bus.h"
#include "spi_bus.h"
#include "can_app.h"
#if DECA_WORK_MODE == UWB_ALARM_MODE
#include "uwb_anchor.h"
#endif /* DECA_WORK_MODE == UWB_ALARM_MODE */

#include "led_app.h"
#include "beep_app.h"

#include "logic.h"
#include "angle_sensor.h"

#include "iwdg.h"

/*创建任务堆栈，容量256*/
OS_STK TaskStartStk[APPMNG_TASK_STK_SIZE];
#if DECA_WORK_MODE == UWB_LISTEN_MODE
	OS_STK UWB_RxStk[UWBRX_TASK_STK_SIZE];
#elif DECA_WORK_MODE == UWB_ALARM_MODE
	OS_STK UWB_AnchorStk[UWB_ANCHOR_TASK_STK_SIZE];
#else
	OS_STK UWB_AnchorStk[UWB_ANCHOR_TASK_STK_SIZE];
#endif  /*DECA_WORK_MODE == UWB_LISTEN_MODE*/

OS_STK AppTaskKLightStk[APP_LIGHT_TASK_STK_SIZE];
OS_STK AppTaskBeepStk[APP_BEEP_TASK_STK_SIZE];

OS_STK anglesensor_task_stk[ANGLE_SENSOR_TASK_STK_SIZE];
OS_STK anglesensormng_task_stk[ANGLE_SENSOR_MNG_TASK_STK_SIZE];
#ifdef IWDG_ENABLED
OS_STK  iwdg_task_stk[IWDG_TASK_STK_SIZE];						//开辟任务堆栈
#endif

OS_STK  wl_manage_task_stk[UWBRX_TASK_STK_SIZE];		//开辟任务堆栈

u32 g_u32ID = 0;		//存储ID配置字
/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
	OSTimeDly(1000);
}

/********************************************************
* 任务名称： uint16_t Get_JTAG_ID()
* 功能描述： 获取单片机设备类型
* 输入参数： 无
* 输出参数： 无
* 返 回 值： 返回0x041是ST系列单片机
			 返回0x7A3是GD系列单片机
* 其他说明： 无
* 修改日期   版本号    修改人  修改内容
* ------------------------------------------------------
********************************************************/
uint16_t Get_JTAG_ID()
{
	if (*(uint8_t*)(0xE00FFFE8) & 0x08)
	{
		return ((*(uint8_t*)(0xE00FFFD0)  & 0x0F) << 8) |
				((*(uint8_t*)(0xE00FFFE4) & 0xFF) >> 3) |
				((*(uint8_t*)(0xE00FFFE8)  & 0x07) << 5) + 1;
	}
	
	return 0;
}
/********************************************************
* 任务名称：DevID_Init
* 功能描述：初始化ID识别端口
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其他说明：无
* 修改日期   版本号    修改人  修改内容
* ------------------------------------------------------
********************************************************/
void DevID_Init(void)
{
	/*******************************
	ID值       配置关系
	ID0			声光
	ID1			UWB
	ID2         倾角GYRO
	ID3         433无线模块
	*******************************/
//	GPIO_InitTypeDef GPIO_InitStructure;
	u32	i;
	g_u32ID = 0;
	// 配置ID识别接口
	ID_GPIO_CLK_ENABLE();			   //使能配置使用端口时钟

	/* enable the ID0 clock */
    rcu_periph_clock_enable(ID0_PORT);
	/* enable the ID3 clock */
    rcu_periph_clock_enable(ID3_PORT);
//    /* configure ID0 GPIO port */ 
//    gpio_init(ID0_GPIO, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, ID0_PIN);
//	/* configure ID1 GPIO port */ 
//    gpio_init(ID1_GPIO, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, ID1_PIN);
//	/* configure ID2 GPIO port */ 
//    gpio_init(ID2_GPIO, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, ID2_PIN);
//	/* configure ID3 GPIO port */ 
//    gpio_init(ID3_GPIO, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, ID3_PIN);
    /* configure ID0 GPIO port */ 
    gpio_init(ID0_GPIO, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, ID0_PIN);		//配置为上拉模式，通过外部电阻置低改变输入状态
	/* configure ID1 GPIO port */ 
    gpio_init(ID1_GPIO, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, ID1_PIN);
	/* configure ID2 GPIO port */ 
    gpio_init(ID2_GPIO, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, ID2_PIN);
	/* configure ID3 GPIO port */ 
    gpio_init(ID3_GPIO, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, ID3_PIN);

#if 0			//IO预留实际不使用
	//LED指示灯
	/* enable the LED clock */
    rcu_periph_clock_enable(LED_PORT);
    /* configure LED2 GPIO port */ 
    gpio_init(LED_GPIO, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED_PIN);
    //LED指示灯亮？
    gpio_bit_reset(LED_GPIO, LED_PIN);
#endif
	i = 0;
	//ID0代表声光，高电平表示具有此功能
//	i = gpio_input_bit_get(ID0_GPIO, ID0_PIN);
//	if (i == 1)
//	{
//		g_u32ID |= BEEP_ALARM_ENABLED;
//	}
	//声光报警功能不需要配置，默认都有此功能
	g_u32ID |= BEEP_ALARM_ENABLED;
	//ID1代表UWB，高电平表示具有此功能
	i = gpio_input_bit_get(ID1_GPIO, ID1_PIN);//PA1
	if (i == 1)
	{
		g_u32ID |= UWB_PERSONNEL_ENABLED;
	}
	//ID2代表倾角，高电平表示具有此功能
	i = gpio_input_bit_get(ID2_GPIO, ID2_PIN);//PA0
	if (i == 1)
	{
		g_u32ID |= GYRO_ANGLE_ENABLED;
	}
	//ID3代表433无线模块，低电平表示具有此功能
	i = gpio_input_bit_get(ID3_GPIO, ID3_PIN);//PC3
	if (i == 0)
	{
		g_u32ID |= WM433_WIRELESS_ENABLED;
	}
}

u32 Get_DevID(void)
{
	return g_u32ID;
}

/* ########################### Systick Configuration ######################### */
void systick_config(void)
{
    /* setup systick timer for 1000Hz interrupts */
    if (SysTick_Config(SystemCoreClock / OS_TICKS_PER_SEC))
	{
        /* capture error */
        while (1)
		{
			Error_Handler();
        }
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, (uint32_t)0U);
}

void HAL_Init(void)
{
  /* Set Interrupt Group Priority */
	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

  /* Use systick as time base source and configure 1ms tick (default clock after Reset is HSI) */
	systick_config();

}
/*******************************************************************************************
* 函数名称：TaskStart(void *pdata)
* 功能描述：OS系统运行的第一个任务，负责外设初始化及其它任务的创建
* 入口参数：*pdata
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void TaskStart(void *pdata)
{
	/* Configure the Vector Table location add offset address */
	nvic_vector_table_set(NVIC_VECTTAB_FLASH, VECT_TAB_OFFSET);			//FLASH启动地址:0x08004000
	/* 初始化ID识别外设 */
	DevID_Init();				//根据ID设置不同功能使能状态
	/* 配置Systick周期1ms */
	HAL_Init();

	//使能人员定位功能
	if ((g_u32ID & UWB_PERSONNEL_ENABLED) == UWB_PERSONNEL_ENABLED)
	{
		Dw1000Init();			//DW1000模块
		Reset_DW1000();			//DW1000复位
		SpiBusInit();			//DW1000外设SPI接口
		/********
		当芯片内部的CLKPLL锁定后，SPI的时钟速率最大支持20MHz，否则SPI的时钟速率最大为3MHz，
		所以初始化配置时时钟速率不超过3MHz
		*********/
		SPI_DW1000_SetRateLow();		//降低SPI时钟频率72/32MHz（Init状态下，SPICLK不超过3MHz）
		if (DWT_Initialise(DWT_LOADUCODE) == DWT_ERROR)		//初始化DW1000失败
		{
//			while (1)				//是否需要死等？
//			{
//				Error_Handler();
//			}
		}
	}

	OSTimeDly(100);									//延时0.1秒...
//	if ((g_u32ID & BEEP_ALARM_ENABLED) == BEEP_ALARM_ENABLED)		//使能声光报警器功能
//	{
		/*蜂鸣器发声控制处理任务*/
		OSTaskCreateExt(AppTaskBeep,							/* 启动任务函数指针 */
					   (void *)0,								/* 传递给任务的参数 */
					   (OS_STK *)&AppTaskBeepStk[APP_BEEP_TASK_STK_SIZE - 1], /* 指向任务栈栈顶的指针 */
					   APP_TASK_BEEP_PRIO,						/* 任务的优先级，必须唯一，数字越低优先级越高 */
					   APP_TASK_BEEP_PRIO,						/* 任务ID，一般和任务优先级相同 */
					   (OS_STK *)&AppTaskBeepStk[0],			/* 指向任务栈栈底的指针。OS_STK_GROWTH 决定堆栈增长方向 */
					   APP_BEEP_TASK_STK_SIZE, 					/* 任务栈大小 */
					   (void *)0,								/* 一块用户内存区的指针，用于任务控制块TCB的扩展功能 ，填0即可 */
					   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); 

		/*LED发光控制处理任务*/
		OSTaskCreateExt(AppTaskLight,							/* 启动任务函数指针 */
						(void *)0,								/* 传递给任务的参数 */
						(OS_STK *)&AppTaskKLightStk[APP_LIGHT_TASK_STK_SIZE - 1], /* 指向任务栈栈顶的指针 */
						APP_TASK_LIGHT_PRIO,					/* 任务的优先级，必须唯一，数字越低优先级越高 */
						APP_TASK_LIGHT_PRIO,					/* 任务ID，一般和任务优先级相同 */
						(OS_STK *)&AppTaskKLightStk[0],			/* 指向任务栈栈底的指针。OS_STK_GROWTH 决定堆栈增长方向 */
						APP_LIGHT_TASK_STK_SIZE, 				/* 任务栈大小 */
						(void *)0,								/* 一块用户内存区的指针，用于任务控制块TCB的扩展功能 ，填0即可 */
						OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); 
//	}
	
	//can收发任务创建在LED驱动和蜂鸣器驱动之后，防止启动过程中收到控制命令后LED_ON打开锁存上一次的状态。
	CanGpioInit();									//can gpio 初始化
	CAN_Config(200);								//can 通信速率配置200k
	CanAppInit();         							//建立can收发任务
	
#if DECA_WORK_MODE == UWB_LISTEN_MODE
	OSTaskCreateExt(UwbReceiveTask,
					(void *)0,
					(OS_STK *)&UWB_RxStk[UWBRX_TASK_STK_SIZE - 1],
					UWBRX_TASK_PRIO,
					UWBRX_TASK_PRIO,
					(OS_STK *)&UWB_RxStk[0],
					UWBRX_TASK_STK_SIZE,
					(void *)0,
					OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

#elif DECA_WORK_MODE == UWB_ALARM_MODE
	OSTimeDly(200);									//延时0.5秒...
	/*UWB定位模块数据处理任务*/
	if ((g_u32ID & UWB_PERSONNEL_ENABLED) == UWB_PERSONNEL_ENABLED)		//使能人员定位功能
	{
		OSTaskCreateExt(UwbAnchorIrqTask,
						(void *)0,
						(OS_STK *)&UWB_AnchorStk[UWB_ANCHOR_TASK_STK_SIZE - 1],
						UWB_ANCHOR_TASK_PRIO,
						UWB_ANCHOR_TASK_PRIO,
						(OS_STK *)&UWB_AnchorStk[0],
						UWB_ANCHOR_TASK_STK_SIZE,
						(void *)0,
						OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	}
#endif /*DECA_WORK_MODE == UWB_LISTEN_MODE*/
	
	// 创建逻辑处理主任务，对无线模块的数据收发以及状态进行管理
	if ((g_u32ID & WM433_WIRELESS_ENABLED) == WM433_WIRELESS_ENABLED)		//使能433无线模块功能
	{
		OSTaskCreate(WL_MNG_Task, 
					(void *)0, 
					&wl_manage_task_stk[UWBRX_TASK_STK_SIZE - 1], 
					WL_MANAGE_TASK_PRIO);
	}

	//传感器状态周期管理任务(单次读取角度四元数周期长，影响周期上报定时时间，单独创建任务管理周期上报角度数据)
	OSTaskCreateExt(AngleSensorMNG_Task, 
					(void *)0, 
					(OS_STK *)&anglesensormng_task_stk[ANGLE_SENSOR_TASK_STK_SIZE - 1], 
					ANGLE_SENSOR_MNG_TASK_PRIO,
					ANGLE_SENSOR_MNG_TASK_PRIO,
					(OS_STK *)&anglesensormng_task_stk[0],
					ANGLE_SENSOR_MNG_TASK_STK_SIZE,
					(void *)0,
					OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK);

	/*角度传感器状态数据采集任务*/
	if ((g_u32ID & GYRO_ANGLE_ENABLED) == GYRO_ANGLE_ENABLED)		//使能角度采集功能
	{
		//角度传感器状态采集及计算主任务，延时时间比较长，任务最后启动
		OSTaskCreateExt(AngleSensor_Task, 
						(void *)0, 
						(OS_STK *)&anglesensor_task_stk[ANGLE_SENSOR_TASK_STK_SIZE - 1], 
						ANGLE_SENSOR_TASK_PRIO,
						ANGLE_SENSOR_TASK_PRIO,
						(OS_STK *)&anglesensor_task_stk[0],
						ANGLE_SENSOR_TASK_STK_SIZE,
						(void *)0,
						OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK);

	}
	/*看门狗任务***/
	#ifdef IWDG_ENABLED
		OSTaskCreateExt(IWDG_Task, 
						(void *)0, 
						&iwdg_task_stk[IWDG_TASK_STK_SIZE - 1], 
						IWDG_TASK_PRIO,
						IWDG_TASK_PRIO,
						&iwdg_task_stk[0],
						IWDG_TASK_STK_SIZE,
						(void *)0,
						(uint16_t)(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));			
	#endif

	OSTaskDel(OS_PRIO_SELF);
}


int main(void)
{
	OSInit();           //初始化OS系统
	/*主任务入口处理任务*/
	OSTaskCreateExt(TaskStart,
					(void *)0,
					(OS_STK *)&TaskStartStk[APPMNG_TASK_STK_SIZE - 1],
					APPMNG_TASK_START_PRIO,
					APPMNG_TASK_START_PRIO,
					(OS_STK *)&TaskStartStk[0],
					APPMNG_TASK_STK_SIZE,
					(void *)0,
					OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	OSStart();         //开始运行OS系统
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
