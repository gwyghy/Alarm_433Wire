/*! ----------------------------------------------------------------------------
 * @file	l4spi.h
 * @brief	stm32l452 spi Çý¶¯
 *
 * @attention
 *
 * Copyright 2013 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 */

#ifndef __L4SPI_H__
#define __L4SPI_H__		

#include "string.h"
#include "stdlib.h"	
#include "stdio.h"
#include "stdarg.h"
//#include "stm32f4xx.h"
//#include "stm32f4xx_hal.h"
#include "gd32f30x.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* User can use this section to tailor SPIx instance used and associated 
   resources */
/* Definition for SPIx clock resources */


//#define SPIx_PRESCALER			SPI_BAUDRATEPRESCALER_8

#define SPI_IRQ_PRIO          1

//#define SPIx						    ((SPI_HandleTypeDef *)&hspi1)
//#define SPIx_GPIO					  GPIOA
//#define SPIx_CS					  	D_CS_Pin
//#define SPIx_CS_GPIO				D_CS_GPIO_Port
//#define SPIx_SCK					  GPIO_Pin_5
//#define SPIx_MISO					  GPIO_Pin_6
//#define SPIx_MOSI					  GPIO_Pin_7
//#define SPI_I2S_FLAG_RXNE   SPI_FLAG_RXNE
#define SPI_NORMAL_MODE       0
#define SPI_INT_MODE          1

#define SPI_MODE             SPI_NORMAL_MODE

#define ZSPIx                             SPI1
#define SPIx_CLK_ENABLE()                __HAL_RCC_SPI1_CLK_ENABLE()
#define SPIx_SCK_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIx_MISO_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIx_MOSI_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIx_CS_GPIO_CLK_ENABLE()      	 __HAL_RCC_GPIOA_CLK_ENABLE()

#define SPIx_FORCE_RESET()               __HAL_RCC_SPI1_FORCE_RESET()
#define SPIx_RELEASE_RESET()             __HAL_RCC_SPI1_RELEASE_RESET()

/* Definition for SPIx Pins */
#define SPIx_SCK_PIN                     GPIO_PIN_5
#define SPIx_SCK_GPIO_PORT               GPIOA
#define SPIx_SCK_AF                      GPIO_AF5_SPI1
#define SPIx_MISO_PIN                    GPIO_PIN_6
#define SPIx_MISO_GPIO_PORT              GPIOA
#define SPIx_MISO_AF                     GPIO_AF5_SPI1
#define SPIx_MOSI_PIN                    GPIO_PIN_7
#define SPIx_MOSI_GPIO_PORT              GPIOA
#define SPIx_MOSI_AF                     GPIO_AF5_SPI1


/*
****************************************************************************************************
*                                        
*
* Description : 
*
* Note(s)     : none.
****************************************************************************************************
*/

typedef struct 
{
	uint8_t         *pRxBuffPtr;
	uint16_t 		RxXferCount;
	uint16_t        RxOff;
	uint8_t         *pTxBuffPtr;    /*!< Pointer to SPI Tx transfer Buffer        */
	uint16_t        TxXferSize;     /*!< SPI Tx Transfer size                     */
	         
}sSpiRTInfo;

//extern SPI_HandleTypeDef hspi1;



void MX_SPI1_Init(void);
//HAL_StatusTypeDef Z_SPI_TransmitReceive_IT(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size);

#endif  /*__L4SPI_H__*/
	 



