/*
*********************************************************************************************************
*
*	ģ������ : SWD��������ģ��
*	�ļ����� : bsp_i2c.c
*	��    �� : V1.0
*	˵    �� : ��gpioģ��Serial Wire����
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0
*
*
*
*********************************************************************************************************
*/
#include "bsp_iic.h"
#include "dm633.h"
/****************************************************************************************************
*	�� �� ��: void bsp_InitSWD(uint8_t device)
*	����˵��: ����Serial Wire���ߵ�GPIO������ģ��IO��ת�ķ�ʽʵ��
*	��    ��: device:1��2
*	�� �� ֵ: ��
*****************************************************************************************************/
void bsp_InitSWD(uint8_t device)
{
	if (device == 1)
	{
		//��ʹ������IOʱ��	
		rcu_periph_clock_enable(RCU_PORT_I2C1_SCL);
		rcu_periph_clock_enable(RCU_PORT_I2C1_SDA);
		/* configure SWD GPIO port */ 
		gpio_init(GPIO_PORT_I2C1_SCL, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, I2C_SCL_PIN_1);
		gpio_init(GPIO_PORT_I2C1_SDA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, I2C_SDA_PIN_1);
		
		I2C_SCL_1_0();
		I2C_SDA_1_0();
	}
	else
	{
		//��ʹ������IOʱ��
		rcu_periph_clock_enable(RCU_PORT_I2C2_SCL);
		rcu_periph_clock_enable(RCU_PORT_I2C2_SDA);
		/* configure SWD GPIO port */ 
		gpio_init(GPIO_PORT_I2C2_SCL, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, I2C_SCL_PIN_2);
		gpio_init(GPIO_PORT_I2C2_SDA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, I2C_SDA_PIN_2);

		I2C_SCL_2_0();
		I2C_SDA_2_0();
	}
}

/**************************************************************************************************
*	�� �� ��: SW_Delay
*	����˵��: Serial_Wire����λ�ӳ٣����25MHz
*	��    ��: ��
*	�� �� ֵ: ��
***************************************************************************************************/
static void SW_Delay(void)
{
	uint8_t i;

	for (i = 0; i < 3; i++)
	{
		;		//������nop()
	}
}

/************************************************************************************************
*	�� �� ��: void SW_SendByte(uint8_t device, uint16_t _ucByte)
*	����˵��: CPU��SWD�����豸����һ��ͨ��12bit���ݣ���16�μ�������һ��IC
*	��    ��:   device �� �豸ID
				_ucByte �����͵��ֽ�
*	�� �� ֵ: ��
************************************************************************************************/
void SW_SendByte(uint8_t device, uint16_t _ucByte)
{
	uint8_t i;

	if (_ucByte > 0x0fff)			//DM633�Ҷȵȼ�0-4095������ֻ��12bit��Чλ�����ܳ���0x0fff
		_ucByte = 0x0fff;

	if (device == 1)
	{
		for (i = 12; i > 0; i--)
		{
			I2C_SCL_1_0();			//on the rising edge of the CLK,the serial-in data will be clocked 
									//into 12 * 16 bit shift registers synchronized, so input "0" first
			SW_Delay();   			//��ʱ����CLKʱ��

			if (_ucByte & 0x0800)		//MSB�ȷ�
			{
				I2C_SDA_1_1();			//�߼���1��Ϊ�ߵ�ƽ
			}
			else
			{
				I2C_SDA_1_0();			//�߼���0��Ϊ�͵�ƽ
			}
			SW_Delay();   		//��ʱ����SDA�����ȶ�
			I2C_SCL_1_1();		//rising edge
			SW_Delay();   		//��ʱ����CLKʱ��
			_ucByte <<= 1;		//��λȡ��һ��bit
		}
		I2C_SCL_1_0();			//�����������ָ�DCK�ź�0
		I2C_SDA_1_0();			//DAI��λ���ݺ�����
	}
	else
	{
		for (i = 12; i > 0; i--)
		{
			I2C_SCL_2_0();				//on the rising edge of the CLK,so SET 0 first
			
			SW_Delay();   		//��ʱ����CLKʱ��

			if (_ucByte & 0x0800)		//MSB�ȷ�
			{
				I2C_SDA_2_1();
			}
			else
			{
				I2C_SDA_2_0();
			}
			SW_Delay();   		//��ʱ����SDA�����ȶ�
			I2C_SCL_2_1();
			SW_Delay();   		//��ʱ����CLKʱ��
			_ucByte <<= 1;		//����һλ��ȡ��һbit
		}
		I2C_SCL_2_0();			//�����������ָ�DCK�ź�0
		I2C_SDA_2_0();			//DAI��λ���ݺ�����
	}
}

