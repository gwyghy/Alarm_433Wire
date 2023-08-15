#include "delay.h"

static u8 fac_us;
//static u16 fac_ms;

//初始化延迟函数
//SYSTICK的时钟固定为HCLK时钟的1/8
//SYSCLK:系统时钟
//void Delay_Init(void)
//{
//	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//选择外部时钟  HCLK/8
//	fac_us = SystemCoreClock / 8000000;			//为系统时钟的1/8
//	fac_ms = fac_us * 1000;						//非OS下,代表每个ms需要的systick时钟数 
//}

//延时nus
//nus为要延时的us数.		    								   
void Delay_us(u32 nus)
{
	SysTick->VAL = 0x00;										//清空计数器
	SysTick->LOAD = nus * fac_us;								//时间加载	 
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;					//开始倒数
	while(!(SysTick->CTRL&SysTick_CTRL_COUNTFLAG_Msk));		//等待时间到达
	
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;				//关闭计数器
	SysTick->VAL=0x00;										//清空计数器
}

//延时nms
//nms:要延时的ms数
//指令周期：1MHZ时1秒可执行1.25M条指令，32MHZ时，1ms可执行40000条指令
//此处使用50000，因为该值在无os下，验证无问题
 void delay_ms(u32 nms)
{
  u32 i;
  
  for(; nms != 0; nms--)	
       for (i=0; i < 5000; i++);	//5000减为500可以使上电初始化时间减少1秒，但未长期拷机；减为50也可以工作，但初始化时间与500时相同	parry 2021.7.5
}





