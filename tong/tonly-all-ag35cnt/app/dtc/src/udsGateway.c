/* ========================================================================== */
/*  文件名称  : uds_gateway.c                                                 */
/*  模块名称  : UDS ServiceData 网关（依赖外部 MCU 服务）                       */
/*  版本      : v1.2.0   (+ 0x31 DID-Control via UdsDidControl)               */
/*  作者      : <SIRUN HF / Jaxsen>                                           */
/*  日期      : 2025-09-11                                                     */
/* ========================================================================== */

#include "udsGateway.h"
#include "sr_log.h"
#include <string.h>
#include <stdio.h>
#include <didRegistry.h>

/* --------------------------- 常量集中，避免魔法数字 -------------------------- */
#define UDS_MIN_SERVICE_DATA_LEN          (3u)      /* SID(1)+DID(2)      —— 对 0x22/0x2E 必需
                                                       对 0x31：实际最小为 4 (SID+sub+DID)，
                                                       我们在 0x31 分支里单独校验 */
#define UDS_NEG_STD_LEN                   (3u)      /* 7F SID NRC */
#define UDS_NEG_EXT_LEN                   (5u)      /* 7F SID NRC DID */
#define UDS_POS_READ_BASE                 (3u)      /* 62 DID ... 的固定头部 */
#define UDS_POS_WRITE_LEN                 (3u)      /* 6E DID */

/* --------------------------- 小工具（端序） ---------------------------------- */
static uint16_t BeGetU16(const uint8_t *p)
{
    return (uint16_t)((((uint16_t)p[0]) << 8) | p[1]);
}

static void BePutU16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)((v >> 8) & 0xFFu);
    p[1] = (uint8_t)(v & 0xFFu);
}

/* --------------------------- 打包辅助 --------------------------------------- */
static gboolean PackReadPositive(uint8_t origSid, uint16_t did, const uint8_t *data, uint16_t len,
                                 uint8_t *out, uint32_t cap, uint32_t *outLen)
{
    const uint32_t need = (uint32_t)UDS_POS_READ_BASE + (uint32_t)len;
    if ((out == NULL) || (outLen == NULL) || (cap < need))
    {
        return FALSE;
    }
    out[0] = (uint8_t)(origSid + 0x40u);
    BePutU16(&out[1], did);
    if ((len > 0u) && (data != NULL))
    {
        (void)memcpy(&out[3], data, len);
    }
    *outLen = need;
    return TRUE;
}

static gboolean PackWritePositive(uint8_t origSid, uint16_t did,
                                  uint8_t *out, uint32_t cap, uint32_t *outLen)
{
    if ((out == NULL) || (outLen == NULL) || (cap < (uint32_t)UDS_POS_WRITE_LEN))
    {
        return FALSE;
    }
    out[0] = (uint8_t)(origSid + 0x40u);
    BePutU16(&out[1], did);
    *outLen = (uint32_t)UDS_POS_WRITE_LEN;
    return TRUE;
}

static gboolean PackNegativeStd(uint8_t origSid, uint8_t nrc,
                                uint8_t *out, uint32_t cap, uint32_t *outLen)
{
    if ((out == NULL) || (outLen == NULL) || (cap < (uint32_t)UDS_NEG_STD_LEN))
    {
        return FALSE;
    }
    out[0] = (uint8_t)UdsSidNegativeResponse_e;
    out[1] = origSid;
    out[2] = nrc;
    *outLen = (uint32_t)UDS_NEG_STD_LEN;
    return TRUE;
}

static gboolean PackNegativeExt(uint8_t origSid, uint8_t nrc, uint16_t did,
                                uint8_t *out, uint32_t cap, uint32_t *outLen)
{
    if ((out == NULL) || (outLen == NULL) || (cap < (uint32_t)UDS_NEG_EXT_LEN))
    {
        return FALSE;
    }
    out[0] = (uint8_t)UdsSidNegativeResponse_e;
    out[1] = origSid;
    out[2] = nrc;
    BePutU16(&out[3], did);
    *outLen = (uint32_t)UDS_NEG_EXT_LEN;
    return TRUE;
}

