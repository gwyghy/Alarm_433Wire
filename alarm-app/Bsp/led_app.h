#ifndef __LED_H__
#define __LED_H__

//#include "stm32f10x.h"

/***************************************************************************************************
 LED WORK MODE
 ***************************************************************************************************/
enum
{
	LED_CLASSIC_MODE = 0,			//经典模式，兼容老版红蓝灯的版本
	LED_EXTEND_MODE					//扩展模式，用于颜色的配置逻辑
};

void ClrCurrentLightSta(void);
void Led_PowerOn(void);
void Led_PowerOff(void);
void AppTaskLight(void *p_arg);

u16 GetCurrentLightType(void);
void SetCurrentLightType(u16 value);

#endif


