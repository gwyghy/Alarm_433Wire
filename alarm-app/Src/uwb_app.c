/**
  ******************************************************************************
  * File Name          : anchor.c
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
#include "uwb_app.h"
#include "dw1000_bus.h"

extern DWT_LocalData_s *pdw1000local;
sAncInfo   gAncInfo;
uint32_t   gUwbStateReg;
uint8_t 	 gUwbRxBuff[DW1000_RX_LEN];
uint32_t   gTraceFrame;

static sDW100ConfigPara gDwUserConfig=
{
	RX_ANT_DLY,
	TX_ANT_DLY,
	PAN_ID,
	0
	
};

__inline void SetAncLocationStep(ANC_LOCATION_STEP step)
{
	gAncInfo.step = step;
}

__inline void SetAncLocationState(ANC_LOCATION_STATE state)
{
	gAncInfo.state = state;
}

__inline void SetAncLocationErr(ANC_ERROR err)
{
	gAncInfo.err = err;
}










void UwbRxDataProcess(uint16_t len)
{
	sCAN_FRAME 		tmp;
	int16_t 			i;
	
	memset(&tmp, 0x00, sizeof(tmp));
	tmp.Stdid = gTraceFrame++;
	for(i = 0; i < (len/8); i ++)
	{
		memcpy(tmp.Data, gUwbRxBuff+(i *8), 8);
		tmp.DLC = 8;
		CanBufferWrite(&gUwbCanTxBuf, tmp);
	}
	if(len%8)
	{
		memcpy(tmp.Data, gUwbRxBuff+(i *8), len%8);
		tmp.DLC = len%8;
		CanBufferWrite(&gUwbCanTxBuf, tmp);
	}
}



/*******************************************************************************************
* 函数名称：void UwbReceiveTask(void *pdata)
* 功能描述：UWB数据接收处理任务
* 入口参数：*pdata
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void UwbReceiveTask(void *pdata)
{
	INT8U 		err;
	uint16_t  frameLen;
	uint16_t  frameCotrl;	
	
//	WaitUwbStartSem();   //等待配置信息
	gDwUserConfig.eui = pdw1000local->partID;
//	DwtInitConfig();
  Dw1000InitConfig(&gDwUserConfig);
	DWT_SetRxTimeOut(0);												//设定接收超时时间，0--没有超时时间，无限等待

	DWT_SetInterrupt(DWT_INT_ALL, ENABLE);			//开启中断
//	DWT_RxEnable(DWT_START_RX_IMMEDIATE);				//打开接收
	for(;;)
	{
//		SetAncLocationStep(ANC_INIT_SATA);


		DWT_RxEnable(DWT_START_RX_IMMEDIATE);				//打开接收

		
		Dw1000ProcPend(0, &err);
		gUwbStateReg = DWT_Read32BitReg(SYS_STATUS_ID);
		if (gUwbStateReg & SYS_STATUS_RXFCG)	//成功接收
		{
			DWT_Write32BitReg(SYS_STATUS_ID, SYS_STATUS_RXFCG);		//清除标志位
			
			frameLen = DWT_Read32BitReg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;	//获得接收数据长度			
			if(frameLen <= DW1000_RX_LEN)
			{
				DWT_ReadRxData(gUwbRxBuff,  frameLen, 0);														//读取接收数据
				frameCotrl = *((uint16_t *)gUwbRxBuff);
				
				if(frameCotrl & FRAME_CTRL_BYTE_ADDR_LEN_BIT)  										//判断使用的通讯地址长度（16Bit或者32Bit）
				{
						//32Bit地址，不做处理，丢弃
				}
				else
				{
					UwbRxDataProcess(frameLen);//数据帧处理
				}
			}
    }
		else
		{
			DWT_Write32BitReg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
			DWT_RxReset();
		}
  }
}


