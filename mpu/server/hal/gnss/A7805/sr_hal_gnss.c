#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "sr_hal_gnss.h"
#include "glib.h"
#include <pthread.h>
#include "Sunsea_GNSS.h"

#define HAL_GNSS_INITIALED (0)
#define HAL_GNSS_UNINITIALED (-1)

#define HAL_GNSS_STARTED (0)
#define HAL_GNSS_UNSTARTED (-1)

//板子里没有gnss.ini时使用此处天线检测配置
#define ANT_ADC_VALUE_MV_NORMAL_MIN (2500) //gnss天线正常电压值下限
#define ANT_ADC_VALUE_MV_NORMAL_MAX (3210) //gnss天线正常电压值上限

#define ANT_ADC_VALUE_MV_OPEN_MIN (3280) //gnss天线开路电压下限
#define ANT_ADC_VALUE_MV_OPEN_MAX (3360) //gnss天线开路电压上限

#define ANT_ADC_VALUE_MV_SHORT_GND_MAX (400)//gnss天线短路至地的上限电压
#define ANT_ADC_VALUE_MV_SHORT_VCC_MIN (3360)//gnss天线短路至电源电压下限

#define GNSS_ADC_FILE_PATH "/sys/kernel/debug/pm800_adc1"

typedef struct
{
    GnssLocationInfoCallBack locationCallCb;
    // GnssSvInfoCallBack SvCallCb;
    GnssNmeaStringCallBack NmeaCallCb;
    int32_t initialed;
    int32_t started;
    int32_t h_loc;
    HalGnssLocationInfo_t locationInfo;
    HalGnssSvStatus_t SvStatus;
    pthread_mutex_t gnssAPILock;
    uint32_t moduleType;
    uint16_t ant_detect_short_gnd_max;
    uint16_t ant_detect_normal_min;
    uint16_t ant_detect_normal_max;
    uint16_t ant_detect_open_min;
    uint16_t ant_detect_open_max;
    uint16_t ant_detect_short_vcc_min;
} HalGnssPriv_t;

static HalGnssPriv_t gGnssPriv =
    {
        .locationCallCb = NULL,
        // .SvCallCb = NULL,
        .NmeaCallCb = NULL,
        .initialed = HAL_GNSS_UNINITIALED,
        .started = HAL_GNSS_UNSTARTED,
        .h_loc = 0,
        .locationInfo = {0},
        .SvStatus = {0},
        .gnssAPILock = PTHREAD_MUTEX_INITIALIZER,
        .moduleType = 0,
        .ant_detect_short_gnd_max = 0,
        .ant_detect_normal_min = 0,
        .ant_detect_normal_max = 0,
        .ant_detect_open_min = 0,
        .ant_detect_open_max = 0,
        .ant_detect_short_vcc_min = 0
    };

