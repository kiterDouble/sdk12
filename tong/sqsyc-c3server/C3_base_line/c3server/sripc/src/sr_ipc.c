/**
 * @file    sr_ipc.c
 * @brief   进程间通信库实现（工业级重构版）
 *
 * 架构要点：
 *
 *   Pub/Sub：
 *     - 共享内存（/sysQueShareShm）维护 type→[keyId 列表] 的订阅树
 *     - 每个订阅进程拥有独立的 System V 消息队列（key = 进程接收线程 tid）
 *     - 接收线程循环 msgrcv，通过进程内 GTree 分发回调
 *     - 发布时持锁遍历，遇死亡订阅者即时清理，防止鬼队列积累
 *
 *   RPC：
 *     - 全局唯一请求队列（key = RPC_REQ_QUEUE_KEY），按 RPCServerType_e 过滤
 *     - 每个 Server type 持有独立 flock 存活锁，离线时客户端快速失败
 *     - 每个客户端线程拥有独立的回复队列（key = tid），彻底消除竞争
 *     - 服务端可选线程池（1~4 线程）异步处理请求
 *     - userdata 封入 RpcTaskData_t，多 Server 实例下不会错配
 *     - 回复队列生命周期与客户端线程绑定，线程退出后自动失效
 *     - clienttid 使用 int32_t，支持 tid > 65535 的系统配置
 *
 * 修复问题清单（相对于原版）：
 *   #1  clienttid uint16_t 截断 → int32_t
 *   #2  userdata 双路绑定混用 → 封入 RpcTaskData_t
 *   #3  g_threadPool 多 Server 覆盖 → 移入 RpcServerThreadArgs_t
 *   #4  多处 early return → 单出口 result 模式
 *   #5  ipcIsFirstCall 语义反转 → IpcIsInitialized()（正向）
 *   #6  DestorySysQueTree 拼写 → DestroySysQueTree
 *   #7  SendMsgToAllQueue 双重 GetTypeNodeIndex 冗余调用 → 合并到锁内
 *   #8  if-else if 链补全末尾 else
 */

/*
 * 兼容性补丁：必须在包含任何第三方头文件之前。
 *
 * 问题根因：
 *   GLib gmacros.h 在 #if 表达式中展开 fallthrough 宏时，
 *   若该宏被定义为 [[__fallthrough__]]，则 "[" 在预处理表达式中非法，
 *   报：token "[" is not valid in preprocessor expressions。
 *
 * 解决：预定义 __has_attribute(x) 为 0，使 GLib 走降级路径。
 */
#ifndef __has_attribute
#  define __has_attribute(x)  0
#endif

/* --------------------------------------------------------------------------
 * 系统头文件
 * -------------------------------------------------------------------------- */
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

/* --------------------------------------------------------------------------
 * 第三方库头文件
 * -------------------------------------------------------------------------- */
#include <glib.h>
#include <glib/gprintf.h>
#include <gio/gio.h>
#include <gmodule.h>

/* --------------------------------------------------------------------------
 * 项目头文件
 * -------------------------------------------------------------------------- */
#include "err_code.h"
#include "queue_msg.h"
#include "sr_log.h"

/* --------------------------------------------------------------------------
 * 本模块接口头文件（最后包含）
 * -------------------------------------------------------------------------- */
#include "sr_ipc.h"

/* ==========================================================================
 * 编译期断言
 * ========================================================================== */

/*
 * 确保 int32_t 足以容纳 Linux tid 范围（tid_max 默认 4194304，< INT32_MAX）。
 * 若未来迁移到 tid 宽度不同的平台，此断言会在编译期报错。
 */
typedef char StaticAssert_TidFitsInt32[(sizeof(pid_t) <= sizeof(int32_t)) ? 1 : -1];

/* ==========================================================================
 * 宏定义
 * ========================================================================== */

/** 共享内存 POSIX 路径 */
#define TREE_SHM_PATH                   "/sysQueShareShm"

/** RPC 全局请求队列 System V key */
#define RPC_REQ_QUEUE_KEY               (3)

/** RPC Server 存活锁文件；每个 type 使用独立的 flock */
#define RPC_SERVER_LOCK_PATH_FMT        "/tmp/sr_ipc_rpc_%08x.lock"

/** Server 启动等待客户端离线清理完成的重试参数 */
#define RPC_SERVER_LOCK_RETRY_COUNT      (100)
#define RPC_SERVER_LOCK_RETRY_US         (10U * 1000U)

/** RPC 线程池最大工作线程数 */
#define RPC_THREAD_POOL_SIZE_MAX        (4U)

/** RPC 线程池最小工作线程数 */
#define RPC_THREAD_POOL_SIZE_MIN        (1U)

/**
 * RPC 回复队列消息 type 基址。
 * 每线程回复 type = RPC_RESP_TYPE_BASE + clientTid，在同一队列内完全隔离。
 * 取值需大于所有 RPCServerType_e 枚举值，避免与请求消息 type 碰撞。
 */
#define RPC_RESP_TYPE_BASE              ((long)(RPCSERVER_TYPE_MIN_LIMIT) + 0x00010000L)

/** 单个消息 type 最多允许的订阅者数量 */
#define TYPENODE_KEYID_COUNT            (32)

/** 共享内存中最多支持的消息 type 数量 */
#define TYPENODE_COUNT                  (128)

/** IPC 队列注册表最大节点数（纳管所有动态创建的队列：Pub/Sub + RPC 回复） */
#define IPC_QUEUE_REG_COUNT             (256)

/** 注册表中保存的进程名最大长度（含结尾 '\0'），与 /proc/<pid>/comm 对齐 */
#define IPC_PROC_NAME_LEN               (16)

/** RPC server 后台回收孤儿队列的最小间隔（秒） */
#define IPC_REAP_INTERVAL_SEC           (30)

/** 超时轮询基础间隔（ms） */
#define RECV_TIMEOUT_SLEEP_INTERVAL_MS  (2)

/**
 * System V 消息队列消息体大小宏（不包括 type 字段）
 * msgsnd/msgrcv 的 msgsz 参数应该是消息体大小，不包括 long type 字段
 */
#define SYSQUE_MSG_BODY_SIZE  (SYSQUE_MSG_DATA_MAX_LEN + sizeof(SysQueueData_t) - sizeof(long))
#define RPCQUE_MSG_BODY_SIZE  (SYSQUE_MSG_DATA_MAX_LEN + sizeof(RpcQueueData_t) - sizeof(long))

/** RPC 固定消息头的消息体长度（不含 mtype） */
#define RPCQUE_HEADER_BODY_SIZE (sizeof(RpcQueueData_t) - sizeof(long))

/* ==========================================================================
 * 内联工具函数
 * ========================================================================== */

/**
 * @brief 向上对齐到 alignment 的整数倍（alignment 必须为 2 的幂）
 *
 * @param[in] alignment  对齐粒度（2 的幂）
 * @param[in] size       原始大小
 * @return    对齐后的大小
 */
static inline size_t AlignUp(size_t alignment, size_t size)
{
    return (size + alignment - 1U) & ~(alignment - 1U);
}

/* ==========================================================================
 * 内部数据结构
 * ========================================================================== */

/**
 * @brief 共享内存中单个消息 type 的订阅节点
 */
typedef struct
{
    SYSQueMsgType_e type;                        /**< 消息类型 */
    int32_t         len;                         /**< 已注册的订阅者数量 */
    int32_t         keyID[TYPENODE_KEYID_COUNT];  /**< 订阅者消息队列 key（= tid） */
} ShmTypeNode_t;

/**
 * @brief IPC 队列注册表节点（共享内存，定长数组充当链表）
 *
 * 记录每条动态创建的 System V 队列与其属主线程的绑定关系。
 * 回收时遍历全表，对每个 inUse 节点用 tgkill(pid, tid, 0) 校验属主线程存活性：
 *   - 线程已退出 / 进程已退出 / tid 被其它进程复用 → 删除该节点并清除其消息队列。
 * 注意 msgKey 即 tid，属主是线程级（而非进程级），因此 detached/短命子线程退出后
 * 其槽位可被正确回收，避免注册表被逐渐耗尽。
 */
typedef struct
{
    int32_t msgKey;                      /**< System V 队列 key（= 创建时的 tid） */
    int32_t pid;                         /**< 属主进程 pid */
    char    procName[IPC_PROC_NAME_LEN]; /**< 属主进程名（来自 /proc/<pid>/comm，诊断用） */
    int32_t inUse;                       /**< 1=占用，0=空闲 */
} IpcQueueRegNode_t;

/**
 * @brief 共享内存根结构
 *
 * mutex_initialized 在跨进程文件锁保护下，于 mutex 初始化完成后发布。
 */
typedef struct
{
    atomic_int      mutex_initialized; /**< 0=未初始化，1=就绪 */
    pthread_mutex_t mutex;             /**< 跨进程鲁棒互斥锁（PROCESS_SHARED+ROBUST） */
    int32_t         typeNodeLen;       /**< 已使用的 typeNode 数量 */
    ShmTypeNode_t   data[TYPENODE_COUNT]; /**< 订阅树数组 */
    /*
     * IPC 队列注册表（追加在结构末尾，保证旧字段偏移不变）。
     * ftruncate 扩容时新区由内核清零，inUse 自然为 0（空槽），
     * 即使旧版 shm 文件残留也能平滑兼容。
     */
    IpcQueueRegNode_t queueReg[IPC_QUEUE_REG_COUNT]; /**< 队列注册表，inUse 标记占用 */
} SYSQueueMsgShm_t;

/**
 * @brief 进程内回调树节点（存储于 g_CallbackTree）
 *
 * key   = SYSQueMsgType_e（GINT_TO_POINTER 转换）
 * value = IPCCallback_t *（g_try_malloc0 分配，无在途回调后释放）
 */
typedef struct
{
    QueSubCallback callbackFuc; /**< 用户回调函数，轮询模式下可为 NULL */
    void          *userdata;    /**< 用户透传数据 */
    uint32_t       activeCalls; /**< 已取得本节点、尚未执行完成的回调数 */
} IPCCallback_t;

/**
 * @brief RPC 线程池任务单元
 *
 * callbackFuc 和 userdata 均由任务自身携带，彻底消除多 Server 实例下的
 * userdata 错配问题（原版通过 GThreadPool 的 userdata 参数传递，
 * 多 Server 时会被最后一次 g_thread_pool_new 的值覆盖）。
 */
typedef struct
{
    RpcServerCallback callbackFuc; /**< 服务端用户回调 */
    void             *userdata;    /**< 用户透传数据（封入任务，非全局） */
    RpcQueueData_t   *recvData;    /**< 已拷贝的请求消息（堆内存，由任务负责释放） */
} RpcTaskData_t;

/**
 * @brief RPC 服务线程启动参数（堆内存，由 RpcServerThread 尾部释放）
 */
typedef struct
{
    RPCServerType_e   type;        /**< 服务端类型枚举值 */
    RpcServerCallback callbackFuc; /**< 用户回调（不为 NULL，启动时已校验） */
    uint8_t           poolSize;    /**< 已裁剪到 [1, 4] 的工作线程数 */
    void             *userdata;    /**< 用户透传数据 */
    int               serverLockFd; /**< 服务存活锁 fd，由服务线程持有 */
} RpcServerThreadArgs_t;

/* ==========================================================================
 * 模块静态全局变量
 *
 * 所有模块级变量均以 static 修饰，外部不可直接访问。
 * 访问接口：IpcIsInitialized()
 * ========================================================================== */

/** IPC 初始化标志，FALSE=尚未初始化，TRUE=已初始化 */
static gboolean         g_ipcInitialized = FALSE;

/** 保护 g_ipcInitialized 的进程内互斥锁（仅保护初始化一次语义） */
static pthread_mutex_t  g_ipcInitMutex   = PTHREAD_MUTEX_INITIALIZER;

/** 保护进程内 g_CallbackTree 的查找、插入、删除和节点生命周期 */
static pthread_mutex_t  g_callbackTreeMutex = PTHREAD_MUTEX_INITIALIZER;

/** 退订/替换等待旧节点的在途回调全部结束 */
static pthread_cond_t   g_callbackTreeCond = PTHREAD_COND_INITIALIZER;

/** 共享内存映射指针，NULL=未映射 */
static SYSQueueMsgShm_t *g_SYSQueTreeShm = NULL;

/**
 * 当前进程 Pub/Sub 接收线程的消息队列 key（= 接收线程 tid）。
 * 由 InitSysQueMsg() 在接收线程/主轮询初始化时赋值，此后只读。
 * 语义：进程级单值，整个进程共享同一条 Pub/Sub 接收队列。
 */
static int32_t           g_KeyId         = 0;

