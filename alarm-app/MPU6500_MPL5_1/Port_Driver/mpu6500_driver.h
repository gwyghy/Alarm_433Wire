/********************************************************************************
* 文件名称：	mpu6500_driver.h
* 作	者：	潘成勇   
* 当前版本：   	V1.0
* 完成日期：    2021.07.06
* 功能描述: 	完成陀螺仪的驱动(IO模拟I2C)，实现设备初始化(含mpu6500和DMP)、数据读写等操作。
* 历史信息：   
*           	版本信息     完成时间      原作者        注释
*
*       >>>>  在工程中的位置  <<<<
*          	  3-应用层
*           √ 2-协议层
*             1-硬件驱动层
*********************************************************************************
* Copyright (c) 2014,天津华宁电子有限公司 All rights reserved.
*********************************************************************************/
#ifndef __MPU6500_DRIVER_H
#define __MPU6500_DRIVER_H

/********************************************************************************
* .h头文件
*********************************************************************************/
#include "string.h"
#include "stdlib.h"
#include "gd32f30x.h"

//IO方向设置
#if STM32 
#define MPU6500_I2C_SetOut_Mode()	{GPIOB->MODER &= ~(3 << (7*2)); GPIOB->MODER |= 1 << 7*2;}							//PB7：设为推挽输出
#define MPU6500_I2C_SetIn_Mode()	{GPIOB->MODER &= ~(3 << (7*2)); GPIOB->MODER |= 0 << 7*2; GPIOB->ODR |= 0X01 < 7;}	//PB7：设为上拉输入	
//#define  MPU6500_I2C_SetOut_Mode()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=3<<28;}				//PB7：设为推挽输出
//#define  MPU6500_I2C_SetIn_Mode() 	{GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=8<<28;GPIOB->ODR|=0X01<7;}				//PB7：设为上拉输入	
#endif


#define MPU6500_I2C_GPIO_PORT				GPIOB			      /* GPIO端口 */
#define MPU6500_I2C_SCL_PIN					GPIO_PIN_6			  /* 连接到SCL的GPIO */
#define MPU6500_I2C_SDA_PIN					GPIO_PIN_7			  /* 连接到SDA的GPIO */

#define MPU6500_I2C_SetOut_Mode()	{gpio_init(MPU6500_I2C_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, MPU6500_I2C_SDA_PIN);}		//PB7：设为推挽输出
#define MPU6500_I2C_SetIn_Mode()	{gpio_init(MPU6500_I2C_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, MPU6500_I2C_SDA_PIN);}			//PB7：设为上拉输入	


#define MPU6500_INT_PORT			GPIOB		//MPU6500_INT
#define MPU6500_INT_PIN				GPIO_PIN_8
#define RCC_MPU6500_INT				RCC_APB2Periph_GPIOB

#define MPU6500_EXTI_LINE			EXTI_8

#define MPU6500_EXTI_IRQn			EXTI5_9_IRQn
#define MPU6500_EXTI_IRQ_PRIO		0x0f

/*MPU6500中断向量*/
#define MPU6500_IRQHandler			EXTI5_9_IRQHandler

#if STM32 
#define I2C_SDA_READ_2()		((MPU6500_I2C_GPIO_PORT->IDR & MPU6500_I2C_SDA_PIN) != 0)	/* 读SDA口线状态 */
#define MPU6500_I2C_SDA_1()		MPU6500_I2C_GPIO_PORT->BSRR = MPU6500_I2C_SDA_PIN			/* SDA = 1 */
#define MPU6500_I2C_SDA_0()		MPU6500_I2C_GPIO_PORT->BSRR = (uint32_t)MPU6500_I2C_SDA_PIN << 16U			/* SDA = 0 */

#define MPU6500_I2C_SCL_1()		MPU6500_I2C_GPIO_PORT->BSRR = MPU6500_I2C_SCL_PIN							/* SCL = 1 */
#define MPU6500_I2C_SCL_0()		MPU6500_I2C_GPIO_PORT->BSRR = (uint32_t)MPU6500_I2C_SCL_PIN << 16U			/* SCL = 0 */
#endif

#define MPU6500_I2C_SDA_RE 		gpio_input_bit_get(MPU6500_I2C_GPIO_PORT, MPU6500_I2C_SDA_PIN)			//SDA输入	
#define MPU6500_I2C_SDA_1()		gpio_bit_write(MPU6500_I2C_GPIO_PORT, MPU6500_I2C_SDA_PIN, SET)			/* SDA = 1 */
#define MPU6500_I2C_SDA_0()		gpio_bit_write(MPU6500_I2C_GPIO_PORT, MPU6500_I2C_SDA_PIN, RESET)		/* SDA = 0 */

