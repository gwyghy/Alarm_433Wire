/****************************************************************************
* 文件名称：led_app.c
* 作者：输入作者(或修改者)名字
* 当前版本：v1.1
* 完成日期：2011-5-4
* 摘 要：LED逻辑
* 需求文档等。
* 历史信息：
* 历史版本     完成日期      原作者                        注释
* v1.0         2010-12-22    输入原作者(或修改者)名字
*****************************************************************************
* Copyright (c) 2018，天津华宁电子有限公司 All rights reserved.
****************************************************************************/
#include "bsp_iic.h"
#include "dm633.h"
#include "can_app.h"
#include "main.h"
#include "led_app.h"

u16 LightFlag = 0;						//LED驱动工作模式
u16 LightType = LED_CLASSIC_MODE;		//声光报警LED驱动类型

u8 RGrayScale = 0;
u8 GGrayScale = 0;
u8 BGrayScale = 0;

u8 LED_NUM = 0;				//根据灯珠数量选择不同驱动方式
u8 WaterFlow_Type = 0;		//LED流水灯模式

u16 Breathestep_R = 0;
u16 Breathestep_G = 0;
u16 Breathestep_B = 0;
				
u16 u16LightOffTimer = 0;		//间歇计时（x*1ms）
u16 u16LightOnTimer = 0;		//告警计时（x*1ms）
/****************************************************************************
 * 功能描述：清除当前发光告警标志位状态
 * 输入参数：无
 * 创建时间：
 * 创建作者：
*****************************************************************************/
void ClrCurrentLightSta(void)
{
	if ((LightFlag == 0x03)
	&& (GetCurrentLightType() == LED_EXTEND_MODE))		//呼吸灯效果
	{
		ClrBreatheSta();		//清呼吸灯逻辑使用到的状态变量
	}
	
	if (LightFlag)			//非0的值
	{
		LightFlag = 0;			//清零光告警状态
		LightType = 0;			//声光报警驱动类型
	}

	RGrayScale = 0;
	GGrayScale = 0;
	BGrayScale = 0;

	LED_NUM = 0;			//根据灯珠数量选择不同驱动方式
	WaterFlow_Type = 0;		//LED流水灯模式

	Breathestep_R = 0;
	Breathestep_G = 0;
	Breathestep_B = 0;

	u16LightOffTimer = 0;		//间歇计时（x*10ms）
	u16LightOnTimer = 0;		//告警计时（x*10ms）
}
/****************************************************************************
*   函 数 名:  IfLightFlagChange
*   功能说明:  发光模式是否发生变化
*   形    参:  u16 Value：运行值
*   返 回 值:  无
****************************************************************************/
u8 IfLightFlagChange(u16 Value)
{
	if (LightFlag != Value)			//非0的值
	{
		return TRUE;
	}
	
	return FALSE;
}
/**********************************************************************************************************
*	功能描述：获取当前发光类型状态
*	形    参：无
*   返 回 值:  LightType发光类型 0-经典方式，1，扩展可配置形式
**********************************************************************************************************/
u16 GetCurrentLightType(void)
{
	return LightType;
}
/**********************************************************************************************************
*	功能描述：设定当前发光类型状态
*	 形    参:：u16 value：发光类型 0-经典方式，1，扩展可配置形式
*   返 回 值:  无
**********************************************************************************************************/
void SetCurrentLightType(u16 value)
{
	LightType = value;
}
/****************************************************************************
*   函 数 名:  Led_PowerOn
*   功能说明:  发光LED电源控制：开
*   形    参:  无
*   返 回 值:  无
****************************************************************************/
void Led_PowerOn(void) 
{
	LED_POWER_ON();
}
/****************************************************************************
*   函 数 名:  Led_PowerOn
*   功能说明:  发光LED电源控制：开
*   形    参:  无
*   返 回 值:  无
****************************************************************************/
void Led_PowerOff(void) 
{
	LED_POWER_OFF();
}
/****************************************************************************
*   函 数 名:  AppTaskLight
*   功能说明:  声光报警器发光LED控制任务
*   形    参:  无
*   返 回 值:  无
****************************************************************************/
void AppTaskLight(void *p_arg)
{
	DM633Init();			//LED驱动芯片接口

	TurnOffAllUledPro(DEVICE_RED);
	TurnOffAllUledPro(DEVICE_BLUE);

	p_arg = p_arg;

	while(1)
	{
		if(GetCurrentLightType() == LED_CLASSIC_MODE)			//标准
		{
			if (LightFlag == 0x10)		//蓝灯常亮
			{
				TurnOnUledPro(DEVICE_RED, 0x0000);			//红灭，绿灭
				ctrlUledTabValue_BLUE(DEVICE_BLUE);			//蓝亮，绿灭

				OSTimeDly(200);			//间隔一段时间，无需一直驱动LED？？？
			}
			else if(LightFlag == 0x11)						// 红灯闪烁，固定200ms周期
			{
				ctrlUledTabValue_RED(DEVICE_RED);			//红亮，绿灭
				TurnOffAllUledPro(DEVICE_BLUE);				//蓝灭，绿灭
				OSTimeDly(200);

				TurnOffAllUledPro(DEVICE_RED);				//红灭，绿灭
				TurnOffAllUledPro(DEVICE_BLUE);				//蓝灭，绿灭
				OSTimeDly(200);
			}
			else if(LightFlag == 0x12)					// 蓝灯闪烁，固定200ms周期
			{
				TurnOnUledPro(DEVICE_RED, 0x0000);			//红灭，绿灭
				ctrlUledTabValue_BLUE(DEVICE_BLUE);			//蓝亮，绿灭
				OSTimeDly(200);

				TurnOffAllUledPro(DEVICE_RED);				//红灭，绿灭
				TurnOffAllUledPro(DEVICE_BLUE);				//蓝灭，绿灭
				OSTimeDly(200);
			}
			else if(LightFlag == 0x21)					// 红灯闪烁，配置周期
			{
				if(u16LightOnTimer != 0)
				{
					ctrlUledTabValue_RED(DEVICE_RED);			//红亮，绿灭
					TurnOffAllUledPro(DEVICE_BLUE);				//绿灭，蓝灭

					OSTimeDly(u16LightOnTimer);
				}
				if(u16LightOnTimer == 0 || u16LightOffTimer != 0)
				{
					TurnOffAllUledPro(DEVICE_RED);				//红灭，绿灭
					TurnOffAllUledPro(DEVICE_BLUE);				//蓝灭，绿灭

					OSTimeDly(u16LightOffTimer);				// 交替间隔
				}
			}
			else if(LightFlag == 0x22)							// 蓝灯闪烁，配置周期
			{
				if(u16LightOnTimer != 0)
				{
					TurnOnUledPro(DEVICE_RED, 0x0000);			//红灭，绿灭
					ctrlUledTabValue_BLUE(DEVICE_BLUE);			//蓝亮，绿灭

					OSTimeDly(u16LightOnTimer);
				}
				if(u16LightOnTimer == 0 || u16LightOffTimer != 0)
				{
					TurnOffAllUledPro(DEVICE_RED);				//红灭，绿灭
					TurnOffAllUledPro(DEVICE_BLUE);				//蓝灭，绿灭

					OSTimeDly(u16LightOffTimer);				// 交替间隔200mS
				}
			}
			else if(LightFlag == 0x23)								// 红蓝交替闪烁
			{
				if(u16LightOnTimer != 0)
				{
					ctrlUledTabValue_RED(DEVICE_RED);			//红亮，绿灭
					TurnOffAllUledPro(DEVICE_BLUE);				//绿灭，蓝灭
					OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
				}
				if(u16LightOnTimer == 0 || u16LightOffTimer != 0)
				{
					TurnOffAllUledPro(DEVICE_RED);				//红灭，绿灭
					ctrlUledTabValue_BLUE(DEVICE_BLUE);			//蓝亮，绿灭

					OSTimeDly(u16LightOffTimer);					// 交替间隔，配置周期
				}
			}
			//另外一种类型的声光报警器，选中两个红灯灯常亮，动作两个红灯与另外两个蓝灯交替闪烁
			else if(LightFlag == 3)				//预留的一种方式，不一定用得到
			{
				;
			}
			else if (LightFlag == 1)							// 预警命令，红灯常亮
			{
				ctrlUledTabValue_RED(DEVICE_RED);				//红亮，蓝灭
				TurnOnUledPro(DEVICE_BLUE, 0x0000);				//绿灭，蓝灭
				
				OSTimeDly(200);			//间隔一段时间，无需一直驱动LED？？？
			}
			else if(LightFlag == 2)								// 动作命令，红蓝灯交替闪烁
			{
				ctrlUledTabValue_RED(DEVICE_RED);				//红亮，蓝灭
				TurnOnUledPro(DEVICE_BLUE, 0x0000);				//绿灭，蓝灭
				OSTimeDly(200);					//交替间隔200mS

				TurnOnUledPro(DEVICE_RED, 0x0000);				//绿灭，蓝灭
				ctrlUledTabValue_BLUE(DEVICE_BLUE);				//蓝亮，红灭
				OSTimeDly(200);					//交替间隔200mS
			}
			else if (LightFlag == AGING_TEST_BYTE)				// 用于设备老化使用，点亮所有LED，不老化蜂鸣器
			{
				TurnOnUledPro(DEVICE_RED, 0x0fff);
				TurnOnUledPro(DEVICE_BLUE, 0x0fff);

				OSTimeDly(200);					//
			}
			else if (LightFlag == POWER_TEST_BYTE)				// 用于测量最大功耗使用，此处点亮所有LED
			{
				TurnOnUledPro(DEVICE_RED, 0x0fff);
				TurnOnUledPro(DEVICE_BLUE, 0x0fff);
				
				OSTimeDly(200);					//
			}
			else if (LightFlag == LED_TEST_BYTE)				// 用于测量LED是否正常点亮，此处顺序点亮三种颜色的LED
			{
				ctrlUledTabValue_RED(DEVICE_RED);		//红亮，绿灭
				TurnOnUledPro(DEVICE_BLUE, 0x0000);		//蓝灭，绿灭
				OSTimeDly(500);					//交替间隔500mS
				ctrlUledTabValue_GREEN(DEVICE_RED);		//绿亮，红灭
				ctrlUledTabValue_GREEN(DEVICE_BLUE);	//绿亮，蓝灭
				OSTimeDly(500);					//交替间隔500mS
				TurnOnUledPro(DEVICE_RED, 0x0000);		//红灭，绿灭
				ctrlUledTabValue_BLUE(DEVICE_BLUE);		//蓝亮，绿灭
				OSTimeDly(500);					//交替间隔500mS
			}
			else if(LightFlag == 0)								//空闲状态
			{
				//关闭所有
				TurnOffAllUledPro(DEVICE_RED);
				TurnOffAllUledPro(DEVICE_BLUE);
				Led_PowerOff();					//关断LED-5V供电
				DM633_Internal_GCK(DEVICE_RED);
				DM633_Internal_GCK(DEVICE_BLUE);

				OSTimeDly(200);					//间隔一定时间进行关闭操作。
			}
			else								//其它状态灯灭
			{
				TurnOffAllUledPro(DEVICE_RED);
				TurnOffAllUledPro(DEVICE_BLUE);
			}
		}
		else if(GetCurrentLightType() == LED_EXTEND_MODE)			//扩展模式下进行三色灯控制逻辑
		{
			if (LightFlag == 0x01)		//灯闪烁效果
			{
				if(u16LightOnTimer != 0)
				{
					ctrlUledRGBValue(RGrayScale, GGrayScale, BGrayScale);

					OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
				}
				if(u16LightOnTimer == 0 || u16LightOffTimer != 0)
				{
					//关闭所有
					TurnOffAllUledPro(DEVICE_BLUE);
					TurnOffAllUledPro(DEVICE_RED);

					OSTimeDly(u16LightOffTimer);					// 交替间隔，配置周期
				}
			}
			else if (LightFlag == 0x02)		//流水灯效果
			{
				if (WaterFlow_Type == 1)			//1：逐个点亮，2：追逐点亮
				{
					if (LED_NUM == 1)		//根据灯珠数量选择不同驱动方式
					{
						if(u16LightOnTimer != 0)
						{
							//单个灯流水模式
							ctrlUled_RGBValue_D1(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D2(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D3(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D4(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D5(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D6(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D7(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D8(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
						}
					}
					else if (LED_NUM == 2)		//根据灯珠数量选择不同驱动方式
					{
						if(u16LightOnTimer != 0)
						{
							//两个灯流水模式
							ctrlUled_RGBValue_D1_D2(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							
							ctrlUled_RGBValue_D3_D4(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							
							ctrlUled_RGBValue_D5_D6(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							
							ctrlUled_RGBValue_D7_D8(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
						}
					}
					else if (LED_NUM == 4)		//根据灯珠数量选择不同驱动方式
					{
						if(u16LightOnTimer != 0)
						{
							//四个灯流水模式
							ctrlUled_RGBValue_D1_D3_D5_D7(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							
							ctrlUled_RGBValue_D2_D4_D6_D8(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);
						}
					}
				}
				else if (WaterFlow_Type == 2)			//1：逐个点亮，2：追逐点亮
				{
					if (LED_NUM == 1)		//根据灯珠数量选择不同驱动方式
					{
						if(u16LightOnTimer != 0)
						{
							//单个灯追逐流水模式
							ctrlUled_RGBValue_D1(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							
							ctrlUled_RGBValue_D1_D2(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D1_D2_D3(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D1_D2_D3_D4(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D1_D2_D3_D4_D5(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6_D7(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6_D7_D8(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);
							TurnOnUledPro(DEVICE_RED, 0x0000);		//红灭，绿灭
							TurnOnUledPro(DEVICE_BLUE, 0x0000);	//蓝灭，绿灭
							OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
						}
					}
					else if (LED_NUM == 2)		//根据灯珠数量选择不同驱动方式
					{
						if(u16LightOnTimer != 0)
						{
							//两个灯追逐流水模式
							ctrlUled_RGBValue_D1_D2(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							
							ctrlUled_RGBValue_D1_D2_D3_D4(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6_D7_D8(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							TurnOnUledPro(DEVICE_RED, 0x0000);		//红灭，绿灭
							TurnOnUledPro(DEVICE_BLUE, 0x0000);	//蓝灭，绿灭
							OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
						}
					}
					else if (LED_NUM == 4)		//根据灯珠数量选择不同驱动方式
					{
						if(u16LightOnTimer != 0)
						{
							//四个灯追逐流水模式
							ctrlUled_RGBValue_D1_D2_D3_D4(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D1_D2_D3_D4_D5_D6_D7_D8(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							TurnOnUledPro(DEVICE_RED, 0x0000);		//红灭，绿灭
							TurnOnUledPro(DEVICE_BLUE, 0x0000);	//蓝灭，绿灭
							OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
						}
					}
					else
					{
						;
					}
				}
				else
				{
					;
				}
			}
			else if (LightFlag == 0x03)		//呼吸灯效果
			{
				ctrlUledRGBValueBreathe(RGrayScale, GGrayScale, BGrayScale, Breathestep_R, Breathestep_G, Breathestep_B);
			}
			else if (LightFlag == 0x04)		//流星灯效果
			{
				if (LED_NUM == 3)		//根据灯珠数量选择不同驱动方式
				{
					ctrlUled_RGBValue_Meteor_D1_D2_D3(RGrayScale, GGrayScale, BGrayScale);
					OSTimeDly(u16LightOnTimer);
					if (IfLightFlagChange(0x04) == TRUE)
					{
						continue;
					}
					ctrlUled_RGBValue_Meteor_D2_D3_D4(RGrayScale, GGrayScale, BGrayScale);
					OSTimeDly(u16LightOnTimer);
					if (IfLightFlagChange(0x04) == TRUE)
					{
						continue;
					}
					ctrlUled_RGBValue_Meteor_D3_D4_D5(RGrayScale, GGrayScale, BGrayScale);
					OSTimeDly(u16LightOnTimer);
					if (IfLightFlagChange(0x04) == TRUE)
					{
						continue;
					}
					ctrlUled_RGBValue_Meteor_D4_D5_D6(RGrayScale, GGrayScale, BGrayScale);
					OSTimeDly(u16LightOnTimer);
					if (IfLightFlagChange(0x04) == TRUE)
					{
						continue;
					}
					ctrlUled_RGBValue_Meteor_D5_D6_D7(RGrayScale, GGrayScale, BGrayScale);
					OSTimeDly(u16LightOnTimer);
					if (IfLightFlagChange(0x04) == TRUE)
					{
						continue;
					}
					ctrlUled_RGBValue_Meteor_D6_D7_D8(RGrayScale, GGrayScale, BGrayScale);
					OSTimeDly(u16LightOnTimer);
					if (IfLightFlagChange(0x04) == TRUE)
					{
						continue;
					}
					ctrlUled_RGBValue_Meteor_D7_D8_D1(RGrayScale, GGrayScale, BGrayScale);
					OSTimeDly(u16LightOnTimer);
					if (IfLightFlagChange(0x04) == TRUE)
					{
						continue;
					}
					ctrlUled_RGBValue_Meteor_D8_D1_D2(RGrayScale, GGrayScale, BGrayScale);
					OSTimeDly(u16LightOnTimer);
				}
				else if (LED_NUM == 4)		//根据灯珠数量选择不同驱动方式
				{
					ctrlUled_RGBValue_Meteor_D1_D2_D3_D4(RGrayScale, GGrayScale, BGrayScale);
					OSTimeDly(u16LightOnTimer);
					if (IfLightFlagChange(0x04) == TRUE)
					{
						continue;
					}
					ctrlUled_RGBValue_Meteor_D2_D3_D4_D5(RGrayScale, GGrayScale, BGrayScale);
					OSTimeDly(u16LightOnTimer);
					if (IfLightFlagChange(0x04) == TRUE)
					{
						continue;
					}
					ctrlUled_RGBValue_Meteor_D3_D4_D5_D6(RGrayScale, GGrayScale, BGrayScale);
					OSTimeDly(u16LightOnTimer);
					if (IfLightFlagChange(0x04) == TRUE)
					{
						continue;
					}
					ctrlUled_RGBValue_Meteor_D4_D5_D6_D7(RGrayScale, GGrayScale, BGrayScale);
					OSTimeDly(u16LightOnTimer);
					if (IfLightFlagChange(0x04) == TRUE)
					{
						continue;
					}
					ctrlUled_RGBValue_Meteor_D5_D6_D7_D8(RGrayScale, GGrayScale, BGrayScale);
					OSTimeDly(u16LightOnTimer);
					if (IfLightFlagChange(0x04) == TRUE)
					{
						continue;
					}
					ctrlUled_RGBValue_Meteor_D6_D7_D8_D1(RGrayScale, GGrayScale, BGrayScale);
					OSTimeDly(u16LightOnTimer);
					if (IfLightFlagChange(0x04) == TRUE)
					{
						continue;
					}
					ctrlUled_RGBValue_Meteor_D7_D8_D1_D2(RGrayScale, GGrayScale, BGrayScale);
					OSTimeDly(u16LightOnTimer);
					if (IfLightFlagChange(0x04) == TRUE)
					{
						continue;
					}
					ctrlUled_RGBValue_Meteor_D8_D1_D2_D3(RGrayScale, GGrayScale, BGrayScale);
					OSTimeDly(u16LightOnTimer);
				}
			}
			else if (LightFlag == 0x05)		//彩色灯切换闪烁效果
			{
				if(u16LightOnTimer != 0)		//点亮时间有效
				{
					if ((LED_NUM & 0x01) == 0x01)			//红色
					{
						ctrlUledRGBValue(RGrayScale, 0, 0);
						OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
					}
					if ((LED_NUM & 0x02) == 0x02)			//绿色
					{
						ctrlUledRGBValue(0, GGrayScale, 0);
						OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
					}
					if ((LED_NUM & 0x04) == 0x04)			//蓝色
					{
						ctrlUledRGBValue(0, 0, BGrayScale);
						OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
					}
					if ((LED_NUM & 0x08) == 0x08)			//黄色
					{
						ctrlUledRGBValue(RGrayScale, GGrayScale, 0);
						OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
					}
					if ((LED_NUM & 0x10) == 0x10)			//粉色
					{
						ctrlUledRGBValue(RGrayScale, 0, BGrayScale);
						OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
					}
					if ((LED_NUM & 0x20) == 0x20)			//青色
					{
						ctrlUledRGBValue(0, GGrayScale, BGrayScale);
						OSTimeDly(u16LightOnTimer);					// 交替间隔，配置周期
					}
				}
			}
			else
			{
				;		//to be continued;未完待续...
			}
		}

		OSTimeDly(10);		//系统调度10ms
	}
}


