/** *****************************************************************************
* @file         dns.c
* @brief
* @details
* @author       冯笑磊
* @date         2021
* @copyright    斯润天朗（北京）科技有限公司
* @attention    None
* @version      1.0
*               2021-07-10  冯笑磊 
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "dns.h"
#include "sr_log.h"
#include "err_code.h"

#define  DNS_CONFIG_FILE		"/etc/resolv.conf"



/** ******************************************************************************
* @remarks    int32_t head_generate(uint8_t  *pBuffer,
                                    uint16_t *pOffset,
                                    uint16_t  Length,
                                    dns_head *pHead)

* @brief      head 创建
* @param      [IN ] pBuffer    缓存
* @param      [I/O] pOffset    偏移量
* @param      [IN ] Length     缓存长度
* @param      [IN ] pHead      head
* @return     NONE
* @attention
********************************************************************************/
static int32_t head_generate(uint8_t  *pBuffer,
                             uint16_t *pOffset,
                             uint16_t  Length,
                             dns_head *pHead)
{
    uint32_t Offset   = 0;
    uint16_t TempUn16 = 0;

    if ((pBuffer == NULL) || (pOffset == NULL) || (pHead == NULL))
        return ERR_UNDEFINE;

    Offset = *pOffset;
    if (Offset + DNS_LENGTH_HEADER > Length)
        return ERR_OUT_OF_RANGE;

    /* ID */
    pBuffer[Offset + 0] = ((pHead->ID >> 8) & 0X00FF);
    pBuffer[Offset + 1] = ((pHead->ID >> 0) & 0X00FF);
    Offset = Offset + 2;

    /* FLAGS */
    TempUn16 = 0;
    TempUn16 |= (pHead->QR     << DNS_HEAD_FLAG_OFFSET_QR);
    TempUn16 |= (pHead->opcode << DNS_HEAD_FLAG_OFFSET_OPCODE);
    TempUn16 |= (pHead->AA     << DNS_HEAD_FLAG_OFFSET_AA);
    TempUn16 |= (pHead->TC     << DNS_HEAD_FLAG_OFFSET_TC);
    TempUn16 |= (pHead->RD     << DNS_HEAD_FLAG_OFFSET_RD);
    TempUn16 |= (pHead->RA     << DNS_HEAD_FLAG_OFFSET_RA);
    TempUn16 |= (pHead->rcode  << DNS_HEAD_FLAG_OFFSET_RCODE);

    pBuffer[Offset + 0] = ((TempUn16 >> 8) & 0X00FF);
    pBuffer[Offset + 1] = ((TempUn16 >> 0) & 0X00FF);
    Offset = Offset + 2;

    /* QDCOUNT */
    pBuffer[Offset + 0] = ((pHead->QDCOUNT >> 8) & 0X00FF);
    pBuffer[Offset + 1] = ((pHead->QDCOUNT >> 0) & 0X00FF);
    Offset = Offset + 2;

    /* ANCOUNT */
    pBuffer[Offset + 0] = ((pHead->ANCOUNT >> 8) & 0X00FF);
    pBuffer[Offset + 1] = ((pHead->ANCOUNT >> 0) & 0X00FF);
    Offset = Offset + 2;

    /* NSCOUNT */
    pBuffer[Offset + 0] = ((pHead->NSCOUNT >> 8) & 0X00FF);
    pBuffer[Offset + 1] = ((pHead->NSCOUNT >> 0) & 0X00FF);
    Offset = Offset + 2;

    /* ARCOUNT */
    pBuffer[Offset + 0] = ((pHead->ARCOUNT >> 8) & 0X00FF);
    pBuffer[Offset + 1] = ((pHead->ARCOUNT >> 0) & 0X00FF);
    Offset = Offset + 2;

    *pOffset = Offset;
    return NO_ERR;
}