/************************************************************************************************
*	�� �� ��: void DM633_LATCH(uint8_t device)
*	����˵��: ��������ʹ��
*	��    ��: device �� �豸ID
*	�� �� ֵ: ��
************************************************************************************************/
void DM633_LATCH(uint8_t device)
{
	if(device == 1)
	{
		SW_Delay();
		I2C_SCL_1_0();
		SW_Delay();				//DCK�õ���ʱһ��ʱ��
		I2C_LAT_1_1();			//�����ø�
		SW_Delay();
		SW_Delay();
		I2C_LAT_1_0();			//�����õ�
	}
	else
	{
		SW_Delay();
		I2C_SCL_2_0();
		SW_Delay();				//DCK�õ���ʱһ��ʱ��
		I2C_LAT_2_1();			//�����ø�
		SW_Delay();
		SW_Delay();
		I2C_LAT_2_0();			//�����õ�
	}
}

/************************************************************************************************
*	�� �� ��: void DM634_GBC(uint8_t device, uint8_t _ucByte)
*	����˵��: Global Brightness Correction����
*	��    ��: device �� �豸ID��uint8_t _ucByte��GBC��ֵ�����ֵ0x7F��127��LSBΪdetection function
*	�� �� ֵ: ��
************************************************************************************************/
void DM634_GBC(uint8_t device, uint8_t _ucByte)
{
	uint16_t i;

	if(device == 1)
	{
		I2C_SCL_1_1();		//ʱ������DCKλ�ڸߵ�ƽ
		SW_Delay();   		//��ʱ����CLKʱ��
		
		for (i = 4; i > 0; i--)			//���ĸ�LAT�ź�
		{
			I2C_LAT_1_0();
			SW_Delay(); 
			I2C_LAT_1_1();
			SW_Delay(); 
		}
		SW_Delay();
		I2C_LAT_1_0();

		for (i = 8; i > 0; i--)
		{
			I2C_SCL_1_0();				//on the rising edge of the CLK,so SET 0 first
			SW_Delay();   		//��ʱ����CLKʱ��
			if (_ucByte & 0x80)
			{
				I2C_SDA_1_1();
			}
			else
			{
				I2C_SDA_1_0();
			}
			I2C_SCL_1_1();
			SW_Delay();   		//��ʱ����CLKʱ��
			_ucByte <<= 1;
		}

		I2C_SCL_1_0();			//on the rising edge of the CLK,so SET 0 first
		SW_Delay();   			//��ʱ����CLKʱ��
		I2C_LAT_1_1();
		SW_Delay(); 
		I2C_LAT_1_0();
	}
	else
	{
		I2C_SCL_2_1();		//ʱ������DCKλ�ڸߵ�ƽ
		SW_Delay();   		//��ʱ����CLKʱ��
		
		for (i = 4; i > 0; i--)			//���ĸ�LAT�ź�
		{
			I2C_LAT_2_0();
			SW_Delay(); 
			I2C_LAT_2_1();
			SW_Delay(); 
		}
		SW_Delay();
		I2C_LAT_2_0();

		for (i = 8; i > 0; i--)
		{
			I2C_SCL_2_0();				//on the rising edge of the CLK,so SET 0 first
			SW_Delay();   		//��ʱ����CLKʱ��
			if (_ucByte & 0x80)
			{
				I2C_SDA_2_1();
			}
			else
			{
				I2C_SDA_2_0();
			}
			I2C_SCL_2_1();
			SW_Delay();   		//��ʱ����CLKʱ��
			_ucByte <<= 1;
		}

		I2C_SCL_2_0();				//on the rising edge of the CLK,so SET 0 first
		SW_Delay();   		//��ʱ����CLKʱ��
		I2C_LAT_2_1();
		SW_Delay(); 
		I2C_LAT_2_0();
	}
}

