#include <stdint.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include "ota_master_api.h"
#include "ota_master_log.h"

#define LOG_ERROR(fmt, ...) printf("\033[1;31m[ERROR]\033[0m " fmt "\n", ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) printf("\033[1;33m[WARN]\033[0m  " fmt "\n", ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) printf("\033[1;32m[INFO]\033[0m  " fmt "\n", ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) printf("\033[1;37m[DEBUG]\033[0m  " fmt "\n", ##__VA_ARGS__)

#define CUSTOM_ASSERT_MSG(cond, msg) \
    do {                             \
        if (!(cond)) {               \
            LOG_ERROR("%s\n", msg);  \
            exit(0);                 \
        }                            \
    } while (0)

#define CUSTOM_ASSERT_EXP(cond, cas, e, g)                                              \
    do {                                                                                \
        if (!(cond)) {                                                                  \
            LOG_ERROR("[FAIL] Test %s failed: Expected error %d, got %d\n", cas, e, g); \
            exit(0);                                                                    \
        }                                                                               \
    } while (0)

namespace {
static constexpr const char *LOG_TAG = "otaMasterUnitTest";
static constexpr const char *OTA_MASTER_LOG_SAVE_FOLDER = "/sdcard/ota_master/";
static constexpr const char *OTA_MASTER_CONFIG_FOLDER = "/sdcard/ota_master/config/";
static constexpr const char *OTA_MASTER_DOWNLOAD_FOLDER = "/sdcard/ota_master/";
static constexpr const char *OTA_MASTER_UNZIP_FOLDER = "/sdcard/ota_master/";

// 下载链接中包含文件扩展名
static const StOtaTaskPolicyFileInfo upgradeFileInfo = {.fileSize = 0, .downloadUrl = "", .md5 = ""};
static const StOtaTaskPolicyFileInfo backupFileInfo = {.fileSize = 384846, .downloadUrl = "", .md5 = ""};
static const StOtaTaskPolicyFlashProtocol tboxFlashProto = {.canPort = 0,
                                                            .physicalId = 0x703,
                                                            .functionId = 0x7df,
                                                            .responseId = 0x783,
                                                            .secAccessLv1Mask = 0,
                                                            .secAccessLv2Mask = 0};
static const StOtaTaskPolicyVersionInfo tboxSupportVer = {.partNum = "", .hwVer = "", .oemSwVer = "", .swVer = ""};
static const StOtaTaskPolicyVersionInfo tboxCheckVer = {.partNum = "", .hwVer = "", .oemSwVer = "", .swVer = ""};

static StOtaTaskPolicyEcuInfo otaTaskPolicyEcuInfo[1] = {
    {.ecuId = 0x703,
     .upgradeOrder = 1,
     .supplierId = "DL03005",
     .upgradeFileInfo = upgradeFileInfo,
     .backupFileInfo = backupFileInfo,
     .flashProtocol = tboxFlashProto,
     .supportVer = tboxSupportVer,
     .checkVer = tboxCheckVer},
};

static StOtaTaskPolicyInfo otaTaskPolicyInfo = {.taskId = nullptr,
                                                .taskName = "ota_unit_test",
                                                .taskInfo = "ota_unit_test_info",
                                                .logUploadAddr = "",
                                                .vehicleOtaVer = "CV01.V1.0.1",
                                                .taskStartTime = 0,
                                                .taskDuration = 0,
                                                .upgradeDuration = 10,
                                                .isUploadLog = 0,
                                                .upgradeMode = 1,
                                                .taskCount = 0,
                                                .taskEcuInfo = nullptr};
} // namespace

static void TestOtaMasterLogInit()
{
    // 封装测试逻辑的函数
    auto RunTest = [](const char *name, const char *folder, OtaLogErrorCode expectedError) {
        LOG_DEBUG("===== Testing Case: OtaMasterLogInit(%s, %s) =====\n", name ? name : "nullptr",
                  folder ? folder : "nullptr");

        // 调用初始化函数
        OtaLogErrorCode logError = OtaMasterLogInit(name, folder);
        CUSTOM_ASSERT_EXP((logError == expectedError), "OtaMasterLogInit", expectedError, logError);

        LOG_INFO("[PASS] Test passed\n");
    };

    // 测试参数1为nullptr
    RunTest(nullptr, OTA_MASTER_LOG_SAVE_FOLDER, OTA_LOG_NULL_POINTER_PARAMETER);

    // 测试参数2为nullptr
    RunTest(LOG_TAG, nullptr, OTA_LOG_NULL_POINTER_PARAMETER);

    // 测试参数2为无效路径
    RunTest(LOG_TAG, "/oemapp/", OTA_LOG_FILE_CREATION_FAILED);

    // 测试正常情况
    RunTest(LOG_TAG, OTA_MASTER_LOG_SAVE_FOLDER, OTA_LOG_OK);

    // 测试重复初始化
    RunTest(LOG_TAG, OTA_MASTER_LOG_SAVE_FOLDER, OTA_LOG_ALREADY_INITIALIZED);
}

