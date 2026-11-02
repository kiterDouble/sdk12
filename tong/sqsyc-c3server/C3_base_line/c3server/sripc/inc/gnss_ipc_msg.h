/**
 * @file gnss_ipc_msg.h
 * @author jiajiangze (jiangze.jia@sirun.net)
 * @brief gnss服务的IPC、RPC传输消息类型定义头文件
 * @version 0.1
 * @date 2023-12-08
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __GNSS_IPC_MSG_H__
#define __GNSS_IPC_MSG_H__
#include <stdint.h>

#pragma pack(1)
typedef struct
{
    uint8_t status;           // 模块状态 0：工作正常 1：工作异常
    uint8_t antStatus;        // 电线状态(天线状态) 0: 正常 1：电线断路 2：电线地短路 3：电线电源短路 99：获取失败
    uint8_t gpsState;         // 定位状态Bit0-bit5: GNSS定位状态 0: 未定位 1: 单点定位 2: 浮点定位 3: RTK定位  Bit6: 0-东经，1-西经 Bit7: 0-北纬，1-南纬
    uint32_t longtitudeValue; // 经度，单位：度，精度：10的-7次方
    uint32_t latitudeValue;   // 纬度，单位：度，精度：10的-7次方
    double flongitude;        // 经度的原始浮点型格式。西经负值，东经正值
    double flatitude;         // 纬度的原始浮点型格式。南纬负值，北纬正值
    uint16_t seaHeight;       // 高程，单位：米，精度：0.1米
    uint16_t speed;           // 速度，单位：km/h，精度：0.1km/h
    uint16_t direction;       // 航向，单位：度，精度：0.1度
    uint16_t accuracy;        // 定位水平精度,单位：厘米，精度：1厘米
    uint8_t num_svs;          // 可见卫星数量
    uint8_t moonLinkNum;      // 使用卫星数量
    uint32_t timestamp;       // 时间戳，精度：秒
} GnssLocation_t;             // IPC发送的数据结构

#define GNSS_NMEA_TYPE_GPGGA (1)
#define GNSS_NMEA_TYPE_GPRMC (2)
#define GNSS_NMEA_TYPE_GSA   (3)
#define GNSS_NMEA_TYPE_GSV   (4)
#define GNSS_NMEA_TYPE_VTG   (5)
#define GNSS_NMEA_TYPE_GLL   (6)
#define GNSS_NMEA_MAX_LEN    (256)

typedef struct
{
    uint64_t timestamp; // 时间戳
    uint8_t type;       // nmea类型 1：GPGGA 2：GPRMC 3、GSA
    uint8_t length;     // nmea字串长度
    char nmea[GNSS_NMEA_MAX_LEN];     // nmea字串
} GnssNmea_t;

typedef struct
{
    int prn;         /**<   卫星 ID*/
    float snr;       /**<   信噪比。范围：0~99。*/
    float elevation; /**<   卫星仰角。范围：0~90；单位：度。*/
    float azimuth;   /**<   卫星方位角。范围：0~360；单位：度。*/
} GnssSvInfo_t;

typedef struct
{
    int num_svs;              /**<   可见卫星数量*/
    GnssSvInfo_t sv_list[32]; /**<   卫星数量及卫星基本数据。卫星数量范围：0~32。*/
    uint32_t ephemeris_mask;  /**<   星历有效掩码，指示卫星是否有有效星历。*/
    uint32_t almanac_mask;    /**<   历书有效掩码，指示卫星是否有有效历书。*/
} GnssSvStatus_t;
#pragma pack()

#endif
