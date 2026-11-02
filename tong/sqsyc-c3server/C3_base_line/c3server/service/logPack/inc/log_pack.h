#ifndef __LOG_PACK_H__
#define __LOG_PACK_H__

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <glib.h>
#include "log_pack_crypto_interface.h"

#define LOG_SERVER_TAG "logServer"

#define LOG_PACK_TASK_MAX_NUM 10

/* maxStoreSize 默认值：10MB；配置为 0 时表示不限制，按默认值处理 */
/* maxStoreSize 最小值：10MB；防止配置过小导致“创建即删除” */
#define LOG_PACK_MAX_STORE_SIZE_DEFAULT  (10ULL * 1024 * 1024)
#define LOG_PACK_MAX_STORE_SIZE_MIN      (10ULL * 1024 * 1024)

#define LOG_FILENAME_MAX_SIZE   128
#define LOG_FILEPATH_MAX_SIZE   128
#define LOG_FULLPATH_MAX_SIZE   512   /* 足够容纳 filepath + 最长文件名(255) + 分隔符 */

#define TBOX_VIN_SIZE 17

#define LOG_PACK_CFG_PATH_PRIMARY   "/oemdata/config/log_pack.ini"
#define LOG_PACK_CFG_PATH  "/oemapp/config/log_pack.ini"

/* 解析到 .log 为止，兼容 .log.tgz 和 .log.crypto.tgz 两种扩展名 */
#define LOG_FILENAME_PARSE_FORMAT_TGZ "%31[^-]-%" SCNu64 "-%17[^-]-%12[^.].log"
#define LOG_FILENAME_FORMAT_TXT       "%s-%" PRIu64 "-%s-%s.log.txt"
#define LOG_FILENAME_FORMAT_TGZ       "%s-%" PRIu64 "-%s-%s.log.tgz"
#define LOG_FILENAME_FORMAT_CRYPTO_TGZ "%s-%" PRIu64 "-%s-%s.log.crypto.tgz"

#define LOG_CRYPTO_DEFAULT_PASSWORD   "SRTL1234"


typedef struct {
	uint64_t maxIndex;      /* 当前最大序号 */
	uint64_t oldestIndex;   /* 当前最老序号，用于清理 */
	uint64_t totalSize;     /* 打包目录总大小，单位字节 */
	uint32_t fileCount;     /* 文件总数 */
} LogPackIdx_t;

typedef struct {
	char      name[32];
	uint64_t  index;
	uint64_t  maxStoreSize;		/* 打包目录总大小上限，单位字节 */
	uint8_t   cryptoFlag;       /* 加密开关：0 表示仅压缩，非 0 表示使用 LogSrCtx_t 中的全局加密接口 */
	char      cryptoPassword[32];
	char      logPath[LOG_FILEPATH_MAX_SIZE];
	char      logPackPath[LOG_FILEPATH_MAX_SIZE];
	char      logActiveFileName[LOG_FILENAME_MAX_SIZE];
	char      logRotatedFileName[LOG_FILENAME_MAX_SIZE];
	LogPackIdx_t idx;
} LogSrPackTask_t;

typedef struct {
	gint state;					/** PM 状态，使用 gint 配合 g_atomic_int 线程安全访问 **/
	GTimer* bootUpTimer;		/** 记录启动时间戳 **/
	GTimer* wakeUpTimer;		/** 记录唤醒时间戳 **/
	GMutex lock;				/** 保护 bootUpTimer / wakeUpTimer 的线程安全 **/
	gint needReconcile;			/** 唤醒后需要重新 reconcile 索引，使用 gint 配合 g_atomic_int **/
	gboolean initialized;		/** 标记 pmWorkState 是否已完成初始化，防止重复 deinit **/
} PmWorkState_t;

typedef struct {
	char      cryptoName[32];            /* [base] 中配置的加密方式名称：des3/aes128/none/空 */
	Crypto_interface_t* cryptoInterface; /* [base] 中 crypto 配置对应的加密接口 */
	gint logThreadQuitFlag;
	GThread *logThread;
	PmWorkState_t pmWorkState;
	uint32_t taskCnt;
	LogSrPackTask_t* tasks;
	uint32_t checkInterval;     /* 检查间隔，单位秒，默认 10 */
	uint32_t bootUpDelay;       /* 开机后禁止打包的时长，单位秒，默认 60 */
	uint32_t wakeUpDelay;       /* 唤醒后禁止打包的时长，单位秒，默认 30 */
} LogSrCtx_t;

int32_t LogStoreTaskInit(LogSrCtx_t* ctx);

void LogStoreTaskDeinit(LogSrCtx_t* ctx);

#endif
