#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <cstring>
#include <ctime>
#include <exception>
#include "uds_service.h"
#include "ota_master_flash_info.h"
#include "ota_master_fota_ecu.h"
#include "ota_master_log.h"
#include "ota_master_error_code.h"
#include "ota_master_security_algorithm.h"
#include "ota_master_utils.h"
#include "ota_master_ecu_brush.h"

typedef struct {
    uint32_t errorCode;
    uint32_t keepExit;
    bool suppressResponse;  // TesterPresent是否抑制响应: true=3E 80(不需要响应), false=3E 00(需要响应)
    UdsSrvHandle_t srvHdl;
    FotaEcuInfo_t info;
    std::thread keepSessionThread;
} FotaEcuPriv_t;

static int FotaEcuSetErrorCode(FotaEcuPriv_t *priv, const uint32_t &errorCode)
{
    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    priv->errorCode = errorCode;
    return OTA_RET_OK;
}

// ==================== VCU刷写重试参数 ====================
// 两级重试:
//   1) 单条指令"超时无响应"(ERROR_TP) → 同一指令重发, 最多 VCU_CMD_RETRY_TIMES 次;
//   2) "拒绝响应"(负响应/格式错) 或 重发3次仍超时 → 整段从步骤1(10 01)重新执行, 最多 VCU_PREPROG_TRY_TIMES 次。
#define VCU_PREPROG_TRY_TIMES   (3)     // VCU预编程阶段(步骤1~7含10 02)整段重试次数
#define VCU_CMD_RETRY_TIMES         (3)   // 单条指令"超时无响应"时的重发次数
#define VCU_CMD_RETRY_INTERVAL_MS   (200) // 单条指令重发间隔(ms)

/**
 * @brief VCU刷写单条指令执行: 仅在"超时无响应"(ERROR_TP)时重发同一指令,
 *        最多 VCU_CMD_RETRY_TIMES 次; 负响应/格式错不重发, 直接返回(由外层整段重试处理)。
 *        若"超时重发后恢复正常应答", 额外补发一次 3E 00(保持会话, 失败不影响本步骤结果)。
 * @param priv ECU私有数据结构指针
 * @param stepName 步骤名(日志用)
 * @param step 步骤执行体, 返回 UDS_SERVICE_RESULT_*
 * @return 最后一次执行结果
 */
template <typename StepFunc>
static int VcuResendOnTimeout(FotaEcuPriv_t *priv, const char *stepName, StepFunc &&step)
{
    int res = step();
    bool retried = false;
    for (int retry = 1; (retry <= VCU_CMD_RETRY_TIMES) && (res == UDS_SERVICE_RESULT_ERROR_TP); retry++) {
        OTA_LOG_W("VCU [%s] no response, resend (%d/%d).\n", stepName, retry, VCU_CMD_RETRY_TIMES);
        usleep(VCU_CMD_RETRY_INTERVAL_MS * 1000);
        retried = true;
        res = step();
    }
    // 超时重发后恢复正常应答 → 补发一次 3E 00
    if (retried && (res == UDS_SERVICE_RESULT_OK)) {
        int tpRes = TesterPresentReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, false);
        OTA_LOG_I("VCU [%s] recovered after resend, extra 3E 00 result=%d.\n", stepName, tpRes);
    }
    return res;
}

static void MergeContiguousFlashSegments(Flashinfo_t *info);

/**
 * @brief 检查访问安全性级别1
 * @param priv ECU私有数据结构指针
 * @return OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
static int CheckAccessSecurity1(FotaEcuPriv_t *priv)
{
    uint8_t data[128] = {
        0x0,
    };
    uint32_t len = 0;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 步骤2  0x27 0x01
    if (UDS_SERVICE_RESULT_OK !=
        SecurityAccessReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x01, data, &len)) {
        OTA_LOG_E("%s 0x27 0x01 failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    if (len < 1) {
        OTA_LOG_E("%s invalid seed length.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    uint32_t seed = 0;
    for (uint32_t i = 0; i < len; i++) {
        seed = (seed << 8) | data[i];
    }

    uint32_t key = static_cast<uint32_t>(UdsCalcKeyLv1(seed, priv->info.phyId));

    for (uint32_t i = 0; i < len; i++) {
        data[i] = (key >> ((len - 1 - i) * 8)) & 0xff;
    }

    // 步骤3  0x27 0x02
    if (UDS_SERVICE_RESULT_OK !=
        SecurityAccessReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x02, data, &len)) {
        OTA_LOG_E("%s 0x27 0x02 failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    return OTA_RET_OK;
}

/**
 * @brief 检查访问安全性级别2 (0x27 03 - 0x27 04)
 * @param priv ECU私有数据结构指针
 * @return OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
static int CheckAccessSecurity2(FotaEcuPriv_t *priv)
{
    uint8_t data[128] = {
        0x0,
    };
    uint32_t len = 0;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 步骤: 0x27 0x03 请求种子
    if (UDS_SERVICE_RESULT_OK !=
        SecurityAccessReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x03, data, &len)) {
        OTA_LOG_E("%s 0x27 0x03 failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_I("%s 27 03 response: len=%d, data=[%02X %02X %02X %02X]\n", __FUNCTION__, len,
              data[0], data[1], data[2], data[3]);

    if (len < 1 or len > 4) {
        OTA_LOG_E("%s invalid seed length %d.\n", __FUNCTION__, len);
        return OTA_RET_ERR;
    }

    uint32_t seed = 0;
    for (uint32_t i = 0; i < len; i++) {
        seed = (seed << 8) | data[i];
    }

    // VCU密钥算法
    uint32_t key = static_cast<uint32_t>(UdsCalcKeyVcu(seed, 0));
    OTA_LOG_D("%s VCU key calculation, seed=0x%08X, key=0x%08X\n",
              __FUNCTION__, seed, key);

    for (uint32_t i = 0; i < len; i++) {
        data[i] = (key >> ((len - 1 - i) * 8)) & 0xff;
    }

    // 步骤: 0x27 0x04 发送密钥
    if (UDS_SERVICE_RESULT_OK !=
        SecurityAccessReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x04, data, &len)) {
        OTA_LOG_E("%s 0x27 0x04 failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    return OTA_RET_OK;
}

/**
 * @brief 检查访问安全性级别3
 * @param priv ECU私有数据结构指针
 * @return OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
static int CheckAccessSecurity3(FotaEcuPriv_t *priv)
{
    uint8_t data[128] = {
        0x0,
    };
    uint32_t len = 0;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 步骤8  0x27 0x05
    if (UDS_SERVICE_RESULT_OK !=
        SecurityAccessReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x05, data, &len)) {
        OTA_LOG_E("%s 0x27 0x01 failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    if (len < 1) {
        OTA_LOG_E("%s invalid seed length.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    uint32_t seed = 0;
    for (uint32_t i = 0; i < len; i++) {
        seed = (seed << 8) | data[i];
    }

    uint32_t key = static_cast<uint32_t>(UdsCalcKeyLv3(seed, priv->info.phyId));

    for (uint32_t i = 0; i < len; i++) {
        data[i] = (key >> ((len - 1 - i) * 8)) & 0xff;
    }

    // 步骤9  0x27 0x06
    if (UDS_SERVICE_RESULT_OK !=
        SecurityAccessReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x06, data, &len)) {
        OTA_LOG_E("%s 0x27 0x02 failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    return OTA_RET_OK;
}

/**
 * @brief 擦除Flash
 * @param priv ECU私有数据结构指针
 * @param startAddr 起始地址
 * @param dataLen 数据长度
 * @return OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
static int EraseFlash(FotaEcuPriv_t *priv, uint32_t startAddr, uint32_t dataLen)
{
    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_I("%s EraseFlash startAddr:0x%08X, dataLen:0x%08X\n", __FUNCTION__, startAddr, dataLen);
    uint8_t data[32];
    uint32_t respLen = 0;

    if ((priv->info.ecuId == ECU_ID_VCU) || (priv->info.ecuId == ECU_ID_MCU1_PDU) ||
        (priv->info.ecuId == ECU_ID_MCU1_ACM) || (priv->info.ecuId == ECU_ID_MCU1_TM1) ||
        (priv->info.ecuId == ECU_ID_MCU1_TM2) || (priv->info.ecuId == ECU_ID_MCU1_EPS) ||
        (priv->info.ecuId == ECU_ID_MCU1_DCDC) || (priv->info.ecuId == ECU_ID_TCU)) {
        // VCU/MCU1系列/TCU擦除例程需要带地址长度格式标识0x44(4字节地址+4字节长度)
        data[0] = 0x44;
        data[1] = (startAddr >> 24) & 0xff;
        data[2] = (startAddr >> 16) & 0xff;
        data[3] = (startAddr >> 8) & 0xff;
        data[4] = (startAddr >> 0) & 0xff;
        data[5] = (dataLen >> 24) & 0xff;
        data[6] = (dataLen >> 16) & 0xff;
        data[7] = (dataLen >> 8) & 0xff;
        data[8] = (dataLen >> 0) & 0xff;
        respLen = 9; // 传递数据长度
    } else {
        data[0] = (startAddr >> 24) & 0xff;
        data[1] = (startAddr >> 16) & 0xff;
        data[2] = (startAddr >> 8) & 0xff;
        data[3] = (startAddr >> 0) & 0xff;
        data[4] = (dataLen >> 24) & 0xff;
        data[5] = (dataLen >> 16) & 0xff;
        data[6] = (dataLen >> 8) & 0xff;
        data[7] = (dataLen >> 0) & 0xff;
        respLen = 8; // 传递数据长度
    }

    if ((UDS_SERVICE_RESULT_OK != RoutineControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                    priv->info.channel, UDS_ROUTINECTRL_STARTROUTINE, 0xff00, data,
                                                    &respLen)) ||
        (respLen != 1) || (data[0] != 0x00)) {
        OTA_LOG_E("%s RoutineControlReq fail 0x01 0xff00.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    return OTA_RET_OK;
}

/**
 * @brief 下载请求
 * @param priv ECU私有数据结构指针
 * @param startAddr 起始地址
 * @param dataLen 数据长度
 * @param blockLen 块长度指针
 * @return OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
static int DownloadReq(FotaEcuPriv_t *priv, uint32_t startAddr, uint32_t dataLen, uint32_t *blockLen)
{
    if (!priv || !dataLen || !blockLen) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    int32_t result;
    uint8_t memaddr[4];
    uint8_t memsize[4];
    uint32_t maxNumberOfBlockLength = 0;

    memaddr[0] = (startAddr >> 24) & 0xff;
    memaddr[1] = (startAddr >> 16) & 0xff;
    memaddr[2] = (startAddr >> 8) & 0xff;
    memaddr[3] = (startAddr >> 0) & 0xff;

    memsize[0] = (dataLen >> 24) & 0xff;
    memsize[1] = (dataLen >> 16) & 0xff;
    memsize[2] = (dataLen >> 8) & 0xff;
    memsize[3] = (dataLen >> 0) & 0xff;

    result = RequestDownloadReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x00, 0x44,
                                memaddr, memsize, &maxNumberOfBlockLength);
    if (result != UDS_SERVICE_RESULT_OK) {
        OTA_LOG_E("%s RequestDownloadReq fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    if (maxNumberOfBlockLength == 0) {
        OTA_LOG_E("%s RequestDownloadReq fail, block len invalid.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("RequestDownloadReq maxNumberOfBlockLength = [%d], seg dataLen = [%d]\n", maxNumberOfBlockLength,
              dataLen);
    if (maxNumberOfBlockLength > SUPPORT_MAX_BLOCK_LEN) {
        maxNumberOfBlockLength = SUPPORT_MAX_BLOCK_LEN;
    }
    /* Z20K148 SDK接收缓冲区有限, blockLen过大导致MCU重启, 限制256
     * MCU1系列实测支持 514 字节(512字节数据), 不做该限制
     * TCU 实测支持更大块, 也不做该限制 (上限仍由SUPPORT_MAX_BLOCK_LEN约束) */
    if ((priv->info.ecuId != ECU_ID_MCU1_PDU) && (priv->info.ecuId != ECU_ID_MCU1_ACM) &&
        (priv->info.ecuId != ECU_ID_MCU1_TM1) && (priv->info.ecuId != ECU_ID_MCU1_TM2) &&
        (priv->info.ecuId != ECU_ID_MCU1_EPS) && (priv->info.ecuId != ECU_ID_MCU1_DCDC) &&
        (priv->info.ecuId != ECU_ID_TCU) && (maxNumberOfBlockLength > 256)) {
        maxNumberOfBlockLength = 256;
    }
    *blockLen = maxNumberOfBlockLength;
    return OTA_RET_OK;
}

