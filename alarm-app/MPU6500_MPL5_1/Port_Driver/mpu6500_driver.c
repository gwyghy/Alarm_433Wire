/********************************************************************************
* �ļ����ƣ�	mpu6500_driver.c
* ��	�ߣ�	�˳���   
* ��ǰ�汾��   	V1.0
* ������ڣ�    2021.07.06
* ��������: 	��������ǵ�����(IOģ��I2C)��ʵ���豸��ʼ��(��MPU6500��DMP)�����ݶ�д�Ȳ�����
* ��ʷ��Ϣ��   
*           	�汾��Ϣ     ���ʱ��      ԭ����        ע��
*
*       >>>>  �ڹ����е�λ��  <<<<
*          	  3-Ӧ�ò�
*             2-Э���
*           �� 1-Ӳ��������
*********************************************************************************
* Copyright (c) 2014,������������޹�˾ All rights reserved.
*********************************************************************************/
/********************************************************************************
* .hͷ�ļ�
*********************************************************************************/
#include "mpu6500_driver.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "inv_mpu.h"
#include "math_fun.h"
#include "dmpKey.h"
#include "dmpmap.h"
#include "delay.h"
#include "math.h"

u8 MPU_EXTI_flag = 0;		//MPU6500�����жϷ�����־λ


/* Platform-specific information. Kinda like a boardfile. */
struct platform_data_s {
    signed char orientation[9];
};

/* The sensors can be mounted onto the board in any orientation. The mounting
 * matrix seen below tells the MPL how to rotate the raw data from the
 * driver(s).
 * TODO: The following matrices refer to the configuration on internal test
 * boards at Invensense. If needed, please modify the matrices to match the
 * chip-to-body matrix for your particular set up.
 */
//�����Ƿ�������
//�����Ǻ��Ӻú���X��Y��Z�᷽��͹̶��ˣ���ͨ������������������3������Pitch��Roll��Yaw��3���ǶȵĶ�Ӧ��ϵ���Լ�˳ʱ�뻹����ʱ��ʱ�Ƕ�����
//��һ�ж���X���Ӧ�ĸ��Ƕȣ�1 0 0 -> X���ӦPitch��0 1 0 -> X���ӦRoll��0 0 1 -> X���ӦYaw
//�ڶ��ж���Y���Ӧ�ĸ��Ƕȣ�1 0 0 -> Y���ӦPitch��0 1 0 -> Y���ӦRoll��0 0 1 -> Y���ӦYaw
//�����ж���Z���Ӧ�ĸ��Ƕȣ�1 0 0 -> Z���ӦPitch��0 1 0 -> Z���ӦRoll��0 0 1 -> Z���ӦYaw
//��ֵ1��ʾ˳ʱ�뷽��ת��ʱ�Ƕ����ӣ���ֵ-1��ʾ��ʱ�뷽��ת��ʱ�Ƕ�����(�����˳/��ʱ���Ǵ�оƬ�����������ῴ��ȥ)
static struct platform_data_s gyro_pdata = {
		
		 .orientation = {0, -1, 0,		//X���ӦRoll,��ʱ��Ƕ�����
						 1, 0, 0,		//Y���ӦPitch,˳ʱ��Ƕ�����
						 0, 0, 1}		//Z���ӦYaw,˳ʱ��Ƕ�����

//		 .orientation = {1, 0, 0,		//X���ӦPitch,˳ʱ��Ƕ�����
//                     0, 1, 0,			//Y���ӦRoll,˳ʱ��Ƕ�����
//                     0, 0, 1}			//Z���ӦYaw,˳ʱ��Ƕ�����
};


/*    Axis Transformation matrix
|r11  r12  r13| |vx|     |v'x|
|r21  r22  r23| |vy|  =  |v'y|
|r31  r32  r33| |vz|     |v'z|

v'x  = {(r11  * vx) +( r12 * vy) +( r13 * vz)} 
v'y  = {(r21  * vx) +( r22 * vy) +( r23 * vz)}
v'z  = {(r31  * vx) +( r32 * vy) +( r33 * vz)}
*/

