/*! ----------------------------------------------------------------------------
 * @file	deca_spi.c
 * @brief	SPI access functions
 *
 * @attention
 *
 * Copyright 2013 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */
#include <string.h>

#include "deca_spi.h"
#include "deca_device_api.h"
#include "port.h"

#if STM32
extern SPI_HandleTypeDef hspi1;
#endif
#if INVOKED		//声明实际未被调用
uint8_t Dummy[100];
/*! ------------------------------------------------------------------------------------------------------------------
 * Function: openspi()
 *
 * Low level abstract function to open and initialise access to the SPI device.
 * returns 0 for success, or -1 for error
 */
int openspi(/*SPI_TypeDef* SPIx*/)
{
	// done by port.c, default SPI used is SPI1

	return 0;

}

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: closespi()
 *
 * Low level abstract function to close the the SPI device.
 * returns 0 for success, or -1 for error
 */
int closespi(void)
{
	while (port_SPIx_busy_sending()); //wait for tx buffer to empty

	port_SPIx_disable();

	return 0;

}
#endif
#if INVOKED		//声明实际未被调用
/*! ------------------------------------------------------------------------------------------------------------------
 * Function: bWriteToSPI()
 *
 * Low level abstract function to write to the SPI
 * Takes two separate byte buffers for write header and write data
 * returns 0 for success, or -1 for error
 */
//#pragma GCC optimize ("O3")

int bWriteToSPI(uint16 headerLength, const uint8 *headerBuffer, uint32 bodylength, const uint8 *bodyBuffer)
{
	uint16_t i=0;
	uint8_t tmp;

    decaIrqStatus_t  stat ;

	stat = DecaMutexON() ;

	port_SPIx_set_chip_select();
	
//	HAL_SPI_TransmitReceive(&hspi1,(uint8_t *)headerBuffer,(uint8_t *)bodyBuffer,headerLength,HAL_MAX_DELAY);

//    for(i=0; i<headerLength; i++)
//    {
//			while(((SPI1->SR & SPI_FLAG_TXE) == RESET));
//    	SPI1->DR = (uint8_t)headerBuffer[i];

//    	while ((SPI1->SR & SPI_FLAG_RXNE) == (uint16_t)RESET);

//    	SPI1->DR ;
//    }

//    for(i=0; i<bodylength; i++)
//    {
//			while(((SPI1->SR & SPI_FLAG_TXE) == RESET));
//     	SPI1->DR = (uint8_t)bodyBuffer[i];

//    	while((SPI1->SR & SPI_FLAG_RXNE) == (uint16_t)RESET);

//		SPI1->DR ;
//	}
		
	for(i=0; i<headerLength; i++)
    {
		while(((SPI1->SR & SPI_FLAG_TXE) == RESET))
				;
    	*(__IO uint8_t *)&SPI1->DR = (uint8_t)headerBuffer[i];

    	while ((SPI1->SR & SPI_FLAG_RXNE) == (uint16_t)RESET);

    	tmp = SPI1->DR ;
    }

    for(i=0; i<bodylength; i++)
    {
		while(((SPI1->SR & SPI_FLAG_TXE) == RESET));
     	*(__IO uint8_t *)&SPI1->DR = (uint8_t)bodyBuffer[i];

    	while((SPI1->SR & SPI_FLAG_RXNE) == (uint16_t)RESET);

		  tmp = SPI1->DR ;
		}

	port_SPIx_clear_chip_select();

    DecaMutexOFF(stat) ;

    return 0;
} // end bWriteToSPI()


/*! ------------------------------------------------------------------------------------------------------------------
 * Function: bReadFromSPI()
 *
 * Low level abstract function to read from the SPI
 * Takes two separate byte buffers for write header and read data
 * returns the offset into read buffer where first byte of read data may be found,
 * or returns -1 if there was an error
 */
//#pragma GCC optimize ("O3")
int bReadFromSPI(uint16 headerLength, const uint8 *headerBuffer, uint32 readlength, uint8 *readBuffer)
{
	uint16_t i = 0;

    decaIrqStatus_t  stat ;

    stat = DecaMutexON() ;

    /* Wait for SPIx Tx buffer empty */
    //while (port_SPIx_busy_sending());

    port_SPIx_set_chip_select();
//    for(i = 0; i < headerLength; i++)
//    {
//			
//			while(((SPI1->SR & SPI_FLAG_TXE) == RESET));
//    	SPI1->DR = (uint8_t)headerBuffer[i];

//     	while((SPI1->SR & SPI_FLAG_RXNE) == (uint16_t)RESET);

//     	readBuffer[0] = SPI1->DR ; // Dummy read as we write the header
//    }

//    for(i=0; i<readlength; i++)
//    {
//    	SPI1->DR = (uint8_t)0;  // Dummy write as we read the message body

//    	while((SPI1->SR & SPI_FLAG_RXNE) == (uint16_t)RESET);
// 
//	   	readBuffer[i] = SPI1->DR ;//port_SPIx_receive_data(); //this clears RXNE bit
//    }

    for(i = 0; i < headerLength; i++)
    {
		while(((SPI1->SR & SPI_FLAG_TXE) == RESET));
    	*(__IO uint8_t *)&SPI1->DR = (uint8_t)headerBuffer[i];

     	while((SPI1->SR & SPI_FLAG_RXNE) == (uint16_t)RESET);

     	readBuffer[0] = SPI1->DR ; // Dummy read as we write the header
    }

    for(i = 0; i < readlength; i++)
    {
    	*(__IO uint8_t *)&SPI1->DR = (uint8_t)0;  // Dummy write as we read the message body

    	while((SPI1->SR & SPI_FLAG_RXNE) == (uint16_t)RESET);
 
	   	readBuffer[i] = SPI1->DR ;//port_SPIx_receive_data(); //this clears RXNE bit
    }

    port_SPIx_clear_chip_select();
    DecaMutexOFF(stat) ;

    return 0;
} // end bReadFromSPI()
#endif
