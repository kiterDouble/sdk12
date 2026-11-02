#include <stdio.h>
#include <unistd.h>
#include "cjson_interface.h"
#include "cJSON.h"
#include "mqtt.h"
#include "system_interface.h"
#include "cjson_interface.h"
#include "mcu_interface.h"
#include "sr_log.h"
#include "sr_gnss.h"
#include "timer_interface.h"
#include "queue.h"

// CAN ID转字符串 (如0x353)
void CanIdToString(uint32_t can_id, char *str, size_t str_size) {
    snprintf(str, str_size, "0x%X", can_id);
}

// 创建JSON字符串
static CVJ_Status CreateCanVehicleJson(const VehicleData *data, char **json_str) 
{
    if (!data || !json_str) 
        return CVJ_INVALID_PARAM;

    cJSON *root = cJSON_CreateObject();
    if (!root) 
        return CVJ_MEMORY_ERROR;

    // 添加基础字段
    cJSON_AddStringToObject(root, "VehicleNo", data->vehicle_no);
    cJSON_AddStringToObject(root, "VIN", data->vin);
    cJSON_AddStringToObject(root, "ProtocolVer", data->protocol_ver);
    
    // 16进制格式的服务ID
    cJSON_AddNumberToObject(root, "ServiceId", data->service_id);
    cJSON_AddNumberToObject(root, "SubServiceId", data->sub_service_id);
    cJSON_AddNumberToObject(root, "TimeStamp", data->timestamp);

    // 添加Payload (CAN数据)
    cJSON *payload = cJSON_AddObjectToObject(root, "Payload");
    if (!payload) {
        cJSON_Delete(root);
        return CVJ_MEMORY_ERROR;
    }

    for (size_t i = 0; i < data->frame_count; i++) {
        const CAN_Frame *frame = &data->can_frames[i];
        
        // 将CAN数据转换为16进制字符串
        char *hex_str = (char *)malloc(frame->data_len * 3 + 1);
        if (!hex_str) {
            cJSON_Delete(root);
            return CVJ_MEMORY_ERROR;
        }
        
        for (uint8_t j = 0; j < frame->data_len; j++) {
            //sprintf(hex_str + j * 2, "%02X", frame->data[j]);
            sprintf(hex_str + j * 3, "%02X ", frame->data[j]);
        }
        
        // 添加CAN ID作为键
        char can_id_str[CAN_ID_STR_LEN];
        CanIdToString(frame->can_id, can_id_str, sizeof(can_id_str));
        cJSON_AddStringToObject(payload, can_id_str, hex_str);
        free(hex_str);
    }

    *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return (*json_str) ? CVJ_SUCCESS : CVJ_MEMORY_ERROR;
}

void FreeCanData(VehicleData *data) 
{
  if (!data) return;

    if (data->can_frames) {
        for (size_t i = 0; i < data->frame_count; i++) {
            if (data->can_frames[i].data) {
                free(data->can_frames[i].data);
            }
        }
        free(data->can_frames);
        data->can_frames = NULL;
    }
    data->frame_count = 0;
    
}

static void JsonDoubleTypeProcess(cJSON *object, const char *name, double value, int precision) 
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.*f", precision, value);
    cJSON_AddRawToObject(object, name, buffer);
}


// 创建JSON字符串
static GVJ_Status CreateGpsVehicleJson(const VehicleGPSData *data, char **json_str) 
{
    if (!data || !json_str) return GVJ_INVALID_PARAM;

    cJSON *root = cJSON_CreateObject();
    if (!root) return GVJ_MEMORY_ERROR;

    // 添加基础字段
    cJSON_AddStringToObject(root, "VehicleNo", data->vehicle_no);
    cJSON_AddStringToObject(root, "VIN", data->vin);
    cJSON_AddStringToObject(root, "ProtocolVer", data->protocol_ver);
    
    // 16进制格式的服务ID
    
    cJSON_AddNumberToObject(root, "ServiceId", data->service_id);
    cJSON_AddNumberToObject(root, "SubServiceId", data->sub_service_id);
    cJSON_AddNumberToObject(root, "TimeStamp", data->timestamp);

    // 添加GPS数据Payload
    cJSON *payload = cJSON_AddObjectToObject(root, "Payload");
    if (!payload) {
        cJSON_Delete(root);
        return GVJ_MEMORY_ERROR;
    }

    // 添加GPS字段
    cJSON_AddNumberToObject(payload, "FixStatus", data->gps_data.fix_status);
    
    // 添加NS/WE方向字符
    char ns_str[2] = {data->gps_data.ns, '\0'};
    cJSON_AddStringToObject(payload, "NS", ns_str);
    
    char we_str[2] = {data->gps_data.we, '\0'};
    cJSON_AddStringToObject(payload, "WE", we_str);
    JsonDoubleTypeProcess(payload, "VDOP", data->gps_data.vdop,6);
    JsonDoubleTypeProcess(payload, "Lon", data->gps_data.longitude,6);
    JsonDoubleTypeProcess(payload, "Lat", data->gps_data.latitude,6);
    JsonDoubleTypeProcess(payload, "Hop", data->gps_data.altitude,6);
    JsonDoubleTypeProcess(payload, "Speed", data->gps_data.speed,6);
    JsonDoubleTypeProcess(payload, "HDOP", data->gps_data.hdop,6);
    JsonDoubleTypeProcess(payload, "VDOP", data->gps_data.vdop,6);
    JsonDoubleTypeProcess(payload, "PDOP", data->gps_data.pdop,6);
    JsonDoubleTypeProcess(payload, "SNR", data->gps_data.snr,6);
    cJSON_AddNumberToObject(payload, "SatellitesInView", data->gps_data.satellites);

    *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return (*json_str) ? GVJ_SUCCESS : GVJ_MEMORY_ERROR;
}

//free(json_str);
char *GpsVehicleJsonInterface()
{
    VehicleGPSData data = {0};
    char *json_str = NULL;
    char carid[30] = {0};
    uint8_t vinBuff[32] = {0};
    GnssLocation_t currentGnssInfo = {0};
    if(GetVehicleName(carid) == -1)
    {
        LOG_Error("GetCarNum fail");
        printf("GetCarNum fail \n");
        return NULL;
    }

    if (GetVIN(vinBuff) == -1)
    {
        LOG_Error("get vin Error");
        return NULL;
    }
    memcpy(data.vin,vinBuff,17);
    memcpy(data.vehicle_no,carid,15);
    data.timestamp =  GetCurrentTimeMS();
    data.service_id = GPSSERVICEID;
    data.sub_service_id = GPSSUBSERVICEID;
    memcpy(data.protocol_ver,GPSPROTOCOLVER,strlen(GPSPROTOCOLVER));
    GetGnssLocation(&currentGnssInfo);
    
    
    if(0 == (currentGnssInfo.gpsState & 0x1F))
    {
        data.gps_data.fix_status = 0;

    }else{
        data.gps_data.fix_status = 1;
    }

    if(0 == (currentGnssInfo.gpsState & 0x40))
    {
        data.gps_data.we = 'W';
    }
    else
    {
        data.gps_data.we  = 'E';
    }

    if(0 == (currentGnssInfo.gpsState & 0x80))
    {
        data.gps_data.ns = 'S';
    }
    else
    {
        data.gps_data.ns = 'N';
    }

    data.gps_data.longitude = currentGnssInfo.flongitude;
    data.gps_data.altitude = currentGnssInfo.flatitude;
    data.gps_data.hop = currentGnssInfo.seaHeight * 0.1f;
    data.gps_data.speed = currentGnssInfo.speed * 0.1f;
    data.gps_data.hdop = 0.01;
    data.gps_data.vdop = 0.01;
    data.gps_data.pdop = 0.01;
    data.gps_data.snr = 1;
    data.gps_data.satellites = currentGnssInfo.moonLinkNum;
    
    GVJ_Status status = CreateGpsVehicleJson(&data, &json_str);
    if(status != GVJ_SUCCESS)
    {
        LOG_Error("Create Gps Vehicle Json Fail %d",status);
    }
    //printf("gps json_str : %s \n",json_str);
    return json_str;
}