//����MPU6500���ж�IO��PB8
void MPU6500_Port_EXIT_Init(void)
{
	#if STM32 
	GPIO_InitTypeDef GPIO_InitStructure;
//	EXTI_InitTypeDef EXTI_InitStrue;
//	NVIC_InitTypeDef NVIC_InitStrue;
	
	/* Input clocks enable */
	MPU6500_IRQ_GPIO_CLK_ENABLE();
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_MPU6500_INT, ENABLE);	  //ʹ��ʱ��
	
	/* Configure MPU6500_INT pin */
	GPIO_InitStructure.Pin = MPU6500_INT_PIN;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(MPU6500_INT_PORT, &GPIO_InitStructure);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(MPU6500_EXTI_IRQn, 0x0f, 0);
	HAL_NVIC_EnableIRQ(MPU6500_EXTI_IRQn);

//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource4);
//	
//	EXTI_InitStrue.EXTI_Line = EXTI_Line4;					//ѡ���ж���·4
//	EXTI_InitStrue.EXTI_LineCmd = ENABLE;					//�ⲿ�ж�ʹ��
//	EXTI_InitStrue.EXTI_Mode = EXTI_Mode_Interrupt;			//����Ϊ�ж����󣬷��¼�����
//	EXTI_InitStrue.EXTI_Trigger = EXTI_Trigger_Falling;		//�����жϴ�����ʽΪ���½��ش���
//	EXTI_Init(&EXTI_InitStrue);
//	
//	NVIC_InitStrue.NVIC_IRQChannel = EXTI4_IRQn;
//	NVIC_InitStrue.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_InitStrue.NVIC_IRQChannelPreemptionPriority = 0x0f;//��ʹ���ж����ȼ�Ƕ�ס���ΪSysTick���ж����ȼ�Ϊ0x0f
//	NVIC_InitStrue.NVIC_IRQChannelSubPriority = 2;			//0������SysTick
//	NVIC_Init(&NVIC_InitStrue);
	#endif
	/* enable the User MPU6500_INT_PORT GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_AF);
    
    /* configure MPU6500_INT_PORT pin as input */
    gpio_init(MPU6500_INT_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, MPU6500_INT_PIN);
    /* enable and set MPU6500_INT_PORT EXTI interrupt to the lowest priority */
    nvic_irq_enable(MPU6500_EXTI_IRQn, 0x0f, 0);
    /* connect key EXTI line to MPU6500_INT_PORT GPIO pin */
    gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_8);
    /* configure MPU6500_INT EXTI line */
    exti_init(MPU6500_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
	/* clear EXTI line */
    exti_interrupt_flag_clear(MPU6500_EXTI_LINE);
}

//��ʼ��IIC����
void MPU6500_I2C_PORT_Init(void)
{
	#if STM32
	GPIO_InitTypeDef GPIO_InitStructure;
	
	__HAL_RCC_GPIOB_CLK_ENABLE();
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//��ʹ������IO PORTBʱ�� 
	
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP; 		 		//�������
	GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7;	 		// �˿�����
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;			//IO���ٶ�Ϊ50MHz
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);					//�����趨������ʼ��GPIO 
	
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_SET);
//	GPIO_SetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_7);				//PB6,PB7 �����
	#endif
	
	rcu_periph_clock_enable(RCU_GPIOB);
	/* configure MPU6500_I2C GPIO port */ 
	gpio_init(MPU6500_I2C_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, MPU6500_I2C_SCL_PIN);
	gpio_init(MPU6500_I2C_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, MPU6500_I2C_SDA_PIN);
	gpio_bit_set(MPU6500_I2C_GPIO_PORT, MPU6500_I2C_SCL_PIN);
	gpio_bit_set(MPU6500_I2C_GPIO_PORT, MPU6500_I2C_SDA_PIN);
}

