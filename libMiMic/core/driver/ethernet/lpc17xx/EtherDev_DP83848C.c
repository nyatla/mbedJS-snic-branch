/*
    FreeRTOS V7.0.0 - Copyright (C) 2011 Real Time Engineers Ltd.


    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/

/* Originally adapted from file written by Andreas Dannenberg.  Supplied with permission. */
/*
 * Modified for MiMic by R.Iizuka. 2011.08.27
 * http://nyatla.jp/mimic
 */

#include "NyLPC_config.h"
#if NyLPC_MCU==NyLPC_MCU_LPC17xx


/* Kernel includes. */
#include "../NyLPC_cEthernetMM.h"
#include "EtherDev_DP83848C_protected.h"
#include "LPC17xx.h"
#include "NyLPC_os.h"


#define DP83848C_ID         0x20005C90  /* PHY Identifier                    */

/* DP83848C PHY Registers */
#define PHY_REG_BMCR        0x00        /* Basic Mode Control Register       */
#define PHY_REG_BMSR        0x01        /* Basic Mode Status Register        */
#define PHY_REG_IDR1        0x02        /* PHY Identifier 1                  */
#define PHY_REG_IDR2        0x03        /* PHY Identifier 2                  */
#define PHY_REG_ANAR        0x04        /* Auto-Negotiation Advertisement    */
#define PHY_REG_ANLPAR      0x05        /* Auto-Neg. Link Partner Abitily    */
#define PHY_REG_ANER        0x06        /* Auto-Neg. Expansion Register      */
#define PHY_REG_ANNPTR      0x07        /* Auto-Neg. Next Page TX            */

/* PHY Extended Registers */
#define PHY_REG_STS         0x10        /* Status Register                   */
#define PHY_REG_MICR        0x11        /* MII Interrupt Control Register    */
#define PHY_REG_MISR        0x12        /* MII Interrupt Status Register     */
#define PHY_REG_FCSCR       0x14        /* False Carrier Sense Counter       */
#define PHY_REG_RECR        0x15        /* Receive Error Counter             */
#define PHY_REG_PCSR        0x16        /* PCS Sublayer Config. and Status   */
#define PHY_REG_RBR         0x17        /* RMII and Bypass Register          */
#define PHY_REG_LEDCR       0x18        /* LED Direct Control Register       */
#define PHY_REG_PHYCR       0x19        /* PHY Control Register              */
#define PHY_REG_10BTSCR     0x1A        /* 10Base-T Status/Control Register  */
#define PHY_REG_CDCTRL1     0x1B        /* CD Test Control and BIST Extens.  */
#define PHY_REG_EDCR        0x1D        /* Energy Detect Control Register    */

#define PHY_FULLD_100M      0x2100      /* Full Duplex 100Mbit               */
#define PHY_HALFD_100M      0x2000      /* Half Duplex 100Mbit               */
#define PHY_FULLD_10M       0x0100      /* Full Duplex 10Mbit                */
#define PHY_HALFD_10M       0x0000      /* Half Duplex 10MBit                */
#define PHY_AUTO_NEG        0x3000      /* Select Auto Negotiation           */
#define PHY_AUTO_NEG_COMPLETE 0x0020    /* Auto negotiation have finished.   */
//#define ETHDEV_PHY_DEF_ADR    0x0100      /* Default PHY device address        */


#ifndef configEMAC_INTERRUPT_PRIORITY
    #define configEMAC_INTERRUPT_PRIORITY       5
#endif

/* Time to wait between each inspection of the link status. */
#define emacWAIT_FOR_LINK_TO_ESTABLISH_MS 500

/* Short delay used in several places during the initialisation process. */
#define emacSHORT_DELAY_MS                 10

/* Hardware specific bit definitions. */
#define emacPINSEL2_VALUE           ( 0x50150105 )



/*-----------------------------------------------------------*/

/* Setup the IO and peripherals required for Ethernet communication.*/
static void prvSetupEMACHardware( void );
/* Control the auto negotiate process.*/
static void prvConfigurePHY( void );
/*
 * Wait for a link to be established, then setup the PHY according to the link
 * parameters.
 */
