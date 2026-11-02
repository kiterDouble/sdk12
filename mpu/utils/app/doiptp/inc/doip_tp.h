#ifndef __DOIP_TP_H__
#define __DOIP_TP_H__
#include "data_type.h"


#ifdef __cplusplus
extern "C"{
#endif
#define MAX_BUFFER_LENGTH (0x30d44) //4096
typedef struct
{
    uint32		payloadLength; 
    uint8		payloadBuf[MAX_BUFFER_LENGTH]; 
    uint8		reqType; 
}ethtpData;

typedef struct
{
    uint8  targetIpAddr[16];
    uint16  functionalLa;
    uint16  sourceLa; 
    uint16  targetLa;
}ethtpNode;

typedef void (*ethtpConNotifyCb)(uint16 targetLa, int8 notification, uint8 errcode);
typedef void (*ethtpTxConfirmCb)(uint16 targetLa, ethtpData* data, uint8 errcode); 
typedef	void (*ethtpRxIndicationCb)(uint16 targetLa, ethtpData* data, uint8 errcode); 
typedef	void (*ethtpOnlineNotifyCb)(uint16 targetLa); 

typedef struct {
    ethtpConNotifyCb        conFunc;
    ethtpTxConfirmCb        txFunc; 
    ethtpRxIndicationCb     rxFunc;
    ethtpOnlineNotifyCb     onlineFunc;
} ethtpCallbacks;

uint8 ethtp_init(ethtpCallbacks* cbs);
uint8 ethtp_connect_node(ethtpNode* node);
uint8 ethtp_transmit(ethtpNode* node, ethtpData* data);
uint8 ethtp_disconnect_node(ethtpNode* node);

#ifdef __cplusplus
}
#endif
#endif
