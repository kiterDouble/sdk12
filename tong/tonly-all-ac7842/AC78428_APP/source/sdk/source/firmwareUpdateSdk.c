#include "FreeRTOS.h"
#include "task.h"
#include "stdlib.h"
#include "crc8_16_32.h"
#include "mpuHal.h"
#include "flashHal.h"
#include "peripheralHal.h"
#include "logHal.h"
#include "powerManageSdk.h"
#include "firmwareUpdateSdk.h"

typedef enum
{
    E_FlashState_Idle,
    E_FlashState_FlashIn,
    E_FlashState_FlashUnlock,
    E_FlashState_FlashErease,
    E_FlashState_FlashData,
    E_FlashState_FlashFinish,
    E_FlashState_Reset,
    E_FlashState_Error,
}FlashState_e;

//static uint8_t g_appFlag = 0x00;                      //正在运行app状态保存
static int16_t g_mpuHandle = -1;                      //MPU通信句柄
static uint8_t g_recvDataBuffer[600];                  //cpu通信接口设置数据接收缓存buf；
//static uint8_t g_fotaSuccessFlag = 0;                 //升级成功标记
//static FlashState_e g_flashState = E_FlashState_Idle;  //升级过程中flash操作状态标记
//static uint16_t g_cycleTime = 10;                     //周期接口被调用的周期 ，单位ms
static uint8_t g_respDataBuffer[64] = {0};            //响应数据的数据缓存buffer
static MpuHalDataPack_t  g_responsePack;              //响应数据结构体；
static uint8_t g_versionNumber[20] = {'v','0','0','1','.','0','0','2',};             //软件版本号缓存buf
static uint16_t g_versionLenth = 11;                   //软件版本号长度
//static uint8_t g_fotaModeFlag = 0;                    //升级模式标记 0：非升级状态 1：升级状态
//static uint16_t g_fotaModeTimeCount = 0;              //升级过程时间计数 
//static Crc32Objec_t g_crc32Object;
//static uint32_t g_crcData = 0xFFFFFFFF;
static FlashHalMetaDataInfo_t s_MetaDataInfo;
/*************************************************
  Function:       FirmwareUpdateSdkVersionResponse
  Description:    发送读取版本响应
  Input:          dataPack：请求数据
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
static int16_t FirmwareUpdateSdkVersionResponse(MpuHalDataPack_t *dataPack)
{
    uint8_t i = 0;
    uint8_t lenth= 0;
    
    //校验传入参数
    if(dataPack->dataLength == 0)
    {
        return -1;
    }
    g_responsePack.aid = 0x03;
    g_responsePack.mid = 0x01;
    g_responsePack.subcommand = 0x00;
    
    //设置指令counter
    g_respDataBuffer[0] = dataPack->pDataBuffer[0];
    g_respDataBuffer[1] = dataPack->pDataBuffer[1];
    
    //判断版本号是否有效
    if(g_versionLenth > 0)
    {
        //设置响应状态成功
        g_respDataBuffer[2] = 0x00;
        for(i = 0;i<g_versionLenth;i++)
        {
            g_respDataBuffer[3+i] = g_versionNumber[i];
        }
        lenth = 3 + g_versionLenth;
    }
    else
    {
        //设置响应状态失败
        g_respDataBuffer[2] = 0x01;
        lenth = 3;
    }
    
    g_responsePack.pDataBuffer = g_respDataBuffer;
    g_responsePack.dataLength = lenth;
    g_responsePack.dataBufferSize = sizeof(g_respDataBuffer);
    
    MpuHalTransmit(g_mpuHandle,&g_responsePack, MPU_HAL_UART_MODE);
    
    return 0;
}

/*************************************************
  Function:       FirmwareUpdateSdkSeedResponse
  Description:    发送seed响应
  Input:          dataPack：请求数据
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
static int16_t FirmwareUpdateSdkSeedResponse(MpuHalDataPack_t *dataPack)
{
    uint8_t lenth= 0;
    
    //校验传入参数
    if(dataPack->dataLength == 0)
    {
        return -1;
    }
    g_responsePack.aid=dataPack->aid;
    g_responsePack.mid=dataPack->mid;
    g_responsePack.subcommand=0x00;
    
    //设置指令counter
    g_respDataBuffer[0] = dataPack->pDataBuffer[0];
    g_respDataBuffer[1] = dataPack->pDataBuffer[1];
    
    //设置响应状态成功
    g_respDataBuffer[2] = 0x00;
    g_respDataBuffer[3]=0x01;
    g_respDataBuffer[4]=0x02;
    g_respDataBuffer[5]=0x03;
    g_respDataBuffer[6]=0x04;
    
    lenth = 3 + 4;
    g_responsePack.pDataBuffer = g_respDataBuffer;
    g_responsePack.dataLength=lenth;
    g_responsePack.dataBufferSize = sizeof(g_respDataBuffer);
    
    MpuHalTransmit(g_mpuHandle,&g_responsePack, MPU_HAL_UART_MODE);
    
    return 0;
}

/*************************************************
  Function:       FirmwareUpdateSdkInit
  Description:    执行执行结果响应
  Input:          dataPack：请求数据
                  result：执行结果 0：成功 1：失败
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
static int16_t FirmwareUpdateSdkResultResponse(MpuHalDataPack_t *dataPack,uint8_t result)
{
    uint8_t lenth= 0;
    
    //校验传入参数
    if(dataPack->dataLength == 0)
    {
        return -1;
    }
    
    g_responsePack.aid=dataPack->aid;
    g_responsePack.mid=dataPack->mid;
    g_responsePack.subcommand=0x00;
    
        //设置指令counter
    g_respDataBuffer[0] = dataPack->pDataBuffer[0];
    g_respDataBuffer[1] = dataPack->pDataBuffer[1];

     //设置响应状态成功
    g_respDataBuffer[2] = 0x00;
    lenth = 3;
    g_responsePack.pDataBuffer = g_respDataBuffer;
    g_responsePack.dataLength=lenth;
    g_responsePack.dataBufferSize = sizeof(g_respDataBuffer);
    
    MpuHalTransmit(g_mpuHandle,&g_responsePack, MPU_HAL_UART_MODE);
    
    return 0;
}

/*************************************************
  Function:       FirmwareUpdateSdkLookRuningResponse
  Description:    发送获取当前运行app指令的响应
  Input:          mpuHandle:mpu的通信句柄
                  cycleTime：周期接口调用周期设置
  Output:         无
  Return:         无
  Others:         
*************************************************/
static int16_t FirmwareUpdateSdkLookRuningResponse(MpuHalDataPack_t *dataPack,uint8_t flag)
{
    uint8_t lenth= 0;
    if(dataPack == NULL)
    {
    	return -1;
    }
    
    g_responsePack.aid=dataPack->aid;
    g_responsePack.mid=dataPack->mid;
    g_responsePack.subcommand=0x00;

    //设置指令counter
    g_respDataBuffer[0] = dataPack->pDataBuffer[0];
    g_respDataBuffer[1] = dataPack->pDataBuffer[1];

    //设置响应状态成功
    g_respDataBuffer[2] = 0; 
    //设置查询结果
    g_respDataBuffer[3] = flag;
    lenth = 4;

    g_responsePack.pDataBuffer = g_respDataBuffer;
    g_responsePack.dataLength = lenth;
    g_responsePack.dataBufferSize = sizeof(g_respDataBuffer);

    MpuHalTransmit(g_mpuHandle,&g_responsePack, MPU_HAL_UART_MODE);

    return 0;
}

