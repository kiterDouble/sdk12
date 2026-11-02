#ifndef CANTPFORFOTA_H
#define CANTPFORFOTA_H

/*******************************************************************************
*                              外部接口说明                                   *
********************************************************************************
*
* 本库提供CAN传输协议(CAN Transport Protocol)功能，支持ISO 15765-2标准，
* 实现长CAN消息的分段传输。
*
* 主要功能:
* 1. 初始化和去初始化CAN TP模块
* 2. 发送长CAN消息（超过8字节的数据）
* 3. 接收长CAN消息
*
* 使用方法:
* 1. 调用 CanTp_Init 初始化模块并注册回调函数
* 2. 调用 CanTp_Transmit 发送数据
* 3. 在接收回调中处理接收到的数据
* 4. 应用结束时调用 CanTp_Close 关闭模块
*
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char      		boolean;
typedef signed char         	sint8;
typedef unsigned char       	uint8;
typedef unsigned char			utf8;
typedef char				    char_t;
typedef signed short            sint16;
typedef unsigned short     		uint16;
typedef unsigned short   		ucs2;
typedef signed int          	sint32;
typedef unsigned int       		uint32;
typedef long long 		    	sint64;
typedef unsigned long long		uint64;
typedef float                   float32; 
typedef double                  float64;  


typedef uint8 Std_ReturnType;
#define E_OK                           0x00
#define E_NOT_OK                       0x01



typedef uint8 CanTp_NotifResultType;
#define NTFRSLT_OK				       0x00
#define NTFRSLT_E_NOT_OK		       0x01
#define NTFRSLT_E_TIMEOUT_A		       0x02
#define NTFRSLT_E_TIMEOUT_BS 	       0x03
#define NTFRSLT_E_TIMEOUT_CR	       0x04
#define NTFRSLT_E_WRONG_SN 		       0x05
#define NTFRSLT_E_INVALID_FS	       0x06
#define NTFRSLT_E_UNEXP_PDU		       0x07
#define NTFRSLT_E_WFT_OVRN		       0x08
#define NTFRSLT_E_ABORT			       0x09
#define NTFRSLT_E_NO_BUFFER 	       0x0A
#define NTFRSLT_E_CANCELATION_OK       0x0B
#define NTFRSLT_E_CANCELATION_NOT_OK   0x0C
#define NTFRSLT_PARAMETER_OK           0x0D
#define NTFRSLT_E_PARAMETER_NOT_OK     0x0E
#define NTFRSLT_E_RX_ON                0x0F
#define NTFRSLT_E_VALUE_NOT_OK         0x10

#define NTFRSLT_E_FR_ML_MISMATCH       0x5B
#define NTFRSLT_E_FR_WRONG_BP          0x5C
#define NTFRSLT_E_FR_TX_ON             0x5D


typedef struct 
{
    uint8 *SduDataPtr;			// payload
    uint16 SduLength;	// length of SDU
} PduInfoType;



/**
 * @brief 发送错误回调函数指针类型
 * @param result 错误结果代码
 */
typedef void (* CanTp_TransmitErrorCbk_t)(CanTp_NotifResultType result);

/**
 * @brief 接收指示回调函数指针类型
 * @param canId CAN标识符
 * @param pduInfoPtr 接收到的数据信息指针
 */
typedef void (* CanTp_ReceiveIndicationCbk_t)(uint32 canId, PduInfoType *pduInfoPtr);


/*************************************** Service called by upper layer *****************************************/

/*****************************************************************************
 * Prototype    : CanTp_Init
 * Description  : 初始化CAN TP模块，配置接收数据处理回调和CAN帧发送错误信息回调
 * Parameters   : rxIndicationCbk  接收指示回调函数指针
 *                transmitErrCbk   发送错误回调函数指针
 * Return Value : Std_ReturnType   E_OK: 成功, E_NOT_OK: 失败
 * Calls        : 
 * Called By    : 上层模块，如远程诊断模块或FOTA更新代理模块
 *****************************************************************************/
Std_ReturnType CanTp_Init(CanTp_ReceiveIndicationCbk_t rxIndicationCbk, 
                                CanTp_TransmitErrorCbk_t transmitErrCbk);


/*****************************************************************************
 * Prototype    : CanTp_Transmit
 * Description  : 通过CAN TP通道发送CAN诊断帧
 * Parameters   : canId            CAN标识符
 *                pduInfoPtr      要发送的数据信息指针
 * Return Value : Std_ReturnType   E_OK: 成功, E_NOT_OK: 失败
 * Calls        : 
 * Called By    : 上层模块，如远程诊断模块或FOTA更新代理模块
 *****************************************************************************/
Std_ReturnType CanTp_Transmit(uint32 canId, PduInfoType *pduInfoPtr);


/*****************************************************************************
 * Prototype    : CanTp_Close
 * Description  : 关闭CAN TP通道
 * Parameters   : 无
 * Return Value : Std_ReturnType   E_OK: 成功, E_NOT_OK: 失败
 * Calls        : 
 * Called By    : 上层模块，如远程诊断模块或FOTA更新代理模块
 *****************************************************************************/
Std_ReturnType CanTp_Close();

#ifdef __cplusplus
}
#endif

#endif  /* End of CANTPFORFOTA_H */