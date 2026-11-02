/*************************************************************************
	> File Name: error_code.h
    > Author: 
	> Mail: 
	> Created Time: 2018年11月06日 星期二 09时55分53秒
 ************************************************************************/

#ifndef _ERROR_CODE_H
#define _ERROR_CODE_H

/* SUCCESS */
#define RET_OK (0)

/*GENERAL ERROR*/
#define RET_ERR (-1)

/* NORMAL ERROR, 0x01~0xxx */
#define RET_ERR_NORMAL (RET_ERR - 0x00)
#define RET_ERR_NORMAL_NULL_POINT (RET_ERR_NORMAL - 0x01)
#define RET_ERR_NORMAL_PARAM_INVALID (RET_ERR_NORMAL - 0x02)
#define RET_ERR_NORMAL_MALLOC_FAIL (RET_ERR_NORMAL - 0x03)
#define RET_ERR_NORMAL_TIMEOUT (RET_ERR_NORMAL - 0x04)

/* DOCAN ERROR, 0x101~0x1xx */
#define RET_ERR_DOCAN (RET_ERR - 0x100)

/* DOIP ERROR, 0x201~0x2xx */
#define RET_ERR_DOIP (RET_ERR - 0x200)

/* ECALL ERROR, 0x301~0x3xx */
#define RET_ERR_ECALL (RET_ERR - 0x300)

/* FACTORY ERROR, 0x401~0x4xx */
#define RET_ERR_FACTORY (RET_ERR - 0x400)

/* GB ERROR, 0x501~0x5xx */
#define RET_ERR_GB (RET_ERR - 0x500)

/* GNSS ERROR, 0x601~0x6xx */
#define RET_ERR_GNSS (RET_ERR - 0x600)

/* LOG ERROR, 0x701~0x7xx */
#define RET_ERR_LOG (RET_ERR - 0x700)

/* MCU ERROR, 0x801~0x8xx */
#define RET_ERR_MCU (RET_ERR - 0x800)

/* MODEM HAL ERROR, 0x901~0x9xx */
#define RET_ERR_MODEMH (RET_ERR - 0x900)
#define RET_ERR_MODEMH_SERVICE_ABORT (RET_ERR_MODEMH - 0x01)

/* MODEM SERVER ERROR, 0xA01~0xAxx */
#define RET_ERR_MODEMS (RET_ERR - 0xA00)

/* NET ERROR, 0xB01~0xBxx */
#define RET_ERR_NET (RET_ERR - 0xB00)

/* OTA ERROR, 0xC01~0xCxx */
#define RET_ERR_OTA (RET_ERR - 0xC00)

/* PARAM ERROR, 0xD01~0xdxx */
#define RET_ERR_PARAM (RET_ERR - 0xD00)

/* PM ERROR, 0xE01~0xExx */
#define RET_ERR_PM (RET_ERR - 0xE00)

/* SECURITY ERROR, 0xF01~0xFxx */
#define RET_ERR_SECURITY (RET_ERR - 0xF00)

/* SIGNAL ERROR, 0x1101~0x11xx */
#define RET_ERR_SIGNAL (RET_ERR - 0x1100)

/* TIME ERROR, 0x1201~0x12xx */
#define RET_ERR_TIME (RET_ERR - 0x1200)

/* V2X ERROR, 0x1301~0x13xx */
#define RET_ERR_V2X (RET_ERR - 0x1300)

/* UDS ERROR, 0x1401~0x14xx */
#define RET_ERR_UDS (RET_ERR - 0x1400)
#endif


