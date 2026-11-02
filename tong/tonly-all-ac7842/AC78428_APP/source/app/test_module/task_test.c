#include "task_test.h"

/* FreeRTOS Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
// #include "utility.h"
// #include <string.h>

//driver
#include "canHal.h"
#include "logHal.h"
#include "hsmHal.h"
#include "ecallHal.h"
#include "peripheralHal.h"
#include "powerManageHal.h"
#include "flashHal.h"

/* Include appropriate sensor HAL */
#if (GSENSOR_TYPE_SELECTED == GSENSOR_TYPE_QMI8A01Z)
#include "qmi8a01zHal.h"
#elif (GSENSOR_TYPE_SELECTED == GSENSOR_TYPE_LSM6DSM)
#include "gSensorHal.h"
#endif

//stru_can_hal_msg Test1canBuffer[50];

// #if(1)
// stru_can_hal_msg Test2canBuffer[50];
// #endif
// uint32_t canTxCount;
// uint32_t canRxCount;

#if 0
static void CanTest1(void)
{
  typeCanHandLe canHandle;
  typeTimerHandle timerHandle;
  int16_t ret;
  uint16_t i;
  stru_can_hal_msg canMsg;
  stru_can_hal_msg_filter canFilter;
  uint32_t canId;
  uint8_t canData[8];


  canHandle = CanDriverHalOpen(TBOX_CAN_CHANNEL_1);
  timerHandle = PeripheralDriverHal_TimerOpen();
  if(canHandle<0)//invalid handle
  {
    return ;
  }
  if(timerHandle<0)
  {
    return ;    
  }

  canFilter.useStdFilter = 0x01;
  canFilter.canIdStdMin = 0x00;
  canFilter.canIdStdMax = 0x7F0;
  
  canFilter.useExdtFilter = 0x00; 
  canFilter.canIdExtdMin = 0x01;
  canFilter.canIdExtdMax = 0x01;
  CanDriverHalSetFilter(canHandle,&canFilter); 
  CanDriverHalSetRxBuffer(canHandle,Test1canBuffer,50);
  
  canData[0] = 1;
  canData[1] = 0;  
  canData[2] = 4;  
  canData[3] = 5;  
  canData[4] = 6;  
  canData[5] = 7;  
  canData[6] = 8;  
  canData[7] = 9;
  canId = 0x00000100;
  //PeripheralDriverHal_TimerStart(timerHandle,100);
  canTxCount = 0;
  canRxCount = 0;
  while(1)
  {    
    /*ret = CanDriverHalTransmit(canHandle,canId,canData,8); 
    canTxCount++;*/
    
    while(1)
    {
			for(int i=0;i<10;i++)
			{
				canData[0] = canTxCount>>24;
				canData[1] = canTxCount>>16;
				canData[2] = canTxCount>>8;
				canData[3] = canTxCount>>0;
				
				canData[4] = canRxCount>>24;
				canData[5] = canRxCount>>16;
				canData[6] = canRxCount>>8;
				canData[7] = canRxCount>>0;
				ret = CanDriverHalTransmit(canHandle,canId,canData,8);
				canRxCount++;				
			}
			RTOS_HalApiWait(10);
      /*if(ret==0)
      {
        canRxCount++;
      }
      else
      {
        RTOS_HalApiWait(2);         
      } */   
 
    }

    //RTOS_HalApiWait(10);
  }  
}
#endif

// void Delay(__IO uint32_t nCount)	 //�򵥵���ʱ����
// {
// 	for(; nCount != 0; nCount--);
// }



