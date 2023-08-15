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

extern SPI_HandleTypeDef hspi1;

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

} // end openspi()

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

} // end closespi()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: WriteToSPI()
 *
 * Low level abstract function to write to the SPI
 * Takes two separate byte buffers for write header and write data
 * returns 0 for success, or -1 for error
 */
//#pragma GCC optimize ("O3")
int WriteToSPI(uint16 headerLength, const uint8 *headerBuffer, uint32 bodylength, const uint8 *bodyBuffer)
{

	uint16_t i=0;
	uint8_t tmp;
	uint32_t timeout;

    decaIrqStatus_t  stat ;

    stat = DecaMutexON() ;

    SPIx_CS_GPIO->BRR = D_CS_Pin;
	
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
		timeout = SPI_TIMEOUT;
		while(((SPI1->SR & SPI_FLAG_TXE) == RESET))
		{
				timeout--;
				if(timeout == 0)
				{
					goto SPI_WrError;
				}
		}
		timeout = SPI_TIMEOUT;
		
		*(__IO uint8_t *)&SPI1->DR = (uint8_t)headerBuffer[i];

		while ((SPI1->SR & SPI_FLAG_RXNE) == (uint16_t)RESET)
		{
			timeout--;
			if(timeout == 0)
			{
				goto SPI_WrError;
			}
		}

		tmp = SPI1->DR ;
	}

	for(i=0; i<bodylength; i++)
	{
		timeout = SPI_TIMEOUT;
		while(((SPI1->SR & SPI_FLAG_TXE) == RESET))
		{
			timeout--;
			if(timeout == 0)
			{
				goto SPI_WrError;
			}
		}
		timeout = SPI_TIMEOUT;
		*(__IO uint8_t *)&SPI1->DR = (uint8_t)bodyBuffer[i];

		while((SPI1->SR & SPI_FLAG_RXNE) == (uint16_t)RESET)
		{
			timeout--;
			if(timeout == 0)
			{
				goto SPI_WrError;
			}
		}

		tmp = SPI1->DR ;
	}

	SPIx_CS_GPIO->BSRR = D_CS_Pin;

	DecaMutexOFF(stat) ;

SPI_WrError:
	if(timeout == 0)
	{
		return 1;
	}
	
	return 0;
} // end WriteToSPI()


/*! ------------------------------------------------------------------------------------------------------------------
 * Function: ReadFromSPI()
 *
 * Low level abstract function to read from the SPI
 * Takes two separate byte buffers for write header and read data
 * returns the offset into read buffer where first byte of read data may be found,
 * or returns -1 if there was an error
 */
//#pragma GCC optimize ("O3")
int ReadFromSPI(uint16 headerLength, const uint8 *headerBuffer, uint32 readlength, uint8 *readBuffer)
{

	uint16_t i = 0;
	uint32_t timeout;

	decaIrqStatus_t  stat ;

	stat = DecaMutexON() ;

	/* Wait for SPIx Tx buffer empty */
	//while (port_SPIx_busy_sending());

	SPIx_CS_GPIO->BRR = D_CS_Pin;

//	for(i = 0; i < headerLength; i++)
//	{
//		
//		while(((SPI1->SR & SPI_FLAG_TXE) == RESET));
//		SPI1->DR = (uint8_t)headerBuffer[i];

//		while((SPI1->SR & SPI_FLAG_RXNE) == (uint16_t)RESET);

//		readBuffer[0] = SPI1->DR ; // Dummy read as we write the header
//	}

//	for(i=0; i<readlength; i++)
//	{
//		SPI1->DR = (uint8_t)0;  // Dummy write as we read the message body

//		while((SPI1->SR & SPI_FLAG_RXNE) == (uint16_t)RESET);

//		readBuffer[i] = SPI1->DR ;//port_SPIx_receive_data(); //this clears RXNE bit
//	}

	for(i = 0; i < headerLength; i++)
	{
		timeout = SPI_TIMEOUT;
		while(((SPI1->SR & SPI_FLAG_TXE) == RESET))
		{
			timeout--;
			if(timeout == 0)
			{
				goto SPI_Error;
			}
		}
		
		timeout = SPI_TIMEOUT;
		*(__IO uint8_t *)&SPI1->DR = (uint8_t)headerBuffer[i];

		while((SPI1->SR & SPI_FLAG_RXNE) == (uint16_t)RESET)
		{
			timeout--;
			if(timeout == 0)
			{
				goto SPI_Error;
			}
		}

		readBuffer[0] = SPI1->DR ; // Dummy read as we write the header
	}

	for(i=0; i<readlength; i++)
	{
		timeout = SPI_TIMEOUT;
		*(__IO uint8_t *)&SPI1->DR = (uint8_t)0;  // Dummy write as we read the message body

		while((SPI1->SR & SPI_FLAG_RXNE) == (uint16_t)RESET)
		{
			timeout--;
			if(timeout == 0)
			{
				goto SPI_Error;
			}
		}

		readBuffer[i] = SPI1->DR ;//port_SPIx_receive_data(); //this clears RXNE bit
	}

	SPIx_CS_GPIO->BSRR = D_CS_Pin;

	DecaMutexOFF(stat) ;
	
SPI_Error:
	if(timeout == 0)
	{
		for(i = 0; i < readlength; i++)
		{
			readBuffer[i] = 0;
		}
	}
	
	return 0;
} // end ReadFromSPI()
