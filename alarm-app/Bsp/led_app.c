/****************************************************************************
* �ļ����ƣ�led_app.c
* ���ߣ���������(���޸���)����
* ��ǰ�汾��v1.1
* ������ڣ�2011-5-4
* ժ Ҫ��LED�߼�
* �����ĵ��ȡ�
* ��ʷ��Ϣ��
* ��ʷ�汾     �������      ԭ����                        ע��
* v1.0         2010-12-22    ����ԭ����(���޸���)����
*****************************************************************************
* Copyright (c) 2018��������������޹�˾ All rights reserved.
****************************************************************************/
#include "bsp_iic.h"
#include "dm633.h"
#include "can_app.h"
#include "main.h"
#include "led_app.h"

u16 LightFlag = 0;						//LED��������ģʽ
u16 LightType = LED_CLASSIC_MODE;		//���ⱨ��LED��������

u8 RGrayScale = 0;
u8 GGrayScale = 0;
u8 BGrayScale = 0;

u8 LED_NUM = 0;				//���ݵ�������ѡ��ͬ������ʽ
u8 WaterFlow_Type = 0;		//LED��ˮ��ģʽ

u16 Breathestep_R = 0;
u16 Breathestep_G = 0;
u16 Breathestep_B = 0;
				
u16 u16LightOffTimer = 0;		//��Ъ��ʱ��x*1ms��
u16 u16LightOnTimer = 0;		//�澯��ʱ��x*1ms��
/****************************************************************************
 * ���������������ǰ����澯��־λ״̬
 * �����������
 * ����ʱ�䣺
 * �������ߣ�
*****************************************************************************/
void ClrCurrentLightSta(void)
{
	if ((LightFlag == 0x03)
	&& (GetCurrentLightType() == LED_EXTEND_MODE))		//������Ч��
	{
		ClrBreatheSta();		//��������߼�ʹ�õ���״̬����
	}
	
	if (LightFlag)			//��0��ֵ
	{
		LightFlag = 0;			//�����澯״̬
		LightType = 0;			//���ⱨ����������
	}

	RGrayScale = 0;
	GGrayScale = 0;
	BGrayScale = 0;

	LED_NUM = 0;			//���ݵ�������ѡ��ͬ������ʽ
	WaterFlow_Type = 0;		//LED��ˮ��ģʽ

	Breathestep_R = 0;
	Breathestep_G = 0;
	Breathestep_B = 0;

	u16LightOffTimer = 0;		//��Ъ��ʱ��x*10ms��
	u16LightOnTimer = 0;		//�澯��ʱ��x*10ms��
}
/****************************************************************************
*   �� �� ��:  IfLightFlagChange
*   ����˵��:  ����ģʽ�Ƿ����仯
*   ��    ��:  u16 Value������ֵ
*   �� �� ֵ:  ��
****************************************************************************/
u8 IfLightFlagChange(u16 Value)
{
	if (LightFlag != Value)			//��0��ֵ
	{
		return TRUE;
	}
	
	return FALSE;
}
/**********************************************************************************************************
*	������������ȡ��ǰ��������״̬
*	��    �Σ���
*   �� �� ֵ:  LightType�������� 0-���䷽ʽ��1����չ��������ʽ
**********************************************************************************************************/
u16 GetCurrentLightType(void)
{
	return LightType;
}
/**********************************************************************************************************
*	�����������趨��ǰ��������״̬
*	 ��    ��:��u16 value���������� 0-���䷽ʽ��1����չ��������ʽ
*   �� �� ֵ:  ��
**********************************************************************************************************/
void SetCurrentLightType(u16 value)
{
	LightType = value;
}
/****************************************************************************
*   �� �� ��:  Led_PowerOn
*   ����˵��:  ����LED��Դ���ƣ���
*   ��    ��:  ��
*   �� �� ֵ:  ��
****************************************************************************/
void Led_PowerOn(void) 
{
	LED_POWER_ON();
}
/****************************************************************************
*   �� �� ��:  Led_PowerOn
*   ����˵��:  ����LED��Դ���ƣ���
*   ��    ��:  ��
*   �� �� ֵ:  ��
****************************************************************************/
void Led_PowerOff(void) 
{
	LED_POWER_OFF();
}
/****************************************************************************
*   �� �� ��:  AppTaskLight
*   ����˵��:  ���ⱨ��������LED��������
*   ��    ��:  ��
*   �� �� ֵ:  ��
****************************************************************************/
void AppTaskLight(void *p_arg)
{
	DM633Init();			//LED����оƬ�ӿ�

	TurnOffAllUledPro(DEVICE_RED);
	TurnOffAllUledPro(DEVICE_BLUE);

	p_arg = p_arg;

	while(1)
	{
		if(GetCurrentLightType() == LED_CLASSIC_MODE)			//��׼
		{
			if (LightFlag == 0x10)		//���Ƴ���
			{
				TurnOnUledPro(DEVICE_RED, 0x0000);			//��������
				ctrlUledTabValue_BLUE(DEVICE_BLUE);			//����������

				OSTimeDly(200);			//���һ��ʱ�䣬����һֱ����LED������
			}
			else if(LightFlag == 0x11)						// �����˸���̶�200ms����
			{
				ctrlUledTabValue_RED(DEVICE_RED);			//����������
				TurnOffAllUledPro(DEVICE_BLUE);				//��������
				OSTimeDly(200);

				TurnOffAllUledPro(DEVICE_RED);				//��������
				TurnOffAllUledPro(DEVICE_BLUE);				//��������
				OSTimeDly(200);
			}
			else if(LightFlag == 0x12)					// ������˸���̶�200ms����
			{
				TurnOnUledPro(DEVICE_RED, 0x0000);			//��������
				ctrlUledTabValue_BLUE(DEVICE_BLUE);			//����������
				OSTimeDly(200);

				TurnOffAllUledPro(DEVICE_RED);				//��������
				TurnOffAllUledPro(DEVICE_BLUE);				//��������
				OSTimeDly(200);
			}
			else if(LightFlag == 0x21)					// �����˸����������
			{
				if(u16LightOnTimer != 0)
				{
					ctrlUledTabValue_RED(DEVICE_RED);			//����������
					TurnOffAllUledPro(DEVICE_BLUE);				//��������

					OSTimeDly(u16LightOnTimer);
				}
				if(u16LightOnTimer == 0 || u16LightOffTimer != 0)
				{
					TurnOffAllUledPro(DEVICE_RED);				//��������
					TurnOffAllUledPro(DEVICE_BLUE);				//��������

					OSTimeDly(u16LightOffTimer);				// ������
				}
			}
			else if(LightFlag == 0x22)							// ������˸����������
			{
				if(u16LightOnTimer != 0)
				{
					TurnOnUledPro(DEVICE_RED, 0x0000);			//��������
					ctrlUledTabValue_BLUE(DEVICE_BLUE);			//����������

					OSTimeDly(u16LightOnTimer);
				}
				if(u16LightOnTimer == 0 || u16LightOffTimer != 0)
				{
					TurnOffAllUledPro(DEVICE_RED);				//��������
					TurnOffAllUledPro(DEVICE_BLUE);				//��������

					OSTimeDly(u16LightOffTimer);				// ������200mS
				}
			}
			else if(LightFlag == 0x23)								// ����������˸
			{
				if(u16LightOnTimer != 0)
				{
					ctrlUledTabValue_RED(DEVICE_RED);			//����������
					TurnOffAllUledPro(DEVICE_BLUE);				//��������
					OSTimeDly(u16LightOnTimer);					// ����������������
				}
				if(u16LightOnTimer == 0 || u16LightOffTimer != 0)
				{
					TurnOffAllUledPro(DEVICE_RED);				//��������
					ctrlUledTabValue_BLUE(DEVICE_BLUE);			//����������

					OSTimeDly(u16LightOffTimer);					// ����������������
				}
			}
			//����һ�����͵����ⱨ������ѡ��������ƵƳ�����������������������������ƽ�����˸
			else if(LightFlag == 3)				//Ԥ����һ�ַ�ʽ����һ���õõ�
			{
				;
			}
			else if (LightFlag == 1)							// Ԥ�������Ƴ���
			{
				ctrlUledTabValue_RED(DEVICE_RED);				//����������
				TurnOnUledPro(DEVICE_BLUE, 0x0000);				//��������
				
				OSTimeDly(200);			//���һ��ʱ�䣬����һֱ����LED������
			}
			else if(LightFlag == 2)								// ������������ƽ�����˸
			{
				ctrlUledTabValue_RED(DEVICE_RED);				//����������
				TurnOnUledPro(DEVICE_BLUE, 0x0000);				//��������
				OSTimeDly(200);					//������200mS

				TurnOnUledPro(DEVICE_RED, 0x0000);				//��������
				ctrlUledTabValue_BLUE(DEVICE_BLUE);				//����������
				OSTimeDly(200);					//������200mS
			}
			else if (LightFlag == AGING_TEST_BYTE)				// �����豸�ϻ�ʹ�ã���������LED�����ϻ�������
			{
				TurnOnUledPro(DEVICE_RED, 0x0fff);
				TurnOnUledPro(DEVICE_BLUE, 0x0fff);

				OSTimeDly(200);					//
			}
			else if (LightFlag == POWER_TEST_BYTE)				// ���ڲ�����󹦺�ʹ�ã��˴���������LED
			{
				TurnOnUledPro(DEVICE_RED, 0x0fff);
				TurnOnUledPro(DEVICE_BLUE, 0x0fff);
				
				OSTimeDly(200);					//
			}
			else if (LightFlag == LED_TEST_BYTE)				// ���ڲ���LED�Ƿ������������˴�˳�����������ɫ��LED
			{
				ctrlUledTabValue_RED(DEVICE_RED);		//����������
				TurnOnUledPro(DEVICE_BLUE, 0x0000);		//��������
				OSTimeDly(500);					//������500mS
				ctrlUledTabValue_GREEN(DEVICE_RED);		//����������
				ctrlUledTabValue_GREEN(DEVICE_BLUE);	//����������
				OSTimeDly(500);					//������500mS
				TurnOnUledPro(DEVICE_RED, 0x0000);		//��������
				ctrlUledTabValue_BLUE(DEVICE_BLUE);		//����������
				OSTimeDly(500);					//������500mS
			}
			else if(LightFlag == 0)								//����״̬
			{
				//�ر�����
				TurnOffAllUledPro(DEVICE_RED);
				TurnOffAllUledPro(DEVICE_BLUE);
				Led_PowerOff();					//�ض�LED-5V����
				DM633_Internal_GCK(DEVICE_RED);
				DM633_Internal_GCK(DEVICE_BLUE);

				OSTimeDly(200);					//���һ��ʱ����йرղ�����
			}
			else								//����״̬����
			{
				TurnOffAllUledPro(DEVICE_RED);
				TurnOffAllUledPro(DEVICE_BLUE);
			}
		}
		else if(GetCurrentLightType() == LED_EXTEND_MODE)			//��չģʽ�½�����ɫ�ƿ����߼�
		{
			if (LightFlag == 0x01)		//����˸Ч��
			{
				if(u16LightOnTimer != 0)
				{
					ctrlUledRGBValue(RGrayScale, GGrayScale, BGrayScale);

					OSTimeDly(u16LightOnTimer);					// ����������������
				}
				if(u16LightOnTimer == 0 || u16LightOffTimer != 0)
				{
					//�ر�����
					TurnOffAllUledPro(DEVICE_BLUE);
					TurnOffAllUledPro(DEVICE_RED);

					OSTimeDly(u16LightOffTimer);					// ����������������
				}
			}
			else if (LightFlag == 0x02)		//��ˮ��Ч��
			{
				if (WaterFlow_Type == 1)			//1�����������2��׷�����
				{
					if (LED_NUM == 1)		//���ݵ�������ѡ��ͬ������ʽ
					{
						if(u16LightOnTimer != 0)
						{
							//��������ˮģʽ
							ctrlUled_RGBValue_D1(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// ����������������
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D2(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// ����������������
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D3(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// ����������������
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D4(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// ����������������
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D5(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// ����������������
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D6(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// ����������������
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D7(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// ����������������
							if (IfLightFlagChange(0x02) == TRUE)
							{
								continue;
							}
							ctrlUled_RGBValue_D8(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// ����������������
						}
					}
					else if (LED_NUM == 2)		//���ݵ�������ѡ��ͬ������ʽ
					{
						if(u16LightOnTimer != 0)
						{
							//��������ˮģʽ
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
							OSTimeDly(u16LightOnTimer);					// ����������������
						}
					}
					else if (LED_NUM == 4)		//���ݵ�������ѡ��ͬ������ʽ
					{
						if(u16LightOnTimer != 0)
						{
							//�ĸ�����ˮģʽ
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
				else if (WaterFlow_Type == 2)			//1�����������2��׷�����
				{
					if (LED_NUM == 1)		//���ݵ�������ѡ��ͬ������ʽ
					{
						if(u16LightOnTimer != 0)
						{
							//������׷����ˮģʽ
							ctrlUled_RGBValue_D1(RGrayScale, GGrayScale, BGrayScale);
							OSTimeDly(u16LightOnTimer);					// ����������������
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
							TurnOnUledPro(DEVICE_RED, 0x0000);		//��������
							TurnOnUledPro(DEVICE_BLUE, 0x0000);	//��������
							OSTimeDly(u16LightOnTimer);					// ����������������
						}
					}
					else if (LED_NUM == 2)		//���ݵ�������ѡ��ͬ������ʽ
					{
						if(u16LightOnTimer != 0)
						{
							//������׷����ˮģʽ
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
							TurnOnUledPro(DEVICE_RED, 0x0000);		//��������
							TurnOnUledPro(DEVICE_BLUE, 0x0000);	//��������
							OSTimeDly(u16LightOnTimer);					// ����������������
						}
					}
					else if (LED_NUM == 4)		//���ݵ�������ѡ��ͬ������ʽ
					{
						if(u16LightOnTimer != 0)
						{
							//�ĸ���׷����ˮģʽ
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
							TurnOnUledPro(DEVICE_RED, 0x0000);		//��������
							TurnOnUledPro(DEVICE_BLUE, 0x0000);	//��������
							OSTimeDly(u16LightOnTimer);					// ����������������
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
			else if (LightFlag == 0x03)		//������Ч��
			{
				ctrlUledRGBValueBreathe(RGrayScale, GGrayScale, BGrayScale, Breathestep_R, Breathestep_G, Breathestep_B);
			}
			else if (LightFlag == 0x04)		//���ǵ�Ч��
			{
				if (LED_NUM == 3)		//���ݵ�������ѡ��ͬ������ʽ
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
				else if (LED_NUM == 4)		//���ݵ�������ѡ��ͬ������ʽ
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
			else if (LightFlag == 0x05)		//��ɫ���л���˸Ч��
			{
				if(u16LightOnTimer != 0)		//����ʱ����Ч
				{
					if ((LED_NUM & 0x01) == 0x01)			//��ɫ
					{
						ctrlUledRGBValue(RGrayScale, 0, 0);
						OSTimeDly(u16LightOnTimer);					// ����������������
					}
					if ((LED_NUM & 0x02) == 0x02)			//��ɫ
					{
						ctrlUledRGBValue(0, GGrayScale, 0);
						OSTimeDly(u16LightOnTimer);					// ����������������
					}
					if ((LED_NUM & 0x04) == 0x04)			//��ɫ
					{
						ctrlUledRGBValue(0, 0, BGrayScale);
						OSTimeDly(u16LightOnTimer);					// ����������������
					}
					if ((LED_NUM & 0x08) == 0x08)			//��ɫ
					{
						ctrlUledRGBValue(RGrayScale, GGrayScale, 0);
						OSTimeDly(u16LightOnTimer);					// ����������������
					}
					if ((LED_NUM & 0x10) == 0x10)			//��ɫ
					{
						ctrlUledRGBValue(RGrayScale, 0, BGrayScale);
						OSTimeDly(u16LightOnTimer);					// ����������������
					}
					if ((LED_NUM & 0x20) == 0x20)			//��ɫ
					{
						ctrlUledRGBValue(0, GGrayScale, BGrayScale);
						OSTimeDly(u16LightOnTimer);					// ����������������
					}
				}
			}
			else
			{
				;		//to be continued;δ�����...
			}
		}

		OSTimeDly(10);		//ϵͳ����10ms
	}
}


