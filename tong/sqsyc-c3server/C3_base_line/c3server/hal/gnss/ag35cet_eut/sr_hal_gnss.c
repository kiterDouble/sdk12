#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include "err_code.h"
#include "sr_log.h"
#include "sr_hal_gnss.h"
// #include "test_utils.h"
#include "ql-sdk/ql_type.h"
#include "ql-sdk/ql_gnss.h"
#include "ql-sdk/ql_adc.h"

#define HAL_GNSS_INITIALED (0)
#define HAL_GNSS_UNINITIALED (-1)

#define HAL_GNSS_STARTED (0)
#define HAL_GNSS_UNSTARTED (-1)

//板子里没有gnss.ini时使用此处天线检测配置
#define ANT_ADC_VALUE_MV_NORMAL_MIN (1699) //gnss天线正常电压值下限
#define ANT_ADC_VALUE_MV_NORMAL_MAX (1899) //gnss天线正常电压值上限

#define ANT_ADC_VALUE_MV_OPEN_MIN (2084) //gnss天线开路电压下限
#define ANT_ADC_VALUE_MV_OPEN_MAX (2284) //gnss天线开路电压上限

#define ANT_ADC_VALUE_MV_SHORT_GND_MAX (400)//gnss天线短路至地的上限电压
#define ANT_ADC_VALUE_MV_SHORT_VCC_MIN (3000)//gnss天线短路至电源电压下限

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
    uint32_t moduleType;
    int32_t gnssModuleFd;
    char g_parseGpsBuf[2048];
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
        .moduleType = 0,
        .gnssModuleFd = -1,
        .g_parseGpsBuf = {0},
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

    const char *gnssIniPath = GNSS_INI_FILE_PATH;
    if (0 == access(GNSS_INI_FILE_PATH_PRIMARY, F_OK)) {
        gnssIniPath = GNSS_INI_FILE_PATH_PRIMARY;
        LOG_Debug("Using primary config: %s", gnssIniPath);
    }

    if (!g_key_file_load_from_file(keyFile, gnssIniPath, G_KEY_FILE_NONE, &error))
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

//gGnssPriv.locationInfo.num_svs 可见卫星数量 无来源
//gGnssPriv.locationInfo.accuracy 水平精度 无来源
static void gnss_ind_cb(void *msg)
{
    uint8_t *msg_id = NULL;
    msg_id = msg;
    if (*msg_id == QL_GNSS_NMEA_MSG)
    {
        nmea_srv_ind_msg *data = (nmea_srv_ind_msg *)msg;
        // if (strstr(data->nmea_sentence, "GPTXT") == NULL)
        // {
        //     LOG_Debug("time=%d %s", data->time, data->nmea_sentence);
        // }
        HalGnssNmea_t nmeaMsg = {0};
        nmeaMsg.length = strlen(data->nmea_sentence);
        nmeaMsg.timestamp = data->time;
        memmove(nmeaMsg.nmea, data->nmea_sentence, nmeaMsg.length);
        gchar **gps_data = g_strsplit(nmeaMsg.nmea, ",", -1);
        if (gps_data[0] != NULL)
        {
            // if (strcmp(gps_data[0], "$GNRMC") == 0)
            if (strncmp(&gps_data[0][3], "RMC", 3) == 0)
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
                if((gGnssPriv.locationInfo.date == 0) || (gGnssPriv.locationInfo.time == 0))
                {
                    gGnssPriv.locationInfo.date = 0;
                    gGnssPriv.locationInfo.time = 0;
                }
                gGnssPriv.locationInfo.declination = strtol(gps_data[10], NULL, 10);
                gGnssPriv.locationInfo.declinationDirection = gps_data[11][0];
                gGnssPriv.locationInfo.mode = gps_data[12][0];
                if (gGnssPriv.locationCallCb)
                {
                    HalGnssLocationInfo_t locationMsg = gGnssPriv.locationInfo;
                    gGnssPriv.locationCallCb(&locationMsg);
                }
            }
            // else if (strcmp(gps_data[0], "$GNGGA") == 0)
            else if (strncmp(&gps_data[0][3], "GGA", 3) == 0)
            {
                gGnssPriv.locationInfo.gpsState = strtol(gps_data[6], NULL, 10);
                gGnssPriv.locationInfo.sumNosv = strtol(gps_data[7], NULL, 10);
                gGnssPriv.locationInfo.height = strtod(gps_data[9], NULL);
            }
            else if (strncmp(&gps_data[0][3], "GSA", 3) == 0)
            {
                // do nothing
            }
            else if (strncmp(&gps_data[0][3], "GSV", 3) == 0)
            {
                // do nothing
            }
            else if (strncmp(&gps_data[0][3], "GLL", 3) == 0)
            {
                // do nothing
            }
            else if (strncmp(&gps_data[0][3], "VTG", 3) == 0)
            {
                // do nothing
            }
            else
            {
                g_strfreev(gps_data);
                return;
            }

            if (gGnssPriv.NmeaCallCb)
            {
                gGnssPriv.NmeaCallCb(&nmeaMsg);
            }
        }
        g_strfreev(gps_data);
    }
    else if (*msg_id == QL_GNSS_STATUS_MSG)
    {
        gnss_status_ind_msg *data = (gnss_status_ind_msg *)msg;
        switch (data->g_status)
        {
        case QL_GNSS_STATUS_FOTA_START:
            LOG_Debug("gnss engine fota req");
            break;
        case QL_GNSS_STATUS_FOTA_BOOT:
            LOG_Debug("gnss engine fota bootloader");
            break;
        case QL_GNSS_STATUS_FOTA_FIRM:
            LOG_Debug("gnss engine fota firmware");
            break;
        case QL_GNSS_STATUS_WORKING:
            LOG_Debug("gnss engine working");
            break;
        default:
            break;
        };
    }
    else
    {
        LOG_Debug("Invalue msg !!!");
    }
}