uint32_t m_testCount = 0;
#if 0
static void CanTest2(void)
{
  typeCanHandLe canHandle;
  int16_t ret;
  uint16_t i;
  stru_can_hal_msg canMsg;
  uint32_t canId;
  uint8_t canData[64];
  stru_can_hal_msg_filter canFilter;
     //UartTest1();
  canHandle = CanDriverHalOpen(TBOX_CAN_CHANNEL_1);
  if(canHandle<0)//invalid handle
  {
    return ;
  }
  canFilter.useStdFilter = 0x01;
  canFilter.canIdStdMin = 0x00;
  canFilter.canIdStdMax = 0x100;
  
  canFilter.useExdtFilter = 0x00; 
  canFilter.canIdExtdMin = 0x01;
  canFilter.canIdExtdMax = 0x01;
  CanDriverHalSetFilter(canHandle,&canFilter); 
   
  CanDriverHalSetRxBuffer(canHandle,Test2canBuffer,150);
	memset(canData,0,64);


  canId = 0x00000101;  
	
	TBOX_PRINT("Can Test2 is start...\r\n");
	//FLEXCAN0_transmit_msg();
  while(1)
  { 
    ret = CanDriverHalReceiveWait(canHandle,&canMsg,100);
    if(ret==0)
    {
   //TBOX_PRINT("Can Test2 receive can data...\r\n");
   canData[1]++;
    memcpy(canData,&canMsg.canData,64);
            //canData[24] = canMsg.canData[24];
   m_testCount++;
   ret = CanDriverHalTransmit(canHandle,canId,canData,32);
   TBOX_PRINT("Can Test2 transmit can data...\r\n");
      i++;
    }
    else
    {
      //RTOS_HalApiWait(10);
    }
		
  }  
}
#endif

#if 0
static void CanTest2(void)
{
  typeCanHandLe canHandle;
  int16_t ret;
  uint16_t i;
  stru_can_hal_msg canMsg;
  uint32_t canId;
  uint8_t canData[64];
  stru_can_hal_msg_filter canFilter;
     //UartTest1();
  canHandle = CanDriverHalOpen(TBOX_CAN_CHANNEL_1);
  if(canHandle<0)//invalid handle
  {
    return ;
  }
  canFilter.useStdFilter = 0x01;
  canFilter.canIdStdMin = 0x00;
  canFilter.canIdStdMax = 0x100;
  
  canFilter.useExdtFilter = 0x00; 
  canFilter.canIdExtdMin = 0x01;
  canFilter.canIdExtdMax = 0x01;
  CanDriverHalSetFilter(canHandle,&canFilter); 
   
  CanDriverHalSetRxBuffer(canHandle,Test2canBuffer,50);
  memset(canData,0,64);
  canData[0] = 2;
  canData[1] = 0;  
  canData[2] = 0;  
  canData[3] = 0;  
  canData[4] = 0xAA;  
  canData[5] = 0;  
  canData[6] = 0;  
  canData[7] = 0xBB;
  
  canData[14] = 0xcc;
  
  
  canId = 0x00000101;  
 
 TBOX_PRINT("Can Test2 is start...\r\n");
 //FLEXCAN0_transmit_msg();
  while(1)
  { 
    ret = CanDriverHalReceiveWait(canHandle,&canMsg,100);
    if(ret==0)
    {
   //TBOX_PRINT("Can Test2 receive can data...\r\n");
   canData[1]++;
    memcpy(canData,&canMsg.canData,64);
            //canData[24] = canMsg.canData[24];
   m_testCount++;
   ret = CanDriverHalTransmit(canHandle,canId,canData,32);
   TBOX_PRINT("Can Test2 transmit can data...\r\n");
      i++;
    }
    else
    {
      //RTOS_HalApiWait(10);
    }

  }  
}
#endif

// void BatteryTest(void)
// {
// 	PeripheralDriverHal_Enable_battery_charge(1);
// 	PeripheralDriverHal_Enable_battery_out(1);
// }

// uint8_t Test1TaskUartBuffer[100];
// uint8_t Test2TaskUartBuffer[100];
// uint8_t Test1Data[50];
// uint8_t Test2Data[50];

// uint32_t m_testUartRx = 0;



// void UartTest1(void)
// {
//   typeUartHandLe uartHandle;
// //  uint8_t i;
// 	stru_uartFilterConfig filter;
	