/*********************************************************************
     DM633 Shift to Free-running Mode  (Internal GCK Mode) / Default mode
**********************************************************************/
void DM633_Internal_GCK(uint8_t device) //�ڲ�����ģʽ��DM633Ĭ�ϲ�������ģʽ���������һ�㲻������ʾ������ò��ô�ģʽ
{ 
	if(device == 1)
	{
		I2C_SCL_1_1();
		SW_Delay();
		//һ��CLK �����ĸ�LAT������
		I2C_LAT_1_0();
		SW_Delay();
		I2C_LAT_1_1();
		SW_Delay(); 
		I2C_LAT_1_0();
		SW_Delay(); 
		I2C_LAT_1_1();
		SW_Delay(); 
		I2C_LAT_1_0();
		SW_Delay(); 
		I2C_LAT_1_1();
		SW_Delay(); 
		I2C_LAT_1_0();
		SW_Delay(); 
		I2C_LAT_1_1();
		SW_Delay(); 
		//һ��LAT ��������DCK������
		I2C_SCL_1_0();	
		SW_Delay(); 
		I2C_SCL_1_1();
		SW_Delay(); 
		I2C_SCL_1_0();	
		SW_Delay(); 
		I2C_SCL_1_1();
		SW_Delay(); 
		I2C_SCL_1_0();	
		SW_Delay(); 
		I2C_SCL_1_1();
		SW_Delay(); 
		
		I2C_SCL_1_0();
		I2C_LAT_1_0();
	}
	else
	{
		I2C_SCL_2_1();
		SW_Delay();
		//һ��CLK �����ĸ�LAT������
		I2C_LAT_2_0();
		SW_Delay();
		I2C_LAT_2_1();
		SW_Delay(); 
		I2C_LAT_2_0();
		SW_Delay(); 
		I2C_LAT_2_1();
		SW_Delay(); 
		I2C_LAT_2_0();
		SW_Delay(); 
		I2C_LAT_2_1();
		SW_Delay(); 
		I2C_LAT_2_0();
		SW_Delay(); 
		I2C_LAT_2_1();
		SW_Delay(); 
		//һ��LAT ��������DCK������
		I2C_SCL_2_0();
		SW_Delay();
		I2C_SCL_2_1();
		SW_Delay();
		I2C_SCL_2_0();
		SW_Delay();
		I2C_SCL_2_1();
		SW_Delay();
		I2C_SCL_2_0();
		SW_Delay();
		I2C_SCL_2_1();
		SW_Delay();
		
		I2C_SCL_2_0();
		I2C_LAT_2_0();
	}
}

/*********************************************************************
           DM633 Shift to External GCK Mode 
**********************************************************************/
void DM633_External_GCK(void) //�л����ⲿ����ģʽ����ʱPWM�����źŴ�GCK�����롣һ�㲻�����������ģʽ
{
	I2C_SCL_1_1();
	SW_Delay();
	//һ��CLK ��������LAT������
	I2C_LAT_1_0();
	SW_Delay();
	I2C_LAT_1_1();
	SW_Delay();
	I2C_LAT_1_0();
	SW_Delay();
	I2C_LAT_1_1();
	SW_Delay();
	I2C_LAT_1_0();
	SW_Delay();
	I2C_LAT_1_1();
	SW_Delay();
	//һ��LAT ��������DCK������
	I2C_SCL_1_0();	
	SW_Delay();
	I2C_SCL_1_1();
	SW_Delay();
	I2C_SCL_1_0();
	SW_Delay();
	I2C_SCL_1_1();
	SW_Delay();
		
	I2C_SCL_1_0();
	I2C_LAT_1_0();
} 

