/*! ----------------------------------------------------------------------------
 * @file	uwb_bsp.c
 * @brief	dw1000 驱动
 *
 * @attention
 *
 * Copyright 2013 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 */
#include "uwb_bsp.h"





void UwbError_Handler(void)
{
	while(1)
	{
		;
	}
}

/*******************************************************************************************
* 函数名称：void SPIGpioInit(void)
* 功能描述：can gpio 初始化
* 入口参数：无
* 出口参数：无
* 使用说明：无
********************************************************************************************/

void UwbGpioInit(void)
{
//	GPIO_InitTypeDef   GPIO_InitStruct;

//    /*##-1- Enable peripherals and GPIO Clocks #################################*/
//    /* Enable GPIO TX/RX clock */
//    SPIx_SCK_GPIO_CLK_ENABLE();
//    SPIx_MISO_GPIO_CLK_ENABLE();
//    SPIx_MOSI_GPIO_CLK_ENABLE();
//    /* Enable SPI clock */
//    SPIx_CLK_ENABLE();

//    /*##-2- Configure peripheral GPIO ##########################################*/
//    /* SPI SCK GPIO pin configuration  */
//    GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
//    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
//    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
//    GPIO_InitStruct.Alternate = SPIx_SCK_AF;
//    HAL_GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);

//    /* SPI MISO GPIO pin configuration  */
//    GPIO_InitStruct.Pin = SPIx_MISO_PIN;
//    GPIO_InitStruct.Alternate = SPIx_MISO_AF;
//    HAL_GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStruct);

//    /* SPI MOSI GPIO pin configuration  */
//    GPIO_InitStruct.Pin = SPIx_MOSI_PIN;
//    GPIO_InitStruct.Alternate = SPIx_MOSI_AF;
//    HAL_GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStruct);

//    /*##-3- Configure the NVIC for SPI #########################################*/
//    /* NVIC for SPI */
//    HAL_NVIC_SetPriority(SPIx_IRQn, 1, 0);
//    HAL_NVIC_EnableIRQ(SPIx_IRQn);
		
}







