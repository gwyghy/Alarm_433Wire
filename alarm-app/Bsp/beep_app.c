/***************************************************************************
* 版权所有：2014, 天津华宁电子有限公司
*
* 文件名称：beep.c
* 文件标识：
* 内容摘要：蜂鸣器
* 其它说明：
* 当前版本：

*
* 修改记录1：
*    修改日期：
*    版 本 号：
*    修 改 人：
*    修改内容：
**********************************************************************/
#include "beep_app.h"
#include "can_app.h"


u16 BeepCaseFlag;
u16 u16BeepOffTimer = 0;	//间歇计时（x*1ms）
u16 u16BeepOnTimer = 0;		//告警计时（x*1ms）

/**********************************************************************************************************
*	函 数 名: bsp_InitBeep
*	功能说明: 配置蜂鸣器控制相关的GPIO
*	形    参:  无
*	返 回 值: 无
**********************************************************************************************************/
void bsp_InitBeep(void) 
{
	//beep
	/* enable the beep clock */
    rcu_periph_clock_enable(BUZZER_GPIO);
    /* configure beep GPIO port */ 
    gpio_init(BUZZER_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, BUZZER_PIN);

	gpio_bit_reset(BUZZER_PORT, BUZZER_PIN);
}

/**********************************************************************************************************
 * 功能描述：获取当前发声告警标志位状态
 * 输入参数：无
 * 创建时间：
 * 创建作者：
**********************************************************************************************************/
u8 GetCurrentBeepSta(void)
{
	if (BeepCaseFlag == 1)				//标准发声逻辑
	{
		return 1;
	}
	else if (BeepCaseFlag == 0x11)		//可配置发声逻辑
	{
		return 0x11;
	}
	else if (BeepCaseFlag == 0xffff)	//测试系统最大电流
	{
		return 0xff;
	}
	else
	{
		return 0;
	}
}

/**********************************************************************************************************
 * 功能描述：清除当前发声告警标志位状态
 * 输入参数：无
 * 创建时间：
 * 创建作者：
**********************************************************************************************************/
void ClrCurrentBeepSta(void)
{
	if (BeepCaseFlag)			//非0的值
	{
		BeepCaseFlag = 0;
	}
}

/*******************************************************************************************
* 函数名称：AppTaskBeep(void *pdata)
* 功能描述：蜂鸣器发声控制任务
* 入口参数：*pdata
* 出口参数：无
* 使用说明：无
********************************************************************************************/
void AppTaskBeep(void *p_arg)
{
	p_arg = p_arg;

	bsp_InitBeep();			//蜂鸣器IO初始化
	
	while(1)
	{
		if(GetCurrentBeepSta() == 1)				//标准发声逻辑
		{
			//声音一长一短|__发声400mS__|_停止200mS_|_发声200mS_|__停止400mS__|
			gpio_bit_set(BUZZER_PORT, BUZZER_PIN);
			OSTimeDly(400);
			gpio_bit_reset(BUZZER_PORT, BUZZER_PIN);
			OSTimeDly(200);
			gpio_bit_set(BUZZER_PORT, BUZZER_PIN);
			OSTimeDly(200);
			gpio_bit_reset(BUZZER_PORT, BUZZER_PIN);
			OSTimeDly(400);
		}
		else if(GetCurrentBeepSta() == 0x11)		//可配置发声逻辑，配置化发声时间和静默时间，实现声音告警的功能
		{
			if(u16BeepOnTimer != 0)				//发声时间不为0
			{
				gpio_bit_set(BUZZER_PORT, BUZZER_PIN);
				OSTimeDly(u16BeepOnTimer);
			}
			if(u16BeepOffTimer != 0 || u16BeepOnTimer == 0)				//发声间歇不为0
			{
				gpio_bit_reset(BUZZER_PORT, BUZZER_PIN);
				OSTimeDly(u16BeepOffTimer);
			}
			if(u16BeepOnTimer != 0)				//发声时间不为0
			{
				gpio_bit_set(BUZZER_PORT, BUZZER_PIN);
				OSTimeDly(u16BeepOnTimer);
			}
			if(u16BeepOffTimer != 0 || u16BeepOnTimer == 0)				//发声间歇不为0
			{
				gpio_bit_reset(BUZZER_PORT, BUZZER_PIN);
				OSTimeDly(u16BeepOffTimer);
			}
		}
		else if(GetCurrentBeepSta() == 0xff)			//用于设备老化使用，驱动蜂鸣器，测试系统最大电流
		{
			gpio_bit_set(BUZZER_PORT, BUZZER_PIN);
		}
		else											//无驱动功能，保持静默状态
		{
			gpio_bit_reset(BUZZER_PORT, BUZZER_PIN);
			OSTimeDly(100);
		}
	
		OSTimeDly(10);
	}
}


