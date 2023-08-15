/****************************************************************************
* �ļ����ƣ�dm633.c
* ���ߣ��̺���
* ��ǰ�汾��v1.1
* ������ڣ�2011-5-4
* ժ Ҫ����Ҫ�������ļ��Ĺ��ܼ����ݡ��������ĵ�����Ӳ��ԭ��ͼ�汾�Ż��ļ���
* �����ĵ��ȡ�
* ��ʷ��Ϣ��
* ��ʷ�汾     �������      ԭ����                        ע��
* v1.0         2010-12-22    ����ԭ����(���޸���)����
*****************************************************************************
* Copyright (c) 2018��������������޹�˾ All rights reserved.
****************************************************************************/
#include "dm633.h"
#include "bsp_iic.h"
#include <ucos_ii.h>

#define LED_GRAYSCALE_VALUE		0x0320						//LED�����趨ֵ��������������100mA����

/**************************************************************************************
*	�� �� ��: DM633Init
*	����˵��: ����LED����IC���ܿ�����ص�GPIO
*	��    ��:  ��
*	�� �� ֵ: ��
***************************************************************************************/
void DM633Init(void)
{
	//��ʼ������IO��RCU
	rcu_periph_clock_enable(RCU_PORT_DM633_DEVICE1_LAT);
	rcu_periph_clock_enable(RCU_PORT_DM633_DEVICE2_LAT);
	//��ʼ������LED_5V��ѹ����IO��RCU��GPIO port����LED-5V���ڶϵ�״̬
	rcu_periph_clock_enable(RCU_PORT_LED_POWER);
	gpio_init(GPIO_PORT_LED_POWER, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, LED_POWER_PIN);
	//��ʼ״̬Ĭ�Ϲ���LED_5V���ڶϵ�״̬
	LED_POWER_OFF();
	/* configure LAT GPIO port */ 
	gpio_init(GPIO_PORT_DM633_DEVICE1_LAT, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, DM633_DEVICE1_LAT_PIN);
	gpio_init(GPIO_PORT_DM633_DEVICE2_LAT, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, DM633_DEVICE2_LAT_PIN);
	//��ʼLATCH�ź�״̬
	I2C_LAT_1_0();
	I2C_LAT_2_0();
	//��ʼ��SCK��DAI��GPIO
	bsp_InitSWD(DEVICE_RED);
	bsp_InitSWD(DEVICE_BLUE);
	//�л�Ϊ�ڲ�CLK������һ��internal��RESET�ź�
	DM633_Internal_GCK(DEVICE_RED);
	DM633_Internal_GCK(DEVICE_BLUE);
	
	DM633_LATCH(DEVICE_RED);
	DM633_LATCH(DEVICE_BLUE);
	
	DM634_GBC(DEVICE_RED, 0xFF);			//GBC�趨Ϊ100%��������Ĭ��Ϊ75%
	DM634_GBC(DEVICE_BLUE, 0xFF);			//GBC�趨Ϊ100%��������Ĭ��Ϊ75%
	
}

/*****************************************************************************************
*	�� �� ��: void ctrlUledPro(uint8_t device, uint16_t grayscale)
*	����˵��: CPUͨ��Serial Out���豸����һ��12bit����
*	��    ��: uint8_t device ������оƬID
			  uint16_t grayscale������ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUledPro(uint8_t device, uint16_t grayscale)
{
	SW_SendByte(device, grayscale);
}

/*****************************************************************************************
*	�� �� ��: void TurnOffAllUledPro(uint8_t device)
*	����˵��: CPU��Serial Out�豸����16*12bitȫ0���ݣ��������Ϊ0�����ر�����LED
*	��    ��:  uint8_t device������оƬID
*	�� �� ֵ: ��
******************************************************************************************/
void TurnOffAllUledPro(uint8_t device)
{
	uint8_t j = 0;
	
	for(j = 0; j < 16; j++)
	{
		SW_SendByte(device, 0x0000);
	}
	
	DM633_LATCH(device);			//д��һ��IC������һ������
}

/*****************************************************************************************
*	�� �� ��: void TurnOnUledPro(uint8_t device, uint16_t grayscale)
*	����˵��: CPUͨ��Serial Out���豸����16 * 12bit�Ҷ�PWM���ݣ��������Ϊ�̶�ֵ������LEDΪ�̶��Ҷ�״̬
*	��    ��: uint8_t device ������оƬID
			  uint16_t grayscale������ֵ(0x0000-0x0fff)
*	�� �� ֵ: ��
******************************************************************************************/
void TurnOnUledPro(uint8_t device, uint16_t grayscale)
{
	uint8_t j = 0;
	
	for(j = 0; j < 16; j++)
	{
		ctrlUledPro(device, grayscale);
	}

	DM633_LATCH(device);			//д��һ��IC������һ������
}

