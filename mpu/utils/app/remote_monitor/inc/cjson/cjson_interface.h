#ifndef _CJSON_INTERFACE_H_
#define _CJSON_INTERFACE_H_

#include <iostream>
#include <queue>
#include <string>
#include <cstring>
#include "mqtt.h"
#include "mqtt_data_handler.h"
#include "mcu_interface.h"

#define MAX_CAN_ID_LEN      4
#define MAX_CAN_DATA_LEN    8
#define CAN_ID_STR_LEN      8

#define GPSSERVICEID        (0x0002)
#define GPSSUBSERVICEID     (0x0001)  


#define CANSERVICEID        (0x0001)
#define CANSUBSERVICEID     (0x0001)  


#define RESREMOTECONFIGSERVICEID        (0x0003)
#define RESREMOTECONFIGSUBSERVICEID     (0x0002)  
 
#define RESCONTACTSERVICEID        (0x0004)
#define RESCONTACTSUBSERVICEID     (0x0002)  


#define RESREMOTEOTASERVICEID        (0x0502)
#define RESREMOTEOTASUBSERVICEID     (0x0002)  


#define RESREMOTEOTASWAKEERVICEID        (0x0501)
#define RESREMOTEOTAWAKESUBSERVICEID     (0x0002)  

#define RESREMOTEOTASFIELDSERVICEID        (0x0503)
#define RESREMOTEOTAFIELDSUBSERVICEID     (0x0001)  


#define RESREMOTEDOMAINSERVICEID        (0x0504)
#define PROTOCOLVER         "V2.0.0"
#define CANPROTOCOLVER      "V2.0.0"
#define GPSPROTOCOLVER      "V2.0.0"
// 定义返回状态码
typedef enum {
    CVJ_SUCCESS = 0,
    CVJ_INVALID_PARAM,
    CVJ_MEMORY_ERROR,
    CVJ_PARSE_ERROR,
    CVJ_KEY_NOT_FOUND,
    CVJ_CAN_DATA_ERROR
} CVJ_Status;

// CAN数据帧结构
typedef struct {
    uint16_t can_id;    // CAN ID (如0x353)
    uint8_t *data;      // CAN数据 (最大8字节)
    uint8_t data_len;   // 数据长度 (1-8)
} CAN_Frame;

// 车辆数据结构
typedef struct {
    char vehicle_no[32];
    char vin[32];
    char protocol_ver[16];
    uint16_t service_id;
    uint16_t sub_service_id;
    uint64_t timestamp;
    CAN_Frame *can_frames;
    size_t frame_count;
} VehicleData;



// 返回状态码
typedef enum {
    GVJ_SUCCESS = 0,
    GVJ_INVALID_PARAM,
    GVJ_MEMORY_ERROR,
    GVJ_PARSE_ERROR,
    GVJ_INVALID_GPS_DATA
} GVJ_Status;


// GPS定位数据结构
typedef struct {
    uint8_t fix_status;
    char ns;           // 'N'或'S'
    char we;           // 'W'或'E'
    double longitude;   // 经度
    double latitude;    // 纬度
    double altitude;    // 高程
    float speed;       // 速度
    float hop;       // 高程
    float hdop;        // 水平精度因子
    float vdop;        // 垂直精度因子
    float pdop;        // 位置精度因子
    uint8_t snr;       // 信噪比
    uint8_t satellites;// 可见卫星数
} GPS_Data;

// 车辆数据结构
typedef struct {
    char vehicle_no[32];
    char vin[32];
    char protocol_ver[16];
    uint16_t service_id;
    uint16_t sub_service_id;
    uint64_t timestamp;
    GPS_Data gps_data;
} VehicleGPSData;




// 定义 Payload 结构体
typedef struct {
    char *esc_function;
    //int low_speed_prompt_volume;
    char *external_discharge_func;
    // char *audio_prompt_file;
    char *mileage_sync;
    // char *imu_calibration;
    // int standard_tire_pressure;
    // char *left_front_tire_sensor_id;
    // char *right_front_tire_sensor_id;
    // char *left_rear_tire_sensor_id;
    // char *right_rear_tire_sensor_id;
    // char *cargo_door_configuration_file;
} Payload_t;

// 定义主消息结构体
typedef struct {
    char *vehicle_no;
    char *vin;
    char *protocol_ver;
    char *seqid;
    unsigned int service_id;
    unsigned int sub_service_id;
    long long timestamp;
    Payload_t payload;
} VehicleMessage_t;



