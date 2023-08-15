/*
*********************************************************************************************************
*
*	模块名称 : SWD总线驱动模块
*	文件名称 : bsp_i2c.c
*	版    本 : V1.0
*	说    明 : 用gpio模拟Serial Wire总线
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0
*
*
*
*********************************************************************************************************
*/
#include "bsp_iic.h"
#include "dm633.h"
/****************************************************************************************************
*	函 数 名: void bsp_InitSWD(uint8_t device)
*	功能说明: 配置Serial Wire总线的GPIO，采用模拟IO翻转的方式实现
*	形    参: device:1或2
*	返 回 值: 无
*****************************************************************************************************/
void bsp_InitSWD(uint8_t device)
{
	if (device == 1)
	{
		//先使能外设IO时钟	
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
		//先使能外设IO时钟
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
*	函 数 名: SW_Delay
*	功能说明: Serial_Wire总线位延迟，最快25MHz
*	形    参: 无
*	返 回 值: 无
***************************************************************************************************/
static void SW_Delay(void)
{
	uint8_t i;

	for (i = 0; i < 3; i++)
	{
		;		//空语句或nop()
	}
}

/************************************************************************************************
*	函 数 名: void SW_SendByte(uint8_t device, uint16_t _ucByte)
*	功能说明: CPU向SWD总线设备发送一个通道12bit数据，送16次即可送完一个IC
*	形    参:   device ： 设备ID
				_ucByte ：发送的字节
*	返 回 值: 无
************************************************************************************************/
void SW_SendByte(uint8_t device, uint16_t _ucByte)
{
	uint8_t i;

	if (_ucByte > 0x0fff)			//DM633灰度等级0-4095，所有只有12bit有效位，不能超过0x0fff
		_ucByte = 0x0fff;

	if (device == 1)
	{
		for (i = 12; i > 0; i--)
		{
			I2C_SCL_1_0();			//on the rising edge of the CLK,the serial-in data will be clocked 
									//into 12 * 16 bit shift registers synchronized, so input "0" first
			SW_Delay();   			//延时用于CLK时钟

			if (_ucByte & 0x0800)		//MSB先发
			{
				I2C_SDA_1_1();			//逻辑“1”为高电平
			}
			else
			{
				I2C_SDA_1_0();			//逻辑“0”为低电平
			}
			SW_Delay();   		//延时用于SDA数据稳定
			I2C_SCL_1_1();		//rising edge
			SW_Delay();   		//延时用于CLK时钟
			_ucByte <<= 1;		//移位取下一个bit
		}
		I2C_SCL_1_0();			//单个结束，恢复DCK信号0
		I2C_SDA_1_0();			//DAI置位数据后清零
	}
	else
	{
		for (i = 12; i > 0; i--)
		{
			I2C_SCL_2_0();				//on the rising edge of the CLK,so SET 0 first
			
			SW_Delay();   		//延时用于CLK时钟

			if (_ucByte & 0x0800)		//MSB先发
			{
				I2C_SDA_2_1();
			}
			else
			{
				I2C_SDA_2_0();
			}
			SW_Delay();   		//延时用于SDA数据稳定
			I2C_SCL_2_1();
			SW_Delay();   		//延时用于CLK时钟
			_ucByte <<= 1;		//左移一位，取下一bit
		}
		I2C_SCL_2_0();			//单个结束，恢复DCK信号0
		I2C_SDA_2_0();			//DAI置位数据后清零
	}
}

/************************************************************************************************
*	函 数 名: void DM633_LATCH(uint8_t device)
*	功能说明: 数据锁存使能
*	形    参: device ： 设备ID
*	返 回 值: 无
************************************************************************************************/
void DM633_LATCH(uint8_t device)
{
	if(device == 1)
	{
		SW_Delay();
		I2C_SCL_1_0();
		SW_Delay();				//DCK置低延时一段时间
		I2C_LAT_1_1();			//锁存置高
		SW_Delay();
		SW_Delay();
		I2C_LAT_1_0();			//锁存置低
	}
	else
	{
		SW_Delay();
		I2C_SCL_2_0();
		SW_Delay();				//DCK置低延时一段时间
		I2C_LAT_2_1();			//锁存置高
		SW_Delay();
		SW_Delay();
		I2C_LAT_2_0();			//锁存置低
	}
}

/************************************************************************************************
*	函 数 名: void DM634_GBC(uint8_t device, uint8_t _ucByte)
*	功能说明: Global Brightness Correction数据
*	形    参: device ： 设备ID；uint8_t _ucByte：GBC数值，最大值0x7F（127）LSB为detection function
*	返 回 值: 无
************************************************************************************************/
void DM634_GBC(uint8_t device, uint8_t _ucByte)
{
	uint16_t i;

	if(device == 1)
	{
		I2C_SCL_1_1();		//时钟脉冲DCK位于高电平
		SW_Delay();   		//延时用于CLK时钟
		
		for (i = 4; i > 0; i--)			//给四个LAT信号
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
			SW_Delay();   		//延时用于CLK时钟
			if (_ucByte & 0x80)
			{
				I2C_SDA_1_1();
			}
			else
			{
				I2C_SDA_1_0();
			}
			I2C_SCL_1_1();
			SW_Delay();   		//延时用于CLK时钟
			_ucByte <<= 1;
		}

		I2C_SCL_1_0();			//on the rising edge of the CLK,so SET 0 first
		SW_Delay();   			//延时用于CLK时钟
		I2C_LAT_1_1();
		SW_Delay(); 
		I2C_LAT_1_0();
	}
	else
	{
		I2C_SCL_2_1();		//时钟脉冲DCK位于高电平
		SW_Delay();   		//延时用于CLK时钟
		
		for (i = 4; i > 0; i--)			//给四个LAT信号
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
			SW_Delay();   		//延时用于CLK时钟
			if (_ucByte & 0x80)
			{
				I2C_SDA_2_1();
			}
			else
			{
				I2C_SDA_2_0();
			}
			I2C_SCL_2_1();
			SW_Delay();   		//延时用于CLK时钟
			_ucByte <<= 1;
		}

		I2C_SCL_2_0();				//on the rising edge of the CLK,so SET 0 first
		SW_Delay();   		//延时用于CLK时钟
		I2C_LAT_2_1();
		SW_Delay(); 
		I2C_LAT_2_0();
	}
}

/*********************************************************************
     DM633 Shift to Free-running Mode  (Internal GCK Mode) / Default mode
**********************************************************************/
void DM633_Internal_GCK(uint8_t device) //内部晶振模式。DM633默认采用这种模式。建议如果一般不是做显示屏，最好采用此模式
{ 
	if(device == 1)
	{
		I2C_SCL_1_1();
		SW_Delay();
		//一个CLK 包含四个LAT上升沿
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
		//一个LAT 包含三个DCK上升沿
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
		//一个CLK 包含四个LAT上升沿
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
		//一个LAT 包含三个DCK上升沿
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
void DM633_External_GCK(void) //切换到外部晶振模式，此时PWM晶振信号从GCK端输入。一般不建议采用这种模式
{
	I2C_SCL_1_1();
	SW_Delay();
	//一个CLK 包含三个LAT上升沿
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
	//一个LAT 包含两个DCK上升沿
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

