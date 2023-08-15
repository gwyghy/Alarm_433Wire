/*
*********************************************************************************************************
*
*	模块名称 : I2C总线驱动模块
*	文件名称 : bsp_i2c.h
*	版    本 : V1.0
*	说    明 : 头文件。
*
*	Copyright (C), 2012-2013, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_IIC_H
#define _BSP_IIC_H


#include "string.h"
#include "stdlib.h"	
//#include "stm32f4xx.h"
//#include "stm32f4xx_hal.h"
#include "gd32f30x.h"


#define RCU_PORT_I2C1_SCL			RCU_GPIOC
#define GPIO_PORT_I2C1_SCL			GPIOC					/* GPIO端口 */
#define I2C_SCL_PIN_1				GPIO_PIN_1				/* 连接到SCL时钟线的GPIO */

#define RCU_PORT_I2C1_SDA			RCU_GPIOC
#define GPIO_PORT_I2C1_SDA			GPIOC					/* GPIO端口 */
#define I2C_SDA_PIN_1				GPIO_PIN_0				/* 连接到SDA数据线的GPIO */

#define RCU_PORT_I2C2_SCL			RCU_GPIOC
#define GPIO_PORT_I2C2_SCL			GPIOC					/* GPIO端口 */
#define I2C_SCL_PIN_2				GPIO_PIN_6				/* 连接到SCL时钟线的GPIO */

#define RCU_PORT_I2C2_SDA			RCU_GPIOB
#define GPIO_PORT_I2C2_SDA			GPIOB					/* GPIO端口 */
#define I2C_SDA_PIN_2				GPIO_PIN_15				/* 连接到SDA数据线的GPIO */

#define MOS_ENABLED				1
#if MOS_ENABLED == 1			//MOS管控制输出逻辑电平相反
	/* 定义读写SCK1和SDA1的宏 */
	#define I2C_SCL_1_1()  GPIO_BC(GPIO_PORT_I2C1_SCL) = (uint32_t)I2C_SCL_PIN_1			/* SCL = 0 */
	#define I2C_SCL_1_0()  GPIO_BOP(GPIO_PORT_I2C1_SCL) = (uint32_t)I2C_SCL_PIN_1			/* SCL = 1 */

	#define I2C_SDA_1_1()  GPIO_BC(GPIO_PORT_I2C1_SDA) = (uint32_t)I2C_SDA_PIN_1			/* SDA = 0 */
	#define I2C_SDA_1_0()  GPIO_BOP(GPIO_PORT_I2C1_SDA) = (uint32_t)I2C_SDA_PIN_1 			/* SDA = 1 */

	/* 定义读写SCL2和SDA2的宏 */
	#define I2C_SCL_2_1()  GPIO_BC(GPIO_PORT_I2C2_SCL) = (uint32_t)I2C_SCL_PIN_2			/* SCL = 0 */
	#define I2C_SCL_2_0()  GPIO_BOP(GPIO_PORT_I2C2_SCL) = (uint32_t)I2C_SCL_PIN_2			/* SCL = 1 */

	#define I2C_SDA_2_1()  GPIO_BC(GPIO_PORT_I2C2_SDA) = (uint32_t)I2C_SDA_PIN_2			/* SDA = 0 */
	#define I2C_SDA_2_0()  GPIO_BOP(GPIO_PORT_I2C2_SDA) = (uint32_t)I2C_SDA_PIN_2			/* SDA = 1 */
#else
	/* 定义读写SCK1和SDA1的宏 */
	#define I2C_SCL_1_0()  GPIO_BC(GPIO_PORT_I2C1_SCL) = (uint32_t)I2C_SCL_PIN_1			/* SCL = 0 */
	#define I2C_SCL_1_1()  GPIO_BOP(GPIO_PORT_I2C1_SCL) = (uint32_t)I2C_SCL_PIN_1			/* SCL = 1 */

	#define I2C_SDA_1_0()  GPIO_BC(GPIO_PORT_I2C1_SDA) = (uint32_t)I2C_SDA_PIN_1			/* SDA = 0 */
	#define I2C_SDA_1_1()  GPIO_BOP(GPIO_PORT_I2C1_SDA) = (uint32_t)I2C_SDA_PIN_1 			/* SDA = 1 */

	/* 定义读写SCL2和SDA2的宏 */
	#define I2C_SCL_2_0()  GPIO_BC(GPIO_PORT_I2C2_SCL) = (uint32_t)I2C_SCL_PIN_2			/* SCL = 0 */
	#define I2C_SCL_2_1()  GPIO_BOP(GPIO_PORT_I2C2_SCL) = (uint32_t)I2C_SCL_PIN_2			/* SCL = 1 */

	#define I2C_SDA_2_0()  GPIO_BC(GPIO_PORT_I2C2_SDA) = (uint32_t)I2C_SDA_PIN_2			/* SDA = 0 */
	#define I2C_SDA_2_1()  GPIO_BOP(GPIO_PORT_I2C2_SDA) = (uint32_t)I2C_SDA_PIN_2			/* SDA = 1 */
#endif

void bsp_InitSWD(uint8_t device);
void SW_SendByte(uint8_t device, uint16_t _ucByte);

void DM633_LATCH(uint8_t device);
void DM634_GBC(uint8_t device, uint8_t _ucByte);

void DM633_Internal_GCK(uint8_t device);
void DM633_External_GCK(void);
#endif


