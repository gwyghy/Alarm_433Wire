/*
*********************************************************************************************************
*	                                            UWB
*                           Commuication Controller For The Mining Industry
*
*                                  (c) Copyright 1994-2013  HNDZ 
*                       All rights reserved.  Protected by international copyright laws.
*
*
*    File    : dev_bus.h
*    Module  : dev driver
*    Version : V1.0
*    History :
*   -----------------
*             devµ×²ãÇý¶¯
*              Version  Date           By            Note
*              v1.0     2013-09-09     xxx
*
*********************************************************************************************************
*/

#ifndef __DEV_BUS_H__
#define __DEV_BUS_H__



/*
****************************************************************************************************
*                                             INCLUDES
****************************************************************************************************
*/

#include "l4can.h"                                  /* CAN driver abstraction                   */

#ifndef NO_SYS
#include "uCOS_II.h"
#endif /*NO_SYS*/

#define DEV_NAME  SPI
#define DEVBUS_TX_QSIZE       10
#define DEVBUS_RX_QSIZE       10


enum BUS_READ_STATE{
	 BUS_RX_UNKNOWN_ERR = -11,
	 BUS_RX_TIMEOVER = -10,
	 BUS_RX_NORMAL = 0
};



#define DEV_BUS_PRO(dev_name, data_type)  typedef struct { \
																						uint16_t 					dev;\
																						uint16_t 					rxTimeout;\
																						uint16_t 					txTimeout;\
																						data_type     		bufTx[dev_name##_TX_QSIZE];\
																						uint16_t 					bufTxRd;\
																						uint16_t  				bufTxWr;\
																						data_type     		bufRx[dev_name##_RX_QSIZE]; \
																						uint16_t				  bufRxRd;\
																						uint16_t 					bufRxWr;\
																						OS_EVENT			*   recSem;\
																						OS_EVENT			*   sendEndSem;		\
																								\
																				} s##dev_name##BUS_DATA;\
																				void dev_name##DataInit(s##dev_name##BUS_DATA * data)\
																				{\
																					memset(data,  0x00, sizeof(s##dev_name##BUS_DATA));	\
																					data->recSem = OSSemCreate(0);\
																					data->sendEndSem = OSSemCreate(0);\
																				}\
																				int16_t dev_name##BusRead(s##dev_name##BUS_DATA * data ,void *buffer, uint32_t size) \
																				{\
																					\
																					INT8U err;\
																					OSSemPend(data->recSem, data->rxTimeout, &err);\
																					if(err == OS_ERR_TIMEOUT)\
																					{\
																						return BUS_RX_TIMEOVER;\
																					}\
																					else if(err == OS_ERR_NONE) \
																					{\
																						memcpy(buffer, (void *)&data->bufRx[data->bufRxRd], size*sizeof(data_type));\
																						data->bufRxRd += size;\
																						if(data->bufRxRd > dev_name##_RX_QSIZE - 1)\
																						{\
																							data->bufRxRd  = 0;	\
																						} \
																						 return  BUS_RX_NORMAL;\
																					}\
																					else\
																					{\
																						return  BUS_RX_UNKNOWN_ERR;	\
																					}\
																				\
																				}




#endif  /* #ifndef __DEV_BUS_H__ */

