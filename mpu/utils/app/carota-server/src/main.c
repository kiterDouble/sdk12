#include <curl/curl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glib.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <inttypes.h>  // 添加这个头文件以支持精确宽度整数格式宏
#include "carotaHttpClient.h"
#include "cJSON.h"
#include "civetweb.h"
#include "sr_log.h"
#include "err_code.h"
#include "sr_ipc.h"
#include "modem_ipc_msg.h"
#include "sdk/sr_sdk.h"
#include "sdk/pm_server_api.h"
#include "sdk/sr_param.h"
#include "sdk/mcu_api.h"
#include "sdk/ota_api.h"
#include "sgmw_ipc.h"
#include "ucu_status.h"
#include "ucu_upgrade.h"
#include "vehicle_status.h"
#include "pm_mcu_control.h"
#include "carotaFilehandle.h"

#include "mcuinte.h"

#include "carotsleephandle.h"
#include "carota_type.h"
#include "param_manager.h"
#include "param_types.h"
#include "param_adapter.h"
static volatile gboolean g_is_waiting_for_b = FALSE;

static struct mg_connection *g_pending_a_conn = NULL;
// 互斥锁：保护全局变量访问
static pthread_mutex_t g_state_mutex = PTHREAD_MUTEX_INITIALIZER;
// 条件变量：用于A等待B的通知
static pthread_cond_t g_b_notify_cond = PTHREAD_COND_INITIALIZER;
// 存储B的响应数据（如果需要A使用B的结果）
static char g_b_response[4096] = {0};

static int reboot_num = 0;

static uint8_t OtaPowermModeIntoStatus = 0;

static int temp_kl15 = 0;
static int change_power = 0;
static int OatFotaStatus = 0;
static int OatDotaStatus = 0;

#define STR_LEN 64
typedef struct
{
    char ota_name[STR_LEN];     // 模块名称，例如 zoe / zoe-dnn-model
    char name_sv[STR_LEN];      // 软件版本 VersionName
    char name_hv[STR_LEN];      // 硬件版本
    char name_blv[STR_LEN];     // Bootloader版本
    char name_sn[STR_LEN];      // 序列号
    char name_partnum[STR_LEN]; // 料号
    char name_supplier[STR_LEN];// 供应商
    uint32_t version_id;        // 版本ID
} OtaInfo;
typedef enum
{
    OTA_ZOE = 0,
    OTA_ZOE_CONF,
    OTA_ZOE_DNN_MODEL,
    OTA_ZOE_MAP,
    OTA_COMPUTING_UNIT_MCU,
    OTA_LOCK_CONTROL_BOARD,
    OTA_COMPUTING_UNIT_KERNEL,
    OTA_CAMERA,
    OTA_LIDAR,
    OTA_RADAR,
    OTA_GNSS,
    OTA_SDC,
    OTA_UVM,
    OTA_MODULE_MAX
} OtaModuleType;

static OtaInfo ota_modules[OTA_MODULE_MAX];
typedef struct
{
	uint8_t kl15;
	int16_t kl30;
	//uint16_t fastCharging;
    uint8_t canNet;
} MCUSYNCKINE_tt;

static MCUSYNCKINE_tt g_mcuSyncKine_car = {
    .kl15 = 0,      
    .kl30 = -1,     
    .canNet = 0    
};

#define PORT "20003"

//GET
#define GETINFO_URI "/getinfo"  //与九识相同
//GET
#define GETVIN_URI "/getvin"
//GET
#define PREPARE_URI "/prepare"
//POST
#define ECUUPGRADE_URI "/ecuupgrade"
//GET
#define ECUUPGRADERESULT_URI "/ecuupgraderesult"
//GET
#define RESETWD_URI "/resetwd"
//GET
#define MODEIN_URI "/modein"
//GET
#define MODEINRESULT_URI "/modeinresult"
//GET
#define MODEOUT_URI "/modeout"
//GET
#define MODEOUTRESULT_URI "/modeoutresult"
//GET
#define READY_URI "/ready"
//GET
#define WAKE_URI "/wake"
//GET
#define TSP_WAKE_CAR "/tspwake"
//GET
#define WAKEOUT_URI "/wakeout"
//GET 
#define RTCTIME_URI "/rtctime"
//GET
#define HVOIN_URI "/hvoin"
//GET
#define HVOOUT_URI "/hvoout"
//GET
#define REMOTECOMMAND_URI "/remotecommand"
//POST
#define TSPMESSAGE_URI "/tspmessage"
//GET
#define CANTPLOCK_URI "/cantplock"
//GET
#define CANTPUNLOCK_URI	"/cantpunlock"
//GET
#define FOTAGETCHALLENGE "/fotagetchallenge"
//GET
#define FOTASENDKEY "/fotasendkey"
//GET
#define HVUP_URI "/hvup"
//GET
#define HVUPRESULT_URI "/hvupresult"
//GET
#define HVDOWN_URI "/hvdown"
//GET
#define HVDOWNRESULT_URI "/hvdownresult"
//POST --九识新增
#define FILE_DOWNLOAD_URI "/download/*"

#define SETSCHEDULETIME "/alarm"
//GET --九识新增
#define CANCELSCHEDULETIME "/alarmcancel"
//GET --九识新增  缺少参考示例
#define INDOTAMODE "/dotamodein"
//GET --九识新增 
#define QUERYRESULTOFDOTAMODE "/dotamodeinresult"
//GET --九识新增 缺少参考示例
#define OUTDOTAMODE "/dotamodeout"
//GET --九识新增
#define QUERYRESULTOFINHIGHTVOLTAGEMODE  "/hvoinresult"
//GET --九识新增
#define QUERYRESULTOFOUTHIGHTVOLTAGERMODE "/hvooutresult"
//GET --九识新增
#define GETCARSTATUS "/vehiclestatus"
//POST --九识新增
#define SETFIEL "/setotafield"
//POST --九识新增
#define SYNCTSP "/syncotafield"
//GET--九识新增
#define RDSWATCHDOG "/rdsresetwd"
//GET --九识新增
#define DOTAMODEOUTRESULT "/dotamodeoutresult"
//POST --九识新增
#define PREPAREINSTALL "/prepareinstall"
//POST --九识新增
#define SERVERSWITCH   "/serverswitch"
//POST --九识新增
#define ADCU_SETMSG_QUE "/api/ota/adcu/info/*/set"
//GET --九识新增
#define ADCU_MSG_QUE  "/api/ota/adcu/info/"
//GET --九识新增
#define querypowerStatus "/querypowerStatus"
//GET --九识新增
#define resetpowerset "/resetPowerStatus"
//GET --斯润新增
#define getownstatus "/getowntatus"

#define TBOXSETDOMAINURL   "/TBOXSetDomain"
#define NOTUFYTBOXSLEEPURL "/notifyTBOXCouldSleep"
#define NOTIFYFOTASLEEPURL "/notifyFOTASleep"

#define TESTDEBUG_URI "/testdebug"
#define SRUPGRADE_URL "/srUpgradeStatus"
#define SYS_QUE_SERVER_SWITCH 0x191046046



static unsigned int carota_adcu_tv_no = 0;
typedef struct
{
    uint8_t Act_Gear;
    uint8_t Veh_Speed_1;
	uint8_t Veh_Speed_2;
    uint8_t EPB_Status;
    uint8_t Charg_status;
    uint8_t pwr_1;
	uint8_t pwr_2;
    uint8_t VCU_UpdateFlag;
	uint8_t powermode;
	uint8_t door_state;
	uint8_t arming_state;
	uint8_t AD_VehMisMod;
	uint8_t AD_upgradeCheckStatus;
}NEWMCUMPUDATA_t;

typedef struct
{
	int8_t num; 
} urcBoardcast_tt;

static pthread_mutex_t g_wakeoutTimeoutMutex = PTHREAD_MUTEX_INITIALIZER;
struct timespec g_wakeoutTimeout = {
    .tv_sec = 0,
    .tv_nsec = 0 
};
static struct timespec g_wdTimeout;
static struct timespec g_wdrdsTimeout;
static MPUPmStateChange_t g_pmStatus = {0};
static ModemSrvNotify_t g_modemSrvNotify = {0};
static NEWMCUMPUDATA_t g_prepare_data = {
    .AD_upgradeCheckStatus = 1
};	//MCU发给MPU的科洛达数据
static struct timespec g_modeChangeTimeout;
static int dotamoderesult = -1;
static char sha_num[256] = {0};
static char ota_file_path[256] = {0};
static char package_version[256] = {0};
static guint8 settime_ota=0;
pthread_mutex_t g_sendHeartBeatLock;
pthread_mutex_t g_sendmcuBeatLock;
static int g_car_init_power = 1;//默认认为是上电状态
static int mcu_power_status = -1;


// 定义存储curl响应的结构体
struct curl_response {
    char *data;  // 存储响应数据
    size_t len;  // 响应数据长度
};

// 允许的设备类型列表（与用户需求一致）
static const char* ALLOWED_DEVS[] = {
    "zoe", "zoe-conf", "zoe-dnn-model", "zoe-map", 
    "computing-unit-mcu", "locking-control-board", 
    "computing-unit-kernel", "camera", "lidar", 
    "radar", "gnss", "sdc", "uvm", "tbox", "adcu"
};
static gint32 SendJSON(struct mg_connection *conn, cJSON *json_obj);
static gint32 notifyTboxSleepHandler(struct mg_connection *conn, gpointer cbdata)
{
	LOG_Debug("--%s begin!", __FUNCTION__);
	gint32 postDatalen = 0;
	gchar postData[1024];
	cJSON* root = NULL;
    cJSON* platform = NULL;
	cJSON* r = NULL;
	postDatalen = mg_read(conn, postData, sizeof(postData));
	if (postDatalen <= 0)
	{
		LOG_Error("--%s post data len invalid.", __FUNCTION__);
		goto error_exit;
	}
	postData[postDatalen] = '\0';  
	LOG_Debug("%s recv data : %s\n",__FUNCTION__, postData);
	root = cJSON_Parse(postData);
    if(NULL == root)
    {
        LOG_Error("--%s root fail.", __FUNCTION__);
        goto error_exit;
    }
    platform = cJSON_GetObjectItem(root, "platform"); //fota , dota 
    if (platform == NULL)
    {
        LOG_Error("--%s 'platform' is missing or not a number.", __FUNCTION__);
        goto error_exit;
    }
	r = cJSON_GetObjectItem(root, "r"); //1:允许休眠,0:不允许休眠
    if (r == NULL)
    {
        LOG_Error("--%s 'r' is missing or not a number.", __FUNCTION__);
        goto error_exit;
    }
	if (strstr(platform, "fota")) {
		if(r->valueint == 1)
		{
			PmSetSleepEvent(FOTA_NOTIFY_PM_SLEEP_EVENT);
		}else if(r->valueint == 0)
		{
			PmCleanSleepEvent(FOTA_NOTIFY_PM_SLEEP_EVENT);
		}
		
	} else if (strstr(platform, "dota")) { 
		if(r->valueint == 1)
		{
			PmSetSleepEvent(DOTA_NOTIFY_PM_SLEEP_EVENT);
		}else if(r->valueint == 0)
		{
			PmCleanSleepEvent(DOTA_NOTIFY_PM_SLEEP_EVENT);
		}
	}
	cJSON *obj = cJSON_CreateObject();
	if (!obj) 
	{
		LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
		goto error_exit;
	}
	
	cJSON_AddStringToObject(obj, "platform",platform->valuestring); 

	SendJSON(conn, obj);
	cJSON_Delete(obj);
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("[%s]CARSTATUSHandler ok",__FUNCTION__);
	return 200;
error_exit:
	if (root)
	{
		cJSON_Delete(root);
	}
	mg_send_http_error(conn, 500, "Parameter error / timeout / parse error");
	return 500;
}
static gint32 SetDomainHandler(struct mg_connection *conn, gpointer cbdata)
{
	LOG_Debug("--%s begin!", __FUNCTION__);
	gint32 postDatalen = 0;
	gchar postData[1024];
	cJSON* root = NULL;
	cJSON* target = NULL;
    cJSON* platform = NULL;
	postDatalen = mg_read(conn, postData, sizeof(postData));
	if (postDatalen <= 0)
	{
		LOG_Error("--%s post data len invalid.", __FUNCTION__);
		goto error_exit;
	}
	postData[postDatalen] = '\0';  
	LOG_Debug("%s recv data : %s\n",__FUNCTION__, postData);
	root = cJSON_Parse(postData);
    if(NULL == root)
    {
        LOG_Error("--%s root fail.", __FUNCTION__);
        goto error_exit;
    }
    target = cJSON_GetObjectItem(root, "target"); //prod , uat
    if (target == NULL)
    {
        LOG_Error("--%s 'target' is missing or not a number.", __FUNCTION__);
        goto error_exit;
    }
	platform = cJSON_GetObjectItem(root, "platform");
    if (platform == NULL)
	{
        LOG_Error("--%s 'platform' is missing or not a number.", __FUNCTION__);
        goto error_exit;
    }
	

	char *target_str = target->valuestring;
	char *platform_str = platform->valuestring;
	LOG_Debug("--%s target:%s,platform:%s", __FUNCTION__, target_str, platform_str);
	int count = 0;
	int8_t cmd = 0;
	int ret = 0;
	if (strstr(platform_str, "fota")) {
		if (strstr(target_str, "uat")) 
		{
			OatFotaStatus = -1;
			cmd = 1;
			SendMsgToAllQueue(0x002f0002, (void *)&cmd, sizeof(cmd));
			while (1)
			{
				if(OatFotaStatus == 1|| OatFotaStatus == 0 || count > 30) break;
				sleep(1);
				count++;
			}
			LOG_Debug("--%s OatFotaStatus:%d,count:%d ", __FUNCTION__, OatFotaStatus,count);
			ret = OatFotaStatus;
		} else if (strstr(target_str, "prod")) 
		{
			OatFotaStatus = -1;
			cmd = 2;
			SendMsgToAllQueue(0x002f0002, (void *)&cmd, sizeof(cmd));
			while (1)
			{
				if(OatFotaStatus == 1|| OatFotaStatus == 0 || count > 30) break;
				sleep(1);
				count++;
			}
			LOG_Debug("--%s OatFotaStatus:%d,count:%d ", __FUNCTION__, OatFotaStatus,count);
		}
		ret = OatFotaStatus;
	} else if ((platform_str, "dota")) {
		if (strstr(target_str, "uat")) 
		{
			OatDotaStatus = -1;
			cmd = 1;
			SendMsgToAllQueue(0x002f0004, (void *)&cmd, sizeof(cmd));
			while (1)
			{
				if(OatDotaStatus == 1|| OatDotaStatus == 0 || count > 30) break;
				sleep(1);
				count++;
			}
			LOG_Debug("--%s OatDotaStatus:%d,count:%d ", __FUNCTION__, OatDotaStatus,count);
		} else if (strstr(target_str, "prod")) 
		{
			OatDotaStatus = -1;
			cmd = 2;
			SendMsgToAllQueue(0x002f0004, (void *)&cmd, sizeof(cmd));
			while (1)
			{
				if(OatDotaStatus == 1|| OatDotaStatus == 0 || count > 30) break;
				sleep(1);
				count++;
			}
			LOG_Debug("--%s OatDotaStatus:%d,count:%d ", __FUNCTION__, OatDotaStatus,count);
		}
		ret = OatDotaStatus;
	}


	cJSON *obj = cJSON_CreateObject();
	if (!obj) 
	{
		LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
		goto error_exit;
	}

	cJSON_AddNumberToObject(obj, "r", ret ? 1 : 0); 
	cJSON_AddStringToObject(obj, "target", target_str); 
	SendJSON(conn, obj);
	cJSON_Delete(obj);
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("[%s]CARSTATUSHandler ok",__FUNCTION__);
	return 200;

error_exit:
	if (root)
	{
		cJSON_Delete(root);
	}
	mg_send_http_error(conn, 500, "Parameter error / timeout / parse error");
	return 500;
}
int carotaSetotafield2(CurlResponse *curl_resp,struct mg_connection *conn,char * original_body)
{
	CURL *curl;
	struct curl_slist *headers = NULL;
	CURLcode res;
    
    curl = curl_easy_init();
    if (curl == NULL) {
        LOG_Error("curl init error");
        // 初始化失败时，向客户端返回500错误
        mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
        return -1;
    }
    // 设置curl请求参数
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:20005/setotafield?dev=tbox");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, original_body);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15);

    // 关键：设置回调函数捕获20002的响应
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback1);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)curl_resp);

    // 执行curl请求
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        LOG_Error("perform curl error = %s", curl_easy_strerror(res));
    } else {
        LOG_Debug("perform curl success, response: %s", curl_resp->data);
    }
	curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return 0;
}

