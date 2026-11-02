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
#include "ql-location/ql_gnss.h"
#include "ql_adc.h"

#define ZETA 57.29578f
static ql_gnss_report_callback_t g_gnssReportCb = {0}; // must init  callback funtion be NULL
int start_mode = QL_GNSS_HOT_START;
int fix_mode = QL_GNSS_SUPL_MODE_STANDALONE;
int interval = 1000;
int nmea_type = QL_NMEA_TYPE_BITMASK_ALL;
ql_gnss_robust_loc_conf_t robust = {
    .enabled = 1,
    .enabledForE911 = 1
};
int constellation = QL_GNSS_CONSTELLATION_GLO_BIT |
                    QL_GNSS_CONSTELLATION_BDS_BIT |
                    QL_GNSS_CONSTELLATION_QZSS_BIT |
                    QL_GNSS_CONSTELLATION_GAL_BIT |
                    QL_GNSS_CONSTELLATION_NAVIC_BIT;
int eng_type = 7;
int cbinf = 15;

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


static void gnssLoctionCb(const ql_gnss_location_t location)
{
    ql_gnss_system_time_type_t ggbqSystemTime;
    // basic location info
    LOG_Debug("$BSLOC,%ld,%X,%.8lf,%.8lf,%.2lf,%.2f,%.1f,%.3f,%.2f,%.2f,%.1f,%f,%d,%d,%d,%d,%X,%d",
           location.timestamp,
           location.flags,
           location.latitude,
           location.longitude,
           location.altitude,
           location.speed,
           location.bearing,
           location.horizontalAccuracy,
           location.verticalAccuracy,
           location.speedAccuracy,
           location.bearingAccuracy,
           location.timeUncMs,
           location.horReliability,
           location.verReliability,
           location.numSvUsedInPosition,
           location.techMask,
           location.navSolutionMask,
           location.posTechMask);

    LOG_Debug("$ENUIF,%ld,%.8lf,%.8lf,%.2f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f",
           location.timestamp,
           location.llaVRPBased.latitude, // Vehicle Reference Point positon
           location.llaVRPBased.longitude,
           location.llaVRPBased.altitude,
           location.eastStdDeviation,
           location.northStdDeviation,
           location.eastVelocity,
           location.northVelocity,
           location.upVelocity,
           location.eastVelocityStdDeviation,
           location.northVelocityStdDeviation,
           location.upVelocityStdDeviation,
           location.enuVelocityVRPBased[0], // Vehicle Reference Point Velocity
           location.enuVelocityVRPBased[1],
           location.enuVelocityVRPBased[2]);

    LOG_Debug("$BDFRM,%ld,%X,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f",
           location.timestamp,
           location.bodyFrameData.bodyFrameDataMask,
           location.bodyFrameData.longAccel,
           location.bodyFrameData.latAccel,
           location.bodyFrameData.vertAccel,
           location.bodyFrameData.longAccelUnc,
           location.bodyFrameData.latAccelUnc,
           location.bodyFrameData.vertAccelUnc,
           location.bodyFrameData.pitch * ZETA,
           location.bodyFrameData.roll * ZETA,
           location.bodyFrameData.yaw * ZETA,
           location.bodyFrameData.pitchUnc * ZETA,
           location.bodyFrameData.rollUnc * ZETA,
           location.bodyFrameData.yawUnc * ZETA,
           location.bodyFrameData.pitchRate * ZETA,
           location.bodyFrameData.rollRate * ZETA,
           location.bodyFrameData.yawRate * ZETA,
           location.bodyFrameData.pitchRateUnc * ZETA,
           location.bodyFrameData.rollRateUnc * ZETA,
           location.bodyFrameData.yawRateUnc * ZETA);

    LOG_Debug("$DRINF,%ld,%d,%X,%X,%X,%X",
           location.timestamp,
           location.calibrationConfidencePercent,
           location.calibrationStatus,
           location.drSolutionStatusMask,
           location.locOutputEngType,
           location.locOutputEngMask);

    LOG_Debug("$DOPIF,%ld,%.1f,%.1f,%.1f,%.f,%.1f",
           location.timestamp,
           location.pdop,
           location.hdop,
           location.vdop,
           location.gdop,
           location.tdop);

    LOG_Debug("$GEOIF,%ld,%.2f,%.2f,%.3f,%.3f,%.3f",
           location.timestamp,
           location.altitudeMeanSeaLevel,
           location.magneticDeviation,
           location.horUncEllipseSemiMajor,
           location.horUncEllipseSemiMinor,
           location.horUncEllipseOrientAzimuth);

    // gnss system time
    if (location.gnssSystemTime.gnssSystemTimeSrc == QL_GNSS_LOC_SV_SYSTEM_GLONASS)
    {

        LOG_Debug("$STIME,%ld,%d,%X,%d,%d,%d,%f,%f,%d,%d,%d",
               location.timestamp,
               location.gnssSystemTime.gnssSystemTimeSrc,
               location.gnssSystemTime.u.gloSystemTime.validityMask,
               location.gnssSystemTime.u.gloSystemTime.gloFourYear,
               location.gnssSystemTime.u.gloSystemTime.gloDays,
               location.gnssSystemTime.u.gloSystemTime.gloMsec,
               location.gnssSystemTime.u.gloSystemTime.gloClkTimeBias,
               location.gnssSystemTime.u.gloSystemTime.gloClkTimeUncMs,
               location.gnssSystemTime.u.gloSystemTime.refFCount,
               location.gnssSystemTime.u.gloSystemTime.numClockResets,
               location.leapSeconds);
    }
    else
    {
        memcpy(&ggbqSystemTime, &location.gnssSystemTime.u, sizeof(ql_gnss_system_time_type_t));
        LOG_Debug("$STIME,%ld,%d,%X,%d,%d,%f,%f,%d,%d,%d",
               location.timestamp,
               location.gnssSystemTime.gnssSystemTimeSrc,
               ggbqSystemTime.validityMask,
               ggbqSystemTime.systemWeek,
               ggbqSystemTime.systemMsec,
               ggbqSystemTime.systemClkTimeBias,
               ggbqSystemTime.systemClkTimeUncMs,
               ggbqSystemTime.refFCount,
               ggbqSystemTime.numClockResets,
               location.leapSeconds);
    }
}

