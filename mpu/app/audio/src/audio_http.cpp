#include <unistd.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <json/json.h>
#include "include/civetweb.h"
#include "sr_log.h"
#include "err_code.h"
#include "audio_http.h"
#include "audio_aikit.h"
#include "audio_common.h"

#define NO_SSL

#ifdef NO_SSL
#define PORT "30080"
#define HOST_INFO "http://localhost:30080"
#else
#define PORT "8089r,8843s"
#define HOST_INFO "https://localhost:8843"
#endif

static pthread_mutex_t g_audioFileLock = PTHREAD_MUTEX_INITIALIZER;

int log_message(const struct mg_connection *conn, const char *message)
{
	puts(message);
	return 1;
}

int32_t AudioBackupLicenseFile(const char *fileName)
{
	int32_t retValue = RET_ERR;
	char cmdBuff[128] = {0};

	pthread_mutex_lock(&g_audioFileLock);

	if(NULL != fileName)
	{
		snprintf(cmdBuff, 128, "mv /oemdata/ftp/%s /oemdata/tts/aee.license", fileName);
		SrGetSystemRetValue("mkdir -p /oemdata/ftp/");
		SrGetSystemRetValue("mkdir -p /oemdata/tts/");
		SrGetSystemRetValue("mkdir -p /mnt/emmc/tts/");
		SrGetSystemRetValue(cmdBuff);
	}

	if(0 == access("/oemdata/tts/aee.license", F_OK))
	{
		LOG_Debug("/oemdata/tts/aee.license is exists");
		SrGetSystemRetValue("cp /oemdata/tts/aee.license /mnt/emmc/tts/aee.license ");
		SrGetSystemRetValue("cp /oemdata/tts/aee.license /oemdata/ftp/aee.license");
		retValue = RET_OK;
	}
	else if(0 == access("/mnt/emmc/tts/aee.license", F_OK))
	{
		LOG_Debug("/mnt/emmc/tts/aee.license is exists");
		SrGetSystemRetValue("cp /mnt/emmc/tts/aee.license /oemdata/tts/aee.license");
		SrGetSystemRetValue("cp /mnt/emmc/tts/aee.license /oemdata/ftp/aee.license");
		retValue = RET_OK;
	}
	else if(0 == access("/oemdata/ftp/aee.license", F_OK))
	{
		SrGetSystemRetValue("cp /oemdata/ftp/aee.license /oemdata/tts/aee.license");
		SrGetSystemRetValue("cp /oemdata/ftp/aee.license /mnt/emmc/tts/aee.license");
		LOG_Debug("/mnt/emmc/tts/aee.license is exists");
		retValue = RET_OK;
	}
	else 
	{
		LOG_Error( "license file not exists");
		retValue = RET_ERR;
	}
	pthread_mutex_unlock(&g_audioFileLock);
	return retValue;
}



int32_t AudioStrParseJson(const char *str, int32_t strLen, Json::Value &root)
{
    Json::CharReaderBuilder builder;
    builder["collectComments"] = false; // 是否保留注释（通常设为 false）
	JSONCPP_STRING errs; // 用于收集错误信息

    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

	if(NULL == str || strLen <= 0)
	{
		return RET_ERR;
	}

    bool parsingSuccessful = reader->parse(str, str + strLen, &root, &errs);

    if (!parsingSuccessful) 
	{
        LOG_Error( "Failed to parse JSON: ");
        return RET_ERR;
    }

	return RET_OK;
}
    

int32_t AudioHttpResData(struct mg_connection *conn, Json::Value root)
{
	Json::StreamWriterBuilder builder;

	std::string jsonString = Json::writeString(builder, root);

    LOG_Debug("jsonStr: %s", jsonString.c_str());
    /* Send HTTP message header */
    mg_send_http_ok(conn, "application/json; charset=utf-8", jsonString.length());

    /* Send HTTP message content */
    mg_write(conn, jsonString.c_str(), jsonString.length());

    /* Free string allocated by cJSON_Print* */

    return jsonString.length();
}


