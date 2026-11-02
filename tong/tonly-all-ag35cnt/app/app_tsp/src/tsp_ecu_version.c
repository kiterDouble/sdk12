/**
 * tsp_ecu_version.c - ECU software version read module implementation
 */

#include "tsp_ecu_version.h"
#include "tsp_log.h"
#include "uds_service.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/* ECU version DID configuration */
#define TCU_VERSION_DID         0xF113
#define VCU_VERSION_DID         0x0000  /* TODO: to be filled */
#define TMS_VERSION_DID         0x0000  /* TODO: to be filled */
#define MCU_VERSION_DID         0xF195  /* MCU series version DID */

/* Default ECU configuration file path */
#define DEFAULT_ECU_CONFIG_PATH "/oemapp/config/support_ota_ecu_config.json"

/* Global ECU version cache */
static tly_ecu_version_cache_t g_ecu_version_cache;
static UdsSrvHandle_t g_uds_srv_hdl = NULL;

/* ECU configuration table - initialized from config file or defaults */
static tly_ecu_version_info_t g_ecu_config_table[TLY_ECU_VER_MAX] = {
    /* VCU - DID TBD, CAN port 3 */
    {
        .ecu_type = TLY_ECU_VER_VCU,
        .ecu_name = "VCU",
        .phy_id = 0x07DC,
        .res_id = 0x07D4,
        .channel = 3,
        .did = VCU_VERSION_DID,
        .valid = 0,
        .version_len = 0
    },
    /* TCU - DID F113, CAN port 3 */
    {
        .ecu_type = TLY_ECU_VER_TCU,
        .ecu_name = "TCU",
        .phy_id = 0x18DA03F1,
        .res_id = 0x18DAF103,
        .channel = 3,
        .did = TCU_VERSION_DID,
        .valid = 0,
        .version_len = 0
    },
    /* TMS - DID TBD, CAN port 1 (merged TMS_A + TMS_B) */
    {
        .ecu_type = TLY_ECU_VER_TMS,
        .ecu_name = "TMS",
        .phy_id = 0x067C,
        .res_id = 0x0674,
        .channel = 1,
        .did = TMS_VERSION_DID,
        .valid = 0,
        .version_len = 0
    },
    /* MCU1 series - TM on CAN3, others on CAN1 */
    {
        .ecu_type = TLY_ECU_VER_MCU1_TM1,
        .ecu_name = "MCU1_TM1",
        .phy_id = 0x18DAD2FA,
        .res_id = 0x18DAFAD2,
        .channel = 3,
        .did = MCU_VERSION_DID,
        .valid = 0,
        .version_len = 0
    },
    {
        .ecu_type = TLY_ECU_VER_MCU1_TM2,
        .ecu_name = "MCU1_TM2",
        .phy_id = 0x18DAD9FA,
        .res_id = 0x18DAFAD9,
        .channel = 3,
        .did = MCU_VERSION_DID,
        .valid = 0,
        .version_len = 0
    },
    {
        .ecu_type = TLY_ECU_VER_MCU1_PDU,
        .ecu_name = "MCU1_PDU",
        .phy_id = 0x18DAD6FA,
        .res_id = 0x18DAFAD6,
        .channel = 1,
        .did = MCU_VERSION_DID,
        .valid = 0,
        .version_len = 0
    },
    {
        .ecu_type = TLY_ECU_VER_MCU1_EPS,
        .ecu_name = "MCU1_EPS",
        .phy_id = 0x18DAD4FA,
        .res_id = 0x18DAFAD4,
        .channel = 1,
        .did = MCU_VERSION_DID,
        .valid = 0,
        .version_len = 0
    },
    {
        .ecu_type = TLY_ECU_VER_MCU1_ACM,
        .ecu_name = "MCU1_ACM",
        .phy_id = 0x18DAD5FA,
        .res_id = 0x18DAFAD5,
        .channel = 1,
        .did = MCU_VERSION_DID,
        .valid = 0,
        .version_len = 0
    },
    {
        .ecu_type = TLY_ECU_VER_MCU1_DCDC,
        .ecu_name = "MCU1_DCDC",
        .phy_id = 0x18DAD7FA,
        .res_id = 0x18DAFAD7,
        .channel = 1,
        .did = MCU_VERSION_DID,
        .valid = 0,
        .version_len = 0
    },
    /* MCU2 series - TM on CAN3, others on CAN1 */
    {
        .ecu_type = TLY_ECU_VER_MCU2_TM1,
        .ecu_name = "MCU2_TM1",
        .phy_id = 0x18DAD2FB,
        .res_id = 0x18DAFBD2,
        .channel = 3,
        .did = MCU_VERSION_DID,
        .valid = 0,
        .version_len = 0
    },
    {
        .ecu_type = TLY_ECU_VER_MCU2_TM2,
        .ecu_name = "MCU2_TM2",
        .phy_id = 0x18DAD9FB,
        .res_id = 0x18DAFBD9,
        .channel = 3,
        .did = MCU_VERSION_DID,
        .valid = 0,
        .version_len = 0
    },
    {
        .ecu_type = TLY_ECU_VER_MCU2_PDU,
        .ecu_name = "MCU2_PDU",
        .phy_id = 0x18DAD6FB,
        .res_id = 0x18DAFBD6,
        .channel = 1,
        .did = MCU_VERSION_DID,
        .valid = 0,
        .version_len = 0
    },
    {
        .ecu_type = TLY_ECU_VER_MCU2_EPS,
        .ecu_name = "MCU2_EPS",
        .phy_id = 0x18DAD4FB,
        .res_id = 0x18DAFBD4,
        .channel = 1,
        .did = MCU_VERSION_DID,
        .valid = 0,
        .version_len = 0
    },
    {
        .ecu_type = TLY_ECU_VER_MCU2_ACM,
        .ecu_name = "MCU2_ACM",
        .phy_id = 0x18DAD5FB,
        .res_id = 0x18DAFBD5,
        .channel = 1,
        .did = MCU_VERSION_DID,
        .valid = 0,
        .version_len = 0
    },
    {
        .ecu_type = TLY_ECU_VER_MCU2_DCDC,
        .ecu_name = "MCU2_DCDC",
        .phy_id = 0x18DAD7FB,
        .res_id = 0x18DAFBD7,
        .channel = 1,
        .did = MCU_VERSION_DID,
        .valid = 0,
        .version_len = 0
    },
    /* MCU3 series - all on CAN3 */
    {
        .ecu_type = TLY_ECU_VER_MCU3_TM1,
        .ecu_name = "MCU3_TM1",
        .phy_id = 0x18DAD2FC,
        .res_id = 0x18DAFCD2,
        .channel = 3,
        .did = MCU_VERSION_DID,
        .valid = 0,
        .version_len = 0
    },
    {
        .ecu_type = TLY_ECU_VER_MCU3_TM2,
        .ecu_name = "MCU3_TM2",
        .phy_id = 0x18DAD9FC,
        .res_id = 0x18DAFCD9,
        .channel = 3,
        .did = MCU_VERSION_DID,
        .valid = 0,
        .version_len = 0
    }
};

