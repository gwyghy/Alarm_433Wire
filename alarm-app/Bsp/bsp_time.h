/********************************Copyright (c)**********************************\
**
**                   (c) Copyright 2023, Main, China, QD.
**                           All Rights Reserved
**
**                           天津华宁电子有限公司
**                           
**
**----------------------------------文件信息------------------------------------
** 文件名称: bsp_time.h
** 创建人员: 郑志春
** 创建日期: 2023-03-07
** 文档描述: 
**
**----------------------------------版本信息------------------------------------
** 版本代号: V0.1
** 版本说明: 初始版本
**
**------------------------------------------------------------------------------
\********************************End of Head************************************/
 
#ifndef __BSP_TIME_H_
#define __BSP_TIME_H_

#include "gd32f30x.h"
#include "stdio.h"

typedef void (*BspTimerCallback)(uint16_t);

void BspTimer_Init(uint32_t timer_periph, uint16_t Period, uint16_t Prescaler, BspTimerCallback Callback);
void BspTimer_Ctrl(uint32_t timer_periph, uint8_t state);

#endif

 
/********************************End of File************************************/
