#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "ql_oe.h"
#include "sr_log.h"
#include "sr_hal_gnss.h"
#include "ql_mcm_gps.h"
#include "ql_adc.h"
#include "glib.h"

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

static void HalGnssMsgCallback(loc_client_handle_type h_loc,
                                 E_QL_LOC_NFY_MSG_ID_T e_msg_id,
                                 void *pv_data,
                                 void *contextPtr)
{
    // LOG_Debug("HalGnssMsgCallback:%d", (int)e_msg_id);
    //   QL_USER_LOG("e_msg_id=%d\n", e_msg_id);
    switch (e_msg_id)
    {
    // GNSS 定位状态；
    /*  E_QL_LOC_STATUS_NONE 未知状态
        E_QL_LOC_STATUS_SESSION_BEGIN GNSS 开始定位
        E_QL_LOC_STATUS_SESSION_END GNSS 停止定位
        E_QL_LOC_STATUS_ENGINE_ON GNSS 引擎开启
        E_QL_LOC_STATUS_ENGINE_OFF GNSS 引擎关闭*/
    case E_QL_LOC_NFY_MSG_ID_STATUS_INFO:
    {
        LOG_Debug("recv E_QL_LOC_NFY_MSG_ID_STATUS_INFO:%d", *(int32_t *)pv_data);
        break;
    }
    // GNSS 定位信息；
    /*  uint32_t size 结构体大小

        E_QL_LOC_LOCATION_VALID_FLAG flags 数据有效性指示。掩码为 1 指示当前结构体内数据有效，为 0 则表示数据无效。
        取值为下列掩码的组合：
        E_QL_LOC_LOCATION_LAT_LONG_VALID：经纬度
        E_QL_LOC_LOCATION_ALTITUDE_VALID：海拔
        E_QL_LOC_LOCATION_SPEED_VALID：速度
        E_QL_LOC_LOCATION_BEARING_VALID：航向
        E_QL_LOC_LOCATION_ACCURACY_VALID：定位精度
        E_QL_LOC_LOCATION_SOURCE_INFO_VALID：预留
        E_QL_LOC_LOCATION_IS_INDOOR_VALID：预留
        E_QL_LOC_LOCATION_FLOOR_NUMBE_VALID：预留
        E_QL_LOC_LOCATION_MAP_URL_VALID：预留
        E_QL_LOC_LOCATION_MAP_INDEX_VALID：预留

        E_QL_LOC_ULP_LOCATION_SOURCE position_source 定位数据来源。
        E_QL_LOC_ULP_LOCATION_SOURCE_HYBRID：混合定位数据
        E_QL_LOC_ULP_LOCATION_SOURCE_GNSS：GNSS定位数据

        double latitude 经度。范围：-90~90；单位：度。
        double longitude 纬度。范围：0~180；单位：度。
        double altitude 高度。单位：米。
        float speed 速度。范围：0~540；单位：米/秒。
        float bearing 航向。范围：0~360；单位：度。
        float accuracy 水平精度。单位：米。
        int64_t timestamp UTC 时间。单位：毫秒。
        int32_t Is_indoor 是否在室内（不支持）
        float floor_number 楼层（不支持）
        int32_t raw_data_len 原始观测量数据长度。范围：0~256。（不支持）
        uint8_t raw_data 原始观测量数据（不支持）
        char map_url 预留
        uint8_t map_index 预留
    */
    case E_QL_LOC_NFY_MSG_ID_LOCATION_INFO:
    {
        const QL_LOC_LOCATION_INFO_T *pt_location = (QL_LOC_LOCATION_INFO_T *)pv_data;
        LOG_Debug("recv E_QL_LOC_NFY_MSG_ID_LOCATION_INFO:size=%d, flag=0x%X, position_source=%d, Latitude = %f, Longitude=%f, altitude = % f,speed = % f, bearing = % f, accuracy = % f, timestamp = % lld, is_indoor = % d, floor_number = % f,raw_data_len = % d ",
               pt_location->size,
               pt_location->flags,
               pt_location->position_source,
               pt_location->latitude,
               pt_location->longitude,
               pt_location->altitude,
               pt_location->speed,
               pt_location->bearing,
               pt_location->accuracy,
               pt_location->timestamp,
               pt_location->is_indoor,
               pt_location->floor_number,
               pt_location->raw_data_len);

        gGnssPriv.locationInfo.accuracy = pt_location->accuracy;
        break;
    }
    // 卫星状态；
    /*  uint32_t size 结构体大小
        int num_svs 可见卫星数量

        QL_LOC_SV_INFO_T sv_list 卫星数量及卫星基本数据。卫星数量范围：0~32。卫星基本数据如下。
        uint32_t size 结构体大小
        int prn 卫星 ID
        float snr 信噪比。范围：0~99。
        float elevation 卫星仰角。范围：0~90；单位：度。
        float azimuth 卫星方位角。范围：0~360；单位：度。

        uint32_t ephemeris_mask 星历有效掩码，指示卫星是否有有效星历。
        uint32_t almanac_mask 历书有效掩码，指示卫星是否有有效历书。
        uint32_t used_in_fix_mask 参与定位掩码，指示卫星是否参与定位（不支持）。*/
    case E_QL_LOC_NFY_MSG_ID_SV_INFO:
    {
        memset(&gGnssPriv.SvStatus, 0x0, sizeof(HalGnssSvStatus_t));
        const QL_LOC_SV_STATUS_T *data = (QL_LOC_SV_STATUS_T *)pv_data;
        LOG_Debug("recv E_QL_LOC_NFY_MSG_ID_SV_INFO:size:%d,num_svs:%d,ephemeris_mask:%d,almanac_mask:%d,used_in_fix_mask:%d", data->size, data->num_svs, data->ephemeris_mask, data->almanac_mask, data->used_in_fix_mask);
        // gGnssPriv.SvStatus.almanac_mask = data->almanac_mask;
        // gGnssPriv.SvStatus.ephemeris_mask = data->ephemeris_mask;
        // gGnssPriv.SvStatus.num_svs = data->num_svs;
        // for (int i = 0; i < data->num_svs; i++)
        // {
        //     LOG_Debug("recv E_QL_LOC_NFY_MSG_ID_SV_INFO sv%d:size:%d,prn:%d,snr:%f,elevation:%f,azimuth:%f",
        //            i, data->sv_list[i].size, data->sv_list[i].prn, data->sv_list[i].snr,
        //            data->sv_list[i].elevation, data->sv_list[i].azimuth);
        //     gGnssPriv.SvStatus.sv_list[i].azimuth = data->sv_list[i].azimuth;
        //     gGnssPriv.SvStatus.sv_list[i].elevation = data->sv_list[i].elevation;
        //     gGnssPriv.SvStatus.sv_list[i].prn = data->sv_list[i].prn;
        //     gGnssPriv.SvStatus.sv_list[i].snr = data->sv_list[i].snr;
        // }
        // if (gGnssPriv.SvCallCb)
        // {
        //     gGnssPriv.SvCallCb(gGnssPriv.SvStatus);
        // }
        gGnssPriv.locationInfo.num_svs = data->num_svs;
        break;
    }
    case E_QL_LOC_NFY_MSG_ID_NMEA_INFO:
    {
        HalGnssNmea_t nmeaMsg = {0};
        QL_LOC_NMEA_INFO_T *pt_nmea = (QL_LOC_NMEA_INFO_T *)pv_data;
        // LOG_Debug("recv E_QL_LOC_NFY_MSG_ID_NMEA_INFO info: timestamp=%lld, length=%d, nmea=%s",
        //        pt_nmea->timestamp, pt_nmea->length, pt_nmea->nmea);
        nmeaMsg.length = pt_nmea->length;
        nmeaMsg.timestamp = pt_nmea->timestamp;
        memmove(nmeaMsg.nmea, pt_nmea->nmea, pt_nmea->length);
        gchar **gps_data = g_strsplit(nmeaMsg.nmea, ",", -1);

        if (gps_data[0] != NULL)
        {
            if (strcmp(gps_data[0], "$GPRMC") == 0)
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
            else if (strcmp(gps_data[0], "$GPGGA") == 0)
            {
                gGnssPriv.locationInfo.gpsState = strtol(gps_data[6], NULL, 10);
                gGnssPriv.locationInfo.sumNosv = strtol(gps_data[7], NULL, 10);
                gGnssPriv.locationInfo.height = strtod(gps_data[9], NULL);
            }
            else
            {
                g_strfreev(gps_data);
                break;
            }
            //NMEA语句：GPGSV GLGSV GAGSV PQGSV GPGSA GLGSA GAGSA PQGSA GPVTG GPDTM GPRMC GPGGA
            //只处理GPGGA GPRMC 其他不处理
            if (gGnssPriv.NmeaCallCb)
            {
                gGnssPriv.NmeaCallCb(&nmeaMsg);
            }
        }
        g_strfreev(gps_data);
        break;
    }
    // GNSS 定位能力（不支持）
    case E_QL_LOC_NFY_MSG_ID_CAPABILITIES_INFO:
        break;
    // AGPS 辅助定位信息（不支持）
    case E_QL_LOC_NFY_MSG_ID_AGPS_STATUS:
        break;
    // 事件通知（不支持）
    case E_QL_LOC_NFY_MSG_ID_NI_NOTIFICATION:
        break;
    // XTRA 服务器地址（不支持）
    case E_QL_LOC_NFY_MSG_ID_XTRA_REPORT_SERVER:
        break;
    }
}

