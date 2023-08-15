 /*
*********************************************************************************************************
*	                                           UWB
*                           Commuication Controller For The Mining Industry
*
*                                  (c) Copyright 1994-2013  HNDZ 
*                       All rights reserved.  Protected by international copyright laws.
*
*
*    File    : spi_bus.c
*    Module  : spi driver
*    Version : V1.0
*    History :
*   -----------------
*             spi底层驱动
*              Version  Date           By            Note
*              v1.0     2013-09-09     xxx
*
*********************************************************************************************************
*/
#include "dev_bus.h"
#include "spi_bus.h"
#include <string.h>
#include "l4spi.h"
//#include "deca_mutex.h"
#include "deca_device_api.h"
#include "port.h"

#define SPI_TX_QSIZE       256
#define SPI_RX_QSIZE       256

extern __weak uint32_t HAL_GetTick(void);

//DEV_BUS_PRO(SPI, uint8_t)

uint8_t gSpiTxBuf[SPI_TX_QSIZE];
uint8_t gSpiRxBuf[SPI_RX_QSIZE];

#if SPI_MODE == SPI_INT_MODE
	OS_EVENT *gSpiEndSem;
#else
	uint32_t gSpiErrId;
#endif /*SPI_MODE == SPI_INT_MODE*/
/******************************************************************************
** 函数名称: void SpiBusDataInit(void)
** 功能描述: 
** 参数描述：
*******************************************************************************/
#if SPI_MODE == SPI_INT_MODE
void SpiBusDataInit(void)
{
	gSpiEndSem = OSSemCreate(0);
}
#endif /*#if SPI_MODE == SPI_INT_MODE*/
/******************************************************************************
** 函数名称: void SpiBusInit(void)
** 功能描述: 根据设备parameter初始化设备
** 参数描述：spi底层驱动初始化
*******************************************************************************/
void SpiBusInit(void)
{
#if SPI_MODE == SPI_INT_MODE
	SpiBusDataInit();
#endif /* SPI_MODE == SPI_INT_MODE*/
	MX_SPI1_Init();

}

void SpiBusError_Handler(void)
{
	while(1)
	{
		;
	}
}


#if SPI_MODE == SPI_INT_MODE
int ReadFromSPI(uint16_t headerLength,  uint8_t *headerBuffer, uint32_t readlength, uint8_t *readBuffer)
{
	INT8U err;
	int   ret = 0;
	memcpy(&gSpiTxBuf, headerBuffer, headerLength);
	
	port_SPIx_set_chip_select();
	if(Z_SPI_TransmitReceive_IT(gSpiTxBuf, gSpiRxBuf, headerLength+readlength)  != HAL_OK)
	{
		SpiBusError_Handler();
	}	
	OSSemPend(gSpiEndSem, 0, &err);
	if(err == OS_ERR_TIMEOUT)
	{
	 	ret = BUS_RX_TIMEOVER;
	}
	else if(err == OS_ERR_NONE)
	{
		 memcpy(readBuffer, gSpiRxBuf + headerLength, readlength);
		 ret = BUS_RX_NORMAL;
	}
	else
	{
		ret =   BUS_RX_UNKNOWN_ERR;	                                            //error
	}
	port_SPIx_clear_chip_select();
	return ret;
	
}



int WriteToSPI(uint16_t headerLength,  uint8_t *headerBuffer, uint32_t bodylength,  uint8_t *bodyBuffer)
{

	INT8U err;
	int   ret = 0;
  decaIrqStatus_t  stat ;

  stat = DecaMutexON() ;
  port_SPIx_set_chip_select();
	
	memcpy(gSpiTxBuf, headerBuffer, headerLength);
	memcpy(gSpiTxBuf+headerLength, bodyBuffer, bodylength);
	
//	port_SPIx_set_chip_select();
	if(Z_SPI_TransmitReceive_IT(gSpiTxBuf, gSpiRxBuf, headerLength+bodylength)  != HAL_OK)
	{
		SpiBusError_Handler();
	}	
	OSSemPend(gSpiEndSem, 0, &err);
	if(err == OS_ERR_TIMEOUT)
	{
	 	ret = BUS_RX_TIMEOVER;
	}
	else if(err == OS_ERR_NONE)
	{
		 ret = BUS_RX_NORMAL;
	}
	else
	{
		ret =   BUS_RX_UNKNOWN_ERR;	                                            //error
	}
	port_SPIx_clear_chip_select();
	DecaMutexOFF(stat) ;
	return ret;
} // end WriteToSPI()

