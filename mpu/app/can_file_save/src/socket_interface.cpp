#include "socket_interface.h"
#include <netinet/tcp.h>
#include "sr_log.h"
int stream_sndto(int fd, const void *buff, int size, int *len)
{
    uint8_t *value = (uint8_t *)buff;
    int n, cnt, status;
    cnt = 0;
    *len = 0;
    status = 0;

    while (size)
    {

        n = send(fd, value, size, MSG_NOSIGNAL);
        /* 发送失败 */
        if (0 == n)
        {
            status = -1;
            break;
        }
        /* 发送中断 */
        else if (-1 == n)
        {
            if ((errno == EINTR) || (errno == EAGAIN))
            {
                continue;
            }
            else
            {
                status = -1;
                break;
            }
        }
        /* 成功发送 */
        else
        {
            cnt += n;
            value += n;
            size -= n;
        }
    }

    *len = cnt;
    return status;
}

int stream_rcvfrom(int fd, void *buff, int size, int *len)
{
    uint8_t *pkg = (uint8_t *)buff;
    int n, cnt, status;

    cnt = 0;
    *len = 0;
    status = 0;

    /* 接收数据 */
    while (size)
    {
        n = recv(fd, pkg, size, 0);

        /* 对方关闭套接字 */
        if (0 == n)
        {
            status = -1;
            break;
        }
        /* 接收出错 */
        else if (-1 == n)
        {
            /* 缓冲区为空 */
            if (errno == EAGAIN)
            {
                status = -2;
                break;
            }
            else
            {
                continue;
            }
        }
        /* 读到了有效的数据 */
        else
        {
            cnt += n;
            pkg += n;
            size -= n;
        }
    }

    *len = cnt;
    return status;
}

int stream_rcvtmo(int fd, void *buff, int size, int *len, int msec)
{
    uint8_t *pkg = (uint8_t *)buff;
    int ret = 0;
    fd_set rset;
    struct timeval tv;
    *len = 0;
    FD_ZERO(&rset);
    FD_SET(fd, &rset);

    tv.tv_sec = msec / 1000;
    tv.tv_usec = (msec % 1000) * 1000;

    ret = select(fd + 1, &rset, NULL, NULL, &tv);
    if (ret == 0)
    {
        return -1;
    }
    else if (-1 == ret)
    {
        return -1;
    }

    if (FD_ISSET(fd, &rset))
        return stream_rcvfrom(fd, buff, size, len);

    return -1;
}

int server_connect(const char *addr4, uint16_t port, int *fd, int tmo)
{
    int flag = 1;
    int sock = 0, ret = 0;
    int error = 0;
    socklen_t len = sizeof(error);
    struct sockaddr_in svraddr;
    socklen_t sklen = sizeof(svraddr);
    fd_set rset, wset;
    struct timeval tv;

    memset(&svraddr, 0, sizeof(svraddr));

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock <= 0)
    {
        LOG_Error("create socket err,%d:%s", errno, strerror(errno));
        return -1;
    }

    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    svraddr.sin_addr.s_addr = inet_addr(addr4);
    int bufsize = 256 * 1024;  // 256KB 或更大
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
    ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, len);
    if (ret != 0)
    {
        LOG_Error("setsockopt error,%d:%s\n", errno, strerror(errno));
        close(sock);
        return -1;
    }

    fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK);

    ret = connect(sock, (struct sockaddr *)&svraddr, sklen);
    if (ret < 0)
    {
        if (errno != EINPROGRESS)
        {
            close(sock);
            LOG_Error("%s,%d, n=%d,error=%s\n", __func__, __LINE__, ret, strerror(errno));
            return -1;
        }
    }
    else if (ret == 0)
    {
    }

    if (tmo <= 0)
    {
        *fd = sock;
        return 0;
    }

    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_SET(sock, &rset);
    FD_SET(sock, &wset);

    tv.tv_sec = tmo / 1000;
    tv.tv_usec = (tmo % 1000) * 1000;

    ret = select(sock + 1, &rset, &wset, NULL, &tv);
    if (ret == 0)
    {
        close(sock);
        LOG_Error("ETIMEDOUT error\n");
        return -1;
    }

    if (FD_ISSET(sock, &rset) || FD_ISSET(sock, &wset))
    {
        len = sizeof(error);
        if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
        {
            close(sock);
            return -1;
        }

        if (0 != error)
        {
            close(sock);
            LOG_Error("getsockopt error = %d\n", error);
            return -1;
        }
    }
    else
    {
        close(sock);
        return -1;
    }

    *fd = sock;
    return 0;
}

int connect_server(const char *ip, uint16_t port)
{
    int fd;
    LOG_Debug("ip %s\n", ip);
    LOG_Debug("port %d\n", port);

    if (server_connect(ip, port, &fd, 1000) != 0)
    {
        LOG_Error("Failed to connect\n");
        return -1;
    }
    return fd;
}

int server_create(const char *addr4, uint16_t port, int maxls, int *fd)
{
    int sock, ret, on = 1;
    int nlen = sizeof(int), nflag = 1;
    struct sockaddr_in svraddr;

    memset(&svraddr, 0, sizeof(svraddr));

    /* 创建套接字 */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        return -1;
    }

    memset(&svraddr, 0, sizeof(svraddr));
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);

    if ((addr4 == NULL) || (0 == *addr4))
        svraddr.sin_addr.s_addr = INADDR_ANY;
    else
        svraddr.sin_addr.s_addr = inet_addr(addr4);

    /* 设置为端口复用 */
    ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &nflag, nlen);
    if (ret < 0)
    {
        LOG_Error("setsockopt SO_REUSEADDR error:%s\n", strerror(errno));
        close(sock);
        return -1;
    }
    // setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (void *)&on, sizeof(on));
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
    int bufsize = 256 * 1024;  // 256KB 或更大
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
    fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK);

    if (bind(sock, (struct sockaddr *)&svraddr, sizeof(struct sockaddr_in)) < 0)
    {
        close(sock);
        LOG_Error("server bind fail\n");
        return -1;
    }

    if (listen(sock, maxls) < 0)
    {
        close(sock);
        LOG_Error("server listen fail\n");
        return -1;
    }

    LOG_Error("Server listen addr[%s] port[%d] success\n", addr4, port);

    *fd = sock;

    return 0;
}