/** ******************************************************************************
* @remarks    int32_t question_generate(uint8_t      *pBuffer,
                                        uint16_t     *pOffset,
                                        uint16_t      Length,
                                        dns_question *pQuestion)

* @brief      question 创建
* @param      [IN ] pBuffer    缓存
* @param      [I/O] pOffset    偏移量
* @param      [IN ] Length     缓存长度
* @param      [IN ] pQuestion  question
* @return     NONE
* @attention
********************************************************************************/
static int32_t question_generate(uint8_t      *pBuffer,
                                 uint16_t     *pOffset,
                                 uint16_t      Length,
                                 dns_question *pQuestion)
{
    uint32_t i          = 0;
    uint32_t j          = 0;
    int32_t  RetVal     = 0;

    uint32_t Offset     = 0;
    uint32_t NameLength = 0;

    if ((pBuffer == NULL) || (pOffset == NULL) || (pQuestion == NULL))
        return ERR_UNDEFINE;

    Offset = *pOffset;

    /* name length */
    RetVal = strlen((char *)pQuestion->QName);
    if (RetVal < NO_ERR)
        return ERR_UNDEFINE;

    NameLength = RetVal;

    /* QNAME */
    if (Offset + NameLength + 2 > Length)
        return ERR_OUT_OF_RANGE;

    /* 如： www.baidu.com */
    /* 03 77 77 77 05 62 61 69 64 75 03 63 6f 6d 00  */
    for (i = 0; i < NameLength; i++)
    {
        if ((pQuestion->QName[i] != '.') && (i != 0))
        {
            pBuffer[Offset] = pQuestion->QName[i];
            Offset++;
            continue;
        }

        for (j = i + 1; j < NameLength; j++)
        {
            if (pQuestion->QName[j] == '.')
                break;
        }

        if (i == 0)
        {
            pBuffer[Offset] = j - i;
            Offset++;

            pBuffer[Offset] = pQuestion->QName[i];
            Offset++;
        }
        else
        {
            pBuffer[Offset] = j - (i + 1);
            Offset++;
        }

    }

    pBuffer[Offset] = 0;
    Offset++;

    /* QTYPE + QCLASS */
    if (Offset + 4 > Length)
        return ERR_OUT_OF_RANGE;

    /* QTYPE */
    pBuffer[Offset + 0] = ((pQuestion->QTYPE >> 8) & 0X00FF);
    pBuffer[Offset + 1] = ((pQuestion->QTYPE >> 0) & 0X00FF);
    Offset = Offset + 2;

    /* QCLASS */
    pBuffer[Offset + 0] = ((pQuestion->QCLASS >> 8) & 0X00FF);
    pBuffer[Offset + 1] = ((pQuestion->QCLASS >> 0) & 0X00FF);
    Offset = Offset + 2;

    *pOffset = Offset;
    return NO_ERR;
}

/** ******************************************************************************
* @remarks    int32_t head_parse(uint8_t  *pBuffer,
                                 uint16_t *pOffset,
                                 uint16_t  Length,
                                 dns_head *pHead)

* @brief      head 解析
* @param      [IN ] pBuffer    缓存
* @param      [I/O] pOffset    偏移量
* @param      [IN ] Length     缓存长度
* @param      [OUT] pHead      head
* @return     NONE
* @attention
********************************************************************************/
static int32_t head_parse(uint8_t  *pBuffer,
                          uint16_t *pOffset,
                          uint16_t  Length,
                          dns_head *pHead)
{
    uint32_t Offset   = 0;
    uint16_t TempUn16 = 0;

    if ((pBuffer == NULL) || (pOffset == NULL) || (pHead == NULL))
        return ERR_UNDEFINE;

    Offset = *pOffset;
    if (Offset + DNS_LENGTH_HEADER > Length)
        return ERR_OUT_OF_RANGE;

    /* ID */
    TempUn16 = 0;
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 0];
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 1];
    Offset = Offset + 2;

    pHead->ID = TempUn16;

    /* FLAGS */
    TempUn16 = 0;
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 0];
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 1];
    Offset = Offset + 2;

    pHead->QR     = (TempUn16 >> DNS_HEAD_FLAG_OFFSET_QR);
    pHead->opcode = (TempUn16 >> DNS_HEAD_FLAG_OFFSET_OPCODE);
    pHead->AA     = (TempUn16 >> DNS_HEAD_FLAG_OFFSET_AA);
    pHead->TC     = (TempUn16 >> DNS_HEAD_FLAG_OFFSET_TC);
    pHead->RD     = (TempUn16 >> DNS_HEAD_FLAG_OFFSET_RD);
    pHead->RA     = (TempUn16 >> DNS_HEAD_FLAG_OFFSET_RA);
    pHead->rcode  = (TempUn16 >> DNS_HEAD_FLAG_OFFSET_RCODE);

    /* QDCOUNT */
    TempUn16 = 0;
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 0];
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 1];
    Offset = Offset + 2;

    pHead->QDCOUNT = TempUn16;

    /* ANCOUNT */
    TempUn16 = 0;
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 0];
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 1];
    Offset = Offset + 2;

    pHead->ANCOUNT = TempUn16;

    /* NSCOUNT */
    TempUn16 = 0;
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 0];
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 1];
    Offset = Offset + 2;

    pHead->NSCOUNT = TempUn16;

    /* ARCOUNT */
    TempUn16 = 0;
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 0];
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 1];
    Offset = Offset + 2;

    pHead->ARCOUNT = TempUn16;

    *pOffset = Offset;
    return NO_ERR;
}

