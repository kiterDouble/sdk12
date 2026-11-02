#include <stdint.h>
#include <glib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "audio_capture.h"
#include "audio_common.h"
#include "audio_playback.h"
#include "audio_control.h"
#include "audio_file_upload.h"
#include "sr_log.h"
#include "err_code.h"
#include "audio_aikit.h"
#include "audio_http.h"
#include "common_functions.h"
#include "sr_ipc.h"


void AudioRecvMcuEvetDataHandleCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    static uint8_t vehicleStatus = 3;

    if ((NULL == recvData) && (0 == dataLen) && (NULL == userdata))
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    
    LOG_DebugHex(recvData, dataLen, "MCU Data");

    uint32_t aidMid = (uint32_t)((long long)userdata & 0x00ffffff);
    LOG_Debug("RecvMcuEvetDataHandleCallback aidMid:%08x", (unsigned int)aidMid);

    switch (aidMid)
    {
        case SYS_QUE_TYPE_MCU_VEHICLE_EVENT_DATA:
            // 暂不处理事件类数据
            break;

        case SYS_QUE_TYPE_MCU_VEHICLE_CYCLE_DATA:
            if (dataLen < 3)
            {
                LOG_Error("dataLen:%d > sizeof(VehicleCycleData)", dataLen);
                break;
            }

            if(vehicleStatus != recvData[2] && 0 == recvData[2])
            {
                vehicleStatus = recvData[2];
                AudioSendVehicleStartTTS();
            }
            else if(vehicleStatus != recvData[2] && 3 == recvData[2])
            {
                vehicleStatus = recvData[2];
            }
            
            break;
        default:
            break;
    }
}

int SrGetInterfaceIp(const char *ifName, char *ipBuf, size_t bufLen) 
{
    struct ifreq ifr;
    int sockfd;
    int ret = -1;

    if(NULL == ifName || strlen(ifName) > IFNAMSIZ)
    {
        LOG_Error("%s argv error", __FUNCTION__);
        return RET_ERR;
    }
    // 创建 UDP 套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
    {
        LOG_Error("create socket error");
        return RET_ERR;
    }

    // 设置网卡名称
    strncpy(ifr.ifr_name, ifName, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    // 获取 IP 地址
    if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) 
    {
        LOG_Error("ioctl(SIOCGIFADDR) on %s failed: %s\n", ifName, strerror(errno));
        close(sockfd);
        return RET_ERR;
    }

    // 将 IP 地址转换为字符串
    struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
    if (inet_ntop(AF_INET, &sin->sin_addr, ipBuf, bufLen) == NULL) 
    {
        LOG_Error("inet_ntop error");
        close(sockfd);
        return RET_ERR;
    }

    close(sockfd);
    return RET_OK;
}