/**
 * 进程内回调查找树（key = SYSQueMsgType_e，value = IPCCallback_t*）。
 * 仅在接收线程和 Subscribe/UnSubscribe 接口中访问，并由
 * g_callbackTreeMutex 保护节点查找和生命周期。
 */
static GTree            *g_CallbackTree  = NULL;

/** Pub/Sub 消息接收线程句柄，NULL=未启动 */
static GThread          *g_threadRecv    = NULL;

/* ==========================================================================
 * 内部函数：共享内存互斥锁
 * ========================================================================== */

/**
 * @brief 对共享内存互斥锁执行鲁棒加锁
 *
 * 若持锁进程异常崩溃（EOWNERDEAD），在已持锁状态下恢复锁的一致性，
 * 保证跨进程锁不会因进程崩溃而永久死锁。
 *
 * @param[in] mutex  共享内存中的互斥锁指针（不可为 NULL）
 * @return RET_OK   加锁成功
 * @return RET_ERR  加锁失败（errno 已通过 LOG_Error 记录）
 */
static int32_t ShmSafeMutexLock(pthread_mutex_t *mutex)
{
    int32_t result  = RET_ERR;
    int     lockRet = pthread_mutex_lock(mutex);

    if (lockRet == 0)
    {
        result = RET_OK;
    }
    else if (lockRet == EOWNERDEAD)
    {
        /* EOWNERDEAD 返回时当前线程已经持有锁，恢复一致性后可直接使用。 */
        int consistentRet = pthread_mutex_consistent(mutex);
        if (consistentRet != 0)
        {
            LOG_Error("ShmSafeMutexLock: mutex_consistent failed, ret=%d",
                      consistentRet);
            (void)pthread_mutex_unlock(mutex);
        }
        else
        {
            result = RET_OK;
        }
    }
    else
    {
        LOG_Error("ShmSafeMutexLock: lock failed, ret=%d", lockRet);
    }

    return result;
}

/* ==========================================================================
 * 内部函数：GTree 比较函数
 * ========================================================================== */

/**
 * @brief 进程内回调树的键比较函数
 *
 * 键类型为 SYSQueMsgType_e，通过 GINT_TO_POINTER 转换存储。
 */
static gint CallbackTreeCompareFunc(gconstpointer a,
                                    gconstpointer b,
                                    gpointer      usrdata)
{
    SYSQueMsgType_e ap     = (SYSQueMsgType_e)GPOINTER_TO_INT(a);
    SYSQueMsgType_e bp     = (SYSQueMsgType_e)GPOINTER_TO_INT(b);
    gint            result = 0;

    (void)usrdata;

    if (ap > bp)
    {
        result = 1;
    }
    else if (ap < bp)
    {
        result = -1;
    }
    else
    {
        result = 0;
    }

    return result;
}

/* ==========================================================================
 * 内部函数：共享内存初始化与操作
 * ========================================================================== */

/**
 * @brief 映射共享内存并初始化跨进程鲁棒互斥锁
 *
 * 使用 shm 文件锁保证多进程并发调用时 mutex 只被初始化一次。
 * 重复调用（g_SYSQueTreeShm != NULL）直接返回 RET_OK。
 *
 * @return RET_OK   映射成功且 mutex 可用
 * @return RET_ERR  shm_open / ftruncate / mmap / mutex 初始化任一失败
 */
static int32_t InitSysQueTreeShm(void)
{
    int32_t result     = RET_OK;
    int     fd         = -1;
    bool    initLocked = false;

    if (g_SYSQueTreeShm != NULL)
    {
        LOG_Debug("InitSysQueTreeShm: already mapped, skip");
    }
    else
    {
        fd = shm_open(TREE_SHM_PATH, O_CREAT | O_RDWR, 0644);
        if (fd == -1)
        {
            LOG_Error("InitSysQueTreeShm: shm_open failed [%s]", g_strerror(errno));
            result = RET_ERR;
        }

        /*
         * 用 shm fd 的文件锁串行化首次初始化。进程异常退出时内核会自动
         * 释放文件锁，其他进程不会观察到“标志已置位但 mutex 未就绪”。
         */
        if (result == RET_OK)
        {
            if (flock(fd, LOCK_EX) != 0)
            {
                LOG_Error("InitSysQueTreeShm: flock failed [%s]", g_strerror(errno));
                result = RET_ERR;
            }
            else
            {
                initLocked = true;
            }
        }

        if (result == RET_OK)
        {
            if (ftruncate(fd, (off_t)sizeof(SYSQueueMsgShm_t)) != 0)
            {
                LOG_Error("InitSysQueTreeShm: ftruncate failed [%s]", g_strerror(errno));
                result = RET_ERR;
            }
        }

        if (result == RET_OK)
        {
            void *mapped = mmap(NULL,
                                sizeof(SYSQueueMsgShm_t),
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED,
                                fd,
                                0);
            if (mapped == MAP_FAILED)
            {
                LOG_Error("InitSysQueTreeShm: mmap failed [%s]", g_strerror(errno));
                result = RET_ERR;
            }
            else
            {
                g_SYSQueTreeShm = (SYSQueueMsgShm_t *)mapped;
            }
        }

        /*
         * 初始化跨进程鲁棒互斥锁。
         * 只在 pthread_mutex_init 成功之后发布 mutex_initialized=1。
         */
        if (result == RET_OK)
        {
            if (atomic_load_explicit(&g_SYSQueTreeShm->mutex_initialized,
                                     memory_order_acquire) == 0)
            {
                pthread_mutexattr_t attr;
                bool                attrOk          = true;
                bool                attrInitialized = false;

                if (pthread_mutexattr_init(&attr) != 0)
                {
                    LOG_Error("InitSysQueTreeShm: mutexattr_init failed");
                    attrOk = false;
                }
                else
                {
                    attrInitialized = true;
                }

                if (attrOk &&
                    (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0))
                {
                    LOG_Error("InitSysQueTreeShm: setpshared failed");
                    attrOk = false;
                }

                if (attrOk &&
                    (pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST) != 0))
                {
                    LOG_Error("InitSysQueTreeShm: setrobust failed");
                    attrOk = false;
                }

                if (attrOk &&
                    (pthread_mutex_init(&g_SYSQueTreeShm->mutex, &attr) != 0))
                {
                    LOG_Error("InitSysQueTreeShm: mutex_init failed");
                    attrOk = false;
                }

                if (attrInitialized)
                {
                    (void)pthread_mutexattr_destroy(&attr);
                }

                if (!attrOk)
                {
                    result = RET_ERR;
                }
                else
                {
                    atomic_store_explicit(&g_SYSQueTreeShm->mutex_initialized,
                                          1,
                                          memory_order_release);
                }
            }
        }

        if (initLocked)
        {
            (void)flock(fd, LOCK_UN);
            initLocked = false;
        }

        /* fd 无论成功与否都应及时关闭；mmap 后 fd 不再需要 */
        if (fd != -1)
        {
            close(fd);
            fd = -1;
        }

        /* 初始化失败时撤销本进程映射，允许后续调用重新初始化。 */
        if ((result != RET_OK) && (g_SYSQueTreeShm != NULL))
        {
            (void)munmap(g_SYSQueTreeShm, sizeof(SYSQueueMsgShm_t));
            g_SYSQueTreeShm = NULL;
        }

        if (result == RET_OK)
        {
            LOG_Debug("InitSysQueTreeShm: success");
        }
    }

    return result;
}

/**
 * @brief 在共享内存中按 type 查找 typeNode 下标（调用方持锁）
 *
 * @param[in] type  消息类型
 * @return >=0   找到，返回数组下标
 * @return RET_ERR  未找到
 */
static int32_t GetTypeNodeIndex(SYSQueMsgType_e type)
{
    int32_t index  = 0;
    int32_t result = RET_ERR;

    for (index = 0; index < g_SYSQueTreeShm->typeNodeLen; index++)
    {
        if (g_SYSQueTreeShm->data[index].type == type)
        {
            result = index;
            break;
        }
    }

    return result;
}

/**
 * @brief 在 typeNode 的 keyID 数组中查找 keyId 下标（调用方持锁）
 *
 * @param[in] node   typeNode 指针（不可为 NULL）
 * @param[in] keyId  目标 keyId
 * @return >=0   找到，返回数组下标
 * @return RET_ERR  未找到
 */
static int32_t GetKeyIdIndex(const ShmTypeNode_t *node, int32_t keyId)
{
    int32_t index  = 0;
    int32_t result = RET_ERR;

    for (index = 0; index < node->len; index++)
    {
        if (node->keyID[index] == keyId)
        {
            result = index;
            break;
        }
    }

    return result;
}

/**
 * @brief 向共享内存订阅树注册 type-keyId 对（调用方持锁）
 *
 * 若 type 节点不存在则新建；keyId 已存在则记录 Warn 并跳过，不重复插入。
 *
 * @param[in] type   消息类型
 * @param[in] keyId  订阅者消息队列 key（= 接收线程 tid）
 */
static void UpdateSysQueTreeShm(SYSQueMsgType_e type, int32_t keyId)
{
    int32_t nodeIndex = GetTypeNodeIndex(type);

    if (nodeIndex != RET_ERR)
    {
        ShmTypeNode_t *node   = &g_SYSQueTreeShm->data[nodeIndex];
        int32_t        keyIdx = GetKeyIdIndex(node, keyId);

        if (keyIdx == RET_ERR)
        {
            if (node->len < TYPENODE_KEYID_COUNT)
            {
                node->keyID[node->len] = keyId;
                node->len++;
                LOG_Debug("UpdateSysQueTreeShm: add type[0x%08x] pid[%d] keyId[%d] len=%d",
                          type, getpid(), keyId, node->len);
            }
            else
            {
                LOG_Error("UpdateSysQueTreeShm: keyID array full for type[0x%08x]", type);
            }
        }
        else
        {
            LOG_Warn("UpdateSysQueTreeShm: type[0x%08x] pid[%d] keyId[%d] already exists",
                     type, getpid(), keyId);
        }
    }
    else
    {
        if (g_SYSQueTreeShm->typeNodeLen < TYPENODE_COUNT)
        {
            ShmTypeNode_t *node  = &g_SYSQueTreeShm->data[g_SYSQueTreeShm->typeNodeLen];
            node->type     = type;
            node->len      = 1;
            node->keyID[0] = keyId;
            g_SYSQueTreeShm->typeNodeLen++;
            LOG_Debug("UpdateSysQueTreeShm: new type[0x%08x] keyId[%d]", type, keyId);
        }
        else
        {
            LOG_Error("UpdateSysQueTreeShm: typeNode array full");
        }
    }
}

/**
 * @brief 从共享内存订阅树删除 type-keyId 对（调用方持锁）
 *
 * 使用 swap-remove 策略：用最后一个元素填补被删除位置，O(1) 时间复杂度。
 * typeNode 为空时，同样用 swap-remove 从 data 数组中移除。
 *
 * @param[in] type   消息类型
 * @param[in] keyId  订阅者消息队列 key
 * @return true   该 type 下已无订阅者，typeNode 已被删除
 * @return false  该 type 下仍有其他订阅者，或 type/keyId 不存在
 */
static bool DeleteSysQueTreeShm(SYSQueMsgType_e type, int32_t keyId)
{
    bool    typeNodeEmpty = false;
    int32_t nodeIndex     = GetTypeNodeIndex(type);

    if (nodeIndex == RET_ERR)
    {
        LOG_Warn("DeleteSysQueTreeShm: type[0x%08x] not found", type);
    }
    else
    {
        ShmTypeNode_t *node   = &g_SYSQueTreeShm->data[nodeIndex];
        int32_t        keyIdx = GetKeyIdIndex(node, keyId);

        if (keyIdx != RET_ERR)
        {
            /* swap-remove：用末尾元素填补被删除位置 */
            node->keyID[keyIdx]        = node->keyID[node->len - 1];
            node->keyID[node->len - 1] = 0;
            node->len--;
            LOG_Debug("DeleteSysQueTreeShm: removed type[0x%08x] keyId[%d] len=%d",
                      type, keyId, node->len);
        }
        else
        {
            LOG_Warn("DeleteSysQueTreeShm: keyId[%d] not found in type[0x%08x]",
                     keyId, type);
        }

        if (node->len <= 0)
        {
            /* typeNode 为空：用末尾 typeNode 填补当前位置 */
            int32_t lastIdx = g_SYSQueTreeShm->typeNodeLen - 1;

            if (lastIdx >= 0)
            {
                memmove(&g_SYSQueTreeShm->data[nodeIndex],
                        &g_SYSQueTreeShm->data[lastIdx],
                        sizeof(ShmTypeNode_t));
                memset(&g_SYSQueTreeShm->data[lastIdx], 0, sizeof(ShmTypeNode_t));
                g_SYSQueTreeShm->typeNodeLen--;
            }
            else
            {
                /* typeNodeLen 为 0 的防御分支，正常不会到达 */
                LOG_Warn("DeleteSysQueTreeShm: typeNodeLen already 0");
            }

            typeNodeEmpty = true;
            LOG_Info("DeleteSysQueTreeShm: type[0x%08x] removed from shm", type);
        }
    }

    return typeNodeEmpty;
}