uint8_t GetVehMisModStatus(void)
{
    return g_prepare_data.AD_upgradeCheckStatus;
}

void TspMessageHandler(struct mg_connection *conn, gpointer cbdata) {
    struct mg_request_info* req_info = mg_get_request_info(conn);
    char original_body[4096] = {0};
    size_t body_len = 0;
    
    // 用于存储20005端口返回的响应
    CurlResponse curl_resp = {.data = NULL, .len = 0};

    // 读取POST请求体
    if (req_info->content_length > 0) {
        body_len = mg_read(conn, original_body, 
                          sizeof(original_body) - 1 < req_info->content_length 
                          ? sizeof(original_body) - 1 
                          : req_info->content_length);
        original_body[body_len] = '\0';
    }
    LOG_Debug("[%s] original_body:%s,%d\r\n",__FUNCTION__, original_body, body_len);
    
    // 无请求体时返回400错误
    if (body_len == 0) {
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
        return;
    }
    
    // 整车唤醒逻辑（服务端处理部分）
    if(g_mcuSyncKine_car.kl15==0x00)
    {
        g_car_init_power = 0;
        int p_up = RET_ERR;
        LOG_Debug("[%s]start wake mcu %d\r\n",__FUNCTION__,mcu_power_status);
        while ( mcu_power_status!=9)
        {
            PmSyncDataWakeSourceAndWakeupSystem(PM_WAKEUP_SOURCE_DATA);
            PmWakeupSetDevices("mcu", 1);
            sleep(1);
            LOG_Debug("[%s]start wake mcu %d\r\n",__FUNCTION__,mcu_power_status);
        }
        
        while (RET_OK != p_up )
        {
            p_up=ControlRemotePowerUp();
            sleep(1);
        }
        PmSetSleepEvent(PM_SLEEP_EVENT_WAKEUPCAR);
        PmSetSleepEvent(PM_SLEEP_EVENT_WAKEUPTBOX);
    }
    sleep(1);

    int ret = ForwardRequestToPort20005(&curl_resp, original_body);
    if (ret == RET_OK && curl_resp.len > 0) {
        cJSON *obj = cJSON_CreateObject();
        if (!obj) 
        {
            LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
        } else {
            cJSON_AddStringToObject(obj, "subfun", "carota"); 
            cJSON_AddStringToObject(obj, "message", curl_resp.data); 
            SendJSON(conn, obj);
            cJSON_Delete(obj);
            LOG_Debug("[%s]GET Response:%s,len:%d\r\n", __FUNCTION__,curl_resp.data,curl_resp.len);
            mg_printf(conn,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Connection: close\r\n\r\n");
        }
    } else {
        // 若未收到20005的响应，返回默认成功信息
        const char *default_resp = "{\"status\":\"processed\",\"msg\":\"消息已转发，但未收到目标响应\"}";
        mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n%s",
                strlen(default_resp), default_resp);
    }

    // 清理资源
    if (curl_resp.data != NULL) {
        free(curl_resp.data);
    }
}

static gint32 SendJSON(struct mg_connection *conn, cJSON *json_obj)
{
        gchar *json_str = cJSON_PrintUnformatted(json_obj);
        size_t json_str_len = strlen(json_str);
		
		LOG_Debug("--%s called, json_str:%s", __FUNCTION__, json_str);
        /* Send HTTP message header */
        mg_send_http_ok(conn, "application/json; charset=utf-8", json_str_len);

        /* Send HTTP message content */
        mg_write(conn, json_str, json_str_len);

        /* Free string allocated by cJSON_Print* */
        cJSON_free(json_str);

        return (int)json_str_len;
}

static gint32 GetInfoHandler(struct mg_connection *conn, gpointer cbdata) {
    gchar dev[64] = {0};          // 存储设备类型（如adcu、tbox）
    gchar dev_prefix[64] = {0};   // 存储键名前缀（如adcu_、tbox_）
	const struct mg_request_info *request_info = mg_get_request_info(conn);
	const gchar *qs = request_info->query_string;
    gchar dev_sv[128] = {0};
    gchar dev_hv[128] = {0};
    gchar dev_blv[128] = {0};
    gchar dev_sn[128] = {0};
    gchar dev_partnum[128] = {0};
    gchar dev_supplier[128] = {0};
	uint32_t version_id = 0;
    gint16 sql_rq_num = 0;
    LOG_Debug("--%s called!", __FUNCTION__);
    PmSetSleepEvent("carota");
    usleep(100000);
    // -------------------------- 步骤1：获取并校验dev参数 --------------------------
	mg_get_var(qs, strlen(qs), "dev", dev, sizeof(dev));
    for (int i = 0; i < sizeof(ALLOWED_DEVS)/sizeof(ALLOWED_DEVS[0]); i++) {
        if (g_strcmp0(dev, ALLOWED_DEVS[i]) == 0) {
            sql_rq_num = 105 + i*8;
			if (g_strcmp0(dev, "tbox") == 0) {
            	sql_rq_num = 104 + i*8;
        	} 
            break;
        }  
    }
    if (sql_rq_num == 0)
    {
        goto error_exit;
    }

    // 生成键名前缀（如dev=adcu → prefix=adcu_）
    g_snprintf(dev_prefix, sizeof(dev_prefix), "%s_", dev);
    LOG_Debug("[%s]current dev: %s, key prefix: %s",__FUNCTION__,dev, dev_prefix);
	if(sql_rq_num!=208)
	{
		gint32 module_index = -1;
		const char *valid_names[] = {
			"zoe", "zoe-conf", "zoe-dnn-model", "zoe-map",
			"computing-unit-mcu", "locking-control-board", "computing-unit-kernel",
			"camera", "lidar", "radar", "gnss", "sdc", "uvm",
			NULL
		};

		// 使用 valid_names 数组匹配 ota_name
		for (int i = 0; valid_names[i] != NULL; i++) {
			if (g_strcmp0(dev, valid_names[i]) == 0) {
				module_index = i;
				break;
			}
		}
		
		// 如果没有找到匹配的 ota_name，直接返回失败
		if (module_index == -1) {
			LOG_Error("[%s] 无效的 ota_name: %s", __FUNCTION__, dev);
			goto error_exit;
		}
		
		// 验证模块索引范围
		if (module_index < 0 || module_index >= OTA_MODULE_MAX) {
			LOG_Error("[%s] 模块索引超出范围：%d", __FUNCTION__, module_index);
			goto error_exit;
		}

		// 使用 TBParameterAdapter 接口从参数管理系统批量获取数据
		LOG_Debug("[%s] 从参数管理系统获取模块[%d]数据", __FUNCTION__, module_index);
		
		int base_pid = module_index * 8;
		
		// 字符串参数列表（6个字符串参数）
		TBParamList str_param_list[6];
		
		// 设置字符串参数ID（name_sv, name_hv, name_blv, name_sn, name_partnum, name_supplier）
		for (int i = 0; i < 6; i++) {
			str_param_list[i].pid = base_pid + 1 + i;
			str_param_list[i].index = 0;
			memset(str_param_list[i].valueStr, 0, MPARAM_MAX_PSRRING);
		}
		
		// 批量获取字符串参数
		int ret = TBParameterAdapterStringGetByIDList(str_param_list, 6);
		if (ret != 0) {
			LOG_Error("[%s] TBParameterAdapterStringGetByIDList fail, ret:%d", __FUNCTION__, ret);
			goto error_exit;
		}
		
		// 将获取到的字符串参数值复制到本地变量
		memcpy(dev_sv, str_param_list[0].valueStr, sizeof(dev_sv) - 1);
		dev_sv[sizeof(dev_sv) - 1] = '\0';
		LOG_Debug("[%s] name_sv: %s (pid:0x%04X)", __FUNCTION__, dev_sv, str_param_list[0].pid);

		memcpy(dev_hv, str_param_list[1].valueStr, sizeof(dev_hv) - 1);
		dev_hv[sizeof(dev_hv) - 1] = '\0';
		LOG_Debug("[%s] name_hv: %s (pid:0x%04X)", __FUNCTION__, dev_hv, str_param_list[1].pid);

		memcpy(dev_blv, str_param_list[2].valueStr, sizeof(dev_blv) - 1);
		dev_blv[sizeof(dev_blv) - 1] = '\0';
		LOG_Debug("[%s] name_blv: %s (pid:0x%04X)", __FUNCTION__, dev_blv, str_param_list[2].pid);

		memcpy(dev_sn, str_param_list[3].valueStr, sizeof(dev_sn) - 1);
		dev_sn[sizeof(dev_sn) - 1] = '\0';
		LOG_Debug("[%s] name_sn: %s (pid:0x%04X)", __FUNCTION__, dev_sn, str_param_list[3].pid);

		memcpy(dev_partnum, str_param_list[4].valueStr, sizeof(dev_partnum) - 1);
		dev_partnum[sizeof(dev_partnum) - 1] = '\0';
		LOG_Debug("[%s] name_partnum: %s (pid:0x%04X)", __FUNCTION__, dev_partnum, str_param_list[4].pid);
		
		memcpy(dev_supplier, str_param_list[5].valueStr, sizeof(dev_supplier) - 1);
		dev_supplier[sizeof(dev_supplier) - 1] = '\0';
		LOG_Debug("[%s] name_supplier: %s (pid:0x%04X)", __FUNCTION__, dev_supplier, str_param_list[5].pid);
		
		// 获取整数参数 version_id
		TBParamList int_param;
		int_param.pid = base_pid + 7;  // version_id
		int_param.index = 0;
		
		ret = TBParameterAdapterGetByIDList(&int_param, 1);
		if (ret != 0) {
			LOG_Error("[%s] TBParameterAdapterGetByIDList for version_id fail, ret:%d", __FUNCTION__, ret);
			goto error_exit;
		}
		
		version_id = int_param.valueInt;
		LOG_Debug("[%s] version_id: %d (pid:0x%04X)", __FUNCTION__, version_id, int_param.pid);
	}
	else
	{
		ParamInfo_t param;
		param.id = 208;//param.id = 14; E_ParamId_SN在server中：service\param\src\param_database.c
		if (RET_OK != GetParam(&param))
		{
			LOG_Error("[%s] get tbox sn fail.", __FUNCTION__);
			// goto error_exit;
		}
		CarotaFileVersionGetVersion(dev_sv,sizeof(dev_sv));
		// snprintf(dev_sv, sizeof(dev_sv), "%s", "03060076260115");

		param.id = 15;//E_ParamId_SN在server中：service\param\src\param_database.c
		if (RET_OK != GetParam(&param))
		{
			LOG_Error("[%s] get tbox sn fail.", __FUNCTION__);
			// goto error_exit;
		}
		LOG_Debug("[%s] param2.id:%d",__FUNCTION__,param.id);
		snprintf(dev_hv, sizeof(dev_hv), "%s", param.value);

		param.id = 16;//E_ParamId_SN在server中：service\param\src\param_database.c
		if (RET_OK != GetParam(&param))
		{
			LOG_Error("[%s] get tbox sn fail.", __FUNCTION__);
			// goto error_exit;
		}
		LOG_Debug("[%s] param3.id:%d",__FUNCTION__,param.id);
		snprintf(dev_blv, sizeof(dev_blv), "%s", param.value);

		param.id = 3;//E_ParamId_SN在server中：service\param\src\param_database.c
		if (RET_OK != GetParam(&param))
		{
			LOG_Error("[%s] get tbox sn fail.", __FUNCTION__);
			// goto error_exit;
		}
		LOG_Debug("[%s] param4.id:%d",__FUNCTION__,param.id);
		snprintf(dev_sn, sizeof(dev_sn), "%s", param.value);

		param.id = 19;//E_ParamId_SN在server中：service\param\src\param_database.c
		if (RET_OK != GetParam(&param))
		{
			LOG_Error("[%s] get tbox sn fail.", __FUNCTION__);
			// goto error_exit;
		}
		LOG_Debug("[%s] param5.id:%d",__FUNCTION__,param.id);
		snprintf(dev_partnum, sizeof(dev_partnum), "%s", param.value);
		param.id = 20;//E_ParamId_SN在server中：service\param\src\param_database.c
		if (RET_OK != GetParam(&param))
		{
			LOG_Error("[%s] get tbox sn fail.", __FUNCTION__);
			// goto error_exit;
		}
		LOG_Debug("[%s] param6.id:%d",__FUNCTION__,param.id);
		snprintf(dev_supplier, sizeof(dev_supplier), "%s", param.value);
	}


    // -------------------------- 步骤3：动态生成JSON（按dev前缀） --------------------------
    cJSON *obj = cJSON_CreateObject();
    if (!obj) {
        LOG_Error("--%s call cJSON_CreateObject fail", __FUNCTION__);
        goto error_exit;
    }

    // 动态添加键值对：前缀+后缀（如adcu_sv、adcu_hv）
    cJSON_AddStringToObject(obj, g_strconcat(dev_prefix, "sv", NULL), dev_sv);        // 版本
    cJSON_AddStringToObject(obj, g_strconcat(dev_prefix, "hv", NULL), dev_hv);   // 硬件版本（可动态调整）
    cJSON_AddStringToObject(obj, g_strconcat(dev_prefix, "blv", NULL), dev_blv);      // Bootloader版本（可动态调整）
    cJSON_AddStringToObject(obj, g_strconcat(dev_prefix, "sn", NULL), dev_sn);        // 序列号
    cJSON_AddStringToObject(obj, g_strconcat(dev_prefix, "partnum", NULL), dev_partnum);  // 零件号
    cJSON_AddStringToObject(obj, g_strconcat(dev_prefix, "supplier", NULL), dev_supplier);  // 供应商
	cJSON_AddNumberToObject(obj, "version_id", version_id);  // 供应商
	// cJSON_AddStringToObject(obj, g_strconcat(dev_prefix, "name_model_name", NULL), dev);  // 设备名字
	// cJSON_AddStringToObject(obj, g_strconcat(dev_prefix, "name_model_id", NULL), "");  // 设备名字
    // -------------------------- 步骤4：发送响应并清理资源 --------------------------

    SendJSON(conn, obj);
    cJSON_Delete(obj);
    PmCleanSleepEvent("carota");
	LOG_Debug("[%s]208 PmCleanSleepEvent",__FUNCTION__);
    // 发送HTTP 200响应头
    mg_printf(conn,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"  // 修正为JSON类型（原text/plain）
        "Connection: close\r\n\r\n");
	LOG_Debug("[%s] Get Info Handler",__FUNCTION__);
    return 200;

    // -------------------------- 错误处理 --------------------------
error_exit:
    mg_send_http_error(conn, 500, "Parameter error / timeout / parse error");
    PmCleanSleepEvent("carota");
	LOG_Debug("[%s] Get Info Handler Parameter error timeout parse error",__FUNCTION__);
    return 500;
}