static void TestOtaMasterLogPrint()
{
    LOG_DEBUG("===== Testing Case: TestOtaMasterLogPrint() =====\n");
    OTA_LOG_D("OTA_LOG_D test");
    OTA_LOG_I("OTA_LOG_I test");
    OTA_LOG_W("OTA_LOG_W test");
    OTA_LOG_E("OTA_LOG_E test");
    LOG_INFO("[PASS] Test passed\n");
}

static void TestOtaMasterUploadLog()
{
    LOG_DEBUG("===== Testing Case: OtaMasterUploadLog(nullptr) =====\n");
    OtaMasterUploadLog(nullptr,nullptr);
    LOG_INFO("[PASS] Test passed\n");

    LOG_DEBUG("===== Testing Case: OtaMasterUploadLog(\"https://example.com\") =====\n");
    OtaMasterUploadLog("https://example.com,","bd26d188-3234-434b-1c0b-d9930f3fc0aa");
    LOG_INFO("[PASS] Test passed\n");
}

static void TestOtaMasterLogReset()
{
    LOG_DEBUG("===== Testing Case: OtaMasterLogReset() =====\n");
    // OTA日志未初始化时，OtaMasterLogReset()测试
    OtaMasterLogFinish();
    auto error = OtaMasterLogReset();
    CUSTOM_ASSERT_EXP((error == OTA_LOG_NOT_INITIALIZES), "OtaMasterLogReset", OTA_LOG_NOT_INITIALIZES, error);

    // OTA日志初始化后，OtaMasterLogReset()测试
    OtaLogErrorCode logError = OtaMasterLogInit("otaMasterApi", OTA_MASTER_LOG_SAVE_FOLDER);
    CUSTOM_ASSERT_EXP((logError == OTA_LOG_OK), "OtaMasterLogInit", OTA_LOG_OK, logError);

    error = OtaMasterLogReset();
    CUSTOM_ASSERT_EXP((logError == OTA_LOG_OK), "OtaMasterLogReset", OTA_LOG_OK, error);

    LOG_INFO("[PASS] Test passed\n");
}

static void TestOtaMasterLogFinish()
{
    LOG_DEBUG("===== Testing Case: OtaMasterLogFinish() =====\n");
    OtaMasterLogFinish();
    LOG_INFO("[PASS] Test passed\n");
}

static void TestOtaMasterInit()
{
    // 封装测试逻辑的函数
    auto RunTest = [](const char *configFilePath, const char *downloadFolder, const char *unzipFolder,
                      int expectedError) {
        LOG_DEBUG("===== Testing Case: OtaMasterInit(%s, %s, %s) =====\n",
                  configFilePath ? configFilePath : "nullptr", downloadFolder ? downloadFolder : "nullptr",
                  unzipFolder ? unzipFolder : "nullptr");

        // 调用初始化函数
        int ret = OtaMasterInit(configFilePath, downloadFolder, unzipFolder);
        CUSTOM_ASSERT_EXP((ret == expectedError), "OtaMasterInit", expectedError, ret);

        LOG_INFO("[PASS] Test passed\n");
    };

    // 测试参数1为nullptr
    RunTest(nullptr, OTA_MASTER_DOWNLOAD_FOLDER, OTA_MASTER_UNZIP_FOLDER, OTA_RET_ERR);

    // 测试参数2为nullptr
    RunTest(OTA_MASTER_CONFIG_FOLDER, nullptr, OTA_MASTER_UNZIP_FOLDER, OTA_RET_ERR);

    // 测试参数3为nullptr
    RunTest(OTA_MASTER_CONFIG_FOLDER, OTA_MASTER_DOWNLOAD_FOLDER, nullptr, OTA_RET_ERR);

    // 测试参数1为无效路径
    RunTest("/tmp/test/", OTA_MASTER_DOWNLOAD_FOLDER, OTA_MASTER_UNZIP_FOLDER, OTA_RET_ERR);

    // 测试正常情况
    RunTest(OTA_MASTER_CONFIG_FOLDER, OTA_MASTER_DOWNLOAD_FOLDER, OTA_MASTER_UNZIP_FOLDER, OTA_RET_OK);

    // 测试重复初始化
    RunTest(OTA_MASTER_CONFIG_FOLDER, OTA_MASTER_DOWNLOAD_FOLDER, OTA_MASTER_UNZIP_FOLDER, OTA_RET_ERR);
}