//   uartHandle = UartDriverHalOpen(2);
//   UartDriverHalConfigureBuffer(uartHandle,Test1TaskUartBuffer,sizeof(Test1TaskUartBuffer));
// 	filter.Aid = 0x01;
// 	filter.MinMid = 0x00;
// 	filter.MaxMid = 0xFF;
// 	UartDriverHalConfigureFilterByProtocal(uartHandle,&filter,1);
//   Test1Data[0] = 0x01;
// 	Test1Data[1] = 0x02;
//   Test1Data[2] = 0x03;
// 	Test1Data[3] = 0x04;	
//   Test1Data[4] = 0x05;
// 	Test1Data[5] = 0x06;	
//   Test1Data[6] = 0x07;
// 	Test1Data[7] = 0x08;
//   Test1Data[8] = 0x09;
// 	Test1Data[9] = 0x10;
//   Test1Data[10] = 0x11;
// 	Test1Data[11] = 0x12;
//   Test1Data[12] = 0x13;
// 	Test1Data[13] = 0x14;
// 	Test1Data[14] = 0x15;
// 	Test1Data[15] = 0x16;
	
//   while(1)   
//  {
// 	 uint16_t rxLength = 0;
// 	 UartDriverHalReceiveWait(uartHandle,Test2Data,&rxLength,1000000);
// 	 if(rxLength>0)
// 	 {
// 		 m_testUartRx ++;
// 			UartDriverHalTransmit(uartHandle,Test2Data,rxLength);
// 	 }	 		 
//    RTOS_HalApiWait(100);   
//  } 
// }

//static uint8_t testData[20];
void TaskTest(void *pvParameters )
{
    while(1)   
    {
      // CanHalTestMain();
      PeripheralHalTestMain();
      // CryptoHalTestMain();
      // EcallHalTestMain();
      // vTaskDelay(3000);
      // PowerManageHalTestMain();
      FlashHalTestMain();
      // vTaskDelay(3000);
      // TBOX_PRINT("task test running\r\n");
      Qmi8a01zHalTestMain();  // Add QMI8A01z sensor test
      vTaskDelay(1000);
    }
}

// void Test2Task(void *pvParameters )
// {
//   //uint32_t i;
// 	uint32_t tickCount;
// 	uint32_t idleCount;
//     //UartTest2();
// 	CanTest2();
//     //mutexTest2();
// 	while(1)
// 	{
// 	   tickCount = RTOS_HalGetTickCount();
// 	   idleCount = GetIdleCount();
// 	   TBOX_PRINT("idle count is %d \r\n",idleCount);
// 	   RTOS_HalApiWait(1000);
// 	  }
// }




// typedef enum
// {
//     SyncState_Init,
//     SyncState_Idle,
//     SyncState_IoSyncTransmit,
//     syncState_Sleep,

// } enum_SyncState;

// //��������ָ��
// typedef enum
// {
//     BLE_Config_BRD = 1,
//     BLE_Config_BRD_MSG,//�㲥����
//     BLE_Config_Wakeup,
//     BLE_Config_ON_OFF,
//     BLE_Config_RESERVE,
// } enum_BLE_CMD;
// /***********************��ʱ��**************************************/
// #define PERIPHERAL_TIMER_INSTANCE_NUMBER    3
// typedef int16_t typeTimerHandle;
// typeTimerHandle SE_TimerHandle;//seר�ö�ʱ��


// enum_SyncState m_syncState = SyncState_Init;

// void CpuStartCycleProcess(void);
// void McuCpuTimeSyncCycleProcess(uint8_t *pRxData, uint16_t rxLength);
// void McuCpuTimeSyncInitialize(typeUartHandLe uartHandle);

// //�ų��ǽӿ�ʵ��
// int16_t Iwall_GetSystemTime(struct tm *systime);
// uint32_t Iwall_MpuDataReceive(uint8_t *data,  uint32_t *datalen);
// uint32_t Iwall_MpuDataSend(uint8_t *data, uint32_t *datalen);
// uint32_t Iwall_BleDataReceive(uint8_t *data, uint32_t *datalen);
// uint32_t Iwall_BleDataSend(uint8_t *data, uint16_t datalen);
// uint32_t Iwall_BleConfig(uint8_t cmd, uint8_t *data, uint32_t datalen);
// uint32_t Iwall_BleGetRssi(uint8_t cnt, uint8_t *rssi);
// char     Iwall_BleGetRssi_fresh(uint8_t cnt);
// char     BLE_SendCmdToBle(enum_BLE_CMD ble_cmd,uint8_t *data,uint32_t datalen);
// char     BLE_ReceiveCmdFromBle(enum_BLE_CMD ble_cmd,uint8_t *data);
// char BLE_SetIbeacon(uint8_t *data,uint32_t datalen);
// char  BLE_GetIbeacon(uint8_t *data);
// #ifdef VE21_HW1_2
// #else
// char  Iwall_BleSetRssi_fresh(uint8_t cnt);
// #endif
// int16_t ReadDataFromFlash(uint32_t address,uint8_t *data,uint32_t dataLength);
// int16_t WriteDataToFlash(uint32_t address,const uint8_t *data,uint32_t dataLength);
// uint32_t BLE_separate_rssi(uint8_t *sr,uint8_t *rt,uint16_t datalen,uint8_t *des);
// void SPI_Init_t(void);

