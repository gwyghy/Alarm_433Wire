/**
  ******************************************************************************
  * �ļ�����: main.c
  * ��    ��: 
	* ��ǰ�汾��V1.0
	* ������ڣ�
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

/*���������ջ������256*/
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
OS_STK  iwdg_task_stk[IWDG_TASK_STK_SIZE];						//���������ջ
#endif

OS_STK  wl_manage_task_stk[UWBRX_TASK_STK_SIZE];		//���������ջ

u32 g_u32ID = 0;		//�洢ID������
/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
	OSTimeDly(1000);
}

/********************************************************
* �������ƣ� uint16_t Get_JTAG_ID()
* ���������� ��ȡ��Ƭ���豸����
* ��������� ��
* ��������� ��
* �� �� ֵ�� ����0x041��STϵ�е�Ƭ��
			 ����0x7A3��GDϵ�е�Ƭ��
* ����˵���� ��
* �޸�����   �汾��    �޸���  �޸�����
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
* �������ƣ�DevID_Init
* ������������ʼ��IDʶ��˿�
* �����������
* �����������
* �� �� ֵ����
* ����˵������
* �޸�����   �汾��    �޸���  �޸�����
* ------------------------------------------------------
********************************************************/
void DevID_Init(void)
{
	/*******************************
	IDֵ       ���ù�ϵ
	ID0			����
	ID1			UWB
	ID2         ���GYRO
	ID3         433����ģ��
	*******************************/
//	GPIO_InitTypeDef GPIO_InitStructure;
	u32	i;
	g_u32ID = 0;
	// ����IDʶ��ӿ�
	ID_GPIO_CLK_ENABLE();			   //ʹ������ʹ�ö˿�ʱ��

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
    gpio_init(ID0_GPIO, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, ID0_PIN);		//����Ϊ����ģʽ��ͨ���ⲿ�����õ͸ı�����״̬
	/* configure ID1 GPIO port */ 
    gpio_init(ID1_GPIO, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, ID1_PIN);
	/* configure ID2 GPIO port */ 
    gpio_init(ID2_GPIO, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, ID2_PIN);
	/* configure ID3 GPIO port */ 
    gpio_init(ID3_GPIO, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, ID3_PIN);

#if 0			//IOԤ��ʵ�ʲ�ʹ��
	//LEDָʾ��
	/* enable the LED clock */
    rcu_periph_clock_enable(LED_PORT);
    /* configure LED2 GPIO port */ 
    gpio_init(LED_GPIO, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED_PIN);
    //LEDָʾ������
    gpio_bit_reset(LED_GPIO, LED_PIN);
#endif
	i = 0;
	//ID0�������⣬�ߵ�ƽ��ʾ���д˹���
//	i = gpio_input_bit_get(ID0_GPIO, ID0_PIN);
//	if (i == 1)
//	{
//		g_u32ID |= BEEP_ALARM_ENABLED;
//	}
	//���ⱨ�����ܲ���Ҫ���ã�Ĭ�϶��д˹���
	g_u32ID |= BEEP_ALARM_ENABLED;
	//ID1����UWB���ߵ�ƽ��ʾ���д˹���
	i = gpio_input_bit_get(ID1_GPIO, ID1_PIN);//PA1
	if (i == 1)
	{
		g_u32ID |= UWB_PERSONNEL_ENABLED;
	}
	//ID2������ǣ��ߵ�ƽ��ʾ���д˹���
	i = gpio_input_bit_get(ID2_GPIO, ID2_PIN);//PA0
	if (i == 1)
	{
		g_u32ID |= GYRO_ANGLE_ENABLED;
	}
	//ID3����433����ģ�飬�͵�ƽ��ʾ���д˹���
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
* �������ƣ�TaskStart(void *pdata)
* ����������OSϵͳ���еĵ�һ�����񣬸��������ʼ������������Ĵ���
* ��ڲ�����*pdata
* ���ڲ�������
* ʹ��˵������
********************************************************************************************/
void TaskStart(void *pdata)
{
	/* Configure the Vector Table location add offset address */
	nvic_vector_table_set(NVIC_VECTTAB_FLASH, VECT_TAB_OFFSET);			//FLASH������ַ:0x08004000
	/* ��ʼ��IDʶ������ */
	DevID_Init();				//����ID���ò�ͬ����ʹ��״̬
	/* ����Systick����1ms */
	HAL_Init();

	//ʹ����Ա��λ����
	if ((g_u32ID & UWB_PERSONNEL_ENABLED) == UWB_PERSONNEL_ENABLED)
	{
		Dw1000Init();			//DW1000ģ��
		Reset_DW1000();			//DW1000��λ
		SpiBusInit();			//DW1000����SPI�ӿ�
		/********
		��оƬ�ڲ���CLKPLL������SPI��ʱ���������֧��20MHz������SPI��ʱ���������Ϊ3MHz��
		���Գ�ʼ������ʱʱ�����ʲ�����3MHz
		*********/
		SPI_DW1000_SetRateLow();		//����SPIʱ��Ƶ��72/32MHz��Init״̬�£�SPICLK������3MHz��
		if (DWT_Initialise(DWT_LOADUCODE) == DWT_ERROR)		//��ʼ��DW1000ʧ��
		{
//			while (1)				//�Ƿ���Ҫ���ȣ�
//			{
//				Error_Handler();
//			}
		}
	}

	OSTimeDly(100);									//��ʱ0.1��...
//	if ((g_u32ID & BEEP_ALARM_ENABLED) == BEEP_ALARM_ENABLED)		//ʹ�����ⱨ��������
//	{
		/*�������������ƴ�������*/
		OSTaskCreateExt(AppTaskBeep,							/* ����������ָ�� */
					   (void *)0,								/* ���ݸ�����Ĳ��� */
					   (OS_STK *)&AppTaskBeepStk[APP_BEEP_TASK_STK_SIZE - 1], /* ָ������ջջ����ָ�� */
					   APP_TASK_BEEP_PRIO,						/* ��������ȼ�������Ψһ������Խ�����ȼ�Խ�� */
					   APP_TASK_BEEP_PRIO,						/* ����ID��һ����������ȼ���ͬ */
					   (OS_STK *)&AppTaskBeepStk[0],			/* ָ������ջջ�׵�ָ�롣OS_STK_GROWTH ������ջ�������� */
					   APP_BEEP_TASK_STK_SIZE, 					/* ����ջ��С */
					   (void *)0,								/* һ���û��ڴ�����ָ�룬����������ƿ�TCB����չ���� ����0���� */
					   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); 

		/*LED������ƴ�������*/
		OSTaskCreateExt(AppTaskLight,							/* ����������ָ�� */
						(void *)0,								/* ���ݸ�����Ĳ��� */
						(OS_STK *)&AppTaskKLightStk[APP_LIGHT_TASK_STK_SIZE - 1], /* ָ������ջջ����ָ�� */
						APP_TASK_LIGHT_PRIO,					/* ��������ȼ�������Ψһ������Խ�����ȼ�Խ�� */
						APP_TASK_LIGHT_PRIO,					/* ����ID��һ����������ȼ���ͬ */
						(OS_STK *)&AppTaskKLightStk[0],			/* ָ������ջջ�׵�ָ�롣OS_STK_GROWTH ������ջ�������� */
						APP_LIGHT_TASK_STK_SIZE, 				/* ����ջ��С */
						(void *)0,								/* һ���û��ڴ�����ָ�룬����������ƿ�TCB����չ���� ����0���� */
						OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); 
//	}
	
	//can�շ����񴴽���LED�����ͷ���������֮�󣬷�ֹ�����������յ����������LED_ON��������һ�ε�״̬��
	CanGpioInit();									//can gpio ��ʼ��
	CAN_Config(200);								//can ͨ����������200k
	CanAppInit();         							//����can�շ�����
	
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
	OSTimeDly(200);									//��ʱ0.5��...
	/*UWB��λģ�����ݴ�������*/
	if ((g_u32ID & UWB_PERSONNEL_ENABLED) == UWB_PERSONNEL_ENABLED)		//ʹ����Ա��λ����
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
	
	// �����߼����������񣬶�����ģ��������շ��Լ�״̬���й���
	if ((g_u32ID & WM433_WIRELESS_ENABLED) == WM433_WIRELESS_ENABLED)		//ʹ��433����ģ�鹦��
	{
		OSTaskCreate(WL_MNG_Task, 
					(void *)0, 
					&wl_manage_task_stk[UWBRX_TASK_STK_SIZE - 1], 
					WL_MANAGE_TASK_PRIO);
	}

	//������״̬���ڹ�������(���ζ�ȡ�Ƕ���Ԫ�����ڳ���Ӱ�������ϱ���ʱʱ�䣬��������������������ϱ��Ƕ�����)
	OSTaskCreateExt(AngleSensorMNG_Task, 
					(void *)0, 
					(OS_STK *)&anglesensormng_task_stk[ANGLE_SENSOR_TASK_STK_SIZE - 1], 
					ANGLE_SENSOR_MNG_TASK_PRIO,
					ANGLE_SENSOR_MNG_TASK_PRIO,
					(OS_STK *)&anglesensormng_task_stk[0],
					ANGLE_SENSOR_MNG_TASK_STK_SIZE,
					(void *)0,
					OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK);

	/*�Ƕȴ�����״̬���ݲɼ�����*/
	if ((g_u32ID & GYRO_ANGLE_ENABLED) == GYRO_ANGLE_ENABLED)		//ʹ�ܽǶȲɼ�����
	{
		//�Ƕȴ�����״̬�ɼ���������������ʱʱ��Ƚϳ��������������
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
	/*���Ź�����***/
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
	OSInit();           //��ʼ��OSϵͳ
	/*��������ڴ�������*/
	OSTaskCreateExt(TaskStart,
					(void *)0,
					(OS_STK *)&TaskStartStk[APPMNG_TASK_STK_SIZE - 1],
					APPMNG_TASK_START_PRIO,
					APPMNG_TASK_START_PRIO,
					(OS_STK *)&TaskStartStk[0],
					APPMNG_TASK_STK_SIZE,
					(void *)0,
					OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	OSStart();         //��ʼ����OSϵͳ
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
