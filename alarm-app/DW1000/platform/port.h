/*! ----------------------------------------------------------------------------
 * @file	port.h
 * @brief	HW specific definitions and functions for portability
 *
 * @attention
 *
 * Copyright 2013 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */


#ifndef PORT_H_
#define PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "l4spi.h"
#include "uwb_anchor.h"

/* Define our wanted value of CLOCKS_PER_SEC so that we have a millisecond
 * tick timer. */
#undef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 				1000

#define FP_A				121.74
#define NoiseThresh			30
#define NoiseThreshMax		60

#if STM32 
#define SPIx						    ((SPI_HandleTypeDef *)&hspi1)
#endif

/* SPI0 配置 */
#define UWB_RST_GPIO_CLK_ENABLE()		rcu_periph_clock_enable(RCU_GPIOB)
#define UWB_RST_PIN						GPIO_PIN_0
#define UWB_RST_GPORT					GPIOB

#define UWB_WAKE_GPIO_CLK_ENABLE()		rcu_periph_clock_enable(RCU_GPIOC)
#define UWB_WAKE_PIN					GPIO_PIN_5
#define UWB_WAKE_GPORT					GPIOC

#define UWB_CS_GPIO_CLK_ENABLE()		rcu_periph_clock_enable(RCU_GPIOA)
#define UWB_CS_PIN						GPIO_PIN_4
#define UWB_CS_GPORT					GPIOA

#define UWB_IRQ_GPIO_CLK_ENABLE()		rcu_periph_clock_enable(RCU_GPIOC)
#define UWB_IRQ_PIN						GPIO_PIN_4
#define UWB_IRQ_GPORT					GPIOC
#define UWB_EXTI_IRQn					EXTI4_IRQn
#define UWB_EXTI_IRQ_PRIO				0x0f//2

#define UWB_EXTI_LINE					EXTI_4

/*dw1000中断*/
#define DWM1000_IRQHandler				EXTI4_IRQHandler

#define DECAIRQ_EXTI_IRQn				UWB_IRQ_PIN
#if STM32 
	#define port_SPIx_busy_sending()			(__HAL_SPI_GET_FLAG((SPIx),(SPI_FLAG_TXE))==(RESET))
	#define port_SPIx_no_data()					(__HAL_SPI_GET_FLAG((SPIx),(SPI_I2S_FLAG_RXNE))==(RESET))

	#define port_SPIx_disable()					__HAL_SPI_DISABLE(SPIx)
	#define port_SPIx_enable()					__HAL_SPI_ENABLE(SPIx)

	#define port_SPIx_set_chip_select()			UWB_CS_GPORT->BSRR = UWB_CS_PIN 
	#define port_SPIx_clear_chip_select()		UWB_CS_GPORT->BSRR = (uint32_t)UWB_CS_PIN << 16U
#endif
#define port_SPIx_set_chip_select()			 gpio_bit_reset(UWB_CS_GPORT, UWB_CS_PIN)			//拉低片选
#define port_SPIx_clear_chip_select()		 gpio_bit_set(UWB_CS_GPORT, UWB_CS_PIN)				//置位解除片选

FlagStatus EXTI_GetITEnStatus(exti_line_enum linex);

#define port_GetEXT_IRQStatus()			EXTI_GetITEnStatus(UWB_EXTI_LINE)
#define port_DisableEXT_IRQ()			nvic_irq_disable(UWB_EXTI_IRQn)							//HAL_NVIC_DisableIRQ(UWB_EXTI_IRQn)
#define port_EnableEXT_IRQ()			nvic_irq_enable(UWB_EXTI_IRQn, UWB_EXTI_IRQ_PRIO, 0)	//HAL_NVIC_EnableIRQ(UWB_EXTI_IRQn)
#define port_CheckEXT_IRQ()				GPIO_ReadInputDataBit(UWB_IRQ_GPORT, UWB_IRQ_PIN)

//int NVIC_DisableDECAIRQ(void);


//int is_IRQ_enabled(void);

/* DW1000 IRQ (EXTI0_IRQ) handler type. */
typedef void (*port_DecaISR_f)(void);


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn peripherals_init()
 *
 * @brief Initialise all peripherals.
 *
 * @param none
 *
 * @return none
 */
void peripherals_init (void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn port_set_deca_isr()
 *
 * @brief This function is used to install the handling function for DW1000 IRQ.
 *
 * NOTE:
 *   - As EXTI5_9_IRQHandler does not check that port_deca_isr is not null, the user application must ensure that a
 *     proper handler is set by calling this function before any DW1000 IRQ occurs!
 *   - This function makes sure the DW1000 IRQ line is deactivated while the handler is installed.
 *
 * @param deca_isr function pointer to DW1000 interrupt handler to install
 *
 * @return none
 */
void port_set_deca_isr(port_DecaISR_f deca_isr);

void SPI_ChangeRate(uint16_t scalingfactor);
void SPI_ConfigFastRate(uint16_t scalingfactor);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn SPI_DW1000_SetRateLow()
 *
 * @brief Set SPI rate to less than 3 MHz to properly perform DW1000 initialisation.
 *
 * @param none
 *
 * @return none
 */
void SPI_DW1000_SetRateLow (void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn SPI_DW1000_SetRateHigh()
 *
 * @brief Set SPI rate as close to 20 MHz as possible for optimum performances.
 *
 * @param none
 *
 * @return none
 */
void SPI_DW1000_SetRateHigh (void);

unsigned long portGetTickCnt(void);

#define portGetTickCount() 			portGetTickCnt()

void Reset_DW1000(void);
void setup_DW1000RSTnIRQ(int enable);
void DW1000GpioInit(void);

void NoiseFigureCal(dwt_rxdiag_t *rxdiag, sFrameQuality *framequa);
double FP_PowerLevelCal(dwt_rxdiag_t *rxdiag);
double RX_PowerLevelCal(dwt_rxdiag_t *rxdiag);
void DistAdjust(sDistList_N *midpoint, sFrameQuality *framequa_poll, sFrameQuality *framequa_final, uint16_t *val);

#ifdef __cplusplus
}
#endif

#endif /* PORT_H_ */