// 释放 Payload 内存
void FreeRemoteConfigurationPayload(Payload_t *payload) {
    if (!payload) return;

    if (payload->esc_function) 
    {
        free(payload->esc_function);
        payload->esc_function = NULL;
    }
    if (payload->external_discharge_func) 
    {
        free(payload->external_discharge_func);
        payload->external_discharge_func = NULL;
    }
    //if (payload->audio_prompt_file) free(payload->audio_prompt_file);
    // if (payload->mileage_sync) free(payload->mileage_sync);
    // if (payload->imu_calibration) free(payload->imu_calibration);
    // if (payload->left_front_tire_sensor_id) free(payload->left_front_tire_sensor_id);
    // if (payload->right_front_tire_sensor_id) free(payload->right_front_tire_sensor_id);
    // if (payload->left_rear_tire_sensor_id) free(payload->left_rear_tire_sensor_id);
    // if (payload->right_rear_tire_sensor_id) free(payload->right_rear_tire_sensor_id);
    // if (payload->cargo_door_configuration_file) free(payload->cargo_door_configuration_file);
}
// 释放整个消息内存
void FreeRemoteConfigurationVehicleMessage(VehicleMessage_t *msg) 
{
    if (!msg) return;

    if (msg->vehicle_no) 
        free(msg->vehicle_no);

    if (msg->vin) 
        free(msg->vin);

    if (msg->protocol_ver) 
        free(msg->protocol_ver);

    if (msg->seqid) 
        free(msg->seqid);
    
    FreeRemoteConfigurationPayload(&msg->payload);
    free(msg);
}
// 解析 Payload 对象
int ParseCRemoteonfigurationPayload(cJSON *payload_json, Payload_t *payload) 
{
    if (!payload_json || !cJSON_IsObject(payload_json)) {
        LOG_Error("Error: Invalid payload JSON\n");
        return -1;
    }

    // 初始化 payload 结构体
    memset(payload, 0, sizeof(Payload_t));

    // 解析 ESCFunction
    cJSON *esc_function = cJSON_GetObjectItem(payload_json, "ESCFunction");
    if(esc_function)
    {
        if (cJSON_IsString(esc_function) && esc_function->valuestring != NULL) {
        payload->esc_function = strdup(esc_function->valuestring);
        }else{
            LOG_Error("esc_function is not a string or is NULL");
        }
    }else
    {
        payload->esc_function = NULL;
    }
    
    // 解析 LowSpeedPromptVolume
    cJSON *low_speed_volume = cJSON_GetObjectItem(payload_json, "LowSpeedPromptVolume");
    if(low_speed_volume)
    {
        if (cJSON_IsNumber(low_speed_volume)) {
            payload->low_speed_prompt_volume = low_speed_volume->valueint;
        }else{
            LOG_Error("low_speed_volume is not a number type");
        }
    }else{

        payload->low_speed_prompt_volume = -1;
    }
    

    // 解析 ExternalDischargeFunc
    cJSON *external_discharge = cJSON_GetObjectItem(payload_json, "ExternalDischargeFunc");
    if(external_discharge)
    {
        if (cJSON_IsString(external_discharge) && external_discharge->valuestring != NULL) {
            payload->external_discharge_func = strdup(external_discharge->valuestring);
        }else{
            LOG_Error("external_discharge is not a string or is NULL");
        }
    }else{

        payload->external_discharge_func = NULL;
    }
    

    // // 解析 AudioPromptFile
    // cJSON *audio_prompt = cJSON_GetObjectItem(payload_json, "AudioPromptFile");
    // if (cJSON_IsString(audio_prompt) &&  audio_prompt->valuestring != NULL) {
    //     payload->audio_prompt_file = strdup(audio_prompt->valuestring);
    // }else{
    //     LOG_Error("audio_prompt is not a string or is NULL");
    // }

    // // 解析 MileageSync (16进制字符串)
    // cJSON *mileage_sync = cJSON_GetObjectItem(payload_json, "MileageSync");
    // if (cJSON_IsString(mileage_sync) && mileage_sync->valuestring != NULL) {
    //     payload->mileage_sync = strdup(mileage_sync->valuestring);
    // }else{
    //     LOG_Error("mileage_sync is not a string or is NULL");
    // }

    // // 解析 IMUCalibration
    // cJSON *imu_calibration = cJSON_GetObjectItem(payload_json, "IMUCalibration");
    // if (cJSON_IsString(imu_calibration) && imu_calibration->valuestring != NULL) {
    //     payload->imu_calibration = strdup(imu_calibration->valuestring);
    // }else{
    //     LOG_Error("imu_calibration is not a string or is NULL");
    // }

    // // 解析 StandardTirePressure
    // cJSON *tire_pressure = cJSON_GetObjectItem(payload_json, "StandardTirePressure");
    // if (cJSON_IsNumber(tire_pressure)) {
    //     payload->standard_tire_pressure = tire_pressure->valueint;
    // }else{
    //     LOG_Error("tire_pressure is not a number typr");
    // }

    // // 解析胎压传感器 ID
    // cJSON *left_front = cJSON_GetObjectItem(payload_json, "LeftFrontTireSensorID");
    // if (cJSON_IsString(left_front) && left_front->valuestring != NULL) {
    //     payload->left_front_tire_sensor_id = strdup(left_front->valuestring);
    // }else{
    //     LOG_Error("left_front is not a string or is NULL");
    // }

    // cJSON *right_front = cJSON_GetObjectItem(payload_json, "RightFrontTireSensorID");
    // if (cJSON_IsString(right_front)  &&  right_front->valuestring  != NULL) {
    //     payload->right_front_tire_sensor_id = strdup(right_front->valuestring);
    // }else{
    //     LOG_Error("left_front is not a string or is NULL");
    // }

    // cJSON *left_rear = cJSON_GetObjectItem(payload_json, "LeftRearTireSensorID");
    // if (cJSON_IsString(left_rear) && left_rear->valuestring != NULL) {
    //     payload->left_rear_tire_sensor_id = strdup(left_rear->valuestring);
    // }else{
    //     LOG_Error("left_front is not a string or is NULL");
    // }

    // cJSON *right_rear = cJSON_GetObjectItem(payload_json, "RightRearTireSensorID");
    // if (cJSON_IsString(right_rear) && right_rear->valuestring != NULL) {
    //     payload->right_rear_tire_sensor_id = strdup(right_rear->valuestring);
    // }else{
    //     LOG_Error("right_rear is not a string or is NULL");
    // }

    // // 解析货箱门配置文件
    // cJSON *cargo_door = cJSON_GetObjectItem(payload_json, "CargoDoorConfigurationFile");
    // if (cJSON_IsString(cargo_door) && cargo_door->valuestring != NULL) {
    //     payload->cargo_door_configuration_file = strdup(cargo_door->valuestring);
    // }else{
    //     LOG_Error("right_rear is not a string or is NULL");
    // }

    return 0;
}
// 解析完整的 JSON 消息
VehicleMessage_t* ParseVehicleRemoteConfigurationMessage(const char *json_str) 
{
    //printf("json_str = %s \n",json_str);
    if (!json_str) {
        LOG_Error("Error: JSON string is NULL");
        return NULL;
    }

    // 解析 JSON
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            LOG_Error("Error parsing JSON: %s", error_ptr);
        }
        return NULL;
    }

    // 分配内存
    VehicleMessage_t *msg = (VehicleMessage_t*)malloc(sizeof(VehicleMessage_t));
    if (!msg) {
        LOG_Error("Error: Memory allocation failed");
        cJSON_Delete(root);
        return NULL;
    }

    // 初始化结构体
    memset(msg, 0, sizeof(VehicleMessage_t));

    // 解析 VehicleNo
    cJSON *vehicle_no = cJSON_GetObjectItem(root, "VehicleNo");
    if (cJSON_IsString(vehicle_no) && vehicle_no->valuestring != NULL) {
        msg->vehicle_no = strdup(vehicle_no->valuestring);
    }else{
        LOG_Error("vehicle_no is not a string or is NULL");
    }

    // 解析 VIN
    cJSON *vin = cJSON_GetObjectItem(root, "VIN");
    if (cJSON_IsString(vin) && vin->valuestring != NULL) {
        msg->vin = strdup(vin->valuestring);
    }else{
        LOG_Error("vin is not a string or is NULL");
    }

    // 解析 ProtocolVer
    cJSON *protocol_ver = cJSON_GetObjectItem(root, "ProtocolVer");
    if (cJSON_IsString(protocol_ver) && protocol_ver->valuestring != NULL) {
        msg->protocol_ver = strdup(protocol_ver->valuestring);
    }else{
        LOG_Error("protocol_ver is not a string or is NULL");
    }

    // 解析 ServiceId (16进制)
    cJSON *service_id = cJSON_GetObjectItem(root, "ServiceId");
    if (cJSON_IsNumber(service_id)) {
        msg->service_id = (unsigned int)service_id->valueint;
    }else{
        LOG_Error("service_id is not a number type");
    }

    // 解析 SubServiceId (16进制)
    cJSON *sub_service_id = cJSON_GetObjectItem(root, "SubServiceId");
    if (cJSON_IsNumber(sub_service_id)) {
        msg->sub_service_id = (unsigned int)sub_service_id->valueint;
    }else{
        LOG_Error("sub_service_id is not a number type");
    }

    cJSON *seqId = cJSON_GetObjectItem(root, "SeqId");
    if (seqId->valuestring != NULL && cJSON_IsString(protocol_ver)) 
    {
        // 打印 SeqId 内容
        LOG_Debug("Extracted SeqId: %s", seqId->valuestring);
        msg->seqid = strdup(seqId->valuestring);
    } else {
        LOG_Error("SeqId field not found\n");
    }

    // 解析 TimeStamp
    cJSON *timestamp = cJSON_GetObjectItem(root, "TimeStamp");
    if (cJSON_IsNumber(timestamp)) {
        msg->timestamp = (long long)timestamp->valuedouble;
    }else{
        LOG_Error("timestamp is not a number type");
    }

    // 解析 Payload
    cJSON *payload_json = cJSON_GetObjectItem(root, "Payload");
    if (cJSON_IsObject(payload_json)) {
        if (ParseCRemoteonfigurationPayload(payload_json, &msg->payload) != 0) {
            LOG_Error("Error parsing payload");
            FreeRemoteConfigurationVehicleMessage(msg);
            cJSON_Delete(root);
            return NULL;
        }
    }

    cJSON_Delete(root);
    return msg;
}

// 打印消息内容（用于调试）
void PrintRemoteConfigurationVehicleMessage(const VehicleMessage_t *msg) 
{
    if (!msg) {
        LOG_Debug("Message is NULL\n");
        return;
    }

    LOG_Debug("=== Vehicle Message ===\n");
    LOG_Debug("VehicleNo: %s\n", msg->vehicle_no ? msg->vehicle_no : "NULL");
    LOG_Debug("VIN: %s\n", msg->vin ? msg->vin : "NULL");
    LOG_Debug("ProtocolVer: %s\n", msg->protocol_ver ? msg->protocol_ver : "NULL");
    LOG_Debug("ServiceId: 0x%04X\n", msg->service_id);
    LOG_Debug("SubServiceId: 0x%04X\n", msg->sub_service_id);
    LOG_Debug("TimeStamp: %lld\n", msg->timestamp);
    
    LOG_Debug("\n--- Payload ---\n");
    LOG_Debug("ESCFunction: %s\n", msg->payload.esc_function ? msg->payload.esc_function : "NULL");
    LOG_Debug("LowSpeedPromptVolume: %d%%\n", msg->payload.low_speed_prompt_volume);
    LOG_Debug("ExternalDischargeFunc: %s\n", msg->payload.external_discharge_func ? msg->payload.external_discharge_func : "NULL");
    // printf("AudioPromptFile: %s\n", msg->payload.audio_prompt_file ? msg->payload.audio_prompt_file : "NULL");
    // printf("MileageSync: %s\n", msg->payload.mileage_sync ? msg->payload.mileage_sync : "NULL");
    // printf("IMUCalibration: %s\n", msg->payload.imu_calibration ? msg->payload.imu_calibration : "NULL");
    // printf("StandardTirePressure: %d kPa\n", msg->payload.standard_tire_pressure);
    // printf("LeftFrontTireSensorID: %s\n", msg->payload.left_front_tire_sensor_id ? msg->payload.left_front_tire_sensor_id : "NULL");
    // printf("RightFrontTireSensorID: %s\n", msg->payload.right_front_tire_sensor_id ? msg->payload.right_front_tire_sensor_id : "NULL");
    // printf("LeftRearTireSensorID: %s\n", msg->payload.left_rear_tire_sensor_id ? msg->payload.left_rear_tire_sensor_id : "NULL");
    // printf("RightRearTireSensorID: %s\n", msg->payload.right_rear_tire_sensor_id ? msg->payload.right_rear_tire_sensor_id : "NULL");
    // printf("CargoDoorConfigurationFile: %s\n", msg->payload.cargo_door_configuration_file ? msg->payload.cargo_door_configuration_file : "NULL");
    LOG_Debug("========================\n\n");
}