/** ******************************************************************************
* @remarks    int32_t name_skip(uint8_t      *pBuffer,
                                uint16_t     *pOffset,
                                uint16_t      Length)

* @brief      name 跳过
* @param      [IN ] pBuffer    缓存
* @param      [I/O] pOffset    偏移量
* @param      [IN ] Length     缓存长度
* @return     NONE
* @attention
********************************************************************************/
static int32_t name_skip(uint8_t      *pBuffer,
                         uint16_t     *pOffset,
                         uint16_t      Length)
{
    uint32_t Offset = 0;

    if ((pBuffer == NULL) || (pOffset == NULL))
        return ERR_UNDEFINE;

    Offset = *pOffset;

    for (; Offset < Length;)
    {
        if (pBuffer[Offset] == DNS_PTR_NAME_FLAG)
        {
            Offset = Offset + 2;
            break;
        }

        if (pBuffer[Offset] == 0)
        {
            Offset = Offset + 1;
            break;
        }

        Offset = Offset + 1 + pBuffer[Offset];
    }

    if (Offset > Length)
        return ERR_OUT_OF_RANGE;

    *pOffset = Offset;
    return NO_ERR;
}

/** ******************************************************************************
* @remarks    int32_t question_parse(uint8_t      *pBuffer,
                                     uint16_t     *pOffset,
                                     uint16_t      Length,
                                     dns_question *pQuestion)

* @brief      question解析
* @param      [IN ] pBuffer    缓存
* @param      [I/O] pOffset    偏移量
* @param      [IN ] Length     缓存长度
* @param      [OUT] pQuestion  question
* @return     NONE
* @attention
********************************************************************************/
static int32_t question_parse(uint8_t      *pBuffer,
                              uint16_t     *pOffset,
                              uint16_t      Length,
                              dns_question *pQuestion)
{
    int32_t  RetVal     = 0;
    uint16_t TempUn16   = 0;

    uint16_t Offset     = 0;

    if ((pBuffer == NULL) || (pOffset == NULL) || (pQuestion == NULL))
        return ERR_UNDEFINE;

    Offset = *pOffset;

    /* QNAME */
    pQuestion->QName = &pBuffer[Offset];

    RetVal = name_skip(pBuffer, &Offset, Length);
    if (RetVal < NO_ERR)
        return ERR_UNDEFINE;

    /* QTYPE + QCLASS */
    if (Offset + 4 > Length)
        return ERR_OUT_OF_RANGE;

    /* QTYPE */
    TempUn16 = 0;
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 0];
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 1];
    Offset = Offset + 2;

    pQuestion->QTYPE = TempUn16;

    /* QCLASS */
    TempUn16 = 0;
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 0];
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 1];
    Offset = Offset + 2;

    pQuestion->QCLASS = TempUn16;

    *pOffset = Offset;
    return NO_ERR;
}

