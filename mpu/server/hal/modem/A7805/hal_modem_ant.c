#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "sr_log.h"
#include "err_code.h"
#include "hal_modem_ant.h"


#define HAL_MODEM_ANT_INITIALED (0)
#define HAL_MODEM_ANT_UNINITIALED (-1)

#define ANT_CHANGE_PINNO (41)
#define MAX_BUF 128

typedef struct
{
    int32_t initialed;
}HalModemAntPriv_t;

HalModemAntPriv_t gAntPriv =
{
    .initialed = HAL_MODEM_ANT_UNINITIALED
};

static int32_t GpioFileCreate(uint32_t gpio)
{
	int32_t sfd = -1;
	char checkstr[MAX_BUF] = {0};
	char configstr[MAX_BUF] = {0};
	int32_t reto = 0;
    int32_t len = 0;

	sprintf(checkstr,"/sys/class/gpio/gpio%u/value", gpio);
	if(0 == access(checkstr, F_OK))
    {
		return RET_OK;
	}

	sfd = open("/sys/class/gpio/export",O_WRONLY);
	if (sfd < 0)
    {
		LOG_Error("%s open fail.", __FUNCTION__);
		return RET_ERR;
	}

	len = sprintf(configstr,"%u",gpio);
	reto = write(sfd,configstr,len);

	if (reto != len)
    {
		LOG_Error("%s write fail.", __FUNCTION__);
        close(sfd);
		return RET_ERR;
	}
    close(sfd);
	usleep(1000);
	if (0 != access(checkstr, F_OK))
    {
		LOG_Error("%s:%d,gpio file(%s)not exist.", __FUNCTION__, __LINE__, checkstr);
        return RET_ERR;
	}
	return RET_OK;
}

int32_t GpioFileRemove(uint32_t gpio)
{
	int32_t sfd = -1;
	char checkstr[MAX_BUF] = {0};
	char configstr[MAX_BUF] = {0};
	int32_t reto = 0;
    int32_t len = 0;

	sprintf(checkstr,"/sys/class/gpio/gpio%u/value",gpio);
	if(0 != access(checkstr, F_OK))
    {
        LOG_Error("%s gpio not export.", __FUNCTION__);
		return RET_ERR;
	}

	sfd = open("/sys/class/gpio/unexport",O_WRONLY);

	if (sfd < 0)
    {
		LOG_Error("%s,open fail.",__FUNCTION__);
		return sfd;
	}

	len = sprintf(configstr,"%u",gpio);
	reto = write(sfd,configstr,len);

	if (reto != len)
    {
		LOG_Error("%s write fail.",__FUNCTION__);
		return RET_ERR;
	}

	usleep(1000);
	reto = 0;
	if (0 == access(checkstr, F_OK))
    {
		LOG_Error("%s unexport fail.",__FUNCTION__);
		return RET_ERR;
	}

	close(sfd);
	return RET_OK;
}

static int32_t GpioSetDir(uint32_t gpio, uint32_t dir)
{
	int32_t fd;
	char buf[MAX_BUF];
 
	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%u/direction", gpio);
	fd = open(buf, O_WRONLY);
	if (fd < 0)
    {
		LOG_Error("%s open fail.", __FUNCTION__);
		return RET_ERR;
	}
 
	if (dir)
    {
        write(fd, "out", 4);
    }
	else
    {
        write(fd, "in", 3);
    }
		
	close(fd);
	return RET_OK;
}

static int32_t GpioSetValue(uint32_t gpio, uint32_t value)
{
	int32_t fd;
	char buf[MAX_BUF];
 
	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%u/value", gpio);
	fd = open(buf, O_WRONLY);
	if (fd < 0)
    {
        LOG_Error("%s open fail.", __FUNCTION__);
		return RET_ERR;
	}
 
	if (value)
    {
        write(fd, "1", 2);
    }
	else
    {
        write(fd, "0", 2);
    }
		
	close(fd);
	return RET_OK;
}

static int32_t GpioGetValue(uint32_t gpio, uint32_t *value)
{
    int32_t fd;
    int32_t count;
    char buff[MAX_BUF];
    snprintf(buff, sizeof(buff), "/sys/class/gpio/gpio%u/value", gpio);

    fd = open(buff, O_RDONLY);
    if (fd < 0)
    {
        LOG_Error("%s open fail.", __FUNCTION__);
        return RET_ERR;
    }
    memset(buff,0,MAX_BUF);
    count = read(fd, buff, MAX_BUF);
    if (count == 0)
    {
        LOG_Error("%s read faild.", __FUNCTION__);
        close(fd);
        return RET_ERR;
    }
    buff[strlen(buff)-1] = 0;
    if (strncmp(buff, "0", 2) == 0)
    {
        *value = 0;
    }
    else
    {
        *value = 1;
    }
    close(fd);
	return RET_OK;
}

