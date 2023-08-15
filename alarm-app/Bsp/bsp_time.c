/****************************************************************************
* 文件名称：bsp_time.c
* 作者：郑志春
* 当前版本：v0.1
* 完成日期：2023-03-07
* 摘 要：简要描述本文件的功能及内容、所依赖文档。如硬件原理图版本号或文件、
* 需求文档等。
* 历史信息：
* 历史版本     完成日期          原作者                        注释
* v0.1        2023-03-07        
*****************************************************************************
* Copyright (c) 2023，天津华宁电子有限公司 All rights reserved.
****************************************************************************/
#include "bsp_time.h"
#include <ucos_ii.h>

BspTimerCallback BspCallback = NULL;

/*******************************************************************************
** 函数名称: TIM3_Init
** 功能描述: 
** 参数说明: Period: [输入] 自动重装载值
**			 Prescaler: [输入] 预分频值
**           TIMx: [输入] 定时器号
**           Callback：超时回调
** 返回说明: None
** 创建人员: 郑志春
** 创建日期: 2023-03-02
**------------------------------------------------------------------------------
** 修改人员:
** 修改日期:
** 修改描述:
**------------------------------------------------------------------------------
********************************************************************************/
void BspTimer_Init(uint32_t timer_periph, uint16_t Period, uint16_t Prescaler, BspTimerCallback Callback)
{
        //使能时钟
    rcu_periph_clock_enable(RCU_TIMER1);
	timer_deinit(timer_periph);
    
    timer_parameter_struct timer_initpara;                  //定时器初始化结构
    timer_struct_para_init(&timer_initpara);                //initialize TIMER init parameter struct with a default value

    timer_initpara.prescaler         = Prescaler - 1;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;      //边沿对齐计数模式
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;        //模式,向上计数模式
    timer_initpara.period            = Period - 1; 
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;        //时钟分割
    timer_initpara.repetitioncounter = 0;
    timer_init(timer_periph,&timer_initpara);                         //按结构体的值，初始化定时器
    
	if(Callback != NULL)
	{
		BspCallback = Callback;
	}
    timer_disable(timer_periph);
    //使能中断
    timer_interrupt_enable(timer_periph,TIMER_INT_UP);
	//使能中断线
    nvic_irq_enable(TIMER1_IRQn, 0, 0);
}
/*******************************************************************************************
**函数名称：BspTimer_Control
**函数作用：控制tim的启动，停止
**函数参数：TIMx：定时器，state：启动、停止
**函数输出：无
**注意事项：无
*******************************************************************************************/
void BspTimer_Ctrl(uint32_t timer_periph, uint8_t state)
{
    if(state)
    {
        timer_enable(timer_periph);
    }
    else
    {
        timer_disable(timer_periph);
    }
    timer_interrupt_flag_clear(timer_periph, TIMER_INT_FLAG_UP);
}
/*******************************************************************************************
**函数名称：TIMER1_IRQHandler
**函数作用：定时器1中断服务程序
**函数参数：
**函数输出：无
**注意事项：无
*******************************************************************************************/
void TIMER1_IRQHandler(void)   //TIM中断
{
    uint16_t id = 0;
	OSIntEnter();
	if(timer_interrupt_flag_get(TIMER1,TIMER_INT_FLAG_UP )!= RESET)
	{
		timer_interrupt_flag_clear(TIMER1, TIMER_INT_FLAG_UP);
        BspCallback(id);
	}
	OSIntExit();
}