/**
  * @brief  TxRx Transfer completed callback.
  * @param  hspi: SPI handle
  * @note   This example shows a simple way to report end of Interrupt TxRx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
//	OSIntEnter();  
	OSSemPost(gSpiEndSem);
//	OSIntExit();     
}

#else

/*!
    \brief      send a byte through the SPI interface and return the byte received from the SPI bus
    \param[in]  byte: byte to send
    \param[out] none
    \retval     the value of the received byte
*/
uint8_t spi_send_byte(uint8_t byte)
{
    /* loop while data register in not emplty */
    while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE));
    /* send byte through the SPI0 peripheral */
    spi_i2s_data_transmit(SPI0, byte);
    /* wait to receive a byte */
    while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE));
    /* return the byte read from the SPI bus */
    return (spi_i2s_data_receive(SPI0));
}

/**
  * @brief  HAL Status structures definition
  */
typedef enum
{
	HAL_OK       = 0x00,
	HAL_ERROR    = 0x01,
	HAL_BUSY     = 0x02,
	HAL_TIMEOUT  = 0x03			//发送数据超时
} HAL_StatusTypeDef;

/**
  * @brief  Transmit an amount of data in blocking mode.
  * @param  hspi pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @param  pData pointer to data buffer
  * @param  Size amount of data to be sent
  * @param  Timeout Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SPI_Transmit(/*&hspi1, */uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	uint32_t tickstart;
	HAL_StatusTypeDef errorcode = HAL_OK;
	uint16_t initial_TxXferCount;
	uint8_t *pTxBuffPtr;
	
	pTxBuffPtr = (uint8_t *)pData;
	/* Init tickstart for timeout management*/
	tickstart = HAL_GetTick();
	initial_TxXferCount = Size;

	if ((pData == NULL) || (Size == 0U))
	{
		errorcode = HAL_ERROR;
		return errorcode;
	}

	/* Transmit data in 8 Bit mode */
	if (initial_TxXferCount == 0x01U)			//only one byte
	{
		spi_send_byte(*(uint8_t *)pTxBuffPtr);
	}
    while (initial_TxXferCount > 0U)
    {
		/* Wait until TXE flag is set to send data */
		if (spi_i2s_flag_get(SPI0, SPI_FLAG_TBE))
		{
			if (initial_TxXferCount > 1U)
			{
				/* write on the data register in packing mode */
				spi_send_byte(*(uint8_t *)pTxBuffPtr);
				pTxBuffPtr += sizeof(uint8_t);
				initial_TxXferCount -= 1;
			}
			else
			{
				spi_send_byte(*(uint8_t *)pTxBuffPtr);
				pTxBuffPtr++;
				initial_TxXferCount--;
			}
		}
		else
		{
			/* Timeout management */
			if ((((HAL_GetTick() - tickstart) >=  Timeout) && (Timeout != 0xFFFFFFFFU)) || (Timeout == 0U))
			{
				errorcode = HAL_TIMEOUT;
			}
		}
    }

	return errorcode;
}

