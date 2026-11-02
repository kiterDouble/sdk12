#ifndef __VEHICLE_STATUS_H__
#define __VEHICLE_STATUS_H__

#include <glib.h>


gdouble VehicleStatusGetSpeed(void);
gint32 VehicleStatusGetSOC(void);
gdouble VehicleStatusGetVoltage(void);
gint32 VehicleStatusGetGear(gchar *gear,gint32 len);
gint32 VehicleStatusGetEngine(gchar *engine, gint32 len);
gint32 VehicleStatusGetPower(gchar *power, gint32 len);
gint32 VehicleStatusGetCharge(void);
gint32 VehicleStatusGetHB(void);
gint32 Fota_GetFastCharge(void);
gint32 VehicleStatusInit(void);
gint8 VehicleStatusGetARMING(void);
#endif