static void TestOtaMasterDeinit()
{
    LOG_DEBUG("===== Testing Case: OtaMasterDeinit() =====\n");
    OtaMasterDeinit();

    // 重复调用OtaMasterDeinit()测试
    OtaMasterDeinit();
    LOG_INFO("[PASS] Test passed\n");
}

static void TestOtaMasterRegisterReportCallback()
{
    LOG_DEBUG("===== Testing Case: OtaMasterRegisterReportCallback() =====\n");

    int ret = OtaMasterRegisterReportCallback(nullptr);
    CUSTOM_ASSERT_MSG((ret == OTA_RET_ERR), "OtaMasterRegisterReportCallback expected failed, paramter is nullptr");

    StOtaMasterReportCallback callback = {
        .startDownloadCb = [](const char *) { return OTA_RET_OK; },
        .downloadResultCb = [](const char *, int, OtaUpgradeErrorCode) { return OTA_RET_OK; },
        .upgradeResultCb = [](const char *, const char *, OtaUpgradeResult, OtaUpgradeErrorCode) { return OTA_RET_OK; },
        .otaTaskClearResultCb = [](const char *, int) { return OTA_RET_OK; },
        .preconditionsCheckResultCb = [](const char *, int, OtaPreconditionsErrorCode) { return OTA_RET_OK; },
        .startUpgradeCb = [](const char *) { return OTA_RET_OK; }};
    ret = OtaMasterRegisterReportCallback(&callback);
    CUSTOM_ASSERT_MSG((ret == OTA_RET_ERR),
                      "OtaMasterRegisterReportCallback expected failed, OtaMaster not initialized");

    ret = OtaMasterInit(OTA_MASTER_CONFIG_FOLDER, OTA_MASTER_DOWNLOAD_FOLDER, OTA_MASTER_UNZIP_FOLDER);
    CUSTOM_ASSERT_EXP(ret == OTA_RET_OK, "OtaMasterInit", OTA_RET_OK, ret);
    ret = OtaMasterRegisterReportCallback(&callback);
    CUSTOM_ASSERT_MSG((ret == OTA_RET_OK), "OtaMasterRegisterReportCallback failed");
    LOG_INFO("[PASS] Test passed\n");

    LOG_DEBUG("===== Testing Case: OtaMasterUnregisterReportCallback() =====\n");
    OtaMasterUnregisterReportCallback();
    LOG_INFO("[PASS] Test passed\n");

    OtaMasterDeinit();
}

static void TestOtaMasterRegisterQueryCallback()
{
    LOG_DEBUG("===== Testing Case: OtaMasterRegisterQueryCallback() =====\n");
    int ret = OtaMasterRegisterQueryCallback(nullptr);
    CUSTOM_ASSERT_MSG((ret == OTA_RET_ERR), "OtaMasterRegisterReportCallback expected failed, paramter is nullptr");

    StOtaMasterQueryCallback callback = {
        .queryTaskECUVerSyncCb = [](const StQueryEcuVerInfo *, uint8_t, StEcuVersionInfo *) { return OTA_RET_OK; },
        .queryTaskECUVerAsyncCb = [](const StQueryEcuVerInfo *, uint8_t) {},
        .queryUpgradePrecondsSyncCb = [](struct UpgradePreconditions_st *) { return OTA_RET_OK; },
        .queryUpgradePrecondsAsyncCb = []() {}};
    ret = OtaMasterRegisterQueryCallback(&callback);
    CUSTOM_ASSERT_MSG((ret == OTA_RET_ERR),
                      "OtaMasterRegisterQueryCallback expected failed, OtaMaster not initialized");

    ret = OtaMasterInit(OTA_MASTER_CONFIG_FOLDER, OTA_MASTER_DOWNLOAD_FOLDER, OTA_MASTER_UNZIP_FOLDER);
    CUSTOM_ASSERT_EXP(ret == OTA_RET_OK, "OtaMasterInit", OTA_RET_OK, ret);
    ret = OtaMasterRegisterQueryCallback(&callback);
    CUSTOM_ASSERT_MSG((ret == OTA_RET_OK), "OtaMasterRegisterQueryCallback failed");
    LOG_INFO("[PASS] Test passed\n");

    LOG_DEBUG("===== Testing Case: OtaMasterUnregisterQueryCallback() =====\n");
    OtaMasterUnregisterQueryCallback();
    LOG_INFO("[PASS] Test passed\n");

    OtaMasterDeinit();
}

