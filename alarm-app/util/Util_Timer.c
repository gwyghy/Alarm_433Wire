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

Util_Timer Utiltimer[TIMEREVENT];	//存储的返回值队列
Post_Para_UtilTimer postPara;       //定时时间到后发送给回调函数的参数
OS_EVENT *UtilTimerTaskSendSem;
/*******************************************************************************************
**函数名称：UtilTimerCallback
**函数作用：定时器处理回调函数
**函数参数：value
**函数输出：无
**注意事项：无
*******************************************************************************************/
void UtilTimerCallback(uint16_t value)
{
	uint8_t i, j = 0;
	for(i = 0;i < TIMEREVENT; i++)//2023/03/24修改为i < TIMEREVENT;防止TIMEREVENT参数改变导致数组越界
	{
		if(Utiltimer[i].mode == TIMER_START && Utiltimer[i].timer > 0)
		{
			Utiltimer[i].timer--;
			if(Utiltimer[i].timer == 0)
			{
				if(Utiltimer[i].count == 0XFF)//循环模式
				{
					Utiltimer[i].timer = Utiltimer[i].backTimer;
				}
				else if(Utiltimer[i].count != 0)//单次模式，计时到即停止
				{
					Utiltimer[i].timer = Utiltimer[i].backTimer;
					Utiltimer[i].count--;
				}
				if(Utiltimer[i].count == 0)
                {
					Utiltimer[i].mode = TIMER_STOP;
                }
				
				postPara.id[i] = 1;//计时到标志位
				postPara.value[i] = value;
				j++;
			}
		}
	}
	if(j)
	{
		OSSemPost(UtilTimerTaskSendSem);//发送计时到信号
	}
}

/*******************************************************************************************
**函数名称：UtilTimerInit
**函数作用：util工具数据初始化
**函数参数：TIMx ：定时器
**函数输出：无
**注意事项：请将此函数置于main初始化函数中率先执行，或保证addTimerEvent在此函数后执行
*******************************************************************************************/
void UtilTimerInit(uint32_t timer_periph)
{
	memset(Utiltimer, 0, sizeof(Utiltimer));
	memset(&postPara, 0, sizeof(Post_Para_UtilTimer));
	BspTimer_Init(timer_periph, 1000, 72, UtilTimerCallback);  //1ms
	BspTimer_Ctrl(timer_periph, DISABLE);
}

/*******************************************************************************************
**函数名称：addTimerEvent
**函数作用：新增定时器事件
**函数参数：internal_value：初值，count：= 0xFF为自动重载模式，=其它：循环周期值，Callback：超时回调函数
**函数输出：成功返回i，失败返回0xFF
**注意事项：无
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
**函数名称：startTimerEvent
**函数作用：定时器的事件置开始标志
**函数参数：id ：定时器ID
**函数输出：无
**注意事项：无
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
**函数名称：stopTimerEvent
**函数作用：定时器的事件置停止标志
**函数参数：id ：定时器ID
**函数输出：无
**注意事项：无
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
**函数名称：deleteTimerEvent
**函数作用：定时器的删除事件
**函数参数：id ：定时器ID
**函数输出：无
**注意事项：无
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
**函数名称：Util_Timer_Task
**函数作用：定时器任务
**函数参数：无
**函数输出：无
**注意事项：无
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
					postPara.id[i] = 0;//清除标志位
					if(Utiltimer[i].Callback != NULL)
                    {
						(*Utiltimer[i].Callback)(postPara.value[i]);//执行回调函数
                    }
				}
			}
		}
	}
}