/* ---------------------- 打包辅助（0x31: DID-Control 专用） ------------------ */
/* 0x31 正响应：origSid+0x40 | sub | DID | [status...] */
/* 正响应：origSid+0x40 | sub | DID | [status...] */
static gboolean PackControlPositive(uint8_t origSid, uint8_t sub, uint16_t did,
                                    const uint8_t *data, uint16_t len,
                                    uint8_t *out, uint32_t cap, uint32_t *outLen)
{
    const uint32_t need = 4u + (uint32_t)len;
    if ((out == NULL) || (outLen == NULL) || (cap < need)) { return FALSE; }
    out[0] = (uint8_t)(origSid + 0x40u);
    out[1] = sub;
    BePutU16(&out[2], did);
    if ((len > 0u) && (data != NULL)) { (void)memcpy(&out[4], data, len); }
    *outLen = need;
    return TRUE;
}

/* 小助手：按配置回扩展或标准否定，已知 DID 时可带 DID */
static gboolean PackNegWithOptDid(uint8_t sid, uint8_t nrc, gboolean useExtWithDid,
                                  gboolean haveDid, uint16_t did,
                                  uint8_t *out, uint32_t cap, uint32_t *outLen)
{
    if (haveDid && (useExtWithDid != FALSE))
    {
        return PackNegativeExt(sid, nrc, did, out, cap, outLen);
    }
    return PackNegativeStd(sid, nrc, out, cap, outLen);
}

/* ------------------- 0x22: ReadDataByIdentifier 处理 ------------------- */
static UdsRet_e HandleReadByDid(uint8_t sid,
                                const uint8_t *reqSd, uint32_t reqLen,
                                uint8_t *respSd, uint32_t respCap, uint32_t *outRespLen,
                                gboolean useExtNegWithDid)
{
    if (reqLen != (uint32_t)UDS_MIN_SERVICE_DATA_LEN)
    {
        uint16_t did = (reqLen >= 3u) ? BeGetU16(&reqSd[1]) : 0u;

        LOG_Warn("HandleReadByDid: bad reqLen=%lu (expect=%u), DID=0x%04X",
                 (unsigned long)reqLen, (unsigned)UDS_MIN_SERVICE_DATA_LEN, (unsigned)did);
        if (reqSd != NULL)
        {
            uint32_t n = (reqLen < 64u) ? reqLen : 64u;
            LOG_DebugHex(reqSd, (uint16_t)n, "RBD BAD-LEN req");
        }

        (void)PackNegWithOptDid(sid, (uint8_t)NRC_INCORRECT_MESSAGE_LENGTH_E,
                                useExtNegWithDid, (reqLen >= 3u), did,
                                respSd, respCap, outRespLen);
        return UdsRetOk_e;
    }

    const uint16_t did = BeGetU16(&reqSd[1]);
    uint8_t  tmp[1024u];
    uint16_t len = (uint16_t)sizeof tmp;
    UdsNrc_e nrc = UdsDidRead((UDS_DID_IDS_e)did, tmp, &len);

    if (nrc == NRC_NONE_E)
    {
        if (PackReadPositive(sid, did, tmp, len, respSd, respCap, outRespLen) == FALSE)
        {
            LOG_Error("HandleReadByDid: PackReadPositive failed DID=0x%04X len=%u",
                      (unsigned)did, (unsigned)len);

            if (PackNegWithOptDid(sid, (uint8_t)NRC_RESPONSE_TOO_LONG_E,
                                  useExtNegWithDid, TRUE, did,
                                  respSd, respCap, outRespLen) == FALSE)
            {
                LOG_Error("HandleReadByDid: PackNegWithOptDid failed after POS fail DID=0x%04X",
                          (unsigned)did);
                return UdsRetEbuf_e;
            }
        }
    }
    else
    {
        LOG_Warn("HandleReadByDid: UdsDidRead failed DID=0x%04X NRC=0x%02X",
                 (unsigned)did, (unsigned)nrc);
        (void)PackNegWithOptDid(sid, (uint8_t)nrc, useExtNegWithDid, TRUE, did,
                                respSd, respCap, outRespLen);
    }
    return UdsRetOk_e;
}