static void SPELoctionCb(const ql_gnss_location_t location)
{
    ql_gnss_system_time_type_t ggbqSystemTime;
    // basic location info
    LOG_Debug("$SPEBSLOC,%ld,%X,%.8lf,%.8lf,%.2lf,%.2f,%.1f,%.3f,%.2f,%.2f,%.1f,%f,%d,%d,%d,%d,%X,%d",
           location.timestamp,
           location.flags,
           location.latitude,
           location.longitude,
           location.altitude,
           location.speed,
           location.bearing,
           location.horizontalAccuracy,
           location.verticalAccuracy,
           location.speedAccuracy,
           location.bearingAccuracy,
           location.timeUncMs,
           location.horReliability,
           location.verReliability,
           location.numSvUsedInPosition,
           location.techMask,
           location.navSolutionMask,
           location.posTechMask);

    LOG_Debug("$SPEENUIF,%ld,%.8lf,%.8lf,%.2f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f",
           location.timestamp,
           location.llaVRPBased.latitude, // Vehicle Reference Point positon
           location.llaVRPBased.longitude,
           location.llaVRPBased.altitude,
           location.eastStdDeviation,
           location.northStdDeviation,
           location.eastVelocity,
           location.northVelocity,
           location.upVelocity,
           location.eastVelocityStdDeviation,
           location.northVelocityStdDeviation,
           location.upVelocityStdDeviation,
           location.enuVelocityVRPBased[0], // Vehicle Reference Point Velocity
           location.enuVelocityVRPBased[1],
           location.enuVelocityVRPBased[2]);

    LOG_Debug("$SPEDOPIF,%ld,%.1f,%.1f,%.1f,%.f,%.1f",
           location.timestamp,
           location.pdop,
           location.hdop,
           location.vdop,
           location.gdop,
           location.tdop);

    LOG_Debug("$SPEGEOIF,%ld,%.2f,%.2f,%.3f,%.3f,%.3f",
           location.timestamp,
           location.altitudeMeanSeaLevel,
           location.magneticDeviation,
           location.horUncEllipseSemiMajor,
           location.horUncEllipseSemiMinor,
           location.horUncEllipseOrientAzimuth);

    // gnss system time
    if (location.gnssSystemTime.gnssSystemTimeSrc == QL_GNSS_LOC_SV_SYSTEM_GLONASS)
    {

        LOG_Debug("$SPESTIME,%ld,%d,%X,%d,%d,%d,%f,%f,%d,%d,%d",
               location.timestamp,
               location.gnssSystemTime.gnssSystemTimeSrc,
               location.gnssSystemTime.u.gloSystemTime.validityMask,
               location.gnssSystemTime.u.gloSystemTime.gloFourYear,
               location.gnssSystemTime.u.gloSystemTime.gloDays,
               location.gnssSystemTime.u.gloSystemTime.gloMsec,
               location.gnssSystemTime.u.gloSystemTime.gloClkTimeBias,
               location.gnssSystemTime.u.gloSystemTime.gloClkTimeUncMs,
               location.gnssSystemTime.u.gloSystemTime.refFCount,
               location.gnssSystemTime.u.gloSystemTime.numClockResets,
               location.leapSeconds);
    }
    else
    {
        memcpy(&ggbqSystemTime, &location.gnssSystemTime.u, sizeof(ql_gnss_system_time_type_t));
        LOG_Debug("$SPESTIME,%ld,%d,%X,%d,%d,%f,%f,%d,%d,%d",
               location.timestamp,
               location.gnssSystemTime.gnssSystemTimeSrc,
               ggbqSystemTime.validityMask,
               ggbqSystemTime.systemWeek,
               ggbqSystemTime.systemMsec,
               ggbqSystemTime.systemClkTimeBias,
               ggbqSystemTime.systemClkTimeUncMs,
               ggbqSystemTime.refFCount,
               ggbqSystemTime.numClockResets,
               location.leapSeconds);
    }
}

