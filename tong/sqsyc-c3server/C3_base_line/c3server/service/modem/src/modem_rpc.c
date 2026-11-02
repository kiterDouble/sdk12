#include "modem_call.h"
#include "modem_dms.h"
#include "modem_nw.h"
#include "modem_sim.h"
#include "modem_sms.h"
#include "modem_rpc.h"
 
/**
 * @name ModemSrvRpcProcCallback
 * @brief RPC处理回调函数。此函数用于处理来自调制解调器的RPC（远程过程调用）请求，
 *        根据请求的子命令找到对应的处理函数，并执行相应的处理。
 * @param subcmd 子命令标识，用于指定需要执行的具体操作。
 * @param reqData 请求数据，包含了执行操作所需的参数。
 * @param reqLen 请求数据的长度。
 * @param respData 响应数据，用于返回处理结果。
 * @param respLen 响应数据长度的指针，用于返回响应数据的长度。
 * @param userdata 用户自定义数据，可在处理函数中使用。
 * @return 函数无返回值
 */
void ModemSrvRpcProcCallback(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    static ModemSrvSubProc_t procs[E_MODEM_SRV_RPC_SUB_CMD_MAX] = 
    {
        {E_MODEM_SRV_RPC_SUB_CMD_GET_IMSI, ModemSrvGetImsiProc},
        {E_MODEM_SRV_RPC_SUB_CMD_GET_ICCID, ModemSrvGetIccidProc},
        {E_MODEM_SRV_RPC_SUB_CMD_GET_IMEI, ModemsrvGetImeiProc},
        {E_MODEM_SRV_RPC_SUB_CMD_GET_AIRPLANE_MODE, ModemSrvGetAirplaneModeProc},
        {E_MODEM_SRV_RPC_SUB_CMD_SET_AIRPLANE_MODE, ModemSrvSetAirplaneModeProc},
        {E_MODEM_SRV_RPC_SUB_CMD_GET_QOOS_ENABLE, ModemSrvGetQoosEnableProc},
        {E_MODEM_SRV_RPC_SUB_CMD_SET_QOOS_ENABLE, ModemSrvSetQoosEnableProc},
        {E_MODEM_SRV_RPC_SUB_CMD_GET_QOOS_CFG, ModemSrvGetQoosCfgProc},
        {E_MODEM_SRV_RPC_SUB_CMD_SET_QOOS_CFG,ModemSrvSetQoosCfgProc},
        {E_MODEM_SRV_RPC_SUB_CMD_GET_EXT_SIGNAL_INFO, ModemSrvGetExtSignalInfoProc},
        {E_MODEM_SRV_RPC_SUB_CMD_GET_OPERATOR, ModemSrvGetOperatorProc},
        {E_MODEM_SRV_RPC_SUB_CMD_SEND_SMS_MESG, ModemSrvSendSmsMesgProc},
        {E_MODEM_SRV_RPC_SUB_CMD_START_CALL, ModemSrvStartCallProc},
        {E_MODEM_SRV_RPC_SUB_CMD_ACCEPT_CALL, ModemSrvAcceptCallProc},
        {E_MODEM_SRV_RPC_SUB_CMD_END_CALL, ModemSrvStopCallProc},
        {E_MODEM_SRV_RPC_SUB_CMD_DEINIT_JOB, ModemSrvDeinitProc},
        {E_MODEM_SRV_RPC_SUB_CMD_GET_MSISDN, ModemSrvGetMsisdnProc},
        {E_MODEM_SRV_RPC_SUB_CMD_GET_CPU_INFO, ModemSrvGetCpuInfoProc}
    };
    uint32_t i = (uint32_t)E_MODEM_SRV_JOB_TYPE_SIM;

    if (((reqLen > 0U) && !reqData) || !respData || !respLen || !userdata)
    {
        LOG_Error("%s param invalid.%d,%d,%d,%d", __FUNCTION__, !reqData, !respData, !respLen, !userdata);
        return;
    }

    for (; i < (uint32_t)E_MODEM_SRV_RPC_SUB_CMD_MAX; i++)
    {
        if ((ModemSrvRpcSubCmd_e)subcmd == procs[i].sub)
        {
            procs[i].procCb(reqData, reqLen, respData, respLen, userdata);
            return;
        }
    }

    LOG_Error("%s subcmd invalid.", __FUNCTION__);
    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *)((void*)respData);
    res->result = RET_ERR;
    res->dataLen = 0U;
    *respLen = (uint16_t)sizeof(ModemSrvRpcRes_t);
}