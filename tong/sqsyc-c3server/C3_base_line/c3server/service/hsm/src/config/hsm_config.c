#include "hsm_config.h"
#include "ctx.h"
#include "err_code.h"
#include "glib.h"
#include "sr_log.h"

int32_t HsmConfigParser(void *ctx)
{
    Ctx_t *hsmCtx = (Ctx_t *)ctx;
    int32_t ret = RET_ERR;
    GKeyFile *keyFile = NULL;
    GError *error = NULL;
    const char *section = NULL;
    char *dev = NULL;

    const char *hsmIniPath = HSM_INI_FILE_PATH;
    if (hsmCtx == NULL)
    {
        LOG_Error("%s ctx is NULL.", __FUNCTION__);
        return ret;
    }

    memset(&hsmCtx->config, 0, sizeof(hsmCtx->config));

    keyFile = g_key_file_new();
    if (keyFile == NULL)
    {
        LOG_Error("Error new key file");
        return ret;
    }

    if (0 == access(HSM_INI_FILE_PATH_PRIMARY, F_OK)) {
        hsmIniPath = HSM_INI_FILE_PATH_PRIMARY;
    }

    if (!g_key_file_load_from_file(keyFile, hsmIniPath, G_KEY_FILE_NONE, &error))
    {
        LOG_Error("Error loading key file: %s", error->message);
        g_error_free(error);
        g_key_file_free(keyFile);
        return ret;
    }

    hsmCtx->config.type = g_key_file_get_integer(keyFile, "hsm", "type", &error);
    if (error != NULL)
    {
        LOG_Warn("Failed to get integer value for key 'type', default to ciu98bv2");
        hsmCtx->config.type = E_HSM_TYPE_CIU98BV2;
        g_clear_error(&error);
    }

    hsmCtx->config.select = g_key_file_get_integer(keyFile, "hsm", "select", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'select': %s", error->message);
        g_clear_error(&error);
        g_key_file_free(keyFile);
        return ret;
    }

    hsmCtx->config.hedSDKConfig.supportIrq = g_key_file_get_integer(keyFile, "hsm", "supportIrq", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'supportIrq': %s", error->message);
        g_clear_error(&error);
        g_key_file_free(keyFile);
        return ret;
    }

    section = (hsmCtx->config.select == 2) ? "i2c" : "spi";
    dev = g_key_file_get_string(keyFile, section, "dev", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get string value for key 'dev': %s", error->message);
        g_clear_error(&error);
        g_key_file_free(keyFile);
        return ret;
    }
    strcpy(hsmCtx->config.hedSDKConfig.dev, dev);
    g_free(dev);

    hsmCtx->config.hedSDKConfig.speed = g_key_file_get_integer(keyFile, section, "speed", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'speed': %s", error->message);
        g_clear_error(&error);
        g_key_file_free(keyFile);
        return ret;
    }

    hsmCtx->config.hedSDKConfig.resetPin = g_key_file_get_integer(keyFile, section, "resetPin", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'resetPin': %s", error->message);
        g_clear_error(&error);
        g_key_file_free(keyFile);
        return ret;
    }

    hsmCtx->config.hedSDKConfig.irqPin = g_key_file_get_integer(keyFile, section, "irqPin", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'irqPin': %s", error->message);
        g_clear_error(&error);
        g_key_file_free(keyFile);
        return ret;
    }

    if (hsmCtx->config.select == 2)
    {
        hsmCtx->config.hedSDKConfig.i2c.slaveAddr = g_key_file_get_integer(keyFile, section, "slaveAddr", &error);
        if (error != NULL)
        {
            LOG_Error("Failed to get integer value for key 'irqPin': %s", error->message);
            g_clear_error(&error);
            g_key_file_free(keyFile);
            return ret;
        }

        hsmCtx->config.hedSDKConfig.i2c.timeout = g_key_file_get_integer(keyFile, section, "timeout", &error);
        if (error != NULL)
        {
            LOG_Error("Failed to get integer value for key 'irqPin': %s", error->message);
            g_clear_error(&error);
            g_key_file_free(keyFile);
            return ret;
        }

        hsmCtx->config.hedSDKConfig.i2c.retry = g_key_file_get_integer(keyFile, section, "retry", &error);
        if (error != NULL)
        {
            LOG_Error("Failed to get integer value for key 'irqPin': %s", error->message);
            g_clear_error(&error);
            g_key_file_free(keyFile);
            return ret;
        }
    }
    else if (hsmCtx->config.select == 1)
    {
        hsmCtx->config.hedSDKConfig.spi.mode = g_key_file_get_integer(keyFile, section, "mode", &error);
        if (error != NULL)
        {
            LOG_Error("Failed to get integer value for key 'irqPin': %s", error->message);
            g_clear_error(&error);
            g_key_file_free(keyFile);
            return ret;
        }
    
        hsmCtx->config.hedSDKConfig.spi.bpw = g_key_file_get_integer(keyFile, section, "bpw", &error);
        if (error != NULL)
        {
            LOG_Error("Failed to get integer value for key 'irqPin': %s", error->message);
            g_clear_error(&error);
            g_key_file_free(keyFile);
            return ret;
        }
    }
    
    g_key_file_free(keyFile);
    ret = RET_OK;
    return ret;
}
