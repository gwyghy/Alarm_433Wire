/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
/* USER CODE BEGIN 0 */
#include "deca_regs.h"
#include "deca_device_api.h"
/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */
extern uint8_t DECA_WorkMode;
/* USER CODE END 1 */

/** Configure pins
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = D_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(D_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = D_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(D_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = D_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(D_IRQ_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = D_WAKEUP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(D_WAKEUP_GPIO_Port, &GPIO_InitStruct);
	
	/*RS485发送接收控制脚*/
//	GPIO_InitStruct.Pin = D_RS485_TX_Ctrl_Pin;
//  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//  HAL_GPIO_Init(D_RS485_TX_Ctrl_GPIO_Port, &GPIO_InitStruct);
//	
//	/* 状态指示灯D3/D4 */
//	GPIO_InitStruct.Pin = LED3_PIN | LED4_PIN;
//  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
//  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//  HAL_GPIO_Init(LED3_LED4_PIN_Port, &GPIO_InitStruct);
	
	/* 状态指示灯D1 */
//	GPIO_InitStruct.Pin = LED1_PIN;
//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
//  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//	HAL_GPIO_Init(LED1_PIN_Port, &GPIO_InitStruct);
	
//#if defined(USE_OLED)
//	/*Configure GPIO pin : SCL_Pin   SDA_Pin*/
//	GPIO_InitStruct.Pin = I2C_SCL_Pin | I2C_SDA_Pin;
//  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
//	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//  HAL_GPIO_Init(I2C_GPIO_Port, &GPIO_InitStruct);
//#endif
	
  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(D_IRQ_EXTI_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(D_IRQ_EXTI_IRQn);
	
	/*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(D_CS_GPIO_Port, D_CS_Pin, GPIO_PIN_SET);
	
	/*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(D_WAKEUP_GPIO_Port, D_WAKEUP_Pin, GPIO_PIN_RESET);
	
	/*点亮系统运行指示灯*/
//	HAL_GPIO_WritePin(LED1_PIN_Port, LED1_PIN, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(LED3_LED4_PIN_Port, LED4_PIN, GPIO_PIN_RESET);
//	HAL_GPIO_WritePin(LED3_LED4_PIN_Port, LED3_PIN, GPIO_PIN_RESET);
}

/* USER CODE BEGIN 2 */
/*******************************************************************************************
* 函数名称：HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
* 功能描述：GPIO外部中断回调函数
* 入口参数：GPIO_Pin
* 出口参数：无
* 使用说明：无
********************************************************************************************/
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//	if(DWT_Read32BitReg(SYS_STATUS_ID) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR))
//	{
//		if(DECA_WorkMode == DECA_ANCHOR_MASTER || DECA_WorkMode == DECA_ANCHOR_SLAVER)
//		{
//			OSTaskResume(UWB_ANCHOR_PRIO);
//		}
//		if(DECA_WorkMode == DECA_CLE)
//		{
//			OSTaskResume(UWB_CLE_PRIO);
//		}
//		if(DECA_WorkMode == DECA_TAG)
//		{
//			OSTaskResume(UWB_TAG_PRIO);
//		}
//	}
//}

/*******************************************************************************************
* 函数名称：LED_Blink(void *pdata)
* 功能描述：OS系统运行指示灯（绿色）
* 入口参数：*pdata
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void LED_Blink(void *pdata)
{
	for(;;)
	{
    HAL_GPIO_TogglePin(LED1_PIN_Port, LED1_PIN);
		OSTimeDly(1000);
	}
}
/* USER CODE END 2 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