// 定义嵌套结构体 ConfigStatus
typedef struct {
    const char *escFunction;
    //const char *lowSpeedPromptVolume;
    const char *externalDischargeFunc;
    // const char *audioPromptFile;
    const char *mileageSync;
    // const char *imuCalibration;
    // const char *standardTirePressure;
    // const char *leftFrontTireSensorID;
    // const char *rightFrontTireSensorID;
    // const char *leftRearTireSensorID;
    // const char *rightRearTireSensorID;
    // const char *cargoDoorConfigurationFile;
} RemoteConfigStatus;

typedef struct {
    const char  *result;
    int error_code;
    RemoteConfigStatus configStatus;
} RemoteConfigResponsePayload;

// 定义车辆信息结构体
typedef struct {
    const char *vehicleNo;
    const char *vin;
    const char *protocolVer;
    const char *seqid;
    unsigned int serviceId;
    unsigned int subServiceId;
    unsigned long long timeStamp;
    RemoteConfigResponsePayload ResponsePayload; // 嵌套结构体
} RemoteConfigVehicleInfo;


// 定义 Payload 结构体
typedef struct {
    char *first_level_contact_switch;
    char *first_level_contact_switch_release;
    char *second_level_contact_switch_release;
    char *third_level_contact_switch_release;
} ContactSwitchPayload_t;

// 定义主消息结构体
typedef struct {
    char *vehicle_no;
    char *vin;
    char *protocol_ver;
    char *seqid;
    unsigned int service_id;
    unsigned int sub_service_id;
    long long timestamp;
    ContactSwitchPayload_t payload;
} VehicleContactMessage_t;



// 定义用于存储配置状态的结构体
typedef struct {
    const char *firstLevelContactSwitch;
    const char *firstLevelContactSwitchRelease;
    const char *secondLevelContactSwitchRelease;
    const char *thirdLevelContactSwitchRelease;
} ContactConfigStatus;

typedef struct {
    const char*  result;
    int error_code;
    ContactConfigStatus contact_ctatus;
} ContactConfigStatusResponse;
// 定义车辆信息的主结构体
typedef struct {
    const char *vehicleNo;
    const char *vin;
    const char *protocolVer;
    const char *seqid;
    unsigned int serviceId;
    unsigned int subServiceId;
    long long timeStamp;
    ContactConfigStatusResponse configStatus; // 嵌套结构体
} ContactVehicleInfoResponse;




#if 0
typedef struct {
    char  result;
    int error_code;
    ContactConfigStatus contact_ctatus;
} ContactConfigStatusResponse;
// 定义车辆信息的主结构体
typedef struct {
    const char *vehicleNo;
    const char *vin;
    const char *protocolVer;
    unsigned int serviceId;
    unsigned int subServiceId;
    long long timeStamp;
    ContactConfigStatusResponse configStatus; // 嵌套结构体
} ContactVehicleInfoResponse;



#endif

#if 1
typedef struct {
    char firstLevelContactSwitch;
    char firstLevelContactSwitchRelease;
    char secondLevelContactSwitchRelease;
    char thirdLevelContactSwitchRelease;
} ContactConfigResultStatus;

typedef struct {
    char  result;
    int error_code;
    ContactConfigResultStatus contact_ctatus;
} ContactConfigResultStatusResponse;



#endif



// 定义嵌套结构体 ConfigStatus
typedef struct {
    char escFunction;
    //char lowSpeedPromptVolume;
    char externalDischargeFunc;
    // char audioPromptFile;
    char mileageSync;
    // char imuCalibration;
    // char standardTirePressure;
    // char leftFrontTireSensorID;
    // char rightFrontTireSensorID;
    // char leftRearTireSensorID;
    // char rightRearTireSensorID;
    // char cargoDoorConfigurationFile;
} RemoteConfigResponseStatus;

typedef struct {
    char  result;
    int error_code;
    RemoteConfigResponseStatus configStatus;
} RemoteConfigResponse;


// 定义包含 Playload 的结构体
typedef struct {
    int Result;
} OtaPlayload;

// 定义主结构体来包含 JSON 数据
typedef struct {
    char *VehicleNo;
    char *Vin;
    char *ProtocolVer;
    int ServiceId;
    int SubServiceId;
    long long TimeStamp;
    char *SeqId;
    OtaPlayload playload;
} OtaVehicleData;

// ota唤醒
typedef struct {
    int Silence;
} AwakenPlayload;

typedef struct {
    char *VehicleNo;
    char *Vin;
    char *ProtocolVer;
    int ServiceId;
    int SubServiceId;
    long long TimeStamp;
    char *SeqId;
    AwakenPlayload Playload;;
}OtaAwakenVehicleData;

