#ifndef __HAL_PORT_H__
#define __HAL_PORT_H__

#include "Gpio_Hal.h"
#include "Gpio_Hal_Types.h"


/* MPU Power Enable Pin: PTB10(Output) */
#define MPU_PWR_EN_PORT                 PORTID_B
#define MPU_PWR_EN_PIN                  10
#define MPU_PWR_EN_PIN_MUX              PORT_MUX_AS_GPIO

/* MPU Power Key Pin: PTE11(Output) */
#define MPU_PWR_KEY_PORT                PORTID_E
#define MPU_PWR_KEY_PIN                 11
#define MPU_PWR_KEY_PIN_MUX             PORT_MUX_AS_GPIO

/* MPU VBUS Pin: PTA14(Output) */
#define MPU_VBUS_PORT                   PORTID_A
#define MPU_VBUS_PIN                    14
#define MPU_VBUS_PIN_MUX                PORT_MUX_AS_GPIO

/* MCU wakeup MPU Pin: PTB8(Output) */
#define MPU_WAKEUP_PORT                 PORTID_B
#define MPU_WAKEUP_PIN                  8
#define MPU_WAKEUP_PIN_MUX              PORT_MUX_AS_GPIO

/* MCU reset MPU Pin: PTD12(Output) */
#define MPU_RESET_PORT                  PORTID_D
#define MPU_RESET_PIN                   12
#define MPU_RESET_PIN_MUX               PORT_MUX_AS_GPIO

/* MPU USBID Pin: PTC13(Output) */
#define MPU_USBID_PORT                  PORTID_C
#define MPU_USBID_PIN                   13
#define MPU_USBID_PIN_MUX               PORT_MUX_AS_GPIO


/* SPI GPIO Configuration */
/* SPI0 CLK: PTD15 */
#define MPU_HAL_SPI_CLK_PORT                    PORTID_D
#define MPU_HAL_SPI_CLK_PIN                     15
#define MPU_HAL_SPI_CLK_PIN_MUX                 PORT_MUX_ALT4
#define MPU_HAL_SPI_CLK_PORT_DEINIT             PORT_MUX_AS_GPIO

/* SPI0 MOSI: PTB4 */
#define MPU_HAL_SPI_MOSI_PORT                   PORTID_B
#define MPU_HAL_SPI_MOSI_PIN                    4
#define MPU_HAL_SPI_MOSI_PIN_MUX                PORT_MUX_ALT3
#define MPU_HAL_SPI_MOSI_PORT_DEINIT            PORT_MUX_AS_GPIO

/* SPI0 MISO: PTD16 */
#define MPU_HAL_SPI_MISO_PORT                   PORTID_D
#define MPU_HAL_SPI_MISO_PIN                    16
#define MPU_HAL_SPI_MISO_PIN_MUX                PORT_MUX_ALT4
#define MPU_HAL_SPI_MISO_PORT_DEINIT            PORT_MUX_AS_GPIO

/* SPI IRQ: PTE16 */
#define MPU_HAL_SPI_IEQ_PORT                    PORTID_E
#define MPU_HAL_SPI_IEQ_PORT_PIN                16
#define MPU_HAL_SPI_IEQ_PIN_MUX                 PORT_MUX_AS_GPIO

/* SPI REQ: PTE8 */
#define MPU_HAL_SPI_REQ_PORT                    PORTID_E
#define MPU_HAL_SPI_REQ_PIN                     8
#define MPU_HAL_SPI_REQ_PIN_MUX                 PORT_MUX_AS_GPIO
#define MPU_HAL_SPI_REQ_IRQ                     PORTE_IRQn

/* SPI0 CS: PTB5 */
#define MPU_HAL_SPI_CS_PORT                     PORTID_B
#define MPU_HAL_SPI_CS_PORT_PIN                 5
#define MPU_HAL_SPI_CS_PORT_MUX                 PORT_MUX_ALT3
#define MPU_HAL_SPI_CS_PORT_DEINIT              PORT_MUX_AS_GPIO


#endif  /* __HAL_PORT_H__ */