static void TestOtaMasterRegisterRequestCallback()
{
    LOG_DEBUG("===== Testing Case: OtaMasterRegisterRequestCallback() =====\n");
    int ret = OtaMasterRegisterRequestCallback(nullptr);
    CUSTOM_ASSERT_MSG((ret == OTA_RET_ERR), "OtaMasterRegisterRequestCallback expected failed, paramter is nullptr");

    StOtaMasterRequestCallback callback = {
        .sendUdsDataCb = [](const uint8_t *, uint16_t) { return OTA_RET_OK; },
        .sendUdsDataAckCb = [](const uint8_t *, uint16_t) { return OTA_RET_OK; },
        .reqUpDownHighVoltCb = [](OtaMasterUpgradePowerMode) { return OTA_RET_OK; },
        .reqFotaModeOnOff = [](CanSignalOtaMode) { return OTA_RET_OK; },
        .reqTboxSelfUpgradeCb = [](const char *) { return OTA_RET_OK; },
        .reqTboxSelfRollbackCb = [](const char *) { return OTA_RET_OK; },
        .reqTboxRebootCb = []() {}};
    ret = OtaMasterRegisterRequestCallback(&callback);
    CUSTOM_ASSERT_MSG((ret == OTA_RET_ERR),
                      "OtaMasterRegisterRequestCallback expected failed, OtaMaster not initialized");

    ret = OtaMasterInit(OTA_MASTER_CONFIG_FOLDER, OTA_MASTER_DOWNLOAD_FOLDER, OTA_MASTER_UNZIP_FOLDER);
    CUSTOM_ASSERT_EXP(ret == OTA_RET_OK, "OtaMasterInit", OTA_RET_OK, ret);
    ret = OtaMasterRegisterRequestCallback(&callback);
    CUSTOM_ASSERT_MSG((ret == OTA_RET_OK), "OtaMasterRegisterRequestCallback failed");
    LOG_INFO("[PASS] Test passed\n");

    LOG_DEBUG("===== Testing Case: OtaMasterUnregisterRequestCallback() =====\n");
    OtaMasterUnregisterRequestCallback();
    LOG_INFO("[PASS] Test passed\n");

    OtaMasterDeinit();
}

static void TestOtaMasterStart()
{
    LOG_DEBUG("===== Testing Case: OtaMasterStart() =====\n");
    int ret = OtaMasterStart();
    CUSTOM_ASSERT_MSG((ret == OTA_RET_ERR), "OtaMasterStart failed, OtaMaster not initialized");

    ret = OtaMasterInit(OTA_MASTER_CONFIG_FOLDER, OTA_MASTER_DOWNLOAD_FOLDER, OTA_MASTER_UNZIP_FOLDER);
    CUSTOM_ASSERT_EXP(ret == OTA_RET_OK, "OtaMasterInit", OTA_RET_OK, ret);
    ret = OtaMasterStart();
    CUSTOM_ASSERT_MSG((ret == OTA_RET_OK), "OtaMasterStart failed");
    LOG_INFO("[PASS] Test passed\n");

    OtaMasterDeinit();
}

static void TestOtaMasterStop()
{
    LOG_DEBUG("===== Testing Case: OtaMasterStop() =====\n");
    int ret = OtaMasterStop();
    CUSTOM_ASSERT_MSG((ret == OTA_RET_ERR), "OtaMasterStop expected failed, OtaMaster not initialized");

    ret = OtaMasterInit(OTA_MASTER_CONFIG_FOLDER, OTA_MASTER_DOWNLOAD_FOLDER, OTA_MASTER_UNZIP_FOLDER);
    CUSTOM_ASSERT_EXP(ret == OTA_RET_OK, "OtaMasterInit", OTA_RET_OK, ret);
    ret = OtaMasterStop();
    CUSTOM_ASSERT_MSG((ret == OTA_RET_OK), "OtaMasterStop failed");
    LOG_INFO("[PASS] Test passed\n");

    OtaMasterDeinit();
}