// Response  Remote Configuration 
static char* CreateRemoteConfigurationVehicleJson(const RemoteConfigVehicleInfo *info) 
{
    // 创建根 JSON 对象
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        LOG_Error("Failed to create JSON object");
        return NULL;
    }

    // 添加基本字段
    cJSON_AddStringToObject(root, "VehicleNo", info->vehicleNo);
    
    cJSON_AddStringToObject(root, "VIN", info->vin);
    cJSON_AddStringToObject(root, "ProtocolVer", info->protocolVer);
    cJSON_AddNumberToObject(root, "ServiceId", info->serviceId);
    cJSON_AddNumberToObject(root, "SubServiceId", info->subServiceId);
    cJSON_AddNumberToObject(root, "TimeStamp", info->timeStamp);
    cJSON_AddStringToObject(root, "SeqId", info->seqid);
    // 创建 Response 对象
    cJSON *response = cJSON_CreateObject();
    if (!response) {
        cJSON_Delete(root);
        LOG_Error("Failed to create Response object");
        return NULL;
    }

    // 添加 Response 字段
    cJSON_AddStringToObject(response, "Result",info->ResponsePayload.result);
    cJSON_AddNumberToObject(response, "ErrorCode", info->ResponsePayload.error_code);

    // 创建 ConfigStatus 对象
    cJSON *configStatus = cJSON_CreateObject();
    if (!configStatus) {
        cJSON_Delete(response);
        cJSON_Delete(root);
        LOG_Error("Failed to create ConfigStatus object");
        return NULL;
    }

    // 添加 ConfigStatus 字段，从结构体中读取传入的值
    if(info->ResponsePayload.configStatus.escFunction)
    {
        cJSON_AddStringToObject(configStatus, "ESCFunction", info->ResponsePayload.configStatus.escFunction);

    }else{
        LOG_Debug("ResponsePayload configStatus escFunction null");
    }
    if(info->ResponsePayload.configStatus.lowSpeedPromptVolume )
    {
        cJSON_AddStringToObject(configStatus, "LowSpeedPromptVolume", info->ResponsePayload.configStatus.lowSpeedPromptVolume);
    }else{
        LOG_Debug("ResponsePayload configStatus lowSpeedPromptVolume null");
    }
    
    if(info->ResponsePayload.configStatus.externalDischargeFunc)
    {
        cJSON_AddStringToObject(configStatus, "ExternalDischargeFunc", info->ResponsePayload.configStatus.externalDischargeFunc);
    }else{
        LOG_Debug("RResponsePayload configStatus externalDischargeFunc null");
    }
    
    // cJSON_AddStringToObject(configStatus, "AudioPromptFile", info->ResponsePayload.configStatus.audioPromptFile);
    // cJSON_AddStringToObject(configStatus, "MileageSync", info->ResponsePayload.configStatus.mileageSync);
    // cJSON_AddStringToObject(configStatus, "IMUCalibration", info->ResponsePayload.configStatus.imuCalibration);
    // cJSON_AddStringToObject(configStatus, "StandardTirePressure", info->ResponsePayload.configStatus.standardTirePressure);
    // cJSON_AddStringToObject(configStatus, "LeftFrontTireSensorID", info->ResponsePayload.configStatus.leftFrontTireSensorID);
    // cJSON_AddStringToObject(configStatus, "RightFrontTireSensorID", info->ResponsePayload.configStatus.rightFrontTireSensorID);
    // cJSON_AddStringToObject(configStatus, "LeftRearTireSensorID", info->ResponsePayload.configStatus.leftRearTireSensorID);
    // cJSON_AddStringToObject(configStatus, "RightRearTireSensorID", info->ResponsePayload.configStatus.rightRearTireSensorID);
    // cJSON_AddStringToObject(configStatus, "CargoDoorConfigurationFile", info->ResponsePayload.configStatus.cargoDoorConfigurationFile);

    // 将 ConfigStatus 添加到 Response
    cJSON_AddItemToObject(response, "ConfigStatus", configStatus);

    // 将 Response 添加到根 JSON 对象
    cJSON_AddItemToObject(root, "Response", response);
    char *jsonString = cJSON_PrintUnformatted(root);
    return jsonString;
}


char *ResponseRemoteConfigurationCjaon(RemoteConfigResponse configResponse,char *seqid)
{

    char *json_str = NULL;
    char carid[30] = {0};
    uint8_t vinBuff[32] = {0};
    int ret = 0;
    RemoteConfigVehicleInfo data = {0};

    if(GetVehicleName(carid) == -1)
    {
        LOG_Error("GetCarNum fail");
        return NULL;
    }

    if (GetVIN(vinBuff) == -1)
    {
        LOG_Error("get vin Error");
        return NULL;
    }
    

    data.vin = (char *)vinBuff;
    data.vehicleNo = (char *)carid;
    data.timeStamp =  GetCurrentTimeMS();
    data.serviceId = RESREMOTECONFIGSERVICEID;
    data.subServiceId = RESREMOTECONFIGSUBSERVICEID;
    data.protocolVer = CANPROTOCOLVER;
    
    if(configResponse.result == 0)
    {
        data.ResponsePayload.result = "Success";

    }else if(configResponse.result == 1)
    {
        data.ResponsePayload.result = "Fail";

    }else{
        LOG_Error("data.ResponsePayload.result = %d",data.ResponsePayload.result);
    }

    if(configResponse.error_code == 0)
    {
        data.ResponsePayload.error_code = 0;
    }else{
        LOG_Error("data.ResponsePayload.result = %d",data.ResponsePayload.error_code);
        data.ResponsePayload.error_code = configResponse.error_code;
    }
    // ESC功能
    if(configResponse.configStatus.escFunction == 0)
    {
        data.ResponsePayload.configStatus.escFunction = "已配置";

    }else if(configResponse.configStatus.escFunction == 1)
    {
        data.ResponsePayload.configStatus.escFunction = "配置失败";

    }else{
        data.ResponsePayload.configStatus.escFunction = NULL;
    }
    // 低速提示的音量配置，单位%
    if(configResponse.configStatus.lowSpeedPromptVolume == 0)
    {
        data.ResponsePayload.configStatus.lowSpeedPromptVolume = "已配置";

    }else if(configResponse.configStatus.lowSpeedPromptVolume == 1)
    {
        data.ResponsePayload.configStatus.lowSpeedPromptVolume = "配置失败";

    }else{

        data.ResponsePayload.configStatus.lowSpeedPromptVolume = NULL;
    }
    // 对外放电功能配置 
    if(configResponse.configStatus.externalDischargeFunc == 0)
    {
        data.ResponsePayload.configStatus.externalDischargeFunc = "已配置";

    }else if(configResponse.configStatus.externalDischargeFunc == 1)
    {
        data.ResponsePayload.configStatus.externalDischargeFunc = "配置失败";

    }else{
        data.ResponsePayload.configStatus.externalDischargeFunc = NULL;
    }
    data.seqid = seqid;
    // // 里程同步，16进制报文   
    // if(configResponse.configStatus.mileageSync == 0)
    // {
    //     data.ResponsePayload.configStatus.mileageSync = "已配置";

    // }else if(configResponse.configStatus.mileageSync == 1)
    // {
    //     data.ResponsePayload.configStatus.mileageSync = "配置失败";

    // }

    // // IMU标0 
    // if(configResponse.configStatus.imuCalibration == 0)
    // {
    //     data.ResponsePayload.configStatus.imuCalibration = "已配置";

    // }else if(configResponse.configStatus.imuCalibration == 1)
    // {
    //     data.ResponsePayload.configStatus.imuCalibration = "配置失败";

    // }


    // // 标准胎压设定，单位 kPa
    // if(configResponse.configStatus.standardTirePressure == 0)
    // {
    //     data.ResponsePayload.configStatus.standardTirePressure = "已配置";

    // }else if(configResponse.configStatus.standardTirePressure == 1)
    // {
    //     data.ResponsePayload.configStatus.standardTirePressure = "配置失败";

    // }




    // // 左前胎压传感器 ID
    // if(configResponse.configStatus.leftFrontTireSensorID == 0)
    // {
    //     data.ResponsePayload.configStatus.leftFrontTireSensorID = "已配置";

    // }else if(configResponse.configStatus.leftFrontTireSensorID == 1)
    // {
    //     data.ResponsePayload.configStatus.leftFrontTireSensorID = "配置失败";

    // }

    // // 右前胎压传感器 ID
    // if(configResponse.configStatus.rightFrontTireSensorID == 0)
    // {
    //     data.ResponsePayload.configStatus.rightFrontTireSensorID = "已配置";

    // }else if(configResponse.configStatus.rightFrontTireSensorID == 1)
    // {
    //     data.ResponsePayload.configStatus.rightFrontTireSensorID = "配置失败";

    // }

    // // 左后胎压传感器 ID
    // if(configResponse.configStatus.leftRearTireSensorID == 0)
    // {
    //     data.ResponsePayload.configStatus.leftRearTireSensorID = "已配置";

    // }else if(configResponse.configStatus.leftRearTireSensorID == 1)
    // {
    //     data.ResponsePayload.configStatus.leftRearTireSensorID = "配置失败";

    // }

    // // 右后胎压传感器 ID
    // if(configResponse.configStatus.rightRearTireSensorID == 0)
    // {
    //     data.ResponsePayload.configStatus.rightRearTireSensorID = "已配置";

    // }else if(configResponse.configStatus.rightRearTireSensorID == 1)
    // {
    //     data.ResponsePayload.configStatus.rightRearTireSensorID = "配置失败";

    // }

    // // 货箱及门控配置文件路径
    // if(configResponse.configStatus.cargoDoorConfigurationFile == 0)
    // {
    //     data.ResponsePayload.configStatus.cargoDoorConfigurationFile = "已配置";

    // }else if(configResponse.configStatus.cargoDoorConfigurationFile == 1)
    // {
    //     data.ResponsePayload.configStatus.cargoDoorConfigurationFile = "配置失败";

    // }

    json_str = CreateRemoteConfigurationVehicleJson(&data);

    return json_str;
}

