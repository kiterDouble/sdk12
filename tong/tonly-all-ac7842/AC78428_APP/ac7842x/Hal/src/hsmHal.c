/*************************************************
 * Copyright (C) 2024, AutoChips Technology
 * All Rights Reserved
 *
 * File:     hsmHal.c
 * Brief:    HSM (Hardware Security Module) SPI1 driver
 *           Ported from Z20K148MS to AC7842x HAL APIs
 *************************************************/

#include <stddef.h>
#include "hsmHal.h"
#include "logHal.h"

#include "Gpio_Hal.h"
#include "Spi_Hal.h"
#include "Ckgen_Hal.h"
#include "AC7842x.h"

/* SPI Configuration */
#define HSM_SPI_INSTANCE                1U
#define HSM_SPI_DATA_SIZE               8U
#define HSM_SPI_CLK_DIVIDER             3U
#define HSM_SPI_BAUDRATE                5000000U    /* 5MHz, adjustable per HSM chip spec */
#define HSM_SPI_DMA_BUFFSIZE            2048U

/************************** SPI GPIO Configuration ***************************/
/* SPI1 CS Pin: PTB17 */
#define HSM_SPI_CS_PORT                 PORTID_B
#define HSM_SPI_CS_PIN                  17U
#define HSM_SPI_CS_PIN_MUX              PORT_MUX_ALT2

/* SPI1 CLK Pin: PTB14 */
#define HSM_SPI_CLK_PORT                PORTID_B
#define HSM_SPI_CLK_PIN                 14U
#define HSM_SPI_CLK_PIN_MUX             PORT_MUX_ALT2

/* SPI1 MOSI Pin: PTB16 */
#define HSM_SPI_MOSI_PORT               PORTID_B
#define HSM_SPI_MOSI_PIN                16U
#define HSM_SPI_MOSI_PIN_MUX            PORT_MUX_ALT2

/* SPI1 MISO Pin: PTB15 */
#define HSM_SPI_MISO_PORT               PORTID_B
#define HSM_SPI_MISO_PIN                15U
#define HSM_SPI_MISO_PIN_MUX            PORT_MUX_ALT2

/* SPI1 RST Pin: PTB3(Output) */
#define HSM_SPI_RST_PORT                PORTID_B
#define HSM_SPI_RST_PIN                 3U
#define HSM_SPI_RST_PIN_MUX             PORT_MUX_AS_GPIO

/* SPI IRQ Pin: PTA11(Input) */
#define HSM_SPI_IRQ_PORT                PORTID_A
#define HSM_SPI_IRQ_PIN                 11U
#define HSM_SPI_IRQ_PIN_MUX             PORT_MUX_AS_GPIO

/* DMA Channel Definitions (reserved for future DMA support) */
#define SPI_DMA_TX_CHANNEL              14U
#define SPI_DMA_RX_CHANNEL              15U
#define SPI_DMA_TX_REQ                  DMA_REQ_SPI1_TX     /* 48U */
#define SPI_DMA_RX_REQ                  DMA_REQ_SPI1_RX     /* 47U */

/* SPI Master Configuration */
static const Spi_HalConfigType g_hsmSpiConfig = {
    .Mode       = SPI_MASTER,
    .Cpol       = SPI_CPOL_LOW,
    .Cpha       = SPI_CPHA_0,
    .Width      = SPI_DATA_WIDTH_1BIT,
    .MsbFirst   = TRUE,
    .FrmSize    = HSM_SPI_DATA_SIZE,
    .PcsCfg     = SPI_PCS_0,
    .CsOutputEn = TRUE,
    .ContinuousCs = FALSE,
    .CsSetup    = 0U,
    .CsHold     = 0U,
    .CsIdle     = 0U,
    .HreqEn     = FALSE,
    .PcsPol     = 0U,
    .HreqPol    = 0U,
    .PinCfg     = SPI_SOUT_MOSI_SIN_MISO,
    .TxDmaChannel = 0U,     /* DMA not used (callbacks #if 0'd) */
    .RxDmaChannel = 0U,     /* DMA not used */
    .BaudRate   = HSM_SPI_BAUDRATE,
    .Callback   = NULL
};