static void *QLLocationReset(void *pArg)
{
    // 1.调用 ql_gnss_deinit()关闭当前客户端；
    int32_t err = ql_gnss_deinit();
    if (err != QL_ERR_OK)
    {
        LOG_Debug("[QLLocationReset]Failed to ql_gnss_deinit, err=%d", err);
    }
    else
    {
        LOG_Debug("[QLLocationReset]ql_gnss_deinit Successful");
    }
    // 2.执行/etc/init.d/ql_locationd restart 重启 ql_locationd；
    system("/etc/init.d/ql_locationd restart");
    return NULL;
}

static void gnss_service_error_cb(int error)
{
    LOG_Debug("===== GNSS Service Abort. error code:%d =====", error);
    g_thread_new("QLLocationReset", QLLocationReset, NULL);
}

int32_t configure_serial_port(int32_t fd)
{
    struct termios options;

    // 获取当前串口配置
    if (tcgetattr(fd, &options) != 0)
    {
        perror("tcgetattr failed");
        return -1;
    }

    // 设置波特率为115200
    cfsetispeed(&options, B115200); // 输入波特率
    cfsetospeed(&options, B115200); // 输出波特率

    // 设置数据位、停止位和校验位
    options.c_cflag &= ~PARENB; // 无校验
    options.c_cflag &= ~CSTOPB; // 1 个停止位
    options.c_cflag &= ~CSIZE;  // 清除数据位掩码
    options.c_cflag |= CS8;     // 8 个数据位

    // 设置为原始模式
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    // 应用配置
    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        perror("tcsetattr failed");
        return -1;
    }

    return 0;
}

static void* recvGpsData(void* pArg)
{
    uint32_t rdLen;
    uint32_t s32Ret = 0;
    static uint32_t gpsErrorCount = 0;

    static fd_set fdset;
    static struct timeval timeout = {5, 0};
    char gpsDataBuffer[1024] = {0};

    while (1)
    {
        FD_ZERO(&fdset);
        FD_SET(gGnssPriv.gnssModuleFd, &fdset);

        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        s32Ret = select(gGnssPriv.gnssModuleFd + 1, &fdset, NULL, NULL, &timeout);

        if (s32Ret > 0)
        {
            if (FD_ISSET(gGnssPriv.gnssModuleFd, &fdset))
            {
                memset(gpsDataBuffer, 0x00, sizeof(gpsDataBuffer));
                rdLen = read(gGnssPriv.gnssModuleFd, gpsDataBuffer, sizeof(gpsDataBuffer));
                if (rdLen < 0)
                {
                    perror("read gps:");
                    break;
                }
                else
                {
                    char *startPtr = NULL, *endPtr = NULL;
                    int32_t packageLen = 0;

                    strcat(gGnssPriv.g_parseGpsBuf, gpsDataBuffer);
                    startPtr = gGnssPriv.g_parseGpsBuf;

                    while (1)
                    {
                        if (startPtr != gGnssPriv.g_parseGpsBuf)
                        {
                            memmove(gGnssPriv.g_parseGpsBuf, startPtr, (gGnssPriv.g_parseGpsBuf + 2048) - startPtr);
                            startPtr = gGnssPriv.g_parseGpsBuf;
                        }
                        startPtr = strstr(startPtr, "$");
                        if (startPtr == NULL)
                        {
                            break;
                        }
                        endPtr = strstr(startPtr, "*");
                        if (endPtr == NULL)
                        {
                            break;
                        }
                        packageLen = endPtr - startPtr + 3;

                        // LOG_Debug("gnss Package,Len[%d]-[%s]", packageLen, startPtr);

                        nmea_srv_ind_msg nmeamsg = {0};
                        nmeamsg.msg_id = QL_GNSS_NMEA_MSG;
                        memcpy(nmeamsg.nmea_sentence, startPtr, packageLen);
                        gnss_ind_cb(&nmeamsg);

                        startPtr = endPtr + 2;
                    }
                }
            }
        }
        else if (0 == s32Ret)
        {
            gpsErrorCount++;
            if (gpsErrorCount > 10)
            {
                gpsErrorCount = 0;
                LOG_Debug("gps timeout error 10 times ");
                break;
            }
        }
        else
        {
            LOG_Debug("select err.");
            break;
        }
    }

    return RET_OK;
}