#define MPU6500_I2C_SCL_1()		gpio_bit_write(MPU6500_I2C_GPIO_PORT, MPU6500_I2C_SCL_PIN, SET)			/* SCL = 1 */
#define MPU6500_I2C_SCL_0()		gpio_bit_write(MPU6500_I2C_GPIO_PORT, MPU6500_I2C_SCL_PIN, RESET)		/* SCL = 0 */

#define MPU6500_device_addr     0x68
#define MPU6500_ID				0x70

//定义输出速度
#define DEFAULT_MPU_HZ 			100		//100Hz

//****************************************
// 定义MPU6050内部地址
//****************************************
#define	SMPLRT_DIV		0x19	//陀螺仪采样率，典型值：0x07(125Hz)
#define	CONFIG			0x1A	//低通滤波频率，典型值：0x06(5Hz)
#define	GYRO_CONFIG		0x1B	//陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
#define	ACCEL_CONFIG	0x1C	//加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)

#define FIFO_EN_REG		0X23	//FIFO使能寄存器
#define INTBP_CFG_REG	0X37	//中断/旁路设置寄存器
#define INT_EN_REG		0X38	//中断使能寄存器

#define	ACCEL_XOUT_H	0x3B	//加速度值,X轴高8位寄存器
#define	ACCEL_XOUT_L	0x3C	//加速度值,X轴低8位寄存器
#define	ACCEL_YOUT_H	0x3D	//加速度值,Y轴高8位寄存器
#define	ACCEL_YOUT_L	0x3E	//加速度值,Y轴低8位寄存器
#define	ACCEL_ZOUT_H	0x3F	//加速度值,Z轴高8位寄存器
#define	ACCEL_ZOUT_L	0x40	//加速度值,Z轴低8位寄存器

#define	TEMP_OUT_H		0x41	//温度值高八位寄存器
#define	TEMP_OUT_L		0x42	//温度值低8位寄存器

#define	GYRO_XOUT_H		0x43	//陀螺仪值,X轴高8位寄存器
#define	GYRO_XOUT_L		0x44	//陀螺仪值,X轴低8位寄存器	
#define	GYRO_YOUT_H		0x45	//陀螺仪值,Y轴高8位寄存器
#define	GYRO_YOUT_L		0x46	//陀螺仪值,Y轴低8位寄存器
#define	GYRO_ZOUT_H		0x47	//陀螺仪值,Z轴高8位寄存器
#define	GYRO_ZOUT_L		0x48	//陀螺仪值,Z轴低8位寄存器

#define USER_CTRL_REG	0X6A	//用户控制寄存器
#define	PWR_MGMT_1		0x6B	//电源管理，典型值：0x00(正常启用)
#define PWR_MGMT_2   	0X6C	//电源管理寄存器2 
#define	WHO_AM_I		0x75	//器件ID寄存器(MPU6500读回0x70，只读)

#define	SlaveAddress	0xD0	//IIC写入时的地址字节数据，+1为读取

/********************************************************************************
* 全局变量声明
*********************************************************************************/
extern u8 MPU_EXTI_flag;		//MPU6500有数据输出时引脚产生中断

/********************************************************************************
* 函数声明
*********************************************************************************/
void MPU6500_Port_EXIT_Init(void);
void MPU6500_I2C_PORT_Init(void);
void MPU6500_I2C_delay(void);
void MPU6500_I2C_start(void);
void MPU6500_I2C_stop(void);
u8 MPU6500_I2C_check_ack(void);
void MPU6500_I2C_ack(void);
void MPU6500_I2C_NoAck(void);
void MPU6500_I2C_write_char(u8 dat);
u8 MPU6500_I2C_read_char(void);
u8 MPU6500_write_byte(u8 reg, u8 data);
u8 MPU6500_read_byte(u8 reg);
u8 MPU6500_Read_Len(u8 DeviceAddr, u8 RegAddr, u8 len, u8 *pbuff);
u8 MPU6500_Write_Len(u8 DeviceAddr, u8 RegAddr, u8 len, u8 *pbuff);
u8 InitMPU6050(void);
short GetData(u8 REG_Address);
u8 MPU6500_Get_Accelerometer(short *ax, short *ay, short *az);
u8 MPU6500_Get_Gyroscope(short *gx, short *gy, short *gz);
float MPU6500_GetTemperature(void);
u8 MPU6500_Set_Gyro_Fsr(u8 fsr);
u8 MPU6500_Set_Accel_Fsr(u8 fsr);
u8 MPU6500_Set_Rate(u16 rate);
u8 MPU6500_Set_LPF(u16 lpf);
u8 MPU6500_run_self_test(void);
u8 MPU6500_DMP_Init(void);
s8 MPU6500_dmp_get_euler_angle(short *accel, short *gyro, float *pitch, float *roll, float *yaw);



#endif
