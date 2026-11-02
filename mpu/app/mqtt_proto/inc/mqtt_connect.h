#ifndef __MQTT_CONNECT_H__
#define __MQTT_CONNECT_H__

#include <glib.h>
#include "mqtt_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define MQTT_TEST_TOPIC         "E300/pre/vehicle/status/"      //测试环境 发布topic 后缀需加入VIN 
#define MQTT_PROD_TOPIC         "E300/prod/vehicle/status/"     //正式环境 发布topic 后缀需加入VIN 
#define MQTT_TEST_E260S_TOPIC   "E260S/pre/vehicle/status/"      //测试环境 发布topic 后缀需加入VIN 
#define MQTT_PROD_E260S_TOPIC   "E260S/prod/vehicle/status/"     //正式环境 发布topic 后缀需加入VIN 
#define MQTT_TEST_EQ100_TOPIC   "EQ100/pre/vehicle/status/"      //测试环境 发布topic 后缀需加入VIN 
#define MQTT_PROD_EQ100_TOPIC   "EQ100/prod/vehicle/status/"     //正式环境 发布topic 后缀需加入VIN 
#define MQTT_TEST_F510C_TOPIC   "F510C/pre/vehicle/status/"      //测试环境 发布topic 后缀需加入VIN 
#define MQTT_PROD_F510C_TOPIC   "F510C/prod/vehicle/status/"     //正式环境 发布topic 后缀需加入VIN 
#define MQTT_TEST_F710C_TOPIC   "F710C/pre/vehicle/status/"      //测试环境 发布topic 后缀需加入VIN 
#define MQTT_PROD_F710C_TOPIC   "F710C/prod/vehicle/status/"     //正式环境 发布topic 后缀需加入VIN
#define MQTT_TEST_F520M_TOPIC   "F520M/pre/vehicle/status/"      //测试环境 发布topic 后缀需加入VIN 
#define MQTT_PROD_F520M_TOPIC   "F520M/prod/vehicle/status/"     //正式环境 发布topic 后缀需加入VIN

#define MQTT_TEST_F710C_PARK_TOPIC   "comm/pre/vehicle/parking/status/"      //测试环境 发布topic 后缀需加入VIN 
#define MQTT_PROD_F710C_PARK_TOPIC   "comm/prod/vehicle/parking/status/"     //正式环境 发布topic 后缀需加入VIN

#define DEVICE_CERT     "/oemdata/cert/device.pem"
#define CERT_CHAIN        "/oemapp/cert/certificate_chain_client.pem" 


#define MQTT_CONNECT_IP_ADDR   "mqtt.zelostech.com.cn"
// #define MQTT_CONNECT_IP_ADDR   "mqtt-uat.zelostech.com.cn"

//#define MQTT_CONNECT_IP_ADDR   "mqtt-volcbj1.zelostech.com.cn"

//#define MQTT_CONNECT_PORT      (1993)
#define MQTT_CONNECT_PORT      (6993)


#define MQTT_QOS0               (0)
#define MQTT_QOS1               (1)
#define MQTT_QOS2               (2)

#define TSP_DATA_MAX_LEN (1024)


gboolean IsConnected(void);
/**
 * @brief Get the Package Data object
 * 
 * @param pData 
 * @return gint 
 */
gint GetTspPackageData(gpointer *pData);
/**
 * @brief Cache the Package Data object
 * 
 * @param pData 
 * @return gint 
 */
gint CachePackageData(gpointer pData);
/**
 * @brief Connect Thread Entry
 * 
 * @param pArg 
 * @return void * 
 */
void *ConnectTaskEntry(void *pArg);

void *SendPackageDataTaskEntry(void *pArg);

/**
 * @brief Send Package Data To Tsp Server
 * 
 * @param[in]   u8Data 
 * @param[in]   u32length 
 * @retval      RET_OK  Success
 *              RET_ERR Failed
 */
gint SendPackageData(const gchar *topic, gint qos, gpointer u8Data, guint u32length);
/**
 * 获取电源状态
*/
gint GetSleepMode();

int32_t GetMqttSendStatus(void);

void KLINEStatusCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata);

void PmStatusCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata);

void *NetworkMonitorThread(void *pArg);

/**
 * @brief 获取QoS开关标志
 * 
 * @return gint 
 */
gint GetQoSFlag(void);

/**
 * @brief 设置QoS开关标志
 * 
 * @param flag 
 */
void SetQoSFlag(int flag);

void GatewaySync(const uint8_t* data, const uint16_t len, void* userdata);

#ifdef __cplusplus
}
#endif
#endif