//Collision state configuration

// 释放 Contact Switch Payload 内存
void FreeContactSwitchPayload(ContactSwitchPayload_t *payload) {
    if (!payload) return;

    if (payload->first_level_contact_switch) free(payload->first_level_contact_switch);
    if (payload->first_level_contact_switch_release) free(payload->first_level_contact_switch_release);
    if (payload->second_level_contact_switch_release) free(payload->second_level_contact_switch_release);
    if (payload->third_level_contact_switch_release) free(payload->third_level_contact_switch_release);
}

// 释放整个车辆接触器消息内存
void FreeVehicleContactMessage(VehicleContactMessage_t *msg) {
    if (!msg) return;

    if (msg->vehicle_no) free(msg->vehicle_no);

    if (msg->vin) free(msg->vin);

    if (msg->protocol_ver) free(msg->protocol_ver);

    if (msg->seqid) free(msg->seqid);
    
    FreeContactSwitchPayload(&msg->payload);
    free(msg);
}

// 解析 Contact Switch Payload 对象
int ParseContactSwitchPayload(cJSON *payload_json, ContactSwitchPayload_t *payload) {
    if (!payload_json || !cJSON_IsObject(payload_json)) {
        LOG_Error("Error: Invalid payload JSON");
        return -1;
    }

    // 初始化 payload 结构体
    memset(payload, 0, sizeof(ContactSwitchPayload_t));

    // 解析 FirstLevelContactSwitch
    cJSON *first_level_switch = cJSON_GetObjectItem(payload_json, "FirstLevelContactSwitch");
    if(first_level_switch)
    {
        if (cJSON_IsString(first_level_switch) && first_level_switch->valuestring != NULL) {
            payload->first_level_contact_switch = strdup(first_level_switch->valuestring);
        } else {
            LOG_Error("Warning: FirstLevelContactSwitch not found or not string or is NULL");
        }
    }else{
        payload->first_level_contact_switch = NULL;
        LOG_Debug("FirstLevelContactSwitch NULL");
    }
  

    // 解析 FirstLevelContactSwitchRelease
    cJSON *first_level_release = cJSON_GetObjectItem(payload_json, "FirstLevelContactSwitchRelease");
    if(first_level_release)
    {
        if (cJSON_IsString(first_level_release) && first_level_release->valuestring != NULL) {
            payload->first_level_contact_switch_release = strdup(first_level_release->valuestring);
        } else {
            LOG_Error("Warning: FirstLevelContactSwitchRelease not found or not string or is NULL");
        }
    }else{
        payload->first_level_contact_switch_release = NULL;
        LOG_Debug("FirstLevelContactSwitchRelease NULL");
    }
   

    // 解析 SecondLevelContactSwitchRelease
    cJSON *second_level_release = cJSON_GetObjectItem(payload_json, "SecondLevelContactSwitchRelease");
    if(second_level_release)
    {
        if (cJSON_IsString(second_level_release) && second_level_release->valuestring != NULL) {
            payload->second_level_contact_switch_release = strdup(second_level_release->valuestring);
        } else {
            LOG_Error("Warning: SecondLevelContactSwitchRelease not found or not string or is NULL");
        }
    }else{
        payload->second_level_contact_switch_release = NULL;
        LOG_Debug("SecondLevelContactSwitchRelease NULL");
    }
    

    // 解析 ThirdLevelContactSwitchRelease
    cJSON *third_level_release = cJSON_GetObjectItem(payload_json, "ThirdLevelContactSwitchRelease");
    if(third_level_release)
    {
        if (cJSON_IsString(third_level_release) && third_level_release->valuestring != NULL) {
            payload->third_level_contact_switch_release = strdup(third_level_release->valuestring);
        } else {
            LOG_Error("Warning: ThirdLevelContactSwitchRelease not found or not string or is NULL");
        }
    }else{
        LOG_Debug("ThirdLevelContactSwitchRelease NULL");
        payload->third_level_contact_switch_release = NULL;
    }
  

    return 0;
}

// 解析完整的车辆接触器开关消息
VehicleContactMessage_t* ParseVehicleContactMessage(const char *json_str) 
{
    if (!json_str) {
        LOG_Error("Error: JSON string is NULL");
        return NULL;
    }

    // 解析 JSON
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            LOG_Error("Error parsing JSON: %s", error_ptr);
        }
        return NULL;
    }

    // 分配内存
    VehicleContactMessage_t *msg = (VehicleContactMessage_t*)malloc(sizeof(VehicleContactMessage_t));
    if (!msg) {
        LOG_Error("Error: Memory allocation failed");
        cJSON_Delete(root);
        return NULL;
    }

    // 初始化结构体
    memset(msg, 0, sizeof(VehicleContactMessage_t));

    // 解析 VehicleNo
    cJSON *vehicle_no = cJSON_GetObjectItem(root, "VehicleNo");
    if (cJSON_IsString(vehicle_no) && vehicle_no->valuestring != NULL) {
        msg->vehicle_no = strdup(vehicle_no->valuestring);
    } else {
        LOG_Error("Warning: VehicleNo not found or not string or is NULL\n");
    }

    // 解析 VIN
    cJSON *vin = cJSON_GetObjectItem(root, "VIN");
    if (cJSON_IsString(vin) && vin->valuestring != NULL) {
        msg->vin = strdup(vin->valuestring);
    } else {
        LOG_Error("Warning: VIN not found or not string or is NULL\n");
    }

    // 解析 ProtocolVer
    cJSON *protocol_ver = cJSON_GetObjectItem(root, "ProtocolVer");
    if (cJSON_IsString(protocol_ver) && protocol_ver->valuestring != NULL) {
        msg->protocol_ver = strdup(protocol_ver->valuestring);
    } else {
        LOG_Error("Warning: ProtocolVer not found or not string or is NULL\n");
    }
    
    cJSON *seqId = cJSON_GetObjectItem(root, "SeqId");
    if (seqId->valuestring != NULL && cJSON_IsString(protocol_ver)) 
    {
        // 打印 SeqId 内容
        LOG_Debug("Extracted SeqId: %s", seqId->valuestring);
        msg->seqid = strdup(seqId->valuestring);
    } else {
        LOG_Error("SeqId field not found\n");
    }
    // 解析 ServiceId (16进制)
    cJSON *service_id = cJSON_GetObjectItem(root, "ServiceId");
    if (cJSON_IsNumber(service_id)) {
        msg->service_id = (unsigned int)service_id->valueint;
    } else {
        LOG_Error("Warning: ServiceId not found or not number\n");
    }

    // 解析 SubServiceId (16进制)
    cJSON *sub_service_id = cJSON_GetObjectItem(root, "SubServiceId");
    if (cJSON_IsNumber(sub_service_id)) {
        msg->sub_service_id = (unsigned int)sub_service_id->valueint;
    } else {
        LOG_Error("Warning: SubServiceId not found or not number\n");
    }

    // 解析 TimeStamp
    cJSON *timestamp = cJSON_GetObjectItem(root, "TimeStamp");
    if (cJSON_IsNumber(timestamp)) {
        msg->timestamp = (long long)timestamp->valuedouble;
    } else {
        LOG_Error("Warning: TimeStamp not found or not number\n");
    }

    // 解析 Payload
    cJSON *payload_json = cJSON_GetObjectItem(root, "Payload");
    if (cJSON_IsObject(payload_json)) {
        if (ParseContactSwitchPayload(payload_json, &msg->payload) != 0) {
            LOG_Error("Error parsing contact switch payload\n");
            FreeVehicleContactMessage(msg);
            cJSON_Delete(root);
            return NULL;
        }
    } else {
        LOG_Error("Warning: Payload not found or not object\n");
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_Delete(root);
    return msg;
}


// 打印车辆接触器消息内容（用于调试）
void PrintVehicleContactMessage(const VehicleContactMessage_t *msg) 
{
    if (!msg) {
        LOG_Debug("Message is NULL\n");
        return;
    }

    LOG_Debug("=== Vehicle Contact Switch Message ===\n");
    LOG_Debug("VehicleNo: %s\n", msg->vehicle_no ? msg->vehicle_no : "NULL");
    LOG_Debug("VIN: %s\n", msg->vin ? msg->vin : "NULL");
    LOG_Debug("ProtocolVer: %s\n", msg->protocol_ver ? msg->protocol_ver : "NULL");
    LOG_Debug("ServiceId: 0x%04X\n", msg->service_id);
    LOG_Debug("SubServiceId: 0x%04X\n", msg->sub_service_id);
    LOG_Debug("TimeStamp: %lld\n", msg->timestamp);
    
    LOG_Debug("\n--- Contact Switch Payload ---\n");
    LOG_Debug("FirstLevelContactSwitch: %s\n", 
           msg->payload.first_level_contact_switch ? msg->payload.first_level_contact_switch : "NULL");
    LOG_Debug("FirstLevelContactSwitchRelease: %s\n", 
           msg->payload.first_level_contact_switch_release ? msg->payload.first_level_contact_switch_release : "NULL");
    LOG_Debug("SecondLevelContactSwitchRelease: %s\n", 
           msg->payload.second_level_contact_switch_release ? msg->payload.second_level_contact_switch_release : "NULL");
    LOG_Debug("ThirdLevelContactSwitchRelease: %s\n", 
           msg->payload.third_level_contact_switch_release ? msg->payload.third_level_contact_switch_release : "NULL");
    LOG_Debug("==================================\n\n");
}


