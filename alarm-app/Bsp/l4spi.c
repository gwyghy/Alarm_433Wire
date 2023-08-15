/*! ----------------------------------------------------------------------------
 * @file	l4spi.h
 * @brief	stm32l452 spi 驱动
 *
 * @attention
 *
 * Copyright 2013 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 */
#include "l4spi.h"

#if STM32
SPI_HandleTypeDef hspi1;
#endif
#if INVOKED		//声明实际未被调用
sSpiRTInfo gSpiRTInfo;
#endif

void SpiError_Handler(void)
{
	while(1)
	{
		;
	}
}
#if INVOKED		//声明实际未被调用
/*******************************************************************************************
* 函数名称：void SPIGpioInit(void)
* 功能描述：can gpio 初始化
* 入口参数：无
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void SpiGpioInit(void)
{
	 GPIO_InitTypeDef GPIO_InitStruct = {0};

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    SPIx_SCK_GPIO_CLK_ENABLE();
    SPIx_MISO_GPIO_CLK_ENABLE();
    SPIx_MOSI_GPIO_CLK_ENABLE();
    /* Enable SPI clock */
    SPIx_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* SPI SCK GPIO pin configuration  */
    GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = SPIx_SCK_AF;
    HAL_GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);

    /* SPI MISO GPIO pin configuration  */
    GPIO_InitStruct.Pin = SPIx_MISO_PIN;
    GPIO_InitStruct.Alternate = SPIx_MISO_AF;
    HAL_GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStruct);

    /* SPI MOSI GPIO pin configuration  */
    GPIO_InitStruct.Pin = SPIx_MOSI_PIN;
    GPIO_InitStruct.Alternate = SPIx_MOSI_AF;
    HAL_GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStruct);

#if SPI_MODE == SPI_INT_MODE
    HAL_NVIC_SetPriority(SPIx_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(SPIx_IRQn);
#endif 		/* SPI_MODE == SPI_INT_MODE*/
}
#endif
/* SPI1 init function */
void MX_SPI1_Init(void)
{
	#if STM32
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
	hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 7;
//	hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
//	hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
	if (HAL_SPI_Init(&hspi1) != HAL_OK)
	{
		SpiError_Handler();
	}
	
	__HAL_SPI_ENABLE(&hspi1);
	
	#endif

	spi_parameter_struct spi_init_struct;

    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_SPI0);
	/**SPI0 GPIO Configuration
	PA5 ------> SPIx_SCK_PIN
	PA6 ------> SPIx_MISO_PIN
	PA7 ------> SPIx_MOSI_PIN
	**************************/
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, SPIx_SCK_PIN);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, SPIx_MISO_PIN);
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, SPIx_MOSI_PIN);

    /* SPI0 parameter config */
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_4;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI0, &spi_init_struct);

    /* set crc polynomial */
    spi_crc_polynomial_set(SPI0, 7);			//默认值为0x0007h。
    /* enable SPI0 */
    spi_enable(SPI0);
}

#if STM32
void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	if(spiHandle->Instance == SPI1)
	{
		/* USER CODE BEGIN SPI1_MspInit 0 */

		/* USER CODE END SPI1_MspInit 0 */
		/* SPI1 clock enable */
		__HAL_RCC_SPI1_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**SPI1 GPIO Configuration    
		PA5     ------> SPI1_SCK
		PA6     ------> SPI1_MISO
		PA7     ------> SPI1_MOSI 
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_5;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_6;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
			
		GPIO_InitStruct.Pin = GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* USER CODE BEGIN SPI1_MspInit 1 */

		/* USER CODE END SPI1_MspInit 1 */
	}
	
	  /*##-3- Configure the NVIC for SPI #########################################*/
    /* NVIC for SPI */
#if SPI_MODE == SPI_INT_MODE
    HAL_NVIC_SetPriority(SPIx_IRQn, SPI_IRQ_PRIO,  0);
    HAL_NVIC_EnableIRQ(SPIx_IRQn);
