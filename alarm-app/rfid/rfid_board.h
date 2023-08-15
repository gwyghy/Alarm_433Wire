/*********************************************************************************************************************************
** 文件名:  rfid_board.h
** 描　述:  RFID应用接口头文件
**************************Copyright (c) 1998-1999 天津华宁电子技术有限公司技术开发部************************************************/
#ifndef __RFID_BOARD_H__
#define __RFID_BOARD_H__

#include "includes.h"

/***********************************************************************************************/
//借用和修改自电液控开发v1.0。SPI2同于与RFID通信
//马如意定义以下部分
/*************************RFID接口*************************/
/*** SPI RFID接口 **************/
/**SPI RFID所使用的硬件定义***/
#define RFID_SPI						SPI2//rfid所使用的SPI口定义
#define GPIO_RFID_AF_DEFINE				GPIO_AF_SPI2//复用功能定义
#define RCC_RFID_APBxCmd()				RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE)//SPI使能时钟，依据属于的APB1\APB2进行修改
//#define RCC_RFID_AF_APBxCmd()			RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

/**输出端口、引脚、复用引脚(GPIO_Pin_sources)、时钟定义*****/
#define GPIO_RFID_SCLK		  			GPIOC
#define PIN_RFID_SCLK					GPIO_PIN_10				// GPIO_Pin_10
#define GPIO_RFID_PINSOURCE_SCLK		GPIO_PinSource10
#define RCC_RFID_SCLK					RCU_GPIOC	//RCC_AHB1Periph_GPIOB

#define GPIO_RFID_MOSI		  			GPIOC					//GPIOC
#define PIN_RFID_MOSI					GPIO_PIN_12				//GPIO_Pin_3
#define GPIO_RFID_PINSOURCE_MOSI		GPIO_PinSource12
#define RCC_RFID_MOSI					RCU_GPIOC	//RCC_AHB1Periph_GPIOC

#define GPIO_RFID_MISO		  			GPIOC					//GPIOC
#define PIN_RFID_MISO					GPIO_PIN_11				//GPIO_Pin_2
#define GPIO_RFID_PINSOURCE_MISO		GPIO_PinSource11
#define RCC_RFID_MIS0					RCU_GPIOC	//RCC_AHB1Periph_GPIOC

#define GPIO_RFID_CS	 				GPIOA					//GPIOF
#define PIN_RFID_CS						GPIO_PIN_15				//GPIO_Pin_10
#define RCC_RFID_CS						RCU_GPIOA	//RCC_AHB1Periph_GPIOF

/**输入端口、引脚、时钟定义*****/
#define GPIO_RFID_GDO2					GPIOD					//GPIOF
#define PIN_RFID_GDO2					GPIO_PIN_2				//GPIO_Pin_9
#define RCC_RFID_GDO2					RCU_GPIOD	//RCC_AHB1Periph_GPIOF

#define GPIO_RFID_GDO0					GPIOB					//GPIOF
#define PIN_RFID_GDO0					GPIO_PIN_3				//GPIO_Pin_8
#define RCC_RFID_GDO0					RCU_GPIOB	//RCC_AHB1Periph_GPIOF
/**RFID SPI最大支持10M***/
#define RFID_SPI_BAUNDRATE_PRESCALER	SPI_PSC_8//时钟分频系数.APB1:42M/16 = 1.3125 MHz

/*******************中断定义***********************/
#define RFID_GDO0_EXTI_PORT_SOURCE		GPIO_PortSourceGPIOB	//EXTI_PortSourceGPIOF//GPIO_PortSourceGPIOC
#define RFID_GDO2_EXTI_PORT_SOURCE		GPIO_PortSourceGPIOD	//EXTI_PortSourceGPIOF//GPIO_PortSourceGPIOC
#define RFID_GDO0_EXTI_PIN_SOURCE		GPIO_PinSource3
#define RFID_GDO2_EXTI_PIN_SOURCE		GPIO_PinSource2
#define RFID_GDO0_EXTI_LINE				EXTI_3
#define RFID_GDO2_EXTI_LINE				EXTI_2
#define RFID_GDO0_EXTI_IRQn				EXTI3_IRQn
#define RFID_GDO2_EXTI_IRQn				EXTI2_IRQn
#define RFID_GDO0_EXTI_IRQHandler		EXTI3_IRQHandler
#define RFID_GDO2_EXTI_IRQHandler		EXTI2_IRQHandler
/*******************结束借用定义*******************/
/*****************************************************************************************************************/

/*
 * RFID - SPI 对应MCU管脚宏定义
 */
#define RFID_SPI_MCU_CS_PIN				PIN_RFID_CS
#define RFID_SPI_MCU_SCK_PIN			PIN_RFID_SCLK
#define RFID_SPI_MCU_MISO_PIN			PIN_RFID_MISO
#define RFID_SPI_MCU_MOSI_PIN			PIN_RFID_MOSI
#define RFID_SPI_MCU_GDO2_PIN			PIN_RFID_GDO2
#define RFID_SPI_MCU_GDO0_PIN			PIN_RFID_GDO0
/*
 * RFID - SPI 对应端口宏定义
 */
#define RFID_SPI_MCU_CS_PORT			GPIO_RFID_CS			
#define RFID_SPI_MCU_SCK_PORT			GPIO_RFID_SCLK
#define RFID_SPI_MCU_MISO_PORT			GPIO_RFID_MISO
#define RFID_SPI_MCU_MOSI_PORT			GPIO_RFID_MOSI
#define RFID_SPI_MCU_GDO2_PORT			GPIO_RFID_GDO2
#define RFID_SPI_MCU_GDO0_PORT			GPIO_RFID_GDO0
/*
 * RFID - SPI 对应端口时钟宏定义
 */
#define RFID_SPI_MCU_PORT_RCC			RCC_AHB1Periph_GPIOC
#define RFID_SPI_MCU_RCC				RCC_APB1Periph_SPI2
#define RFID_SPI_MCU_AFIO_RCC										//RCC_APB2Periph_AFIO
#define RFID_GDO_MCU_RCC				RCC_RFID_GDO
#define RFID_SPI_PORT					RFID_SPI
/*
 * GDOx中断配置
 */
#define RFID_CC1101_GDOx_PORT_SOURCE	RFID_GDOx_EXTI_PORT_SOURCE
#define RFID_CC1101_GDO0_PIN_SOURCE		RFID_GDO0_EXTI_PIN_SOURCE
#define RFID_CC1101_GDO2_PIN_SOURCE		RFID_GDO2_EXTI_PIN_SOURCE
#define RFID_CC1101_GDO0_EXTI_LINE		RFID_GDO0_EXTI_LINE
#define RFID_CC1101_GDO2_EXTI_LINE		RFID_GDO2_EXTI_LINE
#define RFID_CC1101_GDOx_EXTI_IRQn		RFID_GDOx_EXTI_IRQn

#endif /* __RFID_BOARD_H__*/
/*********************************天津华宁电子有限公司*************************************************************/
