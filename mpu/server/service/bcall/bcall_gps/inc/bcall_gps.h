#ifndef _BCALL_GPS_H_
#define _BCALL_GPS_H_

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
    uint8_t status;                               /*数据有效性 0-无效,非0-有效*/
    uint16_t direction;                            /*方向（偏离正北方向的角度）*/
    double latitude_value_N1;                   /*上一次纬度*/    
    double longtitude_value_N1;                 /*上一次经度*/     
    double latitude_value_N2;                   /*上二次纬度*/    
    double longtitude_value_N2;                 /*上二次经度*/       
    uint32_t timestamp;                           /*时间戳*/ 
    double latitude_value;                       /*纬度的原始浮点型格式。南纬负值，北纬正值*/
    double longtitude_value;                     /*经度的原始浮点型格式。西经负值，东经正值*/
    uint32_t latitude_degree;                   // 经度，单位：度，精度：10的-7次方
    uint32_t longtitude_degree;                 // 纬度，单位：度，精度：10的-7次方
}StrGPSInfo;

/**
* @brief 当接收到GPS消息时，此函数将被调用。
* @param data 接收到的GPS消息数据。
* @param len GPS消息数据的长度。
* @param userdata 回调函数传递的用户数据。
* @return void
*/
extern void GnssMsgSubCallBack(const uint8_t* data, const uint16_t len, void* userdata);

/**
* @brief 获取GPS信息
*
* @return GPSInfo_t 返回GPS信息结构体
*/
extern StrGPSInfo GetGpsInfo(void);

#ifdef __cplusplus
}
#endif

#endif //_BCALL_GPS_H_