static NyLPC_TBool prvSetupLinkStatus( void );





static NyLPC_TBool start(const struct NyLPC_TEthAddr* i_eth_addr,NyLPC_TiEthernetDevice_onEvent i_handler,void* i_param);
static void stop(void);
static void* allocTxBuf(NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_size);
static void releaseTxBuf(void* i_buf);
/*-----------------------------------------------------------*/
#define ETH_TX_BUF_BASE (void*)(ETH_BUF_BASE+ETH_FRAG_SIZE*NUM_RX_FRAG)


const static struct TiEthernetDevice _interface=
{
    "DP83848C",
    start,
    stop,
    EthDev_LPC17xx_getRxEthFrame,
    EthDev_LPC17xx_nextRxEthFrame,
    allocTxBuf,
    releaseTxBuf,
    EthDev_LPC17xx_sendTxEthFrame,
    EthDev_LPC17xx_processTx
};

static void* _event_param;
static NyLPC_TiEthernetDevice_onEvent _event_handler;

/** EMAC ISRハンドラ*/
static void emacIsrHandler(unsigned long i_status);

/*
 * EthernetDeviceのファクトリー関数
 */

NyLPC_TBool EthDev_DP83848C_getInterface(
    const struct TiEthernetDevice** o_dev)
{
    unsigned long ulID1, ulID2;
    NyLPC_TBool lReturn = NyLPC_TBool_TRUE;
    //Reset MCU Interface. and wait for reset.
    prvSetupEMACHardware();
    //Check peripheral name
    ulID1 = EthDev_LPC17xx_prvReadPHY( PHY_REG_IDR1, &lReturn );
    ulID2 = EthDev_LPC17xx_prvReadPHY( PHY_REG_IDR2, &lReturn );
    if( ( (ulID1 << 16UL ) | ( ulID2 & 0xFFF0UL ) ) != DP83848C_ID)
    {
        return NyLPC_TBool_FALSE;
    }
    *o_dev=&_interface;
	LPC_EMAC->TxProduceIndex = 0;
	LPC_EMAC->RxConsumeIndex = 0;
    return NyLPC_TBool_TRUE;
}


static NyLPC_TBool start(const struct NyLPC_TEthAddr* i_eth_addr,NyLPC_TiEthernetDevice_onEvent i_handler,void* i_param)
{
	_event_handler=i_handler;
	_event_param=i_param;
	NyLPC_cIsr_setEnetISR(emacIsrHandler);
	/* Set the Ethernet MAC Address registers */
    LPC_EMAC->SA0 = (((uint32_t)(i_eth_addr->addr[0])) << 8 ) | i_eth_addr->addr[1];
    LPC_EMAC->SA1 = (((uint32_t)(i_eth_addr->addr[2])) << 8 ) | i_eth_addr->addr[3];
    LPC_EMAC->SA2 = (((uint32_t)(i_eth_addr->addr[4])) << 8 ) | i_eth_addr->addr[5];

	//TXメモリマネージャの準備
	NyLPC_cEthernetMM_initialize(ETH_TX_BUF_BASE);

	/* Initialize Tx and Rx DMA Descriptors */
    EthDev_LPC17xx_prevRxDescriptor();
    EthDev_LPC17xx_prevTxDescriptor();

    /* Setup the PHY. */
    prvConfigurePHY();

    //wait for Link up...
    while(!prvSetupLinkStatus())
    {
        NyLPC_cThread_sleep(100);
    }

    /* Receive Broadcast and Perfect Match Packets */
    LPC_EMAC->RxFilterCtrl = RFC_BCAST_EN | RFC_PERFECT_EN | RFC_MCAST_EN;

    //Ethernetの割込み開始設定
    NyLPC_cIsr_enterCritical();
    {
        /* Reset all interrupts */
        LPC_EMAC->IntClear = 0xffff;
        LPC_EMAC->IntEnable = ( INT_RX_DONE | INT_TX_DONE );

        /* Enable receive and transmit mode of MAC Ethernet core */
        LPC_EMAC->Command |= ( CR_RX_EN | CR_TX_EN );
        LPC_EMAC->MAC1 |= MAC1_REC_EN;

        /* Set the interrupt priority to the max permissible to cause some
        interrupt nesting. */
        NVIC_SetPriority( ENET_IRQn, configEMAC_INTERRUPT_PRIORITY );

        /* Enable the interrupt. */
        NVIC_EnableIRQ( ENET_IRQn );
    }
    NyLPC_cIsr_exitCritical();

    return NyLPC_TBool_TRUE;

}
static void stop(void)
{
    NyLPC_cIsr_enterCritical();
    {
        LPC_EMAC->IntEnable = (~(INT_RX_DONE|INT_TX_DONE))&LPC_EMAC->IntEnable;
        NVIC_DisableIRQ( ENET_IRQn );
    }
    NyLPC_cIsr_exitCritical();
	LPC_EMAC->Command &= ~( CR_RX_EN | CR_TX_EN );
	LPC_EMAC->MAC1 &= ~MAC1_REC_EN;
	//ISR割り込み解除
	NyLPC_cIsr_setEnetISR(NULL);
	//TXメモリマネージャの終了
	NyLPC_cEthernetMM_finalize();
    return;
}

