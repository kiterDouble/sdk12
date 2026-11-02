#ifndef _ECALL_GPS_H_
#define _ECALL_GPS_H_

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0
typedef struct{
    uint8_t status;                             /* 数据有效性 0-无效,非0-有效 */
    uint16_t direction;                         /* 方向（偏离正北方向的角度）*/
    double latitude_value_N1;                   /* 上一次纬度 */
    double longtitude_value_N1;                 /* 上一次经度 */
    double latitude_value_N2;                   /* 上二次纬度 */
    double longtitude_value_N2;                 /* 上二次经度 */
    uint32_t timestamp;                         /* 时间戳 */
    double latitude_value;                      /* 纬度的原始浮点型格式。南纬负值，北纬正值 */
    double longtitude_value;                    /* 经度的原始浮点型格式。西经负值，东经正值 */
    uint32_t latitude_degree;                   /* 经度，单位：度，精度：10的-7次方 */
    uint32_t longtitude_degree;                 /* 纬度，单位：度，精度：10的-7次方 */
}StrGPSInfo;
#else
typedef struct{
    uint8_t trusted;                            /* 数据有效性 0-无效,非0-有效 */
    uint8_t direction;                          /* 方向（偏离正北方向的角度）*/
    int32_t latitude;
    int32_t longitude;
    int32_t latitudeDeltaN1;
    int32_t longitudeDeltaN1;
    int32_t latitudeDeltaN2;
    int32_t longitudeDeltaN2;
    uint32_t timestamp;                         /* 时间戳 */
}StrGPSInfo;
#endif


/**
* @brief 初始化Ecall的GNSS模块
* @return void
*/
extern void EcallGnssInit(void);

extern void EcallGPSInfoDump(StrGPSInfo *data);
extern void EcallGnssDump(void);

/**
* @brief 获取GPS信息
*
* @return GPSInfo_t 返回GPS信息结构体
*/
extern StrGPSInfo *GetGpsInfo(void);

#ifdef __cplusplus
}
#endif

#endif //_ECALL_GPS_H_


