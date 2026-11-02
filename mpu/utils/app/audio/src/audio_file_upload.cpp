#include <stdint.h>
#include "audio_common.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define  AUDIO_FILE_UPLOAD_PORT     (7991)
#define  AUDIO_PLAYLOAD_OFFSET      (12)

int32_t AudioCheckUdpProtocol(uint32_t *index, uint16_t *session,const char *udpData, int32_t datLen)
{
    int32_t payloadLen = 0;
    int32_t count = 0;
    uint8_t xorValue = 0;
    uint32_t sequence = 0;
    uint16_t tmpSession = 0;

    if(NULL == udpData || NULL == session)
    {
        LOG_Error("%s argv is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(datLen < 12)
    {
        LOG_Error("%s datLen < 12 ", __FUNCTION__);
        return RET_ERR;
    }

    if(udpData[0] != 0xFA && udpData[1] != 0xFB)
    {
        LOG_Error("%s heard not FA FB ", __FUNCTION__);
        return RET_ERR;
    }

    if(udpData[2] != 0x01)
    {
        LOG_Error("%s version not 01 ", __FUNCTION__);
        return RET_ERR;
    }

    if(udpData[3] != 0x01)
    {
        LOG_Error("%s Type not 01 ", __FUNCTION__);
        return RET_ERR;
    }

    sequence  = (udpData[7] << 24);
    sequence |= (udpData[6] << 16);
    sequence |= (udpData[5] << 8);
    sequence |= (udpData[4] << 0);

    if(sequence != *index)
    {
        LOG_Error("%s index error %d, %d", __FUNCTION__, *index, udpData[4]);
        if(udpData[4] == 0){
            *index = 0;
        }else{
            return RET_ERR;
        }
    }

    tmpSession = (udpData[9] << 8);
    tmpSession |= (udpData[8] << 0);

    payloadLen = (udpData[11] << 8) + udpData[10];
    if(datLen != (payloadLen + 14))
    {
        LOG_Error("%s payloadLen error:%d", __FUNCTION__, payloadLen);
        return RET_ERR;
    }

    for(count = 2; count < datLen - 2; count++)
    {
        xorValue ^= udpData[count];
    }

    if(xorValue != udpData[count])
    {
        LOG_Error("%s xor error:%d:%d", __FUNCTION__, xorValue, udpData[count]);
        return RET_ERR;
    }

    if(udpData[datLen - 1] != 0xED)
    {
        LOG_Error("%s Tail not ED ", __FUNCTION__);
        return RET_ERR;
    }

    *session = tmpSession;
    return payloadLen;
}


uint32_t AudioAdler32(uint8_t *data, size_t len, uint8_t flag) 
{
    static int32_t a = 1, b = 0;
    uint32_t count;
    
    if(0 == flag)
    {
        a = 1;
        b = 0;
    }
    
    if(NULL == data)
    {
        return (b << 16) | a;
    }

    for (count = 0; count < len; count++) 
    {
        a = (a + data[count]) % 65521;
        b = (b + a)     % 65521;
    }

    return (b << 16) | a;
}

int32_t AudioGetFileName(char *fileName, uint32_t buffLen, const char *udpData, int32_t datLen)
{
    uint16_t fileNum = 0;
    uint16_t fileCodec = 0;

    if(NULL == fileName)
    {
        LOG_Error("%s fileName is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == udpData)
    {
        LOG_Error("%s udpData is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(buffLen < 64)
    {
        LOG_Error("%s buffLen < 64 ", __FUNCTION__);
        return RET_ERR;
    }

    if(datLen < 32)
    {
        LOG_Error("%s datLen < 32 ", __FUNCTION__);
        return RET_ERR;
    }

    fileNum = (udpData[13] << 8);
    fileNum |= (udpData[12] << 0);

    fileCodec = (udpData[15] << 8);
    fileCodec |= (udpData[14] << 0);

    LOG_Debug("%s %02X fileNum:%d fileCodec:%d", __FUNCTION__,udpData[12],fileNum,fileCodec);

    switch(fileCodec)
    {
        case 1:
            snprintf(fileName, buffLen, "/oemdata/tts/%03u.mp3", fileNum);
            break;
        case 2:
            snprintf(fileName, buffLen, "/oemdata/tts/%03u.wav", fileNum);
            break;
        case 3:
            snprintf(fileName, buffLen, "/oemdata/tts/%03u.pcm", fileNum);
            break;
    }

    return strlen(fileName);
}

int32_t AudioGetFileLen(uint32_t *fileLen, const char *udpData, int32_t datLen)
{
    if(NULL == fileLen)
    {
        LOG_Error("%s fileLen is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == udpData)
    {
        LOG_Error("%s udpData is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(datLen < 32)
    {
        LOG_Error("%s datLen < 32 ", __FUNCTION__);
        return RET_ERR;
    }

    // 修正：按照小端序正确解析文件长度
    *fileLen = (uint32_t)udpData[16] << 0;
    *fileLen |= (uint32_t)udpData[17] << 8;
    *fileLen |= (uint32_t)udpData[18] << 16;
    *fileLen |= (uint32_t)udpData[19] << 24;
    
    LOG_Debug("%s FileLen bytes: [%02X %02X %02X %02X], value: %u", 
              __FUNCTION__, udpData[16], udpData[17], udpData[18], udpData[19], *fileLen);
              
    return *fileLen;
}

int32_t AudioGetFileContent(char *fileContent, uint32_t contentLen, const char *udpData, int32_t datLen)
{
    int32_t payloadLen = 0;

    if(NULL == fileContent)
    {
        LOG_Error("%s fileLen is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == udpData)
    {
        LOG_Error("%s udpData is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(datLen < 14)
    {
        LOG_Error("%s datLen < 32 ", __FUNCTION__);
        return RET_ERR;
    }

    payloadLen = (udpData[11] << 8) + udpData[10];
    if(payloadLen > contentLen)
    {
        LOG_Error("%s payloadLen > contentLen ", __FUNCTION__);
        return RET_ERR;
    }
    memset(fileContent, 0, contentLen);
    memcpy(fileContent, &udpData[12], payloadLen);

    return payloadLen;
}


int32_t AudioGetFileAdler32(uint32_t *adler32, const char *udpData, int32_t datLen)
{
    if(NULL == adler32)
    {
        LOG_Error("%s adler32 is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == udpData)
    {
        LOG_Error("%s udpData is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(datLen < 32)
    {
        LOG_Error("%s datLen < 32 ", __FUNCTION__);
        return RET_ERR;
    }

    // 修正：按照小端序正确解析Adler32校验和
    *adler32 = (uint32_t)udpData[20] << 0;
    *adler32 |= (uint32_t)udpData[21] << 8;
    *adler32 |= (uint32_t)udpData[22] << 16;
    *adler32 |= (uint32_t)udpData[23] << 24;

    LOG_Debug("%s adler32 bytes: [%02X %02X %02X %02X], value: %u", 
              __FUNCTION__, udpData[20], udpData[21], udpData[22], udpData[23], *adler32);
    return *adler32;
}

int32_t AudioGenerateUdpRes(char *udpData, int32_t datLen, uint32_t sequence, uint16_t session, uint16_t resData)
{
    uint32_t index = 0;
    int32_t count = 0;
    uint8_t xorValue = 0;

    if(NULL == udpData)
    {
        LOG_Error("%s argv is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(datLen < 16)
    {
        LOG_Error("%s datLen < 14 ", __FUNCTION__);
        return RET_ERR;
    }

    udpData[index++] = 0xFA;
    udpData[index++] = 0xFB;
    udpData[index++] = 0x01;
    udpData[index++] = 0x11;
    udpData[index++] = (sequence >> 0)& 0xFF;
    udpData[index++] = (sequence >> 8)& 0xFF;
    udpData[index++] = (sequence >> 16)& 0xFF;
    udpData[index++] = (sequence >> 24)& 0xFF;
    udpData[index++] = (session >> 0)& 0xFF;
    udpData[index++] = (session >> 8)& 0xFF;
    udpData[index++] = 0x00;
    udpData[index++] = 0x03;
    udpData[index++] = (resData >> 0)& 0xFF;
    udpData[index++] = (resData >> 8)& 0xFF;
    udpData[index++] = 0x00;

    for(count = 2; count < index; count++)
    {
        xorValue ^= udpData[count];
    }

    udpData[index++] = xorValue;
    udpData[index++] = 0xED;
    //LOG_DebugHex(udpData,index, "udpData");
    return index;
}

void *AudioFileUploadThread(void *argv)
{
    int32_t socketFd = -1;
    char recvBuff[1025] = {0};
    char sendBuff[1025] = {0};
    char fileContent[1025] = {0};
    char fileName[64] = {0};
    int32_t recvLen = 0;
    uint32_t index = 0;
    uint32_t fileLen = 0;
    uint32_t adler32 = 0;
    uint32_t tmpadler32 = 0;
    int32_t sendLen = 0;
    int32_t playloadLen = 0;
    int32_t retValue = RET_ERR;
    uint16_t session = 0;
    int32_t fileFd = -1;
    uint32_t writeLen = 0;
    uint32_t writeSum = 0;
    UdpInfo_t udpInfo;
    LOG_Debug("%s  tid: %d", __FUNCTION__, gettid());
    socketFd = AudioUdpBindPort(AUDIO_FILE_UPLOAD_PORT);

    while(1)
    {
        recvLen = AudioUdpRecFromFd_1(socketFd, recvBuff, 1025, &udpInfo);
        if(recvLen <= 0)
        {
            continue;
        }
        LOG_Debug("%s index:%u ", __FUNCTION__,index);
        //LOG_DebugHex(recvBuff,recvLen, "Recv udpData");
        playloadLen = AudioCheckUdpProtocol(&index, &session, recvBuff, recvLen);
        if(playloadLen < 0)
        {
            LOG_Error("%s AudioCheckUdpProtocol Error ", __FUNCTION__);
            sendLen = AudioGenerateUdpRes(sendBuff, 1024, index, session, 0x02);
            AudioUdpSendWithFd(socketFd, udpInfo.ipAddr, udpInfo.udpPort, sendBuff, sendLen);
            continue;
        }
        else if(0 == index)
        {
            AudioGetFileName(fileName, 64, recvBuff, recvLen);
            AudioGetFileLen(&fileLen, recvBuff, recvLen);
            AudioGetFileAdler32(&adler32, recvBuff, recvLen);
            if(fileFd > 2){
                close(fileFd);
                fileFd = -1;
            }
            fileFd = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, 0644);
            if(fileFd < 2)
            {
                LOG_Error("%s open Error ", __FUNCTION__);
                sendLen = AudioGenerateUdpRes(sendBuff, 1024, index, session, 0x02);
                AudioUdpSendWithFd(socketFd, udpInfo.ipAddr, udpInfo.udpPort, sendBuff, sendLen);
                 // 文件打开失败后，重置index以允许新的传输
                index = 0;
                writeSum = 0;
                continue; 
            }
            writeSum = 0;
            LOG_Debug("%s fileName:%s fileLen:%u adler32:%u", __FUNCTION__,fileName,fileLen,adler32);
            // 重置adler32校验算法状态
            AudioAdler32(NULL, 0, 0);
            memset(fileContent, 0, sizeof(fileContent));
        }
        else
        {
            playloadLen = AudioGetFileContent(fileContent, 1024, recvBuff, recvLen);
            writeLen = 0;
            while(writeLen != playloadLen)
            {
                retValue = write(fileFd, &fileContent[writeLen], playloadLen - writeLen);
                if(retValue <= 0)
                {
                    LOG_Error("%s write failed, retValue=%d", __FUNCTION__, retValue);
                    break;
                }
                writeLen += retValue;
            }
            if(writeLen != playloadLen)
            {
                LOG_Error("%s write Error ", __FUNCTION__);
                sendLen = AudioGenerateUdpRes(sendBuff, 1024, index, session, 0x01);
                AudioUdpSendWithFd(socketFd, udpInfo.ipAddr, udpInfo.udpPort, sendBuff, sendLen);
                continue;
            }
            // 更新已写入的总字节数
            writeSum += writeLen;
            tmpadler32 = AudioAdler32((uint8_t *)fileContent, playloadLen, 1);
            if(writeSum == fileLen && tmpadler32 != adler32)
            {
                LOG_Error("%s adler32 Error ", __FUNCTION__);
                sendLen = AudioGenerateUdpRes(sendBuff, 1024, index, session, 0x01);
                AudioUdpSendWithFd(socketFd, udpInfo.ipAddr, udpInfo.udpPort, sendBuff, sendLen);
                close(fileFd);
                unlink(fileName);  // 校验失败时删除已写入的文件
                fileFd = -1;
                index = 0;
                writeSum = 0;
                continue;
            }
            else if(writeSum == fileLen && tmpadler32 == adler32)
            {
                LOG_Debug("%s upload file Successful", __FUNCTION__);
                sendLen = AudioGenerateUdpRes(sendBuff, 1024, index, session, 0x00);
                AudioUdpSendWithFd(socketFd, udpInfo.ipAddr, udpInfo.udpPort, sendBuff, sendLen);
                close(fileFd);
                
                // 删除同编号的其他格式音频文件，保留刚上传的文件
                char baseName[64];
                int fileNum;
                char currentExt[10];
                
                // 解析当前文件的编号和扩展名
                if (sscanf(fileName, "/oemdata/tts/%03d.%9s", &fileNum, currentExt) == 2) {
                    // 遍历同编号的不同格式文件，删除除当前扩展名外的其他格式
                    char cmd[128];
                    snprintf(cmd, sizeof(cmd), 
                             "find /oemdata/tts/ -name '%03d.*' ! -name '%03d.%s' -delete 2>/dev/null", 
                             fileNum, fileNum, currentExt);
                    LOG_Debug("%s will do cmd:%s", __FUNCTION__,cmd);
                    SrGetSystemRetValue(cmd);
                }
                
                index = 0;
                writeSum = 0;
                continue;
            }else{
                LOG_Debug("%s sum %u:%u AudioAdler32 %u:%u writeLen:%u:%u", __FUNCTION__,writeSum,fileLen,tmpadler32,adler32,writeLen,playloadLen);
            }
        }

        sendLen = AudioGenerateUdpRes(sendBuff, 1024, index, session, 0x00);
        AudioUdpSendWithFd(socketFd, udpInfo.ipAddr, udpInfo.udpPort, sendBuff, sendLen);
        index++;
        LOG_Debug("%s will index++:%u ", __FUNCTION__,index);
    }

}