//����I2CͨѶʱ������ʱ
void MPU6500_I2C_delay(void)
{
	u16 i;
	/*��
	 	�����ʱ����ͨ���߼������ǲ��Եõ��ġ�
    ����������CPU��Ƶ72MHz ��MDK���뻷����0���Ż�
  
	ѭ������Ϊ10ʱ��SCLƵ�� = 205KHz 
	ѭ������Ϊ7ʱ��SCLƵ�� = 347KHz�� SCL�ߵ�ƽʱ��1.5us��SCL�͵�ƽʱ��2.87us 
	ѭ������Ϊ5ʱ��SCLƵ�� = 421KHz�� SCL�ߵ�ƽʱ��1.25us��SCL�͵�ƽʱ��2.375us 
	*/
	//��Ƶ72M i=30����Ƶ8M~72M��i=5�������Բ��ܲ���ϵͳʱ������
	//��Ƶ72M i=2,Լ��ʱ1us
	//200k   5  16M       380k   10   8M
	for(i = 0; i < 20; i ++)			//
		;
}

//����IIC��ʼ�ź�
void MPU6500_I2C_start(void)
{
	MPU6500_I2C_SCL_0();
	MPU6500_I2C_SetOut_Mode();
	
	MPU6500_I2C_SDA_1();
	MPU6500_I2C_SCL_1();
	MPU6500_I2C_delay();
	MPU6500_I2C_SDA_0();
	MPU6500_I2C_SCL_0();
	MPU6500_I2C_delay();
}

//����IICֹͣ�ź�
void MPU6500_I2C_stop(void)
{
	MPU6500_I2C_SCL_0();
	MPU6500_I2C_SetOut_Mode();
	
	MPU6500_I2C_SDA_0();
	MPU6500_I2C_SCL_1();
	MPU6500_I2C_delay();
	MPU6500_I2C_SDA_1();
	MPU6500_I2C_delay();
	
}

//�ȴ�IICӦ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 MPU6500_I2C_check_ack(void)
{	
	u32 delay_count = 0;
	
	MPU6500_I2C_SCL_0();
	MPU6500_I2C_SetIn_Mode();
	MPU6500_I2C_SDA_1();
	MPU6500_I2C_SCL_1();
	MPU6500_I2C_delay();

	while(MPU6500_I2C_SDA_RE)
	{
		delay_count++;
		if(delay_count > 0x3fff)
		{
			MPU6500_I2C_stop();
			return 1;
		}
	}
	
	MPU6500_I2C_SCL_0();
	MPU6500_I2C_delay();

	return 0;
}

//IIC����ACKӦ��
void MPU6500_I2C_ack(void)
{
	MPU6500_I2C_SCL_0();
	MPU6500_I2C_SetOut_Mode();
	
	MPU6500_I2C_SDA_0();
	MPU6500_I2C_SCL_1();
	MPU6500_I2C_delay();
	MPU6500_I2C_SCL_0();
	MPU6500_I2C_delay();
	MPU6500_I2C_SDA_1();
	
}

//IIC������ACKӦ��		    
void MPU6500_I2C_NoAck(void)
{
	MPU6500_I2C_SCL_0();
	MPU6500_I2C_SetOut_Mode();
	MPU6500_I2C_SDA_1();
	MPU6500_I2C_delay();
	MPU6500_I2C_SCL_1();
	MPU6500_I2C_delay();
	MPU6500_I2C_SCL_0();
	MPU6500_I2C_delay();
	
}

//IIC����һ���ֽڣ�ֻ��8bit
void MPU6500_I2C_write_char(u8 dat)
{
	u8 i = 0;
	
	MPU6500_I2C_SCL_0();
	MPU6500_I2C_SetOut_Mode();
	for(i = 0; i < 8; i ++)
	{
		if(dat&0x80) MPU6500_I2C_SDA_1();
		else MPU6500_I2C_SDA_0();
		MPU6500_I2C_delay();
		MPU6500_I2C_SCL_1();
		MPU6500_I2C_delay();
		MPU6500_I2C_SCL_0();
		dat <<= 1;
	}
}