static int32_t GnssParserIni(uint32_t *moduleType, void *devPath)
{
    if(devPath == NULL)
    {
        return RET_ERR;
    }
    int32_t ret = RET_ERR;
    GKeyFile *keyFile = NULL;
    GError *error = NULL;
    keyFile = g_key_file_new();

    if (!keyFile)
    {
        LOG_Error("Error new key file");
        return ret;
    }

    if (!g_key_file_load_from_file(keyFile, GNSS_INI_FILE_PATH, G_KEY_FILE_NONE, &error))
    {
        LOG_Error("Error loading key file: %s", error->message);
        g_error_free(error);
        g_key_file_free(keyFile);
        return ret;
    }
    const char *section = "gnss";
    *moduleType = g_key_file_get_integer(keyFile, section, "moduleType", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'moduleType': %s", error->message);
        g_clear_error(&error);
        g_key_file_free(keyFile);
        return ret;
    }
    LOG_Debug("Value of integer key 'moduleType' in section 'gnss': %d",  *moduleType);
    char *dev = g_key_file_get_string(keyFile, section, "dev", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get string value for key 'dev': %s", error->message);
        g_clear_error(&error);
        g_key_file_free(keyFile);
        return ret;
    }
    LOG_Debug("Value of key 'dev' in section 'gnss': %s", dev);
    strcpy(devPath, dev);
    g_free(dev);
    const char *section_ant = "ant";
    gGnssPriv.ant_detect_short_gnd_max = g_key_file_get_integer(keyFile, section_ant, "ant_detect_short_gnd_max", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'ant_detect_short_gnd_max': %s", error->message);
        g_clear_error(&error);
        g_key_file_free(keyFile);
        return ret;
    }
    LOG_Debug("Value of integer key 'ant_detect_short_gnd_max' in section 'ant': %d",  gGnssPriv.ant_detect_short_gnd_max);
    gGnssPriv.ant_detect_normal_min = g_key_file_get_integer(keyFile, section_ant, "ant_detect_normal_min", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'ant_detect_normal_min': %s", error->message);
        g_clear_error(&error);
        g_key_file_free(keyFile);
        return ret;
    }
    LOG_Debug("Value of integer key 'ant_detect_normal_min' in section 'ant': %d",  gGnssPriv.ant_detect_normal_min);
    gGnssPriv.ant_detect_normal_max = g_key_file_get_integer(keyFile, section_ant, "ant_detect_normal_max", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'ant_detect_normal_max': %s", error->message);
        g_clear_error(&error);
        g_key_file_free(keyFile);
        return ret;
    }
    LOG_Debug("Value of integer key 'ant_detect_normal_max' in section 'ant': %d",  gGnssPriv.ant_detect_normal_max);
    gGnssPriv.ant_detect_open_min = g_key_file_get_integer(keyFile, section_ant, "ant_detect_open_min", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'ant_detect_open_min': %s", error->message);
        g_clear_error(&error);
        g_key_file_free(keyFile);
        return ret;
    }
    LOG_Debug("Value of integer key 'ant_detect_open_min' in section 'ant': %d",  gGnssPriv.ant_detect_open_min);
    gGnssPriv.ant_detect_open_max = g_key_file_get_integer(keyFile, section_ant, "ant_detect_open_max", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'ant_detect_open_max': %s", error->message);
        g_clear_error(&error);
        g_key_file_free(keyFile);
        return ret;
    }
    LOG_Debug("Value of integer key 'ant_detect_open_max' in section 'ant': %d",  gGnssPriv.ant_detect_open_max);
    gGnssPriv.ant_detect_short_vcc_min = g_key_file_get_integer(keyFile, section_ant, "ant_detect_short_vcc_min", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'ant_detect_short_vcc_min': %s", error->message);
        g_clear_error(&error);
        g_key_file_free(keyFile);
        return ret;
    }
    LOG_Debug("Value of integer key 'ant_detect_short_vcc_min' in section 'ant': %d",  gGnssPriv.ant_detect_short_vcc_min);
    g_key_file_free(keyFile);
    ret = RET_OK;
    return ret;
}

void sunsea_event_loc_ind(GNSS_LOC_MSG_ID_T event_id, void *data)
{
    pthread_mutex_lock(&gGnssPriv.gnssAPILock);
    switch (event_id)
    {
    case SUNSEA_EVENT_LOC_INFO_IND:
    {
        SunSea_Loc_Info *loc_info = (SunSea_Loc_Info *)data;
        LOG_Debug("LATITUDE:%f  LONGITUDE: %f  ALTITUDE: %f  SPEED: %f",
               loc_info->latitude, loc_info->longitude, loc_info->altitude, loc_info->speed);
    }
    break;
    case SUNSEA_EVENT_LOC_NMEA_IND:
    {
        gchar **datasplit = g_strsplit((char *)data, "\r\n", -1);
        gint num_of_split = g_strv_length(datasplit);
        LOG_Debug("num_of_split:%d", num_of_split);
        for(int i = 0; i < num_of_split; i ++)
        {
            LOG_Debug("split[%d]:[%s]", i, datasplit[i]);
            HalGnssNmea_t nmeaMsg = {0};
            nmeaMsg.length = strlen(datasplit[i]);
            nmeaMsg.timestamp = 0;
            memmove(nmeaMsg.nmea, datasplit[i], nmeaMsg.length);
            gchar **gps_data = g_strsplit(nmeaMsg.nmea, ",", -1);
            if (gps_data[0] != NULL)
            {
                if ((0 == strncmp(&gps_data[0][3], "RMC", 3)) && ('$' == gps_data[0][0]))
                {
                    gGnssPriv.locationInfo.time = strtol(gps_data[1], NULL, 10);
                    if (gps_data[2][0] == 'A')
                    {
                        gGnssPriv.locationInfo.posState = 'A';
                    }
                    else
                    {
                        gGnssPriv.locationInfo.posState = 'V';
                    }
                    gGnssPriv.locationInfo.latitude = strtod(gps_data[3], NULL);
                    if (gps_data[4][0] == 'S')
                    {
                        gGnssPriv.locationInfo.latitudeFlag = 'S';
                    }
                    else
                    {
                        gGnssPriv.locationInfo.latitudeFlag = 'N';
                    }
                    gGnssPriv.locationInfo.longitude = strtod(gps_data[5], NULL);
                    if (gps_data[6][0] == 'W')
                    {
                        gGnssPriv.locationInfo.longitudeFlag = 'W';
                    }
                    else
                    {
                        gGnssPriv.locationInfo.longitudeFlag = 'E';
                    }
                    gGnssPriv.locationInfo.speed = strtod(gps_data[7], NULL);
                    gGnssPriv.locationInfo.direction = strtod(gps_data[8], NULL);
                    gGnssPriv.locationInfo.date = strtol(gps_data[9], NULL, 10);
                    gGnssPriv.locationInfo.declination = strtol(gps_data[10], NULL, 10);
                    gGnssPriv.locationInfo.declinationDirection = gps_data[11][0];
                    gGnssPriv.locationInfo.mode = gps_data[12][0];
                    if (gGnssPriv.locationCallCb)
                    {
                        HalGnssLocationInfo_t locationMsg = gGnssPriv.locationInfo;
                        gGnssPriv.locationCallCb(&locationMsg);
                    }
                }
                else if (0 == (strncmp(&gps_data[0][3], "GGA", 3)) && ('$' == gps_data[0][0]))
                {
                    gGnssPriv.locationInfo.gpsState = strtol(gps_data[6], NULL, 10);
                    gGnssPriv.locationInfo.sumNosv = strtol(gps_data[7], NULL, 10);
                    gGnssPriv.locationInfo.height = strtod(gps_data[9], NULL);
                }
                else
                {
                    g_strfreev(gps_data);
                    continue;
                }
                //只处理GGA RMC 其他不处理
                if (gGnssPriv.NmeaCallCb)
                {
                    gGnssPriv.NmeaCallCb(&nmeaMsg);
                }
            }
            g_strfreev(gps_data);
        }
        g_strfreev(datasplit);
        break;
    }
    default:
        break;
    }
    pthread_mutex_unlock(&gGnssPriv.gnssAPILock);
}