/* ------------------- 0x2E: WriteDataByIdentifier 处理 ------------------- */
static UdsRet_e HandleWriteByDid(uint8_t sid,
                                 const uint8_t *reqSd, uint32_t reqLen,
                                 uint8_t *respSd, uint32_t respCap, uint32_t *outRespLen,
                                 gboolean useExtNegWithDid)
{
    if (reqLen <= (uint32_t)UDS_MIN_SERVICE_DATA_LEN)
    {
        uint16_t did2 = (reqLen >= 3u) ? BeGetU16(&reqSd[1]) : 0u;

        LOG_Warn("HandleWriteByDid: bad reqLen=%lu (min>%u), DID=0x%04X",
                 (unsigned long)reqLen, (unsigned)UDS_MIN_SERVICE_DATA_LEN, (unsigned)did2);
        if (reqSd != NULL)
        {
            uint32_t n = (reqLen < 64u) ? reqLen : 64u;
            LOG_DebugHex(reqSd, (uint16_t)n, "WBD BAD-LEN req");
        }
        (void)PackNegWithOptDid(sid, (uint8_t)NRC_INCORRECT_MESSAGE_LENGTH_E,
                                useExtNegWithDid, (reqLen >= 3u), did2,
                                respSd, respCap, outRespLen);
        return UdsRetOk_e;
    }

    const uint16_t did = BeGetU16(&reqSd[1]);
    const uint8_t *data = &reqSd[3];
    const uint16_t dlen = (uint16_t)(reqLen - (uint32_t)UDS_MIN_SERVICE_DATA_LEN);

    UdsNrc_e nrc = UdsDidWrite((UDS_DID_IDS_e)did, data, dlen);
#if 0
    if ((UDS_DID_IDS_e)did == DID_MANUFACTORY_MODE)
    {
        if (data[0] >= 0x00 && data[0] <= 0x0F) // 退出工厂模式
        {
            // 0xb2cd 需要写成0x18
            const uint8_t value[2] = {0x18, 0x00};
            (void)UdsDidWrite(DID_OTA_SW_VER_ACQUIRE_TIME, value, 1);
        }
    }
#endif
    if (nrc == NRC_NONE_E)
    {
        if (PackWritePositive(sid, did, respSd, respCap, outRespLen) == FALSE)
        {
            LOG_Error("HandleWriteByDid: PackWritePositive failed DID=0x%04X", (unsigned)did);

            if (PackNegWithOptDid(sid, (uint8_t)NRC_RESPONSE_TOO_LONG_E,
                                  useExtNegWithDid, TRUE, did,
                                  respSd, respCap, outRespLen) == FALSE)
            {
                LOG_Error("HandleWriteByDid: PackNegWithOptDid failed after POS fail DID=0x%04X", (unsigned)did);
                return UdsRetEbuf_e;
            }
        }
    }
    else
    {
        LOG_Warn("HandleWriteByDid: UdsDidWrite failed DID=0x%04X NRC=0x%02X len=%u",
                 (unsigned)did, (unsigned)nrc, (unsigned)dlen);
        if (data != NULL && dlen > 0u)
        {
            uint16_t n = (dlen < 64u) ? dlen : 64u;
            LOG_DebugHex(data, n, "WBD data");
        }


        (void)PackNegWithOptDid(sid, (uint8_t)nrc, useExtNegWithDid, TRUE, did,
                                respSd, respCap, outRespLen);
    }
    return UdsRetOk_e;
}