//IIC����һ���ֽڣ�ֻ����8bit
u8 MPU6500_I2C_read_char(void)
{
	u8 i = 0, dat = 0;
	MPU6500_I2C_SCL_0();
	MPU6500_I2C_SetIn_Mode();
	for(i = 0; i < 8; i++)
	{
		dat <<= 1;
		MPU6500_I2C_SCL_1();
		MPU6500_I2C_delay();
		if(MPU6500_I2C_SDA_RE) 
		{
			dat |= 0x01;
		}
		MPU6500_I2C_SCL_0();
		MPU6500_I2C_delay();
	}
	return dat;
}

//IIC����һ���ֽڣ�����MPU6500Э�鷢��
u8 MPU6500_write_byte(u8 reg, u8 data)
{
	MPU6500_I2C_start();
	MPU6500_I2C_write_char(MPU6500_device_addr << 1 | 0x00);
	if(MPU6500_I2C_check_ack())
	{
		MPU6500_I2C_stop();
		return 2;
	}
	
	MPU6500_I2C_write_char(reg);
	MPU6500_I2C_check_ack();
	MPU6500_I2C_write_char(data);
	MPU6500_I2C_check_ack();
	MPU6500_I2C_stop();
	return 0;
}

//IIC����һ���ֽڣ�����MPU6500Э�����
u8 MPU6500_read_byte(u8 reg)
{
	u8 data=0;
	
	MPU6500_I2C_start();
	MPU6500_I2C_write_char(MPU6500_device_addr << 1 | 0x00);
	if(MPU6500_I2C_check_ack())
	{
		MPU6500_I2C_stop();
		return 2;
	}
	MPU6500_I2C_write_char(reg);
	MPU6500_I2C_check_ack();
	
	MPU6500_I2C_start();
	MPU6500_I2C_write_char(MPU6500_device_addr << 1 | 0x01);
	MPU6500_I2C_check_ack();
	data = MPU6500_I2C_read_char();
	MPU6500_I2C_ack();
	MPU6500_I2C_stop();
	
	return data;
}

//IIC������
//DeviceAddr:������ַ
//RegAddr:Ҫ��ȡ�ļĴ�����ַ
//len:Ҫ��ȡ�ĳ���
//pbuff:��ȡ�������ݴ洢��
//����ֵ:0,����
//    ����,�������
u8 MPU6500_Read_Len(u8 DeviceAddr, u8 RegAddr, u8 len, u8 *pbuff)
{
	u8 i;
	MPU6500_I2C_start();
	MPU6500_I2C_write_char(DeviceAddr << 1 | 0x00);
	if(MPU6500_I2C_check_ack())
	{
		MPU6500_I2C_stop();
		return 2;
	}
		
	MPU6500_I2C_write_char(RegAddr);
	MPU6500_I2C_check_ack();
	
	MPU6500_I2C_start();
	MPU6500_I2C_write_char(DeviceAddr << 1 | 0x01);
	MPU6500_I2C_check_ack();
	for(i = 0; i < len; i++)
	{
		*pbuff++ = MPU6500_I2C_read_char();	
		if(i + 1 >= len) {MPU6500_I2C_NoAck(); break;}
		MPU6500_I2C_ack();
	}
	
	MPU6500_I2C_stop();
	
	return 0;
}