int32_t HalGnssInit(GnssLocationInfoCallBack locaCb, GnssNmeaStringCallBack nmeaCb)
{
    pthread_mutex_lock(&gGnssPriv.gnssAPILock);
    if (HAL_GNSS_INITIALED == gGnssPriv.initialed)
    {
        // have initialed, return.
        LOG_Debug("%s have initialed.", __FUNCTION__);
        pthread_mutex_unlock(&gGnssPriv.gnssAPILock);
        return RET_ERR;
    }

    if(locaCb != NULL)
    {
        gGnssPriv.locationCallCb = locaCb;
    }
    if(nmeaCb != NULL)
    {
        gGnssPriv.NmeaCallCb = nmeaCb;
    }

    char gnssDevPath[64] = {0};
    if (RET_OK != GnssParserIni(&(gGnssPriv.moduleType), gnssDevPath))
    {
        LOG_Error("%s GnssParserIni fail.", __FUNCTION__);
        gGnssPriv.moduleType = 0;
        gGnssPriv.ant_detect_short_gnd_max = ANT_ADC_VALUE_MV_SHORT_GND_MAX;
        gGnssPriv.ant_detect_normal_min = ANT_ADC_VALUE_MV_NORMAL_MIN;
        gGnssPriv.ant_detect_normal_max = ANT_ADC_VALUE_MV_NORMAL_MAX;
        gGnssPriv.ant_detect_open_min = ANT_ADC_VALUE_MV_OPEN_MIN;
        gGnssPriv.ant_detect_open_max = ANT_ADC_VALUE_MV_OPEN_MAX;
        gGnssPriv.ant_detect_short_vcc_min = ANT_ADC_VALUE_MV_SHORT_VCC_MIN;
    }

    if (Sunsea_Gnss_Init(sunsea_event_loc_ind,0) != 0)
    {
        LOG_Debug("simcom loc init failed");
        pthread_mutex_unlock(&gGnssPriv.gnssAPILock);
        return RET_ERR;
    }
    gGnssPriv.initialed = HAL_GNSS_INITIALED;
    pthread_mutex_unlock(&gGnssPriv.gnssAPILock);
    return RET_OK;
}

int32_t HalGnssStart(void)
{
    pthread_mutex_lock(&gGnssPriv.gnssAPILock);
    int ret = 0;
    if (HAL_GNSS_STARTED == gGnssPriv.started)
    {
        LOG_Error("%s have started.", __FUNCTION__);
        pthread_mutex_unlock(&gGnssPriv.gnssAPILock);
        return RET_ERR;
    }
    ret = Sunsea_Gnss_Start();
    if (ret != 0)
    {
        LOG_Debug("GNSS start faild");
        pthread_mutex_unlock(&gGnssPriv.gnssAPILock);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("GNSS start success");
    }
    g_usleep(100000);
    //please set mode:(1-7)
    ret = Sunsea_Gnss_SetPositionMode(GNSS_MODE_GPS_BEIDOU);
    if (ret != 0)
    {
        LOG_Debug("GNSS set mode faild");
        pthread_mutex_unlock(&gGnssPriv.gnssAPILock);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("GNSS set mode success");
    }
    //please switch:(0 NMEA or 1 LOC_INFO)
    ret = Sunsea_Gnss_Nmea_Info_Switch(NMEA);
    if (ret != 0)
    {
        LOG_Debug("GNSS set output data faild");
        pthread_mutex_unlock(&gGnssPriv.gnssAPILock);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("GNSS set output data success");
    }
#if 0
    //A232102B01V01A7805M22A版本没有此接口，nmea默认输出到回调
    //please switch:(0 CB or 1 NMEA_PORT)
    ret = Sunsea_Gnss_Output_Channel_Switch(GNSS_INFO_OUTPUT_CB);
    if (ret != 0)
    {
        LOG_Debug("GNSS set output port faild");
        pthread_mutex_unlock(&gGnssPriv.gnssAPILock);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("GNSS set output port success");
    }
#endif
    gGnssPriv.started = HAL_GNSS_STARTED;
    pthread_mutex_unlock(&gGnssPriv.gnssAPILock);
    return RET_OK;
}