/* ------------------- 0x31: RoutineControl (DID-Control) 处理 ------------- */
/* 业务入口：UdsDidControl(did, in, inLen, out, &outLen, &retStatus) */
static UdsRet_e HandleRoutineControl(uint8_t sid,
                                     const uint8_t *reqSd, uint32_t reqLen,
                                     uint8_t *respSd, uint32_t respCap, uint32_t *outRespLen,
                                     gboolean useExtNegWithDid)
{
    /* 最小：SID(1)+Sub(1)+DID(2) */
    if (reqLen < 4u)
    {
        LOG_Warn("HandleRoutineControl: req too short (%lu < 4)", (unsigned long)reqLen);
        if (reqSd != NULL)
        {
            uint32_t n = (reqLen < 64u) ? reqLen : 64u;
            LOG_DebugHex(reqSd, (uint16_t)n, "RC BAD-LEN req");
        }

        /* 未解析出 DID，不能带 DID → 标准否定 */
        if (PackNegativeStd(sid, (uint8_t)NRC_INCORRECT_MESSAGE_LENGTH_E,
                            respSd, respCap, outRespLen) == FALSE)
        {
            LOG_Error("HandleRoutineControl: PackNegativeStd(0x13) failed");
            return UdsRetEbuf_e;
        }
        return UdsRetOk_e;
    }

    const uint8_t  sub = reqSd[1];           /* 0x01/0x02/0x03 */
    const uint16_t did = BeGetU16(&reqSd[2]);
    const uint8_t *inPtr = (reqLen > 4u) ? (&reqSd[4]) : NULL;
    const uint16_t inLen = (uint16_t)((reqLen > 4u) ? (reqLen - 4u) : 0u);

    /* 限定子功能（按需放宽） */
    if (!((sub == 0x01u) || (sub == 0x02u) || (sub == 0x03u)))
    {
        LOG_Warn("HandleRoutineControl: unsupported sub=0x%02X DID=0x%04X", sub, did);
        (void)PackNegWithOptDid(sid, (uint8_t)NRC_SUBFUNCTION_NOT_SUPPORTED_E,
                                useExtNegWithDid, TRUE, did,
                                respSd, respCap, outRespLen);
        return UdsRetOk_e;
    }

    /* 业务控制（不传 subFunc 到业务层） */
    uint8_t  tmp[1024u];
    uint16_t outLen = (uint16_t)sizeof tmp;
    gboolean retStatus = FALSE;

    {
        /* 如果你的 UdsDidControl 签名还包含 subFunc，请按你的声明调用 */
        UdsNrc_e nrc = UdsDidControl((UDS_DID_IDS_e)did, sub, inPtr, inLen, tmp, &outLen, &retStatus);
        LOG_Debug("outlen:%d", outLen);
        LOG_DebugHex(tmp, outLen, "RC data");
        if (nrc != NRC_NONE_E)
        {
            LOG_Warn("HandleRoutineControl: UdsDidControl DID=0x%04X sub=0x%02X -> NRC=0x%02X",
                     did, sub, (unsigned)nrc);
            (void)PackNegWithOptDid(sid, (uint8_t)nrc, useExtNegWithDid, TRUE, did,
                                    respSd, respCap, outRespLen);
            return UdsRetOk_e;
        }
    }

    /* 结果打包：0x03 必须带；0x01/0x02 可选带 */
    if (sub == 0x03u)
    {
        if ((retStatus == FALSE) || (outLen == 0u))
        {
            LOG_Warn("HandleRoutineControl: RC(0x03) fail DID=0x%04X retStatus=%d outLen=%u",
                     did, (int)retStatus, (unsigned)outLen);
            (void)PackNegWithOptDid(sid, (uint8_t)NRC_CONDITIONS_NOT_CORRECT_E,
                                    useExtNegWithDid, TRUE, did,
                                    respSd, respCap, outRespLen);
            return UdsRetOk_e;
        }

        if (PackControlPositive(sid, sub, did, tmp, outLen, respSd, respCap, outRespLen) == FALSE)
        {
            LOG_Error("HandleRoutineControl: PackControlPositive fail (resp too long) DID=0x%04X len=%u",
                      did, (unsigned)outLen);
            (void)PackNegWithOptDid(sid, (uint8_t)NRC_RESPONSE_TOO_LONG_E,
                                    useExtNegWithDid, TRUE, did,
                                    respSd, respCap, outRespLen);
        }
    }
    else
    {
        const uint16_t payloadLen = (retStatus != FALSE) ? outLen : 0u;
        LOG_Debug("payloadLen=%u, cap:%u, retStatus=%d, outLen=%u", (unsigned)payloadLen, respCap, (int)retStatus, (unsigned)outLen);
        if (PackControlPositive(sid, sub, did,
                                (payloadLen > 0u) ? tmp : NULL, payloadLen,
                                respSd, respCap, outRespLen) == FALSE)
        {
            LOG_Error("HandleRoutineControl: PackControlPositive fail (resp too long) DID=0x%04X len=%u",
                      did, (unsigned)payloadLen);
            (void)PackNegWithOptDid(sid, (uint8_t)NRC_RESPONSE_TOO_LONG_E,
                                    useExtNegWithDid, TRUE, did,
                                    respSd, respCap, outRespLen);
        }
    }

    return UdsRetOk_e;
}

