/** *****************************************************************************
* @file         dns.h
* @brief
* @details
* @author       冯笑磊
* @date         2021
* @copyright    斯润天朗（北京）科技有限公司
* @attention    None
* @version      1.0
*               2021-07-10  冯笑磊 
*******************************************************************************/
#ifndef _COMPONENT_DNS_H_
#define _COMPONENT_DNS_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Define --------------------------------------------------------------------*/
/* Typedef -------------------------------------------------------------------*/

/*
    +--+--+--+--+--+--+--+
    |        Header      |
    +--+--+--+--+--+--+--+
    |      Question      |
    +--+--+--+--+--+--+--+
    |      Answer        |
    +--+--+--+--+--+--+--+
    |      Authority     |
    +--+--+--+--+--+--+--+
    |      Additional    |
    +--+--+--+--+--+--+--+

Header
    0  1  2  3  4  5  6  7  0  1  2  3  4  5  6  7
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|  opcode   |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QDCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ANCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    NSCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ARCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

    ID: 2个字节(16bit)，标识字段，客户端会解析服务器返回的DNS应答报文，获取ID值与请求报文设置的ID值做比较，
                                如果相同，则认为是同一个DNS会话。

    FLAGS: 2个字节(16bit)的标志字段。包含以下属性:
        QR:     0表示查询报文，1表示响应报文;
        opcode: 通常值为0（标准查询），其他值为1（反向查询）和2（服务器状态请求）,[3,15]保留值;
        AA:     表示授权回答（authoritative answer）, 这个比特位在应答的时候才有意义，
                    指出给出应答的服务器是查询域名的授权解析服务器;
        TC:     表示可截断的（truncated）用来指出报文比允许的长度还要长，导致被截断;
        RD:     表示期望递归(Recursion Desired) 这个比特位被请求设置， 应答的时候使用的相同的值返回。
                    如果设置了RD，就建议域名服务器进行递归解析， 递归查询的支持是可选的;
        RA:     表示支持递归(Recursion Available) 这个比特位在应答中设置或取消，用来代表服务器是否支持递归查询;
        Z :     保留值，暂未使用;
        RCODE:  应答码(Response code) 这4个比特位在应答报文中设置，代表的含义如下:
            0 : 没有错误。
            1 : 报文格式错误(Format error) - 服务器不能理解请求的报文;
            2 : 服务器失败(Server failure) - 因为服务器的原因导致没办法处理这个请求;
            3 : 名字错误(Name Error)       - 只有对授权域名解析服务器有意义，指出解析的域名不存在;
            4 : 没有实现(Not Implemented)  - 域名服务器不支持查询类型;
            5 : 拒绝(Refused)              - 服务器由于设置的策略拒绝给出应答.比如，服务器不希望对某些请求者给出应答，
                                            或者服务器不希望进行某些操作（比如区域传送zone transfer）;
            [6,15] : 保留值，暂未使用。

    QDCOUNT: 无符号16bit整数表示报文请求段中的问题记录数。
    ANCOUNT: 无符号16bit整数表示报文回答段中的回答记录数。
    NSCOUNT: 无符号16bit整数表示报文授权段中的授权记录数。
    ARCOUNT: 无符号16bit整数表示报文附加段中的附加记录数。

Question
      0  1  2  3  4  5  6  7  0  1  2  3  4  5  6  7
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     ...                       |
    |                    QNAME                      |
    |                     ...                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QTYPE                      |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QCLASS                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

    QNAME 无符号8bit为单位长度不限表示查询名(广泛的说就是：域名).
    QTYPE 无符号16bit整数表示查询的协议类型.
    QCLASS 无符号16bit整数表示查询的类,比如，IN代表Internet.

RRs
          0  1  2  3  4  5  6  7  0  1  2  3  4  5  6  7
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    NAME                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    TYPE                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    CLASS                      |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    TTL                        |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    RDLENGTH                   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    RDATA                      |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

    NAME     资源记录包含的域名.
    TYPE     表示DNS协议的类型.
    CLASS    表示RDATA的类.
    TTL      4字节无符号整数表示资源记录可以缓存的时间。0代表只能被传输，但是不能被缓存。
    RDLENGTH 2个字节无符号整数表示RDATA的长度
    RDATA    不定长字符串来表示记录，格式根TYPE和CLASS有关。

TYPE
    1   A       a host address
    2   NS      an authoritative name server
    3   MD      a mail destination (Obsolete - use MX)
    4   MF      a mail forwarder (Obsolete - use MX)
    5   CNAME   the canonical name for an alias
    6   SOA     marks the start of a zone of authority
    7   MB      a mailbox domain name (EXPERIMENTAL)
    8   MG      a mail group member (EXPERIMENTAL)
    9   MR      a mail rename domain name (EXPERIMENTAL)
    10  NULL    a null RR (EXPERIMENTAL)
    11  WKS     a well known service description
    12  PTR     a domain name pointer
    13  HINFO   host information
    14  MINFO   mailbox or mail list information
    15  MX      mail exchange
    16  TXT     text strings
    252 AXFR    A request for a transfer of an entire zone
    253 MAILB   A request for mailbox-related records (MB, MG or MR)
    254 MAILA   A request for mail agent RRs (Obsolete - see MX)
    255 *       A request for all records

CLASS
    1   IN      the Internet
    2   CS      the CSNET class (Obsolete - used only for examples in some obsolete RFCs)
    3   CH      the CHAOS class
    4   HS      Hesiod [Dyer 87]

*/

