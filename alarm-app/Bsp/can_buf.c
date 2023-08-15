 /*
*********************************************************************************************************
*	                                           UWB
*                           Commuication Controller For The Mining Industry
*
*                                  (c) Copyright 1994-2013  HNDZ 
*                       All rights reserved.  Protected by international copyright laws.
*
*
*    File    : can_buf.c
*    Module  : can driver
*    Version : V1.0
*    History :
*   -----------------
*             can底层驱动
*              Version  Date           By            Note
*              v1.0     2013-09-09     xxx
*
*********************************************************************************************************
*/
#include "can_buf.h"
#include <string.h>


/******************************************************************************
** 函数名称: void CanBusDataInit(uint16_t canid)
** 功能描述: 初始化设备所需的数据结构
** 参数描述：can底层驱动初始化
*******************************************************************************/
void CanBufferDataInit(sCAN_BUFFER * databuf)
{
	memset(databuf, 0x00, sizeof(sCAN_BUFFER));	  //init
#ifndef NO_SYS
	databuf->dataSem = OSSemCreate(0);
#endif  /*NO_SYS*/

}
/******************************************************************************
** 函数名称: int32_t CanBufferWrite(sCAN_BUFFER * databuf, sCAN_FRAME data)
** 功能描述: 根据设备parameter初始化设备
** 参数描述：can底层驱动初始化
*******************************************************************************/
int32_t CanBufferWrite(sCAN_BUFFER *databuf, sCAN_FRAME data)
{
	int32_t err = 0;
	
	if(databuf->dataSize >= CAN_BUFFER_SIZE) 
		return -1;

	databuf->dataSize++;
	databuf->buf[databuf->bufWr] = data;
	databuf->bufWr++;
	databuf->bufWr %= CAN_BUFFER_SIZE;
		
//		if(databuf->bufWr == databuf->bufRd)
//		{
//			err = -1;
//		}
#ifndef NO_SYS		
	OSSemPost(databuf->dataSem);
#endif

	return err;
}

/******************************************************************************
** 函数名称: int32_t CanBufferRead(sCAN_BUFFER * databuf, sCAN_FRAME * data)
** 功能描述: 读取can数据发送缓存，阻塞读取
** 参数描述：can底层驱动初始化
*******************************************************************************/
int32_t CanBufferRead(sCAN_BUFFER * databuf, sCAN_FRAME *data)
{
	if(databuf->dataSize <= 0)
		return -1;
//	if(databuf->bufWr == databuf->bufRd)  return -1;
	
	memcpy(data, &databuf->buf[databuf->bufRd], sizeof(sCAN_FRAME));
	databuf->dataSize--;
	databuf->bufRd++;
	databuf->bufRd %= CAN_BUFFER_SIZE;

	return 0;
}

#ifndef NO_SYS
int32_t SysCanBufferRead(sCAN_BUFFER *databuf, sCAN_FRAME *data, uint32_t time)
{
	INT8U err;

	OSSemPend(databuf->dataSem, time, &err);
	if(err == OS_ERR_TIMEOUT)
	{
	 	return OS_ERR_TIMEOUT;
	}
	else if(err == OS_ERR_NONE)
	{
		memcpy(data, &databuf->buf[databuf->bufRd], sizeof(sCAN_FRAME));
		databuf->bufRd++;
		databuf->bufRd %= CAN_BUFFER_SIZE;
		return OS_ERR_NONE;
	}
	else
	{
		return CAN_BUFFER_READ_ERR;							//error
	}

}

#endif  /*NO_SYS*/


