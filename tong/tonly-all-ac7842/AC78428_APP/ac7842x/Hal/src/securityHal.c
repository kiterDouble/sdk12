/*************************************************
 Copyright © 2025 SiRun (Beijing) . All rights reserved.
 File Name: securityHal.c
 Author: 
 Created Time: 
 Description: HSE Security Chip SPI2 Driver (AC7842x)
 Others: Migrated from Z20K148MS Spi_Drv/Gpio_Drv/Sysctrl_Drv/Int_Drv/Clk_Drv
         to AC7842x Spi_Hal/Gpio_Hal/Ckgen_Hal/CMSIS NVIC APIs
*************************************************/
/****************************** include ***************************************/
#include <stddef.h>
#include "securityHal.h"

#include "Gpio_Hal.h"
#include "Spi_Hal.h"
#include "Ckgen_Hal.h"
#include "Device_Types.h"

/****************************** Macro Definitions ******************************/
/* HSE RST pin (defined but not initialized in SPIx_Init, reserved for future use) */
#define HSE_REST_PORT         PORTID_C
#define HSE_REST_PIN          12
#define HSE_REST_PIN_MUX      PORT_MUX_AS_GPIO

/* SPI2 bus clock, peripheral clock and instance */
#define HSE_SPI_BUS_CLK       CKGEN_SPI2_BUS_CLK
#define HSE_SPI_PCLK          CKGEN_SPI2_CLK
#define HSE_SPI_INSTANCE      2U
#define HSE_SPI_IRQN          SPI2_IRQn

/* CS pin — manual GPIO control */
#define HSE_CS_PORT           PORTID_C
#define HSE_CS_PIN            14
#define HSE_CS_PIN_MUX        PORT_MUX_AS_GPIO

/* SPI2 pins: PTC15=SCK, PTC1=MOSI(SOUT), KL15 moved to PTC9 (MISO not used) */
#define SPI_CLK_PORT          PORTID_C
#define SPI_CLK_PORT_PIN      15
#define SPI_CLK_PORT_MUX      PORT_MUX_ALT2

#define SPI_MOSI_PORT         PORTID_C
#define SPI_MOSI_PORT_PIN     1
#define SPI_MOSI_PORT_MUX     PORT_MUX_ALT2

/* KL15 moved to PTC9, PTC0 (SPI2 MISO) not connected */

/* Timeout for polling transfers (system tick count) */
#define SPI_TIMEOUT           5000U

/****************************** Type Definitions ******************************/
/****************************** Global Variables ******************************/
/**
 * SPI2 Master mode configuration
 * - Mode: Master, CPOL=0, CPHA=0 (data captured on 1st edge)
 * - Frame: 8-bit, MSB first, 1-bit data width
 * - CS: manual GPIO control (non-automatic)
 * - BaudRate: 5 MHz (HSE security chip safe speed)
 */
const Spi_HalConfigType SPI_MasterConfig = {
    .CsSetup        = 0U,
    .CsHold         = 0U,
    .CsIdle         = 0U,
    .FrmSize        = 8U,
    .TxDmaChannel   = 0U,
    .RxDmaChannel   = 0U,
    .MsbFirst       = TRUE,
    .CsOutputEn     = FALSE,
    .ContinuousCs   = FALSE,
    .HreqEn         = FALSE,
    .Mode           = SPI_MASTER,
    .Cpol           = SPI_CPOL_LOW,
    .Cpha           = SPI_CPHA_0,
    .PcsCfg         = SPI_PCS_GPIO,
    .PcsPol         = SPI_PCS_POLARITY_LOW,
    .HreqPol        = SPI_HREQ_POLARITY_HIGH,
    .Width          = SPI_DATA_WIDTH_1BIT,
    .PinCfg         = SPI_SOUT_MOSI_SIN_MISO,
    .Callback       = NULL,
    .BaudRate       = 5000000U
};

/****************************** Function Declarations *************************/
/****************************** Public Function Implementations ***************/

