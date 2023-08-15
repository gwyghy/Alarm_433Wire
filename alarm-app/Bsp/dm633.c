/****************************************************************************
* 文件名称：dm633.c
* 作者：程海超
* 当前版本：v1.1
* 完成日期：2011-5-4
* 摘 要：简要描述本文件的功能及内容、所依赖文档。如硬件原理图版本号或文件、
* 需求文档等。
* 历史信息：
* 历史版本     完成日期      原作者                        注释
* v1.0         2010-12-22    输入原作者(或修改者)名字
*****************************************************************************
* Copyright (c) 2018，天津华宁电子有限公司 All rights reserved.
****************************************************************************/
#include "dm633.h"
#include "bsp_iic.h"
#include <ucos_ii.h>

#define LED_GRAYSCALE_VALUE		0x0320						//LED亮度设定值，电流基本上是100mA左右

/**************************************************************************************
*	函 数 名: DM633Init
*	功能说明: 配置LED驱动IC功能控制相关的GPIO
*	形    参:  无
*	返 回 值: 无
***************************************************************************************/
void DM633Init(void)
{
	//初始化锁存IO的RCU
	rcu_periph_clock_enable(RCU_PORT_DM633_DEVICE1_LAT);
	rcu_periph_clock_enable(RCU_PORT_DM633_DEVICE2_LAT);
	//初始化共阳LED_5V电压控制IO的RCU和GPIO port，置LED-5V处于断电状态
	rcu_periph_clock_enable(RCU_PORT_LED_POWER);
	gpio_init(GPIO_PORT_LED_POWER, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, LED_POWER_PIN);
	//初始状态默认共阳LED_5V处于断电状态
	LED_POWER_OFF();
	/* configure LAT GPIO port */ 
	gpio_init(GPIO_PORT_DM633_DEVICE1_LAT, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, DM633_DEVICE1_LAT_PIN);
	gpio_init(GPIO_PORT_DM633_DEVICE2_LAT, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, DM633_DEVICE2_LAT_PIN);
	//初始LATCH信号状态
	I2C_LAT_1_0();
	I2C_LAT_2_0();
	//初始化SCK和DAI的GPIO
	bsp_InitSWD(DEVICE_RED);
	bsp_InitSWD(DEVICE_BLUE);
	//切换为内部CLK，产生一次internal的RESET信号
	DM633_Internal_GCK(DEVICE_RED);
	DM633_Internal_GCK(DEVICE_BLUE);
	
	DM633_LATCH(DEVICE_RED);
	DM633_LATCH(DEVICE_BLUE);
	
	DM634_GBC(DEVICE_RED, 0xFF);			//GBC设定为100%，不设置默认为75%
	DM634_GBC(DEVICE_BLUE, 0xFF);			//GBC设定为100%，不设置默认为75%
	
}

/*****************************************************************************************
*	函 数 名: void ctrlUledPro(uint8_t device, uint16_t grayscale)
*	功能说明: CPU通过Serial Out向设备发送一组12bit数据
*	形    参: uint8_t device ：驱动芯片ID
			  uint16_t grayscale：亮度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUledPro(uint8_t device, uint16_t grayscale)
{
	SW_SendByte(device, grayscale);
}

/*****************************************************************************************
*	函 数 名: void TurnOffAllUledPro(uint8_t device)
*	功能说明: CPU向Serial Out设备发送16*12bit全0数据，输出电流为0，即关闭所有LED
*	形    参:  uint8_t device：驱动芯片ID
*	返 回 值: 无
******************************************************************************************/
void TurnOffAllUledPro(uint8_t device)
{
	uint8_t j = 0;
	
	for(j = 0; j < 16; j++)
	{
		SW_SendByte(device, 0x0000);
	}
	
	DM633_LATCH(device);			//写完一个IC，进行一次锁存
}