/**
 * @brief 检查某 type 下各订阅者线程的存活状态，清理已死线程（调用方持锁）
 *
 * 通过 SYS_tkill(tid, 0) 探测线程存活性；已死线程的 System V 队列一并删除，
 * 防止鬼队列长期占用内核资源。
 *
 * @param[in] type  消息类型
 */
static void CheckAndCleanDeadSubscribers(SYSQueMsgType_e type)
{
    int32_t nodeIndex = GetTypeNodeIndex(type);

    if (nodeIndex == RET_ERR)
    {
        /* type 不存在，无需清理 */
    }
    else
    {
        ShmTypeNode_t *node  = &g_SYSQueTreeShm->data[nodeIndex];
        int32_t        count = 0;
        bool           stop  = false;

        while ((count < node->len) && (!stop))
        {
            int32_t keyId = node->keyID[count];

            if (keyId <= 0)
            {
                count++;
            }
            else
            {
                long probeRet;

                errno = 0;
                probeRet = syscall(SYS_tkill, (int)keyId, 0);
                if ((probeRet == -1) && (errno == ESRCH))
                {
                    LOG_Warn("CheckAndCleanDeadSubscribers: tid[%d] dead, cleaning", keyId);
                    msgctl(msgget(keyId, 0777), IPC_RMID, NULL);

                    if (DeleteSysQueTreeShm(type, keyId))
                    {
                        /* typeNode 已整体删除，停止遍历 */
                        stop = true;
                    }
                    /* 不递增 count：swap-remove 后当前位置是新元素 */
                }
                else
                {
                    if (probeRet == -1)
                    {
                        LOG_Warn("CheckAndCleanDeadSubscribers: tid[%d] probe failed [%s], keep subscriber",
                                 keyId, g_strerror(errno));
                    }
                    count++;
                }
            }
        }
    }
}

/* ==========================================================================
 * 内部函数：IPC 队列注册表（孤儿队列回收）
 * ========================================================================== */

/**
 * @brief 读取指定 pid/tid 的进程名到 buf（来自 /proc/<pid>/comm）
 *
 * @param[in]  pid  进程/线程 id
 * @param[out] buf  输出缓冲区
 * @param[in]  len  缓冲区容量
 * @return RET_OK   读取成功（已去除结尾换行）
 * @return RET_ERR  /proc/<pid>/comm 不存在或读取失败（进程已退出）
 */
static int32_t GetProcName(int32_t pid, char *buf, size_t len)
{
    int32_t result = RET_ERR;

    if ((buf != NULL) && (len > 0U))
    {
        char path[64];
        int  fd;

        (void)g_snprintf(path, sizeof(path), "/proc/%d/comm", pid);

        fd = open(path, O_RDONLY);
        if (fd >= 0)
        {
            ssize_t rd = read(fd, buf, len - 1U);

            if (rd > 0)
            {
                char *nl;

                buf[(size_t)rd] = '\0';
                nl = strchr(buf, '\n'); /* comm 末尾带换行，去掉 */
                if (nl != NULL)
                {
                    *nl = '\0';
                }
                result = RET_OK;
            }

            (void)close(fd);
        }
        /* fd < 0：文件不存在 = 进程已退出，result 保持 RET_ERR */
    }

    return result;
}

/**
 * @brief 判断注册表节点的属主进程是否仍存活且身份一致
 *
 * 双重校验，解决 pid 复用误判：
 *   1. /proc/<pid>/comm 可读 —— pid 仍存在
 *   2. 进程名与注册时一致   —— 没被复用为别的进程
 *
 * @param[in] node  注册表节点
 * @return true   属主存活且身份一致
 * @return false  属主已退出，或 pid 被复用（节点应清理）
 */
static bool IpcRegOwnerAlive(const IpcQueueRegNode_t *node)
{
    bool alive = false;

    /* msgKey 即属主线程 tid（注册时由 gettid 写入）。
     * 用 tgkill(pid, tid, 0) 精确校验“该进程下该线程”是否存活：
     *   - 线程已退出（含子线程退出但进程仍在）→ ESRCH，判死，可回收
     *   - 进程已退出 → ESRCH，判死
     *   - tid 被其它进程复用（pid+tid 不匹配）→ ESRCH，判死
     *   - EPERM（跨用户无权发信号）→ 进程/线程存在，判活
     * 修复原实现只用 pid 判活，导致子线程退出后其注册槽位永远无法回收、
     * 注册表(256槽)被 detached/短命线程逐渐耗尽（registry full）的问题。 */
    if (syscall(SYS_tgkill, node->pid, node->msgKey, 0) == 0)
    {
        alive = true;
    }
    else if (errno == EPERM)
    {
        alive = true;
    }

    return alive;
}

/**
 * @brief 从订阅树中清除某 keyId 在所有 type 下的注册（调用方持锁）
 *
 * 死队列被回收后，其在订阅树里的残留记录一并摘除，避免后续广播
 * 反复向已删除的队列投递失败。
 *
 * @param[in] keyId  要清除的队列 key
 */
static void PurgeKeyFromTree(int32_t keyId)
{
    int32_t i = 0;

    while (i < g_SYSQueTreeShm->typeNodeLen)
    {
        SYSQueMsgType_e type      = g_SYSQueTreeShm->data[i].type;
        int32_t         lenBefore = g_SYSQueTreeShm->typeNodeLen;

        if (GetKeyIdIndex(&g_SYSQueTreeShm->data[i], keyId) != RET_ERR)
        {
            (void)DeleteSysQueTreeShm(type, keyId); /* 内部可能 swap-remove 整个 typeNode */
        }

        /* typeNode 数量未变才前进，否则当前槽位已是新元素，需重查 */
        if (g_SYSQueTreeShm->typeNodeLen == lenBefore)
        {
            i++;
        }
    }
}

/**
 * @brief 遍历注册表，回收属主已死/被复用的孤儿队列（调用方持锁）
 *
 * 对每个 inUse 节点：属主校验失败则删除其 System V 队列、从订阅树摘除、
 * 释放注册表槽位（inUse=0）。
 *
 * @return 本次回收的孤儿队列数量
 */
static int32_t CleanDeadIpcQueues(void)
{
    int32_t reaped = 0;
    int32_t i      = 0;

    for (i = 0; i < IPC_QUEUE_REG_COUNT; i++)
    {
        IpcQueueRegNode_t *node = &g_SYSQueTreeShm->queueReg[i];

        if ((node->inUse != 0) && (!IpcRegOwnerAlive(node)))
        {
            int qid;

            LOG_Warn("CleanDeadIpcQueues: reap key[%d] pid[%d] name[%s]",
                     node->msgKey, node->pid, node->procName);

            /* 删除 System V 队列（可能已不存在，忽略返回值） */
            qid = msgget(node->msgKey, 0644);
            if (qid != -1)
            {
                (void)msgctl(qid, IPC_RMID, NULL);
            }

            PurgeKeyFromTree(node->msgKey);   /* 摘除订阅树残留 */
            (void)memset(node, 0, sizeof(*node)); /* 释放槽位 */
            reaped++;
        }
    }

    return reaped;
}

/**
 * @brief 快速路径探测：本 key 已登记且 pid 未变则命中（调用方持锁）
 *
 * @param[in] msgKey  队列 key
 * @param[in] pid     当前进程 pid
 * @return true  已存在同 key 同 pid 的有效节点
 * @return false 未命中（需走清理+登记流程）
 */
static bool IpcRegHasLiveEntry(int32_t msgKey, int32_t pid)
{
    bool    hit = false;
    int32_t i   = 0;

    for (i = 0; i < IPC_QUEUE_REG_COUNT; i++)
    {
        const IpcQueueRegNode_t *node = &g_SYSQueTreeShm->queueReg[i];

        if ((node->inUse != 0) && (node->msgKey == msgKey) && (node->pid == pid))
        {
            hit = true;
            break;
        }
    }

    return hit;
}

/**
 * @brief 定位可写槽位：优先同 key 槽（覆盖），否则首个空槽（调用方持锁）
 *
 * @param[in] msgKey  队列 key
 * @return >=0     可写槽位下标
 * @return RET_ERR 注册表已满
 */
static int32_t IpcRegFindSlot(int32_t msgKey)
{
    int32_t slot = RET_ERR;
    int32_t i    = 0;

    for (i = 0; i < IPC_QUEUE_REG_COUNT; i++)
    {
        const IpcQueueRegNode_t *node = &g_SYSQueTreeShm->queueReg[i];

        if ((node->inUse != 0) && (node->msgKey == msgKey))
        {
            slot = i; /* 同 key（pid 变了/tid 复用），覆盖此槽 */
            break;
        }
        if ((node->inUse == 0) && (slot == RET_ERR))
        {
            slot = i; /* 记录首个空槽 */
        }
    }

    return slot;
}

/**
 * @brief 写入注册表节点（调用方持锁）
 *
 * @param[in] slot    目标槽位下标
 * @param[in] msgKey  队列 key
 * @param[in] pid     属主进程 pid
 */
static void IpcRegWriteNode(int32_t slot, int32_t msgKey, int32_t pid)
{
    IpcQueueRegNode_t *node = &g_SYSQueTreeShm->queueReg[slot];
    char               procName[IPC_PROC_NAME_LEN] = {0};

    /* 仅首次登记/覆盖槽位时读取，避免 RPC 快速路径反复访问 /proc。 */
    (void)GetProcName(pid, procName, sizeof(procName));

    node->msgKey = msgKey;
    node->pid    = pid;
    (void)g_strlcpy(node->procName, procName, sizeof(node->procName));
    node->inUse  = 1;
}

/**
 * @brief 持锁状态下完成登记：快速路径命中或回收+定位+写入（调用方持锁）
 *
 * @param[in] msgKey  队列 key
 * @param[in] pid     当前进程 pid
 * @return RET_OK / RET_ERR
 */
static int32_t IpcRegCommitLocked(int32_t msgKey, int32_t pid)
{
    int32_t result = RET_ERR;
    int32_t i      = 0;

    if (IpcRegHasLiveEntry(msgKey, pid))
    {
        result = RET_OK; /* 快速路径：已登记且 pid 未变 */
    }
    else
    {
        int32_t slot;

        /* 首次登记/pid 变更：先回收死节点，再定位槽位写入 */
        (void)CleanDeadIpcQueues();
        slot = IpcRegFindSlot(msgKey);

        if (slot == RET_ERR)
        {
            /* 诊断：registry full 时打印当前所有占用槽位，定位是哪个进程在泄漏线程。
             * 占用槽位里的 key=tid、pid=进程 pid、name=进程名，可据此判断泄漏源。
             * 注意：此日志为临时诊断手段，定位后可移除或降级。 */
            int32_t usedSlots = 0;
            LOG_Error("IpcRegCommitLocked: registry full, key[%d] dropped", msgKey);
            LOG_Error("IpcRegCommitLocked: dump %d slots begin", (int)IPC_QUEUE_REG_COUNT);
            for (i = 0; i < IPC_QUEUE_REG_COUNT; i++)
            {
                const IpcQueueRegNode_t *n = &g_SYSQueTreeShm->queueReg[i];
                if (n->inUse != 0)
                {
                    usedSlots++;
                    LOG_Error("  slot[%d] key[tid=%d] pid[%d] name[%s] %s",
                              i, n->msgKey, n->pid, n->procName,
                              IpcRegOwnerAlive(n) ? "ALIVE" : "DEAD");
                }
            }
            LOG_Error("IpcRegCommitLocked: dump end, used=%d/%d",
                      usedSlots, (int)IPC_QUEUE_REG_COUNT);
        }
        else
        {
            IpcRegWriteNode(slot, msgKey, pid);

            /* 诊断：登记成功时打印 tid/pid/槽位/剩余空槽，追踪线程登记过程。
             * 可据此看哪个进程何时登记新 tid、表占用率是否持续上涨。
             * 注意：此日志为临时诊断手段，定位后可移除或降级。 */
            int32_t usedAfter = 0;
            for (i = 0; i < IPC_QUEUE_REG_COUNT; i++)
            {
                if (g_SYSQueTreeShm->queueReg[i].inUse != 0)
                {
                    usedAfter++;
                }
            }
            LOG_Warn("IpcRegCommitLocked: commit key[tid=%d] pid[%d] slot[%d], used=%d/%d",
                     msgKey, pid, slot, usedAfter, (int)IPC_QUEUE_REG_COUNT);

            result = RET_OK;
        }
    }

    return result;
}

