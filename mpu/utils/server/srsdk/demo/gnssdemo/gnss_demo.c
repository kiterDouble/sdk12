#include "sr_gnss.h"
#include "sr_sdk.h"
#include "sr_log.h"
#include "err_code.h"
#include <unistd.h>

/*
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
*/
int main()
{

    GnssLocation_t data = {0};
    LOG_INIT("demo");
    
    InitSRSdk();
    while(1)
    {
	    GetGnssLocation(&data);
	    
	    LOG_Debug("status %d", data.status);
	    LOG_Debug("antStatus %d", data.antStatus);
	    LOG_Debug("gpsState %d", data.gpsState);
	    LOG_Debug("longtitudeValue %d", data.longtitudeValue);
	    LOG_Debug("latitudeValue %d", data.latitudeValue);
	    LOG_Debug("flongitude %f", data.flongitude);
	    LOG_Debug("flatitude %f", data.flatitude);
	    LOG_Debug("seaHeight %d", data.seaHeight);
	    LOG_Debug("speed %d", data.speed);
	    LOG_Debug("direction %d", data.direction);
	    LOG_Debug("accuracy %d", data.accuracy);
	    LOG_Debug("num_svs %d", data.num_svs);
	    LOG_Debug("moonLinkNum %d", data.moonLinkNum);
	    LOG_Debug("timestamp %d", data.timestamp);
	    
	    sleep(1);
    }
    
    return 0;
    
}
