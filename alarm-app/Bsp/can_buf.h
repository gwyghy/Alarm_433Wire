/*
*********************************************************************************************************
*	                                            UWB
*                           Commuication Controller For The Mining Industry
*
*                                  (c) Copyright 1994-2013  HNDZ 
*                       All rights reserved.  Protected by international copyright laws.
*
*
*    File    : can_buf.h
*    Module  : can driver
*    Version : V1.0
*    History :
*   -----------------
*             canµ×²ãÇý¶¯
*              Version  Date           By            Note
*              v1.0     2013-09-09     xxx
*
*********************************************************************************************************
*/

#ifndef __CAN_BUF_H__
#define __CAN_BUF_H__



/*
****************************************************************************************************
*                                             INCLUDES
****************************************************************************************************
*/

#include "l4can.h"                                  /* CAN driver abstraction                   */

#ifndef NO_SYS
#include "uCOS_II.h"
#endif /*NO_SYS*/


#define CAN_BUFFER_SIZE    16

#define CAN_BUFFER_READ_ERR  -3



/*------------------------------------------------------------------------------------------------*/
/*!
* \brief                      CAN BUFFER OBJECT
* \ingroup  UCCAN
*
*           This structure holds the runtime data for the CAN bus management.
*/
/*------------------------------------------------------------------------------------------------*/
typedef struct {
	
	  sCAN_FRAME     	buf[CAN_BUFFER_SIZE];
		uint16_t 		bufRd;
		uint16_t  		bufWr;
volatile int16_t        dataSize;

#ifndef NO_SYS
		OS_EVENT		* dataSem;
#endif /*NO_SYS*/

} sCAN_BUFFER;

/*
****************************************************************************************************
*                                     GLOBAL VARIABLE
****************************************************************************************************
*/


/*
****************************************************************************************************
*                                       FUNCTION PROTOTYPES
****************************************************************************************************
*/
void CanBufferDataInit(sCAN_BUFFER * databuf);
int32_t CanBufferWrite(sCAN_BUFFER * databuf, sCAN_FRAME data);
int32_t CanBufferRead(sCAN_BUFFER * databuf, sCAN_FRAME * data);

#ifndef NO_SYS
int32_t SysCanBufferRead(sCAN_BUFFER * databuf, sCAN_FRAME  * data, uint32_t time);
#endif /*NO_SYS*/


#endif  /* __CAN_BUF_H__ */