/* --------------------------- 主分发：ExecServiceData --------------------------- */
static UdsRet_e ExecServiceData(const uint8_t *reqSd, uint32_t reqLen,
                                uint8_t *respSd, uint32_t respCap, uint32_t *outRespLen,
                                gboolean useExtNegWithDid)
{
    if ((reqSd == NULL) || (respSd == NULL) || (outRespLen == NULL))
    {
        LOG_Error("ExecServiceData: invalid args reqSd=%p respSd=%p outRespLen=%p",
                  (const void*)reqSd, (void*)respSd, (void*)outRespLen);
        return UdsRetEinval_e;
    }

    /* 基础兜底（0x31 的更严格长度在其分支判断） */
    if (reqLen < (uint32_t)UDS_MIN_SERVICE_DATA_LEN)
    {
        LOG_Warn("ExecServiceData: reqLen too short (%lu, min=%u)",
                 (unsigned long)reqLen, (unsigned)UDS_MIN_SERVICE_DATA_LEN);
        if (PackNegativeStd(0x00u, (uint8_t)NRC_INCORRECT_MESSAGE_LENGTH_E,
                            respSd, respCap, outRespLen) == FALSE)
        {
            LOG_Error("ExecServiceData: PackNegativeStd failed (0x13)");
            return UdsRetEbuf_e;
        }
        return UdsRetOk_e;
    }

    const uint8_t sid = reqSd[0];

    if (sid == (uint8_t)UdsSidReadRequest_e)
    {
        return HandleReadByDid(sid, reqSd, reqLen, respSd, respCap, outRespLen, useExtNegWithDid);
    }
    else if (sid == (uint8_t)UdsSidWriteRequest_e)
    {
        return HandleWriteByDid(sid, reqSd, reqLen, respSd, respCap, outRespLen, useExtNegWithDid);
    }
    else if (sid == (uint8_t)UdsSidRoutineControlRequest_e)
    {
        return HandleRoutineControl(sid, reqSd, reqLen, respSd, respCap, outRespLen, useExtNegWithDid);
    }
    else
    {
        LOG_Warn("ExecServiceData: unsupported SID=0x%02X", sid);
        if (PackNegativeStd(sid, (uint8_t)NRC_SERVICE_NOT_SUPPORTED_E,
                            respSd, respCap, outRespLen) == FALSE)
        {
            LOG_Error("ExecServiceData: PackNegativeStd failed (0x11)");
            return UdsRetEbuf_e;
        }
        return UdsRetOk_e;
    }
}

/* --------------------------- 对外 API 实现 ---------------------------------- */

void UdsGatewayInit(UdsGatewayHandle_t *handle, const UdsGatewayConfig_t *cfg)
{
    if ((handle == NULL) || (cfg == NULL))
    {
        return;
    }
    handle->cfg = *cfg;
    if (handle->cfg.maxServiceData == 0u)
    {
        handle->cfg.maxServiceData = 1024u;  /* 设定一个稳妥上限 */
    }
}

/**
 * @brief 模拟向 MCU 发送消息 —— 实际这里直接打印
 */
