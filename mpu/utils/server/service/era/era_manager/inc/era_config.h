#ifndef __ECALL_CONFIG_H__
#define __ECALL_CONFIG_H__

#include "ECallMessage.h"

struct EParamMsg {
    long mtype;
    char mtext[1024];
};

int EParamQueSend(int fd, long mtype, char *data, int len);


#define TEST_CALL_OPEN (1)
#define TEST_CALL_CLOSE (0)

void EConfigInit(void);
int EConfigECallTestMode(void);
void EConfigECallTestNumberFill(char *buff, int len);
const char *EConfigECallTestNumber();
int EConfigVINFill(char *buff, int len);
int EConfigVehicleTypeFill(msd_VehicleType *vehicleType);
int VehiclePropulsionStorageTypeFill(msd_VehiclePropulsionStorageType *vehiclePropulsion);



#endif