static void PPELoctionCb(const ql_gnss_location_t location)
{
    ql_gnss_system_time_type_t ggbqSystemTime;
    // basic location info
    LOG_Debug("$PPEBSLOC,%ld,%X,%.8lf,%.8lf,%.2lf,%.2f,%.1f,%.3f,%.2f,%.2f,%.1f,%f,%d,%d,%d,%d,%X,%d",
           location.timestamp,
           location.flags,
           location.latitude,
           location.longitude,
           location.altitude,
           location.speed,
           location.bearing,
           location.horizontalAccuracy,
           location.verticalAccuracy,
           location.speedAccuracy,
           location.bearingAccuracy,
           location.timeUncMs,
           location.horReliability,
           location.verReliability,
           location.numSvUsedInPosition,
           location.techMask,
           location.navSolutionMask,
           location.posTechMask);

    LOG_Debug("$PPEENUIF,%ld,%.8lf,%.8lf,%.2f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f",
           location.timestamp,
           location.llaVRPBased.latitude, // Vehicle Reference Point positon
           location.llaVRPBased.longitude,
           location.llaVRPBased.altitude,
           location.eastStdDeviation,
           location.northStdDeviation,
           location.eastVelocity,
           location.northVelocity,
           location.upVelocity,
           location.eastVelocityStdDeviation,
           location.northVelocityStdDeviation,
           location.upVelocityStdDeviation,
           location.enuVelocityVRPBased[0], // Vehicle Reference Point Velocity
           location.enuVelocityVRPBased[1],
           location.enuVelocityVRPBased[2]);

    LOG_Debug("$PPEDOPIF,%ld,%.1f,%.1f,%.1f,%.f,%.1f",
           location.timestamp,
           location.pdop,
           location.hdop,
           location.vdop,
           location.gdop,
           location.tdop);

    LOG_Debug("$PPEGEOIF,%ld,%.2f,%.2f,%.3f,%.3f,%.3f",
           location.timestamp,
           location.altitudeMeanSeaLevel,
           location.magneticDeviation,
           location.horUncEllipseSemiMajor,
           location.horUncEllipseSemiMinor,
           location.horUncEllipseOrientAzimuth);

    // gnss system time
    if (location.gnssSystemTime.gnssSystemTimeSrc == QL_GNSS_LOC_SV_SYSTEM_GLONASS)
    {

        LOG_Debug("$PPESTIME,%ld,%d,%X,%d,%d,%d,%f,%f,%d,%d,%d",
               location.timestamp,
               location.gnssSystemTime.gnssSystemTimeSrc,
               location.gnssSystemTime.u.gloSystemTime.validityMask,
               location.gnssSystemTime.u.gloSystemTime.gloFourYear,
               location.gnssSystemTime.u.gloSystemTime.gloDays,
               location.gnssSystemTime.u.gloSystemTime.gloMsec,
               location.gnssSystemTime.u.gloSystemTime.gloClkTimeBias,
               location.gnssSystemTime.u.gloSystemTime.gloClkTimeUncMs,
               location.gnssSystemTime.u.gloSystemTime.refFCount,
               location.gnssSystemTime.u.gloSystemTime.numClockResets,
               location.leapSeconds);
    }
    else
    {
        memcpy(&ggbqSystemTime, &location.gnssSystemTime.u, sizeof(ql_gnss_system_time_type_t));
        LOG_Debug("$PPESTIME,%ld,%d,%X,%d,%d,%f,%f,%d,%d,%d",
               location.timestamp,
               location.gnssSystemTime.gnssSystemTimeSrc,
               ggbqSystemTime.validityMask,
               ggbqSystemTime.systemWeek,
               ggbqSystemTime.systemMsec,
               ggbqSystemTime.systemClkTimeBias,
               ggbqSystemTime.systemClkTimeUncMs,
               ggbqSystemTime.refFCount,
               ggbqSystemTime.numClockResets,
               location.leapSeconds);
    }
}