static void TestOtaMasterStartOtaTask()
{
    LOG_DEBUG("===== Testing Case: OtaMasterStartOtaTask() =====\n");
    int ret = OtaMasterStartOtaTask(nullptr);
    CUSTOM_ASSERT_MSG((ret == OTA_RET_ERR), "OtaMasterStartOtaTask expected failed, OtaMaster not initialized");

    ret = OtaMasterInit(OTA_MASTER_CONFIG_FOLDER, OTA_MASTER_DOWNLOAD_FOLDER, OTA_MASTER_UNZIP_FOLDER);
    CUSTOM_ASSERT_EXP(ret == OTA_RET_OK, "OtaMasterInit", OTA_RET_OK, ret);
    ret = OtaMasterStart();
    CUSTOM_ASSERT_MSG((ret == OTA_RET_OK), "OtaMasterStart failed");
    sleep(2);

    ret = OtaMasterStartOtaTask(nullptr);
    CUSTOM_ASSERT_MSG((ret == OTA_RET_ERR), "OtaMasterStartOtaTask expected failed, parameter is nullptr");

    ret = OtaMasterStartOtaTask(&otaTaskPolicyInfo);
    CUSTOM_ASSERT_MSG((ret == OTA_RET_ERR), "OtaMasterStartOtaTask expected failed, taskID is nullptr");

    otaTaskPolicyInfo.taskId = "";
    ret = OtaMasterStartOtaTask(&otaTaskPolicyInfo);
    CUSTOM_ASSERT_MSG((ret == OTA_RET_ERR), "OtaMasterStartOtaTask expected failed, taskID is empty");

    otaTaskPolicyInfo.taskId = "bd26d188-3234-434b-1c0b-d9930f3fc0aa";
    ret = OtaMasterStartOtaTask(&otaTaskPolicyInfo);
    CUSTOM_ASSERT_MSG((ret == OTA_RET_ERR), "OtaMasterStartOtaTask expected failed, taskEcuInfo is empty");

    otaTaskPolicyInfo.taskEcuInfo = otaTaskPolicyEcuInfo;
    ret = OtaMasterStartOtaTask(&otaTaskPolicyInfo);
    CUSTOM_ASSERT_MSG((ret == OTA_RET_OK), "OtaMasterStartOtaTask failed");
    sleep(1);

    ret = OtaMasterStop();
    CUSTOM_ASSERT_MSG((ret == OTA_RET_OK), "OtaMasterStop failed");
    LOG_INFO("[PASS] Test passed\n");

    OtaMasterDeinit();
}

static void TestOtaMasterClearOtaTask()
{
    LOG_DEBUG("===== Testing Case: TestOtaMasterClearOtaTask() =====\n");
    int ret = OtaMasterClearOtaTask();
    CUSTOM_ASSERT_MSG((ret == OTA_RET_ERR), "OtaMasterClearOtaTask expected failed, OtaMaster not initialized");

    ret = OtaMasterInit(OTA_MASTER_CONFIG_FOLDER, OTA_MASTER_DOWNLOAD_FOLDER, OTA_MASTER_UNZIP_FOLDER);
    CUSTOM_ASSERT_EXP(ret == OTA_RET_OK, "OtaMasterInit", OTA_RET_OK, ret);
    ret = OtaMasterClearOtaTask();
    CUSTOM_ASSERT_MSG((ret == OTA_RET_ERR),
                      "OtaMasterClearOtaTask expected failed, OTA state is not waiting for user confirm");
    LOG_INFO("[PASS] Test passed\n");

    OtaMasterDeinit();
}

static void TestOtaMasterConfirmStartUpgrading()
{
    LOG_DEBUG("===== Testing Case: OtaMasterConfirmStartUpgrading() =====\n");
    int ret = OtaMasterConfirmStartUpgrading("1234");
    CUSTOM_ASSERT_MSG((ret == OTA_RET_ERR),
                      "OtaMasterConfirmStartUpgrading expected failed, OtaMaster not initialized");

    ret = OtaMasterInit(OTA_MASTER_CONFIG_FOLDER, OTA_MASTER_DOWNLOAD_FOLDER, OTA_MASTER_UNZIP_FOLDER);
    CUSTOM_ASSERT_EXP(ret == OTA_RET_OK, "OtaMasterInit", OTA_RET_OK, ret);
    ret = OtaMasterConfirmStartUpgrading(nullptr);
    CUSTOM_ASSERT_MSG((ret == OTA_RET_ERR),
                      "OtaMasterConfirmStartUpgrading expected failed, task id is nullptr");

    ret = OtaMasterConfirmStartUpgrading("1234");
    CUSTOM_ASSERT_MSG((ret == OTA_RET_ERR),
                      "OtaMasterConfirmStartUpgrading expected failed, OTA state is not waiting for user confirm");

    LOG_INFO("[PASS] Test passed\n");
    OtaMasterDeinit();
}