//OTA唤醒回复
typedef struct {
    unsigned char Wakeup;
} AwakenReplyPlayload;

typedef struct {
    char *VehicleNo;
    char *Vin;
    char *ProtocolVer;
    int ServiceId;
    int SubServiceId;
    long long TimeStamp;
    char *SeqId;
    AwakenReplyPlayload Playload;
} AwakenReply;



// 定义一个数据结构来保存解析后的数据
typedef struct {
    uint64_t logStart;
    uint64_t logEnd;
    char *batchNumber;
    char *cmdType;
} LogData;

// 定义一个结构体来封装车辆信息
typedef struct {
    char* vehicleNo;  // 车辆编号
    char* vin;        // VIN
    char *batchNumber;  // 批次号
    const char* fileKey;    // 文件键
    int status;             // 上传状态
} ReplyLogData;

typedef struct {
    char *data;    // 用于存储 "data" 的值
    int code;          // 用于存储 "code" 的值
    char *msg;     // 用于存储 "msg" 的值
} HttpCanJsonResponse;

typedef struct {
    char *batchNumber;  // 批次号
    const char* fileKey;    // 文件键
    int status;             // 上传状态
} ReplyLogDataParameter;



typedef struct {
    const char *VehicleNo;
    const char *Vin;
    const char *ProtocolVer;
    int ServiceId;
    int SubServiceId;
    long long TimeStamp;
    const char *SeqId;
    const char *Field;
} SyncOtaFieldVehicleInfo;


typedef struct {
    char *VehicleNo;
    char *Vin;
    char *ProtocolVer;
    int ServiceId;
    int SubServiceId;
    long long TimeStamp;
    char *SeqId;
    char *PayloadTarget;
} DomainVehicleData;

typedef struct {
    char *VehicleNo;      // 车辆编号
    char *Vin;            // 车辆识别码
    char *ProtocolVer;    // 协议版本
    int ServiceId;        // 服务 ID
    int SubServiceId;     // 子服务 ID
    long long TimeStamp;  // 时间戳
    char *SeqId;          // 序列 ID
    int Result;           // Payload 内的结果值
} DomainReplyVehicleData;

char *CanVehicleJsonInterface();
char *GpsVehicleJsonInterface();
VehicleMessage_t* ParseVehicleRemoteConfigurationMessage(const char *json_str);
void FreeRemoteConfigurationVehicleMessage(VehicleMessage_t *msg);
void PrintRemoteConfigurationVehicleMessage(const VehicleMessage_t *msg) ;
void FreeVehicleContactMessage(VehicleContactMessage_t *msg);
VehicleContactMessage_t* ParseVehicleContactMessage(const char *json_str) ;
void PrintVehicleContactMessage(const VehicleContactMessage_t *msg);
char *ResponseRemoteConfigurationCjaon(RemoteConfigResponse configResponse,char *seqid);
char *ResponseVehicleContactCjaon(ContactConfigResultStatusResponse result,char *seqid);
char* CreateWillJsonInterface();
char* OtaCreateJsonInterface(char *seqid,unsigned char result) ;
char* OtaExtractPlayload(const char* jsonString,char *seqid) ;
int GetServiceIdFromJson(const char* jsonString) ;
char* OtaGetSeqId(const char* jsonString) ;
char* OtaWakenVehicleCreateJSONInterface(char *seqid ,unsigned char result);
int GetHttpErrValue(const char *json_string) ;
int CanLogParseJson(const char *jsonString, LogData *logData) ;
int ParseHttpCanJsonResponse(const char* jsonString, HttpCanJsonResponse* response) ;
int CreateReplyLogDataJson(const ReplyLogData* vehicleInfo, char** jsonString) ;
void FreeHttpCanJson(HttpCanJsonResponse* response);
void FreeCanLogParseJson(LogData *logData);
int ReplyLogDataJsonInterface(ReplyLogDataParameter parameter,char **jsonstring);
char* SyncOtaFieldVehicleInfoCreatJSONIntetface(char *str);
int SyncOtaFieldVehicleInfoParseResult(const char *jsonString, unsigned int *result);
char* TimeStampChanged(const char* original_json,long long new_timestamp);

int DomainParseVehicleData(const char *json_string, DomainVehicleData *data);
void FreeDomainVehicleData(DomainVehicleData *data);
char* DomainCreateJsonInterface(char *seqid,int result);
int GetDomainHttpReValue(const char *json_string) ;
int CreateFotaHttpJson(int r, const char *platform,char* str,int len) ;
#endif