// // 检查是否所有接触器都已释放
// int are_all_contact_switches_released(const VehicleContactMessage_t *msg) {
//     if (!msg) return 0;

//     int released = 1;
    
//     if (msg->payload.first_level_contact_switch_release) {
//         released &= (strcmp(msg->payload.first_level_contact_switch_release, "解除") == 0);
//     }
    
//     if (msg->payload.second_level_contact_switch_release) {
//         released &= (strcmp(msg->payload.second_level_contact_switch_release, "解除") == 0);
//     }
    
//     if (msg->payload.third_level_contact_switch_release) {
//         released &= (strcmp(msg->payload.third_level_contact_switch_release, "解除") == 0);
//     }
    
//     return released;
// }

// 创建 JSON 对象的函数
char* CreateVehicleContactJson(const ContactVehicleInfoResponse *info) {
    // 创建根 JSON 对象
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        LOG_Error("Failed to create JSON object");
        return NULL;
    }

    // 添加基本字段
    cJSON_AddStringToObject(root, "VehicleNo", info->vehicleNo);
    cJSON_AddStringToObject(root, "VIN", info->vin);
    cJSON_AddStringToObject(root, "ProtocolVer", info->protocolVer);
    cJSON_AddStringToObject(root, "SeqId", info->seqid);
    cJSON_AddNumberToObject(root, "ServiceId", info->serviceId);
    cJSON_AddNumberToObject(root, "SubServiceId", info->subServiceId);
    cJSON_AddNumberToObject(root, "TimeStamp", info->timeStamp);

    // 创建 Response 对象
    cJSON *response = cJSON_CreateObject();
    if (!response) {
        cJSON_Delete(root);
        LOG_Error("Failed to create Response object");
        return NULL;
    }

    // 添加 Response 字段
    cJSON_AddStringToObject(response, "Result", info->configStatus.result);
    cJSON_AddNumberToObject(response, "ErrorCode", info->configStatus.error_code);
    
    // 创建 ConfigStatus 对象
    cJSON *configStatus = cJSON_CreateObject();
    if (!configStatus) {
        cJSON_Delete(response);
        cJSON_Delete(root);
        LOG_Error("Failed to create ConfigStatus object");
        return NULL;
    }

    // 添加 ConfigStatus 字段，从结构体中读取传入的值
    if(info->configStatus.contact_ctatus.firstLevelContactSwitch)
    {
        cJSON_AddStringToObject(configStatus, "FirstLevelContactSwitch", info->configStatus.contact_ctatus.firstLevelContactSwitch);
    }else{
        LOG_Debug("FirstLevelContactSwitch null");
    }
    
    if(info->configStatus.contact_ctatus.firstLevelContactSwitchRelease)
    {
        cJSON_AddStringToObject(configStatus, "FirstLevelContactSwitchRelease", info->configStatus.contact_ctatus.firstLevelContactSwitchRelease);
    }else{
        LOG_Debug("FirstLevelContactSwitchRelease null");
    }
    
    if(info->configStatus.contact_ctatus.secondLevelContactSwitchRelease)
    {
        cJSON_AddStringToObject(configStatus, "SecondLevelContactSwitchRelease", info->configStatus.contact_ctatus.secondLevelContactSwitchRelease);
    }else{
        LOG_Debug("SecondLevelContactSwitchRelease null");
    }
    
    if(info->configStatus.contact_ctatus.thirdLevelContactSwitchRelease)
    {
        cJSON_AddStringToObject(configStatus, "ThirdLevelContactSwitchRelease", info->configStatus.contact_ctatus.thirdLevelContactSwitchRelease);
    }else{
        LOG_Debug("ThirdLevelContactSwitchRelease null");
    }
    

    // 将 ConfigStatus 添加到 Response
    cJSON_AddItemToObject(response, "ConfigStatus", configStatus);

    // 将 Response 添加到根 JSON 对象
    cJSON_AddItemToObject(root, "Response", response);
    char* json_str = cJSON_PrintUnformatted(root);
    
    cJSON_Delete(root);
    return json_str;
}


char *ResponseVehicleContactCjaon(ContactConfigResultStatusResponse result,char *seqid)
{

    char *json_str = NULL;
    char carid[30] = {0};
    uint8_t vinBuff[32] = {0};
    int ret = 0;
    ContactVehicleInfoResponse data = {0};

    if(GetVehicleName(carid) == -1)
    {
        LOG_Error("GetCarNum fail");
        return NULL;
    }

    if (GetVIN(vinBuff) == -1)
    {
        LOG_Error("get vin Error");
        return NULL;
    }
    

    data.vin = (char *)vinBuff;
    data.vehicleNo = (char *)carid;
    data.timeStamp =  GetCurrentTimeMS();
    data.serviceId = RESCONTACTSERVICEID;
    data.subServiceId = RESCONTACTSUBSERVICEID;
    data.protocolVer = PROTOCOLVER;
    data.seqid = seqid;
    if(result.result == 0)
    {
        data.configStatus.result = "Success";
    }else if(result.result == 1)
    {
        data.configStatus.result = "Fail";
    }else{

        LOG_Error("result.result = %d",result.result);
    }

    data.configStatus.error_code = result.error_code;
    LOG_Debug("result.error_code = %d",result.error_code);

    if(result.contact_ctatus.firstLevelContactSwitch == 0)
    {
        data.configStatus.contact_ctatus.firstLevelContactSwitch = "已执行";

    }else if(result.contact_ctatus.firstLevelContactSwitch == 1)
    {
        data.configStatus.contact_ctatus.firstLevelContactSwitch = "执行失败";
    }else if(result.contact_ctatus.firstLevelContactSwitch == -1)
    {
        data.configStatus.contact_ctatus.firstLevelContactSwitch = NULL;

    }else{
        LOG_Error("firstLevelContactSwitch value unknow :%d",result.contact_ctatus.firstLevelContactSwitch);
    }

    if(result.contact_ctatus.firstLevelContactSwitchRelease == 0)
    {
        data.configStatus.contact_ctatus.firstLevelContactSwitchRelease = "已执行";

    }else if(result.contact_ctatus.firstLevelContactSwitchRelease == 1)
    {
        data.configStatus.contact_ctatus.firstLevelContactSwitchRelease = "执行失败";

    }else if(result.contact_ctatus.firstLevelContactSwitchRelease == -1)
    {

        data.configStatus.contact_ctatus.firstLevelContactSwitchRelease = NULL;
    }else{
        LOG_Error("firstLevelContactSwitchRelease value unknow :%d",result.contact_ctatus.firstLevelContactSwitchRelease);
    }


    if(result.contact_ctatus.secondLevelContactSwitchRelease == 0)
    {
        data.configStatus.contact_ctatus.secondLevelContactSwitchRelease = "已执行";

    }else if(result.contact_ctatus.secondLevelContactSwitchRelease == 1)
    {
        data.configStatus.contact_ctatus.secondLevelContactSwitchRelease = "执行失败";
        
    }else if(result.contact_ctatus.secondLevelContactSwitchRelease == -1)
    {
        data.configStatus.contact_ctatus.secondLevelContactSwitchRelease = NULL;
    }else{
        LOG_Error("secondLevelContactSwitchRelease value unknow :%d",result.contact_ctatus.secondLevelContactSwitchRelease);
    }


    if(result.contact_ctatus.thirdLevelContactSwitchRelease == 0)
    {
        data.configStatus.contact_ctatus.thirdLevelContactSwitchRelease = "已执行";
    }else if(result.contact_ctatus.thirdLevelContactSwitchRelease == 1)
    {
        data.configStatus.contact_ctatus.thirdLevelContactSwitchRelease = "执行失败";
    }else if(result.contact_ctatus.thirdLevelContactSwitchRelease == -1)
    {
        data.configStatus.contact_ctatus.thirdLevelContactSwitchRelease = NULL;
    }else{
        LOG_Error("thirdLevelContactSwitchRelease value unknow :%d",result.contact_ctatus.thirdLevelContactSwitchRelease);
    }
    
    return CreateVehicleContactJson(&data) ;
}



// 封装 JSON 的函数
char* CreateWillJsonString(const char* id, const char* t, const char* vin, const char* vehicleNo) 
{
    // 创建一个 JSON 对象
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        LOG_Error("Failed to create JSON object");
        return NULL;
    }

    // 设置 JSON 对象的属性
    cJSON_AddStringToObject(json, "id", id);
    cJSON_AddStringToObject(json, "t", t);
    cJSON_AddStringToObject(json, "vin", vin);
    cJSON_AddStringToObject(json, "vehicleNo", vehicleNo);

    // 将 JSON 对象转换为字符串
    char *json_string = cJSON_PrintUnformatted(json);
    if (json_string == NULL) {
        LOG_Error("Failed to print JSON");
        cJSON_Delete(json);
        return NULL;
    }

    // 清理 JSON 对象
    cJSON_Delete(json);

    return json_string;
}


