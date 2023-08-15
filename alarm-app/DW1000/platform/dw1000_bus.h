/*
*********************************************************************************************************
*	                                            UWB
*                           Commuication Controller For The Mining Industry
*
*                                  (c) Copyright 1994-2013  HNDZ 
*                       All rights reserved.  Protected by international copyright laws.
*
*
*    File    : dw1000_bus.h
*    Module  : dw1000 bus
*    Version : V1.0
*    History :
*   -----------------
*              dw1000Çý¶¯
*              Version  Date           By            Note
*              v1.0     2013-09-09     xxx
*
*********************************************************************************************************
*/

#ifndef __DW1000_BUS_H__
#define __DW1000_BUS_H__



/*
****************************************************************************************************
*                                             INCLUDES
****************************************************************************************************
*/

#include "port.h"                                


typedef struct {
	uint16_t      rxAntDelay;
	uint16_t      txAntDelay; 	
	uint16_t      panId;					//PAN IDÖµ
	uint64_t 			eui;						//EUI
}sDW100ConfigPara;

void DW1000SemInit(void);
//void Dw1000ProcPend(uint16_t timeout, int8_t *perr);
void Dw1000ProcPend(uint16_t timeout, INT8U *perr);
uint64_t GetTxTimeStamp_u64(void);
uint64_t GetRxTimeStamp_u64(void);
uint64_t GetSysTimeStamp_u64(void);
void DwtWriteTxData(uint8_t * data, uint16_t len);
void Dw1000Init(void);

void Dw1000InitConfig(sDW100ConfigPara * config);
void HAL_GPIO_EXTI_Callback(exti_line_enum extiline);
#endif  /* #ifndef __DW1000_BUS_H__ */

