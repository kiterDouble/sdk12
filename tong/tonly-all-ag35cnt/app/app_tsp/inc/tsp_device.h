#ifndef __TSP_DEVICE_H__
#define __TSP_DEVICE_H__

#include "tsp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

int  tly_device_get_vin(char *buf, int buf_len);
int  tly_device_get_iccid(char *buf, int buf_len);
uint8_t  tly_device_get_sw_version(void);
uint16_t tly_device_get_hw_version(void);
uint32_t tly_device_get_main_mcu_version(void);
uint16_t tly_device_get_sub_mcu_version(void);
uint16_t tly_device_get_non_stage4_version(void);

int  tly_device_battery_heat(uint32_t time_second);

int  tly_device_sd_card_upload(const char *username, const char *password,
                               const char *ftp_url, uint16_t port,
                               const uint8_t begin_time[6], uint16_t time_count);

int  tly_device_vin_unbind(const char *target_vin);

int  tly_device_vin_bind(const char *target_vin);

int  tly_device_time_sync(uint32_t time_second);


typedef struct {
    uint8_t handbrake_valid;    /* 手刹有效: 0=无效/未拉起, 1=有效/已拉起 */
    uint8_t zero_speed;         /* 0车速: 0=非零车速, 1=零车速 */
    uint8_t soc_50_percent;     /* SOC>=50%: 0=SOC<50%, 1=SOC>=50% */
    uint8_t key_on_position;    /* 钥匙ON档: 0=非ON档, 1=ON档 */
} tly_vehicle_state_t;


int tly_vehicle_state_get(tly_vehicle_state_t *state);

int tly_vehicle_state_check_upgrade_ready(const tly_vehicle_state_t *state);

int tly_vehicle_state_get_unready_reason(const tly_vehicle_state_t *state,
                                          char *buf, uint32_t buf_len);

#ifdef __cplusplus
}
#endif

#endif /* __TSP_DEVICE_H__ */
