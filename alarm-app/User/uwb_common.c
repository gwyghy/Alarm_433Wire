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
#include "uwb_common.h"


void SetFirstAddr(uint8_t * dest, uint16_t addr)
{
	memcpy(dest + FIRST_ADDR_OFF, (void *)&addr, 2);
}


void SetSecondAddr(uint8_t * dest, uint16_t addr)
{
	memcpy(dest + SECOND_ADDR_OFF, (void *)&addr, 2);
}


/*******************************************************************************************
* 函数名称：FinalMsgGetTS()
* 功能描述：
* 入口参数：
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void FinalMsgGetTS_64(const uint8_t *ts_field, uint64_t *ts)
{
	uint32_t i;
	uint64_t tmp = 0;
	*ts = 0;
	for (i = 0; i < FINAL_MSG_TS_LEN; i++)
	{
		tmp = ts_field[i];
		*ts += tmp << (i * 8);
	}
}

/*******************************************************************************************
* 函数名称：FinalMsgGetTS()
* 功能描述：
* 入口参数：
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void FinalMsgSetTS(uint8 *ts_field, uint64_t ts)
{
	int i;
	for (i = 0; i < FINAL_MSG_TS_LEN; i++)
	{
		ts_field[i] = (uint8) ts;
		ts >>= 8;
	}
}