//IIC����д
//DeviceAddr:������ַ 
//RegAddr:�Ĵ�����ַ
//len:д�볤��
//pbuff:������
//����ֵ:0,����
//    ����,�������
u8 MPU6500_Write_Len(u8 DeviceAddr, u8 RegAddr, u8 len, u8 *pbuff)
{
	u8 i;
	
	MPU6500_I2C_start();
	MPU6500_I2C_write_char(DeviceAddr << 1 | 0x00);
	if(MPU6500_I2C_check_ack())
	{
		MPU6500_I2C_stop();
		return 2;
	}

	MPU6500_I2C_write_char(RegAddr);
	MPU6500_I2C_check_ack();
	for(i = 0; i < len; i++)
	{
		MPU6500_I2C_write_char(*pbuff++);
		if(MPU6500_I2C_check_ack()) 
		{
			MPU6500_I2C_stop();
			return 3;
		}
	}
	
	MPU6500_I2C_stop();
	return 0;
}

/******************************************************************************************
* �������ƣ�u8 InitMPU6050(void)
* ������������ʼ��MPU6050
* ��ڲ�����none
* ���ڲ�����0,�ɹ�
			����,�������
* ʹ��˵������
*******************************************************************************************/	
u8 InitMPU6050(void)
{
	int i = 0, j = 0;
	u8 res = 0;
	MPU6500_I2C_PORT_Init();	//��ʼ��IIC����
	
  //�ڳ�ʼ��֮ǰҪ��ʱһ��ʱ�䣬��û����ʱ����ϵ�����ϵ����ݿ��ܻ����
	for(i = 0; i < 1000; i++)
	{
		for(j = 0; j < 15000; j++)
		{
			;
		}
	}
	
	MPU6500_write_byte(PWR_MGMT_1, 0x80);	//��λMPU6050
	delay_ms(100);
	MPU6500_write_byte(PWR_MGMT_1, 0);		//����MPU6050 
	MPU6500_Set_Gyro_Fsr(3);				//�����Ǵ�����,��2000dps
//	MPU6500_Set_Gyro_Fsr(0);				//�����Ǵ�����,��250dps
	MPU6500_Set_Accel_Fsr(0);				//���ٶȴ�����,��2g
	MPU6500_Set_Rate(50);					//���ò�����50Hz
	MPU6500_write_byte(INT_EN_REG, 0X00);	//�ر������ж�
	MPU6500_write_byte(USER_CTRL_REG, 0X00);	//I2C��ģʽ�ر�
	MPU6500_write_byte(FIFO_EN_REG, 0X00);	//�ر�FIFO
	MPU6500_write_byte(INTBP_CFG_REG, 0X80);	//INT���ŵ͵�ƽ��Ч
	res = MPU6500_read_byte(WHO_AM_I);
	if (res == MPU6500_ID)
	{
		MPU6500_write_byte(PWR_MGMT_1, 0X01);	//����CLKSEL,PLL X��Ϊ�ο�
		MPU6500_write_byte(PWR_MGMT_2, 0X00);	//���ٶ��������Ƕ�����
		MPU6500_Set_Rate(50);					//���ò�����Ϊ50Hz
	}
	else 
		return 1;
		
	return 0;
}

//��ȡMPU6500�Ĵ�������
//REG_Address���Ĵ�����ַ
//����ֵ���üĴ�����ַ��ŵĵ��ֽںͼĴ�����ַ+1��ŵĵ��ֽڣ���ɵ�˫�ֽ�����
short GetData(u8 REG_Address)
{
	u8 buff[2];
	MPU6500_Read_Len(MPU6500_device_addr, REG_Address, 2, buff);
	return ((buff[0]<<8) | buff[1]);   //�ϳ�����
}

//�õ����ٶ�ֵ(ԭʼֵ)
//gx,gy,gz:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
u8 MPU6500_Get_Accelerometer(short *ax, short *ay, short *az)
{
	u8 result;
	u8 acecel[6];
	result = MPU6500_Read_Len(MPU6500_device_addr, ACCEL_XOUT_H, 6, acecel);
	if(result==0)
	{
		*ax=((u16)acecel[0]<<8)|acecel[1];
		*ay=((u16)acecel[2]<<8)|acecel[3];
		*az=((u16)acecel[4]<<8)|acecel[5];
	}
	
	return result;
}

