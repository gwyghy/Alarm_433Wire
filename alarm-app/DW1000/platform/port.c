/*! ----------------------------------------------------------------------------
 * @file	port.c
 * @brief	HW specific definitions and functions for portability
 *
 * @attention
 *
 * Copyright 2013 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */
#include "sleep.h"
//#include "lcd.h"
#include "port.h"
#include "main.h"
#include "dw1000_bus.h"
#include "gd32f30x_it.h"

/* DW1000 IRQ handler definition. */
port_DecaISR_f fpport_Deca_ISR = NULL;

int No_Configuration(void)
{
	return -1;
}

unsigned long portGetTickCnt(void)
{
	return uwTick;
}


void DW1000GpioInit(void)
{
#if STM32
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	UWB_RST_GPIO_CLK_ENABLE();
	UWB_WAKE_GPIO_CLK_ENABLE();
	UWB_CS_GPIO_CLK_ENABLE();
	UWB_IRQ_GPIO_CLK_ENABLE();
	
	 /*Configure GPIO pin : 复位 */
	GPIO_InitStruct.Pin = UWB_RST_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(UWB_RST_GPORT, &GPIO_InitStruct);

	/*Configure GPIO pin : spi 片选 */
	GPIO_InitStruct.Pin = UWB_CS_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(UWB_CS_GPORT, &GPIO_InitStruct);

	/*Configure GPIO pin : dw1000 中断 */
	GPIO_InitStruct.Pin = UWB_IRQ_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(UWB_IRQ_GPORT, &GPIO_InitStruct);

	/*Configure GPIO pin : 唤醒 */
	GPIO_InitStruct.Pin = UWB_WAKE_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(UWB_WAKE_GPORT, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(UWB_EXTI_IRQn, UWB_EXTI_IRQ_PRIO, 0);
	HAL_NVIC_EnableIRQ(UWB_EXTI_IRQn);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(UWB_CS_GPORT, UWB_CS_PIN, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(UWB_WAKE_GPORT, UWB_WAKE_PIN, GPIO_PIN_RESET);
#endif
	UWB_RST_GPIO_CLK_ENABLE();
	UWB_WAKE_GPIO_CLK_ENABLE();
	UWB_CS_GPIO_CLK_ENABLE();
	UWB_IRQ_GPIO_CLK_ENABLE();
	rcu_periph_clock_enable(RCU_AF);
	
	/*Configure GPIO pin : 复位 */
	gpio_init(UWB_RST_GPORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, UWB_RST_PIN);
	/*Configure GPIO pin : spi 片选 */
	gpio_init(UWB_CS_GPORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, UWB_CS_PIN);
	/*Configure GPIO pin : dw1000 中断 */
	gpio_init(UWB_IRQ_GPORT, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, UWB_IRQ_PIN);
	/*Configure GPIO pin : 唤醒 */
	gpio_init(UWB_WAKE_GPORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, UWB_WAKE_PIN);
	
	/* connect key EXTI line to UWB_IRQ_GPORT GPIO pin */
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOC, GPIO_PIN_SOURCE_4);
	/* configure UWB_IRQ_GPORT EXTI line */
	exti_init(UWB_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
	/* EXTI interrupt init*/
	nvic_irq_enable(UWB_EXTI_IRQn, UWB_EXTI_IRQ_PRIO, 0);
	/* clear EXTI line */
	exti_interrupt_flag_clear(UWB_EXTI_LINE);

	/*Configure GPIO pin Output Level */
	gpio_bit_set(UWB_CS_GPORT, UWB_CS_PIN);

	/*Configure GPIO pin Output Level */
	gpio_bit_reset(UWB_WAKE_GPORT, UWB_WAKE_PIN);

}

/**
  * @brief  Checks whether the specified EXTI line is enabled or not.
  * @param  EXTI_Line: specifies the EXTI line to check.
  *   This parameter can be:
  *     @arg EXTI_Linex: External interrupt line x where x(0..19)
  * @retval The "enable" state of EXTI_Line (SET or RESET).
  */
FlagStatus EXTI_GetITEnStatus(exti_line_enum EXTI_Line)
{
	FlagStatus bitstatus = RESET;
	uint32_t enablestatus = 0;
  /* Check the parameters */
	#if STM32 
	enablestatus = EXTI->IMR & EXTI_Line;
	#endif
	enablestatus = exti_interrupt_flag_get(EXTI_Line);
	if (enablestatus != (uint32_t)RESET)
	{
		bitstatus = SET;
	}
	else
	{
		bitstatus = RESET;
	}

	return bitstatus;
}

/*******************************************************************************************
* 函数名称：Reset_DW1000()
* 功能描述：DW1000复位函数
* 入口参数：无
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void Reset_DW1000(void)
{
	#if STM32
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	// Enable GPIO used for DW1000 reset
	GPIO_InitStruct.Pin = UWB_RST_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(UWB_RST_GPORT, &GPIO_InitStruct);
	
	//drive the RSTn pin low 10ns at least for reset DW1000
	HAL_GPIO_WritePin(UWB_RST_GPORT, UWB_RST_PIN, GPIO_PIN_RESET);

	//put the pin back to tri-state ... as input
	GPIO_InitStruct.Pin = UWB_RST_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(UWB_RST_GPORT, &GPIO_InitStruct);
	#endif
	// Enable GPIO used for DW1000 reset
	gpio_init(UWB_RST_GPORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, UWB_RST_PIN);
	//drive the RSTn pin low 10ns at least for reset DW1000
	gpio_bit_reset(UWB_RST_GPORT, UWB_RST_PIN);
	//put the pin back to tri-state ... as input
    gpio_init(UWB_RST_GPORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, UWB_RST_PIN);

	sleep_ms(2);
}

/*******************************************************************************************
* 函数名称：SPI_ChangeRate()
* 功能描述：SPI0波特率更改
* 入口参数：波特率分频系数
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void SPI_ChangeRate(uint16_t ScalingFactor)
{
#if STM32
	uint16_t tmpreg = 0;
	/* Get the SPIx CR1 value */
	tmpreg = SPI1->CR1;
	/*clear the scaling bits*/
	tmpreg &= 0xFFC7;
	/*set the scaling bits*/
	tmpreg |= ScalingFactor;
	/* Write to SPIx CR1 */
	SPI1->CR1 = tmpreg;
#endif
	spi_parameter_struct spi_init_struct;
	/* SPI0 parameter config */
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = ScalingFactor;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;

    spi_init(SPI0, &spi_init_struct);
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn SPI_DW1000_SetRateLow()
 *
 * @brief Set SPI rate to less than 3 MHz to properly perform DW1000 initialisation.
 *
 * @param none
 *
 * @return none
 */
void SPI_DW1000_SetRateLow(void)
{
    SPI_ChangeRate(SPI_PSC_32);
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn SPI_DW1000_SetRateHigh()
 *
 * @brief Set SPI rate as close to 20 MHz as possible for optimum performances.
 *
 * @param none
 *
 * @return none
 */
void SPI_DW1000_SetRateHigh(void)
{
    SPI_ChangeRate(SPI_PSC_4);
}

/**
  * @brief  Handle EXTI interrupt request.
  * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_IRQHandler(exti_line_enum extiline)
{
	/* EXTI line interrupt detected */
	if(exti_interrupt_flag_get(extiline) != RESET)
	{
		exti_interrupt_flag_clear(extiline);
		HAL_GPIO_EXTI_Callback(extiline);
	}
}

/**
  * @brief This function handles EXTI line0 interrupt.
  */
void DWM1000_IRQHandler(void)
{
	/* USER CODE BEGIN EXTI0_IRQn 0 */
	OSIntEnter();
	/* USER CODE END EXTI0_IRQn 0 */
	HAL_GPIO_EXTI_IRQHandler(UWB_EXTI_LINE);
	/* USER CODE BEGIN EXTI0_IRQn 1 */
	OSIntExit();
	/* USER CODE END EXTI0_IRQn 1 */
}

#if INVOKED		//声明实际未被调用
/*******************************************************************************************
* 函数名称：NoiseFigureCal()
* 功能描述：SPI1波特率更改
* 入口参数：波特率分频系数
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void NoiseFigureCal(dwt_rxdiag_t *rxdiag, sFrameQuality *framequa)
{
	DWT_ReadDiagnostics(rxdiag);
	
	framequa->NoiseFigure = 1000 * rxdiag->stdNoise / rxdiag->firstPathAmp2;
	framequa->FP_Power = FP_PowerLevelCal(rxdiag);
	framequa->RX_Power = RX_PowerLevelCal(rxdiag);
}


/*******************************************************************************************
* 函数名称：FP_PowerLevelCal()
* 功能描述：SPI1波特率更改
* 入口参数：波特率分频系数
* 出口参数：无
* 使用说明：无
********************************************************************************************/
double FP_PowerLevelCal(dwt_rxdiag_t *rxdiag)
{
	double F12,F22,F32,N2;
	double FP_Power;
	
	F12 = rxdiag->firstPathAmp1 * rxdiag->firstPathAmp1;
	F22 = rxdiag->firstPathAmp2 * rxdiag->firstPathAmp2;
	F32 = rxdiag->firstPathAmp3 * rxdiag->firstPathAmp3;
	
	N2 = rxdiag->rxPreamCount * rxdiag->rxPreamCount;
	
	FP_Power = 10 * log10((F12 + F22 + F32) / N2) - FP_A;
	
	return FP_Power;
}


/*******************************************************************************************
* 函数名称：RX_PowerLevelCal()
* 功能描述：SPI1波特率更改
* 入口参数：波特率分频系数
* 出口参数：无
* 使用说明：无
********************************************************************************************/
double RX_PowerLevelCal(dwt_rxdiag_t *rxdiag)
{
	double N2,C;
	double RX_Power;
	
	C = rxdiag ->maxGrowthCIR * 131072;
	N2 = rxdiag->rxPreamCount * rxdiag->rxPreamCount;
	
	RX_Power = 10 * log10(C / N2) - FP_A;
	
	return RX_Power;
}


/*******************************************************************************************
* 函数名称：DistAdjust()
* 功能描述：SPI1波特率更改
* 入口参数：波特率分频系数
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void DistAdjust(sDistList_N *midpoint, sFrameQuality *framequa_poll, sFrameQuality *framequa_final, uint16_t *val)
{
	double M,M1,M2,N,N1,N2,P,Q;
	uint8_t wr;
	
	M1 = framequa_poll->NoiseFigure > NoiseThresh ? framequa_poll->NoiseFigure / NoiseThresh : 1;
	M1 = framequa_poll->NoiseFigure > NoiseThreshMax ? 10 : M1;
	M2 = framequa_final->NoiseFigure > NoiseThresh ? framequa_final->NoiseFigure / NoiseThresh : 1;
	M2 = framequa_final->NoiseFigure > NoiseThreshMax ? 10 : M2;
	N1 = (framequa_poll->RX_Power - framequa_poll->FP_Power) > 6 ? ((framequa_poll->RX_Power - framequa_poll->FP_Power) / 6) : 1;
	N2 = (framequa_final->RX_Power - framequa_final->FP_Power) > 6 ? ((framequa_final->RX_Power - framequa_final->FP_Power) / 6) : 1;
	
	M = (M1 + M2) / 2;
	N = (N1 + N2) / 2;
	
	P = log10(M * N);
	P = P > 1 ? 1 :(P < 0 ? 0 : P);
	
//	midpoint->p->distraw = *val;
	
	if(midpoint->p->commCnt > 0)
	{
		wr = (midpoint->p->wr == 0 ? ANC_TOF_REC_MAX - 1 : midpoint->p->wr - 1);
		*val = midpoint->p->dist[wr] * P + *val * (1 - P);
	}
	else
	{
		*val = 2000 * P + *val * (1 - P);					//第一次测距值与15m进行加权滤波处理
	}
}
#endif