#endif 		/* SPI_MODE == SPI_INT_MODE*/
}
#endif

#if INVOKED		//声明实际未被调用
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{
	if(spiHandle->Instance==SPI1)
	{
  /* USER CODE BEGIN SPI1_MspDeInit 0 */

  /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
		__HAL_RCC_SPI1_CLK_DISABLE();
  
    /**SPI1 GPIO Configuration    
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI 
    */
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);

  /* USER CODE BEGIN SPI1_MspDeInit 1 */

  /* USER CODE END SPI1_MspDeInit 1 */
	}
} 



/*******************************************************************************************
* 函数名称：void SPI_Config(void)
* 功能描述：can gpio 初始化
* 入口参数：无
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void SPI_Config(uint32_t speed)
{
	;
}
#endif


#if SPI_MODE == SPI_INT_MODE
static void SPI_2linesRxISR_8BIT(struct __SPI_HandleTypeDef *hspi)
{
  /* Receive data in packing mode */
  if (hspi->RxXferCount > 0U)
  {
    *hspi->pRxBuffPtr = *((__IO uint8_t *)&hspi->Instance->DR);
    hspi->pRxBuffPtr++;
    hspi->RxXferCount--;
  }

  /* Check end of the reception */
  if (hspi->RxXferCount == 0U)
  {

    /* Disable RXNE  and ERR interrupt */
    __HAL_SPI_DISABLE_IT(hspi, (SPI_IT_RXNE | SPI_IT_ERR));

    if (hspi->TxXferCount == 0U)
    {
      SPI_CloseRxTx_ISR(hspi);
    }
  }
}


static void SPI_2linesTxISR_8BIT(struct __SPI_HandleTypeDef *hspi)
{
  /* Transmit data in packing Bit mode */
  if (hspi->TxXferCount > 0U)
  {
    *(__IO uint8_t *)&hspi->Instance->DR = (*hspi->pTxBuffPtr);
    hspi->pTxBuffPtr++;
    hspi->TxXferCount--;
  }
	

  /* Check the end of the transmission */
  if (hspi->TxXferCount == 0U)
  {

    /* Disable TXE interrupt */
    __HAL_SPI_DISABLE_IT(hspi, SPI_IT_TXE);

    if (hspi->RxXferCount == 0U)
    {
  //    SPI_CloseRxTx_ISR(hspi);
			SPI_CloseRxTx_ISR(hspi);
    }
  }
}



