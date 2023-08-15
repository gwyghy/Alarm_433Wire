/*! ----------------------------------------------------------------------------
 * @file	l4can.h
 * @brief	stm32l452 can Çý¶¯
 *
 * @attention
 *
 * Copyright 2013 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 */

#ifndef __L4CAN_H__
#define __L4CAN_H__		

#include "string.h"
#include "stdlib.h"	
#include "stdio.h"
#include "stdarg.h"
//#include "stm32f4xx.h"
//#include "stm32f4xx_hal.h"
#include "gd32f30x.h"

/* Definition for CANx clock resources */
#define CANx                            CAN1
#define CANx_CLK_ENABLE()               __HAL_RCC_CAN0_CLK_ENABLE()
#define CANx_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOA_CLK_ENABLE()

#define CANx_FORCE_RESET()              __HAL_RCC_CAN0_FORCE_RESET()
#define CANx_RELEASE_RESET()            __HAL_RCC_CAN0_RELEASE_RESET()

/* Definition for CANx Pins */
#define CANx_TX_PIN                    GPIO_PIN_12
#define CANx_TX_GPIO_PORT              GPIOA
#define CANx_TX_AF                     GPIO_AF9_CAN1
#define CANx_RX_PIN                    GPIO_PIN_11
#define CANx_RX_GPIO_PORT              GPIOA
#define CANx_RX_AF                     GPIO_AF9_CAN1

/* Definition for CAN's NVIC */
#define CANx_RX_IRQn                   CAN0_RX0_IRQn
#define CANx_RX_IRQHandler             CAN0_RX0_IRQHandler

#define CANx_Err_IRQHandler            CAN0_SCE_IRQHandler


#define CAN_RX_IRQ_PRIO                2

/*
****************************************************************************************************
*                                         CAN FRAME STRUCT
*
* Description : Structure defines a CAN Frame
*
* Note(s)     : none.
****************************************************************************************************
*/

typedef struct 
{
	uint32_t	Stdid;
	uint8_t		DLC;
	uint8_t		Data[8];
} sCAN_FRAME;

void CanGpioInit(void);
void CAN_Config(uint32_t speed);
int32_t STM32_CAN_Write(uint16_t canid, sCAN_FRAME txMessage, uint16_t frametype);
void StartCanRx(void);

#endif  /*__L4CAN_H__*/
	 



