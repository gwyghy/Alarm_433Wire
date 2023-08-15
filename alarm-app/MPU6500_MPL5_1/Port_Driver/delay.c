#include "delay.h"

static u8 fac_us;
//static u16 fac_ms;

//��ʼ���ӳٺ���
//SYSTICK��ʱ�ӹ̶�ΪHCLKʱ�ӵ�1/8
//SYSCLK:ϵͳʱ��
//void Delay_Init(void)
//{
//	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//ѡ���ⲿʱ��  HCLK/8
//	fac_us = SystemCoreClock / 8000000;			//Ϊϵͳʱ�ӵ�1/8
//	fac_ms = fac_us * 1000;						//��OS��,����ÿ��ms��Ҫ��systickʱ���� 
//}

//��ʱnus
//nusΪҪ��ʱ��us��.		    								   
void Delay_us(u32 nus)
{
	SysTick->VAL = 0x00;										//��ռ�����
	SysTick->LOAD = nus * fac_us;								//ʱ�����	 
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;					//��ʼ����
	while(!(SysTick->CTRL&SysTick_CTRL_COUNTFLAG_Msk));		//�ȴ�ʱ�䵽��
	
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;				//�رռ�����
	SysTick->VAL=0x00;										//��ռ�����
}

//��ʱnms
//nms:Ҫ��ʱ��ms��
//ָ�����ڣ�1MHZʱ1���ִ��1.25M��ָ�32MHZʱ��1ms��ִ��40000��ָ��
//�˴�ʹ��50000����Ϊ��ֵ����os�£���֤������
 void delay_ms(u32 nms)
{
  u32 i;
  
  for(; nms != 0; nms--)	
       for (i=0; i < 5000; i++);	//5000��Ϊ500����ʹ�ϵ��ʼ��ʱ�����1�룬��δ���ڿ�������Ϊ50Ҳ���Թ���������ʼ��ʱ����500ʱ��ͬ	parry 2021.7.5
}





