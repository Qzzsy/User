/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND 
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, SEMTECH SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 * 
 * Copyright (C) SEMTECH S.A.
 */
/*! 
 * \file       sx1276-Hal.c
 * \brief      SX1276 Hardware Abstraction Layer
 *
 * \version    2.0.B2 
 * \date       Nov 21 2012
 * \author     Miguel Luis
 *
 * Last modified by Miguel Luis on Jun 19 2013
 */
//#include <stdint.h>
//#include <stdbool.h> 

#include "platform.h"

#if defined( USE_SX1276_RADIO )

#include "sx12xx_spi.h"
#include "sx1276-Hal.h"
#include "sx1276-lora.h"
#include "sx1276-LoRaMisc.h"
#include "radio.h"

#include "string.h"
#include "stdio.h"

#define RESET_IOPORT                                GPIOB
#define RESET_PIN                                   GPIO_Pin_0

/*!
 * SX1276 SPI NSS I/O definitions
 */
#define NSS_IOPORT                                  GPIOB
#define NSS_PIN                                     GPIO_Pin_1     //GPIO_Pin_1

/*!
 * SX1276 DIO pins  I/O definitions
 */
#define DIO0_IOPORT                                 GPIOC
#define DIO0_PIN                                    GPIO_Pin_0

#define DIO1_IOPORT                                 GPIOC
#define DIO1_PIN                                    GPIO_Pin_1

#define DIO2_IOPORT                                 GPIOC
#define DIO2_PIN                                    GPIO_Pin_2

#define DIO3_IOPORT                                 GPIOC
#define DIO3_PIN                                    GPIO_Pin_3

#define DIO4_IOPORT                                 GPIOC
#define DIO4_PIN                                    GPIO_Pin_4

#define DIO5_IOPORT                                 GPIOC
#define DIO5_PIN                                    GPIO_Pin_5

#define RXTX_IOPORT                                 
#define RXTX_PIN                                    FEM_CTX_PIN

#define RXE_PORT       			GPIOA
#define RXE_PIN  				GPIO_Pin_2
#define RXE_CLOCK  				RCC_APB2Periph_GPIOA
#define RXE_HIGH()         		GPIO_SetBits(RXE_PORT,RXE_PIN)
#define RXE_LOW()          		GPIO_ResetBits(RXE_PORT,RXE_PIN)
#define RXE_STATE()        		GPIO_ReadOutputDataBit(RXE_PORT,RXE_PIN)

#define TXE_PORT       			GPIOA
#define TXE_PIN  				GPIO_Pin_3
#define TXE_CLOCK  				RCC_APB2Periph_GPIOA
#define TXE_HIGH()         		GPIO_SetBits(TXE_PORT,TXE_PIN)
#define TXE_LOW()          		GPIO_ResetBits(TXE_PORT,TXE_PIN)
#define TXE_STATE()        		GPIO_ReadOutputDataBit(TXE_PORT,TXE_PIN)

void Set_RF_Switch_RX(void)
{
//	RXE_HIGH();
//	TXE_LOW();
}

void Set_RF_Switch_TX(void)
{
//	RXE_LOW();
//	TXE_HIGH();
}


void SX1276InitIo( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;


    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                            RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE );

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
     // Configure SPI-->NSS as output
    GPIO_InitStructure.GPIO_Pin = NSS_PIN;
    GPIO_Init( NSS_IOPORT, &GPIO_InitStructure );
	GPIO_WriteBit( NSS_IOPORT, NSS_PIN, Bit_SET );

	
	//  RXE-->CTRL   TXE--> /CTRL
	GPIO_InitStructure.GPIO_Pin = RXE_PIN;
	GPIO_Init(RXE_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = TXE_PIN;
	GPIO_Init(TXE_PORT, &GPIO_InitStructure);	
    
	Set_RF_Switch_RX();	
	
    // Configure radio DIO as inputs
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

    // Configure DIO0
    GPIO_InitStructure.GPIO_Pin =  DIO0_PIN;
    GPIO_Init( DIO0_IOPORT, &GPIO_InitStructure );
    
    // Configure DIO1
    GPIO_InitStructure.GPIO_Pin =  DIO1_PIN;
    GPIO_Init( DIO1_IOPORT, &GPIO_InitStructure );
    
    // Configure DIO2
    GPIO_InitStructure.GPIO_Pin =  DIO2_PIN;
    GPIO_Init( DIO2_IOPORT, &GPIO_InitStructure );
    
    // REAMARK: DIO3/4/5 configured are connected to IO expander

    // Configure DIO3 as input
    GPIO_InitStructure.GPIO_Pin =  DIO3_PIN;
    GPIO_Init( DIO3_IOPORT, &GPIO_InitStructure );
    // Configure DIO4 as input
    GPIO_InitStructure.GPIO_Pin =  DIO4_PIN;
    GPIO_Init( DIO4_IOPORT, &GPIO_InitStructure );
    // Configure DIO5 as input
	GPIO_InitStructure.GPIO_Pin =  DIO5_PIN;
    GPIO_Init( DIO5_IOPORT, &GPIO_InitStructure );
}

