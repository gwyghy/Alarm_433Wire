/*
*********************************************************************************************************
*	                                            V4
*                           Commuication Controller For The Mining Industry
*
*                                  (c) Copyright 1994-2013  HNDZ 
*                       All rights reserved.  Protected by international copyright laws.
*
*
*    File    : Bsp_Time.h
*    Module  : user
*    Version : V1.0
*    History :
*   -----------------
*              Version  Date           By            Note
*              v1.0         
*
*********************************************************************************************************
*/

#ifndef __UTIL_TIME_H__
#define __UTIL_TIME_H__
#include  "bsp_time.h"




#define TIMEREVENT   1      //定时器事件最大值


typedef struct 
{
	uint16_t            timer;      //初值
	uint8_t             mode;       //模式
	uint8_t             count;      //周期值
	uint16_t            backTimer;  //自动重载值
	BspTimerCallback    Callback;   //回调函数
} Util_Timer;

typedef struct 
{
	uint8_t id[TIMEREVENT];     //定时时间到标志位
	uint16_t value[TIMEREVENT]; //定时时间到后发送给回调函数的参数
} Post_Para_UtilTimer;


enum
{
	TIMER_NOUSE = 0,
	TIMER_STOP  = 1,
	TIMER_START = 2,
};



void UtilTimerInit(uint32_t timer_periph);
uint16_t addTimerEvent(uint16_t internal_value, uint8_t count, BspTimerCallback  Callback);
void startTimerEvent(uint16_t id);
void stopTimerEvent(uint16_t id);
void deleteTimerEvent(uint16_t id);
#endif