char* CreateWillJsonInterface()
{
    char carid[30] = {0};
    uint8_t vinBuff[32] = {0};
    int ret = 0;
    char *str = NULL;
    if(GetVehicleName(carid) == -1)
    {
        LOG_Error("GetCarNum fail");
        return NULL;
    }

    if (GetVIN(vinBuff) == -1)
    {
        LOG_Error("get vin Error");
        return NULL;
    }
    
    str =  CreateWillJsonString(carid, "tbox", (char*)vinBuff, carid);
    if(str == NULL)
    {
        str = (char *)malloc(256);
        if(!str)
        {
            LOG_Error("malloc fail");
            return NULL;
        }
        sprintf(str,"{\"id\":\"%s\",\"t\":\"tbox\",\"vin\":\"%s\",\"vehicleNo\":\"%s\"}",carid,vinBuff,carid);
    }
    return str;
}


//free(playloadString);
char* OtaExtractPlayload(const char* jsonString,char **seqid) 
{
    // 解析 JSON 字符串
    char *playloadString = NULL;
    cJSON *json = cJSON_Parse(jsonString);
    if (json == NULL) {
        LOG_Error("Failed to parse JSON\n");
        return NULL;
    }

    cJSON *seqId = cJSON_GetObjectItem(json, "SeqId");
    if (seqId != NULL) {
        // 打印 SeqId 内容
        LOG_Debug("Extracted SeqId: %s\n", seqId->valuestring);
        *seqid = strdup(seqId->valuestring);
    } else {
        LOG_Error("SeqId field not found\n");
    }
    // 查找 Playload 字段
    cJSON *playload = cJSON_GetObjectItem(json, "Payload");
    if (playload != NULL) {
        // 打印 Playload 内容
        playloadString = cJSON_PrintUnformatted(playload);
        if (playloadString != NULL) {
            LOG_Debug("Extracted Playload: %s\n", playloadString);
            
        } else {
            LOG_Error("Failed to print Playload\n");
        }
    } else {
        LOG_Error("Playload field not found\n");
    }

    // 清理 JSON 对象
    cJSON_Delete(json);
    return playloadString;
}





// 创建 JSON 对象的函数
static char* OtaCreateJson(const OtaVehicleData *data) 
{
    // 创建一个 JSON 对象
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        LOG_Error("Failed to create JSON object");
        return NULL;
    }

    // 添加字段到 JSON
    cJSON_AddStringToObject(json, "VehicleNo", data->VehicleNo);
    cJSON_AddStringToObject(json, "Vin", data->Vin);
    cJSON_AddStringToObject(json, "ProtocolVer", data->ProtocolVer);
    cJSON_AddNumberToObject(json, "ServiceId", data->ServiceId);
    cJSON_AddNumberToObject(json, "SubServiceId", data->SubServiceId);
    cJSON_AddNumberToObject(json, "TimeStamp", data->TimeStamp);
    cJSON_AddStringToObject(json, "SeqId", data->SeqId);

    // 创建 Playload 对象并添加到 JSON 对象
    cJSON *playloadJson = cJSON_CreateObject();
    if (playloadJson == NULL) {
        LOG_Error("Failed to create Playload object");
        cJSON_Delete(json);
        return NULL;
    }
    cJSON_AddNumberToObject(playloadJson, "Result", data->playload.Result);
    cJSON_AddItemToObject(json, "Payload", playloadJson);

    // 打印 JSON 对象为字符串
    char *jsonString = cJSON_PrintUnformatted(json);
    if (jsonString == NULL) {
        LOG_Error("Failed to print JSON");
        cJSON_Delete(json);
        return NULL;
    }

    // 清理 JSON 对象
    cJSON_Delete(json);

    return jsonString;
}


char* OtaCreateJsonInterface(char *seqid,unsigned char result) 
{
   OtaVehicleData data = {0};
   char *json_str = NULL;
    char carid[30] = {0};
    uint8_t vinBuff[32] = {0};
    int ret = 0;
   
    if(GetVehicleName(carid) == -1)
    {
        LOG_Error("GetCarNum fail");
        return NULL;
    }

    if (GetVIN(vinBuff) == -1)
    {
        LOG_Error("get vin Error");
        return NULL;
    }
    

    data.Vin = (char *)vinBuff;
    data.VehicleNo = (char *)carid;
    data.TimeStamp =  GetCurrentTimeMS();
    data.ServiceId = RESREMOTEOTASERVICEID;
    data.SubServiceId = RESREMOTEOTASUBSERVICEID;
    data.ProtocolVer = CANPROTOCOLVER;
    data.SeqId = seqid;
    data.playload.Result = result;

    return  OtaCreateJson(&data) ;
}



// 从 JSON 字符串中提取 ServiceId 字段的函数
int GetServiceIdFromJson(const char* jsonString) 
{
    LOG_Debug("jsonString = %s \n",jsonString);
    // 解析 JSON 字符串
    cJSON *json = cJSON_Parse(jsonString);
    if (json == NULL) {
        LOG_Error("Failed to parse JSON");
         const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            LOG_Error("Error parsing JSON: %s\n", error_ptr);
        }
        return -1;  // 返回 -1 表示解析失败
    }

    // 查找 ServiceId 字段
    cJSON *serviceIdItem = cJSON_GetObjectItem(json, "ServiceId");
    if (serviceIdItem != NULL && cJSON_IsNumber(serviceIdItem)) {
        int serviceId = serviceIdItem->valueint;
        cJSON_Delete(json);
        return serviceId;
    } else {
        LOG_Error("ServiceId field not found or is not a number");
        cJSON_Delete(json);  // 清理 JSON 对象
        return -1;
    }

}

void FreeOtaAwakenVehicleData(OtaAwakenVehicleData *data) 
{
    free(data->VehicleNo);
    free(data->Vin);
    free(data->ProtocolVer);
    free(data->SeqId);
}

//OTA 唤醒代码
int OtaAwakenParseVehicleCjson(const char *json_string, OtaAwakenVehicleData *data) 
{
    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL) {
        LOG_Error("Error parsing JSON");
        return -1;
    }

    // 解析各个字段
    cJSON *vehicleNo = cJSON_GetObjectItem(json, "VehicleNo");
    cJSON *vin = cJSON_GetObjectItem(json, "Vin");
    cJSON *protocolVer = cJSON_GetObjectItem(json, "ProtocolVer");
    cJSON *serviceId = cJSON_GetObjectItem(json, "ServiceId");
    cJSON *subServiceId = cJSON_GetObjectItem(json, "SubServiceId");
    cJSON *timeStamp = cJSON_GetObjectItem(json, "TimeStamp");
    cJSON *seqId = cJSON_GetObjectItem(json, "SeqId");
    cJSON *playload = cJSON_GetObjectItem(json, "Playload");

    // 为字符串分配内存并复制内容
    if (vehicleNo && cJSON_IsString(vehicleNo)) {
        data->VehicleNo = strdup(vehicleNo->valuestring);
    } else {
        data->VehicleNo = NULL;
    }

    if (vin && cJSON_IsString(vin)) {
        data->Vin = strdup(vin->valuestring);
    } else {
        data->Vin = NULL;
    }

    if (protocolVer && cJSON_IsString(protocolVer)) {
        data->ProtocolVer = strdup(protocolVer->valuestring);
    } else {
        data->ProtocolVer = NULL;
    }

    if (serviceId && cJSON_IsNumber(serviceId)) {
        data->ServiceId = serviceId->valueint;
    }

    if (subServiceId && cJSON_IsNumber(subServiceId)) {
        data->SubServiceId = subServiceId->valueint;
    }

    if (timeStamp && cJSON_IsNumber(timeStamp)) {
        data->TimeStamp = timeStamp->valuedouble;
    }

    if (seqId && cJSON_IsString(seqId)) {
        data->SeqId = strdup(seqId->valuestring);
    } else {
        data->SeqId = NULL;
    }

    cJSON *silence = cJSON_GetObjectItem(playload, "Silence");
    if (silence && cJSON_IsNumber(silence)) {
        data->Playload.Silence = silence->valueint;
    }

    // 释放 JSON 对象
    cJSON_Delete(json);

    return 0;
}

//封装回复
char *OtaAwakenVehicleCreateJSON(const AwakenReply *data) 
{
    cJSON *json = cJSON_CreateObject(); // 创建 JSON 对象
    if (json == NULL) {
        LOG_Error("Failed to create JSON object\n");
        return NULL;
    }

    // 添加数据到 JSON 对象
    cJSON_AddStringToObject(json, "VehicleNo", data->VehicleNo);
    cJSON_AddStringToObject(json, "Vin", data->Vin);
    cJSON_AddStringToObject(json, "ProtocolVer", data->ProtocolVer);
    cJSON_AddNumberToObject(json, "ServiceId", data->ServiceId);
    cJSON_AddNumberToObject(json, "SubServiceId", data->SubServiceId);
    cJSON_AddNumberToObject(json, "TimeStamp", data->TimeStamp);
    cJSON_AddStringToObject(json, "SeqId", data->SeqId);

    // 创建 Playload 对象
    cJSON *playload = cJSON_CreateObject();
    if (playload == NULL) {
        cJSON_Delete(json);
        LOG_Error( "Failed to create Playload object\n");
        return NULL;
    }

    // 添加 Wakeup 到 Playload 对象
    cJSON_AddNumberToObject(playload, "Wakeup", data->Playload.Wakeup);
    cJSON_AddItemToObject(json, "Payload", playload); // 将 Payload 添加到 JSON

    // 生成 JSON 字符串
    char *json_string = cJSON_PrintUnformatted(json);
    cJSON_Delete(json); // 释放 JSON 对象
    return json_string; // 返回 JSON 字符串
}


char* OtaWakenVehicleCreateJSONInterface(char *seqid ,unsigned char result)
{
    char *json_str = NULL;
    char carid[30] = {0};
    uint8_t vinBuff[32] = {0};
    AwakenReply data = {0};
    int ret = 0;

    if(GetVehicleName(carid) == -1)
    {
        LOG_Error("GetCarNum fail");
        return NULL;
    }

    if (GetVIN(vinBuff) == -1)
    {
        LOG_Error("get vin Error");
        return NULL;
    }
    

    data.Vin = (char *)vinBuff;
    data.VehicleNo = (char *)carid;
    data.TimeStamp =  GetCurrentTimeMS();
    data.ServiceId = RESREMOTEOTASWAKEERVICEID;
    data.SubServiceId = RESREMOTEOTAWAKESUBSERVICEID;
    data.ProtocolVer = CANPROTOCOLVER;
    data.SeqId = seqid;
    
    
    data.Playload.Wakeup = result;
    return OtaAwakenVehicleCreateJSON(&data);
}