void SX1276SetReset( uint8_t state )
{
    GPIO_InitTypeDef GPIO_InitStructure;

    if( state == RADIO_RESET_ON )
    {
        // Configure RESET as output
		GPIO_InitStructure.GPIO_Pin = RESET_PIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
        GPIO_Init( RESET_IOPORT, &GPIO_InitStructure );
		
		// Set RESET pin to 0
        GPIO_WriteBit( RESET_IOPORT, RESET_PIN, Bit_RESET );
    }
    else
    {
		GPIO_InitStructure.GPIO_Pin =  RESET_PIN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init( RESET_IOPORT, &GPIO_InitStructure );
		
		// Set RESET pin to 1
        GPIO_WriteBit( RESET_IOPORT, RESET_PIN, Bit_SET );

    }
}

void SX1276Write( uint8_t addr, uint8_t data )
{
    SX1276WriteBuffer( addr, &data, 1 );
}

void SX1276Read( uint8_t addr, uint8_t *data )
{
    SX1276ReadBuffer( addr, data, 1 );
}

void SX1276WriteBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;

    //NSS = 0;
    GPIO_WriteBit( NSS_IOPORT, NSS_PIN, Bit_RESET );

    SpiInOut( addr | 0x80 );
    for( i = 0; i < size; i++ )
    {
        SpiInOut( buffer[i] );
    }

    //NSS = 1;
    GPIO_WriteBit( NSS_IOPORT, NSS_PIN, Bit_SET );
}

void SX1276ReadBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;

    //NSS = 0;
    GPIO_WriteBit( NSS_IOPORT, NSS_PIN, Bit_RESET );

    SpiInOut( addr & 0x7F );

    for( i = 0; i < size; i++ )
    {
        buffer[i] = SpiInOut( 0 );
    }

    //NSS = 1;
    GPIO_WriteBit( NSS_IOPORT, NSS_PIN, Bit_SET );
}

void SX1276WriteFifo( uint8_t *buffer, uint8_t size )
{
    SX1276WriteBuffer( 0, buffer, size );
}

void SX1276ReadFifo( uint8_t *buffer, uint8_t size )
{
    SX1276ReadBuffer( 0, buffer, size );
}

inline uint8_t SX1276ReadDio0( void )
{
    return GPIO_ReadInputDataBit( DIO0_IOPORT, DIO0_PIN );
}

inline uint8_t SX1276ReadDio1( void )
{
    return GPIO_ReadInputDataBit( DIO1_IOPORT, DIO1_PIN );
}

inline uint8_t SX1276ReadDio2( void )
{
    return GPIO_ReadInputDataBit( DIO2_IOPORT, DIO2_PIN );
}

inline uint8_t SX1276ReadDio3( void )
{
    return GPIO_ReadInputDataBit( DIO3_IOPORT, DIO3_PIN );
}

inline uint8_t SX1276ReadDio4( void )
{
    return GPIO_ReadInputDataBit( DIO4_IOPORT, DIO4_PIN );
}

inline uint8_t SX1276ReadDio5( void )
{
    return GPIO_ReadInputDataBit( DIO5_IOPORT, DIO5_PIN );
}


inline void SX1276WriteRxTx( uint8_t txEnable )
{
    if( txEnable != 0 )
    {
		Set_RF_Switch_TX(); 
    }
    else
    {
		Set_RF_Switch_RX();  
    }
}

