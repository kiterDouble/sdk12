#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <linux/spi/spidev.h>
#include <glib.h>
#include "err_code.h"
#include "sr_log.h"
#include "mcu_data.h"
#include "mcu_bus.h"
#include "mcu_send.h"
#include "mcu_recv.h"
#include "uart_api.h"
#include "spi_api.h"
#include "ota_api.h"

/**
 * @name McuSrvParserIni
 * @brief 解析MCU配置文件
 *        本函数用于从指定的配置文件中解析MCU的相关参数，并将这些参数存储到提供的配置结构体中。
 * @param cfg 指向McuSrvCfg_t结构体的指针，该结构体用于存储从配置文件中解析出的MCU参数。
 * @return 函数执行状态码。RET_OK表示成功，RET_ERR表示失败。
 */
static int32_t McuSrvParserIni(McuSrvCfg_t *cfg)
{
    int32_t ret = RET_ERR;
    GKeyFile *keyFile = NULL;
    GError *error = NULL;


    if (!cfg)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    keyFile = g_key_file_new();

    if (!keyFile)
    {
        LOG_Error("Error new key file");
        return ret;
    }

    if (!g_key_file_load_from_file(keyFile, "/oemdata/mcu.ini", G_KEY_FILE_NONE, &error))
    {
        LOG_Error("Error loading key file: %s", error->message);
        g_error_free(error);
        g_key_file_free(keyFile);
        return ret;
    }

    const char *section = "mcu";
    cfg->type = g_key_file_get_integer(keyFile, section, "type", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'type': %s", error->message);
        g_clear_error(&error);
        return ret;
    }

    LOG_Debug("Value of integer key 'type' in section 'mcu': %d",  cfg->type);
    cfg->speed = g_key_file_get_integer(keyFile, section, "speed", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'speed': %s", error->message);
        g_clear_error(&error);
        return ret;
    }

    LOG_Debug("Value of integer key 'speed' in section 'mcu': %d", cfg->speed);
    cfg->param1 = g_key_file_get_integer(keyFile, section, "param1", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'param1': %s", error->message);
        g_clear_error(&error);
        return ret;
    }

    LOG_Debug("Value of integer key 'param1' in section 'mcu': %d", cfg->param1);
    cfg->param2 = g_key_file_get_integer(keyFile, section, "param2", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'param2': %s", error->message);
        g_clear_error(&error);
        return ret;
    }

    LOG_Debug("Value of integer key 'param2' in section 'mcu': %d", cfg->param2);
    cfg->param3 = g_key_file_get_integer(keyFile, section, "param3", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'param3': %s", error->message);
        g_clear_error(&error);
        return ret;
    }

    LOG_Debug("Value of integer key 'param3' in section 'mcu': %d", cfg->param3);
    cfg->param4 = g_key_file_get_integer(keyFile, section, "param4", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'mcu': %s", error->message);
        g_clear_error(&error);
        return ret;
    }

    LOG_Debug("Value of integer key 'param4' in section 'mcu': %d", cfg->param4);
    char *dev = g_key_file_get_string(keyFile, section, "dev", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'param4': %s", error->message);
        g_clear_error(&error);
        return ret;
    }

    LOG_Debug("Value of integer key 'dev' in section 'mcu': %s", dev);
    strcpy((void*)cfg->device, dev);
    
    g_key_file_free(keyFile);
    ret = RET_OK;
    return ret;
}

/**
 * @name BusUartInit
 * @brief 初始化UART通信
 *        本函数用于初始化指定的UART设备，配置其通信参数，并准备就绪以便于发送或接收数据。
 * @param cfg 指向McuSrvCfg_t结构体的指针，包含了UART设备的配置信息，如设备名称、波特率等。
 * @return 返回初始化的结果状态码。成功返回RET_OK，失败返回RET_ERR。
 */
static int32_t BusUartInit(McuSrvCfg_t *cfg)
{
    int32_t ret = RET_ERR;

    if (!cfg)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    return UartInit((void*)cfg->device, cfg->speed, cfg->param1, cfg->param2, cfg->param3, cfg->param4);
}