static void gnssSvCb(const ql_gnss_sv_t *gnss_sv, int length)
{
    for (int i = 0; i < length; i++)
    {
        LOG_Debug("$SVINF,%02d,%03d,%d,%04.1f,%04.1f,%05.1f,%04X,%12.1f,%08X,%d",
               i + 1,
               gnss_sv[i].svId,
               gnss_sv[i].type,
               gnss_sv[i].cN0Dbhz,
               gnss_sv[i].elevation,
               gnss_sv[i].azimuth,
               gnss_sv[i].gnssSvOptionsMask,
               gnss_sv[i].carrierFrequencyHz,
               gnss_sv[i].gnssSignalTypeMask,
               gnss_sv[i].gloFrequency);
    }
}

static void gnssNmeaCb(const char *nmea)
{
    LOG_Debug("recv nmea:%s", nmea);

    HalGnssNmea_t nmeaMsg = {0};
    nmeaMsg.length = strlen(nmea);
    nmeaMsg.timestamp = 0;
    memmove(nmeaMsg.nmea, nmea, nmeaMsg.length);
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
        else
        {
            g_strfreev(gps_data);
            return;
        }
        // NMEA语句：GPGSV GLGSV GAGSV PQGSV GPGSA GLGSA GAGSA PQGSA GPVTG GPDTM GPRMC GPGGA
        // 只处理GPGGA GPRMC 其他不处理
        if (gGnssPriv.NmeaCallCb)
        {
            gGnssPriv.NmeaCallCb(&nmeaMsg);
        }
    }
    g_strfreev(gps_data);
}