int32_t HalGnssInit(GnssLocationInfoCallBack locaCb, GnssNmeaStringCallBack nmeaCb)
{
    int err = QL_ERR_OK;
    if (HAL_GNSS_INITIALED == gGnssPriv.initialed)
    {
        // have initialed, return.
        LOG_Debug("%s have initialed.", __FUNCTION__);
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

    if(1 == gGnssPriv.moduleType)
    {
        gGnssPriv.gnssModuleFd = open(gnssDevPath, O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (gGnssPriv.gnssModuleFd < 0)
        {
            perror("open gps:");
            LOG_Error("open gps error:%s", strerror(errno));
            return RET_ERR;
        }
        // 配置串口
        if (configure_serial_port(gGnssPriv.gnssModuleFd) != 0)
        {
            LOG_Error("configure_serial_port error");
            close(gGnssPriv.gnssModuleFd);
            return RET_ERR;
        }
        g_thread_new("recvGpsData", recvGpsData, NULL);
    }
    else
    {
        err = ql_gnss_init();
        if (err == QL_ERR_OK)
        {
            LOG_Debug("ql_gnss_init Successful");
        }
        else
        {
            LOG_Debug("Failed to ql_gnss_init, err=%d", err);
        }
        err = ql_gnss_set_ind_cb(gnss_ind_cb);
        if (err != QL_ERR_OK)
        {
            LOG_Debug("Failed to ql_gnss_set_ind_cb, err=%d", err);
        }
        else
        {
            LOG_Debug("Successful");
        }
        err = ql_gnss_set_service_error_cb(gnss_service_error_cb);
        if (err == QL_ERR_OK)
        {
            LOG_Debug("Successful");
        }
        else
        {
            LOG_Debug("Failed to item_ql_gnss_set_service_error_cb, err=%d", err);
        }
    }
    gGnssPriv.initialed = HAL_GNSS_INITIALED;

    return RET_OK;
}

int32_t HalGnssStart(void)
{
    if (HAL_GNSS_STARTED == gGnssPriv.started)
    {
        LOG_Error("%s have started.", __FUNCTION__);
        return RET_ERR;
    }
    int32_t err = QL_ERR_OK;

    if (1 == gGnssPriv.moduleType)
    {
    }
    else
    {
        err = ql_gnss_start();
        if (err != QL_ERR_OK)
        {
            LOG_Debug("Failed to start gnss , err=%d", err);
        }
        else
        {
            LOG_Debug("ql_gnss_start Successful");
        }
        err = ql_gnss_set_constellation(BDS_ONLY); // GPS_BDS_GLONASS:输出GNRMC和GNGGA
        if (err == QL_ERR_OK)
        {
            LOG_Debug("Successful");
        }
        else
        {
            LOG_Debug("Failed to ql_gnss_set_constellation, err=%d", err);
        }
        err = ql_gnss_set_nmea_version(QL_GNSS_NMEA_VERSION_V41);
        if (err == QL_ERR_OK)
        {
            LOG_Debug("Successful");
        }
        else
        {
            LOG_Debug("Failed to ql_gnss_set_nmea_version, err=%d", err);
        }
        // 按照CER的文档，应该是0xFF全部打开，但是CET的文档没有，ql_gnss.h头文件也没有详细说明 先这样写，后续实测
        err = ql_gnss_set_nmea_type(0xFF);
        if (err == QL_ERR_OK)
        {
            LOG_Debug("Successful");
        }
        else
        {
            LOG_Debug("Failed to ql_gnss_set_nmea_type, err=%d", err);
        }
    }
    gGnssPriv.started = HAL_GNSS_STARTED;
    return err;
}

int32_t HalGnssEnd(void)
{
    if (HAL_GNSS_STARTED != gGnssPriv.started)
    {
        LOG_Error("%s have not started.", __FUNCTION__);
        return RET_ERR;
    }
    int32_t err = QL_ERR_OK - 1;
    if (1 == gGnssPriv.moduleType)
    {
    }
    else
    {

        err = ql_gnss_stop();
        if (err != QL_ERR_OK)
        {
            LOG_Debug("Failed to stop gnss, err=%d", err);
        }
        else
        {
            LOG_Debug(" ql_gnss_stop Successful");
        }
    }
    gGnssPriv.started = HAL_GNSS_UNSTARTED;
    return err;
}

int32_t HalGnssDeInit(void)
{
    LOG_Debug("HalGnssDeInit start");
    if (HAL_GNSS_INITIALED != gGnssPriv.initialed)
    {
        LOG_Error("%s have not started.", __FUNCTION__);
        return RET_ERR;
    }
    int32_t err = QL_ERR_OK - 1;
    if (1 == gGnssPriv.moduleType)
    {
        if (gGnssPriv.gnssModuleFd >= 0)
        {
            close(gGnssPriv.gnssModuleFd);
            gGnssPriv.gnssModuleFd = -1;
        }
    }
    else
    {
        err = ql_gnss_deinit();
        if (err != QL_ERR_OK)
        {
            LOG_Debug("Failed to ql_gnss_deinit, err=%d", err);
        }
        else
        {
            LOG_Debug(" ql_gnss_deinit Successful");
        }
    }
    gGnssPriv.initialed = HAL_GNSS_UNINITIALED;
    return err;
}

void HalGnssGetLocationInfo(HalGnssLocationInfo_t *data)
{
    memmove(data, &gGnssPriv.locationInfo, sizeof(HalGnssLocationInfo_t));
}

void HalGnssGetSvStatus(HalGnssSvStatus_t *data)
{
    memmove(data, &gGnssPriv.SvStatus, sizeof(HalGnssSvStatus_t));
}

//data:0: 正常 1：电线断路 2：电线地短路 3：电线电源短路 99：获取失败
void HalGnssGetAntStatus(HalGnssAntStatus_e *data)
{
    
    int32_t adc_value;
    // on failure, the return value is -1;
    adc_value = ql_adc_show(ADC1);
    //实测 卡文CET板子 插天线ADC1799，不插天线ADC2184或者2188
    //五菱用定位模组板子，不插天线3022，插天线2840或2844
    LOG_Debug("gnss ant adc_value:%d",adc_value);
    /**
    ADC0 : 0V-VBAT_BB
    ADC1 : 0V-VBAT_BB
     */
    if (adc_value == -1)
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

int HalGnssGetNmea(void *nmea, size_t len)
{
    int ret = 0;
    nmeasrc_sentences nmea_str;

    do
    {
        ret = ql_gnss_qgpsgnmea(&nmea_str);
        if (ret != 0)
        {
            LOG_Debug("ql_gnss_qgpsgnmea failed, ret=%d", ret);
            ret = -1;
            break;
        }
        LOG_Debug("ql_gnss_qgpsgnmea Successful");

        if (sizeof(nmeasrc_sentences) > len)
        {
            LOG_Debug("nmea_str.nmea_len > len");
            ret = -1;
            break;
        }
        memmove(nmea, &nmea_str, sizeof(nmeasrc_sentences));
    } while (0);

    return ret;
}

int HalInjectAgnssData()
{
    int ret = RET_OK;
    do
    {
        ret = ql_gnss_inject_agnss_data();
        if (ret != QL_ERR_OK)
        {
            LOG_Error("ql_gnss_inject_agnss_data failed, ret=%d", ret);
            ret = RET_ERR;
            break;
        }
    } while (0);

    return ret;    
}

int HalInjectLocalAgnssData()
{
    int ret = RET_OK;
    do
    {
        ret = ql_gnss_inject_local_agnss_data();
        if (ret != QL_ERR_OK)
        {
            LOG_Error("ql_gnss_inject_local_agnss_data failed, ret=%d", ret);
            ret = RET_ERR;
            break;
        }
    } while (0);

    return ret;    
}
