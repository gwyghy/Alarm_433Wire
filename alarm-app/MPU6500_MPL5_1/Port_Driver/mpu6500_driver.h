/********************************************************************************
* �ļ����ƣ�	mpu6500_driver.h
* ��	�ߣ�	�˳���   
* ��ǰ�汾��   	V1.0
* ������ڣ�    2021.07.06
* ��������: 	��������ǵ�����(IOģ��I2C)��ʵ���豸��ʼ��(��mpu6500��DMP)�����ݶ�д�Ȳ�����
* ��ʷ��Ϣ��   
*           	�汾��Ϣ     ���ʱ��      ԭ����        ע��
*
*       >>>>  �ڹ����е�λ��  <<<<
*          	  3-Ӧ�ò�
*           �� 2-Э���
*             1-Ӳ��������
*********************************************************************************
* Copyright (c) 2014,������������޹�˾ All rights reserved.
*********************************************************************************/
#ifndef __MPU6500_DRIVER_H
#define __MPU6500_DRIVER_H

/********************************************************************************
* .hͷ�ļ�
*********************************************************************************/
#include "string.h"
#include "stdlib.h"
#include "gd32f30x.h"

//IO��������
#if STM32 
#define MPU6500_I2C_SetOut_Mode()	{GPIOB->MODER &= ~(3 << (7*2)); GPIOB->MODER |= 1 << 7*2;}							//PB7����Ϊ�������
#define MPU6500_I2C_SetIn_Mode()	{GPIOB->MODER &= ~(3 << (7*2)); GPIOB->MODER |= 0 << 7*2; GPIOB->ODR |= 0X01 < 7;}	//PB7����Ϊ��������	
//#define  MPU6500_I2C_SetOut_Mode()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=3<<28;}				//PB7����Ϊ�������
//#define  MPU6500_I2C_SetIn_Mode() 	{GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=8<<28;GPIOB->ODR|=0X01<7;}				//PB7����Ϊ��������	
#endif


#define MPU6500_I2C_GPIO_PORT				GPIOB			      /* GPIO�˿� */
#define MPU6500_I2C_SCL_PIN					GPIO_PIN_6			  /* ���ӵ�SCL��GPIO */
#define MPU6500_I2C_SDA_PIN					GPIO_PIN_7			  /* ���ӵ�SDA��GPIO */

#define MPU6500_I2C_SetOut_Mode()	{gpio_init(MPU6500_I2C_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, MPU6500_I2C_SDA_PIN);}		//PB7����Ϊ�������
#define MPU6500_I2C_SetIn_Mode()	{gpio_init(MPU6500_I2C_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, MPU6500_I2C_SDA_PIN);}			//PB7����Ϊ��������	


#define MPU6500_INT_PORT			GPIOB		//MPU6500_INT
#define MPU6500_INT_PIN				GPIO_PIN_8
#define RCC_MPU6500_INT				RCC_APB2Periph_GPIOB

#define MPU6500_EXTI_LINE			EXTI_8

#define MPU6500_EXTI_IRQn			EXTI5_9_IRQn
#define MPU6500_EXTI_IRQ_PRIO		0x0f

/*MPU6500�ж�����*/
#define MPU6500_IRQHandler			EXTI5_9_IRQHandler

#if STM32 
#define I2C_SDA_READ_2()		((MPU6500_I2C_GPIO_PORT->IDR & MPU6500_I2C_SDA_PIN) != 0)	/* ��SDA����״̬ */
#define MPU6500_I2C_SDA_1()		MPU6500_I2C_GPIO_PORT->BSRR = MPU6500_I2C_SDA_PIN			/* SDA = 1 */
#define MPU6500_I2C_SDA_0()		MPU6500_I2C_GPIO_PORT->BSRR = (uint32_t)MPU6500_I2C_SDA_PIN << 16U			/* SDA = 0 */

#define MPU6500_I2C_SCL_1()		MPU6500_I2C_GPIO_PORT->BSRR = MPU6500_I2C_SCL_PIN							/* SCL = 1 */
#define MPU6500_I2C_SCL_0()		MPU6500_I2C_GPIO_PORT->BSRR = (uint32_t)MPU6500_I2C_SCL_PIN << 16U			/* SCL = 0 */
#endif