static gint32 SRUPGRADE_URLhandler(struct mg_connection *conn, gpointer cbdata)
{
    (void)cbdata;

    LOG_Debug("[%s]", __FUNCTION__);

    cJSON *obj = cJSON_CreateObject();
    if (!obj)
    {
        LOG_Error("[%s] cJSON_CreateObject failed", __FUNCTION__);
        goto error_exit;
    }

    int tbox_check = ReadMqttUpgradeStatus();
	if(tbox_check == -1)
	{
		tbox_check = 3;
	}
    cJSON_AddNumberToObject(obj, "UpgradeStatus", tbox_check);

    SendJSON(conn, obj);

    cJSON_Delete(obj);

    LOG_Debug("[%s] success", __FUNCTION__);

    return 200;

error_exit:

    mg_send_http_error(conn, 500, "Internal error");

    LOG_Error("[%s] handler error", __FUNCTION__);

    return 500;
}

static gint32 TESTDEBUG_URIHandler(struct mg_connection *conn, gpointer cbdata)
{

   	LOG_Debug("[%s] kiter TESTDEBUG_URIHandler ,AD_VehMisMod : %d",__FUNCTION__,g_prepare_data.AD_VehMisMod);
	

	TBParamList listtable[] =
    {
        /*0*/ {ADCUNAMEZOE_NAME_HANDLE,       0, {0}},
        /*1*/ {ADCUNAMEZOE_SW_HANDLE,         0, {0}},
        /*2*/ {ADCUNAMEZOE_HV_HANDLE,         0, {0}},
        /*3*/ {ADCUNAMEZOE_BLE_HANDLE,        0, {0}},
        /*4*/ {ADCUNAMEZOE_SN_HANDLE,         0, {0}},
        /*5*/ {ADCUNAMEZOE_PARTNUM_HANDLE,    0, {0}},
        /*6*/ {ADCUNAMEZOE_SUPPLIER_HANDLE,   0, {0}},
		/*7*/ {ADCUNAMEZOE_VERSIONID_HANDLE,  0, {0}},
    };
    int ret;
        
    /* 批量读取 - 使用 TBParameterGetByIDList */
    ret = TBParameterGetByIDList(listtable, sizeof(listtable) / sizeof(TBParamList));
	LOG_Debug("%s 批量读取 %d 个参数 (%s):\n",__FUNCTION__, 8, ret == 0 ? "成功" : "失败");
    LOG_Debug("%s  [0] device_id     = %s\n",__FUNCTION__, listtable[0].valueStr);
    LOG_Debug("%s  [1] device_name   = %s\n",__FUNCTION__, listtable[1].valueStr);
    LOG_Debug("%s  [2] HV            = %s\n",__FUNCTION__, listtable[2].valueStr);
    LOG_Debug("%s  [3] BLE           = %s\n",__FUNCTION__, listtable[3].valueStr);
    LOG_Debug("%s  [4] SN            = %s\n",__FUNCTION__, listtable[4].valueStr);
    LOG_Debug("%s  [5] PARTNUM       = %s\n",__FUNCTION__, listtable[5].valueStr);
    LOG_Debug("%s  [6] SUPPLIER      = %s\n",__FUNCTION__, listtable[6].valueStr);
	LOG_Debug("%s  [7] VERSIONID     = %d\n",__FUNCTION__, listtable[7].valueInt);

	TBParameterSetByID(ADCUNAMEZOE_NAME_HANDLE, 0, "tyyyyyyyyyyyyyyest");
	TBParameterSetByID(ADCUNAMEZOE_SW_HANDLE, 0, "testttttttttttttttt");
	int r1et =100;
	TBParameterSetByID(ADCUNAMEUVM_VERSIONID_HANDLE, 0, &r1et);
	ret = TBParameterGetByIDList(listtable, sizeof(listtable) / sizeof(TBParamList));
	LOG_Debug("%s 批量读取 %d 个参数 (%s):\n",__FUNCTION__, 8, ret == 0 ? "成功" : "失败");
    LOG_Debug("%s  [0] device_id     = %s\n",__FUNCTION__, listtable[0].valueStr);
    LOG_Debug("%s  [1] device_name   = %s\n",__FUNCTION__, listtable[1].valueStr);
    LOG_Debug("%s  [2] HV            = %s\n",__FUNCTION__, listtable[2].valueStr);
    LOG_Debug("%s  [3] BLE           = %s\n",__FUNCTION__, listtable[3].valueStr);
    LOG_Debug("%s  [4] SN            = %s\n",__FUNCTION__, listtable[4].valueStr);
    LOG_Debug("%s  [5] PARTNUM       = %s\n",__FUNCTION__, listtable[5].valueStr);
    LOG_Debug("%s  [6] SUPPLIER      = %s\n",__FUNCTION__, listtable[6].valueStr);
	LOG_Debug("%s  [7] VERSIONID     = %d\n",__FUNCTION__, listtable[7].valueInt);

   	mg_printf(conn,
   		"HTTP/1.1 200 OK\r\n"
   		"Content-Type: application/json\r\n"
   		"Connection: close\r\n\r\n");
   	
   	return 200;
   

error_exit:
	mg_send_http_error(conn, 500, "Parameter error / timeout / parse error");
	LOG_Debug("[%s] GetVinHandler erro",__FUNCTION__);
	return 500;

}
static gint32 GetVinHandler(struct mg_connection *conn, gpointer cbdata)
{
	gchar tboxVIN[18] = "00000000000000000";
	gchar tboxVN[18] = "00000000000000000";
	LOG_Debug("[%s] GetVinHandler",__FUNCTION__);
	cJSON *obj = cJSON_CreateObject();
	LOG_Debug("--%s called!", __FUNCTION__);
	if (!obj) 
	{
		/* insufficient memory? */
		LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
		goto error_exit;
	}

	//获取Tbox序列号
	ParamInfo_t param;
	param.id = E_ParamId_VIN;
	if (RET_OK != GetParam(&param))
	{
		LOG_Error("get tbox sn fail.");
		goto error_exit;
	}
	
	memcpy(tboxVIN, param.value, sizeof(tboxVIN));
	cJSON_AddStringToObject(obj, "vin", tboxVIN);
	LOG_Debug("ZELO_VIN:%s, %s called!",tboxVIN,__FUNCTION__);	

	param.id = 102;//E_ParamId_CarNum
	if (RET_OK != GetParam(&param))
	{
		LOG_Error("get tbox vn fail.");
		goto error_exit;
	}
	memcpy(tboxVN, param.value, sizeof(tboxVN));
	cJSON_AddStringToObject(obj, "vn", tboxVN);
	LOG_Debug("ZELO_VN:%s, %s called!",tboxVN,__FUNCTION__);	

	SendJSON(conn, obj);
	cJSON_Delete(obj);
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("[%s] GetVinHandler ok",__FUNCTION__);
	return 200;

error_exit:
	mg_send_http_error(conn, 500, "Parameter error / timeout / parse error");
	LOG_Debug("[%s] GetVinHandler erro",__FUNCTION__);
	return 500;
}

static gint32 PrepareHandler(struct mg_connection *conn, gpointer cbdata)
{
	guint8 incharge = 0;
	guint8 slow_charge = 0;
	guint8 fast_charge = 0;
	gint32 status = UCU_STATUS_NORMAL;
	LOG_Debug("[%s] ",__FUNCTION__);
	cJSON *obj = cJSON_CreateObject();

	if (!obj)
	{
		/* insufficient memory? */
		LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
		LOG_Debug("[%s] PrepareHandler   cJSON_CreateObject ERRO",__FUNCTION__);
		goto error_exit;
	}
	incharge = VehicleStatusGetCharge();//charge_sts
	slow_charge = VehicleStatusGetCharge();
	fast_charge = Fota_GetFastCharge();
	GWStatusGetStatus(&status);
	float raw_voltage = (float)(g_prepare_data.pwr_1 << 8 | g_prepare_data.pwr_2) / 10.0f;
	double voltage_f = roundf((int)(raw_voltage * 10)) / 10.0f;
	int speed_i = (g_prepare_data.Veh_Speed_1 << 8 | g_prepare_data.Veh_Speed_2)*0.1-100;
	uint16_t speed_t = speed_i / 3.6;

	cJSON_AddNumberToObject(obj, "gear", g_prepare_data.Act_Gear);//ok
	cJSON_AddNumberToObject(obj, "speed", speed_t);//ok
	cJSON_AddNumberToObject(obj, "hb", g_prepare_data.EPB_Status);//ok
	cJSON_AddNumberToObject(obj, "charge_sts", g_prepare_data.Charg_status);//???half connect?
	cJSON_AddNumberToObject(obj, "voltage", voltage_f);//ok
	cJSON_AddNumberToObject(obj, "vcu_result", g_prepare_data.VCU_UpdateFlag);//ok
	
	SendJSON(conn, obj);
	cJSON_Delete(obj);
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("[%s] PrepareHandler ok",__FUNCTION__);
	return 200;

error_exit:

	mg_send_http_error(conn, 500, "Parameter error / timeout / parse error");
	LOG_Debug("[%s] PrepareHandler erro",__FUNCTION__);
	return 500;
}


static gint32 EcuUpgradeHandler(struct mg_connection *conn, gpointer cbdata)
{
	gint32 postDatalen = 0;
	gchar postData[1024];
	LOG_Debug("[%s] EcuUpgradeHandler",__FUNCTION__);
	const struct mg_request_info *request_info = mg_get_request_info(conn);
	size_t content_len = request_info->content_length;  // 获取body长度
	if (content_len >= sizeof(postData)) {  // 检查缓冲区是否足够
		LOG_Debug("[%s]postData buffer too small",__FUNCTION__);
		goto error_exit;
	}
	postDatalen = mg_read(conn, postData, content_len);
	if (postDatalen != content_len) {  // 确保读取完整
		LOG_Debug("[%s]read post data incomplete",__FUNCTION__);
		goto error_exit;
	}
	postData[postDatalen] = '\0';  // 手动添加终止符
	const gchar *qs = request_info->query_string;
	gchar dev[128] = {0x0,};
	gint32 ret = RET_OK;
	// int canUpgrade = 0;
	cJSON* root = NULL;
    cJSON* config = NULL;
	cJSON* version = NULL;
	cJSON* summary = NULL;
	cJSON* adcu_file_name = NULL;
	cJSON* adcu_tv_no = NULL;
	gint8 adcu_pack = 0;
	gint8 istbox = 0;
    gint16 sql_rq_num = 0;
	system("cp -r /oemapp/upgrade/version.txt /oemdata/upgrade/ethernet/version.txt");
	reboot_num = 1;
	LOG_Debug("[%s] get into EcuUpgradeHandler:[%s]",__FUNCTION__,postData);
	mg_get_var(qs, strlen(qs), "dev", dev, sizeof(dev));
    for (int i = 0; i < sizeof(ALLOWED_DEVS)/sizeof(ALLOWED_DEVS[0]); i++) {
        if (g_strcmp0(dev, ALLOWED_DEVS[i]) == 0) {
            sql_rq_num = 104 + i*8;
            if (sql_rq_num == 208)
            {
                istbox = 1;
            }
            
            break;
        }
    }
    if (sql_rq_num == 0)
    {
        goto error_exit;
    }
	LOG_Debug("[%s] get into EcuUpgradeHandler2[%s]",__FUNCTION__,postData);
    root = cJSON_Parse(postData);
    if(NULL == root)
    {
        LOG_Error("--%s parse post fail.", __FUNCTION__);
        goto error_exit;
    }
	
	config = cJSON_GetObjectItem(root, "config");//文件包路径
	if (NULL == config)
	{
		goto error_exit;
	}
	LOG_Debug("[%s]get config:%s\r\n",__FUNCTION__,config->valuestring);
	version = cJSON_GetObjectItem(root, "version");//TBOX版本
	if (NULL == version)
	{
		goto error_exit;
	}
	LOG_Debug("[%s] version : %s\r\n",__FUNCTION__,version->valuestring);
	summary = cJSON_GetObjectItem(root, "summary");//TBOX-SHA256
	if (NULL == summary)
	{
		goto error_exit;
	}
	
	LOG_Debug("[%s]get into summary:%s\r\n",__FUNCTION__,summary->valuestring);
    adcu_file_name = cJSON_GetObjectItem(root, "filename");//TBOX-ADCU文件名
    if (NULL == adcu_file_name)
    {
        goto error_exit;

    }
	adcu_tv_no = cJSON_GetObjectItem(root, "tv_no");
	if (NULL != adcu_tv_no)
    {
        carota_adcu_tv_no = adcu_tv_no->valueint;
		LOG_Debug("[%s] adcu_tv_no : %d\r\n",__FUNCTION__,adcu_tv_no->valueint);
    }
    
    adcu_pack = 1;
    snprintf(sha_num, sizeof(sha_num), "%s", summary->valuestring);
    snprintf(ota_file_path, sizeof(ota_file_path), "http://192.168.1.201:20003/download%s/%s", config->valuestring,adcu_file_name->valuestring);
	snprintf(package_version,sizeof(package_version),"%s",version->valuestring);
	LOG_Debug("[%s]get adcu_file_name:%s\r\n",__FUNCTION__,adcu_file_name->valuestring);
	LOG_Debug("[%s]sql_rq_num:%d\r\n",__FUNCTION__,sql_rq_num);
    if (sql_rq_num!=208)
    {
        UpdateParam(sql_rq_num+7,ota_file_path,sizeof(ota_file_path));
        UpdateParam(sql_rq_num,package_version,sizeof(package_version));
        UpdateParam(sql_rq_num+6,sha_num,sizeof(sha_num));
		if (sql_rq_num == 216)
		{
			UpdateParam(sql_rq_num+8,adcu_file_name->valuestring,sizeof(adcu_file_name->valuestring));
		}
		
    }
    else
    {

        UpdateParam(215,ota_file_path,sizeof(ota_file_path));
            if (istbox)
            {
				int ret2 = system("echo \"1\" > /oemdata/tmp/tbox_upgrade_status.txt");
				LOG_Debug("--%s call UcuUpgradeStart write_status:%d.", __FUNCTION__,ret2);
                ret = UcuUpgradeStart(config->valuestring);//将config的路径输出给升级文件
                if (ret != RET_OK)
                {
                    LOG_Debug("--%s call UcuUpgradeStart fail.", __FUNCTION__);
                    goto error_exit;
                }
            }
    }
	cJSON_Delete(root);
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("[%s]EcuUpgradeHandler ok\r\n",__FUNCTION__);
	return 200;

error_exit:
	if (root)
	{
		cJSON_Delete(root);
	}
	mg_send_http_error(conn, 500, "param error");
	LOG_Debug("[%s]EcuUpgradeHandler erro\r\n",__FUNCTION__);
	return 500;
}

static gint32 EcuUpgradeResultHandler(struct mg_connection *conn, gpointer cbdata)
{
	const struct mg_request_info *request_info = mg_get_request_info(conn);
	const gchar *qs = request_info->query_string;
	gchar dev[128] = {0x0,};
	gint32 ret = RET_OK;
	gint8  sha_result = -1;
	gint8 tbox_chechk = -1;
	char* EcuAllowedDevs[] = {"zoe","zoe-conf","zoe-dnn-model","zoe-map","computing-unit-mcu","ocking-control-board"," computing-unit-kernel","camera","lidar","radar","gnss","sdc","uvm"};
	LOG_Debug("--%s called!", __FUNCTION__);
		
	mg_get_var(qs, strlen(qs), "dev", dev, sizeof(dev));
	if (0 != strcmp("tbox", dev))
	{
		LOG_Error("--%s dev[%s] compare fail.", __FUNCTION__, dev);
		//goto error_exit;
	}
	LOG_Debug("[%s]EcuUpgradeResultHandler dev:%s\r\n",__FUNCTION__,dev);
    if (strcmp("tbox", dev) == 0)//判断TBOX的包检验
    {
		sha_result = 0;
		tbox_chechk = ReadSMainModemUpgradeStatus();//走这个函数去判断升级是否成功。
		ret = tbox_chechk;
	}
	else
	{
		ret = 0;
		for (int i = 0; i < sizeof(EcuAllowedDevs)/sizeof(EcuAllowedDevs[0]); i++) {
			if (g_strcmp0(dev, EcuAllowedDevs[i]) == 0) {
				reboot_num = 0;
			}
    	}
	}
	

	cJSON *obj = cJSON_CreateObject();

	if (!obj) 
	{
		/* insufficient memory? */
		LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
		goto error_exit;
	}
	cJSON_AddStringToObject(obj, "name", dev);
	
	if ((ret < 0))
	{
		cJSON_AddStringToObject(obj, "result", "fail");

	}
	else if (ret == 1)
	{
		cJSON_AddStringToObject(obj, "result", "upgrading");
		ret = 0;
	}
	else if (ret == 2){
		cJSON_AddStringToObject(obj, "result", "success");
		ret = 0;
	}
	else
	{
		ret = 0;
		if(reboot_num !=0)
		{
			cJSON_AddStringToObject(obj, "result", "upgrading");
		}
		else
		{
			cJSON_AddStringToObject(obj, "result", "success");
		}

	}
	cJSON_AddNumberToObject(obj, "err", ret); //??????
	SendJSON(conn, obj);
	cJSON_Delete(obj);
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("--%s EcuUpgradeResultHandle ok.", __FUNCTION__);
	return 200;
error_exit:
	mg_send_http_error(conn, 500, "param error");
	LOG_Debug("--%s EcuUpgradeResultHandle erro.", __FUNCTION__);
	return 404;
}
static void resetWdTimeout(void)
{
    clock_gettime(CLOCK_MONOTONIC, &g_wdTimeout);
	g_wdTimeout.tv_sec += 60;
}