int GpioSetIsrNone(uint32_t gpio)
{
	char fvalue_path[MAX_BUF];
	int fd = -1;

	sprintf(fvalue_path,"/sys/class/gpio/gpio%u/edge",gpio);
	fd = open(fvalue_path,O_RDWR);
	if(fd < 0)
    {
        LOG_Error("%s open fail.", __FUNCTION__);
        return RET_ERR;
	}


    if(write(fd,"none",5) < 0)
    {
        close(fd);
        LOG_Error("%s write %s error.", __FUNCTION__, fvalue_path);
        return RET_ERR;
    }

	close(fd);
	return RET_OK;
}

int32_t HalModemAntInit(void)
{
    if (HAL_MODEM_ANT_INITIALED == gAntPriv.initialed)
    {
        LOG_Debug("%s have initialed.", __FUNCTION__);
        return RET_OK;
    }

    if (RET_OK != GpioFileCreate(ANT_CHANGE_PINNO))
    {
        LOG_Error("%s GpioFileCreate fail.", __FUNCTION__);
        return RET_ERR;
    }

    if (RET_OK != GpioSetDir(ANT_CHANGE_PINNO, 1))
    {
        LOG_Error("%s GpioSetDir fail.", __FUNCTION__);
        return RET_ERR;
    }
    if (RET_OK != GpioSetValue(ANT_CHANGE_PINNO, 0))
    {
        LOG_Error("%s GpioSetValue fail.", __FUNCTION__);
        return RET_ERR;
    }
    if (RET_OK != GpioSetIsrNone(ANT_CHANGE_PINNO))
    {
        LOG_Error("%s GpioSetIsrNone fail.", __FUNCTION__);
        return RET_ERR;
    }
    gAntPriv.initialed = HAL_MODEM_ANT_INITIALED;
    return RET_OK;
}


static int32_t ReadAdcValue(int32_t adcNumber, int32_t *value)
{
    FILE *fp = NULL;
    int32_t ret = RET_ERR;
    char *filepath[] = 
    {
        "/sys/kernel/debug/pm800_adc",
        "/sys/kernel/debug/pm800_adc1",
        "/sys/kernel/debug/pm800_adc2"
    };

    if ((0 > adcNumber) || (2 < adcNumber) || !value)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    fp = fopen(filepath[adcNumber], "r");
    if (fp == NULL)
    {
        LOG_Error("%s fopen fail.", __FUNCTION__);
        return ret; 
    }

    char buffer[MAX_BUF];
    if (fgets(buffer, sizeof(buffer), fp) == NULL)
    {
        LOG_Error("%s fgets fail.", __FUNCTION__);
        fclose(fp);
        return ret; 
    }

    fclose(fp);
    *value= atoi(buffer);
    ret = RET_OK;
    return ret;
}

int32_t HalModemAntGetAdc(int32_t *adcVal)
{
    return ReadAdcValue(0, adcVal);
}

int32_t HalModemSecAntGetAdc(int32_t *adcVal)
{
    return ReadAdcValue(2, adcVal);
}

int32_t HalModemAntGetCurrent(HalModemAntType_e *ant)
{
    if (!ant)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;        
    }

    if (HAL_MODEM_ANT_INITIALED != gAntPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return RET_ERR;
    }

    uint32_t value;
   
    if(GpioGetValue(ANT_CHANGE_PINNO, &value))
    {
        LOG_Error("%s GpioGetValue fail.", __FUNCTION__);
        return RET_ERR;
    }

    if (0 == value)
    {
        *ant = E_HAL_MODEM_ANT_INTERNAL;
    }
    else
    {
        *ant = E_HAL_MODEM_ANT_EXTERNEL;
    }

    LOG_Debug("%s ant value:%d.", __FUNCTION__, *ant);
    return RET_OK;
}

int32_t HalModemAntChange(HalModemAntType_e ant)
{
    if (HAL_MODEM_ANT_INITIALED != gAntPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return RET_ERR;
    }

    if (ant == E_HAL_MODEM_ANT_INTERNAL)
    {
        LOG_Debug("%s set value:1.", __FUNCTION__);
        GpioSetValue(ANT_CHANGE_PINNO, 0);
    }
    else if (ant == E_HAL_MODEM_ANT_EXTERNEL)
    {
        LOG_Debug("%s set value:0.", __FUNCTION__);
        GpioSetValue(ANT_CHANGE_PINNO, 1);
    }
    else
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;
    }
    
    return RET_OK;
}

int32_t HalModemAntDeinit(void)
{
    if (HAL_MODEM_ANT_INITIALED != gAntPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return RET_ERR;
    }

    if (RET_OK != GpioFileRemove(ANT_CHANGE_PINNO))
    {
        LOG_Error("%s Ql_GPIO_Uninit fail.", __FUNCTION__);
        return RET_ERR;        
    }

    gAntPriv.initialed = HAL_MODEM_ANT_UNINITIALED;
    return RET_OK;
}