/** ******************************************************************************
* @remarks    int32_t rrs_parse(uint8_t      *pBuffer,
                                uint16_t     *pOffset,
                                uint16_t      Length,
                                dns_rrs      *pRRs)

* @brief      rrs解析
* @param      [IN ] pBuffer    缓存
* @param      [I/O] pOffset    偏移量
* @param      [IN ] Length     缓存长度
* @param      [OUT] pRRs       RRS
* @return     NONE
* @attention
********************************************************************************/
static int32_t rrs_parse(uint8_t      *pBuffer,
                         uint16_t     *pOffset,
                         uint16_t      Length,
                         dns_rrs      *pRRs)
{
    int32_t  RetVal     = 0;
    uint16_t TempUn16   = 0;
    uint32_t TempUn32   = 0;

    uint16_t Offset     = 0;

    if ((pBuffer == NULL) || (pOffset == NULL) || (pRRs == NULL))
        return ERR_UNDEFINE;

    Offset = *pOffset;

    /* QNAME */
    pRRs->Name = &pBuffer[Offset];

    RetVal = name_skip(pBuffer, &Offset, Length);
    if (RetVal < NO_ERR)
        return ERR_UNDEFINE;

    /* TYPE + CLASS + TTL + RDLENGTH */
    if (Offset + 8 > Length)
        return ERR_OUT_OF_RANGE;

    /* TYPE */
    TempUn16 = 0;
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 0];
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 1];
    Offset = Offset + 2;

    pRRs->TYPE = TempUn16;

    /* CLASS */
    TempUn16 = 0;
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 0];
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 1];
    Offset = Offset + 2;

    pRRs->CLASS = TempUn16;

    /* TTL */
    TempUn32 = 0;
    TempUn32 = ((TempUn32 << 8) & 0XFF00) + pBuffer[Offset + 0];
    TempUn32 = ((TempUn32 << 8) & 0XFF00) + pBuffer[Offset + 1];
    TempUn32 = ((TempUn32 << 8) & 0XFF00) + pBuffer[Offset + 2];
    TempUn32 = ((TempUn32 << 8) & 0XFF00) + pBuffer[Offset + 3];
    Offset = Offset + 4;

    pRRs->TTL = TempUn32;

    /* RDLENGTH */
    TempUn16 = 0;
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 0];
    TempUn16 = ((TempUn16 << 8) & 0XFF00) + pBuffer[Offset + 1];
    Offset = Offset + 2;

    pRRs->RDLENGTH = TempUn16;

    /* RDDATA */
    if (Offset + pRRs->RDLENGTH > Length)
        return ERR_OUT_OF_RANGE;

    pRRs->RDDATA = &pBuffer[Offset];
    Offset = Offset + pRRs->RDLENGTH;

    *pOffset = Offset;
    return NO_ERR;
}

/** ******************************************************************************
* @remarks    int32_t request_generate(dns_s *pStruct)
*
* @brief      dns 发送数据创建
* @param      [IN ] pStruct    DNS数据结构
* @return     NONE
* @attention
********************************************************************************/
static int32_t request_generate(dns_s *pStruct)
{
    int32_t   RetVal = 0;
    dns_head *pHead  = NULL;

    if (pStruct == NULL)
        return ERR_NULL_POINT;

    pHead = &pStruct->head;

    pStruct->packet_length = 0;
    pStruct->ID            = pStruct->ID + 1;
    pHead->ID              = pStruct->ID;

    RetVal = head_generate(pStruct->packet,
                           &pStruct->packet_length,
                           DNS_MAX_LENGTH_PACKET,
                           &pStruct->head);
    if (RetVal < NO_ERR)
        return ERR_UNDEFINE;

    RetVal = question_generate(pStruct->packet,
                               &pStruct->packet_length,
                               DNS_MAX_LENGTH_PACKET,
                               &pStruct->question);
    if (RetVal < NO_ERR)
        return ERR_UNDEFINE;

    return NO_ERR;
}

/** ******************************************************************************
* @remarks    int32_t reponse_parse(dns_s *pStruct)
*
* @brief      dns 应答解析
* @param      [IN ] pStruct    DNS数据结构
* @return     NONE
* @attention
********************************************************************************/
static int32_t reponse_parse(dns_s *pStruct)
{
    uint32_t  i      = 0;
    int32_t   RetVal = 0;

    uint16_t  Offset = 0;
    uint32_t  RRsCnt = 0;
    dns_head *pHead  = NULL;

    if (pStruct == NULL)
        return ERR_NULL_POINT;

    pHead = &pStruct->head;

    /* head */
    RetVal = head_parse(pStruct->packet,
                        &Offset,
                        pStruct->packet_length,
                        &pStruct->head);
    if (RetVal < NO_ERR)
        return ERR_UNDEFINE;

    if (pHead->ID != pStruct->ID)
        return ERR_UNDEFINE;

    if (pHead->rcode != DNS_HEAD_FLAG_RCODE_NONE)
        return ERR_UNDEFINE;

    /* question */
    RetVal = question_parse(pStruct->packet,
                            &Offset,
                            pStruct->packet_length,
                            &pStruct->question);
    if (RetVal < NO_ERR)
        return ERR_UNDEFINE;

    /* rrs */
    RRsCnt = pHead->ANCOUNT + pHead->NSCOUNT + pHead->ARCOUNT;
    if (RRsCnt > DNS_MAX_COUNTER_RRS)
        return ERR_OUT_OF_RANGE;

    for (i = 0; i < RRsCnt; i++)
    {
        RetVal = rrs_parse(pStruct->packet,
                           &Offset,
                           pStruct->packet_length,
                           &pStruct->rrs[i]);
        if (RetVal < NO_ERR)
            return ERR_UNDEFINE;
    }

    return NO_ERR;
}