HAL_StatusTypeDef Z_SPI_TransmitReceive_IT(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size)
{
  uint32_t             tmp_mode;
  HAL_SPI_StateTypeDef tmp_state;
  HAL_StatusTypeDef    errorcode = HAL_OK;
	SPI_HandleTypeDef *  hspi = &hspi1;

  /* Process locked */
  __HAL_LOCK(hspi);

  /* Init temporary variables */
  tmp_state           = hspi->State;
  tmp_mode            = hspi->Init.Mode;

  if (!((tmp_state == HAL_SPI_STATE_READY) || \
        ((tmp_mode == SPI_MODE_MASTER) && (hspi->Init.Direction == SPI_DIRECTION_2LINES) && (tmp_state == HAL_SPI_STATE_BUSY_RX))))
  {
    errorcode = HAL_BUSY;
    goto error;
  }

//  if ((pTxData == NULL) || (pRxData == NULL) || (Size == 0U))
//  {
//    errorcode = HAL_ERROR;
//    goto error;
//  }

  /* Don't overwrite in case of HAL_SPI_STATE_BUSY_RX */
  if (hspi->State != HAL_SPI_STATE_BUSY_RX)
  {
    hspi->State = HAL_SPI_STATE_BUSY_TX_RX;
  }

  /* Set the transaction information */
  hspi->ErrorCode   = HAL_SPI_ERROR_NONE;
  hspi->pTxBuffPtr  = (uint8_t *)pTxData;
  hspi->TxXferSize  = Size;
  hspi->TxXferCount = Size;
  hspi->pRxBuffPtr  = (uint8_t *)pRxData;
  hspi->RxXferSize  = Size;
  hspi->RxXferCount = Size;



	hspi->RxISR     = SPI_2linesRxISR_8BIT;
	hspi->TxISR     = SPI_2linesTxISR_8BIT;

  /* Enable TXE, RXNE and ERR interrupt */
  __HAL_SPI_ENABLE_IT(hspi, (SPI_IT_TXE | SPI_IT_RXNE | SPI_IT_ERR));

  /* Check if the SPI is already enabled */
  if ((hspi->Instance->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE)
  {
    /* Enable SPI peripheral */
    __HAL_SPI_ENABLE(hspi);
  }

error :
  /* Process Unlocked */
  __HAL_UNLOCK(hspi);
  return errorcode;
}



void Z_SPIx_IRQHandler(SPI_HandleTypeDef *hspi)
{
  uint32_t itsource = hspi->Instance->CR2;
  uint32_t itflag   = hspi->Instance->SR;

  /* SPI in mode Receiver ----------------------------------------------------*/
  if ((SPI_CHECK_FLAG(itflag, SPI_FLAG_OVR) == RESET) &&
      (SPI_CHECK_FLAG(itflag, SPI_FLAG_RXNE) != RESET) && (SPI_CHECK_IT_SOURCE(itsource, SPI_IT_RXNE) != RESET))
  {
    hspi->RxISR(hspi);
    return;
  }

  /* SPI in mode Transmitter -------------------------------------------------*/
  if ((SPI_CHECK_FLAG(itflag, SPI_FLAG_TXE) != RESET) && (SPI_CHECK_IT_SOURCE(itsource, SPI_IT_TXE) != RESET))
  {
    hspi->TxISR(hspi);
    return;
  }

  /* SPI in Error Treatment --------------------------------------------------*/
  if (((SPI_CHECK_FLAG(itflag, SPI_FLAG_MODF) != RESET) || (SPI_CHECK_FLAG(itflag, SPI_FLAG_OVR) != RESET) 
				|| (SPI_CHECK_FLAG(itflag, SPI_FLAG_FRE) != RESET)) && (SPI_CHECK_IT_SOURCE(itsource, SPI_IT_ERR) != RESET))
  {
    /* SPI Overrun error interrupt occurred ----------------------------------*/
    if (SPI_CHECK_FLAG(itflag, SPI_FLAG_OVR) != RESET)
    {
      if (hspi->State != HAL_SPI_STATE_BUSY_TX)
      {
        SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_OVR);
        __HAL_SPI_CLEAR_OVRFLAG(hspi);
      }
      else
      {
        __HAL_SPI_CLEAR_OVRFLAG(hspi);
        return;
      }
    }

    /* SPI Mode Fault error interrupt occurred -------------------------------*/
    if (SPI_CHECK_FLAG(itflag, SPI_FLAG_MODF) != RESET)
    {
      SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_MODF);
      __HAL_SPI_CLEAR_MODFFLAG(hspi);
    }

    /* SPI Frame error interrupt occurred ------------------------------------*/
    if (SPI_CHECK_FLAG(itflag, SPI_FLAG_FRE) != RESET)
    {
      SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_FRE);
      __HAL_SPI_CLEAR_FREFLAG(hspi);
    }

    if (hspi->ErrorCode != HAL_SPI_ERROR_NONE)
    {
      /* Disable all interrupts */
      __HAL_SPI_DISABLE_IT(hspi, SPI_IT_RXNE | SPI_IT_TXE | SPI_IT_ERR);

      hspi->State = HAL_SPI_STATE_READY;
      {
        /* Call user error callback */
        HAL_SPI_ErrorCallback(hspi);

      }
    }
    return;
  }
}



void SPIx_IRQHandler(void)
{
  Z_SPIx_IRQHandler(&hspi1);
}
#endif /*SPI_MODE == SPI_INT_MODE*/




