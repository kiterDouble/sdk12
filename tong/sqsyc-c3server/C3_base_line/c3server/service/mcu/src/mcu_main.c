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
#include "mcu_can_filter.h"
#include "uart_api.h"
#include "spi_api.h"

#define MCU_INI_FILE_PATH "/oemapp/config/mcu.ini"
#define MCU_INI_FILE_PATH_PRIMARY "/oemdata/config/mcu.ini"

static McuSrv_t g_srvData = {0};
/**
 * @name McuSrvParserIni
 * @brief 解析MCU配置文件
 *        本函数用于从指定的配置文件中解析MCU的相关参数，并将这些参数存储到提供的配置结构体中。
 * @param cfg 指向McuSrvCfg_t结构体的指针，该结构体用于存储从配置文件中解析出的MCU参数。
 * @return 函数执行状态码。RET_OK表示成功，RET_ERR表示失败。
 */
static int32_t McuSrvParserIni(McuSrvData_t *srv, const char * sectionName)
{
    int32_t ret = RET_ERR;
    GKeyFile *keyFile = NULL;
    GError *error = NULL;


    if (!srv)
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

    const char *mcuIniPath = MCU_INI_FILE_PATH;
    if (0 == access(MCU_INI_FILE_PATH_PRIMARY, F_OK)) {
        mcuIniPath = MCU_INI_FILE_PATH_PRIMARY;
        LOG_Debug("Using primary config: %s", mcuIniPath);
    }

    if (0 == g_key_file_load_from_file(keyFile, mcuIniPath, G_KEY_FILE_NONE, &error))
    {
        LOG_Error("Error loading key file: %s", error->message);
        g_error_free(error);
        g_key_file_free(keyFile);
        return ret;
    }

    {
        const char *section = sectionName;
        srv->cfg.speed = (uint32_t)g_key_file_get_integer(keyFile, section, "speed", &error);
        if (error != NULL)
        {
            LOG_Error("Failed to get integer value for key 'speed': %s", error->message);
            g_clear_error(&error);
            return ret;
        }
        LOG_Debug("Value of integer key 'speed' in section%s: %d", sectionName, (int)srv->cfg.speed);

        srv->cfg.param1 = (uint32_t)g_key_file_get_integer(keyFile, section, "param1", &error);
        if (error != NULL)
        {
            LOG_Error("Failed to get integer value for key 'param1': %s", error->message);
            g_clear_error(&error);
            return ret;
        }
        LOG_Debug("Value of integer key 'param1' in section%s: %d", sectionName, (int)srv->cfg.param1);

        srv->cfg.param2 = (uint32_t)g_key_file_get_integer(keyFile, section, "param2", &error);
        if (error != NULL)
        {
            LOG_Error("Failed to get integer value for key 'param2': %s", error->message);
            g_clear_error(&error);
            return ret;
        }
        LOG_Debug("Value of integer key 'param2' in section%s: %d", sectionName, (int)srv->cfg.param2);

        srv->cfg.param3 = (uint32_t)g_key_file_get_integer(keyFile, section, "param3", &error);
        if (error != NULL)
        {
            LOG_Error("Failed to get integer value for key 'param3': %s", error->message);
            g_clear_error(&error);
            return ret;
        }
        LOG_Debug("Value of integer key 'param3' in section%s: %d", sectionName, (int)srv->cfg.param3);

        srv->cfg.param4 = (uint32_t)g_key_file_get_integer(keyFile, section, "param4", &error);
        if (error != NULL)
        {
            LOG_Error("Failed to get integer value for key 'param4': %s", error->message);
            g_clear_error(&error);
            return ret;
        }
        LOG_Debug("Value of integer key 'param4' in section%s: %d", sectionName, (int)srv->cfg.param4);

        char *dev = g_key_file_get_string(keyFile, section, "dev", &error);
        if (error != NULL)
        {
            LOG_Error("Failed to get integer value for key 'dev': %s", error->message);
            g_clear_error(&error);
            return ret;
        }
        LOG_Debug("Value of integer key 'dev' in section%s: %s", sectionName, (unsigned char*)dev);
        (void)strcpy((void*)srv->cfg.device, dev);

    }

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
    return UartInit((void*)cfg->device, cfg->speed, (uint8_t)cfg->param1, (uint8_t)cfg->param2, (uint8_t)cfg->param3, (uint8_t)cfg->param4);
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

    if ((cfg == NULL) || (cfg->param1 > 3U))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    uint32_t mode[] = {SPI_MODE_0, SPI_MODE_1, SPI_MODE_2, SPI_MODE_3};
    int32_t h = SpiInit((void*)cfg->device, mode[cfg->param1], cfg->param2, cfg->speed, cfg->param3, cfg->param4);
    LOG_Info("SpiInit handle=%d (0x%08x)", h, (uint32_t)h);
    return h;
}

