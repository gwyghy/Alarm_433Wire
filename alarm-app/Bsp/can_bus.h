/*
*********************************************************************************************************
*	                                            UWB
*                           Commuication Controller For The Mining Industry
*
*                                  (c) Copyright 1994-2013  HNDZ 
*                       All rights reserved.  Protected by international copyright laws.
*
*
*    File    : can_bus.h
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

#ifndef __CAN_BUS_H__
#define __CAN_BUS_H__



/*
****************************************************************************************************
*                                             INCLUDES
****************************************************************************************************
*/

#include "l4can.h"                                  /* CAN driver abstraction                   */

#ifndef NO_SYS
#include "uCOS_II.h"
#endif /*NO_SYS*/


#define CANBUS_TX_QSIZE			 10
#define CANBUS_RX_QSIZE			 10


enum CANBUS_READ_STATE{
	 CANBUS_RX_UNKNOWN_ERR = -11,
	 CANBUS_RX_TIMEOVER = -10,
	 CANBUS_RX_NORMAL = 0
};

/*
****************************************************************************************************
*                                              DEFINES
***************************************************************************************************
*/
enum CANBUS_IOCTL_FUNC {
    CANBUS_RESET = 0,
    CANBUS_FLUSH_TX,
    CANBUS_FLUSH_RX,
    CANBUS_SET_TX_TIMEOUT,
    CANBUS_SET_RX_TIMEOUT
};

/*
****************************************************************************************************
*                                           CAN BUS DEVICE
****************************************************************************************************
*/
enum 
{
	STM32_BUS_NODE0 = 0,
	STM32_CAN_N_NODE
};

/*
****************************************************************************************************
*                                         CAN FRAME STRUCT
*
* Description : Structure defines a CAN Frame
*
* Note(s)     : none.
****************************************************************************************************
*/

/*------------------------------------------------------------------------------------------------*/
/*!
* \brief                      CAN BUS CONFIGURATION
* \ingroup  UCCAN
*
*           This structure contains the informations for a bus. A bus represents one
*           interface to the world.
*
* \note     For systems with very limited amount of RAM, this structure can be placed in
*           ROM by declaring (and initializing) a const-variable during compile-time.
*/
/*------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/
/*!
* \brief                      CAN BUS OBJECT
* \ingroup  UCCAN
*
*           This structure holds the runtime data for the CAN bus management.
*/
/*------------------------------------------------------------------------------------------------*/
typedef struct {
    uint16_t 					dev;
    uint16_t 					rxTimeout;
    uint16_t 					txTimeout;
	sCAN_FRAME     				bufTx[CANBUS_TX_QSIZE];
	uint16_t 					bufTxRd;
	uint16_t  					bufTxWr;
	sCAN_FRAME     				bufRx[CANBUS_RX_QSIZE]; 
	uint16_t				  	bufRxRd;
	uint16_t 					bufRxWr;
#ifndef NO_SYS
		OS_EVENT			*   recCanSem;
#endif /*NO_SYS*/

} sCANBUS_DATA;

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

//INT16S CanBusRead      (INT16U Id, void *buffer, INT16U size);
//INT16S CanBusWrite     (CAN_TypeDef* CANx, void *buffer, INT16U size);
//INT16S CanBusEnable(const CANBUS_PARA *cfg);
//INT16S CanBusIoCtl(INT16U busId, INT16U func, void *argp);
//#define       CanBus0TxHandler	CAN1_TX_IRQHandler
//#define       CanBus0RxHandler	CAN1_RX0_IRQHandler
void CanBusDataInit(uint16_t canid);
int16_t DevBusRead(uint16_t id, void *buffer, uint32_t size);

void CAN_BusRxDataCallback(uint16_t canid, can_receive_message_struct head, uint8_t *data);
#endif  /* #ifndef _CAN_BUS_H_ */