// //uint8_t Test1TaskUartBuffer[100];
// //uint8_t Test2TaskUartBuffer[100];
// static uint8_t m_uartRxData[64];


// extern uint16_t BLE_RxBuffer_RecCount;
// extern enum_BLE_CMD BLE_cmd_Send;


// uint8_t BLE_txBuff[BLE_SEND_MAX] = {0};//��������������ÿ�����1024
// uint8_t BLE_rxBuff[BLE_MAX_NUM_REC] = {0};//������������
// uint32_t   ble_count_rtos = 0;
// uint8_t  ret_count = 0;//�����������Դ˴�

// void BLE_Task(void* parmeter)
// {

	
// //	SE_TimerHandle = PeripheralDriverHal_TimerOpen();//��ʱ����
//  //   PeripheralDriverHal_TimerStart(SE_TimerHandle,10*1000);//10 s ��ʱ������	
   
//  //   uartHandle = UartDriverHalOpen(1);

	
// //	Iwall_Ble_Init();//�����������ó�ʼ��
 
// //	se_connect_test();//SE���Ӻ���
// //    se_version_test();//SE�汾��ȡ����

// ////////////	while(getbleisstart()==false)
// ////////////	{
// ////////////		RTOS_HalApiWait(1);
// ////////////	}
//     while (1)
//     {
// #ifdef VE21_HW1_2
// 	//	Iwall_BleConect_Test();
	
		
// 		BleTaskCycleProcess();
// //		RTOS_HalApiWait(2);
// //		AppEventProcess();
// 		RTOS_HalApiWait(5);
	
// 		__NOP();
// 	//	RTOS_HalApiWait(1);//��������1ms
// 	//	BleDataParseProcess();
// 	//	ret = BleDataReceive(BLE_RxBuffer, &BLE_rxlenght);//������������()
// #else
		
// #endif
//     }
// }

// static uint8_t u8TimeSyncMidCnt = 0;
// static uint8_t m_uartDriverRxBuffer[64];


// void McuCpuSync_Task(void *pvParameters)
// {
//     typeUartHandLe uartHandle;
//     stru_uartFilterConfig uartFilter;
//     uint16_t rxLength;
//     uint8_t count;
//     uint32_t mcu_rxLength;//����cpu���ݳ���
//     uint32_t mcu_txLength;//�������ݳ���
//     uint8_t mcu_rxData[50];//����cpu����
//     uint8_t mcu_txData[50];//�������ݸ�cpu