/*****************************************************************************************
*	函 数 名: void TurnOnUledPro(uint8_t device, uint16_t grayscale)
*	功能说明: CPU通过Serial Out向设备发送16 * 12bit灰度PWM数据，输出电流为固定值，驱动LED为固定灰度状态
*	形    参: uint8_t device ：驱动芯片ID
			  uint16_t grayscale：亮度值(0x0000-0x0fff)
*	返 回 值: 无
******************************************************************************************/
void TurnOnUledPro(uint8_t device, uint16_t grayscale)
{
	uint8_t j = 0;
	
	for(j = 0; j < 16; j++)
	{
		ctrlUledPro(device, grayscale);
	}

	DM633_LATCH(device);			//写完一个IC，进行一次锁存
}

/*****************************************************************************************
*	函 数 名: void ctrlUledTabValue_RED(uint8_t device)
*	功能说明: CPU通过Serial Out向设备发送一组 (16 * 12bit)数据
*	形    参: uint8_t device ：驱动芯片ID
*	返 回 值: 无
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

	DM633_LATCH(device);			//写完一个IC，进行一次锁存
}

/*****************************************************************************************
*	函 数 名: void ctrlUledTabValue_BLUE(uint8_t device)
*	功能说明: CPU通过Serial Out向设备发送一组 (16 * 12bit)数据
*	形    参: uint8_t device ：驱动芯片ID
*	返 回 值: 无
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

	DM633_LATCH(device);			//写完16*12bit后，发一个LATCH锁存信号
}

/*****************************************************************************************
*	函 数 名: void ctrlUledTabValue_GREEN(uint8_t device)
*	功能说明: CPU通过Serial Out向设备发送一组 (16 * 12bit)数据
*	形    参: uint8_t device ：驱动芯片ID
*	返 回 值: 无
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
*	函 数 名: ctrlUledRGBValue
*	功能说明: 控制RGB颜色灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUledRGBValue(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//前四组数据为空
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_RED, 0x0000);
	}
	//第4-8组数据为GREEN
	for (i = 4; i < 8; i++)
	{
		ctrlUledPro(DEVICE_RED, GGrayscale << 4);			//输入范围为一个字节0-255，左移4位转换为0-4095范围的12bit，所有格式相同
	}
	for (i = 8; i < 16; i++)
	{
		ctrlUledPro(DEVICE_RED, RGrayscale << 4);
	}
	DM633_LATCH(DEVICE_RED);
	//前四组数据为空
	for (i = 0; i < 4; i++)
	{
		ctrlUledPro(DEVICE_BLUE, 0x0000);
	}
	//第4-8组数据为GREEN
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
*	函 数 名: ctrlUled_RGBValue_D1
*	功能说明: 控制D1点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D1(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_D2
*	功能说明: 控制D2点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D2(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_D3
*	功能说明: 控制D3点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D3(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_D4
*	功能说明: 控制D4点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D4(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_D4
*	功能说明: 控制D4点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D5(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_D6
*	功能说明: 控制D6点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D6(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_D7
*	功能说明: 控制D7点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D7(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_D8
*	功能说明: 控制D8点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//前四组数据为空
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
	//前四组数据为空
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
//控制两个连续的灯点亮
/*****************************************************************************************
*	函 数 名: ctrlUled_RGBValue_D1_D2
*	功能说明: 控制D1_D2点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D1_D2(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_D3_D4
*	功能说明: 控制D3_D4点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D3_D4(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_D5_D6
*	功能说明: 控制D5_D6点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D5_D6(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_D7_D8
*	功能说明: 控制D7_D8点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D7_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;
	//前四组数据为空
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
	//前四组数据为空
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
//控制四个连续的灯点亮
/*****************************************************************************************
*	函 数 名: ctrlUled_RGBValue_D1_D3_D5_D7
*	功能说明: 控制D1_D3_D5_D7点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D1_D3_D5_D7(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_D2_D4_D6_D8
*	功能说明: 控制D2_D4_D6_D8点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D2_D4_D6_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_D1_D2_D3
*	功能说明: 控制D1_D2_D3点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D1_D2_D3(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_D1_D2_D3
*	功能说明: 控制D1_D2_D3点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D1_D2_D3_D4(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_D1_D2_D3_D4_D5
*	功能说明: 控制D1_D2_D3_D4_D5点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D1_D2_D3_D4_D5(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6
*	功能说明: 控制D1_D2_D3_D4_D5_D6点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6_D7
*	功能说明: 控制D1_D2_D3_D4_D5_D6_D7点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6_D7(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6_D7_D8
*	功能说明: 控制D1_D2_D3_D4_D5_D6_D7_D8点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 无
******************************************************************************************/
void ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6_D7_D8(uint8_t RGrayscale, uint8_t GGrayscale, uint8_t BGrayscale)
{
	uint8_t i = 0;

	//前四组数据为空
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
	//前四组数据为空
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

//呼吸灯效果驱动程序
/*****************************************************************************************
*	函 数 名: GetColorNum
*	功能说明: 有几个颜色需要进行组合
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
*	返 回 值: 检查1的个数，表示有几个颜色需要进行组合
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
	
	for(n = 0; casenum; casenum /= 2)			//检查1的个数，表示有几个颜色需要进行组合
		n += casenum % 2;

	return n;
}

uint16_t grayscale_r = 0;		//调整值
uint16_t grayscale_g = 0;
uint16_t grayscale_b = 0;

uint16_t RGS = 0;		//输入值
uint16_t GGS = 0;
uint16_t BGS = 0;

uint16_t StepUpDown = 0;
/****************************************************************************
 * 功能描述：清除当前发光告警标志位状态
 * 输入参数：无
 * 创建时间：
 * 创建作者：
*****************************************************************************/
void ClrBreatheSta(void)
{
	grayscale_r = 0;		//调整值
	grayscale_g = 0;
	grayscale_b = 0;
	RGS = 0;				//输入值
	GGS = 0;
	BGS = 0;
	StepUpDown = 0;			//步长
}
/*****************************************************************************************
*	函 数 名: void ctrlUledRGBValueBreathe
*	功能说明: 控制RGB颜色常亮值
*	形    参: uint8_t RGrayscale：红色灰度值
			  uint8_t GGrayscale：绿色灰度值
			  uint8_t BGrayscale：蓝色灰度值
			  uint16_t rstep：调整步长——红色
			  uint16_t gstep：调整步长——绿色
			  uint16_t bstep：调整步长——蓝色
*	返 回 值: 无
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
		case 1:				//单色模式
		case 2:				//2种颜色组合
		case 3:				//3种颜色组合
		{
			//从暗到亮的变化过程
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
				//前四组数据空
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
				//前四组数据空
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

				OSTimeDly(50);				//每间隔一段时间写入一个灰度值
			}
			//从亮到暗的变化过程
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
				if ((grayscale_r == 0) && (grayscale_g == 0) && (grayscale_b == 0))		//都减到0，重新进入累加计算
				{
					StepUpDown = 0;
				}
				//前四组数据空
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
				//前四组数据空
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

				OSTimeDly(50);				//每间隔一段时间写入一个灰度值
			}
		}
		break;
		
		default:
			break;
	}
}


//流星灯效果驱动程序
uint16_t GrayscaleTAB_R[4] = {0x0000, 0x0000, 0x0000, 0x0000};
uint16_t GrayscaleTAB_G[4] = {0x0000, 0x0000, 0x0000, 0x0000};
uint16_t GrayscaleTAB_B[4] = {0x0000, 0x0000, 0x0000, 0x0000};
/*****************************************************************************************
*	函 数 名: ctrlUled_RGBValue_Meteor_D1_D2_D3_D4
*	功能说明: 控制D1_D2_D3_D4点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度输入值
			  uint8_t GGrayscale：绿色灰度输入值
			  uint8_t BGrayscale：蓝色灰度输入值
*	返 回 值: 无
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
	
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_Meteor_D2_D3_D4_D5
*	功能说明: 控制D2_D3_D4_D5点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度输入值
			  uint8_t GGrayscale：绿色灰度输入值
			  uint8_t BGrayscale：蓝色灰度输入值
*	返 回 值: 无
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
	
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_Meteor_D3_D4_D5_D6
*	功能说明: 控制D3_D4_D5_D6点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度输入值
			  uint8_t GGrayscale：绿色灰度输入值
			  uint8_t BGrayscale：蓝色灰度输入值
*	返 回 值: 无
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
	
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_Meteor_D4_D5_D6_D7
*	功能说明: 控制D4_D5_D6_D7点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度输入值
			  uint8_t GGrayscale：绿色灰度输入值
			  uint8_t BGrayscale：蓝色灰度输入值
*	返 回 值: 无
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
	
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_Meteor_D5_D6_D7_D8
*	功能说明: 控制D5_D6_D7_D8点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度输入值
			  uint8_t GGrayscale：绿色灰度输入值
			  uint8_t BGrayscale：蓝色灰度输入值
*	返 回 值: 无
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
	
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_Meteor_D6_D7_D8_D1
*	功能说明: 控制D6_D7_D8_D1点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度输入值
			  uint8_t GGrayscale：绿色灰度输入值
			  uint8_t BGrayscale：蓝色灰度输入值
*	返 回 值: 无
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
	
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_Meteor_D7_D8_D1_D2
*	功能说明: 控制D7_D8_D1_D2点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度输入值
			  uint8_t GGrayscale：绿色灰度输入值
			  uint8_t BGrayscale：蓝色灰度输入值
*	返 回 值: 无
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
	
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_Meteor_D8_D1_D2_D3
*	功能说明: 控制D8_D1_D2_D3点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度输入值
			  uint8_t GGrayscale：绿色灰度输入值
			  uint8_t BGrayscale：蓝色灰度输入值
*	返 回 值: 无
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
	
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_Meteor_D1_D2_D3
*	功能说明: 控制D1_D2_D3点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度输入值
			  uint8_t GGrayscale：绿色灰度输入值
			  uint8_t BGrayscale：蓝色灰度输入值
*	返 回 值: 无
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
	
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_Meteor_D2_D3_D4
*	功能说明: 控制D2_D3_D4点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度输入值
			  uint8_t GGrayscale：绿色灰度输入值
			  uint8_t BGrayscale：蓝色灰度输入值
*	返 回 值: 无
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
	
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_Meteor_D4_D5_D6
*	功能说明: 控制D4_D5_D6点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度输入值
			  uint8_t GGrayscale：绿色灰度输入值
			  uint8_t BGrayscale：蓝色灰度输入值
*	返 回 值: 无
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
	
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_Meteor_D4_D5_D6
*	功能说明: 控制D4_D5_D6点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度输入值
			  uint8_t GGrayscale：绿色灰度输入值
			  uint8_t BGrayscale：蓝色灰度输入值
*	返 回 值: 无
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
	
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_Meteor_D5_D6_D7
*	功能说明: 控制D5_D6_D7点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度输入值
			  uint8_t GGrayscale：绿色灰度输入值
			  uint8_t BGrayscale：蓝色灰度输入值
*	返 回 值: 无
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
	
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_Meteor_D6_D7_D8
*	功能说明: 控制D6_D7_D8点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度输入值
			  uint8_t GGrayscale：绿色灰度输入值
			  uint8_t BGrayscale：蓝色灰度输入值
*	返 回 值: 无
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
	
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_Meteor_D7_D8_D1
*	功能说明: 控制D7_D8_D1点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度输入值
			  uint8_t GGrayscale：绿色灰度输入值
			  uint8_t BGrayscale：蓝色灰度输入值
*	返 回 值: 无
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
	
	//前四组数据为空
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
	//前四组数据为空
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
*	函 数 名: ctrlUled_RGBValue_Meteor_D8_D1_D2
*	功能说明: 控制D8_D1_D2点亮不同灰度值
*	形    参: uint8_t RGrayscale：红色灰度输入值
			  uint8_t GGrayscale：绿色灰度输入值
			  uint8_t BGrayscale：蓝色灰度输入值
*	返 回 值: 无
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
	
	//前四组数据为空
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
	//前四组数据为空
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