/**
 * @brief 将本进程新建的队列登记进注册表（幂等）
 *
 * 热路径优化：若本 key 已登记且 pid 未变，直接返回，不扫全表。
 * 仅在"首次登记/pid 变更"时顺带回收一次全表死节点（满足"创建时
 * check 所有节点"语义），使 EnsureRpcReplyQueue 的高频调用代价可控。
 *
 * @param[in] msgKey  本进程新建的队列 key（= tid）
 * @return RET_OK / RET_ERR
 */
static int32_t RegisterIpcQueue(int32_t msgKey)
{
    int32_t result = RET_ERR;
    bool    mapped = true;

    /* RPC-only 客户端进程可能未映射 shm，此处兜底（幂等） */
    if (g_SYSQueTreeShm == NULL)
    {
        if (InitSysQueTreeShm() != RET_OK)
        {
            LOG_Error("RegisterIpcQueue: shm map failed");
            mapped = false;
        }
    }

    if (mapped)
    {
        if (ShmSafeMutexLock(&g_SYSQueTreeShm->mutex) != RET_OK)
        {
            LOG_Error("RegisterIpcQueue: lock failed");
        }
        else
        {
            result = IpcRegCommitLocked(msgKey, (int32_t)getpid());
            (void)pthread_mutex_unlock(&g_SYSQueTreeShm->mutex);
        }
    }

    return result;
}

/**
 * @brief 回滚当前进程指定 key 的注册记录
 *
 * 仅清除 key 与 pid 均匹配的节点，不删除消息队列，也不触碰其他进程
 * 可能复用同一 key 后建立的新记录。
 *
 * @param[in] msgKey  要回滚的队列 key
 */
static void RollbackIpcQueueRegistration(int32_t msgKey)
{
    int32_t i;
    int32_t pid = (int32_t)getpid();

    if ((g_SYSQueTreeShm != NULL) &&
        (ShmSafeMutexLock(&g_SYSQueTreeShm->mutex) == RET_OK))
    {
        for (i = 0; i < IPC_QUEUE_REG_COUNT; i++)
        {
            IpcQueueRegNode_t *node = &g_SYSQueTreeShm->queueReg[i];

            if ((node->inUse != 0) &&
                (node->msgKey == msgKey) &&
                (node->pid == pid))
            {
                (void)memset(node, 0, sizeof(*node));
                break;
            }
        }

        (void)pthread_mutex_unlock(&g_SYSQueTreeShm->mutex);
    }
}

/**
 * @brief 后台低频回收入口：加锁遍历全表清理孤儿队列
 *
 * 供 RPC server 常驻线程定期调用，覆盖"长期无新进程启动"的静默期。
 */
static void IpcReapDeadQueues(void)
{
    if (g_SYSQueTreeShm != NULL)
    {
        if (ShmSafeMutexLock(&g_SYSQueTreeShm->mutex) == RET_OK)
        {
            int32_t reaped = CleanDeadIpcQueues();

            (void)pthread_mutex_unlock(&g_SYSQueTreeShm->mutex);

            if (reaped > 0)
            {
                LOG_Info("IpcReapDeadQueues: reaped %d orphan queue(s)", reaped);
            }
        }
    }
}

/* ==========================================================================
 * 内部函数：RPC Server 存活检测
 * ========================================================================== */

/**
 * @brief 打开指定 RPC Server type 的存活锁文件
 *
 * 锁文件不删除，仅依赖 flock 的内核生命周期；Server 进程崩溃或
 * exec 时锁会自动释放，避免 System V 队列无法感知属主死亡的问题。
 */
static int OpenRpcServerLockFile(RPCServerType_e type)
{
    char path[64];

    (void)g_snprintf(path, sizeof(path), RPC_SERVER_LOCK_PATH_FMT,
                     (uint32_t)type);

    return open(path, O_CREAT | O_RDWR | O_CLOEXEC, 0666);
}

/**
 * @brief Server 启动时获取 type 专属存活锁
 *
 * @return >=0 已持锁的 fd，必须在 Server 线程退出时 close
 * @return -1  获取失败或同 type Server 已存在
 */
static int AcquireRpcServerLock(RPCServerType_e type)
{
    int     fd       = OpenRpcServerLockFile(type);
    int32_t retry    = 0;
    bool    acquired = false;

    if (fd == -1)
    {
        LOG_Error("AcquireRpcServerLock: open failed type[0x%08x] [%s]",
                  type, g_strerror(errno));
    }
    else
    {
        /*
         * 客户端确认离线后会短暂持锁清理积压，Server 重启时
         * 给清理留出最多 1 秒。使用有界重试而非永久阻塞，防止
         * 重复启动同 type Server 时 RpcStartMsgServer 永久卡住。
         */
        while ((!acquired) && (retry < RPC_SERVER_LOCK_RETRY_COUNT))
        {
            if (flock(fd, LOCK_EX | LOCK_NB) == 0)
            {
                acquired = true;
            }
            else if ((errno == EWOULDBLOCK) || (errno == EAGAIN) ||
                     (errno == EINTR))
            {
                retry++;
                g_usleep((gulong)RPC_SERVER_LOCK_RETRY_US);
            }
            else
            {
                retry = RPC_SERVER_LOCK_RETRY_COUNT;
            }
        }

        if (!acquired)
        {
            LOG_Error("AcquireRpcServerLock: lock timeout/failed type[0x%08x] [%s]",
                      type, g_strerror(errno));
            (void)close(fd);
            fd = -1;
        }
    }

    return fd;
}

/**
 * @brief 清理全局 RPC 请求队列中指定 Server type 的所有积压请求
 *
 * 仅允许在客户端已取得该 type 存活锁（确认 Server 离线）时调用。
 */