static void* allocTxBuf(NyLPC_TUInt16 i_hint,NyLPC_TUInt16* o_size)
{
	return NyLPC_cEthernetMM_alloc(i_hint,o_size);
}
static void releaseTxBuf(void* i_buf)
{
	NyLPC_cEthernetMM_release(i_buf);
}



/********************************************************************************
 * Private functions
 *******************************************************************************/

/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

static void prvSetupEMACHardware( void )
{
    unsigned short us;
    long x;
    NyLPC_TBool lDummy;

    /* Power Up the EMAC controller. */
    LPC_SC->PCONP |= 0x40000000;
    NyLPC_cThread_sleep( emacSHORT_DELAY_MS);

    /* Enable P1 Ethernet Pins. */
    LPC_PINCON->PINSEL2 = emacPINSEL2_VALUE;
    LPC_PINCON->PINSEL3 = ( LPC_PINCON->PINSEL3 & ~0x0000000F ) | 0x00000005;

    /* Reset all EMAC internal modules. */
    LPC_EMAC->MAC1 = MAC1_RES_TX | MAC1_RES_MCS_TX | MAC1_RES_RX | MAC1_RES_MCS_RX | MAC1_SIM_RES | MAC1_SOFT_RES;
    LPC_EMAC->Command = CR_REG_RES | CR_TX_RES | CR_RX_RES| CR_PASS_RUNT_FRM;
    /* A short delay after reset. */
    NyLPC_cThread_sleep( emacSHORT_DELAY_MS );

    /* Initialize MAC control registers. */
    LPC_EMAC->MAC1 = MAC1_PASS_ALL;
    LPC_EMAC->MAC2 = MAC2_CRC_EN | MAC2_PAD_EN;
    LPC_EMAC->MAXF = ETH_MAX_FLEN;
    LPC_EMAC->CLRT = CLRT_DEF;
    LPC_EMAC->IPGR = IPGR_DEF;

    /*PCLK=18MHz, clock select=6, MDC=18/6=3MHz */ // I don't think so!
    /* Enable Reduced MII interface. */
    LPC_EMAC->MCFG = MCFG_CLK_DIV20 | MCFG_RES_MII;
    NyLPC_cThread_sleep( emacSHORT_DELAY_MS );
    LPC_EMAC->MCFG = MCFG_CLK_DIV20;

    /* Enable Reduced MII interface. */
    LPC_EMAC->Command = CR_RMII | CR_PASS_RUNT_FRM | CR_PASS_RX_FILT;

    /* Reset Reduced MII Logic. */
    LPC_EMAC->SUPP = SUPP_RES_RMII | SUPP_SPEED;
    NyLPC_cThread_sleep( emacSHORT_DELAY_MS );
    LPC_EMAC->SUPP = SUPP_SPEED;

    /* Put the PHY in reset mode */
    EthDev_LPC17xx_prvWritePHY( PHY_REG_BMCR, MCFG_RES_MII );
    NyLPC_cThread_sleep( emacSHORT_DELAY_MS * 5);

    /* Wait for hardware reset to end. */
    for( x = 0; x < 100; x++ )
    {
        NyLPC_cThread_sleep( emacSHORT_DELAY_MS * 5 );
        us = EthDev_LPC17xx_prvReadPHY( PHY_REG_BMCR, &lDummy );
        if( !( us & MCFG_RES_MII ) )
        {
            /* Reset complete */
            break;
        }
    }
}
/*------------------------------------------------
 * Private function depend on device.
 * デバイス依存部分
 ------------------------------------------------*/


