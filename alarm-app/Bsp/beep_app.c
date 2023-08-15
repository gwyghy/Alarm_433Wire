/***************************************************************************
* ��Ȩ���У�2014, ������������޹�˾
*
* �ļ����ƣ�beep.c
* �ļ���ʶ��
* ����ժҪ��������
* ����˵����
* ��ǰ�汾��

*
* �޸ļ�¼1��
*    �޸����ڣ�
*    �� �� �ţ�
*    �� �� �ˣ�
*    �޸����ݣ�
**********************************************************************/
#include "beep_app.h"
#include "can_app.h"


u16 BeepCaseFlag;
u16 u16BeepOffTimer = 0;	//��Ъ��ʱ��x*1ms��
u16 u16BeepOnTimer = 0;		//�澯��ʱ��x*1ms��

/**********************************************************************************************************
*	�� �� ��: bsp_InitBeep
*	����˵��: ���÷�����������ص�GPIO
*	��    ��:  ��
*	�� �� ֵ: ��
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
 * ������������ȡ��ǰ�����澯��־λ״̬
 * �����������
 * ����ʱ�䣺
 * �������ߣ�
**********************************************************************************************************/
u8 GetCurrentBeepSta(void)
{
	if (BeepCaseFlag == 1)				//��׼�����߼�
	{
		return 1;
	}
	else if (BeepCaseFlag == 0x11)		//�����÷����߼�
	{
		return 0x11;
	}
	else if (BeepCaseFlag == 0xffff)	//����ϵͳ������
	{
		return 0xff;
	}
	else
	{
		return 0;
	}
}

/**********************************************************************************************************
 * ���������������ǰ�����澯��־λ״̬
 * �����������
 * ����ʱ�䣺
 * �������ߣ�
**********************************************************************************************************/
void ClrCurrentBeepSta(void)
{
	if (BeepCaseFlag)			//��0��ֵ
	{
		BeepCaseFlag = 0;
	}
}

/*******************************************************************************************
* �������ƣ�AppTaskBeep(void *pdata)
* ����������������������������
* ��ڲ�����*pdata
* ���ڲ�������
* ʹ��˵������
********************************************************************************************/
void AppTaskBeep(void *p_arg)
{
	p_arg = p_arg;

	bsp_InitBeep();			//������IO��ʼ��
	
	while(1)
	{
		if(GetCurrentBeepSta() == 1)				//��׼�����߼�
		{
			//����һ��һ��|__����400mS__|_ֹͣ200mS_|_����200mS_|__ֹͣ400mS__|
			gpio_bit_set(BUZZER_PORT, BUZZER_PIN);
			OSTimeDly(400);
			gpio_bit_reset(BUZZER_PORT, BUZZER_PIN);
			OSTimeDly(200);
			gpio_bit_set(BUZZER_PORT, BUZZER_PIN);
			OSTimeDly(200);
			gpio_bit_reset(BUZZER_PORT, BUZZER_PIN);
			OSTimeDly(400);
		}
		else if(GetCurrentBeepSta() == 0x11)		//�����÷����߼������û�����ʱ��;�Ĭʱ�䣬ʵ�������澯�Ĺ���
		{
			if(u16BeepOnTimer != 0)				//����ʱ�䲻Ϊ0
			{
				gpio_bit_set(BUZZER_PORT, BUZZER_PIN);
				OSTimeDly(u16BeepOnTimer);
			}
			if(u16BeepOffTimer != 0 || u16BeepOnTimer == 0)				//������Ъ��Ϊ0
			{
				gpio_bit_reset(BUZZER_PORT, BUZZER_PIN);
				OSTimeDly(u16BeepOffTimer);
			}
			if(u16BeepOnTimer != 0)				//����ʱ�䲻Ϊ0
			{
				gpio_bit_set(BUZZER_PORT, BUZZER_PIN);
				OSTimeDly(u16BeepOnTimer);
			}
			if(u16BeepOffTimer != 0 || u16BeepOnTimer == 0)				//������Ъ��Ϊ0
			{
				gpio_bit_reset(BUZZER_PORT, BUZZER_PIN);
				OSTimeDly(u16BeepOffTimer);
			}
		}
		else if(GetCurrentBeepSta() == 0xff)			//�����豸�ϻ�ʹ�ã�����������������ϵͳ������
		{
			gpio_bit_set(BUZZER_PORT, BUZZER_PIN);
		}
		else											//���������ܣ����־�Ĭ״̬
		{
			gpio_bit_reset(BUZZER_PORT, BUZZER_PIN);
			OSTimeDly(100);
		}
	
		OSTimeDly(10);
	}
}