static void gnssDataCb(const ql_gnss_data_t gnss_data)
{
    LOG_Debug("$JAMSK,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X",
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_GPS_L1CA],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_GPS_L1C],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_GPS_L2C_L],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_GPS_L5_Q],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_GLONASS_G1],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_GLONASS_G2],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_GALILEO_E1_C],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_GALILEO_E5A_Q],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_GALILEO_E5B_Q],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_BEIDOU_B1_I],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_BEIDOU_B1C],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_BEIDOU_B2_I],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_BEIDOU_B2A_I],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_BEIDOU_B2A_Q],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_QZSS_L1CA],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_QZSS_L1S],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_QZSS_L2C_L],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_QZSS_L5_Q],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_SBAS_L1_CA],
           gnss_data.gnssDataMask[QL_GNSS_SIGNAL_TYPE_NAVIC_L5]);

    LOG_Debug("$JAMIF,%04.1lf,%04.1lf,%04.1lf,%04.1lf,%04.1lf,%04.1lf,%04.1lf,%04.1lf,%04.1lf,%04.1lf,%04.1lf,%04.1lf,%04.1lf,%04.1lf,%04.1lf,%04.1lf,%04.1lf,%04.1lf,%04.1lf,%04.1lf",
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_GPS_L1CA],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_GPS_L1C],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_GPS_L2C_L],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_GPS_L5_Q],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_GLONASS_G1],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_GLONASS_G2],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_GALILEO_E1_C],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_GALILEO_E5A_Q],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_GALILEO_E5B_Q],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_BEIDOU_B1_I],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_BEIDOU_B1C],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_BEIDOU_B2_I],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_BEIDOU_B2A_I],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_BEIDOU_B2A_Q],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_QZSS_L1CA],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_QZSS_L1S],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_QZSS_L2C_L],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_QZSS_L5_Q],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_SBAS_L1_CA],
           gnss_data.jammerInd[QL_GNSS_SIGNAL_TYPE_NAVIC_L5]);

    LOG_Debug("$AGCIF,%05.2lf,%05.2lf,%05.2lf,%05.2lf,%05.2lf,%05.2lf,%05.2lf,%05.2lf,%05.2lf,%05.2lf,%05.2lf,%05.2lf,%05.2lf,%05.2lf,%05.2lf,%05.2lf,%05.2lf,%05.2lf,%05.2lf,%05.2lf",
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_GPS_L1CA],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_GPS_L1C],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_GPS_L2C_L],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_GPS_L5_Q],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_GLONASS_G1],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_GLONASS_G2],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_GALILEO_E1_C],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_GALILEO_E5A_Q],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_GALILEO_E5B_Q],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_BEIDOU_B1_I],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_BEIDOU_B1C],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_BEIDOU_B2_I],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_BEIDOU_B2A_I],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_BEIDOU_B2A_Q],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_QZSS_L1CA],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_QZSS_L1S],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_QZSS_L2C_L],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_QZSS_L5_Q],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_SBAS_L1_CA],
           gnss_data.agc[QL_GNSS_SIGNAL_TYPE_NAVIC_L5]);
}