/*************************************************
  Function:       FirmwareUpdateSdkSetVersion
  Description:    设置当前版本号
  Input:          version：版本号
                  lenth：长度
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t FirmwareUpdateSdkSetVersion(uint8_t * version,uint16_t lenth)
{
    uint8_t i = 0;

    if(version == NULL || lenth == 0 || lenth > 20)
    {
        return -1;
    }
    for(i = 0;i<lenth;i++)
    {
        g_versionNumber[i] = version[i];
    }
    g_versionLenth = lenth;

    return 0;
}

/*************************************************
  Function:       FirmwareUpdateSdkInit
  Description:    升级模块初始化接口
  Input:          mpuHandle:mpu的通信句柄
                  cycleTime：周期接口调用周期设置，单位ms
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t FirmwareUpdateSdkInit(uint16_t cycleTime)
{
    MpuHalFilter_t filter;

    filter.aid = 0x03;
    filter.midMin = 0x01;
    filter.midMax = 0xFF;

    if(cycleTime == 0)
    {
        return -1;
    }

//    g_cycleTime = cycleTime;

    g_mpuHandle = MpuHalOpen();
    if(g_mpuHandle < 0)
    {
        return -1;
    }

    MpuHalSetRxFilter(g_mpuHandle,&filter);
    MpuHalSetRxBuffer(g_mpuHandle,g_recvDataBuffer,sizeof(g_recvDataBuffer));
    return 0;
}




/*************************************************
  Function:       FirmwareUpdateSdkCycleProcess
  Description:    升级模块周期调用接口
  Input:          mpuHandle:mpu的通信句柄
                  cycleTime：周期接口调用周期设置
  Output:         无
  Return:         无
  Others:         
*************************************************/
void FirmwareUpdateSdkCycleProcess(void)
{
    int16_t ret = -1;
    uint8_t mpuData[600] = {0};
    MpuHalDataPack_t  dataPack;
    uint8_t bank = 2;
    
    dataPack.pDataBuffer = mpuData;
    dataPack.dataBufferSize = sizeof(mpuData);
    //接收数据
    ret = MpuHalReceive(g_mpuHandle,&dataPack,0);
    if(ret != MPU_HAL_STATUS_OK)    //接收到数据？
    {
        return;
    }

    if(dataPack.mid == 0x01)        //读取版本号
    {
        TBOX_PRINT("--------------------01 APP Read Version------------------\r\n");
        FirmwareUpdateSdkVersionResponse(&dataPack);
    }
    else if(dataPack.mid == 0x02)  //获取seed指令
    {
        
        TBOX_PRINT("--------------------02 APP Get Seed------------------\r\n");
        FirmwareUpdateSdkSeedResponse(&dataPack);
    }
    else if(dataPack.mid == 0x03)  //解锁flash下载功能
    {
        TBOX_PRINT("--------------------03 APP Unlock Flash Download %d,%d,%d,%d------------------\r\n",dataPack.pDataBuffer[2],dataPack.pDataBuffer[3],dataPack.pDataBuffer[4],dataPack.pDataBuffer[5]);
        //对数据进行校验
        if(dataPack.pDataBuffer[2]==0x01 && dataPack.pDataBuffer[3] == 0x02 && dataPack.pDataBuffer[4]==0x03 && dataPack.pDataBuffer[5]==0x04)
        {
            FirmwareUpdateSdkResultResponse(&dataPack,0);
        }
        else
        {
            FirmwareUpdateSdkResultResponse(&dataPack,1);
        }
    }
    else if(dataPack.mid == 0x04)  //进入下载状态
    {
        TBOX_PRINT("--------------------04 APP Enter Download State %d,%d,%d,%d------------------\r\n",dataPack.pDataBuffer[2],dataPack.pDataBuffer[3],dataPack.pDataBuffer[4],dataPack.pDataBuffer[5]);
        //对数据进行校验
        if(dataPack.pDataBuffer[2] == 0x01 && dataPack.pDataBuffer[3]==0x02 &&
                   dataPack.pDataBuffer[4]==0x03 && dataPack.pDataBuffer[5]==0x04)
        {
            FirmwareUpdateSdkResultResponse(&dataPack,0);
            FlashHalGetMetaDataInfo(&s_MetaDataInfo);
            s_MetaDataInfo.m_upgradeFlag = FLASH_UPGRADE_PENDING;
            FlashHalWriteMetaDataInfo(&s_MetaDataInfo);
            vTaskDelay( pdMS_TO_TICKS((100)));
            TBOX_PRINT("Ready reset to boot loader\r\n");
            PeripheralHalMcuHardReset();
        }
        else
        {
            FirmwareUpdateSdkResultResponse(&dataPack,1);
        }
        //设置唤醒延时，阻止休眠
        PowerManageSdkSetWakeDelay(60*1000);
        PowerManageSdkResetWake(0);
        //g_flashState = E_FlashState_FlashIn;
    }
    else if(dataPack.mid == 0x08)  //复位指令
    {
        TBOX_PRINT("--------------------08 APP Reset system------------------\r\n");
        FirmwareUpdateSdkResultResponse(&dataPack,0);
        //延时100ms
        vTaskDelay( pdMS_TO_TICKS((100)));
        PeripheralHalMcuHardReset();
    }
    else if(dataPack.mid == 0x09)  //获取app运行状态
    {
        //获取当前运行分区
        bank = FlashHalOtaGetActiveBank();
        TBOX_PRINT("--------------------09 APP Get run bank %d------------------\r\n",bank);
        FirmwareUpdateSdkLookRuningResponse(&dataPack,bank);
    }

    return;

}