static void resetRDSWdTimeout(void)
{
    clock_gettime(CLOCK_MONOTONIC, &g_wdrdsTimeout);
	g_wdrdsTimeout.tv_sec += 60;
}

static gint32 ResetWDHandler(struct mg_connection *conn, gpointer cbdata)
{
	LOG_Debug("--%s called!", __FUNCTION__);
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	resetWdTimeout();
	LOG_Debug("--%s  ok.", __FUNCTION__);
	return 200;
}

static gint32 ModeInHandler(struct mg_connection *conn, gpointer cbdata)
{
	LOG_Debug("--%s called!", __FUNCTION__);
	int status = UCU_STATUS_FOTA;
	// int ret =UcuStatusSetStatus(UCU_STATUS_FOTA);//设置OTA状态
	UcuStatusFileGetStatus(&status);
	LOG_Debug("[%s]ModeInHandler:%d",__FUNCTION__,status);
	if (UCU_STATUS_DOTA ==status  )
	{
		mg_send_http_error(conn, 500, "MODE in DOTA mode");
		LOG_Debug("[%s]MODE in DOTA mode",__FUNCTION__);
		return 500;
	}
	int ret =UcuStatusFileSetStatus(UCU_STATUS_FOTA);//设置OTA状态
	LOG_Debug("[%s]UcuStatusFileSetStatus:%d",__FUNCTION__,ret);
	if(RET_ERR == ret)
	{
		LOG_Debug("[%s]ZEOLS UCU set erro -- called!", __FUNCTION__);
		mg_send_http_error(conn, 500, "Parameter error / timeout / parse error");
		LOG_Debug("--%s ModeInHandler erro.", __FUNCTION__);
		return 500;
	}
	
	// 保存当前的kl15状态到文件
	SaveKl15StatusToFile(g_mcuSyncKine_car.kl15);
	
	OtaPowermModeIntoStatus = g_mcuSyncKine_car.kl15;
	LOG_Debug("kiter - OtaPowermModeIntoStatus:%d .", g_mcuSyncKine_car.kl15);
	PmSetSleepEvent(PM_SLEEP_EVENT_UPGRADE);
	status = UCU_STATUS_NORMAL;
	Set_da_mode(1);
	UcuStatusFileGetStatus(&status);
	LOG_Debug("---[%s]ModeInHandler:%d",__FUNCTION__,status);
    clock_gettime(CLOCK_MONOTONIC, &g_modeChangeTimeout);
	g_modeChangeTimeout.tv_sec += 2;//默认
	LOG_Debug("--%s called!", __FUNCTION__);
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("--%s ModeInHandler ok.", __FUNCTION__);
	return 200;
}

static gint32 ModeInResultHandler(struct mg_connection *conn, gpointer cbdata)
{
	struct timespec curTime;	
	gint32 r = 0;
	LOG_Debug("--%s ModeInResultHandler!", __FUNCTION__);
	cJSON *obj = cJSON_CreateObject();
	if (!obj) 
	{
		LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
		goto error_exit;
	}
    clock_gettime(CLOCK_MONOTONIC, &curTime);
	if (timespec_compare(&curTime, &g_modeChangeTimeout) > 0)
	{
		int status = UCU_STATUS_NORMAL;
		UcuStatusFileGetStatus(&status);
		LOG_Debug("[%s]ModeInHandlertimespec_compare:%d",__FUNCTION__,status);
		if (UCU_STATUS_FOTA == status)
		{
			r = 1;
		}
		else
		{
			r= -1;
		}
	}
	else
	{
		r = 0;
	}
	Set_da_mode(2);//写入OTA模式查询
	sleep(1);	
	cJSON_AddNumberToObject(obj, "r", r); 
	cJSON_AddNumberToObject(obj, "err", 0); 
	LOG_Debug("[%s]get into ModeInResultHandler:[%d],[%x]\r\n",__FUNCTION__,r,g_prepare_data.VCU_UpdateFlag);
	SendJSON(conn, obj);
	cJSON_Delete(obj);

	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("--%s ModeInResultHandler ok", __FUNCTION__);
	return 200;

error_exit:
	mg_send_http_error(conn, 404, "cjson creat obj fail.");
	LOG_Debug("--%s ModeInResultHandler ERRO", __FUNCTION__);
	return 404;
}

static gint32 ModeOutHandler(struct mg_connection *conn, gpointer cbdata)
{
	LOG_Debug("--%s called!", __FUNCTION__);
	int status = UCU_STATUS_NORMAL;
	UcuStatusFileGetStatus(&status);
	LOG_Debug("[%s]ModeInResult:[%d]\r\n",__FUNCTION__,status);
	
	// 尝试从文件读取kl15值，如果读取失败则使用OtaPowermModeIntoStatus
	uint8_t saved_kl15_value = 0;
	if (ReadKl15StatusFromFile(&saved_kl15_value) == RET_OK) {
		g_mcuSyncKine_car.kl15 = saved_kl15_value;
	} else {
		LOG_Warn("[%s] Failed to read kl15 status from file, using OtaPowermModeIntoStatus", __FUNCTION__);
		saved_kl15_value = OtaPowermModeIntoStatus;
	}
	Set_da_mode(3);//进入ota模式退出

	sleep(1);
	LOG_Debug("kiter -OtaPowermModeIntoStatus:%d ,status :%d ,saved_kl15_value:%d.", g_mcuSyncKine_car.kl15,status,saved_kl15_value);
	if (UCU_STATUS_FOTA == status)
	{
		UcuStatusClearStatus(UCU_STATUS_FOTA);
		UcuStatusFileSetStatus(UCU_STATUS_NORMAL);
		if(g_prepare_data.AD_VehMisMod == 0)
		{
			if(saved_kl15_value==0)
			{
				ControlRemotePowerDown();
			}else if(saved_kl15_value==0x01)
			{
				ControlrestartPowerUp();
			}
		}
		LOG_Debug("[%s] AD_VehMisMod : %d ",__FUNCTION__,g_prepare_data.AD_VehMisMod);
	}
    clock_gettime(CLOCK_MONOTONIC, &g_modeChangeTimeout);
	g_modeChangeTimeout.tv_sec += 2;
	
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("[%s]normal return ModeOutHandler",__FUNCTION__);
	return 200;
}

static gint32 ModeOutResultHandler(struct mg_connection *conn, gpointer cbdata)
{
	LOG_Debug("--%s called!", __FUNCTION__);
	struct timespec curTime;	
	gint32 r = 0;

	LOG_Debug("--[%s] ModeOutResultHandler!", __FUNCTION__);
	cJSON *obj = cJSON_CreateObject();

	if (!obj) 
	{
		/* insufficient memory? */
		LOG_Debug("--%s call cJSON_CreateObject fail.", __FUNCTION__);
		goto error_exit;
	}

    clock_gettime(CLOCK_MONOTONIC, &curTime);
	if (timespec_compare(&curTime, &g_modeChangeTimeout) > 0)
	{
		int status = UCU_STATUS_NORMAL;
		UcuStatusFileGetStatus(&status);
		LOG_Debug("[%s] ModeOutResultHandler:%d", __FUNCTION__,status);
		if (UCU_STATUS_FOTA != status)
		{
			r = 1;
		}
		else
		{
			r= -1;
		}
	}
	else
	{
		r = 0;
	}
	if ((g_mcuSyncKine_car.kl15!=0x00)&&(g_car_init_power==0x00))
	{
		r = -1;
	}
	
	cJSON_AddNumberToObject(obj, "r", r); 
	cJSON_AddNumberToObject(obj, "err", 0); 
	SendJSON(conn, obj);
	cJSON_Delete(obj);
	Set_da_mode(4);//进入ota模式退出查询
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("[%s] ModeOutResultHandler!ok", __FUNCTION__);
	PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE);
	PmCleanSleepEvent(PM_SLEEP_EVENT_WAKEUPCAR);
	PmCleanSleepEvent(PM_SLEEP_EVENT_WAKEUPTBOX);

	LOG_Debug("[%s]ModeInResult-UCU_STATUS_FOTA:[%d]\r\n",__FUNCTION__);
	return 200;
error_exit:
	mg_send_http_error(conn, 500, "cjson creat obj fail.");
	LOG_Debug("[%s] ModeOutResultHandler!erro", __FUNCTION__);
	return 404;
}


static gint32 ReadyHandler(struct mg_connection *conn, gpointer cbdata)
{

	gboolean allZero = TRUE;
    ParamInfo_t param;
    param.id = E_ParamId_VIN;
    if (RET_OK != GetParam(&param)) {
        LOG_Error("%s failed to get VIN from database.", __FUNCTION__);
        goto error_exit;
    }

    if (strlen(param.value) != 17) {
        LOG_Error("%s VIN length is not 17, actual length: %zu.", __FUNCTION__, strlen(param.value));
        goto error_exit;
    }    
    for (int i = 0; i < sizeof(param.value); i++) {
        if (param.value[i] != '0') {
            allZero = FALSE;
            break;
        }
    }
    if (allZero) {
        LOG_Error("%s VIN is all zeros.", __FUNCTION__);
        goto error_exit;
    }

    if (E_MODEM_SRV_NW_REG_STATUS_REGISTERED != g_modemSrvNotify.dataRegStatus)
    {
        LOG_Error("%s nw data not register,value[%d] invalid.", __FUNCTION__, g_modemSrvNotify.dataRegStatus);
        goto error_exit;
    }
	// 检查时间戳获取状态是否正常
	time_t t = time(NULL);
	if (t == (time_t)-1) {
		LOG_Error("%s failed to get current time.", __FUNCTION__);
		goto error_exit;
	}
	struct tm *tm_info = localtime(&t);
	if (tm_info == NULL) {
		LOG_Error("%s failed to convert time to local time.", __FUNCTION__);
		goto error_exit;
	}
	int year = tm_info->tm_year + 1900;
	LOG_Debug("current year %d", year);
	if (year < 2025 || year > 2100) {
		LOG_Error("%s current year %d is not in valid range (2025-2100).", __FUNCTION__, year);
		goto error_exit;
	}
    // 端口检查通过，发送200响应
    mg_printf(conn,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Connection: close\r\n\r\n");
	LOG_Debug("[%s] ReadyHandler", __FUNCTION__);
    return 200;

error_exit:
    mg_send_http_error(conn, 500, "Not ready");
	LOG_Debug("[%s] ReadyHandler ERRO", __FUNCTION__);
    return 404;    
}

static void resetWakeoutTimeout(void)
{
    pthread_mutex_lock(&g_wakeoutTimeoutMutex);
	clock_gettime(CLOCK_MONOTONIC, &g_wakeoutTimeout);
	g_wakeoutTimeout.tv_sec += 60;       
    pthread_mutex_unlock(&g_wakeoutTimeoutMutex);
}

static gint32 WakeHandler(struct mg_connection *conn, gpointer cbdata)
{
	LOG_Debug("--%s called!", __FUNCTION__);
	int timess = 0;
	int flag = 0;
	int status_wake = UCU_STATUS_NORMAL;
	//********添加仅唤醒TBOX*************
	LOG_Debug("[%s] WakeHandler", __FUNCTION__);
	gchar wake_string[8] = {0x0,};
	const struct mg_request_info *request_info = mg_get_request_info(conn);
	
	
	if((NULL != request_info)&&(NULL != request_info->query_string))
	{
		const gchar *wake_querystring = request_info->query_string;
		LOG_Debug("ZELO Wake post:%s--%s called!",request_info->query_string, __FUNCTION__);
		if(NULL != wake_querystring && -1 == mg_get_var(wake_querystring, strlen(wake_querystring), "dev", wake_string, sizeof(wake_string)))
		{
			LOG_Error("--%s wake_string[%s] compare fail.", __FUNCTION__, wake_string);
		}
	}
	LOG_Debug("kiter  wake_string[%s]",wake_string);
	LOG_Debug("[%s]g_mcuSyncKine_car.kl15:%d,mcu_power_status:%d\r\n",__FUNCTION__,g_mcuSyncKine_car.kl15,mcu_power_status);
	UcuStatusFileGetStatus(&status_wake);

	if(g_mcuSyncKine_car.kl15==0x00)
	{
		
		g_car_init_power = 0;//1为一开始在上电。
		int p_up = RET_ERR;
		PmSetSleepEvent(PM_SLEEP_EVENT_WAKEUPTBOX);
		PmSyncDataWakeSourceAndWakeupSystem(PM_WAKEUP_SOURCE_DATA);
		PmWakeupSetDevices("mcu", 1);
		sleep(1);
		p_up=ControlRemotePowerUp();
		LOG_Debug("[%s]g_mcuSyncKine_car.kl15:%d,mcu_power_status:%d\r\n",__FUNCTION__,g_mcuSyncKine_car.kl15,mcu_power_status);
		while (g_mcuSyncKine_car.kl15 == 0x00)  // 当kl15不等于0时继续循环，直到kl15变为0
		{
			p_up=ControlRemotePowerUp();//执行整车上电
			LOG_Debug("[%s]start p_up %d ,timess:%d\r\n",__FUNCTION__,p_up,timess);
			if (timess == 10)
			{
				timess = 0;
				flag = 1;
				break;
			}
			timess++;
			sleep(1);
		}
		LOG_Debug("[%s]start kl15: %d \r\n",__FUNCTION__,g_mcuSyncKine_car.kl15);
		PmSetSleepEvent(PM_SLEEP_EVENT_WAKEUPCAR);
		PmSetSleepEvent(PM_SLEEP_EVENT_WAKEUPTBOX);
	}
	else
	{
		//唤醒整车
		g_car_init_power = 1;
		PmSetSleepEvent(PM_SLEEP_EVENT_WAKEUPCAR);
		PmCleanSleepEvent(PM_SLEEP_EVENT_WAKEUPTBOX);
		LOG_Debug("[%s]OTAWakeup_car",__FUNCTION__);
		
	}
	
	//**********************************
	PmSetSleepEvent(PM_SLEEP_EVENT_UPGRADE);
	resetWakeoutTimeout();
	gint8 value = 1;
	cJSON *obj = cJSON_CreateObject();
	if (flag == 1)
	{
		value = -1;
	}
	
	if (!obj)
	{
		LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
		goto error_exit;
	}

	cJSON_AddNumberToObject(obj, "r", value);
	SendJSON(conn, obj);
	cJSON_Delete(obj);
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("[%s]end value:%d",__FUNCTION__,value);
	return 200;
error_exit:
	mg_send_http_error(conn, 500, "Parameter error / timeout / parse error");
	LOG_Debug("[%s] erro",__FUNCTION__);
	return 500;	
}

static gint32 WakeoutHandler(struct mg_connection *conn, gpointer cbdata)
{
	LOG_Debug("--%s called!", __FUNCTION__);

	//********添加取消唤醒TBOX和取消唤醒整车*************

	gchar wakeout_string[8] = {0x0,};
	const struct mg_request_info *request_info = mg_get_request_info(conn);
	if((NULL != request_info) && (NULL != request_info->query_string))
	{
		
		const gchar *wakeout_querystring = request_info->query_string;
		LOG_Debug("ZELO Wake Cancel post:%s--%s called!",request_info->query_string, __FUNCTION__);
		if(-1 == mg_get_var(wakeout_querystring, strlen(wakeout_querystring), "dev", wakeout_string, sizeof(wakeout_string)))
		{
			LOG_Error("[%s] wakeout_string[%s] compare fail.\n", __FUNCTION__, wakeout_string);
		}
	}

	LOG_Debug("wakeout_string[%s]",wakeout_string);
	
	PmCleanSleepEvent(PM_SLEEP_EVENT_WAKEUPCAR);
	LOG_Debug("[%s]OTAWakeout_car",__FUNCTION__);		
	

    pthread_mutex_lock(&g_wakeoutTimeoutMutex);
	g_wakeoutTimeout.tv_sec = 0;       
    pthread_mutex_unlock(&g_wakeoutTimeoutMutex);

	PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE);
	guint8 value = 1;
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("[%s]wakeoutHandler",__FUNCTION__);
	return 200;
error_exit:
	mg_send_http_error(conn, 404, "Not ready");
	LOG_Debug("[%s]wakeoutHandler erro",__FUNCTION__);
	return 404;	
}