static int32_t McuSrvSendMesgProcess(const McuMesg_t *msg)
{
    uint32_t i;

    if ((msg == NULL) || (msg->payload == NULL))
    {
        return -1;
    }

    LOG_Debug("=== McuSrvSendMesg ===");
    LOG_Debug("Type: %d", msg->type);
    LOG_Debug("Len : %u", (unsigned)msg->payloadLen);
    LOG_Debug("Data: ");
    // uint32_t n = (msg->payloadLen < 64u) ? msg->payloadLen : 64u;
    LOG_DebugHex(msg->payload, (uint16_t)msg->payloadLen, "McuSrvSendMesg payload");
    LOG_Debug("======================");

    (void)McuSrvSendMesg(msg);
    return 0;
}

UdsRet_e UdsGatewayOnMcuMessage(UdsGatewayHandle_t *handle, const McuMesg_t *inMsg)
{
    UdsRet_e  ret     = UdsRetOk_e;
    uint8_t  *respBuf = NULL;
    uint32_t  respLen = 0u;
    McuMesg_t outMsg;

    if ((handle == NULL) || (inMsg == NULL))
    {
        LOG_Error("UdsGW: invalid args handle=%p inMsg=%p", (void*)handle, (void*)inMsg);
        return UdsRetEinval_e;
    }

    /* 非诊断类型直接忽略，不打日志 */
    if (inMsg->type != handle->cfg.reqType)
    {
        return UdsRetOk_e;
    }

    /* 基本边界检查 */
    if ((inMsg->payload == NULL) ||
        (inMsg->payloadLen < (uint32_t)UDS_MIN_SERVICE_DATA_LEN) ||
        (inMsg->payloadLen > handle->cfg.maxServiceData))
    {
        LOG_Warn("UdsGW: bad payload len=%lu (min=%u max=%lu)",
                 (unsigned long)((inMsg->payload != NULL) ? inMsg->payloadLen : 0ul),
                 (unsigned)UDS_MIN_SERVICE_DATA_LEN,
                 (unsigned long)handle->cfg.maxServiceData);

        if (inMsg->payload != NULL)
        {
            uint32_t n = (inMsg->payloadLen < 64u) ? inMsg->payloadLen : 64u;
            LOG_DebugHex(inMsg->payload, (uint16_t)n, "RX BAD-LEN");
        }

        /* 回 7F 0x13 */
        respBuf = (uint8_t *)g_malloc((gsize)UDS_NEG_STD_LEN);
        if (respBuf == NULL)
        {
            LOG_Error("UdsGW: malloc %u failed (neg resp)", (unsigned)UDS_NEG_STD_LEN);
            return UdsRetEbuf_e;
        }

        (void)PackNegativeStd((uint8_t)0x00u,
                              (uint8_t)NRC_INCORRECT_MESSAGE_LENGTH_E,
                              respBuf, (uint32_t)UDS_NEG_STD_LEN, &respLen);

        outMsg.type       = handle->cfg.respType;
        outMsg.payload    = respBuf;
        outMsg.payloadLen = respLen;

        (void)McuSrvSendMesgProcess(&outMsg);
        g_free(respBuf);
        return UdsRetOk_e;
    }

    /* 执行服务 */
    respBuf = (uint8_t *)g_malloc((gsize)handle->cfg.maxServiceData);
    if (respBuf == NULL)
    {
        LOG_Error("UdsGW: malloc %lu failed (respBuf)",
                  (unsigned long)handle->cfg.maxServiceData);
        return UdsRetEbuf_e;
    }

    ret = ExecServiceData(inMsg->payload, inMsg->payloadLen,
                          respBuf, handle->cfg.maxServiceData, &respLen,
                          handle->cfg.useExtendedNegWithDid);

    if (ret == UdsRetOk_e)
    {
        outMsg.type       = handle->cfg.respType;
        outMsg.payload    = respBuf;
        outMsg.payloadLen = respLen;
        (void)McuSrvSendMesgProcess(&outMsg);
    }
    else
    {
        LOG_Error("UdsGW: ExecServiceData failed, ret=%d", (int)ret);
        if ((respBuf != NULL) && (respLen > 0u))
        {
            uint32_t n = (respLen < 64u) ? respLen : 64u;
            LOG_DebugHex(respBuf, (uint16_t)n, "TX PARTIAL/ERR");
        }
    }

    g_free(respBuf);
    return ret;
}