/**
 * @brief 传输数据
 * @param priv ECU私有数据结构指针
 * @param data 数据指针
 * @param dataLen 数据长度
 * @param MaxBlockLength 最大块长度
 * @return OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
// 块间间隔(ms): 非TCU模块保留30ms, 避免MCU来不及处理而丢ACK
#define TRANSFER_BLOCK_INTERVAL_MS (30)
// 36服务无应答重发策略: 单块无应答时重发同一块, 最多3次
#define TRANSFER_BLOCK_RETRY_TIMES (3)
#define TRANSFER_BLOCK_RETRY_INTERVAL_MS (200)

static int TransferData(FotaEcuPriv_t *priv, const uint8_t *data, uint32_t dataLen, uint32_t MaxBlockLength,
                        uint8_t progressStart, uint8_t progressEnd)
{
    if (!priv || !data || !dataLen) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    uint32_t blockLength;
    uint32_t numberOfFullBlocks;
    uint32_t sizeOfLastBlock;
    uint32_t blockSequenceCounter;
    uint32_t i;
    int32_t result;

    blockLength = MaxBlockLength - 2;
    numberOfFullBlocks = dataLen / blockLength;
    sizeOfLastBlock = dataLen % blockLength;
    blockSequenceCounter = 1;

    if (sizeOfLastBlock > 0) {
        numberOfFullBlocks = numberOfFullBlocks + 1;
    }
    
    OTA_LOG_D("TransferData: totalBlocks=%d, blockLength=%d, progressStart=%d, progressEnd=%d",
              numberOfFullBlocks, blockLength, progressStart, progressEnd);
    
    for (i = 0; i < numberOfFullBlocks; i++) {
        uint32_t len = 0;

        len = blockLength;
        if (i == (numberOfFullBlocks - 1) && (sizeOfLastBlock > 0)) {
            len = sizeOfLastBlock;
        }

        uint8_t nrc = 0;
        uint32_t retry = 0;
        for (;;) {
            result = TransferDataReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel,
                                     blockSequenceCounter, &data[i * blockLength], len, &nrc); // 0x36
            if (result == UDS_SERVICE_RESULT_OK) {
                break;
            }

            // 重发后收到NRC 0x73: 说明首次请求已被ECU接收, 仅响应丢失, 直接继续下一块
            if ((result == UDS_SERVICE_RESULT_ERROR_RESPONE_NEGATIVE) && (nrc == 0x73) && (retry > 0)) {
                OTA_LOG_W("%s blockSequenceCounter [%u] already received (NRC 0x73), continue.\n", __FUNCTION__,
                          blockSequenceCounter);
                break;
            }

            // 仅对"无应答"重发, 其他否定响应直接失败
            if ((result != UDS_SERVICE_RESULT_ERROR_TP) || (retry >= TRANSFER_BLOCK_RETRY_TIMES)) {
                OTA_LOG_E(
                    "%s len = [%d]  numberOfFullBlocks = [%d]  blockSequenceCounter = [%d] sizeOfLastBlock = [%d] i = "
                    "[%d].\n",
                    __FUNCTION__, len, numberOfFullBlocks, blockSequenceCounter, sizeOfLastBlock, i);
                OTA_LOG_E("%s TransferDataReq fail, retry=%u, nrc=0x%02X.\n", __FUNCTION__, retry, nrc);
                return OTA_RET_ERR;
            }

            retry++;
            OTA_LOG_W("%s blockSequenceCounter [%u] no response, resend (%u/%u).\n", __FUNCTION__,
                      blockSequenceCounter, retry, TRANSFER_BLOCK_RETRY_TIMES);
            usleep(TRANSFER_BLOCK_RETRY_INTERVAL_MS * 1000);
        }

        // 在数据传输过程中上报进度(按块数分配)
        if (numberOfFullBlocks > 0) {
            uint8_t progress = progressStart + (progressEnd - progressStart) * (i + 1) / numberOfFullBlocks;
            SetEcuBrushProgress(progress);
        }

        blockSequenceCounter++;
        if (blockSequenceCounter > 255) {
            blockSequenceCounter = 0;
        }

        // TCU去除块间30ms延迟; 其他模块保持原延迟, 避免影响现有流程
        if (priv->info.ecuId != ECU_ID_TCU) {
            usleep(TRANSFER_BLOCK_INTERVAL_MS * 1000);
        }
    }
    return OTA_RET_OK;
}

/**
 * @brief 下载退出
 * @param priv ECU私有数据结构指针
 * @param memdata 内存数据指针
 * @param dataLen 数据长度
 * @return OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
static int DownloadExit(FotaEcuPriv_t *priv)
{
    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    uint8_t data[32] = {
        0x0,
    };
    uint32_t len = 0;
    if (UDS_SERVICE_RESULT_OK !=
        RequestTransferExitReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, data, &len)) {
        OTA_LOG_E("%s RequestTransferExitReq fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    return OTA_RET_OK;
}

/**
 * @brief 升级分段
 * @param priv ECU私有数据结构指针
 * @param flashinfo Flash信息结构指针
 * @param isSoftware 是否为软件升级
 * @return OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
static int UpgradeSigment(FotaEcuPriv_t *priv, Flashinfo_t *flashinfo, bool isSoftware,
                          uint8_t progressStart, uint8_t progressEnd)
{
    if (!priv || !flashinfo) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    int32_t i = 0;
    for (; i < flashinfo->segNum; i++) {
        int32_t retVal;
        uint32_t blockLen = 0;

        // 计算当前段的进度范围
        uint8_t segProgressStart = progressStart + (progressEnd - progressStart) * i / flashinfo->segNum;
        uint8_t segProgressEnd = progressStart + (progressEnd - progressStart) * (i + 1) / flashinfo->segNum;

        if (isSoftware) { // 软件升级需要擦除
            if (OTA_RET_OK != EraseFlash(priv, flashinfo->segment[i].memaddr, flashinfo->segment[i].memsize)) {
                OTA_LOG_E("%s 15 EraseFlash fail.\n", __FUNCTION__);
                return OTA_RET_ERR;
            }
        }
        if (OTA_RET_OK != DownloadReq(priv, flashinfo->segment[i].memaddr, flashinfo->segment[i].memsize, &blockLen)) {
            OTA_LOG_E("%s DownloadReq fail.\n", __FUNCTION__);
            return OTA_RET_ERR;
        }
        if (OTA_RET_OK != TransferData(priv, flashinfo->segment[i].memdata, flashinfo->segment[i].memsize, blockLen,
                                       segProgressStart, segProgressEnd)) {
            OTA_LOG_E("%s TransferData fail.\n", __FUNCTION__);
            return OTA_RET_ERR;
        }
        if (OTA_RET_OK != DownloadExit(priv)) {
            OTA_LOG_E("%s DownloadExit fail.\n", __FUNCTION__);
            return OTA_RET_ERR;
        }
    }

    return OTA_RET_OK;
}

/**
 * @brief 检查编程完整性
 * @param priv ECU私有数据结构指针
 * @param flashinfo Flash信息结构指针
 * @return OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
static int CheckProgrammingIntegrity(FotaEcuPriv_t *priv, Flashinfo_t *flashinfo)
{
    if (!priv || !flashinfo) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    uint8_t data[32];
    uint32_t dataLen = 0;
    uint32_t crc32;

    // VCU/TCU使用非反射型CRC-32(CRC-32/BZIP2), 其他ECU使用通用反射型
    if ((priv->info.ecuId == ECU_ID_VCU) || (priv->info.ecuId == ECU_ID_TCU)) {
        crc32 = UdsCalcCRC32Vcu(flashinfo->data, flashinfo->dataLen);
        OTA_LOG_D("%s ECU[0x%04X] CRC32 = 0x%08X (non-reflected, len=%u)\n", __FUNCTION__, priv->info.ecuId,
                  crc32, flashinfo->dataLen);
    } else {
        crc32 = UdsCalcCRC32(flashinfo->data, flashinfo->dataLen);
    }
    data[0] = (crc32 >> 24) & 0xff;
    data[1] = (crc32 >> 16) & 0xff;
    data[2] = (crc32 >> 8) & 0xff;
    data[3] = crc32 & 0xff;
    dataLen = 4;

    if ((UDS_SERVICE_RESULT_OK != RoutineControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel,
        UDS_ROUTINECTRL_STARTROUTINE, 0x0202, data, &dataLen)) || (dataLen != 1) || (data[0] != 0x00)) {
        OTA_LOG_E("%s RoutineControlReq fail 0x01 0x0202.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    return OTA_RET_OK;
}

/**
 * @brief 写入指纹信息
 * @param priv ECU私有数据结构指针
 * @return OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
static int WriteFingerprintInfo(FotaEcuPriv_t *priv)
{
    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    time_t now = time(nullptr);
    struct tm *tm_now = localtime(&now);
    uint8_t fingerprintData[9] = {0};
    fingerprintData[0] = tm_now->tm_year + 1900 - 2000;
    fingerprintData[1] = tm_now->tm_mon + 1;
    fingerprintData[2] = tm_now->tm_mday;
    memset(&fingerprintData[3], 0xFF, 6);

    if (UDS_SERVICE_RESULT_OK != WriteDataByIdentifierReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                          priv->info.channel, 0xF184, fingerprintData, 9)) {
        OTA_LOG_E("%s failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    return OTA_RET_OK;
}

/**
 * @brief 验证软件版本号是否一致
 * @param priv ECU私有数据结构指针
 * @param targetVer 目标版本号
 * @return OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
static int CheckSoftwareVersion(FotaEcuPriv_t *priv, const std::string &targetVer)
{
    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    uint8_t data[32];
    uint32_t dateLen = 0;

    // 查询软件版本号
    if (UDS_SERVICE_RESULT_OK != ReadDataByIdentifierReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                         priv->info.channel, 0xF189, data, &dateLen)) {
        OTA_LOG_E("%s ReadDataByIdentifierReq fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 判断检查版本号是否一致
    if (dateLen < 4) {
        OTA_LOG_E("%s Invalid version length.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 将data转换为字符串以进行比较（ECU返回的是ASCII编码的版本号）
    // data[0]=0x30('0'), data[1]=0x30('0'), data[2]=0x30('0'), data[3]=0x32('2') -> "0002"
    char verStr[9] = {0};
    uint32_t copyLen = (dateLen < sizeof(verStr) - 1) ? dateLen : sizeof(verStr) - 1;
    for (uint32_t i = 0; i < copyLen; i++) {
        verStr[i] = static_cast<char>(data[i]);
    }
    verStr[copyLen] = '\0';
    std::string currentVer(verStr);
    StringToUpper(currentVer);

    // 比较当前版本与对比版本
    if (currentVer != targetVer) {
        OTA_LOG_E("Version mismatch. Current: %s, Expected: %s\n", currentVer.c_str(), targetVer.c_str());
        FotaEcuSetErrorCode(priv, OTA_ERR_BRUSH_VERSION_INCONSISTENT);
        return OTA_RET_ERR;
    }

    OTA_LOG_D("Software version check passed. Version: %s\n", currentVer.c_str());
    return OTA_RET_OK;
}

static int EcuClearDtc(FotaEcuPriv_t *priv)
{
    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    if (UDS_SERVICE_RESULT_OK !=
        ClearDtcReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0xffffff)) {
        OTA_LOG_E("%s 14 FF FF FF failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    return OTA_RET_OK;
}

int VerifySoftwareVersion(FotaEcuHandle_t hdl, const std::string &targetVer)
{
    FotaEcuPriv_t *priv = (FotaEcuPriv_t *)hdl;
    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    int ret = CheckSoftwareVersion(priv, targetVer);
    EcuClearDtc(priv);
    return ret;
}

/**
 * @brief 保持会话活动的线程函数
 * @param arg 线程参数(FotaEcuPriv_t指针)
 * @return void* 线程返回值(NULL)
 */
static void *KeepSession(void *arg)
{
    FotaEcuPriv_t *priv = (FotaEcuPriv_t *)arg;
    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return NULL;
    }

    while (!priv->keepExit) {
        TesterPresentReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, priv->suppressResponse);
        sleep(2);
    }
    return NULL;
}


/**
 * @brief 预编程&校验
 * @param priv ECU私有数据结构指针
 * @return OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
static int FotaEcuPreviousProgramming(FotaEcuPriv_t *priv)
{
    uint8_t data[32];
    uint32_t dataLen = 0;
    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 步骤1 切换扩展模式
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.funcId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_EXTENDED)) {
        OTA_LOG_E("%s 10 03 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    // 步骤2-3 安全校验Level1
    if (OTA_RET_OK != CheckAccessSecurity1(priv)) {
        OTA_LOG_E("%s CheckAccessSecurity1 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    // 步骤4 预编程检查
    if (UDS_SERVICE_RESULT_OK != RoutineControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel,
                                                   UDS_ROUTINECTRL_STARTROUTINE, 0x0203, data, &dataLen) ||
        (dataLen != 1) || (data[0] != 0x00)) {
        OTA_LOG_E("%s 31 01 02 03 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    // 步骤5 禁止DTC存储
    if (UDS_SERVICE_RESULT_OK != ControlDTCSettingReq(priv->srvHdl, priv->info.funcId, priv->info.resId,
                                                      priv->info.channel, UDS_CTRLDTCSETTING_OFF_NOACK)) {
        OTA_LOG_E("%s 85 82 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    usleep(100000); 
    // 步骤6 关闭非诊断通信
    if (UDS_SERVICE_RESULT_OK !=
        CommunicationControlReq(priv->srvHdl, priv->info.funcId, priv->info.resId, priv->info.channel, 0x83, 0x03)) {
        OTA_LOG_E("%s 28 83 03 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    usleep(100000); 
    // 步骤7 切换编程模式
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_PROGRAMMING)) {
        OTA_LOG_E("%s 10 02 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    usleep(500000); // 等待500ms，确保ECU切换到编程模式(弗迪BMS需求)
    // 步骤8-9 安全校验Level3
    if (OTA_RET_OK != CheckAccessSecurity3(priv)) {
        OTA_LOG_E("%s CheckAccessSecurity3 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    return OTA_RET_OK;
}

/**
 * @brief 驱动程序下载
 * @param priv ECU私有数据结构指针
 * @param flashinfo 驱动程序Flash信息结构指针
 * @return OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
static int FotaEcuDowloadDriverProgramming(FotaEcuPriv_t *priv, Flashinfo_t *flashinfo)
{
    if (!priv || !flashinfo) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    //  步骤11-13 下载 (进度20%~40%)
    if (OTA_RET_OK != UpgradeSigment(priv, flashinfo, false, 20, 40)) {
        OTA_LOG_E("%s 11-13 UpgradeSigment fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    // 步骤14完整性校验
    if (OTA_RET_OK != CheckProgrammingIntegrity(priv, flashinfo)) {
        OTA_LOG_E("%s 31 01 02 02 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    return OTA_RET_OK;
}

/**
 * @brief 软件程序下载
 * @param priv ECU私有数据结构指针
 * @param flashinfo 软件程序Flash信息结构指针
 * @return OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
static int FotaEcuDowloadSoftWareProgramming(FotaEcuPriv_t *priv, Flashinfo_t *flashinfo,
                                              uint8_t progressStart, uint8_t progressEnd)
{
    if (!priv || !flashinfo) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    // 步骤15-18 擦写&下载
    if (OTA_RET_OK != UpgradeSigment(priv, flashinfo, true, progressStart, progressEnd)) {
        OTA_LOG_E("%s 15-18 UpgradeSigment fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    // 步骤19完整性校验
    if (OTA_RET_OK != CheckProgrammingIntegrity(priv, flashinfo)) {
        OTA_LOG_E("%s 31 01 02 02 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    return OTA_RET_OK;
}

/**
 * @brief VCU刷写流程
 * @param priv ECU私有数据结构指针
 * @param flashinfo Flash信息结构指针
 * @param softwareinfo 软件信息结构指针
 * @return OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
static int FotaEcuProgrammingVcu(FotaEcuPriv_t *priv, Flashinfo_t *driverinfo, Flashinfo_t *softwareinfo)
{
    uint8_t data[32];
    if (!priv || !softwareinfo) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }    
   
    // 步骤11-15 软件下载 (进度20%~80%，VCU无驱动下载)
    OTA_LOG_D("FotaEcuProgrammingVcu: softwareinfo=%p, segNum=%d",
              softwareinfo, softwareinfo ? softwareinfo->segNum : 0);
    if (softwareinfo != nullptr) {
        OTA_LOG_D("FotaEcuProgrammingVcu: calling FotaEcuDowloadSoftWareProgramming with progress 20-80");
        if (OTA_RET_OK != FotaEcuDowloadSoftWareProgramming(priv, softwareinfo, 20, 80)) {
            OTA_LOG_E("%s FotaEcuDowloadSoftWareProgramming fail.\n", __FUNCTION__);
            return OTA_RET_ERR;
        }
        OTA_LOG_D("FotaEcuProgrammingVcu: FotaEcuDowloadSoftWareProgramming completed");
    }

    // 步骤16 依赖性校验
    uint32_t dataLen = 0;
    int ret = RoutineControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel,
                                UDS_ROUTINECTRL_STARTROUTINE, 0xFF01, data, &dataLen);
    if ((UDS_SERVICE_RESULT_OK != ret) || (dataLen != 1) || (data[0] != 0x00)) {
        OTA_LOG_E("%s 31 FF 01 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    return OTA_RET_OK;
}


/**
 * @brief 刷写流程
 * @param priv ECU私有数据结构指针
 * @param flashinfo Flash信息结构指针
 * @param softwareinfo 软件信息结构指针
 * @return OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
static int FotaEcuProgramming(FotaEcuPriv_t *priv, Flashinfo_t *driverinfo, Flashinfo_t *softwareinfo)
{
    uint8_t data[32];
    if (!priv || (!driverinfo && !softwareinfo)) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 步骤10 写入指纹
    if (UDS_SERVICE_RESULT_OK != WriteFingerprintInfo(priv)) {
        OTA_LOG_E("%s 2E F184 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    // 步骤11-14 驱动下载
    if (driverinfo != nullptr) {
        if (FotaEcuDowloadDriverProgramming(priv, driverinfo) != OTA_RET_OK) {
            OTA_LOG_E("%s FotaEcuDowloadDriverProgramming fail.\n", __FUNCTION__);
            return OTA_RET_ERR;
        }
    }
    // 步骤15-19 软件下载 (进度40%~80%)
    if (softwareinfo != nullptr) {
        if (OTA_RET_OK != FotaEcuDowloadSoftWareProgramming(priv, softwareinfo, 40, 80)) {
            OTA_LOG_E("%s FotaEcuDowloadSoftWareProgramming fail.\n", __FUNCTION__);
            return OTA_RET_ERR;
        }
    }
    // 步骤20 依赖性校验
    uint32_t dataLen = 0;
    int ret = RoutineControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel,
                                UDS_ROUTINECTRL_STARTROUTINE, 0xFF01, data, &dataLen);
    if ((UDS_SERVICE_RESULT_OK != ret) || (dataLen != 1) || (data[0] != 0x00)) {
        OTA_LOG_E("%s 31 FF 01 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    return OTA_RET_OK;
}

static int FotaEcuPostProgramming(FotaEcuPriv_t *priv)
{
    int ret = OTA_RET_OK;
    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    // 步骤21 硬件复位
    if (UDS_SERVICE_RESULT_OK !=
        EcuResetReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, UDS_ECURESET_HARDRESET)) {
        OTA_LOG_E("%s 11 01 fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
    }
    // 步骤22 切换扩展会话
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.funcId, priv->info.resId,
                                                             priv->info.channel, false, UDS_SESSION_EXTENDED_NOACK)) {
        OTA_LOG_E("%s 10 83 fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
    }
    usleep(100000); 
    // 步骤23 重新启用DTC存储
    if (UDS_SERVICE_RESULT_OK != ControlDTCSettingReq(priv->srvHdl, priv->info.funcId, priv->info.resId,
                                                      priv->info.channel, UDS_CTRLDTCSETTING_ON_NOACK)) {
        OTA_LOG_E("%s 85 81 fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
    }
    usleep(100000); 
    // 步骤24 恢复全车通信
    if (UDS_SERVICE_RESULT_OK !=
        CommunicationControlReq(priv->srvHdl, priv->info.funcId, priv->info.resId, priv->info.channel, 0x80, 0x03)) {
        OTA_LOG_E("%s 28 80 03 fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
    }
    usleep(100000); 
    // 步骤25 切换默认会话
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.funcId, priv->info.resId,
                                                             priv->info.channel, false, UDS_SESSION_DEFAULT_NOACK)) {
        OTA_LOG_E("%s 10 81 fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
    }

    return ret;
}

/**
 * @brief VCU后编程流程
 * @param priv ECU私有数据结构指针
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 */