/**
  * @brief  Transmit an amount of data in blocking mode.
  * @param  hspi pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @param  pData pointer to data buffer
  * @param  Size amount of data to be sent
  * @param  Timeout Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SPI_TransmitReceive(/*SPI_HandleTypeDef *hspi, */uint8_t *pTxData, uint8_t *pRxData, uint16_t Size,
                                          uint32_t Timeout)
{
	uint32_t tickstart;
	HAL_StatusTypeDef errorcode = HAL_OK;
	uint8_t initial_TxXferCount;
//	uint8_t RxXferSize;
	
	uint8_t *pTxBuffPtr;
	uint8_t *pRxBuffPtr;
	pTxBuffPtr = (uint8_t *)pTxData;
	pRxBuffPtr = (uint8_t *)pRxData;
//	RxXferSize = Size;
	initial_TxXferCount = Size;
	/* Init tickstart for timeout management*/
	tickstart = HAL_GetTick();

	if ((pTxData == NULL) || (Size == 0U))
	{
		errorcode = HAL_ERROR;
		return errorcode;
	}

  /* Transmit data in 8 Bit mode */
    if (initial_TxXferCount == 0x01U)
    {
		*(uint8_t *)pRxBuffPtr = spi_send_byte(*(uint8_t *)pTxBuffPtr);
	}
	while (initial_TxXferCount > 0U)
	{
		/* Wait until TXE flag is set to send data */
		if (spi_i2s_flag_get(SPI0, SPI_FLAG_TBE))
		{
			if (initial_TxXferCount > 1U)
			{
				/* write on the data register in packing mode */
				*(uint8_t *)pRxBuffPtr = spi_send_byte(*(uint8_t *)pTxBuffPtr);
				pTxBuffPtr += sizeof(uint8_t);
				pRxBuffPtr += sizeof(uint8_t);
				initial_TxXferCount -= 1;
			}
			else
			{
				*(uint8_t *)pRxBuffPtr = spi_send_byte(*(uint8_t *)pTxBuffPtr);
				pTxBuffPtr ++;
				pRxBuffPtr ++;
				initial_TxXferCount --;
			}
		}
		else
		{
			/* Timeout management */
			if ((((HAL_GetTick() - tickstart) >= Timeout) && (Timeout != 0xFFFFFFFFU)) || (Timeout == 0U))
			{
				errorcode = HAL_TIMEOUT;
			}
		}
	}

	return errorcode;
}

int ReadFromSPI(uint16_t headerLength, uint8_t *headerBuffer, uint32_t readlength, uint8_t *readBuffer)
{
//	INT8U err;
	int ret = 0;
	memcpy(&gSpiTxBuf, headerBuffer, headerLength);
	
	port_SPIx_set_chip_select();
	gSpiErrId = HAL_SPI_TransmitReceive(/*&hspi1, */gSpiTxBuf, gSpiRxBuf, headerLength + readlength, 20);
	if(gSpiErrId != HAL_OK)
	{
		SpiBusError_Handler();
	}	
	else
	{
		memcpy(readBuffer, gSpiRxBuf + headerLength, readlength);
		ret = BUS_RX_NORMAL;
	}
	port_SPIx_clear_chip_select();

	return ret;
} // end ReadFromSPI()

int WriteToSPI(uint16_t headerLength, uint8_t *headerBuffer, uint32_t bodylength, uint8_t *bodyBuffer)
{
//	INT8U err;
	int ret = 0;
	decaIrqStatus_t stat;

	stat = DecaMutexON();
	port_SPIx_set_chip_select();
	
	memcpy(gSpiTxBuf, headerBuffer, headerLength);
	memcpy(gSpiTxBuf + headerLength, bodyBuffer, bodylength);
	

//	port_SPIx_set_chip_select();
//	gSpiErrId = HAL_SPI_TransmitReceive(&hspi1, gSpiTxBuf, gSpiRxBuf, headerLength+readlength, 20);
	gSpiErrId = HAL_SPI_Transmit(/*&hspi1, */gSpiTxBuf, headerLength + bodylength, 20);
	
	if(gSpiErrId != HAL_OK)
	{
		SpiBusError_Handler();
	}	
	else 
	{
		ret = BUS_RX_NORMAL;
	}
	port_SPIx_clear_chip_select();
	DecaMutexOFF(stat);
	
	return ret;
} // end WriteToSPI()

#endif /* SPI_MODE == SPI_INT_MODE*/