/*****************************************************************************************
*	�� �� ��: void ctrlUledTabValue_RED(uint8_t device)
*	����˵��: CPUͨ��Serial Out���豸����һ�� (16 * 12bit)����
*	��    ��: uint8_t device ������оƬID
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUledTabValue_RED(uint8_t device)
{
	uint8_t i = 0;
	uint16_t GrayscaleTAB[16] = {0x0000, 0x0000, 0x0000, 0x0000, 
								0x0000, 0x0000, 0x0000, 0x0000,
								LED_GRAYSCALE_VALUE, LED_GRAYSCALE_VALUE, LED_GRAYSCALE_VALUE, LED_GRAYSCALE_VALUE, 
								LED_GRAYSCALE_VALUE, LED_GRAYSCALE_VALUE, LED_GRAYSCALE_VALUE, LED_GRAYSCALE_VALUE};

	for (i = 0; i < 16; i++)
	{
		ctrlUledPro(device, GrayscaleTAB[i]);
	}

	DM633_LATCH(device);			//д��һ��IC������һ������
}

/*****************************************************************************************
*	�� �� ��: void ctrlUledTabValue_BLUE(uint8_t device)
*	����˵��: CPUͨ��Serial Out���豸����һ�� (16 * 12bit)����
*	��    ��: uint8_t device ������оƬID
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUledTabValue_BLUE(uint8_t device)
{
	uint8_t i = 0;
	uint16_t GrayscaleTAB[16] = {0x0000, 0x0000, 0x0000, 0x0000, 
								0x0000, 0x0000, 0x0000, 0x0000,
								LED_GRAYSCALE_VALUE, LED_GRAYSCALE_VALUE, LED_GRAYSCALE_VALUE, LED_GRAYSCALE_VALUE, 
								LED_GRAYSCALE_VALUE, LED_GRAYSCALE_VALUE, LED_GRAYSCALE_VALUE, LED_GRAYSCALE_VALUE};
	for (i = 0; i < 16; i++)
	{
		ctrlUledPro(device, GrayscaleTAB[i]);
	}

	DM633_LATCH(device);			//д��16*12bit�󣬷�һ��LATCH�����ź�
}

/*****************************************************************************************
*	�� �� ��: void ctrlUledTabValue_GREEN(uint8_t device)
*	����˵��: CPUͨ��Serial Out���豸����һ�� (16 * 12bit)����
*	��    ��: uint8_t device ������оƬID
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUledTabValue_GREEN(uint8_t device)
{
	uint8_t i = 0;
	uint16_t GrayscaleTAB[16] = {0x0000, 0x0000, 0x0000, 0x0000, 
								LED_GRAYSCALE_VALUE, LED_GRAYSCALE_VALUE, LED_GRAYSCALE_VALUE, LED_GRAYSCALE_VALUE,
								0x0000, 0x0000, 0x0000, 0x0000, 
								0x0000, 0x0000, 0x0000, 0x0000};

	for (i = 0; i < 16; i++)
	{
		ctrlUledPro(device, GrayscaleTAB[i]);
	}
	DM633_LATCH(device);
}

/*****************************************************************************************
*	�� �� ��: ctrlUledRGBValue
*	����˵��: ����RGB��ɫ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUledRGBValue(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_RED, 0x0000);
	}
	//��4-8������ΪGREEN
	for (i = 4; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, GGrayscale << 4);			//���뷶ΧΪһ���ֽ�0-255������4λת��Ϊ0-4095��Χ��12bit�����и�ʽ��ͬ
	}
	for (i = 8; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0x0000);
	}
	//��4-8������ΪGREEN
	for (i = 4; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GGrayscale << 4);
	}
	for (i = 8; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	DM633_LATCH(DEVICE_BLUE);
}

/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D1
*	����˵��: ����D1������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D1(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//ǰ��������Ϊ��
	for (i = 0; i < 7; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 7; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, GGrayscale << 4);
	}
	for (i = 8; i < 15; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 15; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 15; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 15; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D2
*	����˵��: ����D2������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D2(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//ǰ��������Ϊ��
	for (i = 0; i < 6; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 6; i < 7; i++)
	{
		ctrlUledPro(DEVICE_RED, GGrayscale << 4);
	}
	for (i = 7; i < 14; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 14; i < 15; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	for (i = 15; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 14; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 14; i < 15; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	for (i = 15; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D3
*	����˵��: ����D3������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D3(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//ǰ��������Ϊ��
	for (i = 0; i < 5; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 5; i < 6; i++)
	{
		ctrlUledPro(DEVICE_RED, GGrayscale << 4);
	}
	for (i = 6; i < 13; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 13; i < 14; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	for (i = 14; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 13; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 13; i < 14; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	for (i = 14; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D4
*	����˵��: ����D4������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D4(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 4; i < 5; i++)
	{
		ctrlUledPro(DEVICE_RED, GGrayscale << 4);
	}
	for (i = 5; i < 12; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 12; i < 13; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	for (i = 13; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 12; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 12; i < 13; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	for (i = 13; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D4
*	����˵��: ����D4������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D5(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//ǰ��������Ϊ��
	for (i = 0; i < 11; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 11; i < 12; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	for (i = 12; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 7; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 7; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GGrayscale << 4);
	}
	for (i = 8; i < 11; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 11; i < 12; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	for (i = 12; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D6
*	����˵��: ����D6������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D6(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//ǰ��������Ϊ��
	for (i = 0; i < 10; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 10; i < 11; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	for (i = 11; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 6; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 6; i < 7; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GGrayscale << 4);
	}
	for (i = 7; i < 10; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 10; i < 11; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	for (i = 11; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}

/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D7
*	����˵��: ����D7������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D7(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//ǰ��������Ϊ��
	for (i = 0; i < 9; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 9; i < 10; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	for (i = 10; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 5; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 5; i < 6; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GGrayscale << 4);
	}
	for (i = 6; i < 9; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 9; i < 10; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	for (i = 10; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}

/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D8
*	����˵��: ����D8������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//ǰ��������Ϊ��
	for (i = 0; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 8; i < 9; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	for (i = 9; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 4; i < 5; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GGrayscale << 4);
	}
	for (i = 5; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 8; i < 9; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	for (i = 9; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}
//�������������ĵƵ���
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D1_D2
*	����˵��: ����D1_D2������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D1_D2(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//ǰ��������Ϊ��
	for (i = 0; i < 6; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 6; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, GGrayscale << 4);
	}
	for (i = 8; i < 14; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 14; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 14; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 14; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D3_D4
*	����˵��: ����D3_D4������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D3_D4(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 4; i < 6; i++)
	{
		ctrlUledPro(DEVICE_RED, GGrayscale << 4);
	}
	for (i = 6; i < 12; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 12; i < 14; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	for (i = 14; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 12; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 12; i < 14; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	for (i = 14; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D5_D6
*	����˵��: ����D5_D6������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D5_D6(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//ǰ��������Ϊ��
	for (i = 0; i < 10; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 10; i < 12; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	for (i = 12; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 6; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 6; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GGrayscale << 4);
	}
	for (i = 8; i < 10; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 10; i < 12; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	for (i = 12; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}

/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D7_D8
*	����˵��: ����D7_D8������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D7_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//ǰ��������Ϊ��
	for (i = 0; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 8; i < 10; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	for (i = 10; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 4; i < 6; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GGrayscale << 4);
	}
	for (i = 6; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 8; i < 10; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	for (i = 10; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}
//�����ĸ������ĵƵ���
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D1_D3_D5_D7
*	����˵��: ����D1_D3_D5_D7������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D1_D3_D5_D7(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//ǰ��������Ϊ��
	for (i = 0; i < 5; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 5; i < 6; i++)
	{
		ctrlUledPro(DEVICE_RED, GGrayscale << 4);
	}
	for (i = 6; i < 7; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 7; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, GGrayscale << 4);
	}
	
	for (i = 8; i < 9; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 9; i < 10; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	for (i = 10; i < 11; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 11; i < 12; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	for (i = 12; i < 13; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 13; i < 14; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	for (i = 14; i < 15; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 15; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 5; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 5; i < 6; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GGrayscale << 4);
	}
	for (i = 6; i < 7; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 7; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GGrayscale << 4);
	}
	for (i = 8; i < 9; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 9; i < 10; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	for (i = 10; i < 11; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 11; i < 12; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	for (i = 12; i < 13; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 13; i < 14; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	for (i = 14; i < 15; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 15; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D2_D4_D6_D8
*	����˵��: ����D2_D4_D6_D8������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D2_D4_D6_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 4; i < 5; i++)
	{
		ctrlUledPro(DEVICE_RED, GGrayscale << 4);
	}
	for (i = 5; i < 6; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 6; i < 7; i++)
	{
		ctrlUledPro(DEVICE_RED, GGrayscale << 4);
	}
	for (i = 7; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	
	for (i = 8; i < 9; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	for (i = 9; i < 10; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 10; i < 11; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	for (i = 11; i < 12; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 12; i < 13; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	for (i = 13; i < 14; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 14; i < 15; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	for (i = 15; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 4; i < 5; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GGrayscale << 4);
	}
	for (i = 5; i < 6; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 6; i < 7; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GGrayscale << 4);
	}
	for (i = 7; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 8; i < 9; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	for (i = 9; i < 10; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 10; i < 11; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	for (i = 11; i < 12; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 12; i < 13; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	for (i = 13; i < 14; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 14; i < 15; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	for (i = 15; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D1_D2_D3
*	����˵��: ����D1_D2_D3������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D1_D2_D3(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//ǰ��������Ϊ��
	for (i = 0; i < 5; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 5; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, GGrayscale << 4);
	}
	for (i = 8; i < 13; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 13; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 13; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 13; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D1_D2_D3
*	����˵��: ����D1_D2_D3������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D1_D2_D3_D4(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 4; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, GGrayscale << 4);
	}
	for (i = 8; i < 12; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 12; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 12; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 12; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D1_D2_D3_D4_D5
*	����˵��: ����D1_D2_D3_D4_D5������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D1_D2_D3_D4_D5(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 4; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, GGrayscale << 4);
	}
	for (i = 8; i < 11; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 11; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 7; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 7; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GGrayscale << 4);
	}
	for (i = 8; i < 11; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 11; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6
*	����˵��: ����D1_D2_D3_D4_D5_D6������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 4; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, GGrayscale << 4);
	}
	for (i = 8; i < 10; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 10; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 6; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 6; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GGrayscale << 4);
	}
	for (i = 8; i < 10; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 10; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6_D7
*	����˵��: ����D1_D2_D3_D4_D5_D6_D7������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6_D7(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 4; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, GGrayscale << 4);
	}
	for (i = 8; i < 9; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 9; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 5; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 5; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GGrayscale << 4);
	}
	for (i = 8; i < 9; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 9; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6_D7_D8
*	����˵��: ����D1_D2_D3_D4_D5_D6_D7_D8������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6_D7_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 4; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, GGrayscale << 4);
	}
	for (i = 8; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 4; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GGrayscale << 4);
	}
	for (i = 8; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, BGrayscale << 4);
	}
	DM633_LATCH(DEVICE_BLUE);
}

//������Ч����������
/*****************************************************************************************
*	�� �� ��: GetColorNum
*	����˵��: �м�����ɫ��Ҫ�������
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
*	�� �� ֵ: ���1�ĸ�������ʾ�м�����ɫ��Ҫ�������
******************************************************************************************/
uint16_t GetColorNum(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	u8 casenum;
	u8 n;

	casenum = 0x00;
	if(RGrayscale != 0)
	{
		casenum |= 0x01;
	}
	if(GGrayscale != 0)
	{
		casenum |= 0x02;
	}
	if(BGrayscale != 0)
	{
		casenum |= 0x04;
	}
	
	for(n = 0; casenum; casenum /= 2)			//���1�ĸ�������ʾ�м�����ɫ��Ҫ�������
		n += casenum % 2;

	return n;
}

