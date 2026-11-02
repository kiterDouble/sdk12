#include <glib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "pm_io.h"
#include "err_code.h"
#include "sr_log.h"
#include "pm_server_api.h"
#include "pm_modem.h"

#define GPIO_NOTIFY "/dev/gpio_notify"
#define IRQ_EDGE "/sys/devices/soc:quec,quectel-low-consume/irq_edge"
#define IRQ_WAKEUP_SOURCE "/sys/devices/soc:quec,quectel-low-consume/irq_wake_source_name"

static gint g_notifyFd = -1;

static int8_t PmIoGetIrqFileContent(char *fileName, char *content, uint32_t contentLen)
{
    gint ret = RET_ERR;
    gint fd = open(fileName, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        LOG_Error("Open WAKEUP_MODULE [%s] error:%s", fileName, g_strerror(errno));
        return RET_ERR;
    }
    if (read(fd, content, contentLen) < 0)
    {
        LOG_Error("Read WAKEUP_MODULE GPIO error:%s", g_strerror(errno));
    }
    else
    {
        ret = RET_OK;
    }
    close(fd);
    return ret;
}

static void PmIoHandleCallBack(int signal)
{
    char edge[8] = {0};
    char fileName[256] = {0x00};
    int irqFlag = 0;
    gint curWakeUpSource = PM_WAKEUP_SOURCE_INVALID;
    LOG_Debug("gpio irq: Have caught signal %d from driver", signal);
    if (access(IRQ_EDGE, F_OK) == 0)
    {
        g_stpcpy(fileName, IRQ_EDGE);
        curWakeUpSource = PM_WAKEUP_SOURCE_MCU;
        irqFlag = 1;
    }
    else if (access(IRQ_WAKEUP_SOURCE, F_OK) == 0)
    {
        g_stpcpy(fileName, IRQ_WAKEUP_SOURCE);
        curWakeUpSource = PM_WAKEUP_SOURCE_INVALID;
        irqFlag = 2;
    }

    if (PmIoGetIrqFileContent(fileName, edge, sizeof(edge)) == RET_ERR)
    {
        LOG_Error("Get IRQ File Error,Default mcu.");
        g_memmove(edge, "mcu", 3);
    }

    if (irqFlag == 2)
    {
        if (strncmp("mcu", edge, 3) == 0)
        {
            curWakeUpSource = PM_WAKEUP_SOURCE_MCU;
        }
        else if (strncmp("bt", edge, 2) == 0)
        {
            curWakeUpSource = PM_WAKEUP_SOURCE_BLE;
        }
        else if (strncmp("gsensor", edge, 7) == 0)
        {
            curWakeUpSource = PM_WAKEUP_SOURCE_G_SENSOR;
            LOG_Debug("[PM] gsensor int");
        }
    }
    /*wake up system*/
    LOG_Debug("Get IO WakeUp Source:%d", curWakeUpSource);
    PmModemWakeup(curWakeUpSource,CPU_INVALID_EVENT_ID);
}

int8_t PmWakeupIoInit(void)
{
    gint ret, Oflags;

    signal(SIGIO, PmIoHandleCallBack);
    g_notifyFd = open(GPIO_NOTIFY, O_RDWR);
    if (g_notifyFd < 0)
    {
        LOG_Error("open [%s] failed:%s", GPIO_NOTIFY, g_strerror(errno));
        goto err;
    }
    ret = fcntl(g_notifyFd, F_SETOWN, getpid());
    if (ret < 0)
    {
        LOG_Error("fcntl [%s] SETOWN Failed:%s", GPIO_NOTIFY, g_strerror(errno));
        goto err;
    }

    Oflags = fcntl(g_notifyFd, F_GETFL);
    if (Oflags < 0)
    {
        LOG_Error("fcntl [%s] GETFL Failed:%s", GPIO_NOTIFY, g_strerror(errno));
        goto err;
    }
    ret = fcntl(g_notifyFd, F_SETFL, Oflags | FASYNC);
    if (ret < 0)
    {
        LOG_Error("fcntl [%s] SETFL Failed:%s", GPIO_NOTIFY, g_strerror(errno));
        goto err;
    }
    LOG_Debug("register gpio irq callback ok");
    return RET_OK;
err:
    return RET_ERR;
}


