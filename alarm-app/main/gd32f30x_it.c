/*!
    \file    gd32f30x_it.c
    \brief   interrupt service routines

    \version 2021-03-23, V2.0.0, demo for GD32F30x
*/

/*
    Copyright (c) 2021, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "gd32f30x_it.h"
//#include "systick.h"
#include "main.h"
//#include "stm32f4xx_it.h"

#include "beep_app.h"
#include "led_app.h"
#include "protocol.h"

__IO uint32_t uwTick;
/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while(1){
    }
}

//void hard_fault_handler_c(unsigned int * hardfault_args)
//{
//	static unsigned int stacked_r0;
//	static unsigned int stacked_r1;
//	static unsigned int stacked_r2;
//	static unsigned int stacked_r3;
//	static unsigned int stacked_r12;
//	static unsigned int stacked_lr;
//	static unsigned int stacked_pc;
//	static unsigned int stacked_psr;
//	static unsigned int SHCSR;
//	static unsigned char MFSR;
//	static unsigned char BFSR;
//	static unsigned short int UFSR;
//	static unsigned int HFSR;
//	static unsigned int DFSR;
//	static unsigned int MMAR;
//	static unsigned int BFAR;
//	stacked_r0 = ((unsigned long) hardfault_args[0]);
//	stacked_r1 = ((unsigned long) hardfault_args[1]);
//	stacked_r2 = ((unsigned long) hardfault_args[2]);
//	stacked_r3 = ((unsigned long) hardfault_args[3]);
//	stacked_r12 = ((unsigned long) hardfault_args[4]);
//	/*异常中断发生时，这个异常模式特定的物理寄存器 R14,即 lr 被设置成该异常模式将要返回的地址*/
//	stacked_lr = ((unsigned long) hardfault_args[5]); 
//	stacked_pc = ((unsigned long) hardfault_args[6]);
//	stacked_psr = ((unsigned long) hardfault_args[7]);
//	SHCSR = (*((volatile unsigned long *)(0xE000ED24))); //系统 Handler 控制及状态寄存器
//	MFSR = (*((volatile unsigned char *)(0xE000ED28))); //存储器管理 fault 状态寄存器
//	BFSR = (*((volatile unsigned char *)(0xE000ED29))); //总线 fault 状态寄存器
//	UFSR = (*((volatile unsigned short int *)(0xE000ED2A)));//用法 fault 状态寄存器
//	HFSR = (*((volatile unsigned long *)(0xE000ED2C))); //硬 fault 状态寄存器
//	DFSR = (*((volatile unsigned long *)(0xE000ED30))); //调试 fault 状态寄存器
//	MMAR = (*((volatile unsigned long *)(0xE000ED34))); //存储管理地址寄存器
//	BFAR = (*((volatile unsigned long *)(0xE000ED38))); //总线 fault 地址寄存器
//	
//	while (1);
//} 
/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SVC_Handler(void)
{
}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
//void PendSV_Handler(void)
//{
//}
/**
  * @brief This function is called to increment a global variable "uwTick"
  *        used as application time base.
  * @note In the default implementation, this variable is incremented each 1ms
  *       in SysTick ISR.
 * @note This function is declared as __weak to be overwritten in case of other
  *      implementations in user file.
  * @retval None
  */
__weak void HAL_IncTick(void)
{
	uwTick++;
}

/**
  * @brief Provide a tick value in millisecond.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @retval tick value
  */
__weak uint32_t HAL_GetTick(void)
{
	return uwTick;
}

/*!
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
u32 BeepLightRxTimeoutTimer = TIMER_CLOSED;					//声光报警器通讯状态中断超时定时器，超时立即清除报警状态，防止声光状态停不下来
void SysTick_Handler(void)
{
	#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register */
    	OS_CPU_SR  cpu_sr = 0;
	#endif
  /* USER CODE BEGIN SysTick_IRQn 0 */
	OSIntEnter();                                    //进入中断
	OSTimeTick();                                    //调用ucos的时钟服务函数
  /* USER CODE END SysTick_IRQn 0 */
	HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */
	OS_ENTER_CRITICAL();
	if (BeepLightRxTimeoutTimer && (BeepLightRxTimeoutTimer != TIMER_CLOSED))
	{
		BeepLightRxTimeoutTimer --;
		if (BeepLightRxTimeoutTimer == TIMER_EXPIRED)			//CAN通讯状态中断，立即清除报警状态
		{
			ClrCurrentLightSta();
			ClrCurrentBeepSta();
			BeepLightRxTimeoutTimer = TIMER_CLOSED;
		}
	}
	OS_EXIT_CRITICAL();
	
	OSIntExit();                                     //触发任务切换软中断
  /* USER CODE END SysTick_IRQn 1 */
}

