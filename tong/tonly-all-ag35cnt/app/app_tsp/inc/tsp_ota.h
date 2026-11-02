#ifndef __TSP_OTA_H__
#define __TSP_OTA_H__

#include "tsp_common.h"
#include "tsp_protocol.h"
#include "tsp_app.h"

#ifdef __cplusplus
extern "C" {
#endif

void tly_ota_dispatch(tly_app_t *app, const tly_ota_info_t *ota);

int  tly_ota_send_upgrade_status(tly_app_t *app,
                                 uint8_t ota_cmd,
                                 uint8_t process_status,
                                 uint8_t error_status,
                                 uint16_t download_percent,
                                 uint16_t upgrade_percent);

void tly_ota_handle_upgrade_status(const uint8_t *data, uint32_t data_len);

#ifdef __cplusplus
}
#endif

#endif /* __TSP_OTA_H__ */