int32_t main(void)
{
    int32_t retValue = 0;
    int32_t buffSize = 0;
    int32_t captureBufferSize = 0;
    int32_t playbackBufferSize = 0;
    char audioBuff[1024] = {0};
    char ipaddrBuff[16] = {0};
    pthread_t controlTask;
    pthread_t controlTtsTask;
    pthread_t controlNotTtsTask;
    pthread_t recvPlaybackTask;
    pthread_t playbackTask;
    pthread_t captureTask;
    pthread_t captureSendTask;
    pthread_t fileUploadTask;
    pthread_t audioHttpTask;

    LOG_INIT("audioApp");

    InitJsonFileHandler();
    // GetConfigJsonKeyValue(audioBuff, 1024);
    // printf("%s\n", audioBuff);
    // memset(audioBuff, 0, 1024);
    // GetTTSJsonKeyValue(audioBuff, 1024);
    // printf("%s\n", audioBuff);
    // memset(audioBuff, 0, 1024);
    // GetStatusJsonKeyValue(audioBuff, 1024);
    // printf("%s\n", audioBuff);
    while(1)
    {
        if(0 == access("/oemdata/tts/audio_switch", F_OK))
        {
            // SrGetSystemRetValue("ip link add name bridge3 type bridge");
            // sleep(1);
            SrGetSystemRetValue("ifconfig eth0:2 192.168.1.8 netmask 255.255.255.0 up");
            SrGetSystemRetValue("mkdir -p /oemdata/ftp/");
            //system("tcpsvd -vE 0.0.0.0 20010 ftpd -w /oemdata/ftp &");
        }

        retValue = SrGetInterfaceIp("eth0:2", ipaddrBuff, 16);
        if(0 == retValue && 0 == strncmp((char *)ipaddrBuff, "192.168.1.8", strlen("192.168.1.8")))
        {
            break;
        }
        sleep(1);
    }

    AudioCreateThread(&audioHttpTask, AudioHttpThread, NULL);

    while(1)
    {
        if(RET_OK == AudioBackupLicenseFile(NULL))
        {
            break;
        }
        sleep(1);
    }

    // SrGetSystemRetValue("ql_cmd at at+qnwprefcfg=\\\"mode_pref\\\",AUTO");
    SrGetSystemRetValue("echo  79 > /sys/class/gpio/export");
    SrGetSystemRetValue("echo  out > /sys/class/gpio/gpio79/direction");
    SrGetSystemRetValue("echo  1 > /sys/class/gpio/gpio79/value");
    AudioAikitSdkInt();
    AudioApiInit();
    StartQueueMsgRcvThread("audio_signal_handle"); //创建消息接收线程；

    SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_EVENT_DATA, AudioRecvMcuEvetDataHandleCallback, (void *)(SYS_QUE_TYPE_MCU_VEHICLE_CYCLE_DATA));
    SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CYCLE_DATA, AudioRecvMcuEvetDataHandleCallback, (void *)(SYS_QUE_TYPE_MCU_VEHICLE_CYCLE_DATA));

    AudioCreateThread(&controlTask, AudioControlThread, NULL);
    AudioCreateThread(&controlTtsTask, AudioControlHandleThread, NULL);
    AudioCreateThread(&controlNotTtsTask, AudioControlNotTtsThread, NULL);
    AudioCreateThread(&recvPlaybackTask, AudioRecvPlayBackDataThread, NULL);
    AudioCreateThread(&playbackTask, AudioPlayBackThread, NULL);
    AudioCreateThread(&captureTask, AudioApiCaptureThread, NULL);
    AudioCreateThread(&captureSendTask, AudioApiCaptureDataSendThread, NULL);
    AudioCreateThread(&fileUploadTask, AudioFileUploadThread, NULL);

    
    // testTtts(100, 10);
    // testTtts(50, 80);

    // AudioApiPlaybackOpen();

    // AudioApiPlaybackStreamPrepare(1, 2, 16000);

    // AudioApiPlaybackPlay();

    // playbackBufferSize = AudioApiPlaybackGetBufferSize();

    // AudioApiCaptureOpen();

    // AudioApiCaptureStreamPrepare(1, 2, 16000);

    // AudioApiCaptureRecord();

    // captureBufferSize = AudioApiCaptureGetBufferSize();

    // buffSize = ((playbackBufferSize > captureBufferSize) ? captureBufferSize : playbackBufferSize);

    // LOG_Debug("buffSize :%d, playbackBufferSize: %d, captureBufferSize:%d", buffSize, playbackBufferSize, captureBufferSize);
    // sleep(5);

    // while(1)
    // {
    //     // sleep(5);
    //     AudioApiCapturePullStream(audioBuff, buffSize);
    //     AudioApiPlaybackPushStream(audioBuff, buffSize);
    // }
    g_main_loop_run(g_main_loop_new(NULL, FALSE));
    
    // AudioApiCaptureStop();
    // AudioApiPlaybackStop();
    // AudioApiCaptureClose();
    // AudioApiPlaybackClose();
    AudioApiDeinit();

    return 0;
}
