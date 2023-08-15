 /*
*********************************************************************************************************
*	                                           UWB
*                           Commuication Controller For The Mining Industry
*
*                                  (c) Copyright 1994-2013  HNDZ 
*                       All rights reserved.  Protected by international copyright laws.
*
*
*    File    : dw1000_bus.c
*    Module  : dw1000 bus
*    Version : V1.0
*    History :
*   -----------------
*             dw1000驱动
*              Version  Date           By            Note
*              v1.0     2013-09-09     xxx
*
*********************************************************************************************************
*/
#include "dw1000_bus.h"
#include <string.h>


OS_EVENT *gDw1000Sem;


/*数据速率6.8M*/
//static DWT_Config_s gDw1000Config = {
//    2,               						/* Channel number. */
//    DWT_PRF_64M,     						/* Pulse repetition frequency. */
//    DWT_PLEN_128,   	 						/* Preamble length. Used in TX only. */
//    DWT_PAC8,        						/* Preamble acquisition chunk size. Used in RX only. */
//    9,               						/* TX preamble code. Used in TX only. */
//    9,               						/* RX preamble code. Used in RX only. */
//    1,               						/* 0 to use standard SFD, 1 to use non-standard SFD. */
//    DWT_BR_6M8,      						/* Data rate. */
//    DWT_PHRMODE_EXT, 						/* PHY header mode. */
//    (1025 + 64 - 8)  						/* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
//};

//static DWT_Config_s gDw1000Config = {
//    5,               						/* Channel number. */
//    DWT_PRF_64M,     						/* Pulse repetition frequency. */
//    DWT_PLEN_64,   	 						/* Preamble length. Used in TX only. */
//    DWT_PAC8,        						/* Preamble acquisition chunk size. Used in RX only. */
//    9,               						/* TX preamble code. Used in TX only. */
//    9,               						/* RX preamble code. Used in RX only. */
//    1,               						/* 0 to use standard SFD, 1 to use non-standard SFD. */
//    DWT_BR_6M8,      						/* Data rate. */
//    DWT_PHRMODE_EXT, 						/* PHY header mode. */
//    (63 + 64 - 8)  						/* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
//};
DWT_Config_s gDw1000Config = {
    5,               						/* Channel number. */
    DWT_PRF_64M,     						/* Pulse repetition frequency. */
    DWT_PLEN_128,   	 					/* Preamble length. Used in TX only. */
    DWT_PAC8,        						/* Preamble acquisition chunk size. Used in RX only. */
    9,               						/* TX preamble code. Used in TX only. */
    9,               						/* RX preamble code. Used in RX only. */
    1,               						/* 0 to use standard SFD, 1 to use non-standard SFD. */
    DWT_BR_6M8,      						/* Data rate. */
    DWT_PHRMODE_EXT, 						/* PHY header mode. */
    (129 + 64 - 8)  						/* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
//	 (2025 + 64 - 8) 
};

/*DW1000发送配置****************/

DWT_TxConfig_s gDwTxConfig =
{
	TC_PGDELAY_CH5,    /*发送脉宽*/
	0x1F1F1F1F         /*发送功率调整*/
//	0x25456585
//	0xC0C0C0C0
};

/*******************************************************************************************
* 函数名称：void DW1000SemInit(void)
* 功能描述：DW1000 数据初始化
* 入口参数：无
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void DW1000SemInit(void)
{
	gDw1000Sem = OSSemCreate(0);
}

/*******************************************************************************************
* 函数名称：void Dw1000InitConfig(sDW100ConfigPara * config)
* 功能描述：DW1000 初始化配置
* 入口参数：配置参数
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void Dw1000InitConfig(sDW100ConfigPara * config)
{
	SPI_DW1000_SetRateHigh();								//恢复SPI时钟频率为72/4MHz（IDLE状态下，SPICLK不超过20MHz）
	DWT_Configure(&gDw1000Config);							//配置通信参数

	DWT_SetRxAntennaDelay(config->rxAntDelay);				//设置接收天线延迟
	DWT_SetTxAntennaDelay(config->txAntDelay);				//设置发射天线延迟
	
	DWT_SetPanID(config->panId);							//设置帧过滤PANID
	DWT_SetEUI((uint8_t *)&(config->eui));					//设置帧过滤64位设备地址
}


/*******************************************************************************************
* 函数名称：GetTxTimeStamp_u64
* 功能描述：应用层要用到的通用函数集合
* 入口参数：配置参数
* 出口参数：无
* 使用说明：无
********************************************************************************************/
uint64_t GetTxTimeStamp_u64(void)
{
	uint8_t ts_tab[5];
	uint64_t ts = 0;
	int i;
	DWT_ReadTxTimestamp(ts_tab);
	for (i = 4; i >= 0; i--)
	{
		ts <<= 8;
		ts |= ts_tab[i];
	}
	return ts;
}


uint64_t GetRxTimeStamp_u64(void)
{
	uint8 ts_tab[5];
	uint64_t ts = 0;
	int i;
	DWT_ReadRxTimeStamp(ts_tab);
	for (i = 4; i >= 0; i--)
	{
		ts <<= 8;
		ts |= ts_tab[i];
	}
    return ts;
}


uint64_t GetSysTimeStamp_u64(void)
{
	uint8_t ts_tab[5];
	uint64_t ts = 0;
	int i;
	DWT_ReadSysTime(ts_tab);
	for (i = 4; i >= 0; i--)
	{
		ts <<= 8;
		ts |= ts_tab[i];
	}
    return ts;
}


void DwtWriteTxData(uint8_t * data, uint16_t len)
{
	DWT_WriteTxData(len, data, 0);									//写入发送数据
	DWT_WriteTxfCtrl(len, 0, 1);									  //设定发送长度
}

/*******************************************************************************************
* 函数名称：HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
* 功能描述：GPIO外部中断回调函数
* 入口参数：GPIO_Pin
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void HAL_GPIO_EXTI_Callback(exti_line_enum extiline)
{
//	port_DisableEXT_IRQ();
//	OSSemPost(gDw1000Sem);
	if(UWB_EXTI_LINE == extiline)			//UWB_IRQ_PIN中断
	{
		dwt_isr();
	}
}

void Dw1000ProcPend(uint16_t timeout, INT8U *perr)
{
	OSSemPend(gDw1000Sem, timeout, perr);
}

void DWT_SetSendInterrupt(uint8 enable)
{
	;
}

void Dw1000Init(void)
{
	DW1000SemInit();
	DW1000GpioInit();
	Reset_DW1000();
}