/** ******************************************************************************
* @remarks    int32_t udp_request_response(dns_s      *pStruct,
                                           const char *pServer,
                                           uint16_t    Port,
                                           const char *pInterface,
                                           uint32_t    WaitMs)
*
* @brief      dns 发送->应答
* @param      [IN ] pStruct    DNS数据结构
* @param      [IN ] pServer    服务器地址
* @param      [IN ] Port       服务器端口
* @param      [IN ] pInterface 网卡
* @param      [IN ] WaitMs     等待时间
* @return     NONE
* @attention
********************************************************************************/
static int32_t udp_request_response(dns_s      *pStruct,
                                    const char *pServer,
                                    uint16_t    Port,
                                    const char *pInterface,
                                    uint32_t    WaitMs)
{
    int32_t              RetVal    = 0;
    uint32_t             Length    = 0;

    int                  fd_socket = -1;
    struct ifreq         if_request;
    struct sockaddr_in   srv_addr;

    fd_set               read_fds;
    struct timeval       time_val;

    if ((pStruct == NULL) || (pServer == NULL))
        return ERR_NULL_POINT;

    /* 创建SOCKET */
    fd_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd_socket < NO_ERR)
        goto _error;

    /* 绑定接口 */
    if (pInterface != NULL)
    {
        RetVal = strlen(pInterface);
        if (RetVal < NO_ERR)
            goto _error;

        Length = RetVal;
        memset(&if_request, 0, sizeof(struct ifreq));
        strncpy(if_request.ifr_name, pInterface, Length);

        RetVal = setsockopt(fd_socket,
                            SOL_SOCKET,  SO_BINDTODEVICE,
                            (char *)&if_request, sizeof(struct ifreq));
        if (RetVal < NO_ERR)
            goto _error;
    }

    /* 创建发送数据 */
    RetVal = request_generate(pStruct);
    if (RetVal < NO_ERR)
        goto _error;

    /* 发送数据 */
    bzero(&srv_addr, sizeof(struct sockaddr_in));

    srv_addr.sin_family      = AF_INET;
    srv_addr.sin_port        = htons(Port);
    srv_addr.sin_addr.s_addr = inet_addr(pServer);

    RetVal = sendto(fd_socket,
                    pStruct->packet, pStruct->packet_length,
                    0
                    , (struct sockaddr *)&srv_addr, sizeof(struct sockaddr));
    if (RetVal < NO_ERR)
        goto _error;

    /* select 等待应答 */
    FD_ZERO(&read_fds);
    FD_SET(fd_socket, &read_fds);

    time_val.tv_sec  = WaitMs / 1000;
    time_val.tv_usec = (WaitMs % 1000) * 1000;

    RetVal = select(fd_socket + 1, &read_fds, NULL, NULL, &time_val);
    if (RetVal < NO_ERR)
        goto _error;

    RetVal = FD_ISSET(fd_socket, &read_fds);
    if (RetVal <= NO_ERR)
        goto _error;

    /* 接收数据 */
    RetVal = recv(fd_socket, pStruct->packet, DNS_MAX_LENGTH_PACKET, 0);
    if (RetVal <= NO_ERR)
        goto _error;

    pStruct->packet_length = RetVal;

    /* request parse */
    RetVal = reponse_parse(pStruct);
    if (RetVal < NO_ERR)
        goto _error;

    close(fd_socket);
    return NO_ERR;

_error:
    if (fd_socket >= NO_ERR)
        close(fd_socket);

    return ERR_UNDEFINE;
}

