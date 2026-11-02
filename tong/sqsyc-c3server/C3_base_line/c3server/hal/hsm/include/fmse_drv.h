
#ifndef __SE_DRV_H__
#define __SE_DRV_H__

#include <stdint.h>
#include <stdio.h>

/* user define SE_TYPE */
// #define SE_TYPE_SPI
#define SE_TYPE_I2C

#ifdef SE_TYPE_SPI
// #define DEV_NAME            "/dev/spidev1.0"
#define DEV_NAME "/dev/spidev6.0"

#else
// 4G TBOX 基线 cevb板子 用的是/dev/i2c-4 用之前需要先给fm芯片上电
// 卡文 cet板子 用的是/dev/i2c-3
// 五菱 cet板子 用的是/dev/i2c-0
/**基线cevb使能fm芯片电源:
 *  echo 24 > /sys/class/gpio/export
    echo out > /sys/class/gpio/gpio24/direction
    echo 1 > /sys/class/gpio/gpio24/value
*/
/**卡文cet使能fm芯片电源 需提前修改gpio设备树
 * echo 38 > /sys/class/gpio/export
   echo out > /sys/class/gpio/gpio38/direction
   echo 1 > /sys/class/gpio/gpio38/value
 */
//#define DEV_NAME "/dev/i2c-4"
// dbg
#define I2C_FMSE_DBG
#define I2C_SDK_DBG

#endif

int fmse_open(char *devname);
void fmse_close(void);
void se_select(uint8_t sel);
void se_init_spi(void);
int se_get_devid(uint8_t *rbuf, uint16_t *rlen);
uint8_t se_transceive(uint8_t *sbuf, uint16_t slen, uint8_t *rbuf, uint16_t *rlen, int timeout);

void dump_data(uint16_t len, uint8_t *buf);
void StrToHex(uint8_t *pbDest, uint8_t *pbSrc, int nLen);

#endif