//     uartHandle = UartDriverHalOpen(GetMainCpuUartChannel());  //UART_CHANNEL_CPU_MAIN
//     if (uartHandle < 0) //invalid handle
//     {
//         for (;;)
//         {
//             RTOS_HalApiWait(10);
//         }
//     }
//     uartFilter.Aid = 0x01;
//     uartFilter.MinMid = 0x02;
//     uartFilter.MaxMid = 0x08;
//     UartDriverHalConfigureFilterByProtocal(uartHandle, &uartFilter, 1);
//     UartDriverHalConfigureBuffer(uartHandle, m_uartDriverRxBuffer, sizeof(m_uartDriverRxBuffer));
//     McuCpuTimeSyncInitialize(uartHandle);
//     while (1)
//     {
//         uint16_t timeSyncRxLength = 0x00;
// 		if(count)
//         {
//            count = 0;
// //////////////           CpuStartCycleProcess();  //CPU �������� 
//         }
//         else
//         {
//            count = 1;
//         }
//         while (1)
//         {
//             rxLength = 0;
//             UartDriverHalReceiveWait(uartHandle, m_uartRxData, &rxLength, 0);
//             if (rxLength == 0x00)
//             {
//                 break;
//             }
//             if (m_uartRxData[3] == 0x02) //time sync
//             {
//                 u8TimeSyncMidCnt += 1;
//                 timeSyncRxLength = rxLength;
//                 McuCpuTimeSyncCycleProcess(m_uartRxData, timeSyncRxLength);
//             }
//         }
// //////////////////		SendMcuMsgToCpu(uartHandle,0x01,0x02,01,mcu_txData,0);
// 	#if 0
//         //��ȡϵͳʱ��ӿ�,���
//         Iwall_GetSystemTime(&mcu_tm);
//         //����cpu����(����������)mcu_txLength
//         Iwall_MpuDataReceive(mcu_rxData,&mcu_rxLength);
//         if(mcu_rxLength!=0)
//         {
//             mcu_rxData[0] = 0x00;
//         }
// 		
//         //��cpu��������
// 		SendMcuMsgToCpu(0x01,0x02,mcu_txData,mcu_txLength);//
//         if (timeSyncRxLength == 0x00)
//         {
//             McuCpuTimeSyncCycleProcess(m_uartRxData, timeSyncRxLength);
//         }
// 	#endif
//         RTOS_HalApiWait(10);

//     }
// }

/*************************************************
  Function:       Qmi8a01zHalTestMain
  Description:    Test QMI8A01z sensor functionality
  Input:          none
  Output:         none
  Return:         none
  Others:         Tests sensor initialization and data reading
*************************************************/
#if (GSENSOR_TYPE_SELECTED == GSENSOR_TYPE_QMI8A01Z)
void Qmi8a01zHalTestMain(void)
{
    static uint16_t testCounter = 0;
    Qmi8a01zHalData_t sensorData;
    int16_t result;
    
    /* Initialize sensor if not done yet */
    if (testCounter == 0) {
        TBOX_PRINT("Initializing QMI8A01z sensor...\r\n");
        result = Qmi8a01zHalInit(0);  // No interrupt wake-up
        if (result == QMI8A01Z_OK) {
            TBOX_PRINT("QMI8A01z: Initialization successful\r\n");
        } else {
            TBOX_PRINT("QMI8A01z: Initialization failed with code %d\r\n", result);
            testCounter = 0;
            return;
        }
    }
    
    /* Read sensor data */
    result = Qmi8a01zHalGetValue(&sensorData);
    if (result == QMI8A01Z_OK) {
        testCounter++;
        if (testCounter % 10 == 0) {  // Print every 10th reading
            TBOX_PRINT("QMI8A01z Data #%d: X=%d mg, Y=%d mg, Z=%d mg\r\n", 
                       testCounter, sensorData.xAcceleratedData, 
                       sensorData.yAcceleratedData, sensorData.zAcceleratedData);
        }
    } else {
        TBOX_PRINT("QMI8A01z: Failed to read sensor data, error=%d\r\n", result);
    }
    
    /* Test wake-up flag every 50 cycles */
    if (testCounter % 50 == 0) {
        int16_t wakeupFlag = Qmi8a01zHalGetWakeupFlag();
        TBOX_PRINT("QMI8A01z: Wake-up flag = %d\r\n", wakeupFlag);
    }
    
    /* Test threshold setting every 100 cycles */
    if (testCounter % 100 == 0) {
        result = Qmi8a01zHalSetThreshold(32);  // Test threshold value
        TBOX_PRINT("QMI8A01z: Setting motion detection threshold, result=%d\r\n", result);
    }
}
#endif

/* Fallback function for LSM6DSM testing if needed */
#if (GSENSOR_TYPE_SELECTED == GSENSOR_TYPE_LSM6DSM)
void Qmi8a01zHalTestMain(void)
{
    TBOX_PRINT("QMI8A01z test function called but LSM6DSM is selected\r\n");
}
#endif