#define NO_ERR                             0x00000000
#define ERR_SELF_SPC_RET                   -1
#define ERR_SYSTEM_STOP                    -1
#define ERR_UNDEFINE                       -100
#define ERR_NULL_POINT                     -101
#define ERR_OUT_OF_RANGE                   -102
#define ERR_INPUT_PARAMETER                -103
#define ERR_ALLOC_MUTEX                    -104
#define ERR_TAKE_MUTEX                     -105
#define ERR_ALLOC_SEMPH                    -106
#define ERR_TAKE_SEMPH                     -107
#define ERR_ALLOC_MEMORY                   -108
#define ERR_FLASH_READ                     -109
#define ERR_FLASH_WRITE                    -110
#define ERR_FLASH_ERASE                    -111
#define ERR_PARAMETER_UNINIT               -112
#define ERR_SUB_FUN_ERROR                  -113
#define ERR_BUFFER_OVERFLOW                -114
#define ERR_TIME_OUT                       -115
#define ERR_THREAD_CREATE                  -116
#define ERR_MUTEX_CREATE                   -117
#define ERR_MUTEX_TAKE                     -118
#define ERR_MUTEX_GIVE                     -119
#define ERR_SEMAPH_CREATE                  -120
#define ERR_SEMAPH_TAKE                    -121
#define ERR_SEMAPH_GIVE                    -122
#define ERR_DRIVER_OPEN                    -123
#define ERR_CRC_CHECK                      -124
#define ERR_DEV_STA                        -125
#define ERR_DEV_REV_FAIL                   -126
#define ERR_START_FAILED                   -127
#define ERR_UNINIT                         -128
#define ERR_INIT                           -129
#define ERR_MALLOC                         -130
#define ERR_EMPTY                          -131
#define ERR_EVENT_NONE                     -132
#define ERR_NO_MACTCH                      -133
#define ERR_RESPONE                        -134
#define ERR_LINE_GETTING                   -135
#define ERR_STATUS                         -136
#define ERR_CONNECT_STATUS                 -137
#define ERR_OTA_VERSION_SAME               -138
#define ERR_READ                           -139
#define ERR_WRITE                          -140
#define ERR_UPGRADE_STOP                   -141
#define ERR_UPGRADE_IGNORE                 -142
#define ERR_STR_2_SN64                     (0X8000000000000000)