static gint32 RtctimeHandler(struct mg_connection *conn, gpointer cbdata)
{	
	LOG_Debug("--%s called!", __FUNCTION__);
	cJSON *obj = cJSON_CreateObject();

	if (!obj)
	{
		LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
		goto error_exit;
	}

	cJSON_AddNumberToObject(obj, "t", time(NULL));
	SendJSON(conn, obj);
	cJSON_Delete(obj);
	LOG_Debug("[%s]RtctimeHandler ok",__FUNCTION__);
	return 200;
error_exit:
	mg_send_http_error(conn, 500, "Not ready");
	LOG_Debug("[%s]RtctimeHandler erro",__FUNCTION__);
	return 404;	
}

void SYNCTSPHandler(struct mg_connection *conn, gpointer cbdata) {
    LOG_Debug("[%s]SYNCTSPHandler start",__FUNCTION__);
    struct mg_request_info* req_info = mg_get_request_info(conn);
    char original_body[4096] = {0};
    char *send_body = (char*)malloc(sizeof(char)*4096);
    size_t body_len = 0;

    // 读取POST请求体
    if (req_info->content_length > 0) {
        body_len = mg_read(conn, original_body, 
                          sizeof(original_body) - 1 < req_info->content_length 
                          ? sizeof(original_body) - 1 
                          : req_info->content_length);
        original_body[body_len] = '\0';
    }
    LOG_Debug("[%s]33original_body:%s",__FUNCTION__,original_body);

    // 无请求体时返回400错误
    if (body_len == 0) {
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
        free(send_body);
        return;
    }
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    char recv_buf[4096] = {0};
    ssize_t recv_len;

    // 1. 创建UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        LOG_Error("[%s] socket create failed: %s", __FUNCTION__, strerror(errno));
        mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
        free(send_body);
        return;
    }

    struct timeval timeout = {9, 0}; // 3秒0毫秒
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeout, sizeof(timeout)) < 0) {
        LOG_Warn("[%s] setsockopt SO_RCVTIMEO failed: %s", __FUNCTION__, strerror(errno));
    }


    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5010); // 目标端口
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        LOG_Error("[%s] invalid server IP: 127.0.0.1", __FUNCTION__);
        close(sockfd);
        mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
        free(send_body);
        return;
    }


    ssize_t send_len = sendto(sockfd, original_body, body_len, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (send_len < 0) {
        LOG_Error("[%s] sendto failed: %s", __FUNCTION__, strerror(errno));
        close(sockfd);
        mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
        free(send_body);
        return;
    }
    LOG_Debug("[%s] sent %zd bytes to UDP server: %s", __FUNCTION__, send_len, original_body);

    recv_len = recvfrom(sockfd, recv_buf, sizeof(recv_buf) - 1, 0, 
                        (struct sockaddr *)&server_addr, &server_addr_len);
    if (recv_len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            LOG_Warn("[%s] UDP recv timeout (3s)", __FUNCTION__);
            mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
        } else {
            LOG_Error("[%s] recvfrom failed: %s", __FUNCTION__, strerror(errno));
            mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
        }
        close(sockfd);
        free(send_body);
        return;
    }
    recv_buf[recv_len] = '\0'; // 确保字符串结束
    LOG_Debug("[%s] received %zd bytes from UDP server: %s", __FUNCTION__, recv_len, recv_buf);

    // 6. 关闭socket
    close(sockfd);
    sprintf(send_body, "%s", recv_buf);
    cJSON *obj = cJSON_Parse(send_body);
    if (!obj) {
        LOG_Error("[%s] cJSON_Parse failed: %s", __FUNCTION__, send_body);
        mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
        free(send_body);
        return;
    }

    SendJSON(conn, obj);
    cJSON_Delete(obj);
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/plain\r\n"
              "Connection: close\r\n\r\n");
    LOG_Debug("[%s] synctspHandler ok", __FUNCTION__);

    free(send_body);
    return;
}


static gint32 CantpLockHandler(struct mg_connection *conn, gpointer cbdata)
{
	LOG_Debug("--%s called!", __FUNCTION__);
	
	guint8 locked = 0;
	int errCode = 0;
	int cantp_status = 0;
	int status = UCU_STATUS_NORMAL;
	UcuStatusFileGetStatus(&status);
	LOG_Debug("[%s]CantpLockHandler status:%d",__FUNCTION__,status);
	if (UCU_STATUS_FOTA == status)
	{
		UcuStatudGetCantpLockType(&cantp_status);
		LOG_Debug("[%s]CantpLockHandler Cantp_status:%d",__FUNCTION__,cantp_status);
			if(UcuStatudCantpLock(CAN_LOCKED_FOTA) != RET_OK) {
				LOG_Error("--%s call UcuStatusCanLock fail.", __FUNCTION__);
				locked = 0;
				errCode = 15;
			} else {
				locked = 1;
				errCode = 0;
			}
	}
	
	if (UCU_STATUS_DOTA == status)
	{
		UcuStatudGetCantpLockType(&cantp_status);
		LOG_Debug("[%s]CantpLockHandler Cantp_status2:%d",__FUNCTION__,cantp_status);
			if(UcuStatudCantpLock(CAN_LOCKED_DOTA) != RET_OK) {
				LOG_Error("--%s call UcuStatusCanLock fail.", __FUNCTION__);
				locked = 0;
				errCode = 15;
			} else {
				locked = 1;
				errCode = 0;
			}
	}
	

	cJSON *obj = cJSON_CreateObject();
	if (!obj) {
		LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
		goto error_exit;
	}

	cJSON_AddNumberToObject(obj, "locked", locked);
	cJSON_AddNumberToObject(obj, "err", errCode);
	cJSON_AddStringToObject(obj, "msg", "");

	SendJSON(conn, obj);
	cJSON_Delete(obj);
	LOG_Debug("[%s]CantpLockHandler ok ",__FUNCTION__);
	return 200;
error_exit:
	mg_send_http_error(conn, 500, "error");
	LOG_Debug("[%s]CantpLockHandler erro ",__FUNCTION__);
	return 500;	
}

static gint32 CantpUnlockHandler(struct mg_connection *conn, gpointer cbdata)
{
	LOG_Debug("--%s called!", __FUNCTION__);

	guint8 unlocked = 1;
	int errCode = 0;
	int status = UCU_STATUS_NORMAL;
	UcuStatusFileGetStatus(&status);
	LOG_Debug("[%s]CantpUnlockHandler status:%d ",__FUNCTION__,status);
	if (UCU_STATUS_FOTA == status)
	{

		if(UcuStatudCantpUnlock(CAN_LOCKED_FOTA) != RET_OK) {
			LOG_Debug("--%s call UcuStatusCanLock fail.", __FUNCTION__);
			
			unlocked = 0;
			errCode = 15;
		} else {
			unlocked = 1;
			errCode = 0;
		}

	}
	if (UCU_STATUS_DOTA == status)
	{

		if(UcuStatudCantpUnlock(CAN_LOCKED_DOTA) != RET_OK) {
			LOG_Debug("--%s call UcuStatusCanLock fail.", __FUNCTION__);
			unlocked = 0;
			errCode = 15;
		} else {
			unlocked = 1;
			errCode = 0;
		}

	}


	cJSON *obj = cJSON_CreateObject();
	if (!obj) {
		LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
		goto error_exit;
	}

	cJSON_AddNumberToObject(obj, "unlocked", unlocked);
	cJSON_AddNumberToObject(obj, "err", errCode);
	cJSON_AddStringToObject(obj, "msg", "");

	SendJSON(conn, obj);
	cJSON_Delete(obj);
	LOG_Debug("[%s]CantpUnlockHandler ok",__FUNCTION__);
	return 200;
error_exit:
	mg_send_http_error(conn, 500, "error");
	LOG_Debug("[%s]CantpUnlockHandler error",__FUNCTION__);
	return 500;	
}

gint32 log_message(const struct mg_connection *conn, const gchar *message)
{
        puts(message);
        return 1;
}

void CarotaModemSrvInfo(const uint8_t *data, const uint16_t len, void* userdata)
{
	
    if((sizeof(ModemSrvNotify_t) == len) && (NULL != data))
    {
        memcpy(&g_modemSrvNotify,data,len);
    }
    else
    {
        LOG_Error("%s recv modem data leng is wrong.",__FUNCTION__);
    }

    return;
}
void OtaDotarecvSrvInfo(const uint8_t *data, const uint16_t len, void* userdata)
{
	LOG_DebugHex(data,len,"OtaDotarecvSrvInfo is:");
    if((len > 0) && (NULL != data))
    {
        OatDotaStatus = data[0];
    }
    else
    {
        LOG_Error("%s recv modem data leng is wrong.",__FUNCTION__);
    }

    return;
}
void OtaFotarecvSrvInfo(const uint8_t *data, const uint16_t len, void* userdata)
{
	LOG_DebugHex(data,len,"OtaFotarecvSrvInfo is:");
    if((len > 0) && (NULL != data))
    {
        OatFotaStatus = data[0];
    }
    else
    {
        LOG_Error("%s recv modem data leng is wrong.",__FUNCTION__);
    }

    return;
}
void VcuUpGradeInfo(const uint8_t *data, const uint16_t len, void* userdata)
{
    LOG_DebugHex(data,len,"VcuUpGradeInfo is:");
    if((sizeof(NEWMCUMPUDATA_t) == len) && (NULL != data))
    {
        if(memcmp(&g_prepare_data, data, len) != 0)
        {
            memcpy(&g_prepare_data, data, len);
        }
    }
    else
    {
         LOG_Error("%s recv modem data leng is wrong.",__FUNCTION__);
    }

    return;
}

static void CarotaPmPowerState(const uint8_t* data, const uint16_t len, void* userdata)
{
	LOG_DebugHex(data,len,"CarotaPmPowerState is:");
    if((sizeof(MPUPmStateChange_t) == len) && (NULL != data))
    {
        memcpy(&g_pmStatus, data, len);
		if(g_pmStatus.PmState != CarotaGetPmState())
		{
			CarotaSetPmState(g_pmStatus.PmState);
		}
    }
    else
    {
        LOG_Debug("%s len error!",__FUNCTION__);
    }
}
static gint32 SETTIMEHandler(struct mg_connection *conn, gpointer cbdata)
{
	const struct mg_request_info *request_info = mg_get_request_info(conn);
	gchar postData[1024];
	gint32 postDatalen = 0;
	cJSON* root = NULL;
    cJSON* timestamp = NULL;
	cJSON* eventduration = NULL;
	gint32 timeset = 0;
	gint32 durletime = 0;
	LOG_Debug("--%s called!", __FUNCTION__);
	postDatalen = mg_read(conn, postData, sizeof(postData));
	if (postDatalen <= 0)
	{
		LOG_Error("--%s post data len invalid.", __FUNCTION__);
		goto error_exit;
	}
	if (postDatalen >= sizeof(postData)) {
		postDatalen = sizeof(postData) - 1;
	}
	// 在读取的数据末尾添加字符串结束符 '\0'
	postData[postDatalen] = '\0';
    root = cJSON_Parse(postData);
    if(NULL == root)
    {
        LOG_Error("--%s parse post fail.", __FUNCTION__);
        goto error_exit;
    }

    // 4. 提取 "timestamp"
    timestamp = cJSON_GetObjectItem(root, "timestamp");
    if (timestamp == NULL || timestamp->type != cJSON_Number)
    {
        LOG_Error("--%s 'timestamp' is missing or not a number.", __FUNCTION__);
        goto error_exit;
    }
    timeset = timestamp->valueint;
    eventduration = cJSON_GetObjectItem(root, "eventduration");
    if (eventduration == NULL || eventduration->type != cJSON_Number)
    {
        LOG_Error("--%s 'eventduration' is missing or not a number.", __FUNCTION__);
        goto error_exit;
    }
    durletime = eventduration->valueint;

	cJSON_Delete(root);
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	return 200;

error_exit:
	if (root)
	{
		cJSON_Delete(root);
	}
	mg_send_http_error(conn, 500, "Parameter error / timeout / parse error");
	return 500;
}

static gint32 SETTIMECANCELHandler(struct mg_connection *conn, gpointer cbdata)
{
	gint32 ret = RET_OK;
	ret=PmCleanSleepEvent("Carota_OTA");
	if (ret == RET_ERR)
	{
		goto error_exit;
	}
	
	// ret=PmCleanSleepEvent(CPU_CAROTA_EVENT_ID);
	ret=PmCleanSleepEvent(PM_SLEEP_EVENT_WAKEUPTBOX);
	if (ret == RET_ERR)
	{
		goto error_exit;
	}
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	return 200;

error_exit:
	mg_send_http_error(conn, 500, "Parameter error / timeout / parse error");
	return 500;
}

// 假设B的URL为GETCARSTATUS("/vehiclestatus")
static gint32 CARSTATUSHandler(struct mg_connection *conn, gpointer cbdata) {
	LOG_Debug("--%s called!", __FUNCTION__);
	cJSON *obj = cJSON_CreateObject();
	if (!obj)
	{
		/* insufficient memory? */
		LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
		mg_send_http_error(conn, 500, "Parameter error / timeout / parse error");
		return 500;
	}
	LOG_Debug("--%s powermode:%d !",__FUNCTION__, g_prepare_data.powermode);
	int powertemp = 0;
	if(g_prepare_data.powermode == 1||g_prepare_data.powermode == 2)
	{
		powertemp = 1;
	}
	cJSON_AddNumberToObject(obj, "powermode", powertemp);//ok
	cJSON_AddNumberToObject(obj, "door_state", g_prepare_data.door_state);
	cJSON_AddNumberToObject(obj, "arming_state", g_prepare_data.arming_state);
    // 保存B的响应（供A参考，可选）
    char *b_json_str = cJSON_PrintUnformatted(g_b_response);
    if (b_json_str) {
        pthread_mutex_lock(&g_state_mutex);
        strncpy(g_b_response, b_json_str, sizeof(g_b_response)-1);
        pthread_mutex_unlock(&g_state_mutex);
        cJSON_free(b_json_str);
    }
    // 检查A是否在等待，若在则通知A
    pthread_mutex_lock(&g_state_mutex);
    if (g_is_waiting_for_b && g_pending_a_conn != NULL) {
        LOG_Debug("B received, notify A to continue\n");
        // 唤醒等待的A
        pthread_cond_signal(&g_b_notify_cond);
    }
    pthread_mutex_unlock(&g_state_mutex);
    // 正常返回B的响应（不修改JSON）
    SendJSON(conn, obj);
    cJSON_Delete(obj);
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("[%s]CARSTATUSHandler ok",__FUNCTION__);
	return 200;
}