static void TestOtaMasterAES128ECBEncryptAndDecrypt()
{
    LOG_DEBUG("===== Testing Case: OtaMasterAES128ECBEncryptAndDecrypt() =====\n");
    // 测试正常情况
    const uint8_t key[] = "PFDL000101435544";
    uint8_t input[] = "VIN-HEV11AF19PFDL0001,SN-SRUNLX00000000001,ICCID-89860623400001435544";
    uint8_t *outputEncrypt = nullptr;
    uint8_t *outputDecrypt = nullptr;

    auto PrintHex = [](const uint8_t *data, size_t len) {
        if (data == nullptr || len == 0) {
            return;
        }
        for (size_t i = 0; i < len; ++i) {
            printf("%02x", data[i]);
        }
        printf("\n");
    };

    // // 测试参数为nullptr
    int encryptLen = OtaMasterAES128ECBEncrypt(nullptr, 0, nullptr, 0, nullptr);
    CUSTOM_ASSERT_EXP((encryptLen == 0), "OtaMasterAES128ECBEncrypt(nullptr, 0, nullptr, 0, nullptr)", 0, encryptLen);
    encryptLen = OtaMasterAES128ECBEncrypt(key, 0, nullptr, 0, nullptr);
    CUSTOM_ASSERT_EXP((encryptLen == 0), "OtaMasterAES128ECBEncrypt(key, 0, nullptr, 0, nullptr)", 0, encryptLen);
    encryptLen = OtaMasterAES128ECBEncrypt(key, strlen(reinterpret_cast<const char *>(key)), nullptr, 0, nullptr);
    CUSTOM_ASSERT_EXP((encryptLen == 0), "OtaMasterAES128ECBEncrypt(key, keyLen, nullptr, 0, nullptr)", 0, encryptLen);
    encryptLen = OtaMasterAES128ECBEncrypt(key, strlen(reinterpret_cast<const char *>(key)), input, 0, nullptr);
    CUSTOM_ASSERT_EXP((encryptLen == 0), "OtaMasterAES128ECBEncrypt(key, keyLen, input, 0, nullptr)", 0, encryptLen);
    encryptLen = OtaMasterAES128ECBEncrypt(key, strlen(reinterpret_cast<const char *>(key)), input,
                                           strlen(reinterpret_cast<const char *>(input)), nullptr);
    CUSTOM_ASSERT_EXP((encryptLen == 0), "OtaMasterAES128ECBEncrypt(key, keyLen, input, inputLen, nullptr)", 0,
                      encryptLen);
    encryptLen = OtaMasterAES128ECBEncrypt(key, strlen(reinterpret_cast<const char *>(key)), input,
                                           strlen(reinterpret_cast<const char *>(input)), &outputEncrypt);
    CUSTOM_ASSERT_MSG((encryptLen > 0),
                      "OtaMasterAES128ECBEncrypt(key, keyLen, input, inputLen, *output) expected return > 0");
    PrintHex(outputEncrypt, encryptLen);
    
    int decryptLen = OtaMasterAES128ECBDecrypt(nullptr, 0, nullptr, 0, nullptr);
    CUSTOM_ASSERT_EXP((decryptLen == 0), "OtaMasterAES128ECBDecrypt(nullptr, 0, nullptr, 0, nullptr)", 0, decryptLen);
    decryptLen = OtaMasterAES128ECBDecrypt(key, 0, nullptr, 0, nullptr);
    CUSTOM_ASSERT_EXP((decryptLen == 0), "OtaMasterAES128ECBDecrypt(key, 0, nullptr, 0, nullptr)", 0, decryptLen);
    decryptLen = OtaMasterAES128ECBDecrypt(key, strlen(reinterpret_cast<const char *>(key)), nullptr, 0, nullptr);
    CUSTOM_ASSERT_EXP((decryptLen == 0), "OtaMasterAES128ECBDecrypt(key, keyLen, nullptr, 0, nullptr)", 0, decryptLen);
    decryptLen = OtaMasterAES128ECBDecrypt(key, strlen(reinterpret_cast<const char *>(key)), outputEncrypt, 0, nullptr);
    CUSTOM_ASSERT_EXP((decryptLen == 0), "OtaMasterAES128ECBDecrypt(key, keyLen, input, 0, nullptr)", 0, decryptLen);
    decryptLen = OtaMasterAES128ECBDecrypt(key, strlen(reinterpret_cast<const char *>(key)), outputEncrypt, encryptLen, nullptr);
    CUSTOM_ASSERT_EXP((decryptLen == 0), "OtaMasterAES128ECBDecrypt(key, keyLen, input, inputLen, nullptr)", 0, decryptLen);
    decryptLen = OtaMasterAES128ECBDecrypt(key, strlen(reinterpret_cast<const char *>(key)), outputEncrypt, encryptLen, &outputDecrypt);
    CUSTOM_ASSERT_MSG((decryptLen > 0),
                      "OtaMasterAES128ECBDecrypt(key, keyLen, input, inputLen, *output) expected return > 0");
    PrintHex(outputDecrypt, decryptLen);
    std::string decrypStr(reinterpret_cast<const char *>(outputDecrypt), decryptLen);
    LOG_DEBUG("Decrypt: %s\n", decrypStr.c_str());

    free(outputEncrypt);
    free(outputDecrypt);
    LOG_INFO("[PASS] Test passed\n");
}

