#include "rtcHal.h"

#include "Gpio_Hal.h"
#include "Gpio_Hal_Types.h"
#include "Ckgen_Hal.h"
#include "I2c_Hal.h"
#include "I2c_Hal_Types.h"

/* RTC SDA Pin: PTA2 */
#define RTC_SDA_PORT PORTID_A
#define RTC_SDA_PIN 2
#define RTC_SDA_PIN_MUX PORT_MUX_ALT2

/* RTC SCL Pin: PTA3 */
#define RTC_SCL_PORT PORTID_A
#define RTC_SCL_PIN 3
#define RTC_SCL_PIN_MUX PORT_MUX_ALT2

/* RTC INT Pin: PTD1 */
#define RTC_INT_PORT PORTID_D
#define RTC_INT_PIN 1
#define RTC_INT_PIN_MUX PORT_MUX_AS_GPIO