/**
 * @name McuSrvInitData
 * @brief 初始化MCU服务数据结构体
 *        该函数用于初始化McuSrvData_t类型的数据结构体，包括清零、设置默认值以及根据配置选择合适的总线通信函数。
 * @param srvData 指向McuSrvData_t结构体的指针，该结构体将被初始化。
 * @return int32_t 返回初始化结果，RET_OK表示成功，RET_ERR表示失败。
 */
static int32_t McuSrvInitData(McuSrv_t *srvData)
{
    int32_t ret = RET_ERR;

    if (NULL == srvData)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    (void)memset(srvData, 0x0, sizeof(McuSrv_t));
    srvData->uartData.dataBusFd = -1;
    srvData->uartData.upgradeStatus = -1;
    srvData->spiData.dataBusFd = -1;
    srvData->spiData.upgradeStatus = -1;

    /*加载UART通信配置*/
    if (RET_OK != McuSrvParserIni(&srvData->uartData, "uart"))
    {
        LOG_Error("%s McuSrvParserIni fail(uart).", __FUNCTION__);
        ret = RET_ERR;
    }
    else
    {
        srvData->uartData.BusOpen = BusUartInit;
        srvData->uartData.busRead = UartRecvData;
        srvData->uartData.BusWrite = UartSendData;
        srvData->uartData.BusClose = UartClose;
    }
    
    /*加载SPI通信配置*/
    if (RET_OK != McuSrvParserIni(&srvData->spiData, "spi"))
    {
        LOG_Error("%s McuSrvParserIni fail(spi).", __FUNCTION__);
        ret = RET_ERR;
    }
    else
    {
        srvData->spiData.BusOpen = BusSpiInit;
        srvData->spiData.busRead = SpiRead;
        srvData->spiData.BusWrite = SpiWrite;
        srvData->spiData.BusClose = SpiClose;
    }

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
    McuSrv_t *srvData = (McuSrv_t *)userdata;

    if (!srvData || !data || (len == 0U))
    {
        return;
    }
    //parser upgrade status.
    srvData->uartData.upgradeStatus = (int32_t)data[0];
    srvData->spiData.upgradeStatus = (int32_t)data[0];
}

int main(int argc, char *argv[])
{
    // McuSrvData_t srvData;
    (void)LOG_INIT("mcuServer");

    (void)McuSrvInitData(&g_srvData);
    (void)StartQueueMsgRcvThread("mcuServer_ipc");
    (void)SubscriptionQueueMsg(SYS_QUE_TYPE_TEST2, McuSrvNotifyUpgradeCallback, (void*)&g_srvData);//todo
    (void)RpcStartMsgServer(RPCSERVER_TYPE_MCUSERVER, McuSrvSendMesgCallback, 1, (void*)&g_srvData);
    (void)McuCanFilterInit();

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    GThread *uartRecvThread = g_thread_new("uartRecvThread", UartDataHandleTask, (gpointer)&g_srvData.uartData);
    GThread *spiRecvThread = NULL;

    /*SPI配置加载失败, 不创建接收线程*/
    if (g_srvData.spiData.BusClose != NULL)
    {
        spiRecvThread = g_thread_new("spiRecvThread", SpiDataHandleTask, (gpointer)&g_srvData.spiData);
    }
        
    g_main_loop_run(loop);
    g_thread_unref(uartRecvThread);
    if (spiRecvThread != NULL)
    {
        g_thread_unref(spiRecvThread);
    }
    g_main_loop_unref(loop);

    return 0;
}