static gint32 ResetRDSWDHandler(struct mg_connection *conn, gpointer cbdata)
{
	LOG_Debug("--%s called!", __FUNCTION__);

	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	//systemd("kill da server");
	resetRDSWdTimeout();
	return 200;

}

static gint32 INDOTAMODETHandler(struct mg_connection *conn, gpointer cbdata)
{
	int dotamodelresult_erro = 0;
	LOG_Debug("--%s called!", __FUNCTION__);
	cJSON *obj = cJSON_CreateObject();
	if (!obj)
	{
		/* insufficient memory? */
		LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
		goto error_exit;
	}
	{
		int status = UCU_STATUS_NORMAL;
		UcuStatusFileGetStatus(&status);
		LOG_Debug("[%s]INDOTAMODETHandler:%d",__FUNCTION__,status);
		if ((UCU_STATUS_FOTA == status))
		{
			dotamoderesult = -1;
		}
		else
		{
			int ret =UcuStatusFileSetStatus(UCU_STATUS_DOTA);//设置OTA状态
			if(RET_ERR == ret)
			{
				dotamoderesult = -1;
				LOG_Debug("ZEOLS UCU set erro --%s called!", __FUNCTION__);
				
				mg_send_http_error(conn, 500, "Parameter error / timeout / parse error");
				LOG_Debug("[%s]INDOTAMODETHandler erro:%d",__FUNCTION__,status);
				return 500;
			}
			else
			{
				PmSetSleepEvent(PM_SLEEP_EVENT_UPGRADE);
				dotamoderesult = 1;
			}
		}

	}
	
	//当前用dotamoderesult来检测，默认-1；
	cJSON_AddNumberToObject(obj, "r", dotamoderesult);//ok
	cJSON_AddNumberToObject(obj, "err", dotamodelresult_erro);//ok
	SendJSON(conn, obj);
	cJSON_Delete(obj);
	Set_da_mode(5);//进入诊断模式
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("[%s]INDOTAMODETHandler ok:%d",__FUNCTION__);
	return 200;
error_exit:
	mg_send_http_error(conn, 500, "Parameter error / timeout / parse error");
	LOG_Debug("[%s]Parameter error / timeout / parse error",__FUNCTION__);
	return 500;
}

static gint32 QUERYRESULTOFDOTAMODETHandler(struct mg_connection *conn, gpointer cbdata)////?
{
	struct timespec curTime;	
	gint32 r = 0;

	LOG_Debug("--%s called!", __FUNCTION__);

	cJSON *obj = cJSON_CreateObject();

	if (!obj) 
	{
		/* insufficient memory? */
		LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
		goto error_exit;
	}

    clock_gettime(CLOCK_MONOTONIC, &curTime);
	if (timespec_compare(&curTime, &g_modeChangeTimeout) > 0)
	{
		int status = UCU_STATUS_NORMAL;
		UcuStatusFileGetStatus(&status);
		LOG_Debug("[%s]QUERYRESULTOFDOTAMODETHandler:%d",__FUNCTION__,status);
		
		if (UCU_STATUS_DOTA == status)
		{
			r = 1;
		}
		else
		{
			r= -1;
		}
	}
	else
	{
		r = 0;
	}
	cJSON_AddNumberToObject(obj, "r", r); 
	cJSON_AddNumberToObject(obj, "err", 0); 
	SendJSON(conn, obj);
	cJSON_Delete(obj);
	Set_da_mode(6);//进入诊断模式询问
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("[%s]QUERYRESULTOFDOTAMODETHandler ok",__FUNCTION__);
	return 200;

error_exit:
	mg_send_http_error(conn, 404, "cjson creat obj fail.");
	LOG_Debug("[%s]QUERYRESULTOFDOTAMODETHandler erro",__FUNCTION__);
	return 404;
}

static gint32 DOTAMODEOUTRESULTHandler(struct mg_connection *conn, gpointer cbdata)
{
	LOG_Debug("--%s called!", __FUNCTION__);
	struct timespec curTime;	
	gint32 r = 0;
	cJSON *obj = cJSON_CreateObject();

	if (!obj) 
	{
		/* insufficient memory? */
		LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
		goto error_exit;
	}

    clock_gettime(CLOCK_MONOTONIC, &curTime);
	if (timespec_compare(&curTime, &g_modeChangeTimeout) > 0)
	{
		int status = UCU_STATUS_NORMAL;
		UcuStatusFileGetStatus(&status);

		if (UCU_STATUS_DOTA != status)
		{
			r = 1;
		}
		else
		{
			r= -1;
		}
	}
	else
	{
		r = 0;
	}
	if ((g_mcuSyncKine_car.kl15!=0x00)&&(g_car_init_power==0x00))
	{
		r = -1;
	}
	
	cJSON_AddNumberToObject(obj, "r", r); 
	cJSON_AddNumberToObject(obj, "err", 0); 
	SendJSON(conn, obj);
	cJSON_Delete(obj);
	Set_da_mode(8);//进入诊断模式退出询问
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("[%s]DOTAMODEOUTRESULTHandler ok",__FUNCTION__);
//在查询结果后再清楚休眠阻塞
	PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE);
	PmCleanSleepEvent(PM_SLEEP_EVENT_WAKEUPCAR);
	PmCleanSleepEvent(PM_SLEEP_EVENT_WAKEUPTBOX);
	sleep(1);
//在查询结果后再清楚休眠阻塞
	return 200;
error_exit:
	mg_send_http_error(conn, 404, "cjson creat obj fail.");
	LOG_Debug("[%s]DOTAMODEOUTRESULTHandler erro",__FUNCTION__);
	return 404;
}

static gint32 DOTAMODEOUTHandler(struct mg_connection *conn, gpointer cbdata)
{
	LOG_Debug("--%s called!", __FUNCTION__);
	LOG_Debug("[%s]DOTAMODEOUTHandler erro",__FUNCTION__);
	int status = UCU_STATUS_NORMAL;
	UcuStatusFileGetStatus(&status);
	if (UCU_STATUS_DOTA == status)
	{
		UcuStatusClearStatus(UCU_STATUS_DOTA);
		UcuStatusFileSetStatus(UCU_STATUS_NORMAL);
	}

	if (g_car_init_power == 0)
	{
		while (g_mcuSyncKine_car.kl15!=0x00)
		{
			ControlRemotePowerDown();
			sleep(1);
			LOG_Debug("set Cardown");
		}
		int p_down =ControlRemotePowerDown();
		LOG_Debug("Carota power down:%d",p_down);
	}
    clock_gettime(CLOCK_MONOTONIC, &g_modeChangeTimeout);
	g_modeChangeTimeout.tv_sec += 2;
	Set_da_mode(7);//退出DOTA模式
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("[%s]DOTAMODEOUTHandler ok",__FUNCTION__);
		return 200;
}



static gint32 PREPAREINSTALLHandler(struct mg_connection *conn, gpointer cbdata)
{
	const struct mg_request_info *request_info = mg_get_request_info(conn);

	gchar postData[1024];
	gint32 postDatalen = 0;
	char ecu_type[20]={0};
	cJSON* root = NULL;
    cJSON* timestamp = NULL;
	
	// gint8 sendmcu[15]= {0};
	int type_ecu = 0;
	int result = 0;
	LOG_Debug("--%s called!", __FUNCTION__);
	postDatalen = mg_read(conn, postData, sizeof(postData));
	if (postDatalen <= 0)
	{
		LOG_Error("[%s] post data len invalid.", __FUNCTION__);
		goto error_exit;
	}
	if (postDatalen >= sizeof(postData)) {
		postDatalen = sizeof(postData) - 1;
	}
	// 在读取的数据末尾添加字符串结束符 '\0'
	postData[postDatalen] = '\0';
    root = cJSON_Parse(postData);
    if(NULL == root)
    {
        LOG_Error("--%s parse post fail.", __FUNCTION__);
        goto error_exit;
    }
    // 4. 提取 "timestamp"
    timestamp = cJSON_GetObjectItem(root, "type");
    if (timestamp == NULL)
    {
        LOG_Error("--%s 'timestamp' is missing or not a number.", __FUNCTION__);
        goto error_exit;
    }
	snprintf(ecu_type, sizeof(ecu_type), "%d", timestamp->valueint);
	type_ecu =atoi(ecu_type);
	LOG_Debug("[%s]type_ecu:%d,%d\r\n",__FUNCTION__,type_ecu,timestamp->valueint);
	cJSON_Delete(root);
	LOG_Debug("[%s]type_ecu2:%x\r\n",__FUNCTION__,g_mcuSyncKine_car.kl15);
	if(g_mcuSyncKine_car.kl15!=0x00)
	{
		result = 0;
	}

	{
		if (type_ecu==1)
		{
			Set_ecu_adcu(1);
		}
		else if (type_ecu == 0)
		{
			Set_ecu_adcu(2);
		}
		
	}
	
	cJSON *obj = cJSON_CreateObject();
	cJSON_AddNumberToObject(obj, "r", result);
	SendJSON(conn, obj);
	cJSON_Delete(obj);
	
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("[%s]PREPAREINSTALLHandler ok:%x\r\n",__FUNCTION__,g_mcuSyncKine_car.kl15);
	return 200;

error_exit:
	if (root)
	{
		cJSON_Delete(root);
	}
	mg_send_http_error(conn, 500, "Parameter error / timeout / parse error");
	LOG_Debug("[%s]PREPAREINSTALLHandler erro:%x\r\n",__FUNCTION__,g_mcuSyncKine_car.kl15);
	return 500;
}

static gint32 SERVERSWITCHTHandler(struct mg_connection *conn, gpointer cbdata)
{
	gchar postData[1024];
	gint32 postDatalen = 0;
	urcBoardcast_tt msg1;
	cJSON* root = NULL;
    cJSON* timestamp = NULL;
	int master_env = 0;
	int result = 1;
	LOG_Debug("--%s called!", __FUNCTION__);
	postDatalen = mg_read(conn, postData, sizeof(postData));
	if (postDatalen <= 0)
	{
		LOG_Error("--%s post data len invalid.", __FUNCTION__);
		goto error_exit;
	}
	if (postDatalen >= sizeof(postData)) {
		postDatalen = sizeof(postData) - 1;
	}
	// 在读取的数据末尾添加字符串结束符 '\0'
	postData[postDatalen] = '\0';
    root = cJSON_Parse(postData);
    if(NULL == root)
    {
        LOG_Error("--%s parse post fail.", __FUNCTION__);
        goto error_exit;
    }
    // 4. 提取 "timestamp"
    timestamp = cJSON_GetObjectItem(root, "env");
    if (timestamp == NULL)
    {
        LOG_Error("--%s 'timestamp' is missing or not a number.", __FUNCTION__);
        goto error_exit;
    }
	master_env = timestamp->valueint;
	msg1.num = master_env;
	LOG_Debug("master_env:%d\r\n",master_env);
	cJSON_Delete(root);
	cJSON *obj = cJSON_CreateObject();
	cJSON_AddNumberToObject(obj, "r", result);
	SendJSON(conn, obj);
	cJSON_Delete(obj);
	SendMsgToAllQueue(SYS_QUE_SERVER_SWITCH, (void *)&msg1, sizeof(urcBoardcast_tt));
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	return 200;

error_exit:
	if (root)
	{
		cJSON_Delete(root);
	}
	mg_send_http_error(conn, 500, "Parameter error / timeout / parse error");
	return 500;
}

// 提取URI中的ota_name并判断是否为带/set的请求
static gint32 parse_uri(const char *uri, char *ota_name, size_t ota_name_len, gboolean *is_set_request)
{
    if (uri == NULL || ota_name == NULL || is_set_request == NULL) {
        return RET_ERR;
    }

    // 预期路径格式：/api/ota/adcu/info/{ota_name} 或 /api/ota/adcu/info/{ota_name}/set
    // 按"/"分割URI（分割后部分为：["", "api", "ota", "adcu", "info", "{ota_name}", ("set")?]）
    char *uri_copy = strdup(uri); // 复制URI避免修改原字符串
    if (uri_copy == NULL) {
        return RET_ERR;
    }

    char *parts[8] = {0}; // 存储分割后的路径部分（最多8个部分）
    int part_count = 0;
    char *token = strtok(uri_copy, "/");

    while (token != NULL && part_count < 8) {
        parts[part_count++] = token;
        token = strtok(NULL, "/");
    }

    // 验证路径前缀是否正确（前4个有效部分必须是"api","ota","adcu","info"）
    if (part_count < 5 || 
        strcmp(parts[0], "api") != 0 || 
        strcmp(parts[1], "ota") != 0 || 
        strcmp(parts[2], "adcu") != 0 || 
        strcmp(parts[3], "info") != 0) {
        free(uri_copy);
        return RET_ERR; // 路径格式错误
    }

    // 提取ota_name（第5个部分，索引4）
    strncpy(ota_name, parts[4], ota_name_len - 1);

    // 判断是否带/set（如果有第6个部分且为"set"）
    *is_set_request = (part_count == 6 && strcmp(parts[5], "set") == 0) ? TRUE : FALSE;

    free(uri_copy);
    return RET_OK;
}

