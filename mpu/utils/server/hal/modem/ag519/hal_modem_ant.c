#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "sr_log.h"
#include "err_code.h"
#include "hal_modem_ant.h"

#define HAL_MODEM_ANT_INITIALED (0)
#define HAL_MODEM_ANT_UNINITIALED (-1)

typedef struct
{
    int32_t initialed;
}HalModemAntPriv_t;

HalModemAntPriv_t gAntPriv =
{
    .initialed = HAL_MODEM_ANT_UNINITIALED
};

#define ANT_CHANGE_PINNO (9)//pin_187->gpio9
#define MAX_BUF 128

typedef enum  ADC_CHANNEL_ENUM{
    QADC_NONE = 0,                      /** Invalid  channel*/
    ADC0 = 1,
    ADC1 = 2,
    ADC2 = 3,
    QADC_END                            /** Invalid  channel*/
}ADC_CHANNEL_E;


#define VADC_ADC0 "in_voltage21_GPS_ANT_input"
#define VADC_ADC1 "in_voltage22_MAIN_ANT_input"
#define VADC_ADC2 "in_voltage23_DRX_ANT_input"


#define ADC_DIR_PREFIX "/sys/bus/iio/devices/iio:device0"

static void buildAdcPath(char *adcPath, size_t pathSize, ADC_CHANNEL_E qadc)
{
    char *adcStr = NULL;

    if (qadc == ADC0)
    {
        adcStr = VADC_ADC0;
    }
    else if (qadc == ADC1)
    {
        adcStr = VADC_ADC1;
    }
    else if (qadc == ADC1)
    {
        adcStr = VADC_ADC2;
    }
    else
    {
        adcStr = "";
    }

    snprintf(adcPath, pathSize, "%s/%s", ADC_DIR_PREFIX, adcStr);
}

static int32_t adcShow(ADC_CHANNEL_E qadc)
{
    char buf[24] = {0};
    char adcPath[256] = {0};
    int32_t adcFd;
    int32_t len = 0;

    if ((qadc <= QADC_NONE) || (qadc >= QADC_END))
    {
        LOG_Debug("%spara is invalid\n", __FUNCTION__);
        return -1;
    }

    buildAdcPath(adcPath, sizeof(adcPath), qadc);

    adcFd = open(adcPath, O_RDONLY | O_NONBLOCK);
    if (adcFd < 0)
    {
        LOG_Debug("%sopen failed", __FUNCTION__);
        return -1;
    }

    if ((len = read(adcFd, buf, sizeof(buf) - 1)) < 0)
    {
        LOG_Debug("%sread failed", __FUNCTION__);
        close(adcFd);
        return -1;
    }
    buf[len] = '\0';

    char *token = strtok(buf, " ");
    if (token == NULL)
    {
        LOG_Debug("%sinvalid data format", __FUNCTION__);
        close(adcFd);
        return -1;
    }

    close(adcFd);
    return atoi(token);
}

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

static int GpioSetIsrNone(uint32_t gpio)
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

int32_t HalModemAntGetAdc(int32_t *adcVal)
{
    if (!adcVal)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;        
    }

    *adcVal = adcShow(ADC0);
    return RET_OK;
}

int32_t HalModemSecAntGetAdc(int32_t *adcVal)
{
    if (!adcVal)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;        
    }

    *adcVal = adcShow(ADC2);
    return RET_OK;
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

    if (1 == value)
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
        GpioSetValue(ANT_CHANGE_PINNO, 1);
    }
    else if (ant == E_HAL_MODEM_ANT_EXTERNEL)
    {
        LOG_Debug("%s set value:0.", __FUNCTION__);
        GpioSetValue(ANT_CHANGE_PINNO, 0);
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