static void gnssMeasCb(const ql_gnss_raw_meas_t raw_meas)
{
    int i;
    LOG_Debug("$QMEAS,CLOCK,%d,%d,%ld,%lf,%ld,%lf,%lf,%lf,%lf,%d",
           raw_meas.clock.flags,
           raw_meas.clock.leapSecond,
           raw_meas.clock.timeNs,
           raw_meas.clock.timeUncertaintyNs,
           raw_meas.clock.fullBiasNs,
           raw_meas.clock.biasNs,
           raw_meas.clock.biasUncertaintyNs,
           raw_meas.clock.driftNsps,
           raw_meas.clock.driftUncertaintyNsps,
           raw_meas.clock.hwClockDiscontinuityCount);
    for (i = 0; i < raw_meas.measCount; i++)
    {
        LOG_Debug("$QMEAS,%02d,%06X,%03d,%d,%lf,%05d,%ld,%f,%ld,%03.1lf,%.3lf,%.3lf,%d,%.3lf,%.3lf,%.1f,%ld,%lf,%lf,%d,%lf,%.2lf,%d,%.3lf,%lf,%lf",
               i + 1,
               raw_meas.measurements[i].flags,
               raw_meas.measurements[i].svId,
               raw_meas.measurements[i].svType,
               raw_meas.measurements[i].timeOffsetNs,
               raw_meas.measurements[i].stateMask,
               raw_meas.measurements[i].receivedSvTimeNs,
               raw_meas.measurements[i].receivedSvTimeSubNs,
               raw_meas.measurements[i].receivedSvTimeUncertaintyNs,
               raw_meas.measurements[i].carrierToNoiseDbHz,
               raw_meas.measurements[i].pseudorangeRateMps,
               raw_meas.measurements[i].pseudorangeRateUncertaintyMps,
               raw_meas.measurements[i].adrStateMask,
               raw_meas.measurements[i].adrMeters,
               raw_meas.measurements[i].adrUncertaintyMeters,
               raw_meas.measurements[i].carrierFrequencyHz,
               raw_meas.measurements[i].carrierCycles,
               raw_meas.measurements[i].carrierPhase,
               raw_meas.measurements[i].carrierPhaseUncertainty,
               raw_meas.measurements[i].multipathIndicator,
               raw_meas.measurements[i].signalToNoiseRatioDb,
               raw_meas.measurements[i].agcLevelDb,
               raw_meas.measurements[i].gnssSignalType,
               raw_meas.measurements[i].basebandCarrierToNoiseDbHz,
               raw_meas.measurements[i].fullInterSignalBiasNs,
               raw_meas.measurements[i].fullInterSignalBiasUncertaintyNs);
    }
}

static void gnssAuthStatusCb(const uint64_t status)
{
    LOG_Debug("Capabilities Status =0x%lX", status);
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

                        char nmeamsg[256] = {0};
                        memcpy(nmeamsg, startPtr, packageLen);
                        gnssNmeaCb(nmeamsg);

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
        g_gnssReportCb.locationCb = gnssLoctionCb;
        g_gnssReportCb.speLocationCb = SPELoctionCb;
        g_gnssReportCb.ppeLocationCb = PPELoctionCb;
        g_gnssReportCb.nmeaCb = gnssNmeaCb;
        g_gnssReportCb.svCb = gnssSvCb;
        g_gnssReportCb.dataCb = gnssDataCb;
        g_gnssReportCb.rawMeasCb = gnssMeasCb;
        g_gnssReportCb.authStatusCb = gnssAuthStatusCb;

        err = ql_gnss_init();
        if (err == QL_ERR_OK)
        {
            LOG_Debug("ql_gnss_init Successful");
        }
        else
        {
            LOG_Debug("Failed to ql_gnss_init, err=%d", err);
        }
        err = ql_gnss_reg_cb(g_gnssReportCb);
        if (err != QL_ERR_OK)
        {
            LOG_Debug("Failed to ql_gnss_reg_cb, err=%d", err);
        }
        else
        {
            LOG_Debug("Successful");
        }
        err = ql_loc_set_start_mode(start_mode);
        if (err != QL_ERR_OK)
        {
            LOG_Debug("Failed to ql_loc_set_start_mode, err=%d", err);
        }
        else
        {
            LOG_Debug("Successful");
        }
        err = ql_gnss_set_nmea_type(nmea_type);
        if (err != QL_ERR_OK)
        {
            LOG_Debug("Failed to ql_gnss_set_nmea_type, err=%d", err);
        }
        else
        {
            LOG_Debug("Successful");
        }
        err = ql_gnss_set_robust_location_config(&robust);
        if (err != QL_ERR_OK)
        {
            LOG_Debug("Failed to ql_gnss_set_robust_location_config, err=%d", err);
        }
        else
        {
            LOG_Debug("Successful");
        }
        err = ql_loc_set_constellation(constellation);
        if (err != QL_ERR_OK)
        {
            LOG_Debug("Failed to ql_loc_set_constellation, err=%d", err);
        }
        else
        {
            LOG_Debug("Successful");
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
        err = ql_gnss_start(interval, fix_mode);
        if (err != QL_ERR_OK)
        {
            LOG_Debug("Failed to start gnss , err=%d", err);
        }
        else
        {
            LOG_Debug("ql_gnss_start Successful");
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
        ql_gnss_stop();
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
