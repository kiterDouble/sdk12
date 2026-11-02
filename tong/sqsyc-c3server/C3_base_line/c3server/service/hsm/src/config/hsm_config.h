
#ifndef HSM_CONFIG_H
#define HSM_CONFIG_H

#include <stdint.h>

#define HSM_DEV_PATH_MAX 32

typedef struct
{
    /* common */
    int supportIrq;      /* 1: support irq, 2: not support irq */
    char dev[HSM_DEV_PATH_MAX]; 
    int speed; 
    int resetPin;       /* reset pin */
    int irqPin;         /* irq pin */

    union {
        struct {
            int mode; 
            int bpw;        /* data size */
        } spi;
        struct {
            int slaveAddr;  /* slave address */
            int timeout;
            int retry;  
        } i2c;
    };
}HedSDKConfig_t;

typedef enum
{
    E_HSM_TYPE_CIU98BV2 = 0,
    E_HSM_TYPE_SIMULATE,
    E_HSM_TYPE_TEE,
} HsmType_e;

typedef struct
{
    int type;           /* 0: ciu98bv2, 1: simulate, 2: tee */
    int select;         /* 1: spi, 2: i2c */
    HedSDKConfig_t hedSDKConfig;
}HsmConfig_t;

int32_t HsmConfigParser(void *ctx);

#endif