//�õ�������ֵ(ԭʼֵ)
//gx,gy,gz:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
u8 MPU6500_Get_Gyroscope(short *gx, short *gy, short *gz)
{
	u8 result;
	u8 gyro[6];
	result = MPU6500_Read_Len(MPU6500_device_addr, GYRO_XOUT_H, 6, gyro);
	if(result==0)
	{
		*gx = ((u16)gyro[0]<<8)|gyro[1];
		*gy = ((u16)gyro[2]<<8)|gyro[3];
		*gz = ((u16)gyro[4]<<8)|gyro[5];
	}
	
	return result;
}

//��ȡMPU6500���¶�
//����ֵ����ǰ�¶�(���϶�)
float MPU6500_GetTemperature(void)
{
	short temp;
	float dat;
	temp = GetData(TEMP_OUT_H);	

	dat = ((double) (temp - 21) / 333.87) + 21;

	return dat;
}

//����MPU6050�����Ǵ����������̷�Χ
//fsr:0,��250dps;1,��500dps;2,��1000dps;3,��2000dps
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
u8 MPU6500_Set_Gyro_Fsr(u8 fsr)
{
	return MPU6500_write_byte(GYRO_CONFIG, fsr << 3);//���������������̷�Χ  
}
//����MPU6050���ٶȴ����������̷�Χ
//fsr:0,��2g;1,��4g;2,��8g;3,��16g
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
u8 MPU6500_Set_Accel_Fsr(u8 fsr)
{
	return MPU6500_write_byte(ACCEL_CONFIG, fsr << 3);//���ü��ٶȴ����������̷�Χ  
}

//����MPU6050�Ĳ�����(�ٶ�Fs=1KHz)
//rate:4~1000(Hz)
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
u8 MPU6500_Set_Rate(u16 rate)
{
	u8 data;
	if(rate > 1000) rate = 1000;
	if(rate < 4) rate = 4;
	data = 1000/rate - 1;
	data = MPU6500_write_byte(SMPLRT_DIV, data);	//�������ֵ�ͨ�˲���
 	return MPU6500_Set_LPF(rate / 2);	//�Զ�����LPFΪ�����ʵ�һ��
}

//����MPU6050�����ֵ�ͨ�˲���
//lpf:���ֵ�ͨ�˲�Ƶ��(Hz)
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
u8 MPU6500_Set_LPF(u16 lpf)
{
	u8 data = 0;
	if(lpf >= 188) data = 1;
	else if(lpf >= 98) data = 2;
	else if(lpf >= 42) data = 3;
	else if(lpf >= 20) data = 4;
	else if(lpf >= 10) data = 5;
	else data = 6; 
	return MPU6500_write_byte(CONFIG, data);//�������ֵ�ͨ�˲���  
}


//MPU6500�Բ���
//����ֵ:0,����
//    ����,ʧ��
u8 MPU6500_run_self_test(void)
{
	int result;
	//char test_packet[4] = {0};
	long gyro[3], accel[3]; 
	result = mpu_run_self_test(gyro, accel);
	if (result == 0x07) 
	{
		/* Test passed. We can trust the gyro data here, so let's push it down
		* to the DMP.
		*/
		float gyro_sens;
		unsigned short accel_sens;
		mpu_get_gyro_sens(&gyro_sens);
//		gyro_sens = 0x00;	//��λʱ�ǶȲ�����	parry 2021.5.25
		gyro[0] = (long)(gyro[0] * gyro_sens);
		gyro[1] = (long)(gyro[1] * gyro_sens);
		gyro[2] = (long)(gyro[2] * gyro_sens);
		dmp_set_gyro_bias(gyro);
		mpu_get_accel_sens(&accel_sens);
		accel_sens = 0x00;	//��λʱ�ǶȲ�����	
		accel[0] *= accel_sens;
		accel[1] *= accel_sens;
		accel[2] *= accel_sens;
		dmp_set_accel_bias(accel);
				
		return 0;
	}
	else
		return 1;
}