/**
 * Parse hex string to uint32
 */
static uint32_t parse_hex_id(const char *str)
{
    if (!str) return 0;
    return (uint32_t)strtoul(str, NULL, 0);
}

/**
 * Parse ECU configuration from JSON file (simple parser) 
 */
static int parse_ecu_config_from_file(const char *config_path)
{
    FILE *fp = fopen(config_path, "r");
    if (!fp) {
        TLY_LOG_W("ECU config file not found: %s, using defaults", config_path);
        return -1;
    }

    char line[256];
    char current_ecu_name[32] = {0};
    uint32_t current_phy_id = 0;
    uint32_t current_res_id = 0;
    uint8_t current_channel = 0;
    int in_ecu_block = 0;

    while (fgets(line, sizeof(line), fp)) {
        /* Look for ECU name */
        if (strstr(line, "\"name\"")) {
            char *start = strchr(line, ':');
            if (start) {
                start = strchr(start, '"');
                if (start) {
                    start++;
                    char *end = strchr(start, '"');
                    if (end) {
                        int len = end - start;
                        if (len < 32) {
                            memcpy(current_ecu_name, start, len);
                            current_ecu_name[len] = '\0';
                            in_ecu_block = 1;
                        }
                    }
                }
            }
        }
        
        /* Look for physical_id */
        if (in_ecu_block && strstr(line, "\"physical_id\"")) {
            char *start = strchr(line, ':');
            if (start) {
                start = strchr(start, '"');
                if (start) {
                    start++;
                    char *end = strchr(start, '"');
                    if (end) {
                        char buf[32];
                        int len = end - start;
                        if (len < 32) {
                            memcpy(buf, start, len);
                            buf[len] = '\0';
                            current_phy_id = parse_hex_id(buf);
                        }
                    }
                }
            }
        }

        /* Look for response_id */
        if (in_ecu_block && strstr(line, "\"response_id\"")) {
            char *start = strchr(line, ':');
            if (start) {
                start = strchr(start, '"');
                if (start) {
                    start++;
                    char *end = strchr(start, '"');
                    if (end) {
                        char buf[32];
                        int len = end - start;
                        if (len < 32) {
                            memcpy(buf, start, len);
                            buf[len] = '\0';
                            current_res_id = parse_hex_id(buf);
                        }
                    }
                }
            }
        }

        /* Look for can_port */
        if (in_ecu_block && strstr(line, "\"can_port\"")) {
            char *start = strchr(line, ':');
            if (start) {
                current_channel = (uint8_t)atoi(start + 1);
            }
        }

        /* End of ECU block - update config table */
        if (in_ecu_block && strstr(line, "}")) {
            /* Find matching ECU in config table */
            for (int i = 0; i < TLY_ECU_VER_MAX; i++) {
                if (strcmp(g_ecu_config_table[i].ecu_name, current_ecu_name) == 0) {
                    if (current_phy_id != 0) {
                        g_ecu_config_table[i].phy_id = current_phy_id;
                    }
                    if (current_res_id != 0) {
                        g_ecu_config_table[i].res_id = current_res_id;
                    }
                    if (current_channel != 0) {
                        g_ecu_config_table[i].channel = current_channel;
                    }
                    TLY_LOG_I("ECU config loaded: %s, phy_id=0x%X, res_id=0x%X, channel=%d",
                              g_ecu_config_table[i].ecu_name,
                              g_ecu_config_table[i].phy_id,
                              g_ecu_config_table[i].res_id,
                              g_ecu_config_table[i].channel);
                    break;
                }
            }
            in_ecu_block = 0;
            memset(current_ecu_name, 0, sizeof(current_ecu_name));
            current_phy_id = 0;
            current_res_id = 0;
            current_channel = 0;
        }
    }

    fclose(fp);
    return 0;
}

