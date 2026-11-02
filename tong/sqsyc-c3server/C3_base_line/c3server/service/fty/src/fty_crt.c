#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <string.h>

#include "err_code.h"
#include "sr_log.h"
#include "fty_crt.h"

typedef enum ENUM_CRT_TYPE
{
    E_CREATE_CSR = 0,
    E_RECV_ROOTCERT,
    E_RECV_COMCERT,
    E_RECV_FINISH
}ENUM_CRT_TYPE_t;

#define CRT_MSG_BUFFER (4096)
#define CRT_ELEMENT_MAX (128)
#define CRT_SERV_PORT (32168)

typedef struct
{
    guint msgtype;
    guint length;
    guint checksum;
} CrtMsg_t;

#define CRT_CONFIG_PATH "/usrdata/status/ca_bak"

static guint checkSum(gchar payload[], gint length)
{
    guint sum = 0;
    gint i = 0;
    while (i < length)
    {
        sum += payload[i];
        i++;
    }
    return sum;
}

static gint GetCrtFileMd5(const gchar *fileName, gchar *versionBuf)
{
    gchar buf[256] = {0x00};
    gint ret = RET_ERR;

    if (!fileName || !versionBuf)
    {
        return ret;
    }
    
    FILE *fp;
    fp = fopen(fileName, "r");
    if (fp)
    {

        if (fgets(buf, sizeof(buf), fp) != NULL)
        {
            LOG_Debug("Read md5 from File[%s]-[%s]", fileName, buf);
            memcpy(versionBuf, buf, strlen(buf));
            ret = RET_OK;
        }
        else
        {
            LOG_Error("Read Error From VersionFile[%s]", strerror(errno));
        }

        fclose(fp);
    }

    return ret;
}


static void CreateCsr(gint connfd, gchar payload[], gint length)
{
    gchar tboxModel[CRT_ELEMENT_MAX] = {
        0x0,
    };
    gchar sn[CRT_ELEMENT_MAX] = {
        0x0,
    };
    gchar buf[CRT_MSG_BUFFER] = {
        0x0,
    };
    gchar *ptr = NULL;

    if (1)
    {
        system("mkdir -p /usrdata/status/ca_bak");
           
    }
    LOG_Debug("-----%s----%d--,", __FUNCTION__, __LINE__);
    memcpy(buf, payload, length);
    ptr = strstr(buf, ";"); //"tboxModel;sn"

    if (ptr == NULL)
    {
        LOG_Debug("createCsr payload error");
        return;
    }

    *ptr = 0x0;
    ptr++;
    strcpy(tboxModel, buf);
    strcpy(sn, ptr);

    gchar cmd[512] = {
        0x0,
    };
    snprintf(cmd, sizeof(cmd), "/usrdata/app/create_csr.sh %s %s %s", CRT_CONFIG_PATH, tboxModel, sn);
    system(cmd);

    memset(buf, 0x0, sizeof(buf));

    FILE *fp = NULL;

    gchar filepath[CRT_ELEMENT_MAX] = {
        0x0,
    };
    snprintf(filepath, sizeof(filepath), "%s/requeryCert.txt", CRT_CONFIG_PATH);
    fp = fopen(filepath, "r");
    if (fp == NULL)
    {
        LOG_Debug("open %s fail!", filepath);
        return;
    }

    gchar line[128] = {
        0x0,
    };
    gint len = 0;
    while (1)
    {
        memset(line, 0x0, sizeof(line));
        if (fgets(line, sizeof(line), fp) == NULL)
            break;
        strcpy(&buf[len], line);
        len += strlen(line);
    }

    LOG_Debug("%s", buf);
    fclose(fp);
    CrtMsg_t msg;
    msg.msgtype = E_CREATE_CSR;
    msg.length = len;
    //msg.length = strlen(buf);
    msg.checksum = checkSum(buf, msg.length);
    write(connfd, &msg, sizeof(msg));
    write(connfd, buf, msg.length);
}

static void RecvCert(gint msgtype, gint connfd, gchar payload[], gint length)
{
    gchar filepath[CRT_ELEMENT_MAX] = {
        0x0,
    };
    FILE *fp;
  
    if (E_RECV_ROOTCERT == msgtype)
    {
        snprintf(filepath, sizeof(filepath), "%s/ca.crt", CRT_CONFIG_PATH);
    }
    else
    {
        snprintf(filepath, sizeof(filepath), "%s/tbox.sirun.test.001.crt", CRT_CONFIG_PATH);
    }
    if ((fp = fopen(filepath, "w")) == NULL)
    {
        LOG_Error("%s", "can not open file");
        return;
    }

    if (fwrite(payload, length, 1, fp) != 1)
    {
        LOG_Error("write error:end");
        fclose(fp);
        return;
    }
    fclose(fp);
    CrtMsg_t msg;
    msg.msgtype = msgtype;
    msg.length = strlen("success");
    msg.checksum = checkSum("success", msg.length);
    write(connfd, &msg, sizeof(msg));
    write(connfd, "success", msg.length);
    sleep(1);
}

