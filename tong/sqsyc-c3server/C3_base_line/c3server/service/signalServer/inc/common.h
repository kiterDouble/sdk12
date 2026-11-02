/**
 * @file common.h
 * @author jiajiangze (jiangze.jia@sirun.net)
 * @brief 
 * @version 0.1
 * @date 2023-12-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef _SIGNAL_COMMON_H_
#define _SIGNAL_COMMON_H_

#include <glib.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <gio/gio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/time.h>
#include "err_code.h"
#include "sr_log.h"



#define IN  //input param
#define OUT //output param
#define INOUT//input & output param


#ifdef __cplusplus
}
#endif
#endif