int32_t HalGnssEnd(void)
{
    pthread_mutex_lock(&gGnssPriv.gnssAPILock);
    int ret = 0;
    if (HAL_GNSS_STARTED != gGnssPriv.started)
    {
        LOG_Error("%s have not started.", __FUNCTION__);
        pthread_mutex_unlock(&gGnssPriv.gnssAPILock);
        return RET_ERR;
    }
    ret = Sunsea_Gnss_Stop();
    if (ret != 0)
    {
        LOG_Debug("GNSS stop faild");
        //这块stop会返回ERR，但是实际上会stop成功，暂时屏蔽，后续反馈给厂家
        // pthread_mutex_unlock(&gGnssPriv.gnssAPILock);
        // return RET_ERR;
    }
    else
    {
        LOG_Debug("GNSS stop success");
    }
    gGnssPriv.started = HAL_GNSS_UNSTARTED;
    pthread_mutex_unlock(&gGnssPriv.gnssAPILock);
    return RET_OK;
}

int32_t HalGnssDeInit(void)
{
    pthread_mutex_lock(&gGnssPriv.gnssAPILock);
    int ret = 0;
    if (HAL_GNSS_INITIALED != gGnssPriv.initialed)
    {
        LOG_Error("%s have not started.", __FUNCTION__);
        pthread_mutex_unlock(&gGnssPriv.gnssAPILock);
        return RET_ERR;
    }
    ret = Sunsea_Gnss_Deinit();
    if (ret != 0)
    {
        LOG_Debug("GNSS deinit faild");
        pthread_mutex_unlock(&gGnssPriv.gnssAPILock);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("GNSS deinit success");
    }
    gGnssPriv.initialed = HAL_GNSS_UNINITIALED;
    pthread_mutex_unlock(&gGnssPriv.gnssAPILock);
    return RET_OK;
}

void HalGnssGetLocationInfo(HalGnssLocationInfo_t *data)
{
    memmove(data, &gGnssPriv.locationInfo, sizeof(HalGnssLocationInfo_t));
}

void HalGnssGetSvStatus(HalGnssSvStatus_t *data)
{
    memmove(data, &gGnssPriv.SvStatus, sizeof(HalGnssSvStatus_t));
}

void HalGnssGetAntStatus(HalGnssAntStatus_e *data)
{
    GError *error = NULL;
    gchar *contents;
    gsize length;
    if (!g_file_get_contents(GNSS_ADC_FILE_PATH, &contents, &length, &error)) 
    {
        LOG_Debug("Error reading file: %s", error->message);
        g_error_free(error);
        *data = E_GNSS_ANT_STATUS_CIRCUIT_UNKNOW;
        return;
    }
    gint adc_value = g_ascii_strtoll(contents, NULL, 10);
    g_free(contents);

    LOG_Debug("gnss ant adc_value:%d",adc_value);
    if (adc_value == 0)
    {
        *data = E_GNSS_ANT_STATUS_CIRCUIT_UNKNOW;
    }
    else if(adc_value < gGnssPriv.ant_detect_short_gnd_max)
    {
        *data = E_GNSS_ANT_STATUS_CIRCUIT_SHORT_GND;
    }
    else if(adc_value >= gGnssPriv.ant_detect_normal_min && adc_value <= gGnssPriv.ant_detect_normal_max)
    {
        *data = E_GNSS_ANT_STATUS_CIRCUIT_NORMAL;
    }
    else if(adc_value >= gGnssPriv.ant_detect_open_min && adc_value < gGnssPriv.ant_detect_open_max)
    {
        *data = E_GNSS_ANT_STATUS_CIRCUIT_OPEN;
    }
    else if(adc_value >= gGnssPriv.ant_detect_short_vcc_min)
    {
        *data = E_GNSS_ANT_STATUS_CIRCUIT_SHORT_VCC;
    }
    else
    {
        *data = E_GNSS_ANT_STATUS_CIRCUIT_UNKNOW;
    }
}