/**
 * Read single ECU version via UDS
 */
static int read_single_ecu_version(tly_ecu_version_info_t *ecu_info)
{
    if (!ecu_info || !g_uds_srv_hdl) {
        return -1;
    }

    /* Skip if DID is not configured */
    if (ecu_info->did == 0x0000) {
        TLY_LOG_D("ECU %s version DID not configured, skip", ecu_info->ecu_name);
        ecu_info->valid = 0;
        return 0;
    }

    uint8_t data[TLY_ECU_VERSION_MAX_LEN] = {0};
    uint32_t len = TLY_ECU_VERSION_MAX_LEN;

    TLY_LOG_I("Reading ECU %s version: phy_id=0x%X, res_id=0x%X, channel=%d, DID=0x%X",
              ecu_info->ecu_name, ecu_info->phy_id, ecu_info->res_id, 
              ecu_info->channel, ecu_info->did);

    /* Send UDS ReadDataByIdentifier request */
    int result = ReadDataByIdentifierReq(g_uds_srv_hdl, 
                                         ecu_info->phy_id, 
                                         ecu_info->res_id, 
                                         ecu_info->channel,
                                         ecu_info->did, 
                                         data, &len);

    if (result == UDS_SERVICE_RESULT_OK && len > 0) {
        memcpy(ecu_info->version, data, len);
        ecu_info->version_len = len;
        ecu_info->valid = 1;
        TLY_LOG_I("ECU %s version read success: len=%u", ecu_info->ecu_name, len);
        TLY_LOG_Dump("ECU version data:", data, len);
        return 0;
    } else {
        TLY_LOG_W("ECU %s version read failed: result=%d", ecu_info->ecu_name, result);
        ecu_info->valid = 0;
        ecu_info->version_len = 0;
        return -1;
    }
}