#define MPU6500_I2C_SDA_RE 		gpio_input_bit_get(MPU6500_I2C_GPIO_PORT, MPU6500_I2C_SDA_PIN)			//SDA����	
#define MPU6500_I2C_SDA_1()		gpio_bit_write(MPU6500_I2C_GPIO_PORT, MPU6500_I2C_SDA_PIN, SET)			/* SDA = 1 */
#define MPU6500_I2C_SDA_0()		gpio_bit_write(MPU6500_I2C_GPIO_PORT, MPU6500_I2C_SDA_PIN, RESET)		/* SDA = 0 */

#define MPU6500_I2C_SCL_1()		gpio_bit_write(MPU6500_I2C_GPIO_PORT, MPU6500_I2C_SCL_PIN, SET)			/* SCL = 1 */
#define MPU6500_I2C_SCL_0()		gpio_bit_write(MPU6500_I2C_GPIO_PORT, MPU6500_I2C_SCL_PIN, RESET)		/* SCL = 0 */

#define MPU6500_device_addr     0x68
#define MPU6500_ID				0x70

//��������ٶ�
#define DEFAULT_MPU_HZ 			100		//100Hz

//****************************************
// ����MPU6050�ڲ���ַ
//****************************************
#define	SMPLRT_DIV		0x19	//�����ǲ����ʣ�����ֵ��0x07(125Hz)
#define	CONFIG			0x1A	//��ͨ�˲�Ƶ�ʣ�����ֵ��0x06(5Hz)
#define	GYRO_CONFIG		0x1B	//�������Լ켰������Χ������ֵ��0x18(���Լ죬2000deg/s)
#define	ACCEL_CONFIG	0x1C	//���ټ��Լ졢������Χ����ͨ�˲�Ƶ�ʣ�����ֵ��0x01(���Լ죬2G��5Hz)

#define FIFO_EN_REG		0X23	//FIFOʹ�ܼĴ���
#define INTBP_CFG_REG	0X37	//�ж�/��·���üĴ���
#define INT_EN_REG		0X38	//�ж�ʹ�ܼĴ���

#define	ACCEL_XOUT_H	0x3B	//���ٶ�ֵ,X���8λ�Ĵ���
#define	ACCEL_XOUT_L	0x3C	//���ٶ�ֵ,X���8λ�Ĵ���
#define	ACCEL_YOUT_H	0x3D	//���ٶ�ֵ,Y���8λ�Ĵ���
#define	ACCEL_YOUT_L	0x3E	//���ٶ�ֵ,Y���8λ�Ĵ���
#define	ACCEL_ZOUT_H	0x3F	//���ٶ�ֵ,Z���8λ�Ĵ���
#define	ACCEL_ZOUT_L	0x40	//���ٶ�ֵ,Z���8λ�Ĵ���

#define	TEMP_OUT_H		0x41	//�¶�ֵ�߰�λ�Ĵ���
#define	TEMP_OUT_L		0x42	//�¶�ֵ��8λ�Ĵ���

#define	GYRO_XOUT_H		0x43	//������ֵ,X���8λ�Ĵ���
#define	GYRO_XOUT_L		0x44	//������ֵ,X���8λ�Ĵ���	
#define	GYRO_YOUT_H		0x45	//������ֵ,Y���8λ�Ĵ���
#define	GYRO_YOUT_L		0x46	//������ֵ,Y���8λ�Ĵ���
#define	GYRO_ZOUT_H		0x47	//������ֵ,Z���8λ�Ĵ���
#define	GYRO_ZOUT_L		0x48	//������ֵ,Z���8λ�Ĵ���

#define USER_CTRL_REG	0X6A	//�û����ƼĴ���
#define	PWR_MGMT_1		0x6B	//��Դ��������ֵ��0x00(��������)
#define PWR_MGMT_2   	0X6C	//��Դ����Ĵ���2 
#define	WHO_AM_I		0x75	//����ID�Ĵ���(MPU6500����0x70��ֻ��)

#define	SlaveAddress	0xD0	//IICд��ʱ�ĵ�ַ�ֽ����ݣ�+1Ϊ��ȡ

/********************************************************************************
* ȫ�ֱ�������
*********************************************************************************/
extern u8 MPU_EXTI_flag;		//MPU6500���������ʱ���Ų����ж�

/********************************************************************************
* ��������
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