void SetLora_CH(uint8_t ch)
{
    tLoRaSettings settings;
    switch(ch)
    {
    case 1:
        settings.RFFrequency = 470000000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;               //带宽          0~9
        break;

    case 2:
        settings.RFFrequency = 472300000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9; 
        break;

    case 3:
        settings.RFFrequency = 474600000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;               //带宽          0~9
        break;

    case 4:
        settings.RFFrequency = 476000000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;              //带宽          0~9
        break;

    case 5:
        settings.RFFrequency = 478500000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;              //带宽          0~9
        break;

    case 6:
        settings.RFFrequency = 480900000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;           //带宽          0~9
        break;

    case 7:
        settings.RFFrequency = 482100000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;             //带宽          0~9
        break;

    case 8:
        settings.RFFrequency = 484400000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;               //带宽          0~9
        break;

    case 9:
        settings.RFFrequency = 486800000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;               //带宽          0~9
        break;

    case 10:
        settings.RFFrequency = 488200000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;             //带宽          0~9
        break;

    case 11:
        settings.RFFrequency = 490500000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;           //带宽          0~9
        break;

    case 12:
        settings.RFFrequency = 492900000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;              //带宽          0~9
        break;

    case 13:
        settings.RFFrequency = 494200000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;              //带宽          0~9
        break;

    case 14:
        settings.RFFrequency = 468900000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;          //带宽          0~9
        break;

    case 15:
        settings.RFFrequency = 466500000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;          //带宽          0~9
        break;

    case 16:
        settings.RFFrequency = 464000000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;            //带宽          0~9
        break;

    case 17:
        settings.RFFrequency = 462600000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;               //带宽          0~9
        break;

    case 18:
        settings.RFFrequency = 460300000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;            //带宽          0~9
        break;

    case 19:
        settings.RFFrequency = 458100000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;              //带宽          0~9
        break;

    case 20:
        settings.RFFrequency = 456800000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;             //带宽          0~9
        break;

    case 21:
        settings.RFFrequency = 454500000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;              //带宽          0~9
        break;

    case 22:
        settings.RFFrequency = 452200000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;              //带宽          0~9
        break;

    case 23:
        settings.RFFrequency = 450000000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;               //带宽          0~9
        break;

    case 24:
        settings.RFFrequency = 448700000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;              //带宽          0~9
        break;

    case 25:
        settings.RFFrequency = 446400000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;              //带宽          0~9
        break;

    default:
        settings.RFFrequency = 470000000;    //频率
        settings.SpreadingFactor = 10;        //扩频因子      6~12
        settings.SignalBw = 9;               //带宽          0~9
        break;
    }
    SX1276LoRaSetRFFrequency(settings.RFFrequency);
    SX1276LoRaSetSpreadingFactor(settings.SpreadingFactor);
    SX1276LoRaSetSignalBandwidth(settings.SignalBw);
}

int SX1276_Spi_Check(void)
{
    uint8_t test = 0;
    
    tLoRaSettings settings;
    SX1276Read(REG_LR_VERSION,&test);
    printf("version code of the chip is %x\r\n", test);
    settings.RFFrequency = SX1276LoRaGetRFFrequency();
    printf("SX1278 Lora parameters are :\r\nRFFrequency is %d\r\n", settings.RFFrequency);
    settings.Power = SX1276LoRaGetRFPower();
    printf("RFPower is %d\r\n",settings.Power);
    settings.SignalBw = SX1276LoRaGetSignalBandwidth();	  
    printf("SignalBw is %d\r\n",settings.SignalBw);
    settings.SpreadingFactor = SX1276LoRaGetSpreadingFactor();
    printf("SpreadingFactor is %d\r\n",settings.SpreadingFactor);
    
    /*SPI 验证*/
    SX1276Write(REG_LR_HOPPERIOD, 0x91); //选一个用不到的寄存器来做验证
    SX1276Read(REG_LR_HOPPERIOD, &test);
    if (test != 0x91)
    {
        printf("check sx12xx fault!\r\n");
        return -1;
    }
    GPIOE->ODR |= (1 << 5);
    
    printf("check sx12xx OK!\r\n");
    return test;
}

tRadioDriver * SX12XX_init(void)
{
    tRadioDriver *Radio = NULL;
    
    sx12xxSpiInit();
    
    Radio = RadioDriverInit( );
    
    Radio->Init( );
    
    SX1276_Spi_Check();
    
    return Radio;
}

#endif // USE_SX1276_RADIO