static gint32 ADCU_SETMSG_QUEHandler(struct mg_connection *conn, gpointer cbdata)
{
    const struct mg_request_info *request_info = mg_get_request_info(conn);
    char ota_name[128] = {0}; // 存储提取的ota_name
    gboolean is_set_request = FALSE; // 标记是否为带/set的请求
    gint32 ret = RET_OK;

    LOG_Debug("--%s called! URI: %s\n", __FUNCTION__, request_info->local_uri);

    // 1. 解析URI，提取ota_name并判断请求类型
    if (parse_uri(request_info->local_uri, ota_name, sizeof(ota_name), &is_set_request) != RET_OK) {
        LOG_Error("--%s: 无效的URI格式: %s\n", __FUNCTION__, request_info->local_uri);
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\n{\"error\":\"invalid uri\"}");
        return RET_ERR;
    }

    // 打印提取的ota_name
    LOG_Debug("提取到的ota_name: %s\n", ota_name);
	// 4. 验证 name 是否在允许的列表中
	const char *valid_names2[] = {
		"zoe", "zoe-conf", "zoe-dnn-model", "zoe-map",
		"computing-unit-mcu", "locking-control-board", "computing-unit-kernel",
		"camera", "lidar", "radar", "gnss", "sdc", "uvm",
		NULL  // 列表结束标志
	};
	gint16 sql_num2 = 0;
	for (int i = 0; valid_names2[i] != NULL; i++) {
		if (strcmp(ota_name, valid_names2[i]) == 0) {
			sql_num2 = i*8+104;
			break;
		}
	}
    LOG_Debug("请求类型: %s\n", is_set_request ? "带/set（有content）" : "不带/set（无content）");

    // 2. 区分处理两种请求
    if (is_set_request) {
        // 带/set的请求：解析 POST 的 content
        char postData[2048] = {0};
        gint32 postDatalen = mg_read(conn, postData, sizeof(postData) - 1);
        if (postDatalen <= 0) {
            LOG_Error("--%s: 带/set的请求但无 content 数据\n", __FUNCTION__);
            mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\n{\"error\":\"no content\"}");
            ret = RET_ERR;
            goto exit;
        }
        postData[postDatalen] = '\0';
        LOG_Debug("%s 接收到的 content 数据：%s\n", __FUNCTION__, postData);

        // 解析 JSON 并打印每个键值
        cJSON *root = cJSON_Parse(postData);
        if (root == NULL) {
            LOG_Error("--%s: JSON 解析失败：%s\n", __FUNCTION__, cJSON_GetErrorPtr());
            mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\n{\"error\":\"invalid json\"}");
            ret = RET_ERR;
            goto exit;
        }

        // 先提取 ota_name 来确定模块索引
        cJSON *ota_name_item = cJSON_GetObjectItem(root, "ota_name");
        if (ota_name_item == NULL || ota_name_item->valuestring == NULL) {
            LOG_Error("--%s: JSON 中缺少 ota_name 字段\n", __FUNCTION__);
            mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\n{\"error\":\"missing ota_name\"}");
            cJSON_Delete(root);
            ret = RET_ERR;
            goto exit;
        }

        // 根据 ota_name 查找对应的模块索引
        gint32 module_index = -1;
        const char *valid_names[] = {
            "zoe", "zoe-conf", "zoe-dnn-model", "zoe-map",
            "computing-unit-mcu", "locking-control-board", "computing-unit-kernel",
            "camera", "lidar", "radar", "gnss", "sdc", "uvm",
            NULL
        };
        
        for (int i = 0; valid_names[i] != NULL; i++) {
            if (strcmp(ota_name_item->valuestring, valid_names[i]) == 0) {
                module_index = i;
                break;
            }
        }
		LOG_Debug("%s module_index:%d\n",__FUNCTION__, module_index);
        if (module_index == -1 || module_index >= OTA_MODULE_MAX) {
            LOG_Error("--%s: 无效的 ota_name: %s\n", __FUNCTION__, ota_name_item->valuestring);
            mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\n{\"error\":\"invalid ota_name\"}");
            cJSON_Delete(root);
            ret = RET_ERR;
            goto exit;
        }
        // 构建 TBParamList 数组，分别处理字符串和整型参数
        TBParamList str_param_list[7];  // 字符串参数（最多7个）
        TBParamList int_param_list[1];  // 整型参数（只有version_id）
        int str_param_count = 0;
        int int_param_count = 0;
        int base_pid = module_index * 8;  // 计算基础PID
        
        cJSON *item = NULL;
        cJSON_ArrayForEach(item, root) {
            int offset = -1;
            int is_string = 1;
            
            // 根据字段名确定偏移量和类型
            if (strcmp(item->string, "ota_name") == 0) {
                offset = 0;
            } else if (strcmp(item->string, "name_sv") == 0) {
                offset = 1;
            } else if (strcmp(item->string, "name_hv") == 0) {
                offset = 2;
            } else if (strcmp(item->string, "name_blv") == 0) {
                offset = 3;
            } else if (strcmp(item->string, "name_sn") == 0) {
                offset = 4;
            } else if (strcmp(item->string, "name_partnum") == 0) {
                offset = 5;
            } else if (strcmp(item->string, "name_supplier") == 0) {
                offset = 6;
            } else if (strcmp(item->string, "version_id") == 0) {
                offset = 7;
                is_string = 0;
            }
            
            if (offset < 0) {
                continue;  // 跳过未知字段
            }
            
            if (is_string) {
                // 处理字符串字段
                if (str_param_count < 7) {
                    str_param_list[str_param_count].pid = base_pid + offset;
                    str_param_list[str_param_count].index = 0;
                    
                    if (item->valuestring != NULL) {
                        size_t len = strlen(item->valuestring);
						// LOG_Debug("--%s ,ledn : %d, size:%d , value:%d\n", __FUNCTION__, len,sizeof(item->valuestring) ,str_param_list[str_param_count].pid);
                        if (len < MPARAM_MAX_PSRRING) {
                            memcpy(str_param_list[str_param_count].valueStr, item->valuestring, len);
                            str_param_list[str_param_count].valueStr[len] = '\0';
                        } else {
                            memcpy(str_param_list[str_param_count].valueStr, item->valuestring, MPARAM_MAX_PSRRING - 1);
                            str_param_list[str_param_count].valueStr[MPARAM_MAX_PSRRING - 1] = '\0';
                        }
                        LOG_Debug("%s, field:%s, value:'%s', pid:0x%04X\n",__FUNCTION__, item->string, item->valuestring, str_param_list[str_param_count].pid);
                    } else {
                        str_param_list[str_param_count].valueStr[0] = '\0';
                        LOG_Warn("%s, field:%s, value is NULL, set to empty\n", __FUNCTION__, item->string);
                    }
                    str_param_count++;
                }
            } else {
                // 处理整数字段（version_id）
                if (int_param_count < 1) {
                    int_param_list[int_param_count].pid = base_pid + offset;
                    int_param_list[int_param_count].index = 0;
                    int_param_list[int_param_count].valueInt = item->valueint;
                    LOG_Debug("%s, field:%s, value:%d, pid:0x%04X\n",__FUNCTION__, item->string, item->valueint, int_param_list[int_param_count].pid);
                    int_param_count++;
                }
            }
        }

        cJSON_Delete(root); // 释放JSON资源
        #if 1
        // 批量设置字符串参数
        if (str_param_count > 0) {
            int ret = TBParameterAdapterStringSetByIDList(str_param_list, str_param_count);
            if (ret != 0) {
                LOG_Error("--%s TBParameterAdapterStringSetByIDList fail, ret:%d", __FUNCTION__, ret);
                mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\n\r\n{\"error\":\"set string param fail\"}");
                return 500;
            }
            LOG_Debug("--%s batch set %d string parameters success", __FUNCTION__, str_param_count);
        }
        
        // 批量设置整型参数
        if (int_param_count > 0) {
            int ret = TBParameterAdapterSetByIDList(int_param_list, int_param_count);
            if (ret != 0) {
                LOG_Error("--%s TBParameterAdapterSetByIDList fail, ret:%d", __FUNCTION__, ret);
                mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\n\r\n{\"error\":\"set int param fail\"}");
                return 500;
            }
            LOG_Debug("--%s batch set %d int parameters success", __FUNCTION__, int_param_count);
        }
        #endif
        // 构建成功响应
        cJSON *obj = cJSON_CreateObject();
        if (!obj) {
            LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
            mg_send_http_error(conn, 500, "Insufficient memory");
            return 500;
        }
        cJSON_AddStringToObject(obj, "status", "success");
        mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Connection: close\r\n\r\n");
        SendJSON(conn, obj);
        cJSON_Delete(obj);
        return 200;

    } else {
			static char IPFILE[200]={0};
			char adcu_ersion[256] = {0};
			char name_box[128] = {0};         // 存储名字（如tbox）
			char version_id[128] = {0};   // 存储VersionID（如1.0.3）
			char sha_nummmay[128] = {0};
			struct mg_request_info* req_info = mg_get_request_info(conn);
			LOG_Debug("--%s called!", __FUNCTION__);

			const char *uri = req_info->local_uri;
			LOG_Debug("ADCU_MSG_QUEHandler:%s\r\n",uri);
			if (!uri) {
				LOG_Error("Invalid URI (NULL)");
				mg_send_http_error(conn, 400, "Invalid request URI");
				return 400;
			}
			
			const char *prefix = "/api/ota/adcu/info/";
			size_t prefix_len = strlen(prefix);

			if (strlen(uri) < prefix_len || strncmp(uri, prefix, prefix_len) != 0) {
				LOG_Error("URI not matching pattern: %s (expected prefix: %s)", uri, prefix);
				mg_send_http_error(conn, 404, "Resource not found");
				return 404;
			}

			// 3. 提取 {name} 部分
			const char *name = uri + prefix_len;
			if (*name == '\0') {  // 确保 name 不为空
				LOG_Error("Missing 'name' in URI");
				mg_send_http_error(conn, 400, "Missing 'name' parameter");
				return 400;
			}

			// 4. 验证 name 是否在允许的列表中
			const char *valid_names[] = {
				"zoe", "zoe-conf", "zoe-dnn-model", "zoe-map",
				"computing-unit-mcu", "locking-control-board", "computing-unit-kernel",
				"camera", "lidar", "radar", "gnss", "sdc", "uvm",
				NULL  // 列表结束标志
			};
			gboolean is_valid = FALSE;
			gint16 sql_num = 0;
			for (int i = 0; valid_names[i] != NULL; i++) {
				if (strcmp(name, valid_names[i]) == 0) {
					is_valid = TRUE;
					sql_num = i*8+104;
					break;
				}
			}
			if (!is_valid) {
				LOG_Error("Invalid 'name': %s", name);
				mg_send_http_error(conn, 404, "Invalid 'name' parameter");
				return 404;
			}	
			cJSON *obj = cJSON_CreateObject();
			if (!obj) {
				LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
				mg_send_http_error(conn, 500, "Insufficient memory");
				return 500;
			}
			
			// 6. 添加请求的 name 到 JSON 响应中
			ParamInfo_t param;
			param.id = sql_num+1;//E_ParamId_SN在server中：service\param\src\param_database.c
			if (RET_OK != GetParam(&param))
			{
				LOG_Error("[%s] get tbox sn fail.", __FUNCTION__);
				// goto error_exit;
			}
			LOG_Debug("%s,param.id:%d\r\n",__FUNCTION__,param.id);
			snprintf(adcu_ersion, sizeof(adcu_ersion), "%s", param.value);
			cJSON_AddStringToObject(obj, "ota_version", adcu_ersion);
			cJSON_AddStringToObject(obj, "ota_name", name);
			if (g_prepare_data.VCU_UpdateFlag==0)
			{
				cJSON_AddStringToObject(obj, "status", "dlready");
			}
			else
			{
				cJSON_AddStringToObject(obj, "status", "unready");
			}

			LOG_Debug("%s,carota_adcu_tv_no:%d\r\n",__FUNCTION__,carota_adcu_tv_no);
			//snprintf(version_id, sizeof(version_id), "%s", param1.value);
			cJSON_AddNumberToObject(obj, "version_id", carota_adcu_tv_no);
			ParamInfo_t param3;
			param3.id = sql_num+6;
			if (RET_OK != GetParam(&param3))
			{
				LOG_Error("[%s] get tbox sn fail.", __FUNCTION__);
				// goto error_exit;
			}
			LOG_Debug("%s param.id:%d\r\n",__FUNCTION__,param3.id);
			snprintf(sha_nummmay, sizeof(sha_nummmay), "%s", param3.value);
			cJSON_AddStringToObject(obj, "summary", sha_nummmay);//
			ParamInfo_t param2;
			param2.id = sql_num+7;
			if (RET_OK != GetParam(&param2))
			{
				LOG_Error("[%s] get tbox sn fail.", __FUNCTION__);
				// goto error_exit;
			}
			LOG_Debug("param.id:%d\r\n",param2.id);
			// 正确地将param2的值赋给IPFILE
			snprintf(IPFILE, sizeof(IPFILE), "%s", param2.value);
			LOG_Debug("%s param.id:%s\r\n",__FUNCTION__,IPFILE);
			cJSON_AddStringToObject(obj, "url", IPFILE);
			mg_printf(conn,
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: application/json\r\n"  // 正确的 JSON 类型
				"Connection: close\r\n\r\n");
			SendJSON(conn, obj);  // 发送 JSON 数据
			cJSON_Delete(obj);    // 释放资源
			return 200;
    }

exit:
    return ret;
}

static gint32 FileDownloadHandler(struct mg_connection *conn, void *cbdata)
{
    const struct mg_request_info *request_info = mg_get_request_info(conn);
    LOG_Debug("--%s called! URI: %s\n", __FUNCTION__, request_info->local_uri);
    
    const char *uri_prefix = "/download";
    const char *file_path = request_info->local_uri;
    
    if (strncmp(file_path, uri_prefix, strlen(uri_prefix)) != 0) {
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\nInvalid request URI");
        return 400;
    }
    
    file_path += strlen(uri_prefix);
    
    if (strstr(file_path, "..") != NULL) {
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\nIllegal path");
        return 400;
    }
    
    if (file_path[0] != '/') {
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\nInvalid path");
        return 400;
    }
    
    char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s", file_path);
        snprintf(full_path, sizeof(full_path), "%s", file_path);
    
    snprintf(full_path, sizeof(full_path), "%s", file_path);
    
    
    struct stat file_stat;
    if (stat(full_path, &file_stat) != 0) {
        LOG_Error("File not found: %s", full_path);
        mg_printf(conn, "HTTP/1.1 404 Not Found\r\n\r\nFile not found");
        return 404;
    }
    
    if (!S_ISREG(file_stat.st_mode)) {
        LOG_Error("Requested path is not a regular file: %s", full_path);
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\nNot a file");
        return 400;
    }
    LOG_Debug("%s full_path : %s\n", __FUNCTION__, full_path);
    FILE *fp = fopen(full_path, "rb");
    if (!fp) {
        LOG_Error("Failed to open file: %s", full_path);
        mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\n\r\nFailed to open file");
        return 500;
    }
    
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    LOG_Debug("%s file_size : %d \n", __FUNCTION__, file_size);
    // 设置响应头
    mg_printf(conn,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Content-Length: %ld\r\n"
        "Content-Disposition: attachment; filename=\"%s\"\r\n"
        "\r\n",
        file_size, strrchr(full_path, '/') ? strrchr(full_path, '/') + 1 : full_path);
    
    char buffer[8192];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        mg_write(conn, buffer, bytes_read);
    }
    
    fclose(fp);
    return 200;
}

static gint32 POWERSTATUSHandle(struct mg_connection *conn, gpointer cbdata)
{
	LOG_Debug("[%s] POWERSTATUSHandle",__FUNCTION__);
	cJSON *obj = cJSON_CreateObject();
	LOG_Debug("--%s called!", __FUNCTION__);
	if (!obj) 
	{
		LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
		goto error_exit;
	}
	cJSON_AddNumberToObject(obj, "r", change_power);
	LOG_Debug("POWERSTATUSHandle:%d, %s called!",change_power,__FUNCTION__);	

	SendJSON(conn, obj);
	cJSON_Delete(obj);
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	LOG_Debug("[%s] GetVinHandler ok",__FUNCTION__);
	return 200;

error_exit:
	mg_send_http_error(conn, 500, "Parameter error / timeout / parse error");
	LOG_Debug("[%s] GetVinHandler erro",__FUNCTION__);
	return 500;
}

