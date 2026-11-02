/**
 * @file sr_hal_gnss.h
 * @author jiajiangze (jiangze.jia@sirun.net)
 * @brief gnss hal层接口头文件
 * @version 0.1
 * @date 2023-12-08
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef ___HAL_SR_GNSS_H__
#define ___HAL_SR_GNSS_H__
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif

    #define GNSS_INI_FILE_PATH "/oemapp/config/gnss.ini"
    
    typedef enum
    {
        E_GNSS_ANT_STATUS_CIRCUIT_NORMAL = 0,    // 正常
        E_GNSS_ANT_STATUS_CIRCUIT_OPEN = 1,      // 电线断路
        E_GNSS_ANT_STATUS_CIRCUIT_SHORT_GND = 2, // 电线地短路
        E_GNSS_ANT_STATUS_CIRCUIT_SHORT_VCC = 3, // 电线电源短路
        E_GNSS_ANT_STATUS_CIRCUIT_UNKNOW = 99    // 获取失败
    } HalGnssAntStatus_e;

#pragma pack(1)
    // 数据来自GPGGA与GPRMC
    typedef struct
    {
        uint32_t time;               /* gps定位时间 hhmmss(时分秒)格式*/
        double latitude;             /*纬度   ddmm.mmmmmm dd - 度 mm.mmmmmm - 分*/
        double longitude;            /* 经度 ddmm.mmmmmm dd - 度 mm.mmmmmm - 分 */
        double speed;                /* 速度  地面速率，单位为节*/
        double direction;            /*航向 地面航向，单位为度，从北向起顺时针计算*/
        double height;               /*海拔高 米*/
        uint32_t date;               /*日期  ddmmyy(日月年)格式*/
        char posState;               /*定位状态，A=有效定位，V=无效定位*/
        int8_t gpsState;             /*GPS状态， 0初始化， 1单点定位， 2码差分， 3无效PPS， 4固定解， 5浮点解， 6正在估算 7，人工输入固定值， 8模拟模式， 9WAAS差分*/
        char latitudeFlag;           /*纬度半球 N或S(北纬或南纬)*/
        char longitudeFlag;          /*经度半球 E或W(东经或西经)*/
        float accuracy;              /*水平精度 米*/
        char mode;                   /* 模式指示(仅NMEA0183 3.00版本输出，A=自主定位，D=差分，E=估算，N=数据无效)*/
        uint8_t declination;         /* 磁偏角 000.0~180.0度*/
        char declinationDirection;   /* 磁偏角 方向 E(东)或W(西)*/
        uint32_t sumNosv;            /* 使用卫星数量*/
        uint32_t num_svs;            /* 可见卫星数量*/
    } HalGnssLocationInfo_t;

#define SVS_MAX_NUM 32 /**  Maximum number of satellites in view. */

    typedef struct
    {
        int prn;         /**<   卫星 ID*/
        float snr;       /**<   信噪比。范围：0~99。*/
        float elevation; /**<   卫星仰角。范围：0~90；单位：度。*/
        float azimuth;   /**<   卫星方位角。范围：0~360；单位：度。*/
    } HalGnssSvInfo_t;

    typedef struct
    {
        int num_svs;                          /**<   可见卫星数量*/
        HalGnssSvInfo_t sv_list[SVS_MAX_NUM]; /**<   卫星数量及卫星基本数据。卫星数量范围：0~32。*/
        uint32_t ephemeris_mask;              /**<   星历有效掩码，指示卫星是否有有效星历。*/
        uint32_t almanac_mask;                /**<   历书有效掩码，指示卫星是否有有效历书。*/
    } HalGnssSvStatus_t;

#define NMEA_MAX_LENGTH 255 /**  NMEA string maximum length. */
    typedef struct
    {
        int64_t timestamp;              /**<   System Timestamp, marked for when got the nmea data */
        int length;                     /**<   NMEA string length. */
        char nmea[NMEA_MAX_LENGTH + 1]; /**<   NMEA string.*/
    } HalGnssNmea_t;                    /* Message */
#pragma pack()

/**
 * @brief 位置信息回调函数类型定义
 * 
 */
typedef void (*GnssLocationInfoCallBack)(HalGnssLocationInfo_t *call);
/**
 * @brief NMEA信息回调函数类型定义
 * 
 */
typedef void (*GnssNmeaStringCallBack)(HalGnssNmea_t *call);


/**
 * @brief Hal Gnss初始化接口
 * 
 * @param locaCb 位置信息回调函数
 * @param nmeaCb NMEA信息回调函数
 * @return int32_t 执行结果：RET_OK/RET_ERR
 */
int32_t HalGnssInit(GnssLocationInfoCallBack locaCb, GnssNmeaStringCallBack nmeaCb);
/**
 * @brief Hal 开始Gnss定位
 * 
 * @return int32_t 执行结果：RET_OK/RET_ERR
 */
int32_t HalGnssStart(void);
/**
 * @brief Hal 结束Gnss定位
 * 
 * @return int32_t 执行结果：RET_OK/RET_ERR
 */
int32_t HalGnssEnd(void);
/**
 * @brief Hal Gnss反初始化接口
 * 
 * @return int32_t 执行结果：RET_OK/RET_ERR
 */
int32_t HalGnssDeInit(void);
/**
 * @brief Hal 主动获取最新定位信息接口
 * 
 * @param data 主动获取到的最新定位信息 方向out
 */
void HalGnssGetLocationInfo(HalGnssLocationInfo_t *data);
/**
 * @brief Hal 主动获取最新卫星信息接口
 * 
 * @param data 主动获取到的最新卫星信息 方向out
 */
void HalGnssGetSvStatus(HalGnssSvStatus_t *data);
/**
 * @brief Hal 主动获取最新天线状态信息接口
 * 
 * @param data 主动获取到的最新天线状态信息 方向out
 */
void HalGnssGetAntStatus(HalGnssAntStatus_e *data);

#ifdef __cplusplus
}
#endif

#endif