#include "hsmSdk.h"
#include "mpuHal.h"

#define HSM_SDK_AID             (0x00)
#define HSM_SDK_MID             (0x00)

#define HSM_SDK_BUF_SIZE        (2048U)

static int16_t g_HsmHandle = -1; // MPU通信句柄
static uint8_t g_HsmSdkBuffer[HSM_SDK_BUF_SIZE];

int HsmSdk_Init(void)
{
    MpuHalFilter_t filter;
    g_HsmHandle = MpuHalOpen();

    filter.aid = HSM_SDK_AID;
    filter.midMin = HSM_SDK_MID;
    filter.midMax = HSM_SDK_MID;

    MpuHalSetRxFilter(g_HsmHandle, &filter);
    MpuHalSetRxBuffer(g_HsmHandle, g_HsmSdkBuffer, sizeof(g_HsmSdkBuffer));

    return 0;
}

int HsmSdk_DeInit(void)
{
    return 0;
}