char* OtaGetSeqId(const char* jsonString) 
{
    // 解析 JSON 字符串
    char *seqid = NULL;
    cJSON *json = cJSON_Parse(jsonString);
    if (json == NULL) {
        LOG_Error("Failed to parse JSON\n");
        return NULL;
    }
    cJSON *seqId = cJSON_GetObjectItem(json, "SeqId");
    if (seqId != NULL) {
        // 打印 SeqId 内容
        LOG_Debug("Extracted SeqId: %s\n", seqId->valuestring);
        seqid = strdup(seqId->valuestring);
    } else {
        LOG_Error("SeqId field not found\n");
    }

    // 清理 JSON 对象
    cJSON_Delete(json);
    return seqid;
}




int GetHttpErrValue(const char *json_string) 
{
    cJSON *json = cJSON_Parse(json_string);  // 解析 JSON 字符串
    if (json == NULL) {
        LOG_Debug("Error parsing JSON\n");
        return -1;  // 返回 -1 表示解析失败
    }

    cJSON *err = cJSON_GetObjectItem(json, "err");  // 获取 err 字段
    if (err && cJSON_IsNumber(err)) {
        int errValue = err->valueint;  // 获取 err 的值
        cJSON_Delete(json);  // 释放 JSON 对象
        return errValue;  // 返回 err 值
    }

    cJSON_Delete(json);  // 释放 JSON 对象
    return -1;  // 返回 -1 表示未找到 err 或其不是数字
}


// 函数接口，用于解析 JSON 并填充 LogData 结构
int CanLogParseJson(const char *jsonString, LogData *logData) 
{
    cJSON *json = cJSON_Parse(jsonString);
    if (json == NULL) {
        LOG_Error("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }

    // 提取字段
    cJSON *logStart = cJSON_GetObjectItem(json, "logStart");
    cJSON *logEnd = cJSON_GetObjectItem(json, "logEnd");
    cJSON *batchNumber = cJSON_GetObjectItem(json, "batchNumber");
    cJSON *cmdType = cJSON_GetObjectItem(json, "cmdType");

    // 检查字段并填充数据结构
    if (cJSON_IsNumber(logStart)) {
        logData->logStart = (long long)logStart->valuedouble;

    }else 
    {
        LOG_Error("logStart is not a number\n");
    }

    if (cJSON_IsNumber(logEnd)) {
        logData->logEnd = (long long)logEnd->valuedouble;
    } else {
        LOG_Error("logEnd is not a number\n");
    }

    if (cJSON_IsString(batchNumber) && (batchNumber->valuestring != NULL)) {
        logData->batchNumber = strdup(batchNumber->valuestring);
    } else {
        LOG_Error("batchNumber is not a number\n");
    }

    if (cJSON_IsString(cmdType) && (cmdType->valuestring != NULL)) {
        logData->cmdType = strdup(cmdType->valuestring);
        
    } else {
        LOG_Error("cmdType is not a string\n");
    }

    // 释放 cJSON 对象
    cJSON_Delete(json);
    return 0;
}

void FreeCanLogParseJson(LogData *logData)
{
    if(!logData)
    {
        return;
    }

    if(logData->batchNumber)
    {
        free(logData->batchNumber);
        logData->batchNumber = NULL;
    }

    if(logData->cmdType)
    {
        free(logData->cmdType);
        logData->cmdType = NULL;
    }

}



int CreateReplyLogDataJson(const ReplyLogData* vehicleInfo, char** jsonString) 
{
    // 创建一个JSON对象
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        LOG_Error("cJSON Create Object Fail");
        return -1; // 返回创建失败的错误码
    }

    // 添加数据到JSON对象
    cJSON_AddStringToObject(json, "vehicleNo", vehicleInfo->vehicleNo);
    cJSON_AddStringToObject(json, "vin", vehicleInfo->vin);
    cJSON_AddStringToObject(json, "batchNumber", vehicleInfo->batchNumber);
    cJSON_AddStringToObject(json, "fileKey", vehicleInfo->fileKey);
    cJSON_AddNumberToObject(json, "status", vehicleInfo->status);
    
    // 转换JSON对象为字符串
    *jsonString = cJSON_PrintUnformatted(json);
    if (*jsonString == NULL) {
        cJSON_Delete(json); // 释放JSON对象
        LOG_Error("cJSON Print Fail");
        return -1; // 返回转换失败的错误码
    }

    // 释放JSON对象
    cJSON_Delete(json);

    return 0;
}

int ReplyLogDataJsonInterface(ReplyLogDataParameter parameter,char **jsonstring)
{
    char *json_str = NULL;
    char carid[30] = {0};
    uint8_t vinBuff[32] = {0};
    ReplyLogData vehicleInfo = {0};
    int ret = 0;

    if(GetVehicleName(carid) == -1)
    {
        LOG_Error("GetCarNum fail");
        return -1;
    }

    if (GetVIN(vinBuff) == -1)
    {
        LOG_Error("get vin Error");
        return -1;
    }
    

    vehicleInfo.vin = (char *)vinBuff;
    vehicleInfo.vehicleNo = (char *)carid;
    vehicleInfo.batchNumber = parameter.batchNumber;
    vehicleInfo.fileKey = parameter.fileKey;
    vehicleInfo.status = parameter.status;

  return  CreateReplyLogDataJson(&vehicleInfo,jsonstring);
}
// JSON 解析函数  HttpCanJsonResponse
int ParseHttpCanJsonResponse(const char* jsonString, HttpCanJsonResponse* response) {
    // 检查输入参数是否有效
    if (jsonString == NULL || response == NULL) 
    {
        LOG_Error("jsonString NULL / response NULL");
        return -1; // 参数无效
    }

    //解析 JSON 字符串
    cJSON* json = cJSON_Parse(jsonString);
    if (json == NULL) {
        LOG_Error("cJSON Parse Fail");
        return -1; // JSON 解析失败
    }

    // 提取 "data" 字段
    cJSON* data = cJSON_GetObjectItem(json, "data");
    if (cJSON_IsString(data) && (data->valuestring != NULL)) {
        response->data = strdup(data->valuestring);
    } else {
        LOG_Error("data error");
    }

    // 提取 "code" 字段
    cJSON* code = cJSON_GetObjectItem(json, "code");
    if (cJSON_IsNumber(code)) {
        response->code = code->valueint;
    } else {
        
        LOG_Error("code error");
    }

    // 提取 "msg" 字段
    cJSON* msg = cJSON_GetObjectItem(json, "msg");
    if (cJSON_IsString(msg) && (msg->valuestring != NULL)) 
    {
        response->msg = strdup(msg->valuestring);
    }else{
        LOG_Error("msg error");
    }

    cJSON_Delete(json);

    return 0;
}

void FreeHttpCanJson(HttpCanJsonResponse* response)
{
    if(!response)
    {
        return;
    }

    if(response->data)
    {
        free(response->data);
        response->data = NULL;
    }

    if(response->msg)
    {
        free(response->msg);
        response->msg = NULL;
    }
}


// 函数接口：将结构体转换为 JSON
char* SyncOtaFieldVehicleInfoCreatJSON(const SyncOtaFieldVehicleInfo *info) 
{
    cJSON *json = cJSON_CreateObject();
    
    if (!json) 
    {
        LOG_Error("cJSON Create Object Fail");
        return NULL;
    }
    

    cJSON_AddStringToObject(json, "VehicleNo", info->VehicleNo);
    cJSON_AddStringToObject(json, "Vin", info->Vin);
    cJSON_AddStringToObject(json, "ProtocolVer", info->ProtocolVer);
    cJSON_AddNumberToObject(json, "ServiceId", info->ServiceId);
    cJSON_AddNumberToObject(json, "SubServiceId", info->SubServiceId);
    cJSON_AddNumberToObject(json, "TimeStamp", info->TimeStamp);
    cJSON_AddStringToObject(json, "SeqId", info->SeqId);

    // 创建 Payload 对象
    //cJSON *payload = cJSON_CreateObject();
    cJSON *payload = cJSON_Parse(info->Field);
    if (payload == NULL) {
        LOG_Error("Error parsing JSON");

    }
    cJSON_AddItemToObject(json, "Payload", payload);
    char *jsonString = cJSON_PrintUnformatted(json);
    LOG_Debug("jsonString = %s ",jsonString);
    if (*jsonString == NULL) {
        cJSON_Delete(json); // 释放JSON对象
        LOG_Error("cJSON Print Fail");
        return NULL; // 返回转换失败的错误码
    }
    return jsonString;
}


char* SyncOtaFieldVehicleInfoCreatJSONIntetface(char *str)
{

    char *json_str = NULL;
    char carid[30] = {0};
    uint8_t vinBuff[32] = {0};
    SyncOtaFieldVehicleInfo data = {0};
    int ret = 0;

    if(GetVehicleName(carid) == -1)
    {
        LOG_Error("GetCarNum fail");
        return NULL;
    }

    if (GetVIN(vinBuff) == -1)
    {
        LOG_Error("get vin Error");
        return NULL;
    }

   
    data.Vin = (char *)vinBuff;
    data.VehicleNo = (char *)carid;
    data.TimeStamp =  GetCurrentTimeMS();
    data.ServiceId = RESREMOTEOTASFIELDSERVICEID;
    data.SubServiceId = RESREMOTEOTAFIELDSUBSERVICEID;
    data.ProtocolVer = CANPROTOCOLVER;
    data.SeqId = " ";
    data.Field = str;
    return SyncOtaFieldVehicleInfoCreatJSON(&data);
}


int SyncOtaFieldVehicleInfoParseResult(const char *jsonString, unsigned int *result) 
{
    cJSON *json = cJSON_Parse(jsonString);
    if (json == NULL) {
        LOG_Error("Error parsing JSON\n");
        return -1;
    }

    // 获取 Payload 对象
    cJSON *payloadObj = cJSON_GetObjectItem(json, "Payload");
    if (payloadObj == NULL) {
        LOG_Error("Payload not found\n");
        cJSON_Delete(json);
        return -1;
    }

    // 获取 Result 字段
    cJSON *Result = cJSON_GetObjectItem(payloadObj, "Result");
    if (Result && cJSON_IsNumber(Result)) {
        *result = Result->valueint; // 提取 Result
    } else {
        LOG_Error("Result not found or is not a number\n");
        cJSON_Delete(json);
        return -1;
    }

    cJSON_Delete(json);  // 释放 JSON 对象
    return 0;
}

    
GVJ_Status CreateKeepAliveVehicleJson(KeepAliveVehicleData *data,char ** json_str)
{
   if (!data || !json_str) return GVJ_INVALID_PARAM;

    cJSON *root = cJSON_CreateObject();
    if (!root) return GVJ_MEMORY_ERROR;

    // 添加基础字段
    cJSON_AddStringToObject(root, "VehicleNo", data->vehicle_no);
    cJSON_AddStringToObject(root, "VIN", data->vin);
    cJSON_AddNumberToObject(root, "TimeStamp", data->timestamp);

    *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return (*json_str) ? GVJ_SUCCESS : GVJ_MEMORY_ERROR;
}


char *KeepAliveVehicleJsonInterface()
{
    KeepAliveVehicleData data = {0};
    char *json_str = NULL;
    char carid[30] = {0};
    uint8_t vinBuff[32] = {0};
    if(GetVehicleName(carid) == -1)
    {
        LOG_Error("GetCarNum fail");
        return NULL;
    }

    if (GetVIN(vinBuff) == -1)
    {
        LOG_Error("get vin Error");
        return NULL;
    }
    memcpy(data.vin,vinBuff,17);
    memcpy(data.vehicle_no,carid,15);
    data.timestamp =  GetCurrentTimeMS();
    GVJ_Status status = CreateKeepAliveVehicleJson(&data, &json_str);
    if(status != GVJ_SUCCESS)
    {
        LOG_Error("Create Keep Alive Vehicle Json Fail %d",status);
    }
    //printf("gps json_str : %s \n",json_str);
    return json_str;
}

int DomainParseVehicleData(const char *json_string, DomainVehicleData *data) 
{
    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL) {
        LOG_Error("Error parsing JSON");
        return -1; // 解析失败
    }
#if 0
    cJSON *vehicleNo = cJSON_GetObjectItemCaseSensitive(json, "VehicleNo");
    if ((vehicleNo != NULL) && (vehicleNo->valuestring != NULL)) {
        data->VehicleNo = strdup(vehicleNo->valuestring); // 动态分配内存
    } else {
        LOG_Error("VehicleNo Parse Fail");
        cJSON_Delete(json);
        return -1; // 解析失败
    }

    cJSON *vin = cJSON_GetObjectItemCaseSensitive(json, "Vin");
    if ((vin != NULL) && (vin->valuestring != NULL)) {
        data->Vin = strdup(vin->valuestring);
    } else {
        cJSON_Delete(json);
        LOG_Error("Vin Parse Fail");
        return -1;
    }

    cJSON *protocolVer = cJSON_GetObjectItemCaseSensitive(json, "ProtocolVer");
    if ((protocolVer != NULL) && (protocolVer->valuestring != NULL)) {
        data->ProtocolVer = strdup(protocolVer->valuestring);
    } else {
        cJSON_Delete(json);
        LOG_Error("ProtocolVer Parse Fail");
        return -1;
    }

    cJSON *serviceId = cJSON_GetObjectItemCaseSensitive(json, "ServiceId");
    if (cJSON_IsNumber(serviceId)) {
        data->ServiceId = serviceId->valueint;
    } else {
        cJSON_Delete(json);
        LOG_Error("ServiceId Parse Fail");
        return -1;
    }

    cJSON *subServiceId = cJSON_GetObjectItemCaseSensitive(json, "SubServiceId");
    if (cJSON_IsNumber(subServiceId)) {
        data->SubServiceId = subServiceId->valueint;
    } else {
        cJSON_Delete(json);
        LOG_Error("SubServiceId Parse Fail");
        return -1;
    }

    cJSON *timeStamp = cJSON_GetObjectItemCaseSensitive(json, "TimeStamp");
    if (cJSON_IsNumber(timeStamp)) {
        data->TimeStamp = timeStamp->valuedouble;
    } else {
        cJSON_Delete(json);
        LOG_Error("TimeStamp Parse Fail");
        return -1;
    }
#endif
    cJSON *seqId = cJSON_GetObjectItemCaseSensitive(json, "SeqId");
    if ((seqId != NULL) && (seqId->valuestring != NULL)) {
        data->SeqId = strdup(seqId->valuestring);
    } else {
        cJSON_Delete(json);
        LOG_Error("SeqId Parse Fail");
        return -1;
    }


    cJSON *payload = cJSON_GetObjectItemCaseSensitive(json, "Payload");
    if (cJSON_IsObject(payload)) {
        
        if ((payload != NULL) ) {
            data->PayloadTarget = cJSON_PrintUnformatted(payload);
        } else {
            cJSON_Delete(json);
            LOG_Error("target Parse Fail");
            return -1;
        }
    } else {
        LOG_Error("Payload Parse Fail");
        cJSON_Delete(json);
        return -1;
    }

    cJSON_Delete(json);
    return 0; // 解析成功
}


