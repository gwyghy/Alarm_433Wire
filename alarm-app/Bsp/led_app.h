#ifndef __LED_H__
#define __LED_H__

//#include "stm32f10x.h"

/***************************************************************************************************
 LED WORK MODE
 ***************************************************************************************************/
enum
{
	LED_CLASSIC_MODE = 0,			//����ģʽ�������ϰ�����Ƶİ汾
	LED_EXTEND_MODE					//��չģʽ��������ɫ�������߼�
};

void ClrCurrentLightSta(void);
void Led_PowerOn(void);
void Led_PowerOff(void);
void AppTaskLight(void *p_arg);

u16 GetCurrentLightType(void);
void SetCurrentLightType(u16 value);

#endif