int32_t HalGnssInit(GnssLocationInfoCallBack locaCb, GnssNmeaStringCallBack nmeaCb)
{
    int32_t ret = E_QL_OK;
    QL_LOC_POS_MODE_INFO_T t_mode = {0};
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
    // if(svcb != NULL)
    // {
    //     gGnssPriv.SvCallCb = svcb;
    // }
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

    // 初始化 GNSS 客户端
    ret = QL_LOC_Client_Init((loc_client_handle_type *)&gGnssPriv.h_loc);
    LOG_Debug("QL_LOC_Client_Init ret %d with h_loc=%d", ret, gGnssPriv.h_loc);

    // 注册 GNSS 数据上报回调函数
    ret = QL_LOC_AddRxIndMsgHandler(HalGnssMsgCallback, (void *)gGnssPriv.h_loc);
    LOG_Debug("QL_LOC_AddRxIndMsgHandler ret %d", ret);

    int32_t bitmask = 0x0f;
    if (((uint64_t)time(NULL) * 1000) > 1696089600000)
    {
        bitmask = bitmask | LOC_IND_UTC_TIME_REQ_ON;
    }
    // 设置待回调数据的标志位 bitmask：
    /*  LOC_IND_LOCATION_INFO_ON (1 << 0) //定位数据
        LOC_IND_STATUS_INFO_ON (1 << 1) //GNSS 引擎状态数据
        LOC_IND_SV_INFO_ON (1 << 2) //卫星数据
        LOC_IND_NMEA_INFO_ON (1 << 3) //NMEA 语句
        LOC_IND_CAP_INFO_ON (1 << 4) //不支持
        LOC_IND_UTC_TIME_REQ_ON (1 << 5) //请求注入 UTC 时间
        LOC_IND_XTRA_DATA_REQ_ON (1 << 6) //请求注入 gpsOneXTRA 数据
        LOC_IND_AGPS_DATA_CONN_CMD_REQ_ON (1 << 7) //不支持
        LOC_IND_NI_NFY_USER_RESP_REQ_ON (1 << 8) //不支持*/
    ret = QL_LOC_Set_Indications(gGnssPriv.h_loc, bitmask);
    LOG_Debug("QL_LOC_Set_Indications ret %d", ret);

    /*  定位模式。
        E_QL_LOC_POS_MODE_STANDALONE：独立模式
        E_QL_LOC_POS_MODE_MS_BASED：MSB 模式（可加速定位）
        E_QL_LOC_POS_MODE_MS_ASSISTED：MSA 模式（不支持）*/
    t_mode.mode = E_QL_LOC_POS_MODE_STANDALONE;
    /*  定位循环模式。
        E_QL_LOC_POS_RECURRENCE_PERIODIC：循环定位
        E_QL_LOC_POS_RECURRENCE_SINGLE：单次定位*/
    t_mode.recurrence = E_QL_LOC_POS_RECURRENCE_PERIODIC;
    /*  定位间隔。单位：毫秒。有效取值：100、200、500、1000 以及其他 1000 的整数倍。*/
    t_mode.min_interval = 1000;     // report nmea frequency 1Hz
    t_mode.preferred_accuracy = 50; // <50m 水平定位精度，30m以上，设置的越小定位时间越长
    t_mode.preferred_time = 200;    // demo设置为了90s 冷启动可能需要100s以上，热启动可能需要2s
    // 设置定位配置项
    ret = QL_LOC_Set_Position_Mode(gGnssPriv.h_loc, &t_mode);
    LOG_Debug("QL_LOC_Set_Position_Mode ret %d", ret);

    if (bitmask & LOC_IND_UTC_TIME_REQ_ON)
    {
        QL_LOC_INJECT_TIME_INTO_T pt_info;
        // 当前 UTC 时间，即从 UTC 时间 1970 年 1 月 1 日 0 时 0 分 0 秒起至现在的总毫秒数。
        pt_info.time = (uint64_t)time(NULL) * 1000;
        // 无效参数；默认设置为 0。
        pt_info.time_reference = 0;
        // 时间精度；设置为 3500。单位：毫秒。
        pt_info.uncertainty = 3500;
        // 注入 UTC 时间到 GNSS 引擎 该函数用于注入 UTC 时间到 GNSS 引擎。应在调用 QL_LOC_Start_Navigation()启动 GNSS 前调用该函数注入 UTC 时间，以加速定位。
        ret = QL_LOC_InjectTime(gGnssPriv.h_loc, &pt_info);
        LOG_Debug("QL_LOC_InjectTime ret %d,time=%lld", ret, pt_info.time);
    }

    // 注入 gpsOneXTRA 文件到 GNSS 引擎 通过注入 gpsOneXTRA 数据可缩短 10~18 秒定位时间，使平均 TTFF 缩短至 18~30 秒。
    // 如果需要用 需要打开LOC_IND_XTRA_DATA_REQ_ON标志
    // ret = QL_LOC_Xtra_InjectFile(gGnssPriv.h_loc, "xtra2.bin");
    // LOG_Debug("QL_LOC_Xtra_InjectFile ret %d,", ret);
    // sleep(3);
    // uint64_t startTimeUtc;
    // uint16_t durationHour;
    // // 查询已注入 gpsOneXTRA 数据的有效期
    // ret = QL_LOC_Xtra_GetValidity(gGnssPriv.h_loc, &startTimeUtc, &durationHour);
    // LOG_Debug("QL_LOC_InjectTime ret %d,startTimeUtc=%lld, durationHour=%d",
    //        ret, startTimeUtc, durationHour);

    ret = QL_LOC_Set_Nmea_Type(gGnssPriv.h_loc, E_QL_LOC_NMEA_MASK_GGA |
                                                    E_QL_LOC_NMEA_MASK_RMC |
                                                    E_QL_LOC_NMEA_MASK_GSV |
                                                    E_QL_LOC_NMEA_MASK_GSA);
    LOG_Debug("QL_LOC_Set_Nmea_Type ret %d", ret);

    uint32_t nmeaType;
    ret = QL_LOC_Get_Nmea_Type(gGnssPriv.h_loc, &nmeaType);
    LOG_Debug("QL_LOC_Set_Nmea_Type ret %d, nmeaType=%x", ret, nmeaType);

    // 设置 SUPL 服务器地址及端口
    // 安全用户平面定位（SUPL）是使用用户层数据承载来传输定位辅助信息（如星历、时间等），并在移动终端和网络之间携带定位技术的协议，用于加快 TTFF。
    // 移远通信 AG35 系列模块支持 SUPL 2.0 版本。
    // 若实际应用场景中无 AGPS 服务器地址及端口信息，请勿调用 QL_LOC_Agps_SetServer()，否则可能会导致 GNSS 定位功能异常。
    // QL_LOC_AGPS_SERVER_INTO_T agps_server;
    // agps_server.e_agps_type = E_QL_LOC_AGPS_TYPE_SUPL;
    // strcpy(agps_server.host_name, "supl-test.qxwz.com");
    // agps_server.port = 7275;
    // ret = QL_LOC_Agps_SetServer(h_loc, &agps_server);
    // LOG_Debug("QL_LOC_Agps_SetServer ret %d", ret);
    // ret = QL_LOC_Install_AgpsCert(h_loc, "123456789", 10);
    // LOG_Debug("QL_LOC_Install_AgpsCert ret %d", ret);
    // sleep(2);
    // ret = QL_LOC_Uninstall_AgpsCert(h_loc);
    // LOG_Debug("QL_LOC_Uninstall_AgpsCert ret %d", ret);

    ret = QL_LOC_Set_Gnss_Config_Type(gGnssPriv.h_loc, E_QL_LOC_GNSS_CONFIG_GLO | E_QL_LOC_GNSS_CONFIG_BDS);
    LOG_Debug("QL_LOC_Set_Gnss_Config_Type ret %d", ret);

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
    // 启动 GNSS
    int32_t ret = QL_LOC_Start_Navigation(gGnssPriv.h_loc);
    LOG_Debug("QL_LOC_Start_Navigation ret=%d", ret);
    gGnssPriv.started = HAL_GNSS_STARTED;
    return ret;
}

