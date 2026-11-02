#ifndef _IPC_STRUCTS_H
#define _IPC_STRUCTS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

    //pubdemo subdemo
    typedef struct
    {
        char unslicitedData[50];
        char smsData[30];
    } urcBoardcast_t;

    //rpcServerDemo rpcServerdemo
    typedef struct
    {
        char cmd[30];
        char prefix[30];
        int32_t responseLines;
    } SendAtCmd_req_t;
    typedef struct
    {
        int32_t cmdResult;
        char response[30];
    } SendAtCmd_resp_t;
    #define RPCSERVERDEMO_MSGTYPE_SUBCMD_ID1 (0)//rpcserverdemo 对外提供的接口1id
    #define RPCSERVERDEMO_MSGTYPE_SUBCMD_ID2 (1)//rpcserverdemo 对外提供的接口2id

#ifdef __cplusplus
}
#endif

#endif
