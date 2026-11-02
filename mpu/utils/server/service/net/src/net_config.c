#include <glib.h>
#include <string.h>

#include "err_code.h"
#include "sr_log.h"
#include "net_config.h"

// 定义配置文件中的各个部分和键
#define BASE_SECTION "base"
#define APN_NUM_KEY "apnNum"
#define AUTO_DIAL_KEY "netAutoDail"
#define AUTO_DNS_PATH_NAME "dnsPathName"

#define PROFILE_INDEXDX "pid"
#define APN_SECTION_PREFIX "apn"
#define PDP_TYPE_KEY "pdpType"
#define AUTH_PROTO_KEY "authProto"
#define APN_NAME_KEY "apnName"
#define USER_NAME_KEY "apnUserName"
#define PASSWORD_KEY "apnPassWd"
#define ROUTE_IP_KEY "routeIp"
#define DOMAIN_KEY "domain"
#define NAMESAVER_PRI "nameserverPri"
#define DNS_ROUTE_SET "dnsRouteSet"

int8_t LoadNetConfig(StrNetInfo_t *strNetInfo, uint8_t *apnNum, uint8_t *autoDail, uint8_t *dnsPathName, uint8_t pathNamLen) 
{
    GKeyFile *keyfile;
    GError *error = NULL;
    gboolean success = FALSE;

    if(NULL == strNetInfo || NULL == apnNum || NULL == autoDail || NULL == dnsPathName || pathNamLen <= 0)
    {
        LOG_Error("%s : parameter is NULL.",__FUNCTION__);
        return RET_ERR;
    }
    // 初始化
    memset(strNetInfo, 0, sizeof(StrNetInfo_t));

    keyfile = g_key_file_new();

    // 先从首选配置文件中加载配置
    if (!g_key_file_load_from_file(keyfile, NET_FIRST_FILE_NAME, G_KEY_FILE_NONE, &error))
    {
        LOG_Warn("Error loading first file: %s", error->message);
        g_error_free(error);
        error = NULL;

        // 如果首选配置文件加载失败，再尝试从第二个默认配置文件中加载配置
        if (!g_key_file_load_from_file(keyfile, NET_CONFIG_FILE_NAME, G_KEY_FILE_NONE, &error))
        {
            LOG_Error("Error loading second file: %s", error->message);
            g_error_free(error);
            g_key_file_free(keyfile);
            return RET_ERR;
        }
        else 
        {
            success = TRUE;
            LOG_Debug("load file %s", NET_CONFIG_FILE_NAME);
        }
    } 
    else 
    {
        success = TRUE;
        LOG_Debug("load file %s", NET_FIRST_FILE_NAME);
    }

    if (success) {
        // 读取基本配置
        *apnNum = g_key_file_get_integer(keyfile, BASE_SECTION, APN_NUM_KEY, NULL);
        if(*apnNum > NETWORK_APN_NUM) {
            *apnNum = NETWORK_APN_NUM;
        }
        *autoDail = g_key_file_get_integer(keyfile, BASE_SECTION, AUTO_DIAL_KEY, NULL);

        g_autofree gchar *val = g_key_file_get_string(keyfile, BASE_SECTION, AUTO_DNS_PATH_NAME, NULL);
        g_strlcpy(dnsPathName, val ? val : "NULL", pathNamLen);
        LOG_Debug("*apnNum :%d, *autoDail:%d, dnsPathName:%s", *apnNum, *autoDail, dnsPathName);
        // 读取每个 APN 的配置
        for (uint8_t i = 0; i < *apnNum; ++i) {
            gchar *section = g_strdup_printf("%s%d", APN_SECTION_PREFIX, i);

            strNetInfo[i].profileIndex = g_key_file_get_integer(keyfile, section, PROFILE_INDEXDX, NULL);;
            strNetInfo[i].pdpType = g_key_file_get_integer(keyfile, section, PDP_TYPE_KEY, NULL);
            strNetInfo[i].authProto = g_key_file_get_integer(keyfile, section, AUTH_PROTO_KEY, NULL);
            strNetInfo[i].nameserverPri = g_key_file_get_integer(keyfile, section, NAMESAVER_PRI, NULL);
            strNetInfo[i].dnsRouteSet = g_key_file_get_integer(keyfile, section, DNS_ROUTE_SET, NULL);
            g_strlcpy(strNetInfo[i].apnName, g_key_file_get_string(keyfile, section, APN_NAME_KEY, NULL), MODEM_TEXT_LEN);
            g_strlcpy(strNetInfo[i].usrName, g_key_file_get_string(keyfile, section, USER_NAME_KEY, NULL), MODEM_TEXT_LEN);
            g_strlcpy(strNetInfo[i].password, g_key_file_get_string(keyfile, section, PASSWORD_KEY, NULL), MODEM_TEXT_LEN);
            g_strlcpy(strNetInfo[i].privateRoute, g_key_file_get_string(keyfile, section, ROUTE_IP_KEY, NULL), CONFIG_DOMAIN_LEN);
            g_strlcpy(strNetInfo[i].domain, g_key_file_get_string(keyfile, section, DOMAIN_KEY, NULL), CONFIG_DOMAIN_LEN);

            LOG_Debug("profileIndex : %d pdpType : %d authProto : %d apnName : %s usrName : %s password : %s privateRoute : %s domain : %s",
                strNetInfo[i].profileIndex, strNetInfo[i].pdpType, strNetInfo[i].authProto, strNetInfo[i].apnName,
                strNetInfo[i].usrName, strNetInfo[i].password, strNetInfo[i].privateRoute, strNetInfo[i].domain);

            g_free(section);
        }
    }

    // 释放 GKeyFile 结构
    g_key_file_free(keyfile);

    return RET_OK;
}