int tly_ecu_version_init(void)
{
    if (g_ecu_version_cache.initialized) {
        TLY_LOG_W("ECU version module already initialized");
        return 0;
    }

    /* Initialize UDS service handle with function ID 0x18DBFFF1 (broadcast) */
    uint32_t func_id = 0x18DBFFF1;
    if (UdsSrvInit(&g_uds_srv_hdl, func_id) != UDS_SERVICE_RESULT_OK) {
        TLY_LOG_E("Failed to initialize UDS service handle");
        return -1;
    }

    /* Parse ECU configuration from JSON file */
    parse_ecu_config_from_file(DEFAULT_ECU_CONFIG_PATH);

    /* Initialize cache */
    memset(&g_ecu_version_cache, 0, sizeof(g_ecu_version_cache));
    for (int i = 0; i < TLY_ECU_VER_MAX; i++) {
        memcpy(&g_ecu_version_cache.ecus[i], &g_ecu_config_table[i],
               sizeof(tly_ecu_version_info_t));
    }
    g_ecu_version_cache.count = TLY_ECU_VER_MAX;
    g_ecu_version_cache.initialized = 1;

    TLY_LOG_I("ECU version module initialized");
    return 0;
}

void tly_ecu_version_deinit(void)
{
    if (!g_ecu_version_cache.initialized) {
        return;
    }

    if (g_uds_srv_hdl) {
        /* UDS service deinit - assuming this function exists */
        /* UdsSrvDeinit(g_uds_srv_hdl); */
        g_uds_srv_hdl = NULL;
    }

    g_ecu_version_cache.initialized = 0;
    TLY_LOG_I("ECU version module deinitialized");
}

int tly_ecu_version_read_all(void)
{
    if (!g_ecu_version_cache.initialized) {
        TLY_LOG_E("ECU version module not initialized");
        return -1;
    }

    TLY_LOG_I("Start reading all ECU versions...");

    int success_count = 0;
    for (int i = 0; i < TLY_ECU_VER_MAX; i++) {
        if (read_single_ecu_version(&g_ecu_version_cache.ecus[i]) == 0) {
            success_count++;
        }
    }

    TLY_LOG_I("ECU version read complete: %d/%d success",
              success_count, TLY_ECU_VER_MAX);
    
    return (success_count > 0) ? 0 : -1;
}

const tly_ecu_version_cache_t *tly_ecu_version_get_cache(void)
{
    if (!g_ecu_version_cache.initialized) {
        return NULL;
    }
    return &g_ecu_version_cache;
}

int tly_ecu_version_get(tly_ecu_ver_type_t ecu_version_type, uint8_t *version, uint32_t *version_len)
{
    if (!g_ecu_version_cache.initialized || !version || !version_len) {
        return -1;
    }

    if (ecu_version_type >= TLY_ECU_VER_MAX) {
        return -1;
    }

    tly_ecu_version_info_t *ecu_info = &g_ecu_version_cache.ecus[ecu_version_type];
    if (!ecu_info->valid) {
        return -1;
    }

    uint32_t copy_len = (ecu_info->version_len < *version_len) ? 
                        ecu_info->version_len : *version_len;
    memcpy(version, ecu_info->version, copy_len);
    *version_len = ecu_info->version_len;

    return 0;
}

void tly_ecu_version_dump(void)
{
    if (!g_ecu_version_cache.initialized) {
        TLY_LOG_I("ECU version module not initialized");
        return;
    }

    TLY_LOG_I("=== ECU Version Cache Dump ===");
    for (int i = 0; i < TLY_ECU_VER_MAX; i++) {
        tly_ecu_version_info_t *ecu = &g_ecu_version_cache.ecus[i];
        if (ecu->valid) {
            TLY_LOG_I("  %s: DID=0x%X, len=%u, valid=%d",
                      ecu->ecu_name, ecu->did, ecu->version_len, ecu->valid);
            TLY_LOG_Dump("  Version:", ecu->version, ecu->version_len);
        } else {
            TLY_LOG_I("  %s: not available (DID=0x%X)", 
                      ecu->ecu_name, ecu->did);
        }
    }
    TLY_LOG_I("=== End of ECU Version Dump ===");
}