static void HsmGpio_Init(void)
{
    Port_Hal_DigitalFilterCfgType filterCfg;

    /* Configure SPI SCK pin */
    Gpio_Hal_SetMuxMode(HSM_SPI_CLK_PORT, HSM_SPI_CLK_PIN, HSM_SPI_CLK_PIN_MUX);
    /* Configure SPI MISO pin */
    Gpio_Hal_SetMuxMode(HSM_SPI_MISO_PORT, HSM_SPI_MISO_PIN, HSM_SPI_MISO_PIN_MUX);
    /* Configure SPI MOSI pin */
    Gpio_Hal_SetMuxMode(HSM_SPI_MOSI_PORT, HSM_SPI_MOSI_PIN, HSM_SPI_MOSI_PIN_MUX);
    /* Configure SPI CS pin */
    Gpio_Hal_SetMuxMode(HSM_SPI_CS_PORT, HSM_SPI_CS_PIN, HSM_SPI_CS_PIN_MUX);
    /* Configure SPI RST pin */
    Gpio_Hal_SetMuxMode(HSM_SPI_RST_PORT, HSM_SPI_RST_PIN, HSM_SPI_RST_PIN_MUX);
    /* Configure SPI IRQ pin */
    Gpio_Hal_SetMuxMode(HSM_SPI_IRQ_PORT, HSM_SPI_IRQ_PIN, HSM_SPI_IRQ_PIN_MUX);

    /* SPI IRQ Input as interrupt (pull-down, digital filter, both-edge trigger) */
    Gpio_Hal_SetPullSel(HSM_SPI_IRQ_PORT, HSM_SPI_IRQ_PIN, PORT_INTERNAL_PULL_DOWN_ENABLED);

    filterCfg.Clock   = PORT_DIGITAL_FILTER_BUS_CLOCK;
    filterCfg.Width   = 0x10U;
    filterCfg.PinMask = (1UL << HSM_SPI_IRQ_PIN);
    Gpio_Hal_ConfigDigitalFilter(HSM_SPI_IRQ_PORT, &filterCfg);
    Gpio_Hal_EnableDigitalFilter(HSM_SPI_IRQ_PORT, HSM_SPI_IRQ_PIN, TRUE);

    Gpio_Hal_SetPinDirection(HSM_SPI_IRQ_PORT, HSM_SPI_IRQ_PIN, GPIO_INPUT_DIRECTION);
    Gpio_Hal_SetPinIntSel(HSM_SPI_IRQ_PORT, HSM_SPI_IRQ_PIN, PORT_INT_EITHER_EDGE);
    NVIC_EnableIRQ(PORTA_IRQn);

    /* SPI RST Output */
    Gpio_Hal_SetPinDirection(HSM_SPI_RST_PORT, HSM_SPI_RST_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(HSM_SPI_RST_PORT, HSM_SPI_RST_PIN, GPIO_HIGH_LEVEL);
}


void SPI1_Init(void)
{
    /* Disable SPI1 bus clock before configuration */
    Ckgen_Hal_EnablePeriphClk(CKGEN_SPI1_BUS_CLK, FALSE);

    /* Set SPI1 clock source to SPLL */
    Ckgen_Hal_SetPeriphClkMux(CKGEN_SPI1_CLK, CKGEN_SPLL_CLK);

    /* Set SPI1 clock divider */
    Ckgen_Hal_SetPeriphClkDiv(CKGEN_SPI1_CLK, HSM_SPI_CLK_DIVIDER);

    /* Enable SPI1 bus clock after configuration */
    Ckgen_Hal_EnablePeriphClk(CKGEN_SPI1_BUS_CLK, TRUE);

    HsmGpio_Init();
    Spi_Hal_Init(HSM_SPI_INSTANCE, &g_hsmSpiConfig);

    /* DMA callback installation (disabled, reserved for future use) */
#if 0
    Dma_InstallCallBackFunc(SPI_DMA_RX_CHANNEL, DMA_INT_DONE, MpuHal_SpiDmaRxCallback);
    Dma_IntMask(SPI_DMA_RX_CHANNEL, DMA_INT_DONE, COMMON_UNMASK);
    Int_SetPriority(SPI_DMA_RX_IRQ, SPI_DMA_RX_IRQ_PRIORITY);
    Int_EnableIrq(SPI_DMA_RX_IRQ);

    Dma_InstallCallBackFunc(SPI_DMA_TX_CHANNEL, DMA_INT_DONE, MpuHal_SpiDmaTxCallback);
    Dma_IntMask(SPI_DMA_TX_CHANNEL, DMA_INT_DONE, COMMON_UNMASK);
    Int_SetPriority(SPI_DMA_TX_IRQ, SPI_DMA_TX_IRQ_PRIORITY);
    Int_EnableIrq(SPI_DMA_TX_IRQ);

    /* Initialize buffer states */
    g_spiDmaTxBuf.dmaBufStatus = DMA_BUF_STATE_IDLE;
    g_spiDmaRxBuf.dmaBufStatus = DMA_BUF_STATE_IDLE;
#endif
}


void HsmHal_Init(void)
{
    SPI1_Init();
}
