#ifndef __TBOX_UDS_TP_H__
#define __TBOX_UDS_TP_H__

//#include "lib_sirun.h"
#include <glib.h>
//#include "uds_mcu_uart.h"
#include <semaphore.h>
//#include "static_queue.h"


#define TBOX_UDS_TP_RESPONSE_TIMEOUT			 1

#define TBOX_UDS_TP_RESULT_OK                    0
#define TBOX_UDS_TP_RESULT_ERROR_BUS_OFF         -1
#define TBOX_UDS_TP_RESULT_ERROR_PARAMETER       -2
#define TBOX_UDS_TP_RESULT_ERROR_TX              -3
#define TBOX_UDS_TP_RESULT_ERROR_TX_TIMEOUT      -4
#define TBOX_UDS_TP_RESULT_ERROR_TX_FORMAT       -5
#define TBOX_UDS_TP_RESULT_ERROR_RX_FORMAT       -6
#define TBOX_UDS_TP_RESULT_ERROR_RX_TIMEOUT      -7
#define TBOX_UDS_TP_RESULT_ERROR_ECU_TIMEOUT     -8
#define TBOX_UDS_TP_RESULT_ERROR_NOT_OPENED      -9
#define TBOX_UDS_TP_RESULT_ERROR_UNKNOWN         -100
















typedef struct 
{
    guint32 physicalRequestId;
    guint32 responseId;
    guint32 functionalRequestId;
}STRUCT_UDS_TP_CONFIGURE;



class CTboxUdsTp
{
    public:
    CTboxUdsTp();
    ~CTboxUdsTp();
    gint32 Open(guint8 canChannel);
    void Close();
    gint32 ServiceExecute(guint8 addressingMode,guint8 *pTxData,guint32 txLength,guint8 *pRxData,guint32 *pRxLength);

    private:
    gint32 Initialize();
    void UnInitialize();	
    gint32 TransmitUdsService(guint8 counterIndex,guint8 *pRequestData,guint16 requestLength,guint8 *pResponseData,guint32 *pResponseLength);
    gint32 ExcuteUdsSerivceRawData(guint8 *pTxData,guint32 txLength,guint8 *pRxData,guint32 *pRxLength);
    void PrintByteArray(guint8 *pData,guint32 length);
    private:
    gint32 m_mcuServerKeyid;
};







#endif//__UDS_TP_H__