/** ******************************************************************************
* @remarks    int32_t domain_2_ipv4(dns_s          *pStruct,
                                    const char     *pServer,
                                    uint16_t        Port,
                                    const char     *pInterface,
                                    const char     *pDomain,
                                    uint8_t        *pIPv4,
                                    uint32_t        WaitMs)
*
* @brief      dns 域名解析
* @param      [IN ] pStruct    DNS数据结构
* @param      [IN ] pServer    服务器地址
* @param      [IN ] Port       服务器端口
* @param      [IN ] pInterface 网卡
* @param      [IN ] pDomain    待解析域名
* @param      [IN ] pIPv4      解析后的ip地址
* @param      [IN ] WaitMs     等待时间
* @return     NONE
* @attention
********************************************************************************/
static int32_t domain_2_ipv4(dns_s          *pStruct,
                             const char     *pServer,
                             uint16_t        Port,
                             const char     *pInterface,
                             const char     *pDomain,
                             uint8_t        *pIPv4,
                             uint32_t        WaitMs)
{
    uint32_t      i          = 0;
    int32_t       RetVal     = 0;
    uint32_t      IPv4Length = 4;

    dns_head     *pHead      = NULL;
    dns_question *pQuestion  = NULL;
    dns_rrs      *pRRs       = NULL;

    uint16_t      TTLMax     = 0xFFFF;

    if ((pStruct == NULL) || (pDomain == NULL) || (pIPv4 == NULL))
        return ERR_NULL_POINT;

    pHead     = &pStruct->head;
    pQuestion = &pStruct->question;

    memset(pHead,     0, sizeof(dns_head));
    memset(pQuestion, 0, sizeof(dns_question));

    /* request -> response */
    pHead->RD         = DNS_HEAD_FLAG_RD;
    pHead->QDCOUNT    = 1;

    pQuestion->QName  = (const uint8_t *)pDomain;
    pQuestion->QTYPE  = DNS_HEAD_TYPE_A;
    pQuestion->QCLASS = DNS_HEAD_CLASS_IN;

    RetVal = udp_request_response(pStruct, pServer, Port, pInterface, WaitMs);
    if (RetVal < NO_ERR)
        return ERR_UNDEFINE;

    /* filter host answer */
    if (pHead->ANCOUNT == 0)
        return ERR_UNDEFINE;

    for (i = 0; i < pHead->ANCOUNT; i++)
    {
        pRRs = &pStruct->rrs[i];
        if (pRRs->TYPE != DNS_HEAD_TYPE_A)
            continue;

        if (pRRs->RDLENGTH != IPv4Length)
            continue;

        if (TTLMax < pRRs->TTL)
            continue;

        TTLMax = pRRs->TTL;
        memcpy(pIPv4, pRRs->RDDATA, IPv4Length);
    }

    if (TTLMax == 0XFFFF)
        return ERR_UNDEFINE;

    return NO_ERR;
}

static int32_t dns_init(dns_s *pStruct)
{
    pStruct->ID            = 0;
    pStruct->packet_length = 0;

    pthread_mutex_init(&(pStruct->mutex_operate), NULL);
   
    return NO_ERR;
}

/** ******************************************************************************
* @remarks    int32_t dns_domain_2_ipv4(dns_s          *pStruct,
                                        const char     *pServer,
                                        uint16_t        Port,
                                        const char     *pInterface,
                                        const char     *pDomain,
                                        uint8_t        *pIPv4,
                                        uint32_t        WaitMs)
*
* @brief      dns 域名解析
* @param      [IN ] pStruct    DNS数据结构
* @param      [IN ] pServer    服务器地址
* @param      [IN ] Port       服务器端口
* @param      [IN ] pInterface 网卡
* @param      [IN ] pDomain    待解析域名
* @param      [IN ] pIPv4      解析后的ip地址
* @param      [IN ] WaitMs     等待时间
* @return     NONE
* @attention
********************************************************************************/
static int32_t dns_domain_2_ipv4(dns_s          *pStruct,
                                 const char     *pServer,
                                 uint16_t        Port,
                                 const char     *pInterface,
                                 const char     *pDomain,
                                 uint8_t        *pIPv4,
                                 uint32_t        WaitMs)
{
    int32_t RetVal = NO_ERR;

    if ((pStruct == NULL))
        return ERR_NULL_POINT;

    pthread_mutex_lock(&pStruct->mutex_operate);

    RetVal = domain_2_ipv4(pStruct, pServer, Port, pInterface,
                           pDomain, pIPv4, WaitMs);

    pthread_mutex_unlock(&pStruct->mutex_operate);
    return RetVal;
}

static dns_s gb_dns;

/** ******************************************************************************
* @remarks    int32_t gb_dns_init(void)
*
* @brief      dns 初始化
* @param      [IN ] NONE
* @return     NONE
* @attention
********************************************************************************/
int32_t gb_dns_init(void)
{
    return dns_init(&gb_dns);
}