uint16_t grayscale_r = 0;		//����ֵ
uint16_t grayscale_g = 0;
uint16_t grayscale_b = 0;

uint16_t RGS = 0;		//����ֵ
uint16_t GGS = 0;
uint16_t BGS = 0;

uint16_t StepUpDown = 0;
/****************************************************************************
 * ���������������ǰ����澯��־λ״̬
 * �����������
 * ����ʱ�䣺
 * �������ߣ�
*****************************************************************************/
void ClrBreatheSta(void)
{
	grayscale_r = 0;		//����ֵ
	grayscale_g = 0;
	grayscale_b = 0;
	RGS = 0;				//����ֵ
	GGS = 0;
	BGS = 0;
	StepUpDown = 0;			//����
}
/*****************************************************************************************
*	�� �� ��: void ctrlUledRGBValueBreathe
*	����˵��: ����RGB��ɫ����ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�ֵ
			  uint16_t rstep����������������ɫ
			  uint16_t gstep����������������ɫ
			  uint16_t bstep����������������ɫ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUledRGBValueBreathe(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale, uint16_t rstep, uint16_t gstep, uint16_t bstep)
{
	u8 casenum;
	uint8_t i = 0;

	casenum = GetColorNum(RGrayscale, GGrayscale, BGrayscale);
	
	RGS = RGrayscale << 4;
	GGS = GGrayscale << 4;
	BGS = BGrayscale << 4;

	switch(casenum)
	{
		case 1:				//��ɫģʽ
		case 2:				//2����ɫ���
		case 3:				//3����ɫ���
		{
			//�Ӱ������ı仯����
			if (StepUpDown == 0)
			{
				if(RGrayscale != 0)
				{
					grayscale_r += rstep;
					if(grayscale_r >= RGS)
					{
						grayscale_r = RGS;
					}
				}
				else
				{
					grayscale_r = 0;
				}
				if(GGrayscale != 0)
				{
					grayscale_g += gstep;
					if(grayscale_g >= GGS)
					{
						grayscale_g = GGS;
					}
				}
				else
				{
					grayscale_g = 0;
				}
				if(BGrayscale != 0)
				{
					grayscale_b += bstep;
					if(grayscale_b >= BGS)
					{
						grayscale_b = BGS;
					}
				}
				else
				{
					grayscale_b = 0;
				}
				if ((grayscale_r == RGS) && (grayscale_g == GGS) && (grayscale_b == BGS))
				{
					StepUpDown = 1;
				}
				//ǰ�������ݿ�
				for (i = 0; i < 4; i++)
				{
					ctrlUledPro(DEVICE_RED, 0x0000);
				}
				for (i = 4; i < 8; i++)
				{
					ctrlUledPro(DEVICE_RED, grayscale_g);
				}
				for (i = 8; i < 16; i++)
				{
					ctrlUledPro(DEVICE_RED, grayscale_r);
				}
				DM633_LATCH(DEVICE_RED);
				//ǰ�������ݿ�
				for (i = 0; i < 4; i++)
				{
					ctrlUledPro(DEVICE_BLUE, 0x0000);
				}
				for (i = 4; i < 8; i++)
				{
					ctrlUledPro(DEVICE_BLUE, grayscale_g);
				}
				for (i = 8; i < 16; i++)
				{
					ctrlUledPro(DEVICE_BLUE, grayscale_b);
				}
				DM633_LATCH(DEVICE_BLUE);

				OSTimeDly(50);				//ÿ���һ��ʱ��д��һ���Ҷ�ֵ
			}
			//���������ı仯����
			if (StepUpDown == 1)
			{
				if(RGrayscale != 0)
				{
					if(grayscale_r <= rstep)
					{
						grayscale_r = 0;
					}
					else
						grayscale_r -= rstep;
				}
				else
				{
					grayscale_r = 0;
				}
				if(GGrayscale != 0)
				{
					if(grayscale_g <= gstep)
					{
						grayscale_g = 0;
					}
					else
						grayscale_g -= gstep;
				}
				else
				{
					grayscale_g = 0;
				}
				if(BGrayscale != 0)
				{
					if(grayscale_b <= bstep)
					{
						grayscale_b = 0;
					}
					else
						grayscale_b -= bstep;
				}
				else
				{
					grayscale_b = 0;
				}
				if ((grayscale_r == 0) && (grayscale_g == 0) && (grayscale_b == 0))		//������0�����½����ۼӼ���
				{
					StepUpDown = 0;
				}
				//ǰ�������ݿ�
				for (i = 0; i < 4; i++)
				{
					ctrlUledPro(DEVICE_RED, 0x0000);
				}
				for (i = 4; i < 8; i++)
				{
					ctrlUledPro(DEVICE_RED, grayscale_g);
				}
				for (i = 8; i < 16; i++)
				{
					ctrlUledPro(DEVICE_RED, grayscale_r);
				}
				DM633_LATCH(DEVICE_RED);
				//ǰ�������ݿ�
				for (i = 0; i < 4; i++)
				{
					ctrlUledPro(DEVICE_BLUE, 0x0000);
				}
				for (i = 4; i < 8; i++)
				{
					ctrlUledPro(DEVICE_BLUE, grayscale_g);
				}
				for (i = 8; i < 16; i++)
				{
					ctrlUledPro(DEVICE_BLUE, grayscale_b);
				}
				DM633_LATCH(DEVICE_BLUE);

				OSTimeDly(50);				//ÿ���һ��ʱ��д��һ���Ҷ�ֵ
			}
		}
		break;
		
		default:
			break;
	}
}


//���ǵ�Ч����������
uint16_t GrayscaleTAB_R[4] = {0x0000, 0x0000, 0x0000, 0x0000};
uint16_t GrayscaleTAB_G[4] = {0x0000, 0x0000, 0x0000, 0x0000};
uint16_t GrayscaleTAB_B[4] = {0x0000, 0x0000, 0x0000, 0x0000};
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_Meteor_D1_D2_D3_D4
*	����˵��: ����D1_D2_D3_D4������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�����ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_Meteor_D1_D2_D3_D4(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	GrayscaleTAB_R[3] = RGrayscale << 4;
	GrayscaleTAB_R[2] = RGrayscale << 1;
	GrayscaleTAB_R[1] = RGrayscale;
	if(RGrayscale != 0)
		GrayscaleTAB_R[0] = 2;
	else
		GrayscaleTAB_R[0] = 0;
	
	GrayscaleTAB_G[3] = GGrayscale << 4;
	GrayscaleTAB_G[2] = GGrayscale << 1;
	GrayscaleTAB_G[1] = GGrayscale;
	if(GGrayscale != 0)
		GrayscaleTAB_G[0] = 2;
	else
		GrayscaleTAB_G[0] = 0;
	
	GrayscaleTAB_B[3] = BGrayscale << 4;
	GrayscaleTAB_B[2] = BGrayscale << 1;
	GrayscaleTAB_B[1] = BGrayscale;
	if(BGrayscale != 0)
		GrayscaleTAB_B[0] = 2;
	else
		GrayscaleTAB_B[0] = 0;
	
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 4; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_G[7 - i]);
	}
	for (i = 8; i < 12; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 12; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[15 - i]);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 12; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 12; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[15 - i]);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_Meteor_D2_D3_D4_D5
*	����˵��: ����D2_D3_D4_D5������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�����ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_Meteor_D2_D3_D4_D5(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	GrayscaleTAB_R[3] = RGrayscale << 4;
	GrayscaleTAB_R[2] = RGrayscale << 1;
	GrayscaleTAB_R[1] = RGrayscale;
	if(RGrayscale != 0)
		GrayscaleTAB_R[0] = 2;
	else
		GrayscaleTAB_R[0] = 0;
	
	GrayscaleTAB_G[3] = GGrayscale << 4;
	GrayscaleTAB_G[2] = GGrayscale << 1;
	GrayscaleTAB_G[1] = GGrayscale;
	if(GGrayscale != 0)
		GrayscaleTAB_G[0] = 2;
	else
		GrayscaleTAB_G[0] = 0;
	
	GrayscaleTAB_B[3] = BGrayscale << 4;
	GrayscaleTAB_B[2] = BGrayscale << 1;
	GrayscaleTAB_B[1] = BGrayscale;
	if(BGrayscale != 0)
		GrayscaleTAB_B[0] = 2;
	else
		GrayscaleTAB_B[0] = 0;
	
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 4; i < 7; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_G[6 - i]);
	}
	for (i = 7; i < 11; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 11; i < 15; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[14 - i]);
	}
	for (i = 15; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 7; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 7; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_G[10 - i]);
	}
	for (i = 8; i < 11; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 11; i < 15; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[14 - i]);
	}
	for (i = 15; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_Meteor_D3_D4_D5_D6
*	����˵��: ����D3_D4_D5_D6������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�����ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_Meteor_D3_D4_D5_D6(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	GrayscaleTAB_R[3] = RGrayscale << 4;
	GrayscaleTAB_R[2] = RGrayscale << 1;
	GrayscaleTAB_R[1] = RGrayscale;
	if(RGrayscale != 0)
		GrayscaleTAB_R[0] = 2;
	else
		GrayscaleTAB_R[0] = 0;
	
	GrayscaleTAB_G[3] = GGrayscale << 4;
	GrayscaleTAB_G[2] = GGrayscale << 1;
	GrayscaleTAB_G[1] = GGrayscale;
	if(GGrayscale != 0)
		GrayscaleTAB_G[0] = 2;
	else
		GrayscaleTAB_G[0] = 0;
	
	GrayscaleTAB_B[3] = BGrayscale << 4;
	GrayscaleTAB_B[2] = BGrayscale << 1;
	GrayscaleTAB_B[1] = BGrayscale;
	if(BGrayscale != 0)
		GrayscaleTAB_B[0] = 2;
	else
		GrayscaleTAB_B[0] = 0;
	
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 4; i < 6; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_G[5 - i]);
	}
	for (i = 6; i < 10; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 10; i < 14; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[13 - i]);
	}
	for (i = 14; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 6; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 6; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_G[9 - i]);
	}
	for (i = 8; i < 10; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 10; i < 14; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[13 - i]);
	}
	for (i = 14; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_Meteor_D4_D5_D6_D7
*	����˵��: ����D4_D5_D6_D7������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�����ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_Meteor_D4_D5_D6_D7(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	GrayscaleTAB_R[3] = RGrayscale << 4;
	GrayscaleTAB_R[2] = RGrayscale << 1;
	GrayscaleTAB_R[1] = RGrayscale;
	if(RGrayscale != 0)
		GrayscaleTAB_R[0] = 2;
	else
		GrayscaleTAB_R[0] = 0;
	
	GrayscaleTAB_G[3] = GGrayscale << 4;
	GrayscaleTAB_G[2] = GGrayscale << 1;
	GrayscaleTAB_G[1] = GGrayscale;
	if(GGrayscale != 0)
		GrayscaleTAB_G[0] = 2;
	else
		GrayscaleTAB_G[0] = 0;
	
	GrayscaleTAB_B[3] = BGrayscale << 4;
	GrayscaleTAB_B[2] = BGrayscale << 1;
	GrayscaleTAB_B[1] = BGrayscale;
	if(BGrayscale != 0)
		GrayscaleTAB_B[0] = 2;
	else
		GrayscaleTAB_B[0] = 0;
	
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 4; i < 5; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_G[4 - i]);
	}
	for (i = 5; i < 9; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 9; i < 13; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[12 - i]);
	}
	for (i = 13; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 5; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 5; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_G[8 - i]);
	}
	for (i = 8; i < 9; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 9; i < 13; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[12 - i]);
	}
	for (i = 13; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_Meteor_D5_D6_D7_D8
*	����˵��: ����D5_D6_D7_D8������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�����ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_Meteor_D5_D6_D7_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	GrayscaleTAB_R[3] = RGrayscale << 4;
	GrayscaleTAB_R[2] = RGrayscale << 1;
	GrayscaleTAB_R[1] = RGrayscale;
	if(RGrayscale != 0)
		GrayscaleTAB_R[0] = 2;
	else
		GrayscaleTAB_R[0] = 0;
	
	GrayscaleTAB_G[3] = GGrayscale << 4;
	GrayscaleTAB_G[2] = GGrayscale << 1;
	GrayscaleTAB_G[1] = GGrayscale;
	if(GGrayscale != 0)
		GrayscaleTAB_G[0] = 2;
	else
		GrayscaleTAB_G[0] = 0;
	
	GrayscaleTAB_B[3] = BGrayscale << 4;
	GrayscaleTAB_B[2] = BGrayscale << 1;
	GrayscaleTAB_B[1] = BGrayscale;
	if(BGrayscale != 0)
		GrayscaleTAB_B[0] = 2;
	else
		GrayscaleTAB_B[0] = 0;
	
	//ǰ��������Ϊ��
	for (i = 0; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 8; i < 12; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[11 - i]);
	}
	for (i = 12; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 4; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_G[7 - i]);
	}
	for (i = 8; i < 12; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[11 - i]);
	}
	for (i = 12; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_Meteor_D6_D7_D8_D1
*	����˵��: ����D6_D7_D8_D1������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�����ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_Meteor_D6_D7_D8_D1(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	GrayscaleTAB_R[3] = RGrayscale << 4;
	GrayscaleTAB_R[2] = RGrayscale << 1;
	GrayscaleTAB_R[1] = RGrayscale;
	if(RGrayscale != 0)
		GrayscaleTAB_R[0] = 2;
	else
		GrayscaleTAB_R[0] = 0;
	
	GrayscaleTAB_G[3] = GGrayscale << 4;
	GrayscaleTAB_G[2] = GGrayscale << 1;
	GrayscaleTAB_G[1] = GGrayscale;
	if(GGrayscale != 0)
		GrayscaleTAB_G[0] = 2;
	else
		GrayscaleTAB_G[0] = 0;
	
	GrayscaleTAB_B[3] = BGrayscale << 4;
	GrayscaleTAB_B[2] = BGrayscale << 1;
	GrayscaleTAB_B[1] = BGrayscale;
	if(BGrayscale != 0)
		GrayscaleTAB_B[0] = 2;
	else
		GrayscaleTAB_B[0] = 0;
	
	//ǰ��������Ϊ��
	for (i = 0; i < 7; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 7; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_G[10 - i]);
	}
	for (i = 8; i < 11; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[10 - i]);
	}
	for (i = 11; i < 15; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 15; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[18 - i]);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 4; i < 7; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_G[6 - i]);
	}
	for (i = 7; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 8; i < 11; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[10 - i]);
	}
	for (i = 11; i < 15; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 15; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[18 - i]);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_Meteor_D7_D8_D1_D2
*	����˵��: ����D7_D8_D1_D2������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�����ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_Meteor_D7_D8_D1_D2(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	GrayscaleTAB_R[3] = RGrayscale << 4;
	GrayscaleTAB_R[2] = RGrayscale << 1;
	GrayscaleTAB_R[1] = RGrayscale;
	if(RGrayscale != 0)
		GrayscaleTAB_R[0] = 2;
	else
		GrayscaleTAB_R[0] = 0;
	
	GrayscaleTAB_G[3] = GGrayscale << 4;
	GrayscaleTAB_G[2] = GGrayscale << 1;
	GrayscaleTAB_G[1] = GGrayscale;
	if(GGrayscale != 0)
		GrayscaleTAB_G[0] = 2;
	else
		GrayscaleTAB_G[0] = 0;
	
	GrayscaleTAB_B[3] = BGrayscale << 4;
	GrayscaleTAB_B[2] = BGrayscale << 1;
	GrayscaleTAB_B[1] = BGrayscale;
	if(BGrayscale != 0)
		GrayscaleTAB_B[0] = 2;
	else
		GrayscaleTAB_B[0] = 0;
	
	//ǰ��������Ϊ��
	for (i = 0; i < 6; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 6; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_G[9 - i]);
	}
	for (i = 8; i < 10; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[9 - i]);
	}
	for (i = 10; i < 14; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 14; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[17 - i]);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 4; i < 6; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_G[5 - i]);
	}
	for (i = 6; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 8; i < 10; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[9 - i]);
	}
	for (i = 10; i < 14; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 14; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[17 - i]);
	}
	DM633_LATCH(DEVICE_BLUE);
}

/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_Meteor_D8_D1_D2_D3
*	����˵��: ����D8_D1_D2_D3������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�����ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_Meteor_D8_D1_D2_D3(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	GrayscaleTAB_R[3] = RGrayscale << 4;
	GrayscaleTAB_R[2] = RGrayscale << 1;
	GrayscaleTAB_R[1] = RGrayscale;
	if(RGrayscale != 0)
		GrayscaleTAB_R[0] = 2;
	else
		GrayscaleTAB_R[0] = 0;
	
	GrayscaleTAB_G[3] = GGrayscale << 4;
	GrayscaleTAB_G[2] = GGrayscale << 1;
	GrayscaleTAB_G[1] = GGrayscale;
	if(GGrayscale != 0)
		GrayscaleTAB_G[0] = 2;
	else
		GrayscaleTAB_G[0] = 0;
	
	GrayscaleTAB_B[3] = BGrayscale << 4;
	GrayscaleTAB_B[2] = BGrayscale << 1;
	GrayscaleTAB_B[1] = BGrayscale;
	if(BGrayscale != 0)
		GrayscaleTAB_B[0] = 2;
	else
		GrayscaleTAB_B[0] = 0;
	
	//ǰ��������Ϊ��
	for (i = 0; i < 5; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 5; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_G[8 - i]);
	}
	for (i = 8; i < 9; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[i - 8]);
	}
	for (i = 9; i < 13; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 13; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[16 - i]);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 4; i < 5; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_G[i - 4]);
	}
	for (i = 5; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 8; i < 9; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[i - 8]);
	}
	for (i = 9; i < 13; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 13; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[16 - i]);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_Meteor_D1_D2_D3
*	����˵��: ����D1_D2_D3������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�����ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_Meteor_D1_D2_D3(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	GrayscaleTAB_R[3] = RGrayscale << 4;
	GrayscaleTAB_R[2] = RGrayscale << 4;
	GrayscaleTAB_R[1] = RGrayscale << 1;
	if(RGrayscale != 0)
	{
		GrayscaleTAB_R[0] = 2;
	}
	else
	{
		GrayscaleTAB_R[0] = 0;
	}
	
	GrayscaleTAB_G[3] = GGrayscale << 4;
	GrayscaleTAB_G[2] = GGrayscale << 4;
	GrayscaleTAB_G[1] = GGrayscale << 1;
	if(GGrayscale != 0)
	{
		GrayscaleTAB_G[0] = 2;
	}
	else
	{
		GrayscaleTAB_G[0] = 0;
	}
	
	GrayscaleTAB_B[3] = BGrayscale << 4;
	GrayscaleTAB_B[2] = BGrayscale << 4;
	GrayscaleTAB_B[1] = BGrayscale << 1;
	if(BGrayscale != 0)
	{
		GrayscaleTAB_B[0] = 2;
	}
	else
	{
		GrayscaleTAB_B[0] = 0;
	}
	
	//ǰ��������Ϊ��
	for (i = 0; i < 5; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 5; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_G[7 - i]);
	}
	for (i = 8; i < 13; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 13; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[15 - i]);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 13; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 13; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[15 - i]);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_Meteor_D2_D3_D4
*	����˵��: ����D2_D3_D4������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�����ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_Meteor_D2_D3_D4(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	GrayscaleTAB_R[3] = RGrayscale << 4;
	GrayscaleTAB_R[2] = RGrayscale << 4;
	GrayscaleTAB_R[1] = RGrayscale << 1;
	if(RGrayscale != 0)
	{
		GrayscaleTAB_R[0] = 2;
	}
	else
	{
		GrayscaleTAB_R[0] = 0;
	}
	
	GrayscaleTAB_G[3] = GGrayscale << 4;
	GrayscaleTAB_G[2] = GGrayscale << 4;
	GrayscaleTAB_G[1] = GGrayscale << 1;
	if(GGrayscale != 0)
	{
		GrayscaleTAB_G[0] = 2;
	}
	else
	{
		GrayscaleTAB_G[0] = 0;
	}
	
	GrayscaleTAB_B[3] = BGrayscale << 4;
	GrayscaleTAB_B[2] = BGrayscale << 4;
	GrayscaleTAB_B[1] = BGrayscale << 1;
	if(BGrayscale != 0)
	{
		GrayscaleTAB_B[0] = 2;
	}
	else
	{
		GrayscaleTAB_B[0] = 0;
	}
	
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 4; i < 7; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_G[6 - i]);
	}
	for (i = 7; i < 12; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 12; i < 15; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[14 - i]);
	}
	for (i = 15; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 12; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 12; i < 15; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[14 - i]);
	}
	for (i = 15; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_Meteor_D4_D5_D6
*	����˵��: ����D4_D5_D6������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�����ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_Meteor_D3_D4_D5(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	GrayscaleTAB_R[3] = RGrayscale << 4;
	GrayscaleTAB_R[2] = RGrayscale << 4;
	GrayscaleTAB_R[1] = RGrayscale << 1;
	if(RGrayscale != 0)
	{
		GrayscaleTAB_R[0] = 2;
	}
	else
	{
		GrayscaleTAB_R[0] = 0;
	}
	
	GrayscaleTAB_G[3] = GGrayscale << 4;
	GrayscaleTAB_G[2] = GGrayscale << 4;
	GrayscaleTAB_G[1] = GGrayscale << 1;
	if(GGrayscale != 0)
	{
		GrayscaleTAB_G[0] = 2;
	}
	else
	{
		GrayscaleTAB_G[0] = 0;
	}
	
	GrayscaleTAB_B[3] = BGrayscale << 4;
	GrayscaleTAB_B[2] = BGrayscale << 4;
	GrayscaleTAB_B[1] = BGrayscale << 1;
	if(BGrayscale != 0)
	{
		GrayscaleTAB_B[0] = 2;
	}
	else
	{
		GrayscaleTAB_B[0] = 0;
	}
	
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 4; i < 6; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_G[5 - i]);
	}
	for (i = 6; i < 11; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 11; i < 14; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[13 - i]);
	}
	for (i = 14; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 7; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 7; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_G[9 - i]);
	}
	for (i = 8; i < 11; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 11; i < 14; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[13 - i]);
	}
	for (i = 14; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_Meteor_D4_D5_D6
*	����˵��: ����D4_D5_D6������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�����ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_Meteor_D4_D5_D6(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	GrayscaleTAB_R[3] = RGrayscale << 4;
	GrayscaleTAB_R[2] = RGrayscale << 4;
	GrayscaleTAB_R[1] = RGrayscale << 1;
	if(RGrayscale != 0)
	{
		GrayscaleTAB_R[0] = 2;
	}
	else
	{
		GrayscaleTAB_R[0] = 0;
	}
	
	GrayscaleTAB_G[3] = GGrayscale << 4;
	GrayscaleTAB_G[2] = GGrayscale << 4;
	GrayscaleTAB_G[1] = GGrayscale << 1;
	if(GGrayscale != 0)
	{
		GrayscaleTAB_G[0] = 2;
	}
	else
	{
		GrayscaleTAB_G[0] = 0;
	}
	
	GrayscaleTAB_B[3] = BGrayscale << 4;
	GrayscaleTAB_B[2] = BGrayscale << 4;
	GrayscaleTAB_B[1] = BGrayscale << 1;
	if(BGrayscale != 0)
	{
		GrayscaleTAB_B[0] = 2;
	}
	else
	{
		GrayscaleTAB_B[0] = 0;
	}
	
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 4; i < 5; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_G[4 - i]);
	}
	for (i = 5; i < 10; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 10; i < 13; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[12 - i]);
	}
	for (i = 13; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 6; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 6; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_G[8 - i]);
	}
	for (i = 8; i < 10; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 10; i < 13; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[12 - i]);
	}
	for (i = 13; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_Meteor_D5_D6_D7
*	����˵��: ����D5_D6_D7������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�����ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_Meteor_D5_D6_D7(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	GrayscaleTAB_R[3] = RGrayscale << 4;
	GrayscaleTAB_R[2] = RGrayscale << 4;
	GrayscaleTAB_R[1] = RGrayscale << 1;
	if(RGrayscale != 0)
	{
		GrayscaleTAB_R[0] = 2;
	}
	else
	{
		GrayscaleTAB_R[0] = 0;
	}
	
	GrayscaleTAB_G[3] = GGrayscale << 4;
	GrayscaleTAB_G[2] = GGrayscale << 4;
	GrayscaleTAB_G[1] = GGrayscale << 1;
	if(GGrayscale != 0)
	{
		GrayscaleTAB_G[0] = 2;
	}
	else
	{
		GrayscaleTAB_G[0] = 0;
	}
	
	GrayscaleTAB_B[3] = BGrayscale << 4;
	GrayscaleTAB_B[2] = BGrayscale << 4;
	GrayscaleTAB_B[1] = BGrayscale << 1;
	if(BGrayscale != 0)
	{
		GrayscaleTAB_B[0] = 2;
	}
	else
	{
		GrayscaleTAB_B[0] = 0;
	}
	
	//ǰ��������Ϊ��
	for (i = 0; i < 9; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 9; i < 12; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[11 - i]);
	}
	for (i = 12; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 5; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 5; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_G[7 - i]);
	}
	for (i = 8; i < 9; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 9; i < 12; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[11 - i]);
	}
	for (i = 12; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_Meteor_D6_D7_D8
*	����˵��: ����D6_D7_D8������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�����ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_Meteor_D6_D7_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	GrayscaleTAB_R[3] = RGrayscale << 4;
	GrayscaleTAB_R[2] = RGrayscale << 4;
	GrayscaleTAB_R[1] = RGrayscale << 1;
	if(RGrayscale != 0)
	{
		GrayscaleTAB_R[0] = 2;
	}
	else
	{
		GrayscaleTAB_R[0] = 0;
	}
	
	GrayscaleTAB_G[3] = GGrayscale << 4;
	GrayscaleTAB_G[2] = GGrayscale << 4;
	GrayscaleTAB_G[1] = GGrayscale << 1;
	if(GGrayscale != 0)
	{
		GrayscaleTAB_G[0] = 2;
	}
	else
	{
		GrayscaleTAB_G[0] = 0;
	}
	
	GrayscaleTAB_B[3] = BGrayscale << 4;
	GrayscaleTAB_B[2] = BGrayscale << 4;
	GrayscaleTAB_B[1] = BGrayscale << 1;
	if(BGrayscale != 0)
	{
		GrayscaleTAB_B[0] = 2;
	}
	else
	{
		GrayscaleTAB_B[0] = 0;
	}
	
	//ǰ��������Ϊ��
	for (i = 0; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 8; i < 11; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[10 - i]);
	}
	for (i = 11; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 4; i < 7; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_G[6 - i]);
	}
	for (i = 7; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 8; i < 11; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[10 - i]);
	}
	for (i = 11; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	DM633_LATCH(DEVICE_BLUE);
}
/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_Meteor_D7_D8_D1
*	����˵��: ����D7_D8_D1������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�����ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_Meteor_D7_D8_D1(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	GrayscaleTAB_R[3] = RGrayscale << 4;
	GrayscaleTAB_R[2] = RGrayscale << 4;
	GrayscaleTAB_R[1] = RGrayscale << 1;
	if(RGrayscale != 0)
	{
		GrayscaleTAB_R[0] = 2;
	}
	else
	{
		GrayscaleTAB_R[0] = 0;
	}
	
	GrayscaleTAB_G[3] = GGrayscale << 4;
	GrayscaleTAB_G[2] = GGrayscale << 4;
	GrayscaleTAB_G[1] = GGrayscale << 1;
	if(GGrayscale != 0)
	{
		GrayscaleTAB_G[0] = 2;
	}
	else
	{
		GrayscaleTAB_G[0] = 0;
	}
	
	GrayscaleTAB_B[3] = BGrayscale << 4;
	GrayscaleTAB_B[2] = BGrayscale << 4;
	GrayscaleTAB_B[1] = BGrayscale << 1;
	if(BGrayscale != 0)
	{
		GrayscaleTAB_B[0] = 2;
	}
	else
	{
		GrayscaleTAB_B[0] = 0;
	}
	
	//ǰ��������Ϊ��
	for (i = 0; i < 7; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 7; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_G[9 - i]);
	}
	for (i = 8; i < 10; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[9 - i]);
	}
	for (i = 10; i < 15; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 15; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[17 - i]);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 4; i < 6; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_G[5 - i]);
	}
	for (i = 6; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 8; i < 10; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[9 - i]);
	}
	for (i = 10; i < 15; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 15; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[17 - i]);
	}
	DM633_LATCH(DEVICE_BLUE);
}

/*****************************************************************************************
*	�� �� ��: ctrlUled_RGBValue_Meteor_D8_D1_D2
*	����˵��: ����D8_D1_D2������ͬ�Ҷ�ֵ
*	��    ��: uint8_t RGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t GGrayscale����ɫ�Ҷ�����ֵ
			  uint8_t BGrayscale����ɫ�Ҷ�����ֵ
*	�� �� ֵ: ��
******************************************************************************************/
void ctrlUled_RGBValue_Meteor_D8_D1_D2(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	GrayscaleTAB_R[3] = RGrayscale << 4;
	GrayscaleTAB_R[2] = RGrayscale << 4;
	GrayscaleTAB_R[1] = RGrayscale << 1;
	if(RGrayscale != 0)
	{
		GrayscaleTAB_R[0] = 2;
	}
	else
	{
		GrayscaleTAB_R[0] = 0;
	}
	
	GrayscaleTAB_G[3] = GGrayscale << 4;
	GrayscaleTAB_G[2] = GGrayscale << 4;
	GrayscaleTAB_G[1] = GGrayscale << 1;
	if(GGrayscale != 0)
	{
		GrayscaleTAB_G[0] = 2;
	}
	else
	{
		GrayscaleTAB_G[0] = 0;
	}
	
	GrayscaleTAB_B[3] = BGrayscale << 4;
	GrayscaleTAB_B[2] = BGrayscale << 4;
	GrayscaleTAB_B[1] = BGrayscale << 1;
	if(BGrayscale != 0)
	{
		GrayscaleTAB_B[0] = 2;
	}
	else
	{
		GrayscaleTAB_B[0] = 0;
	}
	
	//ǰ��������Ϊ��
	for (i = 0; i < 6; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 6; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_G[8 - i]);
	}
	for (i = 8; i < 9; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[i - 8]);
	}
	for (i = 9; i < 14; i++)
	{
		ctrlUledPro(DEVICE_RED, 0);
	}
	for (i = 14; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, GrayscaleTAB_R[16 - i]);
	}
	DM633_LATCH(DEVICE_RED);
	//ǰ��������Ϊ��
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 4; i < 5; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_G[i - 4]);
	}
	for (i = 5; i < 8; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 8; i < 9; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[i - 8]);
	}
	for (i = 9; i < 14; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0);
	}
	for (i = 14; i < 16; i++)
	{
		ctrlUledPro(DEVICE_BLUE, GrayscaleTAB_B[16 - i]);
	}
	DM633_LATCH(DEVICE_BLUE);
}