//mpu6050,dmp��ʼ��
//����ֵ:0,����
//    ����,ʧ��
u8 MPU6500_DMP_Init(void)
{
	struct int_param_s int_param;
	int result;
	
	result = mpu_init(&int_param);	//��ʼ��MPU6050
	if(result) return 1;
	
	result = mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);//��������Ҫ�Ĵ�����
	if(result) return 2;
	
	result = mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);//����FIFO
	if(result) return 3;
	
	result = mpu_set_sample_rate(DEFAULT_MPU_HZ);//���ò�����
	if(result) 
		return 4;
	
	result = dmp_load_motion_driver_firmware();//����dmp�̼�
	if(result) 
		return 5;
	
	result = dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_pdata.orientation));//���������Ƿ���
	if(result) 
		return 6;
	
	result = dmp_enable_feature(DMP_FEATURE_TAP
								| DMP_FEATURE_ANDROID_ORIENT
								| DMP_FEATURE_6X_LP_QUAT
								| DMP_FEATURE_GYRO_CAL
								| DMP_FEATURE_SEND_RAW_ACCEL
								| DMP_FEATURE_SEND_RAW_GYRO);//����dmp����
	if(result) return 7;
	
	result = dmp_set_fifo_rate(DEFAULT_MPU_HZ);//����DMP�������(��󲻳���200Hz)
	if(result) return 8;
	
	result = MPU6500_run_self_test();		//�Լ죬���ڴ˴���6����������	parry
	if(result) return 9;
	
	result = mpu_set_dmp_state(1);			//ʹ��DMP
	if(result) return 10;
	
	result = dmp_set_interrupt_mode(DMP_INT_CONTINUOUS);	//�����жϲ�����ʽ
	if(result) return 11;
	
	return 0;
}

//�õ�dmp����������(ע��,��������Ҫ�Ƚ϶��ջ,�ֲ������е��)
//pitch:������ ����:0.1��   ��Χ:-90.0�� <---> +90.0��
//roll:�����  ����:0.1��   ��Χ:-180.0��<---> +180.0��
//yaw:�����   ����:0.1��   ��Χ:-180.0��<---> +180.0��
//����ֵ:0,����
//    ����,ʧ��
s8 MPU6500_dmp_get_euler_angle(short *accel, short *gyro, float *pitch, float *roll, float *yaw)
{
//q30��ʽ,longתfloatʱ�ĳ���.
#define Q30  ((1 << 30) * 1.0f)
	
	float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;
	unsigned long sensor_timestamp;
	short sensors;
	unsigned char more;
	long quat[4]; 
	s8 result = 0;
	result = dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more);
	if(result)
		return result;
	/* Gyro and accel data are written to the FIFO by the DMP in chip frame and hardware units.
	 * This behavior is convenient because it keeps the gyro and accel outputs of dmp_read_fifo and mpu_read_fifo consistent.
	**/
	/*if (sensors & INV_XYZ_GYRO )
	send_packet(PACKET_TYPE_GYRO, gyro);
	if (sensors & INV_XYZ_ACCEL)
	send_packet(PACKET_TYPE_ACCEL, accel); */
	/* Unlike gyro and accel, quaternions are written to the FIFO in the body frame, q30.
	 * The orientation is set by the scalar passed to dmp_set_orientation during initialization. 
	**/
	if(sensors & INV_WXYZ_QUAT) 
	{
		q0 = quat[0] / Q30;	//q30��ʽת��Ϊ������
		q1 = quat[1] / Q30;
		q2 = quat[2] / Q30;
		q3 = quat[3] / Q30; 
		//����õ�������/�����/�����
		*pitch = asin(-2 * q1 * q3 + 2 * q0 * q2) * 57.3;	// pitch
		*roll  = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * 57.3;	// roll
		*yaw   = atan2(2 * (q1 * q2 + q0 * q3), q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) * 57.3;	//yaw
	}
	else 
		return 2;

	return 0;
}




