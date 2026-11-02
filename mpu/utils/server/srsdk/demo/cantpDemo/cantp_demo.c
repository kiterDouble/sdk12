#include "can_tp.h"
#include "err_code.h"
#include "sr_log.h"


static void TpTransmitError(uint32_t result)
{
    LOG_Debug("TpTransmitError: %d", result);
}
static void UdsRecieve(uint32_t canId, uint8_t *buffer, uint32_t bufLen)
{
    LOG_Debug("UdsRecieve: canId: %d, bufLen: %d", canId, bufLen);
    LOG_DebugHex(buffer, bufLen, "UdsRecieve");
}

int main(int argc, char *argv[])
{
    CanTpHandle_t tp;
    int32_t ret = RET_ERR;
    LOG_INIT("demo");
    ret = CanTpInit(&tp, UdsRecieve, TpTransmitError);
    if (RET_ERR == ret)
    {
        LOG_Error("CanTpInit failed.");
        return -1;
    }

    ret = CanTpLock(tp);
    if (RET_ERR == ret)
    {
        LOG_Error("CanTpLock failed.");
        CanTpDeinit(tp);
        return -1;        
    }
    int32_t count = 0;
    while(count++ < 10)
    {
        uint8_t data[] = {0x19, 0x02, 0xff};
        ret = CanTpTransmit(tp, 0x762, data, sizeof(data));
        LOG_Debug("CanTpTransmit ret:%d", ret);
        sleep(1);
    }

    ret = CanTpUnlock(tp);
    CanTpDeinit(tp);
    return 0;
}