static int FotaEcuPostProgrammingVcu(FotaEcuPriv_t *priv)
{
    int ret = OTA_RET_OK;
    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    // 步骤17 硬件复位
    if (UDS_SERVICE_RESULT_OK !=
        EcuResetReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, UDS_ECURESET_HARDRESET)) {
        OTA_LOG_E("%s 11 01 fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
    }
    usleep(1000000); 
    // 步骤18 恢复全车通信
    if (UDS_SERVICE_RESULT_OK !=
        CommunicationControlReq(priv->srvHdl, priv->info.funcId, priv->info.resId, priv->info.channel, 0x00, 0x01)) {
        OTA_LOG_E("%s 28 00 01 fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
    }
    usleep(100000); 
    // 步骤19 重新启用DTC存储
    if (UDS_SERVICE_RESULT_OK != ControlDTCSettingReq(priv->srvHdl, priv->info.funcId, priv->info.resId,
                                                      priv->info.channel, UDS_CTRLDTCSETTING_ON)) {
        OTA_LOG_E("%s 85 01 fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
    }

    usleep(100000); 
    // 步骤20 切换默认会话
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                             priv->info.channel, false, UDS_SESSION_DEFAULT)) {
        OTA_LOG_E("%s 10 01 fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
    }

    return ret;
}

/**
 * @brief VCU预编程流程 (使用0x27 03 - 0x27 04安全访问)
 * @param priv ECU私有数据结构指针
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 */
static int FotaEcuPreviousProgrammingVcu(FotaEcuPriv_t *priv)
{
    uint8_t data[32];
    uint32_t dataLen = 0;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
   
    // 预编程阶段(步骤1~7, 含10 02与其后的3E):
    //   - 单条指令"超时无响应" → 同一指令重发, 最多3次(VcuResendOnTimeout);
    //   - "拒绝响应"(负响应/格式错) 或 重发3次仍超时 → 返回false, 由外层从步骤1(10 01)整段重试(最多3次)。
    auto runVcuPreProgramming = [&]() -> bool {
        int r = UDS_SERVICE_RESULT_ERROR_TP;

        // 步骤1 切换默认模式 (10 01, 物理寻址0x7DC)
        r = VcuResendOnTimeout(priv, "10 01", [&]() -> int {
            return (int)DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                     priv->info.channel, true, UDS_SESSION_DEFAULT);
        });
        if (r != UDS_SERVICE_RESULT_OK) {
            OTA_LOG_E("%s 10 01 fail(r=%d).\n", __FUNCTION__, r);
            return false;
        }

        // 步骤2 切换扩展模式 (10 03, 功能寻址0x7DF)
        r = VcuResendOnTimeout(priv, "10 03", [&]() -> int {
            return (int)DiagnosticSessionControlReq(priv->srvHdl, priv->info.funcId, priv->info.resId,
                                                     priv->info.channel, true, UDS_SESSION_EXTENDED);
        });
        if (r != UDS_SERVICE_RESULT_OK) {
            OTA_LOG_E("%s 10 03 fail(r=%d).\n", __FUNCTION__, r);
            return false;
        }

        // 步骤3 预编程检查 (31 01 02 03, 物理寻址0x7DC)
        r = VcuResendOnTimeout(priv, "31 01 02 03", [&]() -> int {
            dataLen = 0;
            int rr = RoutineControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel,
                                       UDS_ROUTINECTRL_STARTROUTINE, 0x0203, data, &dataLen);
            if (rr == UDS_SERVICE_RESULT_OK && (dataLen != 1 || data[0] != 0x00)) {
                return (int)UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
            }
            return rr;
        });
        if (r != UDS_SERVICE_RESULT_OK) {
            OTA_LOG_E("%s 31 01 02 03 fail(r=%d).\n", __FUNCTION__, r);
            return false;
        }

        // 步骤4 禁止DTC存储 (85 02): 功能寻址0x7DF, 必须收到应答
        r = VcuResendOnTimeout(priv, "85 02", [&]() -> int {
            return (int)ControlDTCSettingReqTimeout(priv->srvHdl, priv->info.funcId, priv->info.resId,
                                                     priv->info.channel, UDS_CTRLDTCSETTING_OFF, 1000);
        });
        if (r != UDS_SERVICE_RESULT_OK) {
            OTA_LOG_E("%s 85 02 fail(r=%d).\n", __FUNCTION__, r);
            return false;
        }

        // 步骤5 关闭非诊断通信 (28 03 03): 功能寻址0x7DF, 必须收到应答
        r = VcuResendOnTimeout(priv, "28 03 03", [&]() -> int {
            return (int)CommunicationControlReqTimeout(priv->srvHdl, priv->info.funcId, priv->info.resId,
                                                        priv->info.channel, 0x03, 0x03, 1000);
        });
        if (r != UDS_SERVICE_RESULT_OK) {
            OTA_LOG_E("%s 28 03 03 fail(r=%d).\n", __FUNCTION__, r);
            return false;
        }

        // 步骤6 读取电控单元零件号 (22 F1 87, 物理寻址0x7DC)
        r = VcuResendOnTimeout(priv, "22 F1 87", [&]() -> int {
            dataLen = 0;
            return (int)ReadDataByIdentifierReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel,
                                                 0xF187, data, &dataLen);
        });
        if (r != UDS_SERVICE_RESULT_OK) {
            OTA_LOG_E("%s ReadDataByIdentifierReq 0xF187 fail(r=%d).\n", __FUNCTION__, r);
            return false;
        }

        // 步骤7 切换编程模式 (10 02, 物理寻址0x7DC)
        r = VcuResendOnTimeout(priv, "10 02", [&]() -> int {
            return (int)DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                     priv->info.channel, true, UDS_SESSION_PROGRAMMING);
        });
        if (r != UDS_SERVICE_RESULT_OK) {
            OTA_LOG_E("%s 10 02 fail(r=%d).\n", __FUNCTION__, r);
            return false;
        }

        // 10 02正响应(50 02)后连发 5 遍 3E 00(物理寻址0x7DC), 每遍必须收到成功应答7E 00(超时也重发)
        for (int tpTimes = 0; tpTimes < 5; tpTimes++) {
            r = VcuResendOnTimeout(priv, "3E 00", [&]() -> int {
                return (int)TesterPresentReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel,
                                              false);
            });
            if (r != UDS_SERVICE_RESULT_OK) {
                OTA_LOG_E("%s 3E 00 after 10 02 no 7E 00(times=%d, r=%d).\n", __FUNCTION__, tpTimes + 1, r);
                return false;
            }
        }

        return true;
    };

    bool preprogOk = false;
    for (int preprogTry = 1; preprogTry <= VCU_PREPROG_TRY_TIMES; preprogTry++) {
        if (runVcuPreProgramming()) {
            preprogOk = true;
            break;
        }
        OTA_LOG_W("%s preprogramming fail (try %d/%d), restart from step1(10 01).\n", __FUNCTION__, preprogTry,
                  VCU_PREPROG_TRY_TIMES);
        usleep(100 * 1000);
    }
    if (!preprogOk) {
        OTA_LOG_E("%s preprogramming fail after %d tries.\n", __FUNCTION__, VCU_PREPROG_TRY_TIMES);
        return OTA_RET_ERR;
    }
    SetEcuBrushProgress(15);

    // 步骤8 创建会话保持线程（周期发送3E 00保持编程会话）
    // 注意：KeepSession线程与主刷写流程共用counterIdx，会导致响应counterIdx竞态，
    //      擦除/下载期间并发发3E会抢counterIdx，使响应被误判stale丢弃，故关闭。
    // try {
    //     priv->keepSessionThread = std::thread(KeepSession, priv);
    // } catch (const std::exception &e) {
    //     OTA_LOG_E("%s create thread fail: %s\n", __FUNCTION__, e.what());
    //     return OTA_RET_ERR;
    // }

    // usleep(10*1000); ; // 等待500ms，确保ECU切换到编程模式
    // 步骤9-10 安全校验Level2 (0x27 03 - 0x27 04)
    if (OTA_RET_OK != CheckAccessSecurity2(priv)) {
        OTA_LOG_E("%s CheckAccessSecurity2 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }    

    return OTA_RET_OK;
}

/**
 * @brief VCU刷写流程
 * @param priv ECU私有数据结构指针
 * @param driverFlashinfo 驱动Flash信息
 * @param softwareFlashinfo 软件Flash信息
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note VCU使用0x27 03 - 0x27 04安全访问(Level2)
 */
static int FotaEcuExecVcu(FotaEcuPriv_t *priv, Flashinfo_t *driverFlashinfo, Flashinfo_t *softwareFlashinfo)
{
    OTA_LOG_I("Execute VCU[0x%04X] brush flow", priv->info.ecuId);
    OTA_LOG_D("[FotaEcuExecVcu] phyId: 0x%08X, funcId: 0x%08X, resId: 0x%08X, channel: %d",
              priv->info.phyId, priv->info.funcId, priv->info.resId, priv->info.channel);
    
    SetEcuBrushProgress(5);  // 开始刷写
    
    // 执行VCU专用编程前准备 (使用0x27 03 - 0x27 04)
    if (OTA_RET_OK != FotaEcuPreviousProgrammingVcu(priv)) {
        OTA_LOG_E("%s FotaEcuPreviousProgrammingVcu fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    SetEcuBrushProgress(20);  // 预编程完成
    
    // 执行编程 (内部会上报20%~80%的进度)
    if (OTA_RET_OK != FotaEcuProgrammingVcu(priv, driverFlashinfo, softwareFlashinfo)) {
        OTA_LOG_E("%s FotaEcuProgrammingVcu fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    // 执行编程后清理
    if (OTA_RET_OK != FotaEcuPostProgrammingVcu(priv)) {
        OTA_LOG_E("%s FotaEcuPostProgrammingVcu fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    SetEcuBrushProgress(90);  // 后编程完成
    
    // CleanupKeepSessionThread(priv);
    return OTA_RET_OK;
}

/**
 * @brief TMS刷写流程
 * @param priv ECU私有数据结构指针
 * @param driverFlashinfo 驱动Flash信息
 * @param softwareFlashinfo 软件Flash信息
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note 当前走标准流程，后续根据TMS定制
 */
/**
 * @brief TMS预编程步骤
 * 根据TMS升级流程文档:
 * (a) 0x10 03 - 扩展会话
 * (b) 0x31 01 02 03 - 检查预编程条件
 * (c) 0x85 02 - 禁止DTC存储
 * (d) 0x28 03 03 - 关闭非诊断通信
 * (e) 0x22 xx yy - 读取数据
 */
static int FotaEcuPreviousProgrammingTms(FotaEcuPriv_t *priv)
{
    uint8_t data[32];
    uint32_t dataLen = 0;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 步骤1: 进入扩展会话 (0x10 03)
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_EXTENDED)) {
        OTA_LOG_E("%s step1: 0x10 03 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TMS step1: Enter extended session success");
    usleep(100000);
    SetEcuBrushProgress(5);

    // 步骤2: 检查预编程条件 (0x31 01 02 03)
    dataLen = 0;
    if (UDS_SERVICE_RESULT_OK != RoutineControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel,
                                                   UDS_ROUTINECTRL_STARTROUTINE, 0x0203, data, &dataLen) ||
        (dataLen != 1) || (data[0] != 0x00)) {
        OTA_LOG_E("%s step2: 0x31 01 02 03 fail, pre-programming condition check failed\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TMS step2: Pre-programming condition check success");
    SetEcuBrushProgress(8);

    // 步骤3: 禁止DTC存储 (0x85 02)
    if (UDS_SERVICE_RESULT_OK != ControlDTCSettingReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                      priv->info.channel, UDS_CTRLDTCSETTING_OFF)) {
        OTA_LOG_E("%s step3: 0x85 02 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TMS step3: DTC setting off success");
    SetEcuBrushProgress(10);
    usleep(100000);

    // 步骤4: 关闭非诊断通信 (0x28 03 03)
    if (UDS_SERVICE_RESULT_OK !=
        CommunicationControlReq(priv->srvHdl, priv->info.funcId, priv->info.resId, priv->info.channel, 0x03, 0x03)) {
        OTA_LOG_E("%s step4: 0x28 03 03 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TMS step4: Communication control success");
    SetEcuBrushProgress(12);
    usleep(100000);

    // 步骤5: 读取ECU数据 (0x22 F1 87 - 零件号)
    dataLen = 0;
    if (UDS_SERVICE_RESULT_OK != ReadDataByIdentifierReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                         priv->info.channel, 0xF187, data, &dataLen)) {
        OTA_LOG_E("%s step5: 0x22 F187 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TMS step5: Read ECU part number success");
    if (dataLen > 0 && dataLen <= 32) {
        char hexStr[128] = {0};
        for (uint32_t i = 0; i < dataLen; i++) {
            snprintf(hexStr + i*3, sizeof(hexStr) - i*3, "%02X ", data[i]);
        }
        OTA_LOG_I("TMS step5: Read 0xF187 success, len=%d, data=[%s]", dataLen, hexStr);
    }
    SetEcuBrushProgress(15);

    return OTA_RET_OK;
}

/**
 * @brief TMS安全访问Level2
 * 使用0x27 09/0A进行种子密钥认证
 */
static int CheckAccessSecurityTms(FotaEcuPriv_t *priv)
{
    uint8_t data[128] = {0};
    uint32_t len = 0;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 步骤1: 0x27 0x09 请求种子
    if (UDS_SERVICE_RESULT_OK !=
        SecurityAccessReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x09, data, &len)) {
        OTA_LOG_E("%s 0x27 0x09 failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_I("%s 27 09 response: len=%d, data=[%02X %02X %02X %02X]", __FUNCTION__, len, data[0], data[1], data[2], data[3]);

    if (len < 4) {
        OTA_LOG_E("%s invalid seed length %d, expected 4 bytes.\n", __FUNCTION__, len);
        return OTA_RET_ERR;
    }

    // TMS种子是4字节
    uint32_t seed = (uint32_t)(((uint32_t)(data[0]) << 24) + ((uint32_t)(data[1]) << 16) +
                               ((uint32_t)(data[2]) << 8) + (uint32_t)(data[3]));
    uint32_t key = static_cast<uint32_t>(UdsCalcKeyTms(seed));
    OTA_LOG_D("%s TMS key calculation, seed=0x%08X, key=0x%08X", __FUNCTION__, seed, key);

    // 密钥也是4字节
    data[0] = (uint8_t)((key >> 24) & 0xFF);
    data[1] = (uint8_t)((key >> 16) & 0xFF);
    data[2] = (uint8_t)((key >> 8) & 0xFF);
    data[3] = (uint8_t)(key & 0xFF);
    len = 4;

    // 步骤2: 0x27 0x0A 发送密钥
    if (UDS_SERVICE_RESULT_OK !=
        SecurityAccessReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x0A, data, &len)) {
        OTA_LOG_E("%s 0x27 0x0A failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TMS Boot security access success");

    return OTA_RET_OK;
}

/**
 * @brief TMS主编程步骤
 * 根据TMS升级流程文档:
 * (a) 0x10 02 - 编程会话
 * (b) 0x27 09/0A - 安全访问
 * (c) 驱动下载 0x34, 0x36, 0x37, 0x31 (检查编程完整性)
 * (d) 0x2E F1 5A - 写入指纹
 * (e) 0x31 01 FF 00 - 擦除内存
 * (f) 下载过程 0x34, 0x36, 0x37
 * (g) 0x31 01 02 02 - 检查编程完整性
 * (h) 0x31 01 FF 01 - 检查编程依赖性
 * (i) 0x11 01 - ECU复位
 */
static int FotaEcuProgrammingTms(FotaEcuPriv_t *priv, Flashinfo_t *driverFlashinfo, Flashinfo_t *softwareFlashinfo)
{
    uint8_t data[32];
    uint32_t dataLen = 0;

    if (!priv || !softwareFlashinfo) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 步骤1: 进入编程会话 (0x10 02)
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_PROGRAMMING)) {
        OTA_LOG_E("%s step1: 0x10 02 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TMS step1: Enter programming session success");
    usleep(500000); // 等待500ms，确保ECU切换到编程模式
    SetEcuBrushProgress(20);

    // 步骤2: 安全访问 (0x27 09/0A)
    if (OTA_RET_OK != CheckAccessSecurityTms(priv)) {
        OTA_LOG_E("%s step2: CheckAccessSecurityTms fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TMS step2: Security access success");
    SetEcuBrushProgress(25);

    // 步骤3-4: 驱动下载 (进度25%~35%)
    if (driverFlashinfo != nullptr) {
        if (OTA_RET_OK != UpgradeSigment(priv, driverFlashinfo, false, 25, 35)) {
            OTA_LOG_E("%s step3-4: UpgradeSigment for driver fail.\n", __FUNCTION__);
            return OTA_RET_ERR;
        }
        // 检查驱动编程完整性 (0x31 01 02 02)
        if (OTA_RET_OK != CheckProgrammingIntegrity(priv, driverFlashinfo)) {
            OTA_LOG_E("%s step4: Check driver program integrity fail.\n", __FUNCTION__);
            return OTA_RET_ERR;
        }
        OTA_LOG_D("TMS step3-4: Download driver and verify success");
    }

    // 步骤5: 写入指纹信息 (0x2E F1 5A)
    // 指纹格式定义 (P0-2 / P1-12 缺陷修复):
    //   [0]      : 指纹版本号 = 0x01
    //   [1..2]   : ECU ID (高/低字节) 标识被刷写的ECU (TMS_A=0x067C / TMS_B=0x068C)
    //   [3..6]   : 升级Unix时间戳(大端) 用于追溯刷写时点
    //   [7]      : 编程会话次数(单调递增计数器，本实现取时间戳低字节)
    //   [8..11]  : CRC32(以上数据) 用于ECU侧校验指纹完整性
    //   [12..15] : 保留字节 0xFF,0xFF,0xFF,0xFF
    // 原代码: data[0]=0x01, data[1..3]=0 (P0缺陷，不同车写入完全相同指纹)
    {
        const uint8_t fpVer = 0x01;
        const uint16_t ecuId16 = priv->info.ecuId;       // 0x067C / 0x068C
        const uint32_t ts = (uint32_t)time(nullptr);     // 升级时间戳
        const uint32_t crc = UdsCalcCRC32((const uint8_t*)&ts, sizeof(ts))
                           ^ ((uint32_t)ecuId16 << 16) ^ (uint32_t)fpVer;

        data[0]  = fpVer;
        data[1]  = (uint8_t)(ecuId16 >> 8);              // ECU ID 高字节 (0x06)
        data[2]  = (uint8_t)(ecuId16 & 0xFF);            // ECU ID 低字节 (0x7C/0x8C)
        data[3]  = (uint8_t)((ts >> 24) & 0xFF);         // 时间戳 大端
        data[4]  = (uint8_t)((ts >> 16) & 0xFF);
        data[5]  = (uint8_t)((ts >> 8)  & 0xFF);
        data[6]  = (uint8_t)(ts & 0xFF);
        data[7]  = (uint8_t)(ts & 0xFF);                 // 编程次数代理
        data[8]  = (uint8_t)((crc >> 24) & 0xFF);        // CRC32 大端
        data[9]  = (uint8_t)((crc >> 16) & 0xFF);
        data[10] = (uint8_t)((crc >> 8)  & 0xFF);
        data[11] = (uint8_t)(crc & 0xFF);
        data[12] = 0xFF;                                 // 保留
        data[13] = 0xFF;
        data[14] = 0xFF;
        data[15] = 0xFF;
        dataLen  = 16;
    }
    OTA_LOG_D("TMS step5: fingerprint [ver=0x%02X ecu=0x%04X ts=%u len=%u]",
              data[0], priv->info.ecuId, (uint32_t)time(nullptr), dataLen);
    if (UDS_SERVICE_RESULT_OK != WriteDataByIdentifierReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                          priv->info.channel, 0xF15A, data, dataLen)) {
        OTA_LOG_E("%s step5: 0x2E F1 5A fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TMS step5: Write fingerprint success");
    SetEcuBrushProgress(35);

    // 步骤6: 擦除内存 (0x31 01 FF 00)
    for (int i = 0; i < softwareFlashinfo->segNum; i++) {
        data[0] = (softwareFlashinfo->segment[i].memaddr >> 24) & 0xff;
        data[1] = (softwareFlashinfo->segment[i].memaddr >> 16) & 0xff;
        data[2] = (softwareFlashinfo->segment[i].memaddr >> 8) & 0xff;
        data[3] = (softwareFlashinfo->segment[i].memaddr >> 0) & 0xff;
        data[4] = (softwareFlashinfo->segment[i].memsize >> 24) & 0xff;
        data[5] = (softwareFlashinfo->segment[i].memsize >> 16) & 0xff;
        data[6] = (softwareFlashinfo->segment[i].memsize >> 8) & 0xff;
        data[7] = (softwareFlashinfo->segment[i].memsize >> 0) & 0xff;
        dataLen = 8;

        if ((UDS_SERVICE_RESULT_OK != RoutineControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                       priv->info.channel, UDS_ROUTINECTRL_STARTROUTINE,
                                                       0xFF00, data, &dataLen)) ||
            (dataLen != 1) || (data[0] != 0x00)) {
            OTA_LOG_E("%s step6: 0x31 01 FF 00 fail for segment %d.\n", __FUNCTION__, i);
            return OTA_RET_ERR;
        }
        OTA_LOG_D("TMS step6: Erase memory segment %d success, addr=0x%08X, size=0x%08X",
                  i, softwareFlashinfo->segment[i].memaddr, softwareFlashinfo->segment[i].memsize);
    }
    SetEcuBrushProgress(40);

    // 步骤7: 下载软件程序 (进度40%~75%)
    if (OTA_RET_OK != UpgradeSigment(priv, softwareFlashinfo, true, 40, 75)) {
        OTA_LOG_E("%s step7: UpgradeSigment for software fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 步骤8: 检查编程完整性 (0x31 01 02 02)
    if (OTA_RET_OK != CheckProgrammingIntegrity(priv, softwareFlashinfo)) {
        OTA_LOG_E("%s step8: Check software program integrity fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TMS step7-8: Download software and verify success");

    // 步骤9: 检查编程依赖性 (0x31 01 FF 01)
    dataLen = 0;
    if (UDS_SERVICE_RESULT_OK != RoutineControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel,
                                                   UDS_ROUTINECTRL_STARTROUTINE, 0xFF01, data, &dataLen) ||
        (dataLen != 1) || (data[0] != 0x00)) {
        OTA_LOG_E("%s step9: 0x31 01 FF 01 fail, dependency check failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TMS step9: Check programming dependency success");
    SetEcuBrushProgress(78);

    // 步骤10: ECU复位 (0x11 01)
    if (UDS_SERVICE_RESULT_OK !=
        EcuResetReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, UDS_ECURESET_HARDRESET)) {
        OTA_LOG_E("%s step10: 0x11 01 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TMS step10: ECU reset success");
    SetEcuBrushProgress(80);

    return OTA_RET_OK;
}

/**
 * @brief TMS后编程步骤
 * 根据TMS升级流程文档:
 * (a) 0x10 03 - 扩展会话
 * (b) 0x28 00 03 - 恢复通信
 * (c) 0x85 01 - 允许DTC存储
 * (d) 0x10 01 - 默认会话
 * (e) 0x14 FF FF FF - 清除诊断信息
 */
static int FotaEcuPostProgrammingTms(FotaEcuPriv_t *priv)
{
    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 等待ECU重启完成
    sleep(5);
    SetEcuBrushProgress(82);

    // 步骤1: 进入扩展会话 (0x10 03) - 物理寻址(TMS无独立功能寻址)
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_EXTENDED)) {
        OTA_LOG_E("%s step1: 0x10 03 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TMS step1: Enter extended session success");
    usleep(100000);
    SetEcuBrushProgress(84);

    // 步骤2: 恢复通信 (0x28 00 03) - 物理寻址
    if (UDS_SERVICE_RESULT_OK !=
        CommunicationControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x00, 0x03)) {
        OTA_LOG_E("%s step2: 0x28 00 03 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TMS step2: Communication control success");
    usleep(100000);
    SetEcuBrushProgress(86);

    // 步骤3: 允许DTC存储 (0x85 01)
    if (UDS_SERVICE_RESULT_OK != ControlDTCSettingReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                      priv->info.channel, UDS_CTRLDTCSETTING_ON)) {
        OTA_LOG_E("%s step3: 0x85 01 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TMS step3: DTC setting on success");
    usleep(100000);
    SetEcuBrushProgress(88);

    // 步骤4: 进入默认会话 (0x10 01) - 物理寻址
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                             priv->info.channel, false, UDS_SESSION_DEFAULT_NOACK)) {
        OTA_LOG_E("%s step4: 0x10 01 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TMS step4: Enter default session success");
    usleep(100000);

    // 步骤5: 清除诊断信息 (0x14 FF FF FF)
    if (UDS_SERVICE_RESULT_OK !=
        ClearDtcReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0xFFFFFF)) {
        OTA_LOG_E("%s step5: 0x14 FF FF FF fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TMS step5: Clear all DTC success");
    SetEcuBrushProgress(90);

    return OTA_RET_OK;
}

/**
 * @brief TMS刷写入口函数
 * 整合预编程、编程、后编程三个步骤
 */
static int FotaEcuExecTms(FotaEcuPriv_t *priv, Flashinfo_t *driverFlashinfo, Flashinfo_t *softwareFlashinfo)
{
    int ret = OTA_RET_OK;
    OTA_LOG_I("Execute TMS[0x%04X] brush flow", priv->info.ecuId);

    SetEcuBrushProgress(5);  // 开始刷写

    // P1 缺陷修复: 任一步骤失败都要回 ECU 到默认会话 (0x10 01)
    // 原代码任何一步 return OTA_RET_ERR 都 直接退出，没机会下 0x10 01
    // ECU 会停留在 编程 / 扩展 会话 下次点火会出现 NRC 0x7E/0x7F

    // 执行TMS专用预编程
    if (OTA_RET_OK != FotaEcuPreviousProgrammingTms(priv)) {
        OTA_LOG_E("%s FotaEcuPreviousProgrammingTms fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
        goto cleanup;
    }
    SetEcuBrushProgress(15);  // 预编程完成

    // 执行TMS专用编程 (内部会上报20%~80%的进度)
    if (OTA_RET_OK != FotaEcuProgrammingTms(priv, driverFlashinfo, softwareFlashinfo)) {
        OTA_LOG_E("%s FotaEcuProgrammingTms fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
        goto cleanup;
    }

    // 执行TMS专用后编程
    if (OTA_RET_OK != FotaEcuPostProgrammingTms(priv)) {
        OTA_LOG_E("%s FotaEcuPostProgrammingTms fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
        goto cleanup;
    }
    SetEcuBrushProgress(90);  // 后编程完成
    return OTA_RET_OK;

cleanup:
    // 失败路径: 发送 0x10 01 (默认会话) 让 ECU 退出编程会话
    // 忽略返回值,因为失败路径上 ECU 可能已经失联
    (void)DiagnosticSessionControlReq(priv->srvHdl,
                                      priv->info.phyId, priv->info.resId,
                                      priv->info.channel, true,
                                      UDS_SESSION_DEFAULT);
    OTA_LOG_W("%s cleanup done: ECU returned to default session", __FUNCTION__);
    return ret;
}

/**
 * @brief TCU预编程流程
 * @param priv ECU私有数据结构指针
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note 按照TCU升级流程规范实现
 *       步骤1-8: 预编程步骤
 */
static int FotaEcuPreviousProgrammingTcu(FotaEcuPriv_t *priv)
{
    uint8_t data[128];
    uint32_t dataLen = 0;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 步骤1: 读取ECU信息 (0x22 F3 00) - 物理寻址
    dataLen = 0;
    if (UDS_SERVICE_RESULT_OK != ReadDataByIdentifierReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                         priv->info.channel, 0xF300, data, &dataLen)) {
        OTA_LOG_E("%s step1: 0x22 F300 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TCU step1: Read ECU info 0xF300 by physical address success, len=%d", dataLen);

    // 步骤2: 进入默认会话 (0x10 01)
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_DEFAULT)) {
        OTA_LOG_E("%s step2: 0x10 01 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TCU step2: Enter default session success");
    usleep(100000);

    // 步骤3: 进入扩展会话 (0x10 03)
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_EXTENDED)) {
        OTA_LOG_E("%s step3: 0x10 03 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TCU step3: Enter extended session success");
    usleep(100000);

    // 步骤4: 读取 ECU信息 (0x22 F1 A2) - 物理寻址
    if (UDS_SERVICE_RESULT_OK != ReadDataByIdentifierReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                         priv->info.channel, 0xF1A2, data, &dataLen)) {
        OTA_LOG_E("%s step4: 0x22 F1A2 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TCU step4: Read ECU info 0xF1A2 by physical address success");

    SetEcuBrushProgress(5);
    usleep(100000);

    // 步骤5: 预编程条件检查 (0x31 01 F0 02)
    dataLen = 0;
    if (UDS_SERVICE_RESULT_OK != RoutineControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel,
                                                   UDS_ROUTINECTRL_STARTROUTINE, 0xF002, data, &dataLen) ||
        (dataLen != 1) || ((data[0] != 0x00) && (data[0] != 0x01))) {
        OTA_LOG_E("%s step5: 0x31 01 F0 02 fail, status=0x%02X\n", __FUNCTION__, dataLen ? data[0] : 0xFF);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TCU step5: Pre-programming condition check success, status=0x%02X", data[0]);
    SetEcuBrushProgress(8);

    // 步骤6: DTC 设置 (0x85 02)
    if (UDS_SERVICE_RESULT_OK != ControlDTCSettingReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                      priv->info.channel, UDS_CTRLDTCSETTING_OFF)) {
        OTA_LOG_E("%s step6: 0x85 02 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TCU step6: DTC setting off success");    
    SetEcuBrushProgress(10);
    usleep(100000);

    // 步骤7: 通讯报文控制 (0x28 03 03) - 物理寻址
    if (UDS_SERVICE_RESULT_OK !=
        CommunicationControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x03, 0x03)) {
        OTA_LOG_E("%s step7: 0x28 03 03 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TCU step7: Communication control success");    
    SetEcuBrushProgress(12);
    usleep(100000);

    // 步骤8: 进入编程会话 (0x10 02)
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_PROGRAMMING)) {
        OTA_LOG_E("%s step8: 0x10 02 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TCU step8: Enter programming session success");    
    SetEcuBrushProgress(15);
    usleep(500000); // 等待500ms，确保ECU切换到编程模式
    return OTA_RET_OK;
}

/**
 * @brief TCU Boot安全访问 (0x27 61 - 0x27 62)
 * @param priv ECU私有数据结构指针
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note TCU种子和密钥都是4字节
 */
static int CheckAccessSecurityTcu(FotaEcuPriv_t *priv)
{
    uint8_t data[128] = {0};
    uint32_t len = 0;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 步骤10a: 0x27 0x61 请求种子
    if (UDS_SERVICE_RESULT_OK !=
        SecurityAccessReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x61, data, &len)) {
        OTA_LOG_E("%s 0x27 0x61 failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_I("%s 27 61 response: len=%d, data=[%02X %02X %02X %02X]\n", __FUNCTION__, len, data[0], data[1], data[2], data[3]);

    if (len < 4) {
        OTA_LOG_E("%s invalid seed length %d, expected 4 bytes.\n", __FUNCTION__, len);
        return OTA_RET_ERR;
    }

    // TCU种子是4字节: seed = (Seed[0]<<24) + (Seed[1]<<16) + (Seed[2]<<8) + Seed[3]
    uint32_t seed = (uint32_t)(((uint32_t)(data[0]) << 24) + ((uint32_t)(data[1]) << 16) +
                               ((uint32_t)(data[2]) << 8) + (uint32_t)(data[3]));
    uint32_t key = static_cast<uint32_t>(UdsCalcKeyTcu(seed));
    OTA_LOG_D("%s TCU key calculation, seed=0x%08X, key=0x%08X\n", __FUNCTION__, seed, key);

    // 密钥也是4字节: Key[0-3] = (key>>24, key>>16, key>>8, key)
    data[0] = (uint8_t)((key >> 24) & 0xFF);
    data[1] = (uint8_t)((key >> 16) & 0xFF);
    data[2] = (uint8_t)((key >> 8) & 0xFF);
    data[3] = (uint8_t)(key & 0xFF);
    len = 4;

    // 步骤10b: 0x27 0x62 发送密钥
    if (UDS_SERVICE_RESULT_OK !=
        SecurityAccessReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x62, data, &len)) {
        OTA_LOG_E("%s 0x27 0x62 failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TCU Boot security access success");

    return OTA_RET_OK;
}

/**
 * @brief TCU编程流程
 * @param priv ECU私有数据结构指针
 * @param driverFlashinfo 驱动Flash信息（擦除程序）
 * @param softwareFlashinfo 软件Flash信息（App程序）
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note 按照TCU升级流程规范实现
 *       步骤9-19: 正式编程
 */
static int FotaEcuProgrammingTcu(FotaEcuPriv_t *priv, Flashinfo_t *driverFlashinfo, Flashinfo_t *softwareFlashinfo)
{
    if (!priv || !softwareFlashinfo) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    // 步骤9: 10 60 已删除
    // 步骤10: Boot 安全访问 (0x27 61 - 0x27 62)
    if (OTA_RET_OK != CheckAccessSecurityTcu(priv)) {
        OTA_LOG_E("%s step10: CheckAccessSecurityTcu fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TCU step10: Boot security access success");
    SetEcuBrushProgress(20);

    // 步骤11-14: 写入擦除程序（根据段数平均分配进度20%~35%）
    if (driverFlashinfo != nullptr) {
        // 使用UpgradeSigment根据段数平均分配进度
        if (OTA_RET_OK != UpgradeSigment(priv, driverFlashinfo, false, 20, 35)) {
            OTA_LOG_E("%s step11-14: UpgradeSigment for erase program fail.\n", __FUNCTION__);
            return OTA_RET_ERR;
        }
        // 步骤14: 检查擦除程序完整性 (0x31 01 02 02 XX)
        if (OTA_RET_OK != CheckProgrammingIntegrity(priv, driverFlashinfo)) {
            OTA_LOG_E("%s step14: Check erase program integrity fail.\n", __FUNCTION__);
            return OTA_RET_ERR;
        }
        OTA_LOG_D("TCU step11-14: Download and verify erase program success");
    }

    // 步骤15: App区域擦除由 UpgradeSigment(..., true, ...) 内部执行一次(0x44格式)
    // 步骤16-19: 写入 App 程序（根据段数平均分配进度35%~75%）
    if (softwareFlashinfo != nullptr) {
        SetEcuBrushProgress(35);

        // 步骤16-19: 写入 App 程序（使用UpgradeSigment根据段数平均分配进度）
        if (OTA_RET_OK != UpgradeSigment(priv, softwareFlashinfo, true, 35, 75)) {
            OTA_LOG_E("%s step16-18: UpgradeSigment for App program fail.\n", __FUNCTION__);
            return OTA_RET_ERR;
        }
        // 步骤19: 检查 App 程序完整性 (0x31 01 02 02 XX)
        if (OTA_RET_OK != CheckProgrammingIntegrity(priv, softwareFlashinfo)) {
            OTA_LOG_E("%s step19: Check App program integrity fail.\n", __FUNCTION__);
            return OTA_RET_ERR;
        }
        OTA_LOG_D("TCU step15-19: Erase, download and verify App program success");
    }

    return OTA_RET_OK;
}

/**
 * @brief TCU后编程流程
 * @param priv ECU私有数据结构指针
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note 按照TCU升级流程规范实现
 *       步骤21-27: 后编程
 */
static int FotaEcuPostProgrammingTcu(FotaEcuPriv_t *priv)
{
    uint8_t data[128];
    uint32_t dataLen = 0;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 步骤21: 检查程序兼容性 (0x31 01 FF 01)
    dataLen = 0;
    if (UDS_SERVICE_RESULT_OK != RoutineControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel,
                                                   UDS_ROUTINECTRL_STARTROUTINE, 0xFF01, data, &dataLen) ||
        (dataLen != 1) || (data[0] != 0x00)) {
        OTA_LOG_E("%s step21: 0x31 01 FF 01 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TCU step21: Check program compatibility success");
    SetEcuBrushProgress(78);

    // 步骤22: 重启 ECU (0x11 01) - 物理寻址
    if (UDS_SERVICE_RESULT_OK !=
        EcuResetReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, UDS_ECURESET_HARDRESET)) {
        OTA_LOG_E("%s step22: 0x11 01 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TCU step22: ECU reset success");    
    SetEcuBrushProgress(80);
    sleep(5);  // 等待ECU重启

    // 步骤23: 读取 ECU相关信息 (0x22 XX XX)
    if (UDS_SERVICE_RESULT_OK != ReadDataByIdentifierReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                         priv->info.channel, 0xF300, data, &dataLen)) {
        OTA_LOG_E("%s step23: 0x22 F300 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TCU step23: Read ECU info success");
    SetEcuBrushProgress(82);

    // 步骤24: 进入扩展会话 (0x10 03) - 物理寻址
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_EXTENDED)) {
        OTA_LOG_E("%s step24: 0x10 03 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TCU step24: Enter extended session success");
    usleep(100000);
    SetEcuBrushProgress(84);

    // 步骤25: DTC 状态设置 (0x85 01)
    if (UDS_SERVICE_RESULT_OK != ControlDTCSettingReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                      priv->info.channel, UDS_CTRLDTCSETTING_ON)) {
        OTA_LOG_E("%s step25: 0x85 01 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TCU step25: DTC setting on success");
    usleep(100000);
    SetEcuBrushProgress(86);

    // 步骤26: 通讯报文控制 (0x28 00 03) - 物理寻址
    if (UDS_SERVICE_RESULT_OK !=
        CommunicationControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x00, 0x03)) {
        OTA_LOG_E("%s step26: 0x28 00 03 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TCU step26: Communication control success");
    usleep(100000);
    SetEcuBrushProgress(88);

    // 步骤27: 清除 ECU 所有故障码 (0x14 FF FF FF)
    if (UDS_SERVICE_RESULT_OK !=
        ClearDtcReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0xFFFFFF)) {
        OTA_LOG_E("%s step27: 0x14 FF FF FF fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TCU step27: Clear all DTC success");
    SetEcuBrushProgress(90);

    // 步骤28: 再次读取ECU信息, 与厂商工具日志保持一致
    dataLen = 0;
    if (UDS_SERVICE_RESULT_OK != ReadDataByIdentifierReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                         priv->info.channel, 0xF300, data, &dataLen)) {
        OTA_LOG_E("%s step28: 0x22 F300 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TCU step28: Read ECU info 0xF300 after flash success, len=%d", dataLen);

    return OTA_RET_OK;
}

/**
 * @brief 拆分TCU合并镜像: 0x60000000为RAM擦除程序, 0x80070000为App
 * @param combined 合并后的软件Flash信息
 * @param driver 输出擦除程序视图(不拥有data, 不单独释放)
 * @param app 输出App视图(不拥有data, 不单独释放)
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 */
static int SplitTcuCombinedImage(Flashinfo_t *combined, Flashinfo_t *driver, Flashinfo_t *app)
{
    const uint32_t ramDriverAddr = 0x60000000U;
    const uint32_t appAddr = 0x80070000U;
    int32_t driverIdx = -1;
    int32_t appIdx = -1;

    if (!combined || !driver || !app || (combined->segNum < 2)) {
        return OTA_RET_ERR;
    }

    for (uint32_t i = 0; i < combined->segNum; i++) {
        if (combined->segment[i].memaddr == ramDriverAddr) {
            driverIdx = (int32_t)i;
        } else if (combined->segment[i].memaddr == appAddr) {
            appIdx = (int32_t)i;
        }
    }

    if ((driverIdx < 0) || (appIdx < 0)) {
        OTA_LOG_E("%s cannot find driver(0x%08X) or app(0x%08X) segment, segNum=%u", __FUNCTION__, ramDriverAddr,
                  appAddr, combined->segNum);
        return OTA_RET_ERR;
    }

    memset(driver, 0, sizeof(*driver));
    driver->segNum = 1;
    driver->segment[0] = combined->segment[driverIdx];
    driver->data = combined->segment[driverIdx].memdata;
    driver->dataLen = combined->segment[driverIdx].memsize;

    memset(app, 0, sizeof(*app));
    app->segNum = 1;
    app->segment[0] = combined->segment[appIdx];
    app->data = combined->segment[appIdx].memdata;
    app->dataLen = combined->segment[appIdx].memsize;

    OTA_LOG_I("%s driver: addr=0x%08X size=0x%08X, app: addr=0x%08X size=0x%08X", __FUNCTION__,
              driver->segment[0].memaddr, driver->segment[0].memsize, app->segment[0].memaddr,
              app->segment[0].memsize);

    return OTA_RET_OK;
}

/**
 * @brief TCU刷写流程
 * @param priv ECU私有数据结构指针
 * @param driverFlashinfo 驱动Flash信息（擦除程序）
 * @param softwareFlashinfo 软件Flash信息（App程序）
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note 按照TCU升级流程规范实现
 */
static int FotaEcuExecTcu(FotaEcuPriv_t *priv, Flashinfo_t *driverFlashinfo, Flashinfo_t *softwareFlashinfo)
{
    OTA_LOG_I("Execute TCU[0x%04X] brush flow", priv->info.ecuId);
    
    SetEcuBrushProgress(2);  // 开始刷写
    
    // 步骤1-8: 预编程
    if (OTA_RET_OK != FotaEcuPreviousProgrammingTcu(priv)) {
        OTA_LOG_E("%s FotaEcuPreviousProgrammingTcu fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    SetEcuBrushProgress(20);  // 预编程完成

    // 厂商工具对擦除程序/App均为整包一次34下载, 合并64KB切分段
    MergeContiguousFlashSegments(driverFlashinfo);
    MergeContiguousFlashSegments(softwareFlashinfo);

    Flashinfo_t driverView;
    Flashinfo_t appView;
    Flashinfo_t *driver = driverFlashinfo;
    Flashinfo_t *app = softwareFlashinfo;

    /* 合并镜像场景: App文件同时包含
     *   0x60000000/0x18000 (RAM擦除程序, 不擦除)
     *   0x80070000/0x390000 (App, 需要擦除)
     * 按地址拆成Driver/App两个视图, 分别下载和校验 */
    if ((driver == nullptr) && (softwareFlashinfo != nullptr) && (softwareFlashinfo->segNum >= 2)) {
        if (OTA_RET_OK != SplitTcuCombinedImage(softwareFlashinfo, &driverView, &appView)) {
            OTA_LOG_E("%s split TCU combined image fail.\n", __FUNCTION__);
            return OTA_RET_ERR;
        }
        driver = &driverView;
        app = &appView;
    }

    // 步骤9-19: 正式编程
    if (OTA_RET_OK != FotaEcuProgrammingTcu(priv, driver, app)) {
        OTA_LOG_E("%s FotaEcuProgrammingTcu fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    // 进度由FotaEcuProgrammingTcu内部上报
    
    // 步骤21-27: 后编程
    if (OTA_RET_OK != FotaEcuPostProgrammingTcu(priv)) {
        OTA_LOG_E("%s FotaEcuPostProgrammingTcu fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    SetEcuBrushProgress(95);  // 后编程完成
    
    OTA_LOG_I("TCU[0x%04X] brush flow completed successfully", priv->info.ecuId);
    return OTA_RET_OK;
}

/**
 * @brief MCU1安全访问 (0x27 0x03 - 0x27 0x04)
 * @param priv ECU私有数据结构指针
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note 汇川MCU1使用0x27 0x03请求种子/0x27 0x04校验密钥
 *       种子和密钥均为4字节，密钥算法 UdsCalcKeyMcu1
 */
static int CheckAccessSecurityMcu1(FotaEcuPriv_t *priv)
{
    uint8_t data[128] = {0};
    uint32_t len = 0;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 0x27 0x03 请求种子
    if (UDS_SERVICE_RESULT_OK !=
        SecurityAccessReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x03, data, &len)) {
        OTA_LOG_E("%s 0x27 0x03 failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_I("%s 27 03 response: len=%d, data=[%02X %02X %02X %02X]\n", __FUNCTION__, len,
              data[0], data[1], data[2], data[3]);

    if (len < 4) {
        OTA_LOG_E("%s invalid seed length %d, expected 4 bytes.\n", __FUNCTION__, len);
        return OTA_RET_ERR;
    }

    // MCU1种子是4字节
    uint32_t seed = (uint32_t)(((uint32_t)(data[0]) << 24) + ((uint32_t)(data[1]) << 16) +
                               ((uint32_t)(data[2]) << 8) + (uint32_t)(data[3]));
    uint32_t key = UdsCalcKeyMcu1(seed);
    OTA_LOG_D("%s MCU1 key calculation, seed=0x%08X, key=0x%08X\n", __FUNCTION__, seed, key);

    // 密钥也是4字节
    data[0] = (uint8_t)((key >> 24) & 0xFF);
    data[1] = (uint8_t)((key >> 16) & 0xFF);
    data[2] = (uint8_t)((key >> 8) & 0xFF);
    data[3] = (uint8_t)(key & 0xFF);
    len = 4;

    // 0x27 0x04 发送密钥
    if (UDS_SERVICE_RESULT_OK !=
        SecurityAccessReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x04, data, &len)) {
        OTA_LOG_E("%s 0x27 0x04 failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 security access success");

    return OTA_RET_OK;
}

/**
 * @brief MCU1 Level3安全访问 (0x27 0x09 - 0x27 0x0A)
 * @param priv ECU私有数据结构指针
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note PDU/ACM使用0x27 0x09请求种子/0x27 0x0A校验密钥
 *       种子和密钥均为4字节，密钥算法与MCU1一致(UdsCalcKeyMcu1)
 */
static int CheckAccessSecurityMcu1Level3(FotaEcuPriv_t *priv)
{
    uint8_t data[128] = {0};
    uint32_t len = 0;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 0x27 0x09 请求种子
    if (UDS_SERVICE_RESULT_OK !=
        SecurityAccessReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x09, data, &len)) {
        OTA_LOG_E("%s 0x27 0x09 failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_I("%s 27 09 response: len=%d, data=[%02X %02X %02X %02X]\n", __FUNCTION__, len,
              data[0], data[1], data[2], data[3]);

    if (len < 4) {
        OTA_LOG_E("%s invalid seed length %d, expected 4 bytes.\n", __FUNCTION__, len);
        return OTA_RET_ERR;
    }

    uint32_t seed = (uint32_t)(((uint32_t)(data[0]) << 24) + ((uint32_t)(data[1]) << 16) +
                               ((uint32_t)(data[2]) << 8) + (uint32_t)(data[3]));
    uint32_t key = UdsCalcKeyMcu1(seed);
    OTA_LOG_D("%s MCU1_PDU key calculation, seed=0x%08X, key=0x%08X\n", __FUNCTION__, seed, key);

    data[0] = (uint8_t)((key >> 24) & 0xFF);
    data[1] = (uint8_t)((key >> 16) & 0xFF);
    data[2] = (uint8_t)((key >> 8) & 0xFF);
    data[3] = (uint8_t)(key & 0xFF);
    len = 4;

    // 0x27 0x0A 发送密钥
    if (UDS_SERVICE_RESULT_OK !=
        SecurityAccessReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x0A, data, &len)) {
        OTA_LOG_E("%s 0x27 0x0A failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 Level3 security access success");

    return OTA_RET_OK;
}

/**
 * @brief MCU1安全访问Level1 (0x27 0x01 - 0x27 0x02)
 * @param priv ECU私有数据结构指针
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note 实测KD26_ACM流程：进入编程会话前先执行27 01/02 Level1
 *       密钥算法为 UdsCalcKeyMcu1(seed) ^ 0x71018870
 */
static int CheckAccessSecurityMcu1Level1(FotaEcuPriv_t *priv)
{
    uint8_t data[128] = {0};
    uint32_t len = 0;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 0x27 0x01 请求种子
    if (UDS_SERVICE_RESULT_OK !=
        SecurityAccessReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x01, data, &len)) {
        OTA_LOG_E("%s 0x27 0x01 failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_I("%s 27 01 response: len=%d, data=[%02X %02X %02X %02X]\n", __FUNCTION__, len,
              data[0], data[1], data[2], data[3]);

    if (len < 4) {
        OTA_LOG_E("%s invalid seed length %d, expected 4 bytes.\n", __FUNCTION__, len);
        return OTA_RET_ERR;
    }

    uint32_t seed = (uint32_t)(((uint32_t)(data[0]) << 24) + ((uint32_t)(data[1]) << 16) +
                               ((uint32_t)(data[2]) << 8) + (uint32_t)(data[3]));
    uint32_t key = UdsCalcKeyMcu1Level1(seed);
    OTA_LOG_D("%s MCU1 Level1 key calculation, seed=0x%08X, key=0x%08X\n", __FUNCTION__, seed, key);

    data[0] = (uint8_t)((key >> 24) & 0xFF);
    data[1] = (uint8_t)((key >> 16) & 0xFF);
    data[2] = (uint8_t)((key >> 8) & 0xFF);
    data[3] = (uint8_t)(key & 0xFF);
    len = 4;

    // 0x27 0x02 发送密钥
    if (UDS_SERVICE_RESULT_OK !=
        SecurityAccessReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x02, data, &len)) {
        OTA_LOG_E("%s 0x27 0x02 failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 Level1 security access success");

    return OTA_RET_OK;
}

/**
 * @brief MCU1检查编程完整性 (0x31 0x01 0xF1A0)
 * @param priv ECU私有数据结构指针
 * @param dataPtr 待校验数据指针
 * @param dataLen 待校验数据长度
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note 汇川MCU1使用Routine 0xF1A0检查程序完整性，发送CRC32(4字节)
 */
static int CheckProgrammingIntegrityMcu1(FotaEcuPriv_t *priv, const uint8_t *dataPtr, uint32_t dataLen)
{
    uint8_t data[32];
    uint32_t len = 0;

    if (!priv || !dataPtr || !dataLen) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    uint32_t crc32 = 0;
    const char *crcType = "reflected";

    /* TM1/TM2实测使用非反射型CRC:
     * TM1成功日志(F1A0=0xB784EE1B): 同一份0x57B8A镜像, 反射型=0xE88253F2, 非反射型=0xB784EE1B
     * TM2与TM1保持一致, 使用非反射型
     * 其他MCU1(PDU/ACM等)仍使用反射型CRC */
    if ((priv->info.ecuId == ECU_ID_MCU1_TM1) || (priv->info.ecuId == ECU_ID_MCU1_TM2)) {
        crc32 = UdsCalcCRC32Vcu(dataPtr, dataLen);
        crcType = "non-reflected";
    } else {
        crc32 = UdsCalcCRC32(dataPtr, dataLen);
    }

    OTA_LOG_D("%s ecuId=0x%04X crcType=%s, len=%u, crc=0x%08X\n", __FUNCTION__, priv->info.ecuId, crcType, dataLen,
              crc32);

    data[0] = (crc32 >> 24) & 0xff;
    data[1] = (crc32 >> 16) & 0xff;
    data[2] = (crc32 >> 8) & 0xff;
    data[3] = crc32 & 0xff;
    len = 4;

    if ((UDS_SERVICE_RESULT_OK != RoutineControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel,
                                                     UDS_ROUTINECTRL_STARTROUTINE, 0xF1A0, data, &len)) ||
        (len != 1) || (data[0] != 0x00)) {
        OTA_LOG_E("%s RoutineControlReq fail 0x01 0xF1A0.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    return OTA_RET_OK;
}

/**
 * @brief MCU1预编程流程 (汇川)
 * @param priv ECU私有数据结构指针
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note 预编程步骤1-6:
 *       1. 读取ECU硬件版本 (0x22 F193) - 物理寻址
 *       2. 读取ECU软件版本 (0x22 F195) - 物理寻址
 *       3. 扩展会话 (0x10 03) - 功能寻址
 *       4. 检查预编程条件 (0x31 01 FF 02) - 物理寻址
 *       5. 禁用DTC (0x85 02) - 功能寻址
 *       6. 禁用非诊断通讯 (0x28 03 03) - 功能寻址
 */
static int FotaEcuPreviousProgrammingMcu1(FotaEcuPriv_t *priv)
{
    uint8_t data[32];
    uint32_t dataLen = 0;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
#if 0
    // 步骤1: 读取ECU硬件版本 (0x22 F1 93)
    if (UDS_SERVICE_RESULT_OK != ReadDataByIdentifierReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                         priv->info.channel, 0xF193, data, &dataLen)) {
        OTA_LOG_E("%s step1: 0x22 F193 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step1: Read hardware version success");
    SetEcuBrushProgress(3);

    // 步骤2: 读取ECU软件版本 (0x22 F1 95)
    if (UDS_SERVICE_RESULT_OK != ReadDataByIdentifierReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                         priv->info.channel, 0xF195, data, &dataLen)) {
        OTA_LOG_E("%s step2: 0x22 F195 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step2: Read software version success");
    SetEcuBrushProgress(5);
# endif
    // 步骤3: 进入扩展会话 (0x10 03) - 功能寻址
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.funcId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_EXTENDED)) {
        OTA_LOG_E("%s step3: 0x10 03 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step3: Enter extended session success");
    SetEcuBrushProgress(7);
    usleep(100000);

    // 步骤4: 检查预编程条件 (0x31 01 FF 02) - 物理寻址
    // 规范要求: ECU未收到此例程则拒绝后续10 02进入Bootloader
    dataLen = 0;
    if (UDS_SERVICE_RESULT_OK != RoutineControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel,
                                                   UDS_ROUTINECTRL_STARTROUTINE, 0xFF02, data, &dataLen) ||
        (dataLen != 1) || (data[0] != 0x00)) {
        OTA_LOG_E("%s step4: 0x31 01 FF 02 fail, pre-programming condition check failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step4: Pre-programming condition check success");
    SetEcuBrushProgress(9);

    // 步骤5: 禁用DTC (0x85 02) - 功能寻址
    if (UDS_SERVICE_RESULT_OK != ControlDTCSettingReq(priv->srvHdl, priv->info.funcId, priv->info.resId,
                                                      priv->info.channel, UDS_CTRLDTCSETTING_OFF)) {
        OTA_LOG_E("%s step5: 0x85 02 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step5: DTC setting off success");
    SetEcuBrushProgress(10);
    usleep(100000);

    // 步骤6: 禁用非诊断通讯 (0x28 03 03) - 功能寻址
    if (UDS_SERVICE_RESULT_OK !=
        CommunicationControlReq(priv->srvHdl, priv->info.funcId, priv->info.resId, priv->info.channel, 0x03, 0x03)) {
        OTA_LOG_E("%s step6: 0x28 03 03 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step6: Communication control off success");
    SetEcuBrushProgress(12);
    usleep(100000);

    return OTA_RET_OK;
}

/**
 * @brief MCU1_PDU预编程流程 (含进入编程会话和安全访问)
 * @param priv ECU私有数据结构指针
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note PDU实际刷写顺序:
 *       1. 10 01 默认会话 (物理)
 *       2. 10 03 扩展会话 (物理)
 *       3. 28 03 03 通讯控制 (物理)
 *       4. 85 02 关闭DTC (物理)
 *       5. 31 01 FF02 预编程条件检查 (物理)
 *       6. 10 02 编程会话 (物理)
 *       7. 27 09/0A 安全访问 (物理)
 */
static int FotaEcuPreviousProgrammingMcu1Pdu(FotaEcuPriv_t *priv)
{
    uint8_t data[32] = {0};
    uint32_t dataLen = 0;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 步骤1: 进入默认会话 (0x10 01) - 物理寻址
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_DEFAULT)) {
        OTA_LOG_E("%s step1: 0x10 01 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1_PDU step1: Enter default session success");
    SetEcuBrushProgress(3);
    usleep(100000);

    // 步骤2: 进入扩展会话 (0x10 03) - 物理寻址
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_EXTENDED)) {
        OTA_LOG_E("%s step2: 0x10 03 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1_PDU step2: Enter extended session success");
    SetEcuBrushProgress(5);
    usleep(100000);

    // 步骤3: 禁用非诊断通讯 (0x28 03 03) - 物理寻址
    if (UDS_SERVICE_RESULT_OK !=
        CommunicationControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x03, 0x03)) {
        OTA_LOG_E("%s step3: 0x28 03 03 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1_PDU step3: Communication control off success");
    SetEcuBrushProgress(8);
    usleep(100000);

    // 步骤4: 禁用DTC (0x85 02) - 物理寻址
    if (UDS_SERVICE_RESULT_OK != ControlDTCSettingReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                      priv->info.channel, UDS_CTRLDTCSETTING_OFF)) {
        OTA_LOG_E("%s step4: 0x85 02 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1_PDU step4: DTC setting off success");
    SetEcuBrushProgress(11);
    usleep(100000);

    // 步骤5: 检查预编程条件 (0x31 01 FF 02) - 物理寻址
    dataLen = 0;
    if (UDS_SERVICE_RESULT_OK != RoutineControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel,
                                                   UDS_ROUTINECTRL_STARTROUTINE, 0xFF02, data, &dataLen) ||
        (dataLen != 1) || (data[0] != 0x00)) {
        OTA_LOG_E("%s step5: 0x31 01 FF 02 fail, pre-programming condition check failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1_PDU step5: Pre-programming condition check success");
    SetEcuBrushProgress(14);
    usleep(100000);

    // 步骤6: 切换主编程会话 (0x10 02) - 物理寻址
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_PROGRAMMING)) {
        OTA_LOG_E("%s step6: 0x10 02 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1_PDU step6: Enter programming session success");
    SetEcuBrushProgress(18);
    usleep(500000); // 等待ECU切换到编程模式

    // 步骤7: 安全访问 (0x27 09/0A)
    if (OTA_RET_OK != CheckAccessSecurityMcu1Level3(priv)) {
        OTA_LOG_E("%s step7: CheckAccessSecurityMcu1Level3 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1_PDU step7: Security access success");
    SetEcuBrushProgress(22);

    return OTA_RET_OK;
}

/**
 * @brief MCU1通用预编程流程 (含Level1/Level3安全访问)
 * @param priv ECU私有数据结构指针
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note 按KD26_TM/ACM.log实际顺序:
 *       1. 10 01 默认会话 (物理)
 *       2. 10 03 扩展会话 (物理)
 *       3. 28 03 03 通讯控制 (物理)
 *       4. 85 02 关闭DTC (物理)
 *       5. 27 01/02 Level1安全访问 (物理)
 *       6. 31 01 FF02 预编程条件检查 (物理)
 *       7. 10 02 编程会话 (物理)
 *       8. 27 09/0A Level3安全访问 (物理)
 */
static int FotaEcuPreviousProgrammingMcu1Common(FotaEcuPriv_t *priv)
{
    uint8_t data[32] = {0};
    uint32_t dataLen = 0;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 步骤1: 进入默认会话 (0x10 01) - 物理寻址
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_DEFAULT)) {
        OTA_LOG_E("%s step1: 0x10 01 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step1: Enter default session success");
    SetEcuBrushProgress(3);
    usleep(100000);

    // 步骤2: 进入扩展会话 (0x10 03) - 物理寻址
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_EXTENDED)) {
        OTA_LOG_E("%s step2: 0x10 03 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step2: Enter extended session success");
    SetEcuBrushProgress(5);
    usleep(100000);

    // 步骤3: 禁用非诊断通讯 (0x28 03 03) - 物理寻址
    if (UDS_SERVICE_RESULT_OK !=
        CommunicationControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0x03, 0x03)) {
        OTA_LOG_E("%s step3: 0x28 03 03 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step3: Communication control off success");
    SetEcuBrushProgress(8);
    usleep(100000);

    // 步骤4: 禁用DTC (0x85 02) - 物理寻址
    if (UDS_SERVICE_RESULT_OK != ControlDTCSettingReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                      priv->info.channel, UDS_CTRLDTCSETTING_OFF)) {
        OTA_LOG_E("%s step4: 0x85 02 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step4: DTC setting off success");
    SetEcuBrushProgress(11);
    usleep(100000);

    // 步骤5: Level1安全访问 (0x27 01/02)
    if (OTA_RET_OK != CheckAccessSecurityMcu1Level1(priv)) {
        OTA_LOG_E("%s step5: CheckAccessSecurityMcu1Level1 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step5: Level1 security access success");
    SetEcuBrushProgress(14);

    // 步骤6: 检查预编程条件 (0x31 01 FF 02) - 物理寻址
    dataLen = 0;
    if (UDS_SERVICE_RESULT_OK != RoutineControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel,
                                                   UDS_ROUTINECTRL_STARTROUTINE, 0xFF02, data, &dataLen) ||
        (dataLen != 1) || (data[0] != 0x00)) {
        OTA_LOG_E("%s step6: 0x31 01 FF 02 fail, pre-programming condition check failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step6: Pre-programming condition check success");
    SetEcuBrushProgress(17);
    usleep(100000);

    // 步骤7: 切换主编程会话 (0x10 02) - 物理寻址
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_PROGRAMMING)) {
        OTA_LOG_E("%s step7: 0x10 02 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step7: Enter programming session success");
    SetEcuBrushProgress(20);
    usleep(500000); // 等待ECU切换到编程模式

    // 步骤8: Level3安全访问 (0x27 09/0A)
    if (OTA_RET_OK != CheckAccessSecurityMcu1Level3(priv)) {
        OTA_LOG_E("%s step8: CheckAccessSecurityMcu1Level3 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step8: Level3 security access success");
    SetEcuBrushProgress(22);

    return OTA_RET_OK;
}

/**
 * @brief MCU1通用后编程流程
 * @param priv ECU私有数据结构指针
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note 按KD26_ACM.log: 11 01复位后等待1000ms, 只发送14 FF FF FF清除DTC
 */
static int FotaEcuPostProgrammingMcu1Common(FotaEcuPriv_t *priv)
{
    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    usleep(1000 * 1000);

    if (OTA_RET_OK != EcuClearDtc(priv)) {
        OTA_LOG_E("%s step: 0x14 FF FF FF fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 post programming: Clear DTC success");

    return OTA_RET_OK;
}

/**
 * @brief MCU1主编程公共流程 (指纹/下载/校验/依赖/复位)
 * @param priv ECU私有数据结构指针
 * @param driverFlashinfo 驱动Flash信息
 * @param softwareFlashinfo 软件Flash信息
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note 调用前必须已进入编程会话并完成安全访问
 *       4. 写入指纹信息 (0x2E F184) - 由writeFingerprint控制(PDU/ACM不支持)
 *       5-6. 下载驱动 + 检查驱动完整性 (0x31 01 F1A0)
 *       7-10. 循环: 擦除(FF00) -> 下载 -> 完整性校验(F1A0)
 *       11. 检查程序依赖性 (0x31 01 FF01)
 *       12. ECU硬件重置 (0x11 01)
 */
static int FotaEcuProgrammingMcu1Body(FotaEcuPriv_t *priv, Flashinfo_t *driverFlashinfo,
                                      Flashinfo_t *softwareFlashinfo, bool writeFingerprint)
{
    uint8_t data[32];
    uint32_t dataLen = 0;

    if (!priv || !softwareFlashinfo) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 步骤4: 写入指纹信息 (0x2E F184)
    if (writeFingerprint) {
        if (OTA_RET_OK != WriteFingerprintInfo(priv)) {
            OTA_LOG_E("%s step4: 0x2E F184 fail.\n", __FUNCTION__);
            return OTA_RET_ERR;
        }
        OTA_LOG_D("MCU1 step4: Write fingerprint success");
    }
    SetEcuBrushProgress(25);

    // 步骤5-6: 下载驱动程序 + 检查驱动完整性
    if (driverFlashinfo != nullptr) {
        // 下载驱动 (进度25%~35%)
        if (OTA_RET_OK != UpgradeSigment(priv, driverFlashinfo, false, 25, 35)) {
            OTA_LOG_E("%s step5: UpgradeSigment for driver fail.\n", __FUNCTION__);
            return OTA_RET_ERR;
        }
        // 检查驱动完整性 (0x31 01 F1A0)
        if (OTA_RET_OK != CheckProgrammingIntegrityMcu1(priv, driverFlashinfo->data, driverFlashinfo->dataLen)) {
            OTA_LOG_E("%s step6: Check driver integrity fail.\n", __FUNCTION__);
            return OTA_RET_ERR;
        }
        OTA_LOG_D("MCU1 step5-6: Download driver and verify success");
    }

    // 步骤7-10: 下载应用软件 (循环: 擦除 -> 下载 -> 完整性校验, 进度35%~75%)
    {
        int segNum = softwareFlashinfo->segNum;
        uint8_t segStart = 35;
        uint8_t segEnd = 75;
        for (int i = 0; i < segNum; i++) {
            uint8_t progStart = segStart + (segEnd - segStart) * i / segNum;
            uint8_t progEnd = segStart + (segEnd - segStart) * (i + 1) / segNum;

            // 步骤7: 擦除逻辑单元 (0x31 01 FF00)
            if (OTA_RET_OK != EraseFlash(priv, softwareFlashinfo->segment[i].memaddr,
                                         softwareFlashinfo->segment[i].memsize)) {
                OTA_LOG_E("%s step7: 0x31 01 FF00 fail for segment %d.\n", __FUNCTION__, i);
                return OTA_RET_ERR;
            }
            OTA_LOG_D("MCU1 step7: Erase segment %d success", i);

            // 步骤8: 下载数据 (0x34/0x36/0x37)
            uint32_t blockLen = 0;
            if (OTA_RET_OK != DownloadReq(priv, softwareFlashinfo->segment[i].memaddr,
                                          softwareFlashinfo->segment[i].memsize, &blockLen)) {
                OTA_LOG_E("%s step8: DownloadReq fail for segment %d.\n", __FUNCTION__, i);
                return OTA_RET_ERR;
            }
            /* Z20K148 SDK接收缓冲区有限, blockLen超1024会导致MCU重启, 强制限制 */
            if (blockLen > 1024) {
                blockLen = 1024;
            }
            if (OTA_RET_OK != TransferData(priv, softwareFlashinfo->segment[i].memdata,
                                           softwareFlashinfo->segment[i].memsize, blockLen,
                                           progStart, progEnd)) {
                OTA_LOG_E("%s step8: TransferData fail for segment %d.\n", __FUNCTION__, i);
                return OTA_RET_ERR;
            }
            if (OTA_RET_OK != DownloadExit(priv)) {
                OTA_LOG_E("%s step8: DownloadExit fail for segment %d.\n", __FUNCTION__, i);
                return OTA_RET_ERR;
            }

            // 步骤9: 检查程序完整性 (0x31 01 F1A0)
            if (OTA_RET_OK != CheckProgrammingIntegrityMcu1(priv, softwareFlashinfo->segment[i].memdata,
                                                            softwareFlashinfo->segment[i].memsize)) {
                OTA_LOG_E("%s step9: Check program integrity fail for segment %d.\n", __FUNCTION__, i);
                return OTA_RET_ERR;
            }
            OTA_LOG_D("MCU1 step7-9: Erase, download and verify segment %d success", i);
        }
    }
    SetEcuBrushProgress(75);

    // 步骤11: 检查程序依赖性 (0x31 01 FF01)
    dataLen = 0;
    if (UDS_SERVICE_RESULT_OK != RoutineControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel,
                                                   UDS_ROUTINECTRL_STARTROUTINE, 0xFF01, data, &dataLen) ||
        (dataLen != 1) || (data[0] != 0x00)) {
        OTA_LOG_E("%s step11: 0x31 01 FF01 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step11: Check program dependency success");
    SetEcuBrushProgress(78);

    // 步骤12: ECU硬件重置 (0x11 01)
    if (UDS_SERVICE_RESULT_OK !=
        EcuResetReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, UDS_ECURESET_HARDRESET)) {
        OTA_LOG_E("%s step12: 0x11 01 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step12: ECU reset success");
    SetEcuBrushProgress(80);

    return OTA_RET_OK;
}

/**
 * @brief MCU1主编程流程 (汇川)
 * @param priv ECU私有数据结构指针
 * @param driverFlashinfo 驱动Flash信息
 * @param softwareFlashinfo 软件Flash信息
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note 主编程步骤1-3:
 *       1. 切换主编程会话 (0x10 02)
 *       2-3. 安全访问 (0x27 03/04)
 *       之后执行公共下载流程
 */
static int FotaEcuProgrammingMcu1(FotaEcuPriv_t *priv, Flashinfo_t *driverFlashinfo, Flashinfo_t *softwareFlashinfo)
{
    if (!priv || !softwareFlashinfo) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 步骤1: 切换主编程会话 (0x10 02) - 物理寻址
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_PROGRAMMING)) {
        OTA_LOG_E("%s step1: 0x10 02 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step1: Enter programming session success");
    usleep(500000); // 等待ECU切换到编程模式
    SetEcuBrushProgress(20);

    // 步骤2-3: 安全访问 (0x27 03/04)
    if (OTA_RET_OK != CheckAccessSecurityMcu1(priv)) {
        OTA_LOG_E("%s step2-3: CheckAccessSecurityMcu1 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step2-3: Security access success");
    SetEcuBrushProgress(22);

    return FotaEcuProgrammingMcu1Body(priv, driverFlashinfo, softwareFlashinfo, true);
}

/**
 * @brief MCU1后编程流程 (汇川)
 * @param priv ECU私有数据结构指针
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note 后编程步骤(重同步CAN网络):
 *       1. 扩展会话 (0x10 03) - 功能寻址
 *       2. 恢复非诊断通讯 (0x28 00 01) - 功能寻址
 *       3. 使能DTC存储 (0x85 01) - 功能寻址
 *       4. 默认会话 (0x10 01) - 功能寻址
 *       5. 清除诊断信息 (0x14 FF FF FF) - 物理寻址
 */
static int FotaEcuPostProgrammingMcu1(FotaEcuPriv_t *priv)
{
    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 等待ECU重启完成
    sleep(5);
    SetEcuBrushProgress(82);

    // 步骤1: 进入扩展会话 (0x10 03) - 功能寻址
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.funcId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_EXTENDED)) {
        OTA_LOG_E("%s step1: 0x10 03 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step1: Enter extended session success");
    usleep(100000);
    SetEcuBrushProgress(84);

    // 步骤2: 恢复非诊断通讯 (0x28 00 01) - 功能寻址
    if (UDS_SERVICE_RESULT_OK !=
        CommunicationControlReq(priv->srvHdl, priv->info.funcId, priv->info.resId, priv->info.channel, 0x00, 0x01)) {
        OTA_LOG_E("%s step2: 0x28 00 01 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step2: Communication control on success");
    usleep(100000);
    SetEcuBrushProgress(86);

    // 步骤3: 使能DTC存储 (0x85 01) - 功能寻址
    if (UDS_SERVICE_RESULT_OK != ControlDTCSettingReq(priv->srvHdl, priv->info.funcId, priv->info.resId,
                                                      priv->info.channel, UDS_CTRLDTCSETTING_ON)) {
        OTA_LOG_E("%s step3: 0x85 01 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step3: DTC setting on success");
    usleep(100000);
    SetEcuBrushProgress(88);

    // 步骤4: 切换默认会话 (0x10 01) - 功能寻址
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.funcId, priv->info.resId,
                                                             priv->info.channel, false, UDS_SESSION_DEFAULT_NOACK)) {
        OTA_LOG_E("%s step4: 0x10 01 fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step4: Enter default session success");
    usleep(100000);

    // 步骤5: 清除诊断信息 (0x14 FF FF FF) - 物理寻址
    if (UDS_SERVICE_RESULT_OK !=
        ClearDtcReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel, 0xFFFFFF)) {
        OTA_LOG_E("%s step5: 0x14 FF FF FF fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    OTA_LOG_D("MCU1 step5: Clear all DTC success");
    SetEcuBrushProgress(90);

    return OTA_RET_OK;
}

/**
 * @brief 旧版MCU刷写流程 (当前由MCU2调用)
 * @param priv ECU私有数据结构指针
 * @param driverFlashinfo 驱动Flash信息
 * @param softwareFlashinfo 软件Flash信息
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note MCU1系列已切换到FotaEcuExecMcu1Common; 该流程保留给MCU2使用
 */
static int FotaEcuExecMcu1(FotaEcuPriv_t *priv, Flashinfo_t *driverFlashinfo, Flashinfo_t *softwareFlashinfo)
{
    int ret = OTA_RET_OK;
    OTA_LOG_I("Execute MCU1[0x%04X] brush flow", priv->info.ecuId);

    SetEcuBrushProgress(2);  // 开始刷写

    // 步骤1-5: 预编程
    if (OTA_RET_OK != FotaEcuPreviousProgrammingMcu1(priv)) {
        OTA_LOG_E("%s FotaEcuPreviousProgrammingMcu1 fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
        goto cleanup;
    }
    SetEcuBrushProgress(15);  // 预编程完成

    // 步骤1-12: 主编程
    if (OTA_RET_OK != FotaEcuProgrammingMcu1(priv, driverFlashinfo, softwareFlashinfo)) {
        OTA_LOG_E("%s FotaEcuProgrammingMcu1 fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
        goto cleanup;
    }

    // 后编程: 重同步CAN网络
    if (OTA_RET_OK != FotaEcuPostProgrammingMcu1(priv)) {
        OTA_LOG_E("%s FotaEcuPostProgrammingMcu1 fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
        goto cleanup;
    }
    SetEcuBrushProgress(95);  // 后编程完成

    OTA_LOG_I("MCU1[0x%04X] brush flow completed successfully", priv->info.ecuId);
    return OTA_RET_OK;

cleanup:
    // 失败路径: 发送 0x10 01 让 ECU 退出编程会话
    (void)DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                      priv->info.channel, true, UDS_SESSION_DEFAULT);
    OTA_LOG_W("%s cleanup done: ECU returned to default session", __FUNCTION__);
    return ret;
}

/**
 * @brief 合并地址连续且数据连续的Flash分段
 * @param info Flash信息
 * @note GetFlashInfoFromFile()会按64KB切分下载段, 但PDU/ACM参考日志
 *       都是整包一次擦除/34下载/31 F1A0校验, 因此刷写前需要把切分段合并
 */
static void MergeContiguousFlashSegments(Flashinfo_t *info)
{
    if (!info || info->segNum <= 1) {
        return;
    }

    uint32_t writeIdx = 0;
    for (uint32_t i = 1; i < info->segNum; i++) {
        FlashSegmentInfo_t *prev = &info->segment[writeIdx];
        FlashSegmentInfo_t *cur = &info->segment[i];

        if (((prev->memaddr + prev->memsize) == cur->memaddr) &&
            ((prev->memdata + prev->memsize) == cur->memdata)) {
            prev->memsize += cur->memsize;
        } else {
            writeIdx++;
            if (writeIdx != i) {
                info->segment[writeIdx] = *cur;
            }
        }
    }

    info->segNum = writeIdx + 1;
    OTA_LOG_D("%s merged segNum=%u, first addr=0x%08X, size=0x%08X", __FUNCTION__, info->segNum,
              info->segment[0].memaddr, info->segment[0].memsize);
}

/**
 * @brief MCU1_PDU刷写流程
 * @param priv ECU私有数据结构指针
 * @param driverFlashinfo 驱动Flash信息
 * @param softwareFlashinfo 软件Flash信息
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note PDU使用独立预编程顺序和0x27 09/0A安全访问
 */
static int FotaEcuExecMcu1Pdu(FotaEcuPriv_t *priv, Flashinfo_t *driverFlashinfo, Flashinfo_t *softwareFlashinfo)
{
    int ret = OTA_RET_OK;
    OTA_LOG_I("Execute MCU1_PDU[0x%04X] brush flow", priv->info.ecuId);

    SetEcuBrushProgress(2);  // 开始刷写

    // 步骤1-7: 预编程 + 进入编程会话 + 安全访问
    if (OTA_RET_OK != FotaEcuPreviousProgrammingMcu1Pdu(priv)) {
        OTA_LOG_E("%s FotaEcuPreviousProgrammingMcu1Pdu fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
        goto cleanup;
    }

    // PDU参考工具是整包一次擦除/34下载/F1A0校验, 合并64KB切分段
    MergeContiguousFlashSegments(driverFlashinfo);
    MergeContiguousFlashSegments(softwareFlashinfo);

    // 步骤8-12: 下载/完整性/依赖性/复位 (PDU不写指纹)
    if (OTA_RET_OK != FotaEcuProgrammingMcu1Body(priv, driverFlashinfo, softwareFlashinfo, false)) {
        OTA_LOG_E("%s FotaEcuProgrammingMcu1Body fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
        goto cleanup;
    }

    // 后编程: 重同步CAN网络
    if (OTA_RET_OK != FotaEcuPostProgrammingMcu1(priv)) {
        OTA_LOG_E("%s FotaEcuPostProgrammingMcu1 fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
        goto cleanup;
    }
    SetEcuBrushProgress(95);  // 后编程完成

    OTA_LOG_I("MCU1_PDU[0x%04X] brush flow completed successfully", priv->info.ecuId);
    return OTA_RET_OK;

cleanup:
    // 失败路径: 发送 0x10 01 让 ECU 退出编程会话
    (void)DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                      priv->info.channel, true, UDS_SESSION_DEFAULT);
    OTA_LOG_W("%s cleanup done: ECU returned to default session", __FUNCTION__);
    return ret;
}

/**
 * @brief MCU1通用刷写流程
 * @param priv ECU私有数据结构指针
 * @param driverFlashinfo 驱动Flash信息
 * @param softwareFlashinfo 软件Flash信息
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note 按KD26_TM/ACM.log: 27 01/02 Level1 + 27 09/0A Level3,
 *       不写指纹, 512字节块, 擦除带0x44, 复位后等待1000ms清DTC
 */
static int FotaEcuExecMcu1Common(FotaEcuPriv_t *priv, Flashinfo_t *driverFlashinfo, Flashinfo_t *softwareFlashinfo)
{
    int ret = OTA_RET_OK;
    OTA_LOG_I("Execute MCU1[0x%04X] brush flow", priv->info.ecuId);

    SetEcuBrushProgress(2);  // 开始刷写

    // 步骤1-8: 预编程 + Level1/Level3安全访问
    if (OTA_RET_OK != FotaEcuPreviousProgrammingMcu1Common(priv)) {
        OTA_LOG_E("%s FotaEcuPreviousProgrammingMcu1Common fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
        goto cleanup;
    }

    // MCU1参考工具是整包一次擦除/34下载/F1A0校验, 合并64KB切分段
    MergeContiguousFlashSegments(driverFlashinfo);
    MergeContiguousFlashSegments(softwareFlashinfo);

    // 下载/完整性/依赖性/复位 (MCU1系列不写指纹)
    if (OTA_RET_OK != FotaEcuProgrammingMcu1Body(priv, driverFlashinfo, softwareFlashinfo, false)) {
        OTA_LOG_E("%s FotaEcuProgrammingMcu1Body fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
        goto cleanup;
    }

    // 后编程: 等待1000ms后清除DTC
    if (OTA_RET_OK != FotaEcuPostProgrammingMcu1Common(priv)) {
        OTA_LOG_E("%s FotaEcuPostProgrammingMcu1Common fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
        goto cleanup;
    }
    SetEcuBrushProgress(95);  // 后编程完成

    OTA_LOG_I("MCU1[0x%04X] brush flow completed successfully", priv->info.ecuId);
    return OTA_RET_OK;

cleanup:
    // 失败路径: 发送 0x10 01 让 ECU 退出编程会话
    (void)DiagnosticSessionControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                      priv->info.channel, true, UDS_SESSION_DEFAULT);
    OTA_LOG_W("%s cleanup done: ECU returned to default session", __FUNCTION__);
    return ret;
}

/**
 * @brief MCU2刷写流程
 * @param priv ECU私有数据结构指针
 * @param driverFlashinfo 驱动Flash信息
 * @param softwareFlashinfo 软件Flash信息
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note 当前走标准流程，后续根据MCU2定制
 */
static int FotaEcuExecMcu2(FotaEcuPriv_t *priv, Flashinfo_t *driverFlashinfo, Flashinfo_t *softwareFlashinfo)
{
    return FotaEcuExecMcu1(priv, driverFlashinfo, softwareFlashinfo);
}

/**
 * @brief MCU3刷写流程
 * @param priv ECU私有数据结构指针
 * @param driverFlashinfo 驱动Flash信息
 * @param softwareFlashinfo 软件Flash信息
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 * @note 当前走标准流程，后续根据MCU3定制
 */
static int FotaEcuExecMcu3(FotaEcuPriv_t *priv, Flashinfo_t *driverFlashinfo, Flashinfo_t *softwareFlashinfo)
{
    OTA_LOG_I("Execute MCU3[0x%04X] brush flow", priv->info.ecuId);
    
    SetEcuBrushProgress(10);  // 开始刷写
    
    // TODO: 后续根据MCU3的刷写流程定制
    if (OTA_RET_OK != FotaEcuPreviousProgramming(priv)) {
        OTA_LOG_E("%s FotaEcuPreviousProgramming fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    SetEcuBrushProgress(20);  // 预编程完成
    
    // 执行编程 (内部会上报20%~80%的进度)
    if (OTA_RET_OK != FotaEcuProgramming(priv, driverFlashinfo, softwareFlashinfo)) {
        OTA_LOG_E("%s FotaEcuProgramming fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    // 进度由FotaEcuProgramming内部上报，此处不再重复设置
    
    if (OTA_RET_OK != FotaEcuPostProgramming(priv)) {
        OTA_LOG_E("%s FotaEcuPostProgramming fail.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    SetEcuBrushProgress(90);  // 后编程完成
    
    return OTA_RET_OK;
}

/**
 * @brief 根据ECU ID分发到对应的刷写流程
 * @param priv ECU私有数据结构指针
 * @param driverFlashinfo 驱动Flash信息
 * @param softwareFlashinfo 软件Flash信息
 * @return OTA_RET_OK 成功, OTA_RET_ERR 失败
 */
static int FotaEcuExecDispatch(FotaEcuPriv_t *priv, Flashinfo_t *driverFlashinfo, Flashinfo_t *softwareFlashinfo)
{
    OTA_LOG_D("[FotaEcuExecDispatch] ecuId: 0x%04X, phyId: 0x%08X, funcId: 0x%08X, resId: 0x%08X, channel: %d",
              priv->info.ecuId, priv->info.phyId, priv->info.funcId, priv->info.resId, priv->info.channel);

    switch (priv->info.ecuId) {
        case ECU_ID_VCU:
            OTA_LOG_D("[FotaEcuExecDispatch] Matched ECU_ID_VCU (0x7DC), calling FotaEcuExecVcu");
            return FotaEcuExecVcu(priv, driverFlashinfo, softwareFlashinfo);
        case ECU_ID_TMS:
        case ECU_ID_TMS_B:   // TMS_B (0x68C) 复用 TMS 刷写流程，P0-1 缺陷修复
            OTA_LOG_D("[FotaEcuExecDispatch] Matched ECU_ID_TMS/TMS_B (0x%04X), calling FotaEcuExecTms",
                      priv->info.ecuId);
            return FotaEcuExecTms(priv, driverFlashinfo, softwareFlashinfo);
        case ECU_ID_TCU:
            return FotaEcuExecTcu(priv, driverFlashinfo, softwareFlashinfo);
        case ECU_ID_MCU1_PDU:
            OTA_LOG_D("[FotaEcuExecDispatch] Matched MCU1_PDU (0x%04X), phyId=0x%08X, ch=%d, "
                      "calling FotaEcuExecMcu1Pdu",
                      priv->info.ecuId, priv->info.phyId, priv->info.channel);
            return FotaEcuExecMcu1Pdu(priv, driverFlashinfo, softwareFlashinfo);
        case ECU_ID_MCU1_ACM:
        case ECU_ID_MCU1_TM1:
        case ECU_ID_MCU1_TM2:
        case ECU_ID_MCU1_EPS:
        case ECU_ID_MCU1_DCDC:
            /* 注: ECU_ID_MCU1 兼容旧名,值等同ECU_ID_MCU1_TM1, 已由TM1分支覆盖
             * MCU1系列(TM1/TM2/EPS/ACM/DCDC)统一走Common流程:
             * 27 01/02 Level1 + 27 09/0A Level3, 不写指纹, 整包下载/CRC */
            OTA_LOG_D("[FotaEcuExecDispatch] Matched MCU1 (0x%04X), phyId=0x%08X, ch=%d, "
                      "calling FotaEcuExecMcu1Common",
                      priv->info.ecuId, priv->info.phyId, priv->info.channel);
            return FotaEcuExecMcu1Common(priv, driverFlashinfo, softwareFlashinfo);
        case ECU_ID_MCU2_TM1:
        case ECU_ID_MCU2_TM2:
        case ECU_ID_MCU2_PDU:
        case ECU_ID_MCU2_EPS:
        case ECU_ID_MCU2_ACM:
        case ECU_ID_MCU2_DCDC:
            /* 注: ECU_ID_MCU2 兼容旧名,值等同MCU2_TM1, 已由TM1分支覆盖 */
            OTA_LOG_D("[FotaEcuExecDispatch] Matched MCU2 (0x%04X), phyId=0x%08X, ch=%d, calling FotaEcuExecMcu2",
                      priv->info.ecuId, priv->info.phyId, priv->info.channel);
            return FotaEcuExecMcu2(priv, driverFlashinfo, softwareFlashinfo);
        case ECU_ID_MCU3:
            return FotaEcuExecMcu3(priv, driverFlashinfo, softwareFlashinfo);
        default:
            OTA_LOG_W("Unknown ECU ID[0x%04X], use standard brush flow", priv->info.ecuId);
            // 未知ECU ID走标准流程
            if (OTA_RET_OK != FotaEcuPreviousProgramming(priv)) {
                return OTA_RET_ERR;
            }
            if (OTA_RET_OK != FotaEcuProgramming(priv, driverFlashinfo, softwareFlashinfo)) {
                return OTA_RET_ERR;
            }
            if (OTA_RET_OK != FotaEcuPostProgramming(priv)) {
                return OTA_RET_ERR;
            }
            return OTA_RET_OK;
    }
}


static void CleanupKeepSessionThread(FotaEcuPriv_t *priv)
{
    if (priv) {
        priv->keepExit = 1;
        if (priv->keepSessionThread.joinable()) {
            priv->keepSessionThread.join();
        }
    }
}

int FotaEcuInit(FotaEcuHandle_t *hdl, const FotaEcuInfo_t *info)
{
    if (!hdl) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    FotaEcuPriv_t *priv = (FotaEcuPriv_t *)calloc(1, sizeof(FotaEcuPriv_t));
    if (!priv) {
        OTA_LOG_E("%s memory allocation failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    memcpy(&priv->info, info, sizeof(FotaEcuInfo_t));

    // 初始化suppressResponse: VCU(7DC)需要响应(3E 00), 其他ECU不需要响应(3E 80)
    priv->suppressResponse = (priv->info.ecuId != ECU_ID_VCU);

    if (OTA_RET_OK != UdsSrvInit(&priv->srvHdl, priv->info.funcId)) {
        OTA_LOG_E("%s UdsSrvInit failed.\n", __FUNCTION__);
        free(priv);
        return OTA_RET_ERR;
    }

    *hdl = (FotaEcuHandle_t)priv;
    return OTA_RET_OK;
}

int FotaEcuExec(FotaEcuHandle_t hdl, const std::string &targetVer)
{
    int ret = OTA_RET_OK;
    FotaEcuPriv_t *priv = (FotaEcuPriv_t *)hdl;
    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    
    std::unique_ptr<Flashinfo_t, void (*)(Flashinfo_t *)> driverFlashinfoGuard(nullptr, [](Flashinfo_t *p) {
        if (p) FlashInfoFree(p);
    });
    std::unique_ptr<Flashinfo_t, void (*)(Flashinfo_t *)> softwareFlashinfoGuard(nullptr, [](Flashinfo_t *p) {
        if (p) FlashInfoFree(p);
    });

    // 获取驱动程序Flash信息
    Flashinfo_t *driverFlashinfo = nullptr;
    if (priv->info.driverFilePath.length() > 0) {
        if (OTA_RET_OK != GetFlashInfoFromFile(&driverFlashinfo, priv->info.driverFilePath.c_str())) {
            OTA_LOG_E("%s GetFlashInfo fail for driver.\n", __FUNCTION__);
            FotaEcuSetErrorCode(priv, OTA_ERR_PROGRAM_FILE_CHECK_FAILED);
            return OTA_RET_ERR;
        }
        OTA_LOG_D("Get driver flash info from file: %s\n", priv->info.driverFilePath.c_str());
        driverFlashinfoGuard.reset(driverFlashinfo);
    }

    // 获取软件程序Flash信息
    Flashinfo_t *softwareFlashinfo = nullptr;
    OTA_LOG_D("Software file paths count: %zu\n", priv->info.softwareFilePaths.size());
    if (priv->info.softwareFilePaths.size() > 0) {
        if (OTA_RET_OK != GetFlashInfoFromFile(&softwareFlashinfo, priv->info.softwareFilePaths[0].c_str())) {
            OTA_LOG_E("%s GetFlashInfo fail for first software.\n", __FUNCTION__);
            FotaEcuSetErrorCode(priv, OTA_ERR_PROGRAM_FILE_CHECK_FAILED);
            return OTA_RET_ERR;
        }
        softwareFlashinfoGuard.reset(softwareFlashinfo);

        // 大于一个固件包
        for (size_t i = 1; i < priv->info.softwareFilePaths.size(); i++) {
            Flashinfo_t *tempFlashinfo = nullptr;
            if (OTA_RET_OK != GetFlashInfoFromFile(&tempFlashinfo, priv->info.softwareFilePaths[i].c_str())) {
                OTA_LOG_E("%s GetFlashInfo fail for software %zu.\n", __FUNCTION__, i);
                FotaEcuSetErrorCode(priv, OTA_ERR_PROGRAM_FILE_CHECK_FAILED);
                return OTA_RET_ERR;
            }

            if (OTA_RET_OK != MergeFlashInfo(softwareFlashinfo, tempFlashinfo)) {
                OTA_LOG_E("%s merge flash info failed.\n", __FUNCTION__);
                FlashInfoFree(tempFlashinfo);
                FotaEcuSetErrorCode(priv, OTA_ERR_PROGRAM_FILE_CHECK_FAILED);
                return OTA_RET_ERR;
            }

            FlashInfoFree(tempFlashinfo);
        }
    }

    // 创建会话保持线程
    // try {
    //     priv->keepSessionThread = std::thread(KeepSession, priv);
    // } catch (const std::exception &e) {
    //     OTA_LOG_E("%s create thread fail: %s\n", __FUNCTION__, e.what());
    //     return OTA_RET_ERR;
    // }

    // 根据ECU ID分发到对应的刷写流程
    ret = FotaEcuExecDispatch(priv, driverFlashinfo, softwareFlashinfo);
    // CleanupKeepSessionThread(priv);

    // 如果刷写过程失败 ，即直接返回失败
    if (ret == OTA_RET_ERR) {
        return ret;
    }
    OTA_LOG_E("%s FotaEcuExecDispatch success.\n", __FUNCTION__);

    sleep(10);
    // 清空消息队列中的残留响应数据
    UdsSrvClearQueue(priv->srvHdl);
    //ret = CheckSoftwareVersion(priv, targetVer);
    // EcuClearDtc(priv);
    return ret;
}

// 版本检查最大重试次数
#define VERSION_CHECK_MAX_RETRIES  3

/**
 * @brief 带重试的版本检查函数
 * @param priv ECU私有数据结构指针
 * @param targetVer 目标版本号
 * @return OTA_RET_OK 成功，OTA_RET_ERR 失败
 */
static int CheckSoftwareVersionWithRetry(FotaEcuPriv_t *priv, const std::string &targetVer)
{
    for (int retryCount = 0; retryCount <= VERSION_CHECK_MAX_RETRIES; ++retryCount) {
        if (CheckSoftwareVersion(priv, targetVer) == OTA_RET_OK) {
            OTA_LOG_I("Software version check success after %d attempt(s)", retryCount + 1);
            return OTA_RET_OK;
        }
        if (retryCount < VERSION_CHECK_MAX_RETRIES) {
            OTA_LOG_W("Software version check failed, retrying... (%d/%d)", retryCount + 1, VERSION_CHECK_MAX_RETRIES);
            sleep(10);
        }
    }
    OTA_LOG_E("Software version check failed after %d attempt(s)", VERSION_CHECK_MAX_RETRIES + 1);
    return OTA_RET_ERR;
}

int FotaEcuRollback(FotaEcuHandle_t hdl, uint16_t timeOutMs, const std::string &rollbackVer)
{
    uint8_t data[32];
    uint32_t dateLen = 0;
    FotaEcuPriv_t *priv = (FotaEcuPriv_t *)hdl;
    int ret = OTA_RET_OK;
    time_t startTime = time(nullptr);
    const int POLL_INTERVAL = 30;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 创建会话保持线程
    // try {
    //     priv->keepSessionThread = std::thread(KeepSession, priv);
    // } catch (const std::exception &e) {
    //     OTA_LOG_E("%s create thread fail: %s\n", __FUNCTION__, e.what());
    //     return OTA_RET_ERR;
    // }

    // 切换扩展模式
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.funcId, priv->info.resId,
                                                             priv->info.channel, true, UDS_SESSION_EXTENDED)) {
        OTA_LOG_E("%s 10 03 fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
        goto EXIT;
    }
    usleep(500000);
    // 安全校验Level1
    if (OTA_RET_OK != CheckAccessSecurity1(priv)) {
        OTA_LOG_E("%s CheckAccessSecurity1 fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
        goto EXIT;
    }
    usleep(500000);
    // 发送回退指令
    if (UDS_SERVICE_RESULT_OK != RoutineControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId, priv->info.channel,
                                                   UDS_ROUTINECTRL_STARTROUTINE, 0xDF01, data, &dateLen) ||
        (dateLen != 1) || (data[0] != 0x00)) {
        sleep(6);
        if (UDS_SERVICE_RESULT_OK != RoutineControlReq(priv->srvHdl, priv->info.phyId, priv->info.resId,
                                                       priv->info.channel, UDS_ROUTINECTRL_STARTROUTINE, 0xDF01, data,
                                                       &dateLen) ||
            (dateLen != 1) || (data[0] != 0x00)) {
            ret = OTA_RET_ERR;
            goto EXIT;
        }
    }
EXIT:
    // 切换默认会话
    if (UDS_SERVICE_RESULT_OK != DiagnosticSessionControlReq(priv->srvHdl, priv->info.funcId, priv->info.resId,
                                                             priv->info.channel, false, UDS_SESSION_DEFAULT_NOACK)) {
        OTA_LOG_E("%s 10 81 fail.\n", __FUNCTION__);
        ret = OTA_RET_ERR;
    }

    // CleanupKeepSessionThread(priv);

    sleep(10);

    // 查询软件版本号是否一致（失败后重试）
    int verCheck = CheckSoftwareVersionWithRetry(priv, rollbackVer);

    EcuClearDtc(priv);
    return (ret == OTA_RET_OK) ? verCheck : ret;
}

int FotaEcuGetErrorCode(FotaEcuHandle_t hdl, uint32_t &errorCode)
{
    FotaEcuPriv_t *priv = (FotaEcuPriv_t *)hdl;
    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    errorCode = priv->errorCode;
    return OTA_RET_OK;
}

int FotaEcuDeinit(FotaEcuHandle_t hdl)
{
    FotaEcuPriv_t *priv = (FotaEcuPriv_t *)hdl;
    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // CleanupKeepSessionThread(priv);
    UdsSrvDeinit(priv->srvHdl);
    free(priv);
    return OTA_RET_OK;
}