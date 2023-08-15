/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
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
#include "usart.h"


#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式  
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数
int fputc(int ch, FILE *f)
{
	RS485_TX;
	USART1->TDR = (uint8_t) ch; 
	while((USART1->ISR & 0X40) == 0);//循环发送，直到完毕       
	RS485_RX;
	return ch;
}
#endif




/* USER CODE BEGIN 0 */
extern uint16_t DECA_WorkMode;

uint8_t USART1RxBuff[USART_REC_LEN];       //接收缓冲

/*接收数据状态
* bit15:接收完成标志
* bit14:接收到0x0d
* bit13~0:接收到的有效字节数目*/
uint16_t USART_RX_STA = 0;                 //接收数据标记及已接收数据长度
/* USER CODE END 0 */

UART_HandleTypeDef huart1;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
	HAL_NVIC_EnableIRQ(USART1_IRQn);				    //使能USART1中断通道
	HAL_NVIC_SetPriority(USART1_IRQn,9,0);			//抢占优先级9，子优先级0
	__HAL_UART_ENABLE_IT(USART, UART_IT_RXNE);  //使能接收中断
	
	RS485_RX;//默认接收模式
}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();
  
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();
  
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
void USART1_RxData(void *pdata)
{
	uint8_t Res;
	for(;;)
	{
		Res = huart1.Instance->RDR & (uint16_t)0x1FF;//读取数据
		
		if((USART_RX_STA & 0x8000) == 0)//接收未完成
		{
			if(USART_RX_STA & 0x4000)//已接收到0x0d
			{
				if(Res != 0x0a)
					USART_RX_STA = 0;//接收错误，重新开始
				else
				{					
					USART_RX_STA |= 0x8000;	//接收完成
					if(USART1RxBuff[0] == 0x01)
					{
						if(USART1RxBuff[1] == 0x10)//接收到0x01,0x10,配置模块为标签模式
						{
							DECA_WorkMode = DECA_TAG;
							USART_RX_STA = 0;
							OSTaskResume(UWB_MODECHOOSE_PRIO);
						}
						if(USART1RxBuff[1] == 0x11)//接收到0x01,0x11,配置模块为基站模式
						{
							DECA_WorkMode = DECA_ANCHOR_SLAVER;
							USART_RX_STA = 0;
							OSTaskResume(UWB_MODECHOOSE_PRIO);
						}
					}
					else
						OSTaskResume(USART1_TXDATA_PRIO);
				}
			}
			else //还未接收到0X0D
			{	
				if(Res == 0x0d)
					USART_RX_STA |= 0x4000;
				else
				{
					USART1RxBuff[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据溢出，开始数据覆盖	  
				}		 
			}
		}
		
		OSTaskSuspend(OS_PRIO_SELF);
	}
}

void USART1_TxData(void *pdata)
{
	uint16_t len = 1;
	uint8_t i;
	for(;;)
	{
		len = USART_RX_STA & 0x3FFF;
		RS485_TX;
		for(i = 0;i < len;i++)
		{
			huart1.Instance->TDR = USART1RxBuff[i];
			while(__HAL_UART_GET_FLAG(USART,UART_FLAG_TC) == RESET);
		}
		USART_RX_STA = 0;
//		OSTimeDly(1000);
		RS485_RX;
		OSTaskSuspend(OS_PRIO_SELF);
	}
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