/*for mbed
 */
#define emacLINK_ESTABLISHED        ( 0x0001 )
#define emacFULL_DUPLEX_ENABLED     ( 0x0004 )
#define emac10BASE_T_MODE           ( 0x0002 )


static void prvConfigurePHY( void )
{
    unsigned short us;
    long x;
    NyLPC_TBool lDummy;

    /* Auto negotiate the configuration. */
    if( EthDev_LPC17xx_prvWritePHY( PHY_REG_BMCR, PHY_AUTO_NEG ) )
    {
        NyLPC_cThread_sleep( emacSHORT_DELAY_MS * 5 );

        for( x = 0; x < 10; x++ )
        {
            us = EthDev_LPC17xx_prvReadPHY( PHY_REG_BMSR, &lDummy );

            if( us & PHY_AUTO_NEG_COMPLETE )
            {
                break;
            }

            NyLPC_cThread_sleep( emacWAIT_FOR_LINK_TO_ESTABLISH_MS);
        }
    }
}

static NyLPC_TBool prvSetupLinkStatus( void )
{
    NyLPC_TBool lReturn = NyLPC_TBool_FALSE;
    long x;
    unsigned short usLinkStatus;

    /* Wait with timeout for the link to be established. */
    for( x = 0; x < 10; x++ )
    {
        usLinkStatus = EthDev_LPC17xx_prvReadPHY( PHY_REG_STS, &lReturn );
        if( usLinkStatus & emacLINK_ESTABLISHED )
        {
            /* Link is established. */
            lReturn = NyLPC_TBool_TRUE;
            break;
        }

        NyLPC_cThread_sleep( emacWAIT_FOR_LINK_TO_ESTABLISH_MS);
    }

    if( lReturn == NyLPC_TBool_TRUE )
    {
        /* Configure Full/Half Duplex mode. */
        if( usLinkStatus & emacFULL_DUPLEX_ENABLED )
        {
            /* Full duplex is enabled. */
            LPC_EMAC->MAC2 |= MAC2_FULL_DUP;
            LPC_EMAC->Command |= CR_FULL_DUP;
            LPC_EMAC->IPGT = IPGT_FULL_DUP;
        }
        else
        {
            /* Half duplex mode. */
            LPC_EMAC->IPGT = IPGT_HALF_DUP;
        }

        /* Configure 100MBit/10MBit mode. */
        if( usLinkStatus & emac10BASE_T_MODE )
        {
            /* 10MBit mode. */
            LPC_EMAC->SUPP = 0;
        }
        else
        {
            /* 100MBit mode. */
            LPC_EMAC->SUPP = SUPP_SPEED;
        }
    }

    return lReturn;
}

/**
 * EMACからのハンドラ
 */
static void emacIsrHandler(unsigned long i_status)
{
	if( i_status & INT_RX_DONE )
	{
		_event_handler(_event_param,NyLPC_TiEthernetDevice_EVENT_ON_RX);
	}
	if( i_status & INT_TX_DONE )
	{
		_event_handler(_event_param,NyLPC_TiEthernetDevice_EVENT_ON_TX);
	}
}

#endif