static int32_t AudioSetLicenseName(struct mg_connection *conn, void *cbdata)
{
    const struct mg_request_info *requestInfo = mg_get_request_info(conn);
    const char *method = requestInfo->request_method;
	Json::Value root;
	Json::Value reqData;
	char fileNamePath[128] = {0};
	char postData[1024] = {0};
	int32_t postDatalen = 0;

    if (0 != strcmp(method, "POST")) 
    {
		root["code"] = 405;
		root["error"] = "Method not allowed";
		root["msg"] = "This endpoint only accepts GET requests.";
        AudioHttpResData(conn, root);
        return 405;
    }

	postDatalen = mg_read(conn, postData, sizeof(postData));
	if (postDatalen <= 0)
	{
        root["code"] = 400;
		root["error"] = "Invalid request";
		root["msg"] = "Field 'email' is missing or invalid.";
        AudioHttpResData(conn, root);
		return 400;
	}

	if(AudioStrParseJson(postData, postDatalen, reqData) < 0)
	{
        root["code"] = 400;
		root["error"] = "Invalid request";
		root["msg"] = "Field 'email' is missing or invalid.";
        AudioHttpResData(conn, root);
		return 400;
	}

	if(reqData["fileName"].isNull())
	{
		root["code"] = 400;
		root["error"] = "Invalid request";
		root["msg"] = "Field 'email' is missing or invalid.";
        AudioHttpResData(conn, root);
		return 400;
	}

	if(!reqData["fileName"].isString())
	{
		root["code"] = 400;
		root["error"] = "Invalid request";
		root["msg"] = "Field 'email' is missing or invalid.";
        AudioHttpResData(conn, root);
		return 400;
	}

	snprintf(fileNamePath, 128, "%s/%s", "/oemdata/ftp/",reqData["fileName"].asString().c_str());
	if(0 != access(fileNamePath, F_OK))
	{
        root["code"] = 400;
		root["error"] = "file not exists";
		root["msg"] = "Field 'email' is missing or invalid.";
        AudioHttpResData(conn, root);
		return 400;
	}

	root["code"] = 200;
	if(RET_OK == AudioBackupLicenseFile(reqData["fileName"].asString().c_str()))
	{
		root["status"] = "success";
	}
	else
	{
		root["status"] = "field";
	}
	
	root["msg"] = "Data received";
	AudioHttpResData(conn, root);

    return 200;
}

static int32_t AudioGetActiveStatus(struct mg_connection *conn, void *cbdata)
{
	const struct mg_request_info *requestInfo = mg_get_request_info(conn);
    const char *method = requestInfo->request_method;
	Json::Value root;
	int32_t active = 0;

    if (0 != strcmp(method, "GET")) 
    {
		root["code"] = 405;
		root["error"] = "Method not allowed";
		root["msg"] = "This endpoint only accepts GET requests.";
        AudioHttpResData(conn, root);
        return 405;
    }

	if(RET_OK != GetAudioAikitStatus())
	{
		AudioUdpSend(UDP_AUDIO_BAND_ADDR, UDP_CMD_RET_PORT, "#[TTS][codec:utf][v5]音频激活测试", strlen("#[TTS][codec:utf][v5]音频激活测试"));
		usleep(2000000);
	}

	root["code"] = 200;
	if(RET_OK == GetAudioAikitStatus())
	{
		root["status"] = "success";
		active = 1;
	}
	else
	{
		root["status"] = "field";
		active = 0;
	}
	root["msg"] = "TTS Generator Success";
	root["active"] = active;
	AudioHttpResData(conn, root);

    return 200;
}


void *AudioHttpThread(void *argv)
{
	LOG_Debug("%s  tid: %d", __FUNCTION__, gettid());

	const char *options[] = {"listening_ports",
	                         PORT,
	                         "request_timeout_ms",
	                         "10000",
	                         "error_log_file",
	                         "error.log",
#ifndef NO_SSL
	                         "ssl_certificate",
	                         "./server.pem",
	                         "ssl_protocol_version",
	                         "3",
	                         "ssl_cipher_list",
	                         "DES-CBC3-SHA:AES128-SHA:AES128-GCM-SHA256",
#endif
	                         "enable_auth_domain_check",
	                         "no",
	                         0};

	struct mg_callbacks callbacks;
	struct mg_context *ctx;
	GMainLoop *loop = NULL;
	int err = 0;

/* Check if libcivetweb has been built with all required features. */
#ifndef NO_SSL
	if (!mg_check_feature(2)) {
		fprintf(stderr,
		        "Error: Embedded example built with SSL support, "
		        "but civetweb library build without.\n");
		err = 1;
	}
	mg_init_library(MG_FEATURES_SSL);

#else
	mg_init_library(0);

#endif

	if (err) {
		LOG_Error("Cannot start CivetWeb - inconsistent build.");
		return NULL;
	}

	/* Callback will print error messages to console */
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.log_message = log_message;

	/* Start CivetWeb web server */
	ctx = mg_start(&callbacks, 0, options);
	if (ctx == NULL) {
		LOG_Error("Cannot start CivetWeb - mg_start failed.");
		return NULL;
	}

	mg_set_request_handler(ctx, AUDIO_POST_SET_LICENSE_NAME, AudioSetLicenseName, 0);
	mg_set_request_handler(ctx, AUDIO_POST_GET_ACTIVE_STA, AudioGetActiveStatus, 0);
	/* Wait until the server should be closed */
    loop = g_main_loop_new (NULL, TRUE);
	// g_timeout_add(1000, checkAlive, NULL); 
	// g_timeout_add(1000, checkReboot, NULL); 
    g_main_loop_run(loop);

	/* Stop the server */
	mg_stop(ctx);

	LOG_Debug("Server stopped.");
	LOG_Debug("Bye!\n");

	return EXIT_SUCCESS;
}