static void TestOtaMasterCheckMD5()
{
    LOG_DEBUG("===== Testing Case: OtaMasterCheckMD5() =====\n");
    // 测试参数为nullptr
    int ret = OtaMasterCheckMD5(nullptr, nullptr);
    CUSTOM_ASSERT_EXP((ret == OTA_RET_ERR), "OtaMasterCheckMD5", OTA_RET_ERR, ret);
    ret = OtaMasterCheckMD5(nullptr, "1234567890");
    CUSTOM_ASSERT_EXP((ret == OTA_RET_ERR), "OtaMasterCheckMD5", OTA_RET_ERR, ret);
    ret = OtaMasterCheckMD5("/oemdata/test", nullptr);
    CUSTOM_ASSERT_EXP((ret == OTA_RET_ERR), "OtaMasterCheckMD5", OTA_RET_ERR, ret);
    ret = OtaMasterCheckMD5("/oemdata/test", "");
    CUSTOM_ASSERT_EXP((ret == OTA_RET_ERR), "OtaMasterCheckMD5", OTA_RET_ERR, ret);

    // 测试文件不存在
    ret = OtaMasterCheckMD5("/oemdata/test", "1234567890");
    CUSTOM_ASSERT_EXP((ret == OTA_RET_ERR), "OtaMasterCheckMD5", OTA_RET_ERR, ret);

    // 测试文件存在，但MD5不匹配
    std::string filePath = "/oemdata/test.txt";
    std::string targetMD5 = "1234567890";
    std::ofstream file(filePath);
    file << "Hello, World!";
    file.close();
    ret = OtaMasterCheckMD5(filePath.c_str(), targetMD5.c_str());
    CUSTOM_ASSERT_EXP((ret == OTA_RET_ERR), "OtaMasterCheckMD5", OTA_RET_ERR, ret);

    // 测试文件存在，且MD5匹配
    targetMD5 = "65a8e27d8879283831b664bd8b7f0ad4";
    ret = OtaMasterCheckMD5(filePath.c_str(), targetMD5.c_str());
    CUSTOM_ASSERT_EXP((ret == OTA_RET_OK), "OtaMasterCheckMD5", OTA_RET_OK, ret);

    // 删除测试文件
    remove(filePath.c_str());
    LOG_INFO("[PASS] Test passed\n");
}

static void TestOtaMasterNoticeMcuUdsDataAck() {}

static void TestOtaMasterNoticeEcuUdsData() {}

static void TestOtaMasterNoticeEcuVersionAsyncQueryResult()
{
    LOG_DEBUG("===== Testing Case: OtaMasterNoticeEcuVersionAsyncQueryResult() =====\n");
    // 测试参数为nullptr
    OtaMasterNoticeEcuVersionAsyncQueryResult(nullptr, 0);

    // 测试正常情况
    StEcuVersionInfo ecuVerInfo[1];
    ecuVerInfo[0].ecuId = 0;
    ecuVerInfo[0].swVersion = "";
    ecuVerInfo[0].hwVersion = "";
    ecuVerInfo[0].supplierId = "";
    ecuVerInfo[0].partNum = "";
    OtaMasterNoticeEcuVersionAsyncQueryResult(ecuVerInfo, sizeof(ecuVerInfo) / sizeof(StEcuVersionInfo));

    LOG_INFO("[PASS] Test passed\n");
}

static void TestOtaMasterNoticeUpgradePreconditionsAsyncQueryResult()
{
    LOG_DEBUG("===== Testing Case: OtaMasterNoticeUpgradePreconditionsAsyncQueryResult() =====\n");

    // 测试参数为nullptr
    OtaMasterNoticeUpgradePreconditionsAsyncQueryResult(nullptr);
    // 测试正常情况
    StUpgradePreconditions preconditions = {0};
    OtaMasterNoticeUpgradePreconditionsAsyncQueryResult(&preconditions);
    LOG_INFO("[PASS] Test passed\n");
}

