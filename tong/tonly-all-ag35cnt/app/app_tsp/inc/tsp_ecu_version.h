/**
 * tsp_ecu_version.h - ECU software version read module
 *
 * Read ECU software versions via UDS diagnostic before TSP connects to platform
 */

#ifndef __TSP_ECU_VERSION_H__
#define __TSP_ECU_VERSION_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ECU version info max length */
#define TLY_ECU_VERSION_MAX_LEN     64

/* ECU version read types - internal index for version cache */
typedef enum {
    TLY_ECU_VER_VCU = 0,
    TLY_ECU_VER_TCU = 1,
    TLY_ECU_VER_TMS = 2,  /* TMS merged (TMS_A + TMS_B) */
    /* MCU1 series - CAN3 for TM, CAN1 for others */
    TLY_ECU_VER_MCU1_TM1 = 3,
    TLY_ECU_VER_MCU1_TM2 = 4,
    TLY_ECU_VER_MCU1_PDU = 5,
    TLY_ECU_VER_MCU1_EPS = 6,
    TLY_ECU_VER_MCU1_ACM = 7,
    TLY_ECU_VER_MCU1_DCDC = 8,
    /* MCU2 series - CAN3 for TM, CAN1 for others */
    TLY_ECU_VER_MCU2_TM1 = 9,
    TLY_ECU_VER_MCU2_TM2 = 10,
    TLY_ECU_VER_MCU2_PDU = 11,
    TLY_ECU_VER_MCU2_EPS = 12,
    TLY_ECU_VER_MCU2_ACM = 13,
    TLY_ECU_VER_MCU2_DCDC = 14,
    /* MCU3 series - CAN3 */
    TLY_ECU_VER_MCU3_TM1 = 15,
    TLY_ECU_VER_MCU3_TM2 = 16,
    TLY_ECU_VER_MAX
} tly_ecu_ver_type_t;

/* ECU version info structure */
typedef struct {
    tly_ecu_ver_type_t ecu_type;          /* ECU type */
    char ecu_name[32];                    /* ECU name */
    uint8_t version[TLY_ECU_VERSION_MAX_LEN]; /* Version data */
    uint32_t version_len;                 /* Version length */
    uint8_t valid;                        /* Is valid (successfully read) */
    uint32_t phy_id;                      /* Physical CAN ID */
    uint32_t res_id;                      /* Response CAN ID */
    uint8_t channel;                      /* CAN channel */
    uint32_t did;                         /* DID for reading version */
} tly_ecu_version_info_t;

/* All ECU version info cache */
typedef struct {
    tly_ecu_version_info_t ecus[TLY_ECU_VER_MAX];
    uint32_t count;                       /* Valid ECU count */
    uint8_t initialized;                  /* Is initialized */
} tly_ecu_version_cache_t;

/**
 * Initialize ECU version read module
 * @return 0 success, negative failure
 */
int tly_ecu_version_init(void);

/**
 * Deinitialize ECU version read module
 */
void tly_ecu_version_deinit(void);

/**
 * Read all ECU software versions
 * Read each ECU's version DID via UDS diagnostic protocol
 * @return 0 success, negative failure
 */
int tly_ecu_version_read_all(void);

/**
 * Get ECU version cache
 * @return ECU version cache pointer
 */
const tly_ecu_version_cache_t *tly_ecu_version_get_cache(void);

/**
 * Get specified ECU version info
 * @param ecu_type ECU version type
 * @param version Output version data buffer
 * @param version_len Output version length
 * @return 0 success, negative failure (not read)
 */
int tly_ecu_version_get(tly_ecu_ver_type_t ecu_type, uint8_t *version, uint32_t *version_len);

/**
 * Dump ECU version info (for debug)
 */
void tly_ecu_version_dump(void);

#ifdef __cplusplus
}
#endif

#endif /* __TSP_ECU_VERSION_H__ */