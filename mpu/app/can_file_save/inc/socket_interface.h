#ifndef __TCP_SOCKET_H__
#define __TCP_SOCKET_H__

#include <stdio.h>          
#include <stdint.h>         
#include <stdlib.h>         
#include <string.h>       
#include <unistd.h>       
#include <errno.h>         
#include <fcntl.h>          

#include <sys/types.h>
#include <sys/socket.h>    
#include <sys/select.h>     
#include <arpa/inet.h>      
#include <netinet/in.h>  

#ifdef __cplusplus
extern "C" {
#endif

int stream_sndto(int fd, const void *buff, int size, int *len);

int stream_rcvfrom(int fd, void *buff, int size, int *len);

int stream_rcvtmo(int fd, void *buff, int size, int *len, int msec);

int server_connect(const char *addr4, uint16_t port, int *fd, int tmo);

int connect_server(const char *ip, uint16_t port);

int server_create(const char *addr4, uint16_t port, int maxls, int *fd);
#ifdef __cplusplus
}
#endif

#endif