static void TestOtaMasterNoticeTboxRollbackResult()
{
    LOG_DEBUG("===== Testing Case: OtaMasterNoticeTboxRollbackResult() =====\n");
    OtaMasterNoticeTboxRollbackResult(OTA_RET_OK, OTA_TBOX_NO_REBOOT);
    LOG_INFO("[PASS] Test passed\n");
}

static void TestOtaMasterNoticeTboxSelfUpgradeResult()
{
    LOG_DEBUG("===== Testing Case: OtaMasterNoticeTboxSelfUpgradeResult() =====\n");
    OtaMasterNoticeTboxSelfUpgradeResult(OTA_RET_ERR, OTA_TBOX_NO_REBOOT);
    LOG_INFO("[PASS] Test passed\n");
}

static void TestOtaMasterNoticeReportDownloadResultHasACK()
{
    LOG_DEBUG("===== Testing Case: OtaMasterNoticeReportDownloadResultHasACK() =====\n");
    // 测试参数为nullptr
    OtaMasterNoticeReportDownloadResultHasACK(nullptr);

    // 测试正常情况
    OtaMasterNoticeReportDownloadResultHasACK("bd26d188-3234-434b-1c0b-d9930f3fc0aa");
    LOG_INFO("[PASS] Test passed\n");
}

static void TestOtaMasterNoticeReportUpgradeResultHasACK()
{
    LOG_DEBUG("===== Testing Case: OtaMasterNoticeReportUpgradeResultHasACK() =====\n");
    // 测试参数为nullptr
    OtaMasterNoticeReportUpgradeResultHasACK(nullptr);

    // 测试正常情况
    OtaMasterNoticeReportUpgradeResultHasACK("bd26d188-3234-434b-1c0b-d9930f3fc0aa");
    LOG_INFO("[PASS] Test passed\n");
}

static void TestOtaMasterNoticeUpDownHighVoltageResult()
{
    LOG_DEBUG("===== Testing Case: OtaMasterNoticeUpDownHighVoltageResult() =====\n");
    OtaMasterNoticeUpDownHighVoltageResult(OTA_RET_ERR, OTA_HIGH_VOLTAGE_UPGRADE);
    LOG_INFO("[PASS] Test passed\n");
}

static void TestOtaMasterNoticeFotaModeOnOffSwResult()
{
    LOG_DEBUG("===== Testing Case: OtaMasterNoticeFotaModeOnOffSwResult() =====\n");
    OtaMasterNoticeFotaModeOnOffSwResult(OTA_RET_ERR, CAN_SIG_OTA_MODE_OFF);
    LOG_INFO("[PASS] Test passed\n");
}

int main()
{
    // ota_master_log单元测试
    TestOtaMasterLogInit();
    TestOtaMasterLogPrint();
    TestOtaMasterUploadLog();
    TestOtaMasterLogReset();
    TestOtaMasterLogFinish();

    // 初始化日志模块，用于输出测试结果
    auto error = OtaMasterLogInit(LOG_TAG, OTA_MASTER_LOG_SAVE_FOLDER);
    assert(error == OTA_LOG_OK && "OtaMasterLogInit failed");
    // ota_master_api单元测试
    TestOtaMasterInit();
    TestOtaMasterDeinit();
    TestOtaMasterRegisterReportCallback();
    TestOtaMasterRegisterQueryCallback();
    TestOtaMasterRegisterRequestCallback();
    TestOtaMasterStart();
    TestOtaMasterStop();
    TestOtaMasterStartOtaTask();
    TestOtaMasterClearOtaTask();
    TestOtaMasterConfirmStartUpgrading();
    TestOtaMasterAES128ECBEncryptAndDecrypt();
    TestOtaMasterCheckMD5();
    TestOtaMasterNoticeMcuUdsDataAck();
    TestOtaMasterNoticeEcuUdsData();
    TestOtaMasterNoticeEcuVersionAsyncQueryResult();
    TestOtaMasterNoticeUpgradePreconditionsAsyncQueryResult();
    TestOtaMasterNoticeTboxRollbackResult();
    TestOtaMasterNoticeTboxSelfUpgradeResult();
    TestOtaMasterNoticeReportDownloadResultHasACK();
    TestOtaMasterNoticeReportUpgradeResultHasACK();
    TestOtaMasterNoticeUpDownHighVoltageResult();
    TestOtaMasterNoticeFotaModeOnOffSwResult();
    // 释放日志模块
    OtaMasterLogFinish();

    LOG_INFO("[ALL PASSED]\n");
    return 0;
}