static void DrainOfflineRpcRequests(RPCServerType_e type)
{
    int             qid     = msgget(RPC_REQ_QUEUE_KEY, 0644);
    int32_t         dropped = 0;
    RpcQueueData_t *msgData = NULL;

    if (qid == -1)
    {
        if (errno != ENOENT)
        {
            LOG_Warn("DrainOfflineRpcRequests: msgget failed type[0x%08x] [%s]",
                     type, g_strerror(errno));
        }
    }
    else
    {
        msgData = (RpcQueueData_t *)g_try_malloc0(
                      sizeof(RpcQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);

        if (msgData == NULL)
        {
            LOG_Error("DrainOfflineRpcRequests: malloc failed type[0x%08x]", type);
        }
        else
        {
            bool draining = true;

            while (draining)
            {
                ssize_t rcvRet = msgrcv(qid, msgData, RPCQUE_MSG_BODY_SIZE,
                                        (long)type, IPC_NOWAIT | MSG_NOERROR);

                if (rcvRet >= 0)
                {
                    dropped++;
                    (void)memset(msgData, 0,
                                 sizeof(RpcQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);
                }
                else
                {
                    draining = false;
                    if ((errno != ENOMSG) && (errno != EIDRM) && (errno != EINVAL))
                    {
                        LOG_Warn("DrainOfflineRpcRequests: msgrcv failed type[0x%08x] [%s]",
                                 type, g_strerror(errno));
                    }
                }
            }

            g_free(msgData);
            msgData = NULL;
        }
    }

    if (dropped > 0)
    {
        LOG_Warn("DrainOfflineRpcRequests: dropped %d stale request(s) type[0x%08x]",
                 dropped, type);
    }
}

/**
 * @brief 客户端发送前检查指定 RPC Server 是否在线
 *
 * Server 在线时持有排他锁，客户端尝试取锁会得到 EWOULDBLOCK。
 * 客户端能取得锁则说明 Server 离线；在释放锁前清理该 type 的积压
 * 请求，保证 Server 重启与清理不会并发。
 */
static bool RpcServerIsOnline(RPCServerType_e type)
{
    bool online = false;
    int  fd     = OpenRpcServerLockFile(type);

    if (fd == -1)
    {
        LOG_Error("RpcServerIsOnline: open failed type[0x%08x] [%s]",
                  type, g_strerror(errno));
    }
    else if (flock(fd, LOCK_EX | LOCK_NB) == 0)
    {
        DrainOfflineRpcRequests(type);
        (void)flock(fd, LOCK_UN);
    }
    else
    {
        int lockErrno = errno;

        if ((lockErrno == EWOULDBLOCK) || (lockErrno == EAGAIN))
        {
            online = true;
        }
        else
        {
            LOG_Error("RpcServerIsOnline: lock failed type[0x%08x] [%s]",
                      type, g_strerror(lockErrno));
        }
    }

    if (fd != -1)
    {
        (void)close(fd);
    }

    return online;
}

/* ==========================================================================
 * 内部函数：消息队列底层收发
 * ========================================================================== */

/**
 * @brief 向指定 keyId 的 System V 队列发送 Pub/Sub 消息
 *
 * 使用 IPC_NOWAIT，队列满时立即返回失败（EAGAIN），由调用方决定处理策略。
 *
 * @param[in] keyId  目标队列 key
 * @param[in] data   消息缓冲区（mtype 字段由调用方填好）
 * @param[in] len    总消息结构体长度（含 mtype 字段）
 * @return RET_OK   发送成功
 * @return RET_ERR  队列不存在或发送失败
 */
static int32_t SendDataToQueue(int32_t keyId, SysQueueData_t *data, int32_t len)
{
    int32_t result = RET_ERR;
    int     fd     = msgget(keyId, 0644);

    if (fd == -1)
    {
        LOG_Error("SendDataToQueue: msgget failed keyId[%d] [%s]",
                  keyId, g_strerror(errno));
    }
    else
    {
        if (msgsnd(fd, (const void *)data,
                   (size_t)len - sizeof(long), IPC_NOWAIT) == -1)
        {
            LOG_Error("SendDataToQueue: msgsnd failed keyId[%d] [%s] len[%d]",
                      keyId, g_strerror(errno), len);
        }
        else
        {
            result = RET_OK;
        }
    }

    return result;
}

/**
 * @brief 向指定 keyId 的 System V 队列发送 RPC 消息
 *
 * 使用 IPC_NOWAIT，队列满时立即返回失败。
 *
 * @param[in] keyId  目标队列 key
 * @param[in] data   RPC 消息缓冲区（mtype 字段由调用方填好）
 * @param[in] len    总消息结构体长度（含 mtype 字段）
 * @return RET_OK   发送成功
 * @return RET_ERR  队列不存在或发送失败
 */
static int32_t SendDataToRpcQueue(int32_t keyId, RpcQueueData_t *data, int32_t len)
{
    int32_t result = RET_ERR;
    int     fd     = msgget(keyId, 0644);

    if (fd == -1)
    {
        LOG_Error("SendDataToRpcQueue: msgget failed keyId[%d] [%s]",
                  keyId, g_strerror(errno));
    }
    else
    {
        if (msgsnd(fd, (const void *)data,
                   (size_t)len - sizeof(long), IPC_NOWAIT) == -1)
        {
            LOG_Error("SendDataToRpcQueue: msgsnd failed keyId[%d] [%s] len[%d]",
                      keyId, g_strerror(errno), len);
        }
        else
        {
            result = RET_OK;
        }
    }

    return result;
}

/**
 * @brief 从 System V 队列接收 Pub/Sub 消息
 *
 * timeOut < 0 时阻塞等待；timeOut >= 0 时以 RECV_TIMEOUT_SLEEP_INTERVAL_MS
 * 为步长轮询，直到收到消息或超时。
 *
 * @param[in]  keyId    队列 key
 * @param[out] msgData  接收缓冲区（不可为 NULL）
 * @param[in]  timeOut  超时（ms），< 0 阻塞，>= 0 超时轮询
 * @return >0   实际接收的消息体字节数（不含 mtype）
 * @return RET_ERR  失败或超时
 */
static int32_t GetQueueMsg(int32_t keyId, SysQueueData_t *msgData, int32_t timeOut)
{
    int32_t result = RET_ERR;
    int     fd     = -1;

    if (msgData == NULL)
    {
        LOG_Error("GetQueueMsg: msgData is NULL");
    }
    else
    {
        fd = msgget(keyId, 0644);

        if (fd == -1)
        {
            LOG_Error("GetQueueMsg: msgget failed keyId[%d] [%s]",
                      keyId, g_strerror(errno));
        }
        else if (timeOut < 0)
        {
            /* 阻塞接收：使用负数type排除RPC响应消息（type >= RPC_RESP_TYPE_BASE）
             * msgrcv的负数type表示接收type <= |msgtyp|的消息
             */
            long msgtyp = -(RPC_RESP_TYPE_BASE - 1);
            ssize_t rcvRet = msgrcv(fd, msgData, SYSQUE_MSG_BODY_SIZE, msgtyp, 0);
            if (rcvRet == -1)
            {
                LOG_Warn("GetQueueMsg: msgrcv block failed keyId[%d] [%s]",
                         keyId, g_strerror(errno));
            }
            else
            {
                result = (int32_t)rcvRet;
            }
        }
        else
        {
            /* 超时轮询：排除RPC响应消息 */
            int32_t sleepCount = timeOut / RECV_TIMEOUT_SLEEP_INTERVAL_MS;
            bool    done       = false;
            long    msgtyp     = -(RPC_RESP_TYPE_BASE - 1);

            while (!done)
            {
                ssize_t rcvRet = msgrcv(fd, msgData, SYSQUE_MSG_BODY_SIZE,
                                        msgtyp, IPC_NOWAIT);
                if (rcvRet != -1)
                {
                    result = (int32_t)rcvRet;
                    done   = true;
                }
                else if (errno == ENOMSG)
                {
                    if (sleepCount <= 0)
                    {
                        done = true;
                    }
                    else
                    {
                        g_usleep(1000U * (gulong)RECV_TIMEOUT_SLEEP_INTERVAL_MS);
                        sleepCount--;
                    }
                }
                else
                {
                    LOG_Warn("GetQueueMsg: msgrcv poll failed [%s]", g_strerror(errno));
                    done = true;
                }
            }
        }
    }

    return result;
}

/**
 * @brief 从 RPC 队列接收消息（按 msgType 过滤）
 *
 * timeout <= 0 时使用默认 5 秒；timeout > 0 时按指定超时轮询。
 * 超时返回 RET_ERR_NORMAL_TIMEOUT，其他错误返回 RET_ERR。
 *
 * @param[in]  keyId    队列 key
 * @param[out] msgData  接收缓冲区（不可为 NULL）
 * @param[in]  msgType  System V msgrcv 的 msgtyp 参数（按 type 过滤）
 * @param[in]  timeout  超时（ms），<= 0 使用默认 5 秒，> 0 按指定值超时
 * @return >0                     实际接收的消息体字节数（不含 mtype）
 * @return RET_ERR                失败
 * @return RET_ERR_NORMAL_TIMEOUT 超时
 */
static int32_t GetRpcQueueMsg(int32_t         keyId,
                              RpcQueueData_t *msgData,
                              long            msgType,
                              int32_t         timeout)
{
    int32_t result = RET_ERR;
    int     fd     = -1;

    if (msgData == NULL)
    {
        LOG_Error("GetRpcQueueMsg: msgData is NULL");
    }
    else
    {
        fd = msgget(keyId, 0644);

        if (fd == -1)
        {
            LOG_Error("GetRpcQueueMsg: msgget failed keyId[%d] [%s]",
                      keyId, g_strerror(errno));
        }
        else
        {
            /* 超时轮询：timeout<=0时使用默认5秒，避免永久阻塞 */
            int32_t actualTimeout = (timeout <= 0) ? 5000 : timeout;
            int32_t sleepCount = actualTimeout / RECV_TIMEOUT_SLEEP_INTERVAL_MS;
            bool    done       = false;

            result = RET_ERR_NORMAL_TIMEOUT;

            while (!done)
            {
                ssize_t rcvRet = msgrcv(fd, msgData, RPCQUE_MSG_BODY_SIZE,
                                        msgType, IPC_NOWAIT);
                if (rcvRet != -1)
                {
                    result = (int32_t)rcvRet;
                    done   = true;
                }
                else if (errno == ENOMSG)
                {
                    if (sleepCount <= 0)
                    {
                        done = true;
                    }
                    else
                    {
                        g_usleep(1000U * (gulong)RECV_TIMEOUT_SLEEP_INTERVAL_MS);
                        sleepCount--;
                    }
                }
                else
                {
                    LOG_Warn("GetRpcQueueMsg: msgrcv poll failed [%s]", g_strerror(errno));
                    result = RET_ERR;
                    done   = true;
                }
            }
        }
    }

    return result;
}

/**
 * @brief 校验 RPC 报文头中的 datalen 与 msgrcv 实际长度是否一致
 *
 * msgrcv 返回值不包含 long mtype。所有拷贝 datalen 指定的 payload 之前
 * 都必须调用本函数，防止损坏或异常消息造成越界读。
 */
static bool IsRpcMessageValid(const RpcQueueData_t *msgData,
                              int32_t               receivedBodyLen)
{
    bool   valid       = false;
    size_t expectedLen = 0U;

    if ((msgData != NULL) &&
        (receivedBodyLen >= (int32_t)RPCQUE_HEADER_BODY_SIZE) &&
        (msgData->datalen <= (uint16_t)SYSQUE_MSG_DATA_MAX_LEN))
    {
        expectedLen = RPCQUE_HEADER_BODY_SIZE + (size_t)msgData->datalen;
        valid = ((size_t)receivedBodyLen == expectedLen);
    }

    return valid;
}

/* ==========================================================================
 * 内部函数：进程内回调树初始化
 * ========================================================================== */

/**
 * @brief 初始化进程内回调查找树（幂等）
 */
static void InitCallbackTree(void)
{
    if (g_CallbackTree == NULL)
    {
        g_CallbackTree = g_tree_new_full(CallbackTreeCompareFunc, NULL, NULL, NULL);
    }
}

/**
 * @brief 等待回调节点不再被接收线程使用并释放
 *
 * 调用方必须已持有 g_callbackTreeMutex，且节点必须已从
 * g_CallbackTree 摘除，确保不会再产生新的 activeCalls 引用。
 */
static void WaitAndFreeCallbackLocked(IPCCallback_t *cbMsg)
{
    if (cbMsg != NULL)
    {
        while (cbMsg->activeCalls > 0U)
        {
            (void)pthread_cond_wait(&g_callbackTreeCond,
                                    &g_callbackTreeMutex);
        }

        g_free(cbMsg);
    }
}

/**
 * @brief 释放接收线程持有的一个回调节点引用
 *
 * 调用方必须已持有 g_callbackTreeMutex。
 */
static void ReleaseCallbackLocked(IPCCallback_t *cbMsg)
{
    if ((cbMsg != NULL) && (cbMsg->activeCalls > 0U))
    {
        cbMsg->activeCalls--;
        if (cbMsg->activeCalls == 0U)
        {
            (void)pthread_cond_broadcast(&g_callbackTreeCond);
        }
    }
}

/* ==========================================================================
 * 内部函数：Pub/Sub 消息队列初始化
 * ========================================================================== */

/**
 * @brief 创建本线程/进程专属的 Pub/Sub 接收队列
 *
 * 使用线程 tid 作为 System V msgget key，保证每个订阅进程独立。
 * 成功后将 key 写入模块全局变量 g_KeyId（仅写一次）。
 *
 * @return >0   成功，返回 keyId（= tid）
 * @return RET_ERR  失败
 */
static int32_t InitSysQueMsg(void)
{
    int32_t result = RET_ERR;
    int32_t keyId  = (int32_t)syscall(SYS_gettid);

    if (keyId == -1)
    {
        LOG_Error("InitSysQueMsg: gettid failed [%s]", g_strerror(errno));
    }
    else
    {
        /* 先清理/登记，再创建队列，避免 TID 复用时误删刚创建的新队列。 */
        if (RegisterIpcQueue(keyId) != RET_OK)
        {
            LOG_Error("InitSysQueMsg: register failed keyId[%d]", keyId);
        }
        else if (msgget(keyId, IPC_CREAT | 0644) == -1)
        {
            LOG_Error("InitSysQueMsg: msgget create failed keyId[%d] [%s]",
                      keyId, g_strerror(errno));
            RollbackIpcQueueRegistration(keyId);
        }
        else
        {
            g_KeyId = keyId;
            LOG_Debug("InitSysQueMsg: success pid[%d] keyId[%d]", getpid(), keyId);
            result = keyId;
        }
    }

    return result;
}

/* ==========================================================================
 * 内部函数：Pub/Sub 接收线程
 * ========================================================================== */

/**
 * @brief Pub/Sub 消息接收线程主函数
 *
 * 循环阻塞等待消息，通过进程内 GTree 查找回调并分发。
 * 回调入参 data 使用 4 字节对齐分配。
 * data 指针仅在回调返回前有效，回调内不得持有。
 */
static void *SysQueMsgRecvThread(void *userdata)
{
    SysQueueData_t *recvData = NULL;

    (void)userdata;

    recvData = (SysQueueData_t *)g_try_malloc0(
                   sizeof(SysQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);

    if (recvData == NULL)
    {
        LOG_Error("SysQueMsgRecvThread: malloc failed, thread exit");
    }
    else
    {
        while (1)
        {
            memset(recvData, 0x00, sizeof(SysQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);

            if (GetQueueMsg(g_KeyId, recvData, -1) == RET_ERR)
            {
                LOG_Error("SysQueMsgRecvThread: GetQueueMsg failed, retry");
                continue;
            }

            SYSQueMsgType_e recvMsgType = recvData->type;

            /* 跳过RPC响应消息，这些消息应由RpcGetMsgSyncEx接收 */
            if ((long)recvMsgType >= RPC_RESP_TYPE_BASE)
            {
                LOG_Debug("SysQueMsgRecvThread: skip RPC response type[0x%08x]", recvMsgType);
                continue;
            }

            QueSubCallback callbackFunc = NULL;
            void          *callbackData = NULL;
            bool           callbackFound = false;

            (void)pthread_mutex_lock(&g_callbackTreeMutex);
            IPCCallback_t *cbMsg =
                (IPCCallback_t *)g_tree_lookup(
                    g_CallbackTree, GINT_TO_POINTER((gint)recvMsgType));
            if (cbMsg != NULL)
            {
                callbackFunc = cbMsg->callbackFuc;
                callbackData = cbMsg->userdata;
                callbackFound = true;
                if (callbackFunc != NULL)
                {
                    cbMsg->activeCalls++;
                }
            }
            (void)pthread_mutex_unlock(&g_callbackTreeMutex);

            if (!callbackFound)
            {
                LOG_Error("SysQueMsgRecvThread: no callback for type[0x%08x]",
                          recvMsgType);
                continue;
            }

            if (callbackFunc == NULL)
            {
                LOG_Warn("SysQueMsgRecvThread: NULL callback for type[0x%08x]",
                         recvMsgType);
                continue;
            }

            /*
             * 分配 4 字节对齐缓冲区并拷贝数据，传给回调。
             * datalen == 0 时直接传 NULL，跳过分配与拷贝。
             * 回调返回后立即释放，回调内不得持有此指针。
             */
            void    *buffer  = NULL;
            uint16_t dataLen = recvData->datalen;

            if (dataLen > 0U)
            {
                buffer = aligned_alloc(4U, AlignUp(4U, (size_t)dataLen));
                if (buffer == NULL)
                {
                    LOG_Error("SysQueMsgRecvThread: aligned_alloc failed type[0x%08x]",
                              recvMsgType);
                    (void)pthread_mutex_lock(&g_callbackTreeMutex);
                    ReleaseCallbackLocked(cbMsg);
                    (void)pthread_mutex_unlock(&g_callbackTreeMutex);
                    continue;
                }
                memcpy(buffer, recvData->data, dataLen);
            }

            callbackFunc((const uint8_t *)buffer, dataLen, callbackData);

            /*
             * 与退订/替换配对：回调完成前节点及 userdata 引用保持有效。
             * activeCalls 归零时唤醒等待释放旧节点的线程。
             */
            (void)pthread_mutex_lock(&g_callbackTreeMutex);
            ReleaseCallbackLocked(cbMsg);
            cbMsg = NULL;
            (void)pthread_mutex_unlock(&g_callbackTreeMutex);

            free(buffer);
            buffer = NULL;
        }

        g_free(recvData);
        recvData = NULL;
    }

    LOG_Error("SysQueMsgRecvThread: exit unexpectedly");
    return NULL;
}

/* ==========================================================================
 * 内部函数：RPC 工作线程
 * ========================================================================== */

/**
 * @brief RPC 线程池工作函数
 *
 * 执行用户回调，将响应打包后发送到客户端线程专属回复队列。
 *
 * 回复队列设计：
 *   队列 key  = clientTid（每线程独立队列，无竞争）
 *   消息 type = RPC_RESP_TYPE_BASE + clientTid（唯一标识该线程的回复）
 *
 * userdata 由 RpcTaskData_t 携带，不依赖 GThreadPool 的 pool-level userdata，
 * 支持多 Server 实例并发而不错配。
 *
 * @param[in] taskData  RpcTaskData_t * 指针（堆内存，本函数负责释放）
 * @param[in] poolData  未使用（设为 NULL 可忽略）
 */
static void RpcTaskWorker(gpointer taskData, gpointer poolData)
{
    RpcTaskData_t     *task      = (RpcTaskData_t *)taskData;
    RpcQueueData_t    *recvData  = NULL;
    RpcServerCallback  cbFunc    = NULL;
    void              *userdata  = NULL;
    void              *reqBuffer = NULL;
    uint8_t           *respData  = NULL;
    RpcQueueData_t    *queueData = NULL;
    uint16_t           reqDataLen = 0U;
    uint16_t           respLen   = 0U;
    int32_t            sendLen   = 0;
    bool               ok        = true;

    (void)poolData;

    recvData  = task->recvData;
    cbFunc    = task->callbackFuc;
    userdata  = task->userdata;
    reqDataLen = recvData->datalen;

    /* Step1: 为用户回调准备 4 字节对齐的请求缓冲区 */
    if (reqDataLen > 0U)
    {
        reqBuffer = aligned_alloc(4U, AlignUp(4U, (size_t)reqDataLen));
        if (reqBuffer == NULL)
        {
            LOG_Error("RpcTaskWorker: aligned_alloc reqBuffer failed type[0x%08lx]",
                      (long)recvData->type);
            ok = false;
        }
        else
        {
            memcpy(reqBuffer, recvData->data, reqDataLen);
        }
    }

    /* Step2: 为用户回调预分配响应缓冲区 */
    if (ok)
    {
        respData = (uint8_t *)g_try_malloc0(SYSQUE_MSG_DATA_MAX_LEN);
        if (respData == NULL)
        {
            LOG_Error("RpcTaskWorker: malloc respData failed");
            ok = false;
        }
    }

    /* Step3: 执行用户回调 */
    if (ok)
    {
        cbFunc(recvData->subcmd,
               (const uint8_t *)reqBuffer,
               reqDataLen,
               respData,
               &respLen,
               userdata);

        /* 安全检查：如果回调未设置respLen，提供默认错误响应 */
        if (respLen == 0U)
        {
            LOG_Warn("RpcTaskWorker: callback didn't set respLen, using default error response");
            respData[0] = (uint8_t)RET_ERR;
            respLen = 1U;
        }

        /* 防止用户回调写越界 */
        if (respLen > SYSQUE_MSG_DATA_MAX_LEN)
        {
            LOG_Error("RpcTaskWorker: respLen[%u] exceeds max[%u], truncate",
                      respLen, (uint16_t)SYSQUE_MSG_DATA_MAX_LEN);
            respLen = (uint16_t)SYSQUE_MSG_DATA_MAX_LEN;
        }
    }

    /* Step4: 打包响应消息 */
    if (ok)
    {
        sendLen   = (int32_t)(sizeof(RpcQueueData_t) + respLen);
        queueData = (RpcQueueData_t *)g_try_malloc0((gsize)sendLen);

        if (queueData == NULL)
        {
            LOG_Error("RpcTaskWorker: malloc queueData failed");
            ok = false;
        }
        else
        {
            /*
             * 回复消息 type = RPC_RESP_TYPE_BASE + clientTid。
             * clientTid 为 int32_t，RPC_RESP_TYPE_BASE 为 long，
             * 两者相加结果赋给 RPCServerType_e（底层 int32_t）。
             * tid 值不超过 INT32_MAX，强转安全，消除 -Wconversion 警告。
             */
            queueData->type      = (RPCServerType_e)(int32_t)
                                   (RPC_RESP_TYPE_BASE + (long)recvData->clienttid);
            queueData->subcmd    = recvData->subcmd;
            queueData->clienttid = recvData->clienttid;
            queueData->datalen   = respLen;
            memmove(&queueData->data, respData, respLen);
        }
    }

    /* Step5: 确认客户端线程仍存活后发送回复 */
    if (ok)
    {
        int32_t clientQueueKey = recvData->clienttid;

        if (syscall(SYS_tkill, (int)clientQueueKey, 0) == 0)
        {
            SendDataToRpcQueue(clientQueueKey, queueData, sendLen);
        }
        else
        {
            LOG_Warn("RpcTaskWorker: client tid[%d] died before reply, drop response",
                     recvData->clienttid);
        }
    }

    /* 统一释放所有堆内存 */
    free(reqBuffer);
    reqBuffer = NULL;

    g_free(respData);
    respData = NULL;

    g_free(queueData);
    queueData = NULL;

    g_free(recvData);
    recvData = NULL;

    g_free(task);
    task = NULL;
}

/**
 * @brief RPC 服务端主接收线程
 *
 * 持续从全局请求队列按 type 接收请求，校验客户端存活性后，
 * 将任务分发给线程池（poolSize > 1）或直接调用 RpcTaskWorker（poolSize == 1）。
 *
 * 线程池的 userdata 参数设为 NULL（unused），所有 userdata 由任务自身携带，
 * 避免多 Server 实例间的错配。
 *
 * @param[in] data  RpcServerThreadArgs_t *（堆内存，本函数尾部释放）
 */
static void *RpcServerThread(void *data)
{
    RpcServerThreadArgs_t *args      = (RpcServerThreadArgs_t *)data;
    RPCServerType_e        type      = args->type;
    RpcServerCallback      cbFunc    = args->callbackFuc;
    uint8_t                poolSize  = args->poolSize;
    void                  *userdata  = args->userdata;
    int                    serverLockFd = args->serverLockFd;
    GThreadPool           *pool      = NULL;
    RpcQueueData_t        *recvData  = NULL;
    bool                   running   = true;

    /* Step1: 按需创建线程池（poolSize > 1） */
    if (poolSize > 1U)
    {
        pool = g_thread_pool_new(RpcTaskWorker, NULL, (gint)poolSize, TRUE, NULL);
        if (pool == NULL)
        {
            LOG_Error("RpcServerThread: create thread pool failed, fallback to single");
            poolSize = RPC_THREAD_POOL_SIZE_MIN;
        }
    }

    /* Step2: 分配接收缓冲区 */
    recvData = (RpcQueueData_t *)g_try_malloc0(
                   sizeof(RpcQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);

    if (recvData == NULL)
    {
        LOG_Error("RpcServerThread: malloc recvData failed, thread exit");
        running = false;
    }

    /* Step3: 主循环 */
    gint64 lastReapUs = g_get_monotonic_time();

    while (running)
    {
        memset(recvData, 0x00, sizeof(RpcQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);

        /* 后台低频回收孤儿队列：每 IPC_REAP_INTERVAL_SEC 触发一次 */
        gint64 nowUs = g_get_monotonic_time();
        if ((nowUs - lastReapUs) >= ((gint64)IPC_REAP_INTERVAL_SEC * G_USEC_PER_SEC))
        {
            IpcReapDeadQueues();
            lastReapUs = nowUs;
        }

        int32_t ret = GetRpcQueueMsg(RPC_REQ_QUEUE_KEY, recvData, (long)type, 0);
        if (ret < 0)
        {
            if (ret != RET_ERR_NORMAL_TIMEOUT)
            {
                LOG_Error("RpcServerThread: GetRpcQueueMsg failed, ret=%d", ret);
            }
            continue;
        }

        if (!IsRpcMessageValid(recvData, ret))
        {
            LOG_Error("RpcServerThread: invalid request type[0x%08lx] bodyLen[%d] datalen[%u]",
                      (long)type, ret, (unsigned int)recvData->datalen);
            continue;
        }

        // LOG_Debug("RpcServerThread: received request type[0x%08lx] subcmd[%u] from pid[%d] tid[%d]",
        //           (long)type, recvData->subcmd, getpid(), recvData->clienttid);

        /* 客户端存活性校验：死亡请求直接丢弃，避免资源泄漏 */
        if (syscall(SYS_tkill, (int)recvData->clienttid, 0) != 0)
        {
            LOG_Warn("RpcServerThread: client tid[%d] dead, discard request",
                     recvData->clienttid);
            continue;
        }

        /* 拷贝请求数据给工作线程，避免 recvData 被下次接收覆盖 */
        int32_t         copyLen      = (int32_t)(sizeof(RpcQueueData_t) + recvData->datalen);
        RpcQueueData_t *recvDataCopy = (RpcQueueData_t *)g_try_malloc0((gsize)copyLen);
        RpcTaskData_t  *taskData     = NULL;
        bool            taskOk       = true;

        if (recvDataCopy == NULL)
        {
            LOG_Error("RpcServerThread: malloc recvDataCopy failed");
            taskOk = false;
        }

        if (taskOk)
        {
            memcpy(recvDataCopy, recvData, (size_t)copyLen);

            taskData = (RpcTaskData_t *)g_try_malloc0(sizeof(RpcTaskData_t));
            if (taskData == NULL)
            {
                LOG_Error("RpcServerThread: malloc taskData failed");
                g_free(recvDataCopy);
                recvDataCopy = NULL;
                taskOk = false;
            }
        }

        if (taskOk)
        {
            taskData->callbackFuc = cbFunc;
            taskData->userdata    = userdata;
            taskData->recvData    = recvDataCopy;

            if (poolSize > 1U)
            {
                GError *error = NULL;

                if (!g_thread_pool_push(pool, taskData, &error))
                {
                    LOG_Error("RpcServerThread: thread pool push failed [%s]",
                              (error != NULL) ? error->message : "unknown");
                    if (error != NULL)
                    {
                        g_error_free(error);
                    }
                    g_free(recvDataCopy);
                    g_free(taskData);
                }
            }
            else
            {
                RpcTaskWorker(taskData, NULL);
            }
        }
    }

    /* 清理资源（正常情况下不会到达，仅在 malloc 失败时执行） */
    if (pool != NULL)
    {
        g_thread_pool_free(pool, TRUE, TRUE);
        pool = NULL;
    }

    g_free(recvData);
    recvData = NULL;

    if (serverLockFd != -1)
    {
        (void)flock(serverLockFd, LOCK_UN);
        (void)close(serverLockFd);
        serverLockFd = -1;
    }

    g_free(args);
    args = NULL;

    LOG_Error("RpcServerThread: exit unexpectedly");
    return NULL;
}

/* ==========================================================================
 * 公开接口实现：Pub/Sub
 * ========================================================================== */

int32_t InitSysQueTree(void)
{
    return InitSysQueTreeShm();
}

int32_t DestroySysQueTree(void)
{
    int32_t result = RET_OK;

    if (g_SYSQueTreeShm != NULL)
    {
        pthread_mutex_destroy(&g_SYSQueTreeShm->mutex);
        munmap(g_SYSQueTreeShm, sizeof(SYSQueueMsgShm_t));
        g_SYSQueTreeShm = NULL;
        shm_unlink(TREE_SHM_PATH);
    }

    if (g_CallbackTree != NULL)
    {
        g_tree_destroy(g_CallbackTree);
        g_CallbackTree = NULL;
    }

    return result;
}

bool ipcIsFirstCall(void)
{
    return IpcIsInitialized();
}

bool IpcIsInitialized(void)
{
    bool result = false;

    pthread_mutex_lock(&g_ipcInitMutex);
    result = (g_ipcInitialized == TRUE);
    pthread_mutex_unlock(&g_ipcInitMutex);

    return result;
}

int32_t StartQueueMsgRcvThread(const char *threadName)
{
    int32_t result = RET_OK;

    /* Step1: 幂等检查，保证同一进程只初始化一次 */
    pthread_mutex_lock(&g_ipcInitMutex);
    if (g_ipcInitialized)
    {
        LOG_Info("StartQueueMsgRcvThread: already initialized");
        result = RET_ERR;
    }
    else
    {
        g_ipcInitialized = TRUE;
    }
    pthread_mutex_unlock(&g_ipcInitMutex);

    /* Step2: 初始化进程内回调树 */
    if (result == RET_OK)
    {
        InitCallbackTree();

        if (InitSysQueTree() != RET_OK)
        {
            LOG_Error("StartQueueMsgRcvThread: InitSysQueTree failed");
            result = RET_ERR;
        }
    }

    /* Step3: 创建本进程专属 Pub/Sub 接收队列 */
    if (result == RET_OK)
    {
        if (InitSysQueMsg() == RET_ERR)
        {
            LOG_Error("StartQueueMsgRcvThread: InitSysQueMsg failed");
            result = RET_ERR;
        }
    }

    /* Step4: 启动接收线程 */
    if (result == RET_OK)
    {
        if (g_threadRecv == NULL)
        {
            g_threadRecv = g_thread_new(threadName, SysQueMsgRecvThread, NULL);
        }
    }

    return result;
}

int32_t StartQueueMsgRcv(void)
{
    int32_t result = RET_OK;

    /* Step1: 幂等检查 */
    pthread_mutex_lock(&g_ipcInitMutex);
    if (g_ipcInitialized)
    {
        LOG_Info("StartQueueMsgRcv: already initialized");
        result = RET_ERR;
    }
    else
    {
        g_ipcInitialized = TRUE;
    }
    pthread_mutex_unlock(&g_ipcInitMutex);

    /* Step2: 初始化进程内回调树 */
    if (result == RET_OK)
    {
        InitCallbackTree();

        if (InitSysQueTree() != RET_OK)
        {
            LOG_Error("StartQueueMsgRcv: InitSysQueTree failed");
            result = RET_ERR;
        }
    }

    /* Step3: 创建本进程专属 Pub/Sub 接收队列 */
    if (result == RET_OK)
    {
        if (InitSysQueMsg() == RET_ERR)
        {
            LOG_Error("StartQueueMsgRcv: InitSysQueMsg failed");
            result = RET_ERR;
        }
    }

    return result;
}

int32_t SubscriptionQueueMsg(SYSQueMsgType_e type,
                             QueSubCallback  callbackFuc,
                             void           *userdata)
{
    int32_t        result = RET_OK;
    IPCCallback_t *cbMsg  = NULL;
    IPCCallback_t *oldCb  = NULL;

    LOG_Debug("SubscriptionQueueMsg: type[0x%08x]", type);

    /* Step1: 前置检查：IPC 已初始化 */
    if ((g_CallbackTree == NULL) || (g_SYSQueTreeShm == NULL))
    {
        LOG_Error("SubscriptionQueueMsg: not initialized");
        result = RET_ERR;
    }

    /* Step2: 分配回调节点 */
    if (result == RET_OK)
    {
        cbMsg = (IPCCallback_t *)g_try_malloc0(sizeof(IPCCallback_t));
        if (cbMsg == NULL)
        {
            LOG_Error("SubscriptionQueueMsg: malloc failed");
            result = RET_ERR;
        }
        else
        {
            cbMsg->callbackFuc = callbackFuc;
            cbMsg->userdata    = userdata;
        }
    }

    /* Step3: 加锁更新共享内存订阅树 */
    if (result == RET_OK)
    {
        if (ShmSafeMutexLock(&g_SYSQueTreeShm->mutex) != RET_OK)
        {
            LOG_Error("SubscriptionQueueMsg: lock failed");
            g_free(cbMsg);
            cbMsg  = NULL;
            result = RET_ERR;
        }
        else
        {
            UpdateSysQueTreeShm(type, g_KeyId);
            CheckAndCleanDeadSubscribers(type);

            /*
             * 在共享订阅表解锁前安装本地回调，避免发布者已经可见该队列，
             * 而接收线程尚未找到回调的启动窗口。
             */
            (void)pthread_mutex_lock(&g_callbackTreeMutex);
            oldCb = (IPCCallback_t *)g_tree_lookup(
                g_CallbackTree, GINT_TO_POINTER((gint)type));
            g_tree_insert(g_CallbackTree,
                          GINT_TO_POINTER((gint)type),
                          cbMsg);
            (void)pthread_mutex_unlock(&g_callbackTreeMutex);
            pthread_mutex_unlock(&g_SYSQueTreeShm->mutex);

            /*
             * 共享锁必须先释放：旧回调可能正在 SendMsgToAllQueue，
             * 若持共享锁等待会形成锁反转死锁。
             */
            (void)pthread_mutex_lock(&g_callbackTreeMutex);
            WaitAndFreeCallbackLocked(oldCb);
            oldCb = NULL;
            (void)pthread_mutex_unlock(&g_callbackTreeMutex);
        }
    }

    return result;
}

int32_t UnSubscriptionQueueMsg(SYSQueMsgType_e type)
{
    int32_t result = RET_OK;

    LOG_Debug("UnSubscriptionQueueMsg: type[0x%08x]", type);

    /* Step1: 前置检查 */
    if ((g_CallbackTree == NULL) || (g_SYSQueTreeShm == NULL))
    {
        LOG_Error("UnSubscriptionQueueMsg: not initialized");
        result = RET_ERR;
    }

    /* Step2: 加锁更新共享内存 */
    if (result == RET_OK)
    {
        if (ShmSafeMutexLock(&g_SYSQueTreeShm->mutex) != RET_OK)
        {
            LOG_Error("UnSubscriptionQueueMsg: lock failed");
            result = RET_ERR;
        }
        else
        {
            DeleteSysQueTreeShm(type, g_KeyId);
            pthread_mutex_unlock(&g_SYSQueTreeShm->mutex);

            /* Step3: 移除进程内回调节点并释放内存 */
            (void)pthread_mutex_lock(&g_callbackTreeMutex);
            IPCCallback_t *cbMsg =
                (IPCCallback_t *)g_tree_lookup(g_CallbackTree,
                                               GINT_TO_POINTER((gint)type));
            if (cbMsg != NULL)
            {
                if (g_tree_remove(g_CallbackTree, GINT_TO_POINTER((gint)type)))
                {
                    WaitAndFreeCallbackLocked(cbMsg);
                    cbMsg = NULL;
                }
            }
            (void)pthread_mutex_unlock(&g_callbackTreeMutex);
        }
    }

    return result;
}

int32_t SendMsgToAllQueue(SYSQueMsgType_e type,
                          const void     *data,
                          int32_t         len)
{
    int32_t         result    = RET_OK;
    int32_t         keyIds[TYPENODE_KEYID_COUNT];
    int32_t         keyCount  = 0;
    int32_t         deadTids[TYPENODE_KEYID_COUNT];
    int32_t         deadCount = 0;
    SysQueueData_t *queueData = NULL;
    int32_t         sendLen   = 0;
    int32_t         i;
    bool            hasNode   = false;

    /* Step1: 参数检查 */
    if ((data == NULL) || (len <= 0) || (len >= SYSQUE_MSG_DATA_MAX_LEN))
    {
        LOG_Error("SendMsgToAllQueue: invalid param data=%p len=%d", data, len);
        result = RET_ERR;
    }
    else if (g_SYSQueTreeShm == NULL)
    {
        LOG_Error("SendMsgToAllQueue: shared memory not initialized");
        result = RET_ERR;
    }

    /*
     * Step2: 短暂持锁，抓取订阅者 keyID 快照后立即解锁。
     * msgsnd 是 IPC_NOWAIT，本身不会阻塞；但仍是 syscall，多订阅者时
     * 累积开销可观。改为持锁只做 memcpy，其余在锁外完成，显著缩短
     * shm 锁的持有时间，降低发布/订阅并发路径的争用。
     */
    if (result == RET_OK)
    {
        if (ShmSafeMutexLock(&g_SYSQueTreeShm->mutex) != RET_OK)
        {
            LOG_Error("SendMsgToAllQueue: lock failed");
            result = RET_ERR;
        }
        else
        {
            int32_t nodeIndex = GetTypeNodeIndex(type);

            if (nodeIndex != RET_ERR)
            {
                ShmTypeNode_t *node = &g_SYSQueTreeShm->data[nodeIndex];

                hasNode  = true;
                keyCount = node->len;
                if (keyCount > (int32_t)TYPENODE_KEYID_COUNT)
                {
                    keyCount = (int32_t)TYPENODE_KEYID_COUNT; /* 防御性钳制 */
                }
                if (keyCount > 0)
                {
                    memcpy(keyIds,
                           node->keyID,
                           (size_t)keyCount * sizeof(int32_t));
                }
            }

            pthread_mutex_unlock(&g_SYSQueTreeShm->mutex);

            if (!hasNode)
            {
                /* 无订阅者是常态（如 MCU 周期上报帧无人关心），静默返回 */
                result = RET_ERR;
            }
        }
    }

    /* Step3: 锁外分配并组包 */
    if ((result == RET_OK) && (keyCount > 0))
    {
        sendLen   = (int32_t)(sizeof(SysQueueData_t) + (size_t)len);
        queueData = (SysQueueData_t *)g_try_malloc0((gsize)sendLen);

        if (queueData == NULL)
        {
            LOG_Error("SendMsgToAllQueue: malloc failed");
            result = RET_ERR;
        }
        else
        {
            queueData->type    = type;
            queueData->datalen = (uint16_t)len;
            memmove(&queueData->data, data, (size_t)len);
        }
    }

    /* Step4: 锁外广播；失败且属主线程已死的记入 deadTids 待清理 */
    if ((result == RET_OK) && (queueData != NULL))
    {
        for (i = 0; i < keyCount; i++)
        {
            int32_t keyId = keyIds[i];

            if (SendDataToQueue(keyId, queueData, sendLen) == RET_OK)
            {
                continue;
            }

            LOG_Warn("SendMsgToAllQueue: send to keyId[%d] failed, check alive",
                     keyId);

            errno = 0;
            if ((syscall(SYS_tkill, (int)keyId, 0) == -1) &&
                (errno == ESRCH))
            {
                LOG_Warn("SendMsgToAllQueue: tid[%d] dead, will remove", keyId);
                if (deadCount < (int32_t)TYPENODE_KEYID_COUNT)
                {
                    deadTids[deadCount++] = keyId;
                }
            }
            else
            {
                /* 存活或无法确认死亡（如 EPERM）：不做破坏性清理。 */
                LOG_Warn("SendMsgToAllQueue: keyId[%d] not confirmed dead, skip cleanup",
                         keyId);
            }
        }
    }

    /* Step5: 若有死订阅者，短暂重取锁批量清理 */
    if (deadCount > 0)
    {
        if (ShmSafeMutexLock(&g_SYSQueTreeShm->mutex) == RET_OK)
        {
            for (i = 0; i < deadCount; i++)
            {
                int32_t tid = deadTids[i];
                int32_t nodeIndex = GetTypeNodeIndex(type);

                /*
                 * 锁外判死到此处存在时间窗口。重新确认当前 type 仍包含该
                 * tid，且 tid 此刻确实不存在，避免误删复用 tid 的新队列。
                 */
                if ((nodeIndex != RET_ERR) &&
                    (GetKeyIdIndex(&g_SYSQueTreeShm->data[nodeIndex], tid) != RET_ERR))
                {
                    errno = 0;
                    if ((syscall(SYS_tkill, (int)tid, 0) == -1) &&
                        (errno == ESRCH))
                    {
                        int qid = msgget(tid, 0777);

                        if (qid != -1)
                        {
                            (void)msgctl(qid, IPC_RMID, NULL);
                        }
                        (void)DeleteSysQueTreeShm(type, tid);
                    }
                }
            }
            pthread_mutex_unlock(&g_SYSQueTreeShm->mutex);
        }
        else
        {
            LOG_Warn("SendMsgToAllQueue: relock for cleanup failed, %d dead entries skipped",
                     deadCount);
        }
    }

    if (queueData != NULL)
    {
        g_free(queueData);
        queueData = NULL;
    }

    return result;
}

int32_t GetQueueMsgWithType(SYSQueMsgType_e  type,
                            uint8_t         *recvData,
                            uint16_t         recvBufLen,
                            uint16_t        *recvLen,
                            int32_t          timeout)
{
    int32_t         result  = RET_ERR;
    SysQueueData_t *msgData = NULL;
    int             fd      = -1;
    ssize_t         rcvRet  = -1;

    /* Step1: 参数检查 */
    if ((recvData == NULL) || (recvLen == NULL))
    {
        LOG_Error("GetQueueMsgWithType: NULL output pointer");
    }
    else
    {
        /* Step2: 分配内部接收缓冲区 */
        msgData = (SysQueueData_t *)g_try_malloc0(
                      sizeof(SysQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);

        if (msgData == NULL)
        {
            LOG_Error("GetQueueMsgWithType: malloc failed");
        }
        else
        {
            /* Step3: 获取队列 fd */
            fd = msgget(g_KeyId, 0644);

            if (fd == -1)
            {
                LOG_Error("GetQueueMsgWithType: msgget failed keyId[%d] [%s]",
                          g_KeyId, g_strerror(errno));
            }
            else if (timeout <= 0)
            {
                /* Step4a: 阻塞接收，按 type 过滤 */
                rcvRet = msgrcv(fd, msgData, SYSQUE_MSG_BODY_SIZE, (long)type, 0);
            }
            else
            {
                /* Step4b: 超时轮询 */
                int32_t sleepCount = timeout / RECV_TIMEOUT_SLEEP_INTERVAL_MS;
                bool    done       = false;

                while (!done)
                {
                    rcvRet = msgrcv(fd, msgData, SYSQUE_MSG_BODY_SIZE,
                                    (long)type, IPC_NOWAIT);
                    if (rcvRet != -1)
                    {
                        done = true;
                    }
                    else if (errno == ENOMSG)
                    {
                        if (sleepCount <= 0)
                        {
                            done = true;
                        }
                        else
                        {
                            g_usleep(1000U * (gulong)RECV_TIMEOUT_SLEEP_INTERVAL_MS);
                            sleepCount--;
                        }
                    }
                    else
                    {
                        LOG_Warn("GetQueueMsgWithType: msgrcv failed [%s]",
                                 g_strerror(errno));
                        done = true;
                    }
                }
            }

            /* Step5: 拷贝数据到调用方缓冲区 */
            if (rcvRet != -1)
            {
                uint16_t copyLen = msgData->datalen;

                if (copyLen > recvBufLen)
                {
                    LOG_Warn("GetQueueMsgWithType: datalen[%u] > bufLen[%u], truncate",
                             copyLen, recvBufLen);
                    copyLen = recvBufLen;
                }

                memcpy(recvData, msgData->data, copyLen);
                *recvLen = copyLen;
                result   = RET_OK;
            }

            g_free(msgData);
            msgData = NULL;
        }
    }

    return result;
}

/* ==========================================================================
 * 公开接口实现：RPC
 * ========================================================================== */

int32_t CreatRpcQueue(void)
{
    int32_t result = RET_OK;

    if (msgget(RPC_REQ_QUEUE_KEY, IPC_CREAT | 0644) == -1)
    {
        LOG_Error("CreatRpcQueue: create req queue failed [%s]", g_strerror(errno));
        result = RET_ERR;
    }
    else
    {
        // LOG_Debug("CreatRpcQueue: success reqKey[%d]", RPC_REQ_QUEUE_KEY);
    }

    return result;
}

int32_t RpcStartMsgServer(RPCServerType_e   type,
                          RpcServerCallback callbackFuc,
                          uint8_t           poolSize,
                          void             *userdata)
{
    int32_t result       = RET_OK;
    int     serverLockFd = -1;

    /* Step1: 参数检查 */
    if (callbackFuc == NULL)
    {
        LOG_Error("RpcStartMsgServer: callbackFuc is NULL");
        result = RET_ERR;
    }

    /* Step2: 先占有 type 专属存活锁 */
    if (result == RET_OK)
    {
        serverLockFd = AcquireRpcServerLock(type);
        if (serverLockFd == -1)
        {
            result = RET_ERR;
        }
    }

    /* Step3: 确保全局请求队列存在 */
    if (result == RET_OK)
    {
        if (CreatRpcQueue() != RET_OK)
        {
            result = RET_ERR;
        }
    }

    /* Step4: 分配并初始化服务线程参数 */
    if (result == RET_OK)
    {
        RpcServerThreadArgs_t *args =
            (RpcServerThreadArgs_t *)g_try_malloc0(sizeof(RpcServerThreadArgs_t));

        if (args == NULL)
        {
            LOG_Error("RpcStartMsgServer: malloc args failed");
            result = RET_ERR;
        }
        else
        {
            uint8_t clampedPoolSize = poolSize;

            if (clampedPoolSize < RPC_THREAD_POOL_SIZE_MIN)
            {
                clampedPoolSize = RPC_THREAD_POOL_SIZE_MIN;
            }
            else if (clampedPoolSize > RPC_THREAD_POOL_SIZE_MAX)
            {
                clampedPoolSize = RPC_THREAD_POOL_SIZE_MAX;
            }
            else
            {
                /* poolSize 在合法范围内，不修改 */
            }

            args->type         = type;
            args->callbackFuc  = callbackFuc;
            args->poolSize     = clampedPoolSize;
            args->userdata     = userdata;
            args->serverLockFd = serverLockFd;

            gchar threadName[32];
            g_snprintf(threadName, sizeof(threadName), "rpcserver_%ld", (long)type);
            g_thread_new(threadName, RpcServerThread, args);

            /* fd 的生命周期已交给 RpcServerThread */
            serverLockFd = -1;
        }
    }

    if (serverLockFd != -1)
    {
        (void)flock(serverLockFd, LOCK_UN);
        (void)close(serverLockFd);
    }

    return result;
}

/**
 * @brief 确保当前线程专属 RPC 回复队列存在（幂等）
 *
 * 回复队列 key = 当前线程 tid，若已存在则直接成功，若不存在则创建。
 * 创建前登记进注册表：高频调用走快速路径（已登记同 pid 直接返回），
 * 首次登记时顺带回收全表中属主已死的孤儿队列；创建失败则回滚登记。
 *
 * @param[in] tid  当前线程 tid
 * @return RET_OK   队列可用
 * @return RET_ERR  创建失败
 */
static int32_t EnsureRpcReplyQueue(int32_t tid)
{
    int32_t result = RET_OK;

    /* 先清理/登记，再创建队列，避免 TID 复用时误删刚创建的新队列。 */
    if (RegisterIpcQueue(tid) != RET_OK)
    {
        LOG_Error("EnsureRpcReplyQueue: register failed tid[%d]", tid);
        result = RET_ERR;
    }
    else if (msgget(tid, IPC_CREAT | 0644) == -1)
    {
        LOG_Error("EnsureRpcReplyQueue: msgget create failed tid[%d] [%s]",
                  tid, g_strerror(errno));
        RollbackIpcQueueRegistration(tid);
        result = RET_ERR;
    }

    return result;
}

int32_t RpcGetMsgSync(RPCServerType_e  type,
                      uint16_t         subcmd,
                      int32_t          timeOut,
                      const uint8_t   *reqData,
                      uint16_t         reqLen,
                      uint8_t         *respData,
                      uint16_t        *respLen)
{
    return RpcGetMsgSyncEx(type,
                           subcmd,
                           timeOut,
                           reqData,
                           reqLen,
                           respData,
                           (uint16_t)SYSQUE_MSG_DATA_MAX_LEN,
                           respLen);
}

int32_t RpcGetMsgSyncEx(RPCServerType_e  type,
                        uint16_t         subcmd,
                        int32_t          timeOut,
                        const uint8_t   *reqData,
                        uint16_t         reqLen,
                        uint8_t         *respData,
                        uint16_t         respBufLen,
                        uint16_t        *respLen)
{
    int32_t         result       = RET_OK;
    int32_t         threadId     = 0;
    long            replyMsgType = 0;
    RpcQueueData_t *flushBuf     = NULL;
    RpcQueueData_t *queueData    = NULL;
    RpcQueueData_t *recvData     = NULL;
    int32_t         sendLen      = 0;

    /* Step1: 获取当前线程 tid */
    threadId = (int32_t)syscall(SYS_gettid);
    if (threadId == -1)
    {
        LOG_Error("RpcGetMsgSyncEx: gettid failed [%s]", g_strerror(errno));
        result = RET_ERR;
    }

    /* Step2: 参数检查 */
    if (result == RET_OK)
    {
        if (respLen == NULL)
        {
            LOG_Error("RpcGetMsgSyncEx: respLen is NULL");
            result = RET_ERR;
        }
        else if ((respBufLen > 0U) && (respData == NULL))
        {
            LOG_Error("RpcGetMsgSyncEx: respData is NULL with respBufLen[%u]",
                      (unsigned int)respBufLen);
            result = RET_ERR;
        }
        else if (reqLen > (uint16_t)SYSQUE_MSG_DATA_MAX_LEN)
        {
            LOG_Error("RpcGetMsgSyncEx: reqLen[%u] exceeds max[%u]",
                      (unsigned int)reqLen,
                      (unsigned int)SYSQUE_MSG_DATA_MAX_LEN);
            result = RET_ERR;
        }
        else if ((reqLen > 0U) && (reqData == NULL))
        {
            LOG_Error("RpcGetMsgSyncEx: reqData is NULL with reqLen[%u]",
                      (unsigned int)reqLen);
            result = RET_ERR;
        }
        else
        {
            *respLen = 0U;
        }
    }

    /* Step3: Server 离线时快速失败，并清理其在全局队列的积压请求 */
    if (result == RET_OK)
    {
        if (!RpcServerIsOnline(type))
        {
            LOG_Debug("RpcGetMsgSyncEx: server offline type[0x%08x]", type);
            result = RET_ERR;
        }
    }

    /* Step4: 确保线程专属回复队列存在 */
    if (result == RET_OK)
    {
        if (EnsureRpcReplyQueue(threadId) != RET_OK)
        {
            LOG_Error("RpcGetMsgSyncEx: EnsureRpcReplyQueue failed");
            result = RET_ERR;
        }
    }

    /* Step5: 清理线程队列中超时残留的旧回复，防止脏数据污染本次响应 */
    if (result == RET_OK)
    {
        replyMsgType = RPC_RESP_TYPE_BASE + (long)threadId;

        flushBuf = (RpcQueueData_t *)g_try_malloc0(
                       sizeof(RpcQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);

        if (flushBuf == NULL)
        {
            LOG_Error("RpcGetMsgSyncEx: malloc flushBuf failed");
            result = RET_ERR;
        }
        else
        {
            bool flushing = true;

            while (flushing)
            {
                memset(flushBuf, 0,
                       sizeof(RpcQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);

                ssize_t rcvRet = msgrcv(msgget(threadId, 0644), flushBuf,
                                        RPCQUE_MSG_BODY_SIZE, replyMsgType,
                                        IPC_NOWAIT);

                if (rcvRet == -1)
                {
                    /* ENOMSG 表示队列已空，正常退出；其他错误也退出，避免死循环 */
                    flushing = false;
                    if (errno != ENOMSG)
                    {
                        LOG_Error("RpcGetMsgSyncEx: flush msgrcv error [%s]",
                                  g_strerror(errno));
                    }
                }
                else
                {
                    LOG_Warn("RpcGetMsgSyncEx: flushed stale reply tid[%d]", threadId);
                }
            }

            g_free(flushBuf);
            flushBuf = NULL;
        }
    }

    /* Step6: 打包并发送请求 */
    if (result == RET_OK)
    {
        sendLen   = (int32_t)(sizeof(RpcQueueData_t) + (size_t)reqLen);
        queueData = (RpcQueueData_t *)g_try_malloc0((gsize)sendLen);

        if (queueData == NULL)
        {
            LOG_Error("RpcGetMsgSyncEx: malloc queueData failed");
            result = RET_ERR;
        }
        else
        {
            queueData->type      = type;
            queueData->subcmd    = subcmd;
            queueData->clienttid = threadId;
            queueData->datalen   = reqLen;

            if ((reqData != NULL) && (reqLen > 0U))
            {
                memcpy(queueData->data, reqData, reqLen);
            }

            if (SendDataToRpcQueue(RPC_REQ_QUEUE_KEY, queueData, sendLen) != RET_OK)
            {
                LOG_Error("RpcGetMsgSyncEx: send request failed");
                result = RET_ERR;
            }

            g_free(queueData);
            queueData = NULL;
        }
    }

    /* Step7: 等待并接收回复 */
    if (result == RET_OK)
    {
        recvData = (RpcQueueData_t *)g_try_malloc0(
                       sizeof(RpcQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);

        if (recvData == NULL)
        {
            LOG_Error("RpcGetMsgSyncEx: malloc recvData failed");
            result = RET_ERR;
        }
        else
        {
            int32_t rcvRet = GetRpcQueueMsg(threadId,
                                            recvData,
                                            replyMsgType,
                                            timeOut);

            if (rcvRet < RET_OK)
            {
                if (rcvRet == RET_ERR_NORMAL_TIMEOUT)
                {
                    LOG_Error("RpcGetMsgSyncEx: recv reply timeout pid[%d] tid[%d]", getpid(), threadId);
                }
                else
                {
                    LOG_Error("RpcGetMsgSyncEx: recv reply error pid[%d] tid[%d]", getpid(), threadId);
                }
                result = rcvRet;
            }
            else if (!IsRpcMessageValid(recvData, rcvRet))
            {
                LOG_Error("RpcGetMsgSyncEx: invalid response type[0x%08x] bodyLen[%d] datalen[%u]",
                          type, rcvRet, (unsigned int)recvData->datalen);
                result = RET_ERR;
            }
            else
            {
                /* Step8: 拷贝响应数据到调用方缓冲区 */
                uint16_t copyLen = recvData->datalen;

                if (copyLen > respBufLen)
                {
                    LOG_Warn("RpcGetMsgSyncEx: respLen[%u] > bufLen[%u], truncate",
                             copyLen, respBufLen);
                    copyLen = respBufLen;
                }

                if (copyLen > 0U)
                {
                    memcpy(respData, recvData->data, copyLen);
                }
                *respLen = copyLen;
            }

            g_free(recvData);
            recvData = NULL;
        }
    }

    return result;
}
