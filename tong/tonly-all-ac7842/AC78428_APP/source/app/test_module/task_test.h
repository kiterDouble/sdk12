#ifndef _MODULE_TEST_H
#define _MODULE_TEST_H

#include "taskManage.h"  // Include for GSENSOR_TYPE_SELECTED
//#include "hal_type_def.h"

#define SPI_SEND  0
#define SPI_REC   1

#define BLE_SEND_MAX 1024
#define BLE_REC_MAX  400
#define BLE_IBEACON  50
#define BLE_RECSUCCESS  0
#define BLE_RECFAILED  1

void TaskTest(void *pvParameters );
void Test2Task(void *pvParameters );

/*************************************************
  Function:       Qmi8a01zHalTestMain
  Description:    Test QMI8A01z sensor functionality
  Input:          none
  Output:         none
  Return:         none
  Others:         Tests sensor initialization and data reading
*************************************************/
void Qmi8a01zHalTestMain(void);

void McuCpuSync_Task(void *pvParameters );
void BLE_Task(void* parmeter);//??????1??������ by phoenix 3.3













#endif    //_MODULE_TEST_H
