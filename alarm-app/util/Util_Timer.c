/*
*********************************************************************************************************
*	                                                 V4
*                           Commuication Controller For The Mining Industry
*
*                                  (c) Copyright 1994-2013  HNDZ 
*                       All rights reserved.  Protected by international copyright laws.
*
*
*    File    : BspTimer_Init.c
*    Module  : user
*    Version : V1.0
*    History :
*   -----------------
*              Version  Date           By            Note
*              v1.0     2013-08-15     
*
*********************************************************************************************************
*/
#include "includes.h"
#include "bsp_Time.h"
#include "Util_Timer.h"

Util_Timer Utiltimer[TIMEREVENT];	//�洢�ķ���ֵ����
Post_Para_UtilTimer postPara;       //��ʱʱ�䵽���͸��ص������Ĳ���
OS_EVENT *UtilTimerTaskSendSem;
/*******************************************************************************************
**�������ƣ�UtilTimerCallback
**�������ã���ʱ������ص�����
**����������value
**�����������
**ע�������
*******************************************************************************************/
void UtilTimerCallback(uint16_t value)
{
	uint8_t i, j = 0;
	for(i = 0;i < TIMEREVENT; i++)//2023/03/24�޸�Ϊi < TIMEREVENT;��ֹTIMEREVENT�����ı䵼������Խ��
	{
		if(Utiltimer[i].mode == TIMER_START && Utiltimer[i].timer > 0)
		{
			Utiltimer[i].timer--;
			if(Utiltimer[i].timer == 0)
			{
				if(Utiltimer[i].count == 0XFF)//ѭ��ģʽ
				{
					Utiltimer[i].timer = Utiltimer[i].backTimer;
				}
				else if(Utiltimer[i].count != 0)//����ģʽ����ʱ����ֹͣ
				{
					Utiltimer[i].timer = Utiltimer[i].backTimer;
					Utiltimer[i].count--;
				}
				if(Utiltimer[i].count == 0)
                {
					Utiltimer[i].mode = TIMER_STOP;
                }
				
				postPara.id[i] = 1;//��ʱ����־λ
				postPara.value[i] = value;
				j++;
			}
		}
	}
	if(j)
	{
		OSSemPost(UtilTimerTaskSendSem);//���ͼ�ʱ���ź�
	}
}

/*******************************************************************************************
**�������ƣ�UtilTimerInit
**�������ã�util�������ݳ�ʼ��
**����������TIMx ����ʱ��
**�����������
**ע������뽫�˺�������main��ʼ������������ִ�У���֤addTimerEvent�ڴ˺�����ִ��
*******************************************************************************************/
void UtilTimerInit(uint32_t timer_periph)
{
	memset(Utiltimer, 0, sizeof(Utiltimer));
	memset(&postPara, 0, sizeof(Post_Para_UtilTimer));
	BspTimer_Init(timer_periph, 1000, 72, UtilTimerCallback);  //1ms
	BspTimer_Ctrl(timer_periph, DISABLE);
}

/*******************************************************************************************
**�������ƣ�addTimerEvent
**�������ã�������ʱ���¼�
**����������internal_value����ֵ��count��= 0xFFΪ�Զ�����ģʽ��=������ѭ������ֵ��Callback����ʱ�ص�����
**����������ɹ�����i��ʧ�ܷ���0xFF
**ע�������
*******************************************************************************************/
uint16_t addTimerEvent(uint16_t internal_value, uint8_t count, BspTimerCallback Callback)
{
	uint8_t i;
	for(i = 0; i < TIMEREVENT; i++)
	{
		if(Utiltimer[i].mode > TIMER_NOUSE)
			continue;
		
		Utiltimer[i].mode = TIMER_STOP;
		Utiltimer[i].timer = internal_value;
		Utiltimer[i].count = count;
		Utiltimer[i].backTimer = internal_value;
		Utiltimer[i].Callback = Callback;
		
		return i;		
	}
	return 0xff;
}

/*******************************************************************************************
**�������ƣ�startTimerEvent
**�������ã���ʱ�����¼��ÿ�ʼ��־
**����������id ����ʱ��ID
**�����������
**ע�������
*******************************************************************************************/
void startTimerEvent(uint16_t id)
{
	#if OS_CRITICAL_METHOD == 3u                               /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr = 0u;
	#endif
	OS_ENTER_CRITICAL();
	Utiltimer[id].timer = Utiltimer[id].backTimer;
	Utiltimer[id].mode = TIMER_START;
	OS_EXIT_CRITICAL();
}
/*******************************************************************************************
**�������ƣ�stopTimerEvent
**�������ã���ʱ�����¼���ֹͣ��־
**����������id ����ʱ��ID
**�����������
**ע�������
*******************************************************************************************/
void stopTimerEvent(uint16_t id)
{
	#if OS_CRITICAL_METHOD == 3u                               /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr = 0u;
	#endif
	OS_ENTER_CRITICAL();
	Utiltimer[id].mode = TIMER_STOP;
	OS_EXIT_CRITICAL();
}

/*******************************************************************************************
**�������ƣ�deleteTimerEvent
**�������ã���ʱ����ɾ���¼�
**����������id ����ʱ��ID
**�����������
**ע�������
*******************************************************************************************/
void deleteTimerEvent(uint16_t id)
{
	#if OS_CRITICAL_METHOD == 3u                               /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr = 0u;
	#endif
	OS_ENTER_CRITICAL();
	Utiltimer[id].timer = 0;
	Utiltimer[id].mode = 0;
	Utiltimer[id].count = 0;
	Utiltimer[id].backTimer = 0;
	OS_EXIT_CRITICAL();
}

/*******************************************************************************************
**�������ƣ�Util_Timer_Task
**�������ã���ʱ������
**������������
**�����������
**ע�������
*******************************************************************************************/
void Util_Timer_Task(void *p_arg) 
{
	u8 err;
	uint16_t i = 0;
    UtilTimerTaskSendSem = OSSemCreate(0);
    BspTimer_Ctrl(TIMER1, ENABLE);
	while (1) 
	{	
        OSSemPend(UtilTimerTaskSendSem, 0, &err);
		if(err == OS_ERR_NONE)
		{
			for(i = 0; i < TIMEREVENT; i++)
			{
				if(postPara.id[i] == 1)
				{
					postPara.id[i] = 0;//�����־λ
					if(Utiltimer[i].Callback != NULL)
                    {
						(*Utiltimer[i].Callback)(postPara.value[i]);//ִ�лص�����
                    }
				}
			}
		}
	}
}