/** ******************************************************************************
* @remarks    int32_t gb_dns_domain_2_ipv4(const char     *pServer,
                                           uint16_t        Port,
                                           const char     *pInterface,
                                           const char     *pDomain,
                                           uint8_t        *pIPv4,
                                           uint32_t        WaitMs);
*
* @brief      dns 域名解析
* @param      [IN ] pServer    服务器地址
* @param      [IN ] Port       服务器端口
* @param      [IN ] pInterface 网卡
* @param      [IN ] pDomain    待解析域名
* @param      [IN ] pIPv4      解析后的ip地址
* @param      [IN ] WaitMs     等待时间
* @return     NONE
* @attention
********************************************************************************/
int32_t gb_dns_domain_2_ipv4(const char     *pServer,
                             uint16_t        Port,
                             const char     *pInterface,
                             const char     *pDomain,
                             uint8_t        *pIPv4,
                             uint32_t        WaitMs)
{
	dns_s *dnsInfo = NULL;
	int32_t retValue = -1;
	dnsInfo = (dns_s *)malloc(sizeof(dns_s));
	if(NULL == dnsInfo)
	{
		return ERR_NULL_POINT;
	}
	
	dns_init(dnsInfo);
	
    retValue = dns_domain_2_ipv4(dnsInfo, pServer, Port, pInterface,
                             pDomain, pIPv4, WaitMs);
							 
	free(dnsInfo);
	return retValue;
}



/** ******************************************************************************
* @remarks    int GetLineStrFormFile(int fileFd, char *fileBuff, int fileBuffLen)
*
* @brief      从文件中读一行数据
* @param      [IN ] fileFd      文件描述符
* @param      [OUT ] fileBuff    缓冲区BUFF
* @param      [IN ] fileBuffLen 缓冲区BUFF大小
* @return     -1： 错误  >0 ：读出的数据长度
* @attention
********************************************************************************/
int GetLineStrFormFile(int fileFd, char *fileBuff, int fileBuffLen)
{
	int retValue = -1;
	char readCh = 0;
	int readCount = 0;
	if(NULL == fileBuff || fileFd < 0)
	{
		return RET_ERR;
	}
	
	while(readCh != '\n')
	{
		retValue = read(fileFd, &readCh, 1);
		if(retValue <= 0)
		{
			readCh = '\0';
			break;
		}
		
		if(readCount >= fileBuffLen)
		{
			return RET_ERR;
		}
		fileBuff[readCount++] = readCh;
	}

	if(retValue <= 0 && readCount == 0)
	{
		return RET_ERR;
	}
	
	fileBuff[readCount - 1] = '\0';
	
	return readCount;
}

/** ******************************************************************************
* @remarks    int ParserIPv4AddrFromStr(const char *srcStr, int srcStrLen, 
                                        char       *ipBuff, int ipBuffLen)
*
* @brief      从指定字符串中获取IP地址
* @param      [IN ] srcStr          字符串
* @param      [IN ] srcStrLen       字符串长度
* @param      [OUT ] ipBuff         IP地址缓冲区
* @param      [IN ] ipBuffLen       IP地址缓冲区大小
* @return     -1： 错误  >0 ：IP地址长度
* @attention
********************************************************************************/
int ParserIPv4AddrFromStr(const char *srcStr, int srcStrLen, 
                          char       *ipBuff, int ipBuffLen)
{
	int ipFrist  = 0;
	int ipSecond = 0;
	int ipThird  = 0;;
	int ipFourth = 0;
	int retValue = 0;
	if(NULL == srcStr || NULL == ipBuff)
	{
		return RET_ERR;
	}
	
	if(ipBuffLen < 16)
	{
		return RET_ERR;
	}
	
	retValue = sscanf(srcStr, "nameserver %d.%d.%d.%d", 
							  &ipFrist, &ipSecond, 
							  &ipThird, &ipFourth);
	if(4 != retValue)
	{
		return RET_ERR;
	}
	
	if(ipFrist < 0 || ipFrist > 255)
	{
		return RET_ERR;
	}
	
	if(ipSecond < 0 || ipSecond > 255)
	{
		return RET_ERR;
	}
	
	if(ipThird < 0 || ipThird > 255)
	{
		return RET_ERR;
	}
	
	if(ipFourth < 0 || ipFourth > 255)
	{
		return RET_ERR;
	}
	
	retValue = snprintf(ipBuff, ipBuffLen, "%d.%d.%d.%d", 
						ipFrist, ipSecond, ipThird, ipFourth);
						
	if(retValue < 0 || retValue > ipBuffLen)
	{
		return RET_ERR;
	}
	
	return retValue;
}