void FreeDomainVehicleData(DomainVehicleData *data) 
{
    if(data)
    {
        if(data->VehicleNo)
        {
            free(data->VehicleNo);
            data->VehicleNo = NULL;

        }
        
        if(data->Vin)
        {
            free(data->Vin);
            data->Vin = NULL;

        }
        
        if(data->ProtocolVer)
        {
            free(data->ProtocolVer);
            data->ProtocolVer = NULL;

        }
        
        if(data->SeqId)
        {
            free(data->SeqId);
            data->SeqId = NULL;

        }

        if(data->PayloadTarget)
        {
            free(data->PayloadTarget);
            data->PayloadTarget = NULL;

        }
    }else{
        LOG_Error("data NULL");
    }
    
}


static char* DoaminVehicleDataToJson(const DomainReplyVehicleData *data) 
{
    if(!data)
    {
        LOG_Error("data NULL");
        return NULL;
    }

    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        return NULL; // 创建 JSON 对象失败
    }

    cJSON_AddStringToObject(json, "VehicleNo", data->VehicleNo);
    cJSON_AddStringToObject(json, "Vin", data->Vin);
    cJSON_AddStringToObject(json, "ProtocolVer", data->ProtocolVer);
    cJSON_AddNumberToObject(json, "ServiceId", data->ServiceId);
    cJSON_AddNumberToObject(json, "SubServiceId", data->SubServiceId);
    cJSON_AddNumberToObject(json, "TimeStamp", data->TimeStamp);
    cJSON_AddStringToObject(json, "SeqId", data->SeqId);
    
    cJSON *payload = cJSON_CreateObject();
    if (payload == NULL) {
        cJSON_Delete(json);
        return NULL; // 创建 Payload 对象失败
    }
    
    cJSON_AddNumberToObject(payload, "Result", data->Result);
    cJSON_AddItemToObject(json, "Payload", payload);
    
    char *json_string = cJSON_PrintUnformatted(json);
    cJSON_Delete(json); // 释放 JSON 对象
    return json_string; // 返回构建的 JSON 字符串
}

char* DomainCreateJsonInterface(char *seqid,int result) 
{
   DomainReplyVehicleData data = {0};
   char *json_str = NULL;
    char carid[30] = {0};
    uint8_t vinBuff[32] = {0};
    int ret = 0;
   
    if(GetVehicleName(carid) == -1)
    {
        LOG_Error("GetCarNum fail");
        return NULL;
    }

    if (GetVIN(vinBuff) == -1)
    {
        LOG_Error("get vin Error");
        return NULL;
    }
    

    data.Vin = (char *)vinBuff;
    data.VehicleNo = (char *)carid;
    data.TimeStamp =  GetCurrentTimeMS();
    data.ServiceId = RESREMOTEDOMAINSERVICEID;
    data.SubServiceId = RESREMOTEOTASUBSERVICEID;
    data.ProtocolVer = CANPROTOCOLVER;
    data.SeqId = seqid;
    data.Result = result;

    return  DoaminVehicleDataToJson(&data) ;
}



int GetDomainHttpReValue(const char *json_string) 
{
    cJSON *json = cJSON_Parse(json_string);  
    if (json == NULL) {
        LOG_Debug("Error parsing JSON");
        return -1;  
    }

    cJSON *err = cJSON_GetObjectItem(json, "r");
    if (err && cJSON_IsNumber(err)) {
        int errValue = err->valueint;
        cJSON_Delete(json);
        return errValue;
    }
    LOG_Error("Get r Error ");
    cJSON_Delete(json);  // 释放 JSON 对象
    return -1;  // 返回 -1 表示未找到 err 或其不是数字
}



int CreateFotaHttpJson(int r, const char *platform,char* str,int len) 
{
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        LOG_Error("创建 JSON 对象失败");
        return -1;
    }

    cJSON_AddNumberToObject(json, "r", r);
    cJSON_AddStringToObject(json, "target", platform);

    char *json_string = cJSON_PrintUnformatted(json);
    if (json_string == NULL) {
        LOG_Error("转换 JSON 对象为字符串失败");
        cJSON_Delete(json);
        return -1;
    }

    if(len <= strlen(json_string))
    {
        LOG_Error("len < json_string len");
    }else{
        strncpy(str,json_string,strlen(json_string));
        LOG_Debug("%s   --->  %s\n",json_string,str);
    }
    
    if(json_string)
    {
        free(json_string);
        json_string = NULL;
    }
    
    cJSON_Delete(json);
    return 0;
}