static void FinishCert(gint connfd)
{
    LOG_Debug("-----%s----%d--,", __FUNCTION__, __LINE__);
    gchar keymd5[256] = {0x0,};
    gchar csrmd5[256] = {0x0,};
    system("openssl rsa -noout -modulus -in /usrdata/status/ca_bak/tbox.sirun.test.001.key | openssl md5 > /home/root/keymd5.txt");
    system("openssl x509 -noout -modulus -in /usrdata/status/ca_bak/tbox.sirun.test.001.crt | openssl md5 > /home/root/csrmd5.txt");

    GetCrtFileMd5("/home/root/keymd5.txt", keymd5);
    GetCrtFileMd5("/home/root/csrmd5.txt", csrmd5);

    system("rm -f /home/root/*md5.txt");
    if (strcmp(keymd5, csrmd5))
    {
        LOG_Error("-----csrmd5:%s----len:%d--,", csrmd5, strlen(csrmd5));
        LOG_Error("-----keymd5:%s----len:%d--,", keymd5, strlen(keymd5));
        return;
    }
    
    system("rm /usrdata/status/ca_bak/requeryCert.txt /usrdata/status/ca_bak/tboxcert.csr"); 
    system("chmod 444 /usrdata/status/ca_bak/*");
    system("rm -rf /usrdata/status/ca");
    system("sync");
    system("cp -r /usrdata/status/ca_bak/ /usrdata/status/ca");
    system("sync");
    system("sync");
    CrtMsg_t msg;
    msg.msgtype = E_RECV_FINISH;
    msg.length = strlen("success");
    msg.checksum = checkSum("success", msg.length);
    write(connfd, &msg, sizeof(msg));
    write(connfd, "success", msg.length);
}

static gint ReadN(gint fd, gchar *pBuf, gint len)
{
    gint nleft;
    gint nread;
    gchar *ptr;

    ptr = pBuf;
    nleft = len;
    while (nleft > 0)
    {
        nread = read(fd, ptr, nleft);
        if (nread < 0)
        {
            LOG_Debug("ReadN,errno:%d", errno);
            if (errno == EINTR)
            {
                nread = 0; /* and call read() again */
            }
            else
            {
                return -1;
            }
        }
        else if (nread == 0)
            break; /* EOF */

        nleft -= nread;
        ptr += nread;
    }
    return (len - nleft); /* return >= 0 */
}

gpointer FtyCrtHandle(gpointer pArg)
{
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    gint listenfd, connfd;
    gchar buf[CRT_MSG_BUFFER] = {
        0x0,
    };

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    //bzero(&servaddr, sizeof(servaddr));
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(CRT_SERV_PORT);

    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(listenfd, 20);

    while (1)
    {
        cliaddr_len = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
        if (connfd == -1)
        {
            continue;
        }
        CrtMsg_t msg;
        memset(&msg, 0x0, sizeof(CrtMsg_t));
        if (ReadN(connfd, buf, sizeof(CrtMsg_t)) != sizeof(CrtMsg_t))
        {
            LOG_Debug("read msg fail!");
            close(connfd);
            continue;
        }
        memcpy(&msg, buf, sizeof(CrtMsg_t));
        #if 0
        gint i = 0;
        LOG_Debug("CrtMsg_t:%d", sizeof(CrtMsg_t));
        for (; i < sizeof(CrtMsg_t); i++)
            LOG_Debug(" %x", buf[i]);
        LOG_Debug("");
        if (msg.length > CRT_MSG_BUFFER)
        {
            LOG_Debug("msg length error!");
            close(connfd);
            continue;
        }
        #endif
        if (ReadN(connfd, buf, msg.length) != msg.length)
        {
            LOG_Debug("read payload fail!");
            close(connfd);
            continue;
        }
        #if 0
        i = 0;
        LOG_Debug("msg.length:%d", msg.length);
        for (; i < msg.length; i++)
            LOG_Debug(" %x", buf[i]);
        LOG_Debug("");
        #endif
        if (msg.checksum != checkSum(buf, msg.length))
        {
            LOG_Debug("check sum fail!,%d,%d", msg.checksum, checkSum(buf, msg.length));
            close(connfd);
            continue;
        }
        switch (msg.msgtype)
        {
        case E_CREATE_CSR:
            CreateCsr(connfd, buf, msg.length);
            break;
        case E_RECV_ROOTCERT:
        case E_RECV_COMCERT:
            RecvCert(msg.msgtype, connfd, buf, msg.length);
            break;
        case E_RECV_FINISH:
            FinishCert(connfd);
            break;
        }
        close(connfd);
    }
}