/** ******************************************************************************
* @remarks    int ParserIPv4AddrFromStr(const char *srcStr, int srcStrLen, 
                                        char       *ipBuff, int ipBuffLen)
*
* @brief      从指定字符串中获取IP地址
* @param      [OUT ] ipAddr[][16]    存放DNS地址的二维数组
* @param      [IN ] ipAddrLen        存放IP地址数组的大小
* @param      [IN ] ipAddrArrayLen   可以存放几个IP地址
* @return     -1： 错误  >0 ：存放的IP地址个数
* @attention
********************************************************************************/
int GetDNSIpAddrFromConfig(char ipAddr[][16], int ipAddrLen, int ipAddrArrayLen)
{
	static int ipIdex = 0;
	int readIPCount = 0;
	char readBuff[256] = {0};
	int readLen = 0;
	int retValue = -1;
	struct stat fileStat;
	int fileFd = -1;
	
	if(NULL == ipAddr || ipAddrLen < 16)
	{
		return RET_ERR;
	}
	
	if(0 != access(DNS_CONFIG_FILE, F_OK))
    {
        LOG_Error("%s does not exist", DNS_CONFIG_FILE);
        return RET_ERR;
    }

    fileFd = open(DNS_CONFIG_FILE,  O_RDONLY);
    if(-1 == fileFd)
    {
        LOG_Debug("open %s failed",DNS_CONFIG_FILE);
        return -1;
    }

    if(0 != fstat(fileFd, &fileStat))
    {
        LOG_Error("get file stat Error");
        close(fileFd);
        return RET_ERR;
    }

    if(!S_ISREG(fileStat.st_mode))
    {
        LOG_Error("get file not a file");
        close(fileFd);
        return RET_ERR;
    }

    if(fileStat.st_size < 18)
    {
        LOG_Error("get file size < 18");
        close(fileFd);
        return RET_ERR;
    }
	
	while(1)
	{
		memset(readBuff, 0, 256);
		retValue = GetLineStrFormFile(fileFd, readBuff, 256);
		if(retValue <= 0)
		{
			break;
		}
		
		LOG_Debug("readBuff: %s", readBuff);
		readLen = retValue;
		if(readIPCount >= ipAddrArrayLen)
		{
			break;
		}

		retValue = ParserIPv4AddrFromStr(readBuff,
										 readLen, 
										 ipAddr[readIPCount], 
										 ipAddrLen);
		
		if(retValue > 0)
		{
			readIPCount++;
		}
	}

	close(fileFd);

	return readIPCount;
}


/** ******************************************************************************
* @remarks    int ParserIPv4AddrFromStr(const char *srcStr, int srcStrLen, 
                                        char       *ipBuff, int ipBuffLen)
*
* @brief      通过DNS服务器解析域名
* @param      [IN ] domainName      域名名称
* @param      [IN ] domainNameLen   域名名称长度
* @param      [IN ] interface       网卡名称
* @param      [IN ] interfaceLen    网卡名称长度
* @param      [OUT ] ipAddrBuff     IP地址缓冲区
* @param      [IN ] ipAddrBuffLen   IP地址缓冲区长度
* @return     -1： 错误  >0 ：IP地址的长度
* @attention
********************************************************************************/
int GetHostIpAddrFromDNSServer(const char *domainName, int domainNameLen,
                               const char *interface , int interfaceLen ,
                               char       *ipAddrBuff, int ipAddrBuffLen)
{
    int retValue = -1;
    // const char *dnsAddr = "218.4.4.4";
	// const char *interface = "rmnet_data0";
	uint8_t ipV4Addr[4] = {0};
    char dnsIpBuff[4][16] = {0};
    int dnsIPCount = 0;
    int count = 0;

    retValue = GetDNSIpAddrFromConfig(dnsIpBuff,  16, 4);
    if(retValue < 0)
    {
        return RET_ERR;
    }
    dnsIPCount = retValue;

    for(count = 0; count < dnsIPCount; count++)
    {
        LOG_Debug("request dns addr is :%s", dnsIpBuff[count]);
        retValue = gb_dns_domain_2_ipv4(dnsIpBuff[count], 53, interface,
                                        domainName, ipV4Addr, 200);
        if(retValue < 0)
        {
            continue;
        }
        else
        {
            break;
        }
    }
	
    if(count >= dnsIPCount)
    {
        return RET_ERR;
    }

    retValue = snprintf(ipAddrBuff, ipAddrBuffLen,"%d.%d.%d.%d", 
                        ipV4Addr[0], ipV4Addr[1], ipV4Addr[2], ipV4Addr[3]);
	if(retValue < 0 || retValue >= ipAddrBuffLen)
    {
        return RET_ERR;
    }

	return RET_OK;
}
/** *****************************************************************************
 **                            End Of File
 ******************************************************************************/