/*************************************************
  Function:       SPIx_Init
  Description:    Initialize SPI2 module in Master mode with manual chip selection
  Input:          none
  Output:         none
  Return:         none
  Others:         Configures SPI SCK frequency via BaudRate in Spi_HalConfigType
                  Sets up GPIO pins for SPI communication and manual CS control
                  All SPI interrupts managed by HAL driver internally
*************************************************/
void SPIx_Init(void)
{
    /* ---- GPIO pinmux for SPI2 ---- */
    Gpio_Hal_SetMuxMode(SPI_CLK_PORT,  SPI_CLK_PORT_PIN,  SPI_CLK_PORT_MUX);   /* PTC15 -> SPI2_SCK  */
    /* KL15 moved to PTC9, PTC0 (SPI2 MISO) not configured */
    Gpio_Hal_SetMuxMode(SPI_MOSI_PORT, SPI_MOSI_PORT_PIN, SPI_MOSI_PORT_MUX);  /* PTC1  -> SPI2_SOUT */

    /* CS pin: GPIO output, idle HIGH (slave de-selected) */
    Gpio_Hal_SetMuxMode(HSE_CS_PORT, HSE_CS_PIN, HSE_CS_PIN_MUX);
    Gpio_Hal_SetPinDirection(HSE_CS_PORT, HSE_CS_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(HSE_CS_PORT, HSE_CS_PIN, 1);

    /* ---- SPI2 clock: disable, set source to HSE (40MHz), divider /1, re-enable ---- */
    Ckgen_Hal_EnablePeriphClk(HSE_SPI_BUS_CLK, FALSE);
    Ckgen_Hal_SetPeriphClkMux(HSE_SPI_PCLK, CKGEN_HSE_CLK);
    Ckgen_Hal_SetPeriphClkDiv(HSE_SPI_PCLK, 1U);
    Ckgen_Hal_EnablePeriphClk(HSE_SPI_BUS_CLK, TRUE);

    /* ---- SPI2 module init ---- */
    Spi_Hal_Init(HSE_SPI_INSTANCE, &SPI_MasterConfig);

    /* ---- Disable SPI2 interrupt (not used, HAL handles polling internally) ---- */
    NVIC_SetPriority(HSE_SPI_IRQN, 0x7U);
    NVIC_DisableIRQ(HSE_SPI_IRQN);
}


/*************************************************
  Function:       SPIx_SendData
  Description:    Send multiple 8-bit data via SPI2 interface (polling)
  Input:          txdataP - Pointer to data to be sent
                  dlc     - Number of data bytes to send
  Output:         none
  Return:         STATUS_SUCCESS - Send successful
                  STATUS_ERROR   - Send error
  Others:         Uses manual chip selection via GPIO
                  Uses Spi_Hal_TransceivePoll for blocking transfer
                  RX data discarded via local dummy buffer
*************************************************/
Hal_StatusType SPIx_SendData(unsigned char *txdataP, unsigned int dlc)
{
    Hal_StatusType result;
    unsigned char  dummyRx;

    /* Assert manual CS (LOW) */
    Gpio_Hal_WritePin(HSE_CS_PORT, HSE_CS_PIN, 0);

    /* Blocking SPI transfer: send txdata, receive into dummy buffer */
    result = Spi_Hal_TransceivePoll(HSE_SPI_INSTANCE,
                                    (const uint8 *)txdataP,
                                    &dummyRx,
                                    (uint16)dlc,
                                    SPI_TIMEOUT);

    /* Release CS (HIGH) */
    Gpio_Hal_WritePin(HSE_CS_PORT, HSE_CS_PIN, 1);

    return result;
}


/*************************************************
  Function:       SPIx_ReceiveData
  Description:    Receive multiple 8-bit data via SPI2 interface (polling)
  Input:          rxdataP - Pointer to store received data
                  dlc     - Number of data bytes to receive
  Output:         none
  Return:         STATUS_SUCCESS - Receive successful
                  STATUS_ERROR   - Receive error
  Others:         Sends dummy 0xFF bytes to provide clock for slave
                  Uses manual chip selection via GPIO
*************************************************/
Hal_StatusType SPIx_ReceiveData(unsigned char *rxdataP, unsigned int dlc)
{
    Hal_StatusType result;

    /* Assert manual CS (LOW) */
    Gpio_Hal_WritePin(HSE_CS_PORT, HSE_CS_PIN, 0);

    /* Blocking SPI transfer: send dummy 0xFF (automatic by HAL when TX=NULL), receive into rxdataP */
    result = Spi_Hal_TransceivePoll(HSE_SPI_INSTANCE,
                                    NULL,
                                    (uint8 *)rxdataP,
                                    (uint16)dlc,
                                    SPI_TIMEOUT);

    /* Release CS (HIGH) */
    Gpio_Hal_WritePin(HSE_CS_PORT, HSE_CS_PIN, 1);

    return result;
}


/*************************************************
  Function:       SPIx_ReadData
  Description:    Read multiple 8-bit data from slave device via SPI2 interface
                  Specifically designed for BYD slave display timing
  Input:          txdataP - Pointer to 5-byte command to be sent first
                  rxdataP - Pointer to store received data
                  dlc     - Number of data bytes to receive
  Output:         none
  Return:         STATUS_SUCCESS - Read successful
                  STATUS_ERROR   - Read error
  Others:         First sends 5 bytes of command/data to slave
                  Then sends dummy 0xAA bytes to provide clock for slave during read
                  CS held LOW during entire transaction
*************************************************/
Hal_StatusType SPIx_ReadData(unsigned char *txdataP, unsigned char *rxdataP, unsigned int dlc)
{
    Hal_StatusType result;

    /* Assert manual CS (LOW) — hold throughout the entire transaction */
    Gpio_Hal_WritePin(HSE_CS_PORT, HSE_CS_PIN, 0);

    /* Step 1: send 5-byte command to slave */
    result = Spi_Hal_TransceivePoll(HSE_SPI_INSTANCE,
                                    (const uint8 *)txdataP,
                                    NULL,
                                    5U,
                                    SPI_TIMEOUT);
    if (result != STATUS_SUCCESS)
    {
        Gpio_Hal_WritePin(HSE_CS_PORT, HSE_CS_PIN, 1);
        return result;
    }

    /* Step 2: send dummy 0xAA bytes to provide clock, receive data from slave */
    result = Spi_Hal_TransceivePoll(HSE_SPI_INSTANCE,
                                    NULL,
                                    (uint8 *)rxdataP,
                                    (uint16)dlc,
                                    SPI_TIMEOUT);

    /* Release CS (HIGH) */
    Gpio_Hal_WritePin(HSE_CS_PORT, HSE_CS_PIN, 1);

    return result;
}


/*************************************************
  Function:     SecurityHalInit
  Description:  Security module init
  Input:        None
  Output:       None
  Return:       0 on success
  Others:       None
*************************************************/
int16_t SecurityHalInit(void)
{
    SPIx_Init();
    return 0;
}


void SecurityHalSetMode(uint8_t wakeMode)
{
    if (0 == wakeMode)
    {
        /* Normal mode - reserved */
    }
    else if (1 == wakeMode)
    {
        /* Wake mode - reserved */
    }
}


void SecurityHalTestMain(void)
{
}
