/**
  ******************************************************************************
  * File Name          : uwb_beb.c
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
#include "uwb_beb.h"
#include "dw1000_bus.h"
#include "randomapp.h"

uint32_t gListUwbState;
uint16_t gCollsWindow = 1;				//随机回退冲突窗口Cw

#if INVOKED		//声明实际未被调用
uint32_t listen_channel_state(void)
{
	INT8U err;

	DWT_SetRxTimeOut(LISTEN_CHANNEL_TIME);				//设定接收超时时间	
	DWT_SetInterrupt(DWT_INT_ALL, ENABLE);			  //开启中断
	
	DWT_RxEnable(DWT_START_RX_IMMEDIATE);				//打开接收
	
	Dw1000ProcPend(0, &err);
	return DWT_Read32BitReg(SYS_STATUS_ID);
}


void listen_channel_err(void)
{
	;
}


/*******************************************************************************************
* 函数名称：ChannelStatu ListenChannel(void)
* 功能描述：信道监听
* 入口参数：无
* 出口参数：当前信道状态：ChannelFree -- 1，ChannelBusy -- 0
* 使用说明：无
********************************************************************************************/
ChannelStatus ListenChannel(void)
{
	uint16_t rngnum;
	uint8_t listencnt = 0;
	
	while(1)
	{
		gListUwbState = listen_channel_state();
		if ((gListUwbState & SYS_STATUS_RXRFTO) && ((gListUwbState & SYS_STATUS_ALL_RX_GOOD) == 0))				//接收超时，且接收器当前并未正在接收数据帧。表示信道空闲
		{ //信道空闲
			DWT_Write32BitReg(SYS_STATUS_ID, SYS_STATUS_RXRFTO);
			rngnum = RNG_Get_RandomRange(0, gCollsWindow);				//获取随机回退值
			OSTimeDly(rngnum * BACK_OFF_TIME_MS);
			gListUwbState = listen_channel_state();
			
			if ((gListUwbState & SYS_STATUS_RXRFTO) && ((gListUwbState & SYS_STATUS_ALL_RX_GOOD) == 0))			//接收超时，且接收器当前并未正在接收数据帧。表示信道空闲
			{//回退后继续空闲
				DWT_Write32BitReg(SYS_STATUS_ID, SYS_STATUS_RXRFTO);	//清除标志位

				listencnt = 0;							//清除冲突次数
				gCollsWindow /= 2;					//降低竞争窗口大小
				gCollsWindow = gCollsWindow < COLLISION_WINDOW_MIN ? COLLISION_WINDOW_MIN: gCollsWindow;
					
				return CHANNEL_FREE;
			}
		}
		
		DWT_Write32BitReg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR_CLE | SYS_STATUS_ALL_RX_GOOD);
		listencnt++;
		gCollsWindow = (gCollsWindow * 2 >= COLLISION_WINDOW_MAX ? COLLISION_WINDOW_MAX : gCollsWindow * 2);
		rngnum = RNG_Get_RandomRange(0, gCollsWindow);				//获取随机回退值
		OSTimeDly(rngnum * BACK_OFF_TIME_MS);
		
		if(listencnt >= BACK_OFF_COUNT_MAX)					//连续冲突16次
		{
			return CHANNEL_BUSY;
		}
	}
}
#endif

