/**
  ******************************************************************************
  * File Name          : uwb_debug.c
  * Description        : This file provides code for the configuration
  *                      of the anchor instances.
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

/* Includes ------------------------------------------------------------------*/
//#include "uwb_app.h"

#include "uwb_common.h"
#include "uwb_debug.h"

#if UWB_DEBUG_ENABLE == 1

OS_STK      gUwbDebugStk[UWB_DEBUG_STK_SIZE];
GPIO_PinState gUwbIntState;
int           gDebugCount;
uint32_t      gDebugState;
uint32_t      gDebugMask;
uint32_t      gDebugSys;
uint16_t      gDebugCtrl;

uint32_t      gODebugState;
uint32_t      gODebugMask;
uint32_t      gODebugSys;
uint16_t      gODebugCtrl;

/*******************************************************************************************
* 函数名称：UWB_Debug(void *pdata)
* 功能描述：UWB调试任务
* 入口参数：*pdata
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void UwbDebugTask(void *pdata)
{
	int val;
	int cout = 0;
	uint32 sysconfig;
	
	while(1)
	{
		
		gUwbIntState = HAL_GPIO_ReadPin(UWB_IRQ_GPORT, GPIO_PIN_15);
		OSTimeDly(DEBUG_TIME_DELAY);
		
		if(val == gDebugCount)
		{
			cout++;
			if(cout > 2)
			{
				gDebugState = DWT_Read32BitReg(SYS_STATUS_ID);
				gDebugMask = DWT_Read32BitReg(SYS_MASK_ID) ;
				sysconfig = DWT_Read32BitReg(SYS_CFG_ID) ; 
				gDebugSys = SYS_CFG_MASK & sysconfig;
				gDebugCtrl =  DWT_Read16BitOffsetReg(SYS_CTRL_ID, SYS_CTRL_OFFSET);
				
				DWT_Write32BitReg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
				DWT_ForceTRxOff();
				
				DWT_RxReset();
				DWT_SetRxTimeOut(0);												//设定接收超时时间，0--没有超时时间，无限等待
				DWT_RxEnable(DWT_START_RX_IMMEDIATE);				//打开接收
			}
		}
		else
		{
			val = gDebugCount;
			cout = 0;
		}
	}
}


void SetDebugCount(void)
{
	if( gDebugCount == 0)
	{
		gODebugState = DWT_Read32BitReg(SYS_STATUS_ID);
		gODebugMask = DWT_Read32BitReg(SYS_MASK_ID) ;
		gODebugSys = SYS_CFG_MASK & DWT_Read32BitReg(SYS_CFG_ID) ;
		gODebugCtrl =  DWT_Read16BitOffsetReg(SYS_CTRL_ID, SYS_CTRL_OFFSET);
	}
	gDebugCount++;
}

/******************************************************************************
** 函数名称: void CreatUwbDebugTask   (void)
** 功能描述:创建调试程序
** 参数描述：
*******************************************************************************/
void CreatUwbDebugTask(void)
{

	INT8U err;

	err = OSTaskCreateExt((void (*)(void *))UwbDebugTask,
					(void          * )0,
					(OS_STK        * )&gUwbDebugStk[UWB_DEBUG_STK_SIZE - 1],
					(uint8_t         )UWB_DEBUG_TASK_PRIO,
					(uint16_t        )UWB_DEBUG_TASK_PRIO,
					(OS_STK        * )&gUwbDebugStk[0],
					(INT32U          )UWB_DEBUG_STK_SIZE,
					(void          * )0,
					(uint16_t        )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));
	if (err == OS_ERR_NONE)
	{
		OSTaskNameSet(UWB_DEBUG_TASK_PRIO, (uint8_t *)"uwb debug Task", &err);
	}

}
#endif /*UWB_DEBUG_ENABLE == 1 */

