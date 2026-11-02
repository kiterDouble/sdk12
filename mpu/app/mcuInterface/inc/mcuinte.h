#ifndef __MCU_INT_H__
#define __MCU_INT_H__ 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glib.h>

#include "sdk/mcu_api.h"

#ifdef __cplusplus
extern "C" {
#endif

gint ControlRemotePowerDown(void);
gint ControlRemotePowerUp(void);
gint ControlrestartPowerUp(void);
gint Set_ecu_adcu(int num);
gint Set_da_mode(int num);
gint Set_upgrade_mode(int cmd);

#ifdef __cplusplus
}
#endif

#endif