static gint32 RESETSTATUSHandle(struct mg_connection *conn, gpointer cbdata)
{
	cJSON *obj = cJSON_CreateObject();
	LOG_Debug("--%s called!", __FUNCTION__);
	change_power = 0;
	if (!obj) 
	{
		LOG_Error("--%s call cJSON_CreateObject fail.", __FUNCTION__);
		goto error_exit;
	}
	if (change_power == 0)
	{
		cJSON_AddNumberToObject(obj, "r", 1);
	}
	else
	{
		cJSON_AddNumberToObject(obj, "r", 0);
	}
	LOG_Debug("RESETSTATUSHandle:%d, %s called!",change_power,__FUNCTION__);	
	SendJSON(conn, obj);
	cJSON_Delete(obj);
	mg_printf(conn,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n\r\n");
	// LOG_Debug("[%s] RESETSTATUSHandle ok",__FUNCTION__);
	return 200;

error_exit:
	mg_send_http_error(conn, 500, "Parameter error / timeout / parse error");
	LOG_Debug("[%s] RESETSTATUSHandle erro",__FUNCTION__);
	return 500;
}


static void KL15INEStatusCallback(const guint8 *recvData, const guint16 dataLen, void* userdata)
{
    if((NULL == recvData) || (NULL == userdata) || (8 > dataLen))
    {
        LOG_Error("%s argv is invalid, dataLen = %d", __FUNCTION__, dataLen);
        return;
    }

    LOG_Debug("%s, recv DataLen:%d, type:%08x, userdata:%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW, (uint64_t)userdata);
    LOG_DebugHex(recvData, dataLen, "KL15INEStatusCallback mcu_debug");

    pthread_mutex_lock(&g_sendHeartBeatLock);
    guint8 tmpBuf[dataLen + 1];
	memset(tmpBuf, 0, sizeof(tmpBuf));
    memcpy(tmpBuf, recvData, dataLen);
    g_mcuSyncKine_car.kl15 = tmpBuf[0];
	if((g_mcuSyncKine_car.kl15!=temp_kl15)&&(g_mcuSyncKine_car.kl15==1))
	{
		change_power = 1;
		temp_kl15 = g_mcuSyncKine_car.kl15;
	}
	if((g_mcuSyncKine_car.kl15!=temp_kl15)&&(g_mcuSyncKine_car.kl15==0))
	{
		change_power = 0;
		temp_kl15 = g_mcuSyncKine_car.kl15;
	}
    g_mcuSyncKine_car.kl30 = tmpBuf[1] * 256 + tmpBuf[2];
    g_mcuSyncKine_car.canNet = tmpBuf[7];
    LOG_Debug("%s, KL15 Status:%02x, KL30 Status:%d, canNet:%02x", __FUNCTION__, g_mcuSyncKine_car.kl15, (int32_t)g_mcuSyncKine_car.kl30, g_mcuSyncKine_car.canNet);
	
    pthread_mutex_unlock(&g_sendHeartBeatLock);
}

//MCU同步电源
static void MCUPOWERStatusCallback(const guint8 *recvData, const guint16 dataLen, void* userdata)
{
	LOG_Debug("MCUPOWERStatusCallback:%d\r\n",dataLen);
    if((NULL == recvData) || (NULL == userdata) || (4 > dataLen))
    {
        LOG_Error("%s argv is invalid, dataLen = %d", __FUNCTION__, dataLen);
        return;
    }

    LOG_Debug("%s, recv DataLen:%d, type:%08x, userdata:%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_SYNC_POWER_STATUS, (uint64_t)userdata);
    LOG_DebugHex(recvData, dataLen, "MCUPOWERStatusCallback mcu_debug");

    pthread_mutex_lock(&g_sendmcuBeatLock);
    guint8 tmpBuf[dataLen + 1];
	memset(tmpBuf, 0, sizeof(tmpBuf));
    memcpy(tmpBuf, recvData, dataLen);
	mcu_power_status = tmpBuf[2];
	LOG_Debug("mcu_power_status:%d\r\n",mcu_power_status);
    pthread_mutex_unlock(&g_sendmcuBeatLock);
}

/**
 * @brief 检测异常断电后的升级状态并处理 (g_timeout_add 回调函数)
 * @param user_data 用户数据(未使用)
 * @return TRUE - 继续定时器; FALSE - 停止定时器
 * @note 升级状态: 1-升级中, 2-升级成功, 3-升级失败
 *       ConcurrentOtaGetAllUpgradeStatus 返回: 5-升级中, 6-需要备份(成功), 8-失败
 */
static gboolean CheckAndHandlePowerOffUpgradeStatus(gpointer user_data)
{
    static int checkState = 0;  // 0=初始检查, 1=等待中
    static int timeout_counter = 0;
    const int MAX_TIMEOUT = 300;
    
    int upgradeStatus = 0;
    UpgradeStatus_E mcuStatus = UPGRADE_INIT;
    UpgradeStatus_E mpuStatus = UPGRADE_INIT;
    int ret = 0;
    
    (void)user_data; // 未使用，避免警告
    
    // 状态0: 初始检查升级状态
    if (checkState == 0) {
        upgradeStatus = ReadSMainModemUpgradeStatus();
        
        if (upgradeStatus != 1) {
            LOG_Debug("%s: Not in upgrading state, continue normal startup", __FUNCTION__);
            return G_SOURCE_REMOVE; // 停止定时器
        }
        
        LOG_Debug("%s: Current upgrade status = %d", __FUNCTION__, upgradeStatus);
        
        ret = ConcurrentOtaGetAllUpgradeStatus("/oemdata/upgrade/", &mcuStatus, &mpuStatus);
        if (ret != 0) {
            LOG_Error("%s: Failed to get upgrade status, ret = %d", __FUNCTION__, ret);
            WriteSMainModemUpgradeStatus(3);
            SrTboxSystemReboot("carota-server", REMOTE_CONTROL_REBOOT);
            return G_SOURCE_REMOVE; // 停止定时器
        }
        
        LOG_Debug("%s: MCU status = %d, MPU status = %d", __FUNCTION__, mcuStatus, mpuStatus);
        
        // 如果当前状态是升级中(5)，切换到等待状态
        if (mpuStatus == UPGRADE_MPU_FLASHING) {
            LOG_Debug("%s: MPU is still upgrading, waiting for completion...", __FUNCTION__);
            checkState = 1;  // 切换到等待状态
            timeout_counter = 0;
            return G_SOURCE_CONTINUE; // 继续定时器
        }
        // 已经成功了
        else if (mpuStatus == UPGRADE_MPU_NEED_BACKUP || mpuStatus ==  UPGRADE_INIT ) {
            LOG_Debug("%s: MPU already completed successfully", __FUNCTION__);
            WriteSMainModemUpgradeStatus(3);
            SrTboxSystemReboot("carota-server", REMOTE_CONTROL_REBOOT);
            return G_SOURCE_REMOVE; // 停止定时器
        }
        // 已经失败了
        else if (mpuStatus == UPGRADE_MPU_FAILED) {
            LOG_Debug("%s: MPU already failed", __FUNCTION__);
            WriteSMainModemUpgradeStatus(3);
            SrTboxSystemReboot("carota-server", REMOTE_CONTROL_REBOOT);
            return G_SOURCE_REMOVE; // 停止定时器
        }
        // 其他未知状态
        else {
            LOG_Debug("%s: Unknown status, mark as failed", __FUNCTION__);
            WriteSMainModemUpgradeStatus(3);
            SrTboxSystemReboot("carota-server", REMOTE_CONTROL_REBOOT);
            return G_SOURCE_REMOVE; // 停止定时器
        }
    }
    
    // 状态1: 等待OTA升级完成
    if (checkState == 1) {
        timeout_counter++;
        
        ret = ConcurrentOtaGetAllUpgradeStatus("/oemdata/upgrade/", &mcuStatus, &mpuStatus);
        if (ret != 0) {
            LOG_Error("%s: Failed to get upgrade status in loop, ret = %d", __FUNCTION__, ret);
            if (timeout_counter >= MAX_TIMEOUT) {
                goto timeout_handler;
            }
            return G_SOURCE_CONTINUE; // 继续定时器
        }
        
        LOG_Debug("%s: Waiting... MCU status = %d, MPU status = %d", __FUNCTION__, mcuStatus, mpuStatus);
        
        // 检查是否完成: 6-成功(UPGRADE_MPU_NEED_BACKUP), 8-失败(UPGRADE_MPU_FAILED)
        if (mpuStatus == UPGRADE_MPU_NEED_BACKUP) {
            LOG_Debug("%s: MPU upgrade completed successfully", __FUNCTION__);
            WriteSMainModemUpgradeStatus(2);
            SrTboxSystemReboot("carota-server", REMOTE_CONTROL_REBOOT);
            return G_SOURCE_REMOVE; // 停止定时器
        }
        
        if (mpuStatus == UPGRADE_MPU_FAILED) {
            LOG_Debug("%s: MPU upgrade failed", __FUNCTION__);
            WriteSMainModemUpgradeStatus(3);
            SrTboxSystemReboot("carota-server", REMOTE_CONTROL_REBOOT);
            return G_SOURCE_REMOVE; // 停止定时器
        }
        
        // 还在升级中，继续等待
        if (timeout_counter >= MAX_TIMEOUT) {
            goto timeout_handler;
        }
        
        return G_SOURCE_CONTINUE; // 继续定时器
    }
    
    return G_SOURCE_REMOVE; // 默认停止定时器

timeout_handler:
    LOG_Error("%s: Timeout waiting for upgrade completion", __FUNCTION__);
    WriteSMainModemUpgradeStatus(3);
    SrTboxSystemReboot("carota-server", REMOTE_CONTROL_REBOOT);
    return G_SOURCE_REMOVE; // 停止定时器
}

gint32 main(gint32 argc, gchar *argv[])
{
	const gchar *options[] = {"listening_ports",
	                         PORT,
	                         "request_timeout_ms",
	                         "10000",
	                         "error_log_file",
	                         "error.log",
	                         "enable_auth_domain_check",
	                         "no",
	                         0};

	struct mg_callbacks callbacks;
	struct mg_context *ctx;
	gint32 err = 0;
	GMainLoop *loop = NULL;
	LOG_INIT("carota");
	LOG_Debug("[%s]Carota start \r\n",__FUNCTION__);
	InitSRSdk();
	/* Check if libcivetweb has been built with all required features. */
	mg_init_library(0);
	if (err)
	{
		LOG_Debug("Cannot start CivetWeb - inconsistent build.");
		return EXIT_FAILURE;
	}
	if (ParamManagerInit("/oemdata/parameters/paramconfig.ini", NULL) != PARAM_ERR_OK) {
        printf("初始化失败\n");
        return -1;
    }
	ParamTableRegister(TBzoeStatus, "TBStatus",sizeof(TBzoeStatus)/sizeof(ParamListTable), NULL, NULL);
	ParamTableRegister(TBcomputingKernelStatus, "TBStatus",sizeof(TBcomputingKernelStatus)/sizeof(ParamListTable), NULL, NULL);


	StartQueueMsgRcvThread("carotaServer");
	SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, CarotaPmPowerState, NULL);
	SubscriptionQueueMsg(SYS_QUE_TYPE_MODEM_SIGNAL_NOFIFY,CarotaModemSrvInfo,NULL);
	SubscriptionQueueMsg(0x002f0003,OtaDotarecvSrvInfo,NULL);
	SubscriptionQueueMsg(0x002f0001,OtaFotarecvSrvInfo,NULL);
	SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW, KL15INEStatusCallback, (void *)(SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW));  //MCUPOWERStatusCallback
	SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_SYNC_POWER_STATUS, MCUPOWERStatusCallback, (void *)(SYS_QUE_TYPE_MCU_SYNC_POWER_STATUS)); 
    SubscriptionQueueMsg(0x00011900, VcuUpGradeInfo, NULL);//0x070102
	UcuUpgradeinit();


	/* Callback will print error messages to console */
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.log_message = log_message;
	/* Start CivetWeb web server */
	ctx = mg_start(&callbacks, 0, options);
	/* Check return value: */
	if (ctx == NULL)
	{
		LOG_Debug("Cannot start CivetWeb - mg_start failed.");
		return EXIT_FAILURE;
	}


    pthread_t carota_timer_thread;
    int thread_create_ret = pthread_create(&carota_timer_thread,NULL,carotaTimerThread,NULL);
    if (thread_create_ret != 0)
    {
        LOG_Error("Failed to create carota timer thread. Error code: %d", thread_create_ret);
        // 线程创建失败不终止主程序，仅打印错误日志（可根据需求调整）
    }
    else
    {
        LOG_Debug("Carota timer thread created successfully");
    }


	//获取tbox信息 
	mg_set_request_handler(ctx, GETINFO_URI, GetInfoHandler, 0);
	//获取vin
	mg_set_request_handler(ctx, GETVIN_URI, GetVinHandler, 0);
	//查询前置条件整车电量、档位、车速、充电枪连接状态（慢充）、高压状态、蓄电池电压、钥匙档位等
	mg_set_request_handler(ctx, PREPARE_URI, PrepareHandler, 0);
	//升级ECU
	mg_set_request_handler(ctx, ECUUPGRADE_URI, EcuUpgradeHandler, 0);
	//查询ECU升级结果
	mg_set_request_handler(ctx, ECUUPGRADERESULT_URI, EcuUpgradeResultHandler, 0);
	//喂狗 接口用于保持长连接，如果1分钟没有收到喂狗，UCU需要重新运行柯络达的otamaster程序
	mg_set_request_handler(ctx, RESETWD_URI, ResetWDHandler, 0);
	//设置定时时间（与预约相关）
	mg_set_request_handler(ctx, SETSCHEDULETIME, SETTIMEHandler, 0);
	//取消设置定时时间（与预约相关）
	mg_set_request_handler(ctx, CANCELSCHEDULETIME, SETTIMECANCELHandler, 0);
	//设置ucustatus为FOTA(进入OTA模式)
	mg_set_request_handler(ctx, MODEIN_URI, ModeInHandler, 0);
	//进入OTA模式结果查询
	mg_set_request_handler(ctx, MODEINRESULT_URI, ModeInResultHandler, 0);
	//退出OTA模式，清除状态，恢复到NORMAL
	mg_set_request_handler(ctx, MODEOUT_URI, ModeOutHandler, 0);
	//退出OTA模式结果查询
	mg_set_request_handler(ctx, MODEOUTRESULT_URI, ModeOutResultHandler, 0);
	//检测ucu环境
	mg_set_request_handler(ctx, READY_URI, ReadyHandler, 0);
	//唤醒整车，设置事件，阻止休眠
	mg_set_request_handler(ctx, WAKE_URI, WakeHandler, 0);
	//取消唤醒整车，清除事件，可以进入休眠
	mg_set_request_handler(ctx, WAKEOUT_URI, WakeoutHandler, 0);
	//获取RTC时间
	mg_set_request_handler(ctx, RTCTIME_URI, RtctimeHandler, 0);
	//获取远程控制指令
	mg_set_request_handler(ctx, TSPMESSAGE_URI, TspMessageHandler, 0);//接收TSP发过来的应用响应
	//cantplock上锁
	mg_set_request_handler(ctx, CANTPLOCK_URI, CantpLockHandler, 0);
	//cantplock解锁
	mg_set_request_handler(ctx, CANTPUNLOCK_URI, CantpUnlockHandler, 0);
	//DA控制TBOX本地上下高压操作
	mg_set_request_handler(ctx, PREPAREINSTALL, PREPAREINSTALLHandler, 0);
	//本地获取车辆状态
	mg_set_request_handler(ctx, GETCARSTATUS, CARSTATUSHandler, 0);
	//RDS定期喂狗
	mg_set_request_handler(ctx, RDSWATCHDOG, ResetRDSWDHandler, 0);
	//In dota mode 进入诊断模式
	mg_set_request_handler(ctx, INDOTAMODE, INDOTAMODETHandler, 0);
	//进入诊断模式查询
	mg_set_request_handler(ctx, QUERYRESULTOFDOTAMODE, QUERYRESULTOFDOTAMODETHandler, 0);
	//退出诊断模式
	mg_set_request_handler(ctx, OUTDOTAMODE, DOTAMODEOUTHandler, 0);
	//退出诊断模式查询
	mg_set_request_handler(ctx, DOTAMODEOUTRESULT, DOTAMODEOUTRESULTHandler, 0);
	//宿主环境更换
	mg_set_request_handler(ctx, SERVERSWITCH, SERVERSWITCHTHandler, 0);
	mg_set_request_handler(ctx, TBOXSETDOMAINURL, SetDomainHandler, 0);
	mg_set_request_handler(ctx, NOTUFYTBOXSLEEPURL, notifyTboxSleepHandler, 0);
	//同步状态
	mg_set_request_handler(ctx, SYNCTSP,SYNCTSPHandler, 0);
	//获取电源状态
	mg_set_request_handler(ctx,querypowerStatus,POWERSTATUSHandle,0);
	//重置电源状态
	mg_set_request_handler(ctx,resetpowerset,RESETSTATUSHandle,0);
	//ADCU消息队列设置
	mg_set_request_handler(ctx, ADCU_MSG_QUE,ADCU_SETMSG_QUEHandler,0);
	mg_set_request_handler(ctx, FILE_DOWNLOAD_URI, FileDownloadHandler, 0);

	mg_set_request_handler(ctx, TESTDEBUG_URI,TESTDEBUG_URIHandler,0); //调试接口
	mg_set_request_handler(ctx, SRUPGRADE_URL,SRUPGRADE_URLhandler,0);
	//获取车辆状态信息
	VehicleStatusInit();
	/* Wait until the server should be closed */
	//等待服务器关闭，时间60s
	resetWdTimeout();

	//关联信号回调函数
    loop = g_main_loop_new (NULL, TRUE);
	g_timeout_add(2000, CheckAndHandlePowerOffUpgradeStatus, NULL);
    g_main_loop_run (loop);
	/* Stop the server */
	mg_stop(ctx);
	LOG_Debug("Server stopped.");
	LOG_Debug("Bye!");
	return EXIT_SUCCESS;
}