int32_t HalGnssEnd(void)
{
    if (HAL_GNSS_STARTED != gGnssPriv.started)
    {
        LOG_Error("%s have not started.", __FUNCTION__);
        return RET_ERR;
    }
    // 关闭 GNSS
    int32_t ret = QL_LOC_Stop_Navigation(gGnssPriv.h_loc);
    LOG_Debug("QL_LOC_Stop_Navigation ret=%d", ret);
    gGnssPriv.started = HAL_GNSS_UNSTARTED;
    return ret;
}

int32_t HalGnssDeInit(void)
{
    LOG_Debug("HalGnssDeInit start");
    if (HAL_GNSS_INITIALED != gGnssPriv.initialed)
    {
        LOG_Error("%s have not started.", __FUNCTION__);
        return RET_ERR;
    }
    // 注销 GNSS 客户端
    int32_t ret = QL_LOC_Client_Deinit(gGnssPriv.h_loc);
    LOG_Debug("QL_LOC_Client_Deinit ret=%d", ret);
    gGnssPriv.initialed = HAL_GNSS_UNINITIALED;
    return ret;
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
    // ADC0、ADC1、CBC、ADC2、 GPS天线的ADC检测接口是ADC1
    // adc value(mv) on success
    // on failure, the return value is -1;
    adc_value = ql_adc_show(ADC1);
    LOG_Debug("gnss ant adc_value:%d",adc_value);
    // 20231206与硬件沟通，开路3.297，正常3.11，地短路0，天线与电源短路3.33，实际根据天线参数后续可能会有变化
    // 20240111 基线4G板子实测 不插天线3357-3358mv 插天线2667-2668mv
    // 20240806 卡文evd板子 不插天线2178mv 插天线1995mv
    /**
     * ADC0 : 0.3V-VBAT_BB(3.8V)
     * ADC1 : 0.3V-VBAT_BB
     * ADC2 : 0.1V-1.7V
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