/**
 * @name BusSpiInit
 * @brief 初始化SPI总线
 *        本函数用于根据给定的配置信息初始化SPI总线。配置信息包括设备节点、模式、参数等。
 * @param cfg 配置信息结构体指针，包含SPI设备的配置参数。
 * @return 返回初始化结果，成功返回RET_OK，失败返回RET_ERR。
 */
static int32_t BusSpiInit(McuSrvCfg_t *cfg)
{
    int32_t ret = RET_ERR;

    if (!cfg || (cfg->param1 > 3))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    uint32_t mode[] = {SPI_MODE_0, SPI_MODE_1, SPI_MODE_2, SPI_MODE_3};
    return SpiInit((void*)cfg->device, mode[cfg->param1], cfg->param2, cfg->speed, cfg->param3, cfg->param4);
}

/**
 * @name McuSrvInitData
 * @brief 初始化MCU服务数据结构体
 *        该函数用于初始化McuSrvData_t类型的数据结构体，包括清零、设置默认值以及根据配置选择合适的总线通信函数。
 * @param srvData 指向McuSrvData_t结构体的指针，该结构体将被初始化。
 * @return int32_t 返回初始化结果，RET_OK表示成功，RET_ERR表示失败。
 */
static int32_t McuSrvInitData(McuSrvData_t *srvData)
{
    int32_t ret = RET_ERR;

    if (!srvData)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    memset(srvData, 0x0, sizeof(McuSrvData_t));
    srvData->dataBusFd = -1;
    srvData->upgradeStatus = -1;

    if (RET_OK != McuSrvParserIni(&srvData->cfg))
    {
        LOG_Error("%s McuSrvParserIni fail.", __FUNCTION__);
        return ret;
    }

    LOG_Debug("srvData->cfg.type: %d", srvData->cfg.type);
    if (0 == srvData->cfg.type)
    {
        srvData->BusOpen = BusUartInit;
        srvData->busRead = UartRecvData;
        srvData->BusWrite = UartSendData;
        srvData->BusClose = UartClose;
    }
    else
    {
        srvData->BusOpen = BusSpiInit;
        srvData->busRead = SpiRead;
        srvData->BusWrite = SpiWrite;
        srvData->BusClose = SpiClose;
    }
    ret = RET_OK;
    return ret;
}

/**
 * @name McuSrvNotifyUpgradeCallback
 * @brief MCU服务通知升级回调函数
 *        此函数被用于处理MCU服务端发送的升级通知。它会解析传入的数据，以确定升级的状态。
 * @param data 指向接收到的数据的指针。
 * @param len 接收到的数据的长度。
 * @param userdata 用户自定义数据，通常用于传递额外的上下文信息。
 * @return 该函数无返回值。
 */
static void McuSrvNotifyUpgradeCallback(const uint8_t *data, const uint16_t len, void* userdata)
{
    McuSrvData_t *srvData = (McuSrvData_t *)userdata;

    if (!srvData)
    {
        return;
    }
    UpgradeStatus_E tmpUpgradeStatus = UPGRADE_INIT;
    tmpUpgradeStatus = (UpgradeStatus_E)data[0];
    //LOG_Debug("upgradeStatus: %d", tmpUpgradeStatus);
    if (tmpUpgradeStatus == UPGRADE_MCU_FLASHING)
    {
        srvData->upgradeStatus = 0;
        LOG_Debug("upgradeStatus: UPGRADE_MCU_FLASHING");
    }
    else
    {
        srvData->upgradeStatus = -1;
        LOG_Debug("upgradeStatus: UPGRADE_MCU_FLASHING stop");
    }
}

int main(int argc, char *argv[])
{
    McuSrvData_t srvData;

    LOG_INIT("mcuServer");

    McuSrvInitData(&srvData);
    StartQueueMsgRcvThread("mcuServer_ipc");
    SubscriptionQueueMsg(SYS_QUE_TYPE_SGMW_UPGRADE_STATUS_SYNC, McuSrvNotifyUpgradeCallback, (void*)&srvData);//todo
    RpcStartMsgServer(RPCSERVER_TYPE_MCUSERVER, McuSrvSendMesgCallback, 1, (void*)&srvData);

    while (1)
    {

        if (srvData.dataBusFd < 0)
        {
            McuSrvBusInit(&srvData);
            usleep(500000);
        }
        else
        {
            McuSrvRecvMesgProcess(&srvData);
        }
    }
}