#define DNS_MAX_LENGTH_PACKET                 512
#define DNS_MAX_COUNTER_RRS                   32
#define DNS_MAX_NAME                          64

#define DNS_LENGTH_HEADER                     12

/* head flag */
#define DNS_HEAD_FLAG_OFFSET_QR               15
#define DNS_HEAD_FLAG_OFFSET_OPCODE           14
#define DNS_HEAD_FLAG_OFFSET_AA               10
#define DNS_HEAD_FLAG_OFFSET_TC               9
#define DNS_HEAD_FLAG_OFFSET_RD               8
#define DNS_HEAD_FLAG_OFFSET_RA               7
#define DNS_HEAD_FLAG_OFFSET_Z                6
#define DNS_HEAD_FLAG_OFFSET_RCODE            3

#define DNS_HEAD_FLAG_QR_QUERY                0
#define DNS_HEAD_FLAG_QR_RESPONE              1

#define DNS_HEAD_FLAG_OPCODE_QUERY            0
#define DNS_HEAD_FLAG_OPCODE_IQUERY           1
#define DNS_HEAD_FLAG_OPCODE_STATUS           2

#define DNS_HEAD_FLAG_AA                      1
#define DNS_HEAD_FLAG_TC                      1
#define DNS_HEAD_FLAG_RD                      1
#define DNS_HEAD_FLAG_RA                      1

#define DNS_HEAD_FLAG_RCODE_NONE              0
#define DNS_HEAD_FLAG_RCODE_FORMAT            1
#define DNS_HEAD_FLAG_RCODE_SERVER            2
#define DNS_HEAD_FLAG_RCODE_NAME              3
#define DNS_HEAD_FLAG_RCODE_NOT_IMPLEMENTED   4
#define DNS_HEAD_FLAG_RCODE_REFUSED           5

/* type */
#define DNS_HEAD_TYPE_A                       1
#define DNS_HEAD_TYPE_CNAME                   5
#define DNS_HEAD_TYPE_PTR                     12

/* class */
#define DNS_HEAD_CLASS_IN                     1

/* PTRNAME */
#define DNS_PTR_NAME_FLAG                     0XC0


typedef pthread_mutex_t mutex_t;

typedef struct
{
    uint16_t ID;

    uint16_t QR     : 1;
    uint16_t opcode : 4;
    uint16_t AA     : 1;
    uint16_t TC     : 1;
    uint16_t RD     : 1;
    uint16_t RA     : 1;
    uint16_t rcode  : 4;

    uint16_t QDCOUNT;
    uint16_t ANCOUNT;
    uint16_t NSCOUNT;
    uint16_t ARCOUNT;

} dns_head;

typedef struct
{
    const uint8_t *QName;
    uint16_t       QTYPE;
    uint16_t       QCLASS;

} dns_question;

typedef struct
{
    const uint8_t *Name;
    uint16_t       TYPE;
    uint16_t       CLASS;
    uint32_t       TTL;
    uint16_t       RDLENGTH;
    const uint8_t *RDDATA;

} dns_rrs;

typedef struct
{
    mutex_t      mutex_operate;
    uint16_t     ID;

    uint16_t     packet_length;
    uint8_t      packet[DNS_MAX_LENGTH_PACKET];

    dns_head     head;
    dns_question question;
    dns_rrs      rrs[DNS_MAX_COUNTER_RRS];

} dns_s;

/** ******************************************************************************
* @remarks    int32_t gb_dns_init(void)
*
* @brief      dns 初始化
* @param      [IN ] NONE
* @return     NONE
* @attention
********************************************************************************/
extern int32_t gb_dns_init(void);

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
extern int32_t gb_dns_domain_2_ipv4(const char     *pServer,
                                    uint16_t        Port,
                                    const char     *pInterface,
                                    const char     *pDomain,
                                    uint8_t        *pIPv4,
                                    uint32_t        WaitMs);

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
                               char       *ipAddrBuff, int ipAddrBuffLen);


#endif
/** *****************************************************************************
**                            End Of File
******************************************************************************/
