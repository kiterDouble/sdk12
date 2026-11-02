
#include "sr_ipc.h"
#include "queue_msg.h"
#include "sr_log.h"
#include "err_code.h"
#include <glib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <gio/gio.h>
#include <gmodule.h>
#include <glib/gprintf.h>
#include <flatcc/flatcc_flatbuffers.h>
#include <sys/time.h>
#include <stdatomic.h>

#define TREE_SHM_PATH "/sysQueShareShm"
#define RPC_REQ_QUEUE_KEY (3)                                      // RPC 请求队列
#define RPC_RESP_QUEUE_KEY (4)                                     // RPC 回复队列
#define RPC_THREAD_POOL_SIZE_MAX (4)                               // RPC Server端使用的最大线程池数量
#define RPC_RESP_TYPE_BASE (RPCSERVER_TYPE_MIN_LIMIT + 0x00010000) // RPC 回复的消息类型基础大小
#define TYPENODE_KEYID_COUNT (16)                                  // IPC消息支持的最大订阅数量
#define TYPENODE_COUNT (128)                                       // 参与IPC通信的最大消息数量
#define align_up(alignment, size) \
    (((size_t)(size) + (size_t)(alignment)-1) & ~((size_t)(alignment)-1))

/**
 * @brief TypeNode structure
 *
 */
typedef struct
{
    // 每一个type类型的消息都会有多个订阅 所以len即代表该type类型的订阅数量
    // keyID中保存着该消息的每个订阅队列节点
    SYSQueMsgType_e type;                  /**< type of MsgQueueType*/
    int32_t len;                            /**< the len of keyId*/
    uint32_t keyID[TYPENODE_KEYID_COUNT]; /**< store MsgQueue ID*/
} ShmTypeNode_t;

/**
 * @brief MsgTreeShm structure
 *
 */
typedef struct
{
    atomic_int mutex_initialized;
    pthread_mutex_t mutex;
    int32_t typeNodeLen;                 /**< The len of data*/
    ShmTypeNode_t data[TYPENODE_COUNT]; /**<arrary of TypeNode*/
} SYSQueueMsgShm_t;

typedef struct
{
    QueSubCallback callbackFuc;
    void* userdata;
} IPCCallback_t;

typedef struct
{
    RpcServerCallback callbackFuc;
    RpcQueueData_t* recvData;
} RpcTaskData;

static gboolean ipcFirstCall = TRUE;
static pthread_mutex_t ipcInitMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t rpcRequestMutex = PTHREAD_MUTEX_INITIALIZER;
static SYSQueueMsgShm_t *g_SYSQueTreeShm = NULL;
static int32_t Key_Id = 0;
static GTree *g_CallbackTree = NULL;
GThread *thread_recv = NULL;
GThreadPool *threadPool = NULL;

static int32_t SHMSafeMutexLock(pthread_mutex_t *mutex)
{
    int ret = pthread_mutex_lock(mutex);
    if (ret == EOWNERDEAD)
    {
        // 标记锁为一致
        if (pthread_mutex_consistent(mutex) != 0)
        {
            LOG_Error("Failed to make mutex consistent");
            return RET_ERR;
        }
        pthread_mutex_unlock(mutex);
        ret = pthread_mutex_lock(mutex);
    }
    else if (ret != 0)
    {
        LOG_Error("Lock failed: %s", g_strerror(ret));
        return RET_ERR;
    }
    return RET_OK;
}

static int32_t CallbackTreeCompareFunc(gconstpointer a, gconstpointer b, void* usrdata)
{
    SYSQueMsgType_e ap = GPOINTER_TO_INT(a);
    SYSQueMsgType_e bp = GPOINTER_TO_INT(b);

    if (ap == bp)
    {
        return 0;
    }
    else if (ap > bp)
    {
        return 1;
    }
    return -1;
}

// 映射shm
static int32_t InitSysQueTreeShm()
{
    if (g_SYSQueTreeShm == NULL)
    {
        int32_t fd = shm_open(TREE_SHM_PATH, O_CREAT | O_RDWR, 0644);
        // LOG_Debug("shm_open fd:%d",fd);
        if (fd == -1)
        {
            LOG_Error("Open TreeShm File Error:%s", g_strerror(errno));
            return RET_ERR;
        }
        if (ftruncate(fd, sizeof(SYSQueueMsgShm_t)) != 0)
        {
            LOG_Error("ftruncate TreeShm File error:%s", g_strerror(errno));
            close(fd);
            return RET_ERR;
        }
        // LOG_Debug("ftruncate:%d");
        g_SYSQueTreeShm = mmap(NULL, sizeof(SYSQueueMsgShm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (g_SYSQueTreeShm == MAP_FAILED)
        {
            LOG_Error("mmap TreeShm Error:%s", g_strerror(errno));
            close(fd);
            return RET_ERR;
        }
        // LOG_Debug("mmap:%d", (int)g_SYSQueTreeShm);
        // 初始化共享内存互斥锁
        // LOG_Debug("g_SYSQueTreeShm->mutex_initialized start:%d", g_SYSQueTreeShm->mutex_initialized);
        if (atomic_load(&g_SYSQueTreeShm->mutex_initialized) == 0)
        {
            int32_t expected = 0;
            if (atomic_compare_exchange_strong(&g_SYSQueTreeShm->mutex_initialized, &expected, 1))
            {
                pthread_mutexattr_t mutex_attr;
                if (pthread_mutexattr_init(&mutex_attr) != 0)
                {
                    LOG_Error("Failed to init mutexattr");
                }
                if (pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED) != 0)
                {
                    LOG_Error("Failed to set pshared");
                }
                if (pthread_mutexattr_setrobust(&mutex_attr, PTHREAD_MUTEX_ROBUST) != 0)
                {
                    LOG_Error("Failed to set robust");
                }
                if (pthread_mutex_init(&g_SYSQueTreeShm->mutex, &mutex_attr) != 0)
                {
                    LOG_Error("Failed to init mutex");
                }
                pthread_mutexattr_destroy(&mutex_attr);
            }
        }
        // LOG_Debug("g_SYSQueTreeShm->mutex_initialized fino:%d", g_SYSQueTreeShm->mutex_initialized);
        close(fd);
    }
    LOG_Debug("Init TreeShm Success");
    return RET_OK;
}

// 从shm中查询指定type的数据
static int32_t GetTypeNodeIndexFromShmByType(SYSQueMsgType_e type)
{
    ShmTypeNode_t *temp = NULL;
    int32_t index = 0;
    for (index = 0; index < g_SYSQueTreeShm->typeNodeLen; index++)
    {
        temp = &g_SYSQueTreeShm->data[index];
        if (type == temp->type)
        {
            return index;
        }
    }
    LOG_Warn("Not Found type[0x%08x] in Shm", type);
    return RET_ERR;
}

static int32_t GetKeyIndexFromShmByKeyId(ShmTypeNode_t *temp, int32_t keyId)
{
    int32_t index = 0;
    for (index = 0; index < temp->len; index++)
    {
        // LOG_Debug("%s:%d", __func__, temp->keyID[index]);
        if (temp->keyID[index] == keyId)
        {
            return index;
        }
    }
    LOG_Warn("Not Found KeyID[%d] in Shm", keyId);
    return RET_ERR;
}

/*根据type与id从shm中查找
如果已经存在在shm中 不进行任何操作
如果shm中存在type 但是该type的数组中不存在id 则更新id到该type的数组中
如果都不存在 那么添加该type数组 该type数组中添加id
*/
static void UpdateSysQueTreeShm(SYSQueMsgType_e type, int32_t keyId)
{
    int32_t nodeIndex = GetTypeNodeIndexFromShmByType(type);
    ShmTypeNode_t *temp = NULL;
    if (nodeIndex != RET_ERR)
    {
        temp = &g_SYSQueTreeShm->data[nodeIndex];
        int32_t keyIndex = GetKeyIndexFromShmByKeyId(temp, keyId);
        if (keyIndex == RET_ERR)
        {
            temp->keyID[temp->len] = keyId;
            temp->len++;
            LOG_Debug("Update Shm Success:type[0x%08x]-keyId[%d]-len[%d]", type, keyId, temp->len);
        }
        else
        {
            LOG_Warn("Update Shm Success:KeyId hax exist--type[0x%08x]-keyId[%d]-len[%d]", type, keyId, temp->len);
        }
    }
    else
    {
        /*new */
        temp = &g_SYSQueTreeShm->data[g_SYSQueTreeShm->typeNodeLen];
        temp->type = type;
        temp->len = 0;
        temp->keyID[0] = keyId;
        temp->len++;
        g_SYSQueTreeShm->typeNodeLen++;
        LOG_Debug("Update Shm Success:type[0x%08x]-keyId[%d]-len[%d]", type, keyId, temp->len);
    }
}

/*根据type与id从shm中查找
如果不存在 不进行任何操作
如果shm中存在type 与 id 则删除该type数组中的id
如果shm中存在type 但是该type的id数组中没有数据 那么在shm中删除该type的数组
*/
static bool DeleteSysQueTreeShm(SYSQueMsgType_e type, int32_t keyId)
{
    //函数执行结束后返回一个bool值 表示该type是否被完全清理
    bool typeNodeIsEmpty = false;
    int32_t nodeIndex = GetTypeNodeIndexFromShmByType(type);
    ShmTypeNode_t *temp = NULL;
    if (nodeIndex != RET_ERR)
    {
        temp = &g_SYSQueTreeShm->data[nodeIndex];
        int32_t keyIndex = GetKeyIndexFromShmByKeyId(temp, keyId);
        if (keyIndex != RET_ERR)
        {
            temp->keyID[keyIndex] = temp->keyID[temp->len - 1];
            temp->keyID[temp->len - 1] = 0;
            temp->len--;
            if(temp->len < 0)
            {
                temp->len = 0;
            }
            LOG_Debug("Delete type[0x%08x]-keyId[%d] from shm success", type, keyId);
        }
        if (temp->len == 0)
        {
            /*Del typeNode*/
            LOG_Info("Type[0x%08x] del to 0,clean Type in shm", type);
            int32_t mvIndex = g_SYSQueTreeShm->typeNodeLen - 1;
            memmove(&g_SYSQueTreeShm->data[nodeIndex], &g_SYSQueTreeShm->data[mvIndex], sizeof(ShmTypeNode_t));
            memset(&g_SYSQueTreeShm->data[mvIndex], 0, sizeof(ShmTypeNode_t));
            g_SYSQueTreeShm->typeNodeLen--;
            typeNodeIsEmpty = true;
        }
    }
    else
    {
        LOG_Debug("Delete type[0x%08x]-keyId[%d] falid from shm,Not Found type in shm", type, keyId);
    }
    return typeNodeIsEmpty;
}

int32_t InitSysQueTree()
{
    return InitSysQueTreeShm();
}

void InitCallbackTree()
{
    if (g_CallbackTree == NULL)
    {
        g_CallbackTree = g_tree_new_full(CallbackTreeCompareFunc, NULL, NULL, NULL);
    }
}

// 清除shm
int32_t DestorySysQueTree()
{
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
    return RET_OK;
}

int32_t InitSysQueMsg()
{
    LOG_Debug("%s:Start", __func__);
    int32_t keyId = syscall(SYS_gettid);
    if (keyId == -1)
    {
        LOG_Error("Get keyid error:[%s]", g_strerror(errno));
        return RET_ERR;
    }

    if (msgget(keyId, IPC_CREAT | 0644) == -1)
    {
        LOG_Error("Create App keyid[%d] RecvQueue Error:[%s]", keyId, g_strerror(errno));
        return RET_ERR;
    }
    LOG_Debug("%s:End,InitAtMsg Success,keyId[%d]", __func__, keyId);
    Key_Id = keyId;
    return keyId;
}


int32_t GetQueueMsgCount(const int32_t keyId, int32_t *number)
{
    int32_t fd = msgget(keyId, 0644);
    if (fd == -1)
    {
        LOG_Error("msgget Error: keyid[%d] Error:[%s]", keyId, g_strerror(errno));
        return 0;
    }
    struct msqid_ds msq_info;
    if (msgctl(fd, IPC_STAT, &msq_info) == -1)
    {
        LOG_Error("msgctl Error: keyid[%d] Error:[%s]", keyId, g_strerror(errno));
        return 0;
    }
    *number = msq_info.msg_qnum;
    return 1;
}

int32_t GetQueueMsg(const int32_t keyId, SysQueueData_t *msgData, int32_t timeOut);

void* SysQueMsgRecvThread(void* userdata)
{
    //4.定义接收buf
    SysQueueData_t *recvData = g_try_malloc0(sizeof(SysQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);
    if (recvData == NULL)
    {
        LOG_Error("malloc recv data error");
        g_free(recvData);
        return NULL;
    }
    //4.循环接收
    while(1)
    {
        memset(recvData, 0x00, sizeof(SysQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);
        if (GetQueueMsg(Key_Id, recvData, -1) == RET_ERR)
        {
            LOG_Error("msgrcv Recv Msg Error");
            continue;
        }
        SYSQueMsgType_e recvMsgType = (int)recvData->type;
        const IPCCallback_t *callbackMsg = g_tree_lookup(g_CallbackTree, GINT_TO_POINTER(recvMsgType));
        QueSubCallback callbackFunc = callbackMsg->callbackFuc;
        void* callbackdata = callbackMsg->userdata;
        // g_tree_foreach(g_CallbackTree, PrintTree, NULL);
        if (callbackFunc == NULL)
        {
            LOG_Warn("msgrcv Msg Type[0x%08x] but no callback Func!", recvMsgType);
            continue;
        }
        //flatcc进行verify_as_root操作时需要4字节对齐 所以传递给处理函数时需要对齐操作 详见https://github.com/dvidelabs/flatcc/issues/65
        //这样能保证给回调函数的数据起始位置必然是4的整数倍
        //假设recvData->data是一字节对齐的情况下，aligned_alloc并不会改变这一事实，给回调函数的数据也是一字节对齐的
        void *buffer = aligned_alloc(4, align_up(4, recvData->datalen));
        if(!buffer)
        {
            LOG_Error("msgrcv Msg Type[0x%08x] but aligned_alloc failed!", recvMsgType);
            continue;
        }
        memcpy(buffer, recvData->data, recvData->datalen);
        callbackFunc(buffer, recvData->datalen, callbackdata);
        aligned_free(buffer);
    }
    g_free(recvData);
    LOG_Error("%s:exit",__func__);
    return NULL;
}

void checkQueueMsg(SYSQueMsgType_e type)
{
    int32_t nodeIndex = GetTypeNodeIndexFromShmByType(type);
    ShmTypeNode_t *temp = NULL;
    if (nodeIndex != RET_ERR)
    {
        temp = &g_SYSQueTreeShm->data[nodeIndex];
        int32_t count = 0;
        for (count = 0; count < temp->len; count++)
        {
            if (temp->keyID[count] > 0)
            {
                if (syscall(SYS_tkill, temp->keyID[count], 0) == -1)
                {
                    LOG_Warn("The Thread tid[%d] has dead,and delete the Msg keyId", temp->keyID[count]);
                    msgctl(msgget(temp->keyID[count], 0777), IPC_RMID, NULL);
                    if(DeleteSysQueTreeShm(type, temp->keyID[count]))
                    {
                        break;
                    }
                    count--;
                    // LOG_Debug("%s", strerror(errno));
                }
            }
        }
    }
}

//订阅消息函数
int32_t SubscriptionQueueMsg(SYSQueMsgType_e type, QueSubCallback callbackFuc, void* userdata)
{
    if((g_CallbackTree == NULL) && (g_SYSQueTreeShm == NULL))
    {
        return RET_ERR;
    }
    IPCCallback_t *cbmsg = g_try_malloc0(sizeof(IPCCallback_t));
    if (cbmsg == NULL)
    {
        LOG_Error("malloc error");
        return RET_ERR;
    }
    cbmsg->callbackFuc = callbackFuc;
    cbmsg->userdata = userdata;
    if (SHMSafeMutexLock(&g_SYSQueTreeShm->mutex) != RET_OK)
    {
        return RET_ERR;
    }
    //更新到shm
    UpdateSysQueTreeShm(type, Key_Id);
    //检查这个type下订阅的线程有没有挂掉的 有的话需要把消息队列和shm清除
    checkQueueMsg(type);
    pthread_mutex_unlock(&g_SYSQueTreeShm->mutex);
    g_tree_insert(g_CallbackTree, GINT_TO_POINTER(type), cbmsg);
    return RET_OK;
}

int32_t UnSubscriptionQueueMsg(SYSQueMsgType_e type)
{
    if((g_CallbackTree == NULL) && (g_SYSQueTreeShm == NULL))
    {
        return RET_ERR;
    }
    if (SHMSafeMutexLock(&g_SYSQueTreeShm->mutex) != RET_OK)
    {
        return RET_ERR;
    }
    DeleteSysQueTreeShm(type, Key_Id);
    pthread_mutex_unlock(&g_SYSQueTreeShm->mutex);
    IPCCallback_t *cbmsg = g_tree_lookup(g_CallbackTree, GINT_TO_POINTER(type));
    g_tree_remove(g_CallbackTree, &type);
    if (cbmsg != NULL)
    {
        g_free(cbmsg);
    }
    return RET_OK;
}

#define RECV_TIMEOUT_SLEEP_INTERVAL_MS (2)
int32_t GetQueueMsg(const int32_t keyId, SysQueueData_t *msgData, int32_t timeOut)
{
    if (msgData == NULL)
    {
        LOG_Warn("APP Recv Msg Buff Is NULL");
        return RET_ERR;
    }
    int32_t fd = msgget(keyId, 0644);
    if (fd == -1)
    {
        LOG_Error("Open App keyid[%d] RecvQueue Error:[%s]", keyId, g_strerror(errno));
        return RET_ERR;
    }
    int32_t ret = 0;
    if (timeOut < 0)
    {
        /*recv msg from queue with block*/
        ret = msgrcv(fd, msgData, SYSQUE_MSG_DATA_MAX_LEN + sizeof(SysQueueData_t), 0, 0);
        if (ret == -1)
        {
            LOG_Warn("APP GetData key[%d],recv error[%d-%s]", keyId, errno, g_strerror(errno));
            return RET_ERR;
        }
        else
        {
            // LOG_Debug("APP GetData key[%d],recv [%d] OK,MsgType[0x%08x],datalen[%d]",
            //           keyId, ret, msgData->type, msgData->datalen);
        }
    }
    else
    {
        /*recv msg from queue with timeout*/
        int32_t sleepCount = timeOut / RECV_TIMEOUT_SLEEP_INTERVAL_MS;
        do
        {
            ret = msgrcv(fd, msgData, SYSQUE_MSG_DATA_MAX_LEN + sizeof(SysQueueData_t), 0, IPC_NOWAIT);
            if (ret == -1)
            {
                if (errno == ENOMSG)
                {
                    g_usleep(1000 * RECV_TIMEOUT_SLEEP_INTERVAL_MS);
                    // ret = ERR_MCU_RECVQUEUE_TIMEOUT;
                }
                else
                {
                    break;
                }
            }
            else
            {
                // LOG_Debug("APP GetData key[%d],recv [%d] OK,MsgType[0x%08x],MsgLen[%d]",
                //           keyId, ret, msgData->type, msgData->datalen);
                break;
            }
        } while ((sleepCount--) > 0);
    }
    return ret;
}

int32_t GetQueueMsgWithType(SYSQueMsgType_e type, uint8_t *recvData, uint16_t *recvLen, int32_t timeout)
{
    SysQueueData_t *msgData = g_try_malloc0(sizeof(SysQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);
    if (msgData == NULL)
    {
        LOG_Error("malloc recv data error");
        g_free(msgData);
        return RET_ERR;
    }
    memset(msgData, 0x00, sizeof(SysQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);
    int32_t fd = msgget(Key_Id, 0644);
    if (fd == -1)
    {
        LOG_Error("Open App keyid[%d] RecvQueue Error:[%s]", Key_Id, g_strerror(errno));
        g_free(msgData);
        return RET_ERR;
    }
    int32_t ret = 0;
    /*recv msg from queue with block*/
    if (timeout <= 0)
    {
        ret = msgrcv(fd, msgData, SYSQUE_MSG_DATA_MAX_LEN + sizeof(SysQueueData_t), (long)type, 0);
        if (ret == -1)
        {
            LOG_Warn("APP GetData key[%d],recv error[%d-%s]", Key_Id, errno, g_strerror(errno));
            g_free(msgData);
            return RET_ERR;
        }
    }
    else
    {
        /*recv msg from queue with timeout*/
        int32_t sleepCount = timeout / RECV_TIMEOUT_SLEEP_INTERVAL_MS;
        do
        {
            ret = msgrcv(fd, msgData, SYSQUE_MSG_DATA_MAX_LEN + sizeof(SysQueueData_t), (long)type, IPC_NOWAIT);
            if (ret == -1)
            {
                if (errno == ENOMSG)
                {
                    g_usleep(1000 * RECV_TIMEOUT_SLEEP_INTERVAL_MS);
                }
                else
                {
                    LOG_Warn("APP GetData key[%d],recv error[%d-%s]", Key_Id, errno, g_strerror(errno));
                    break;
                }
            }
            else
            {
                break;
            }
        } while ((sleepCount--) > 0);
    }
    if (recvData == NULL)
    {
        LOG_Error("recvData is null!!");
        g_free(msgData);
        return RET_ERR;
    }
    memcpy(recvData, msgData->data, msgData->datalen);
    *recvLen = msgData->datalen;
    g_free(msgData);
    return ret;
}

int32_t SendDataToQueue(int32_t keyId, SysQueueData_t *data, const int32_t len)
{
    int32_t ret = RET_OK;
    int32_t fd = msgget(keyId, 0644);

    if (fd == -1)
    {
        LOG_Error("msgget failed:keyid[%d]-[%s]", keyId, g_strerror(errno));
        ret = RET_ERR;
    }
    else
    {
        ret = msgsnd(fd, (const void *)data, len - sizeof(long), IPC_NOWAIT);
        if (ret == -1)
        {
            LOG_Error("msgsnd failed:keyId[%d]-[%s]-len[%d]", keyId, g_strerror(errno), len);
            ret = RET_ERR;
        }
        else
        {
            //实际消息体长度=len-6
            ret = RET_OK;
        }
    }
    return ret;
}

int32_t SendDataToRpcQueue(int32_t keyId, RpcQueueData_t *data, const int32_t len)
{
    int32_t ret = RET_OK;
    int32_t fd = msgget(keyId, 0644);

    if (fd == -1)
    {
        LOG_Error("msgget failed:keyid[%d]-[%s]", keyId, g_strerror(errno));
        ret = RET_ERR;
    }
    else
    {
        ret = msgsnd(fd, (const void *)data, len - sizeof(long), IPC_NOWAIT);
        if (ret == -1)
        {
            LOG_Error("msgsnd failed:keyId[%d]-[%s]-len[%d]", keyId, g_strerror(errno), len);
            ret = RET_ERR;
        }
        else
        {
            //实际消息体长度=len-6
            ret = RET_OK;
        }
    }
    return ret;
}

//根据type查找共享内存中的keyid组 然后给这些keyid发送数据
//data: 字节流
//len: 字节流长度
int32_t SendMsgToAllQueue(SYSQueMsgType_e type, const void* data, const int32_t len)
{
    if(len >= SYSQUE_MSG_DATA_MAX_LEN)
    {
        LOG_Error("Send SysQueMsg To Queue Faild:len is too long. len:[%d]", len);
        return RET_ERR;
    }
    //根据type查找keyid组
    int32_t nodeIndex = GetTypeNodeIndexFromShmByType(type);
    ShmTypeNode_t *temp = NULL;
    if (nodeIndex == RET_ERR)//没有此keyid组数据
    {
        LOG_Warn("Type[0x%08x] msg no subscriber", type);
        return RET_ERR;
    }

    //组包消息
    int32_t sendLen = sizeof(SysQueueData_t) + len;//结构大小+序列化后的字节流大小
    SysQueueData_t *queueData = g_try_malloc0(sendLen);
    if (queueData == NULL)
    {
        LOG_Error("Send SysQueMsg To Queue Faild:malloc error[%s]", g_strerror(errno));
        return RET_ERR;
    }
    queueData->type = type;
    queueData->datalen = len;
    memmove(&queueData->data, data, len);

    //发送消息
    temp = &g_SYSQueTreeShm->data[nodeIndex];
    int32_t i = 0;
    for (i = 0; i < temp->len; i++)
    {
        int32_t keyId = temp->keyID[i];
        // 当接收方挂了后 msgsnd依然可以执行成功 用ipcs命令可以看到keyId队列里的数据会一直累加
        // 直到该队列的used-bytes累加到限制时(default max size of queue) msgsnd才会返回错误 进入到这里的错误处理
        if (SendDataToQueue(keyId, queueData, sendLen) != RET_OK)
        {
            /*check Thread alive,if thread dead,del the keyid*/
            LOG_Warn("Send Msg To Queue keyid[%d] failed,and check the thread is alive", keyId);
            int32_t ret = syscall(SYS_tkill, keyId, 0);
            LOG_Debug("thread %d status :[%d]", keyId, ret);
            if (ret == -1)
            {
                LOG_Warn("The Thread tid[%d] has dead,and delete the keyId from tree", keyId);
                msgctl(msgget(keyId, 0777), IPC_RMID, NULL);
                if (SHMSafeMutexLock(&g_SYSQueTreeShm->mutex) != RET_OK)
                {
                    return RET_ERR;
                }
                if (DeleteSysQueTreeShm(type, keyId))
                {
                    pthread_mutex_unlock(&g_SYSQueTreeShm->mutex);
                    break;
                }
                pthread_mutex_unlock(&g_SYSQueTreeShm->mutex);
                i--;
            }
        }
    }

    g_free(queueData);
    return RET_OK;
}

bool ipcIsFirstCall()
{
    pthread_mutex_lock(&ipcInitMutex);
    bool res = ipcFirstCall;
    pthread_mutex_unlock(&ipcInitMutex);
    return res;
}

int32_t StartQueueMsgRcvThread(const gchar *threadName)
{
    pthread_mutex_lock(&ipcInitMutex);
    if (!ipcFirstCall)
    {
        LOG_Info("System Queue already initialized!");
        pthread_mutex_unlock(&ipcInitMutex); 
        return RET_ERR;
    }
    ipcFirstCall = FALSE;
    pthread_mutex_unlock(&ipcInitMutex);
    InitCallbackTree();
    if (InitSysQueTree() != RET_OK)
    {
        LOG_Error("Init Tree Shm failed");
        return RET_ERR;
    }
    if (InitSysQueMsg() == RET_ERR)
    {
        LOG_Error("Init System Queue Msg failed");
        return RET_ERR;
    }
    if (thread_recv == NULL)
    {
        thread_recv = g_thread_new(threadName, SysQueMsgRecvThread, NULL);
    }
    return RET_OK;
}

int32_t StartQueueMsgRcv(void)
{
    pthread_mutex_lock(&ipcInitMutex);
    if (!ipcFirstCall)
    {
        LOG_Info("System Queue already initialized!");
        pthread_mutex_unlock(&ipcInitMutex); 
        return RET_ERR;
    }
    ipcFirstCall = FALSE;
    pthread_mutex_unlock(&ipcInitMutex);
    InitCallbackTree();
    if (InitSysQueTree() != RET_OK)
    {
        LOG_Error("Init Tree Shm failed");
        return RET_ERR;
    }
    if (InitSysQueMsg() == RET_ERR)
    {
        LOG_Error("Init System Queue Msg failed");
        return RET_ERR;
    }
    return RET_OK;
}

//rpc server
typedef struct
{
    RPCServerType_e type;
    RpcServerCallback callbackFuc;
    uint8_t poolSize;
    void* userdata;
} RpcServerThreadArgs_t;

int32_t GetRpcQueueMsg(const int32_t keyId, RpcQueueData_t *msgData, RPCServerType_e type, const int32_t timeout)
{
    if (msgData == NULL)
    {
        LOG_Warn("APP Recv Msg Buff Is NULL");
        return RET_ERR;
    }
    int32_t fd = msgget(keyId, 0644);
    if (fd == -1)
    {
        LOG_Error("Open App keyid[%d] RecvQueue Error:[%s]", keyId, g_strerror(errno));
        return RET_ERR;
    }
    int32_t ret = 0;
    /*recv msg from queue with block*/
    if (timeout <= 0)
    {
        ret = msgrcv(fd, msgData, SYSQUE_MSG_DATA_MAX_LEN + sizeof(RpcQueueData_t), (long)type, 0);
        if (ret == -1)
        {
            LOG_Warn("APP GetData key[%d],recv error[%d-%s]", keyId, errno, g_strerror(errno));
            return RET_ERR;
        }
    }
    else
    {
        /*recv msg from queue with timeout*/
        int32_t sleepCount = timeout / RECV_TIMEOUT_SLEEP_INTERVAL_MS;
        do
        {
            ret = msgrcv(fd, msgData, SYSQUE_MSG_DATA_MAX_LEN + sizeof(RpcQueueData_t), (long)type, IPC_NOWAIT);
            if (ret == -1)
            {
                if (errno == ENOMSG)
                {
                    ret = RET_ERR_NORMAL_TIMEOUT;
                    g_usleep(1000 * RECV_TIMEOUT_SLEEP_INTERVAL_MS);
                }
                else
                {
                    LOG_Warn("APP GetData key[%d],recv error[%d-%s]", keyId, errno, g_strerror(errno));
                    break;
                }
            }
            else
            {
                break;
            }
        } while ((sleepCount--) > 0);
    }
    return ret;
}

int32_t GetRpcQueueMsgNoWait(const int32_t keyId, RpcQueueData_t *msgData, RPCServerType_e type)
{
    if (msgData == NULL)
    {
        LOG_Warn("APP Recv Msg Buff Is NULL");
        return RET_ERR;
    }
    int32_t fd = msgget(keyId, 0644);
    if (fd == -1)
    {
        LOG_Error("Open App keyid[%d] RecvQueue Error:[%s]", keyId, g_strerror(errno));
        return RET_ERR;
    }
    int32_t ret = 0;
    /*recv msg from queue with block*/
    ret = msgrcv(fd, msgData, SYSQUE_MSG_DATA_MAX_LEN + sizeof(RpcQueueData_t), (long)type, IPC_NOWAIT);
    if (ret == -1)
    {
        LOG_Warn("GetRpcQueueMsgNoWait: key[%d],recv error[%d-%s]", keyId, errno, g_strerror(errno));
    }
    return ret;
}

void RpcTaskWorker(gpointer taskData, gpointer userdata)
{
    RpcTaskData *args = (RpcTaskData *)taskData;
    RpcQueueData_t *recvData = args->recvData;
    RpcServerCallback callbackFuc = args->callbackFuc;

    // flatcc进行verify_as_root操作时需要首字节地址与4字节对齐 所以传递给处理函数时需要对齐操作 详见https://github.com/dvidelabs/flatcc/issues/65
    void *buffer = NULL;
    if(recvData->datalen > 0)
    {
        buffer = aligned_alloc(4, align_up(4, recvData->datalen));
        if (!buffer)
        {
            LOG_Error("rpcrcv Msg Type[0x%08lx] but aligned_alloc failed!", (long)recvData->type);
            g_free(recvData);
            g_free(args);
            return;
        }
        memcpy(buffer, recvData->data, recvData->datalen);
    }

    // 先申请一个buffer 用于存放用户的response序列化后的消息
    void *respData = g_try_malloc0(SYSQUE_MSG_DATA_MAX_LEN);
    if (respData == NULL)
    {
        LOG_Error("malloc resp data error");
        g_free(recvData);
        g_free(args);
        aligned_free(buffer);
        return;
    }
    uint16_t respDataLen = 0;
    // 调用用户的回调函数将req消息传入 并得到用户的resp回复
    callbackFuc(recvData->subcmd, buffer, recvData->datalen, respData, &respDataLen, userdata);
    aligned_free(buffer);

    // 将用户的response消息组包后发出
    // 组包消息
    int32_t sendLen = sizeof(RpcQueueData_t) + respDataLen; // 结构大小+序列化后的字节流大小
    RpcQueueData_t *queueData = g_try_malloc0(sendLen);
    if (queueData == NULL)
    {
        LOG_Error("Send respQueMsg To Queue Faild:malloc error[%s]", g_strerror(errno));
        g_free(recvData);
        g_free(args);
        g_free(respData);
        return;
    }
    queueData->type = RPC_RESP_TYPE_BASE + recvData->clienttid; // 回复的消息统一在回复的基础type大小上加上请求端的tid作为回复的类型
    queueData->subcmd = recvData->subcmd;
    queueData->clienttid = recvData->clienttid;
    queueData->datalen = respDataLen;
    memmove(&queueData->data, respData, respDataLen);
    g_free(respData);
    /**
     * 如果发送请求的线程还活着才发送
     * 这个机制解决了一个这样的问题：当一个客户端线程启动后发送了一个RPC请求，同时Server端接收到这个请求，但此时该客户端线程还未死掉，
     * 所以可以通过Server端刚接收到请求时的syscall检查。但是在Server端调用用户的处理函数时，请求端的线程死掉了，
     * 这时候等用户Server端处理完再发送回复已经没有意义了，反而会向回复队列push一个垃圾数据
     */
    if (syscall(SYS_tkill, recvData->clienttid, 0) == 0)
    {
        SendDataToRpcQueue(RPC_RESP_QUEUE_KEY, queueData, sendLen);
    }
    g_free(queueData);
    g_free(recvData);
    g_free(args);
}

void* RpcServerThread(void* data)
{
    RpcServerThreadArgs_t *args = (RpcServerThreadArgs_t*)data;

    RPCServerType_e type = args->type;
    RpcServerCallback callbackFuc = args->callbackFuc;
    uint8_t poolSize = args->poolSize;
    void* userdata = args->userdata;

    if (poolSize > 1)
    {
        // 线程池 独占给RPC Server使用 不共享
        threadPool = g_thread_pool_new(RpcTaskWorker, userdata, poolSize, TRUE, NULL);
    }

    //定义接收buf
    RpcQueueData_t *recvData = g_try_malloc0(sizeof(RpcQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);
    if (recvData == NULL)
    {
        LOG_Error("malloc recv data error");
        g_free(recvData);
        return NULL;
    }
    //循环接收
    while(1)
    {
        memset(recvData, 0x00, sizeof(RpcQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);
        //只会收到该type的消息
        if (GetRpcQueueMsg(RPC_REQ_QUEUE_KEY, recvData, type, 0) == RET_ERR)
        {
            LOG_Error("rpcrcv Recv Msg Error");
            continue;
        }
        /**
         * 检查请求的消息来源的那个线程是不是还活着。如果请求的线程已死，说明这个请求消息没必要再处理了
         * 这个机制同时解决了一个这样的问题：假设一个进程重复启动N次，每次在RPC请求后异常退出，则会在请求队列中push N条请求消息。
         * 此时如果RPCServer端启动，会连续处理N条请求消息，这前面的N-1条请求消息都是无需传递给用户函数进行处理的（因为请求端已死），否则会浪费大量资源
         */
        if (syscall(SYS_tkill, recvData->clienttid, 0) != 0)
        {
            continue;
        }
        RPCServerType_e recvMsgType = (RPCServerType_e)recvData->type;
        if (callbackFuc == NULL)
        {
            LOG_Warn("rpcrcv Msg Type[0x%08lx] but no callback Func!", (long)recvMsgType);
            continue;
        }

        //将收到的消息复制给线程池中的处理线程进行处理
        int32_t dataLen_toPool = sizeof(RpcQueueData_t) + recvData->datalen;
        RpcQueueData_t *recvData_toPool = g_try_malloc0(dataLen_toPool);
        if (recvData_toPool == NULL)
        {
            LOG_Error("malloc data error");
            g_free(recvData_toPool);
            continue;
        }
        memcpy(recvData_toPool, recvData, dataLen_toPool);

        RpcTaskData *taskData = g_try_malloc0(sizeof(RpcTaskData));
        if (taskData == NULL)
        {
            LOG_Error("malloc data error");
            g_free(recvData_toPool);
            g_free(taskData);
            continue;
        }
        taskData->callbackFuc = callbackFuc;
        taskData->recvData = recvData_toPool;
        if (poolSize > 1)
        {
            GError *error = NULL;
            // 如果线程池的工作线程已经全部用完, g_thread_pool_push将任务放入队列中,等待线程池中的某个线程空闲并可用时再执行
            // 如果线程池的队列也已满 g_thread_pool_push 可能会失败并返回 FALSE
            // 线程池的任务队列大小由GLib库内部确定并管理
            if (!g_thread_pool_push(threadPool, taskData, &error))
            {
                LOG_Error("threadPool Error occurred: %s", error->message);
                g_error_free(error);
                g_free(recvData_toPool);
                g_free(taskData);
            }
        }
        else
        {
            RpcTaskWorker(taskData, userdata);
        }
    }
    g_thread_pool_free(threadPool, TRUE, TRUE);
    g_free(recvData);
    g_free(args);
    LOG_Error("%s:exit",__func__);
    return NULL;
}

int32_t CreatRpcQueue(void)
{
    if (msgget(RPC_REQ_QUEUE_KEY, IPC_CREAT | 0644) == -1)
    {
        LOG_Error("Create App keyid[%d] RecvQueue Error:[%s]", RPC_REQ_QUEUE_KEY, g_strerror(errno));
        return RET_ERR;
    }

    if (msgget(RPC_RESP_QUEUE_KEY, IPC_CREAT | 0644) == -1)
    {
        LOG_Error("Create App keyid[%d] RecvQueue Error:[%s]", RPC_RESP_QUEUE_KEY, g_strerror(errno));
        return RET_ERR;
    }
    LOG_Debug("%s:End, keyId[%d-%d]", __func__, RPC_REQ_QUEUE_KEY, RPC_RESP_QUEUE_KEY);
    return RET_OK;
}

int32_t RpcStartMsgServer(RPCServerType_e type, RpcServerCallback callbackFuc, uint8_t poolSize, void* userdata)
{
    // 1.创建初始化RPC专属消息队列
    if(CreatRpcQueue() == RET_ERR)
    {
        return RET_ERR;
    }
    // 2.启动线程当作rpc消息server
    RpcServerThreadArgs_t *args = g_try_malloc0(sizeof(RpcServerThreadArgs_t));
    if (args == NULL)
    {
        LOG_Error("malloc RpcServerThreadArgs_t error");
        g_free(args);
        return RET_ERR;
    }
    args->type = type;
    args->callbackFuc = callbackFuc;
    args->poolSize = poolSize > RPC_THREAD_POOL_SIZE_MAX ? RPC_THREAD_POOL_SIZE_MAX : poolSize;
    args->userdata = userdata;
    gchar str[30];
    sprintf(str, "rpcserver%ld", (long)type);
    g_thread_new(str, RpcServerThread, args);
    return RET_OK;
}

//rpc client get message
int32_t RpcGetMsgSync(RPCServerType_e type,
                   const uint16_t subcmd,
                   const int32_t timeOut,
                   const uint8_t *reqData,
                   const uint16_t reqLen,
                   uint8_t *respData,
                   uint16_t *respLen)
{
    LOG_Debug("rpc request: type:0x%08x,subcmd:%d,timeout:%d,reqLen:%d", type, subcmd, timeOut, reqLen);
    // 1.获取tid
    int32_t threadid = syscall(SYS_gettid);
    if (threadid == -1)
    {
        LOG_Error("Get threadid error:[%s]", g_strerror(errno));
        return RET_ERR;
    }
    // 2.准备接收回复消息的buffer
    RpcQueueData_t *recvData = g_try_malloc0(sizeof(RpcQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);
    if (recvData == NULL)
    {
        LOG_Error("malloc recv data error");
        return RET_ERR;
    }
    /**
     * 3.清除回复队列中的超时消息
     * 当一个RPC客户端发送了一个请求并且设置了一个超时时间，但是在超时后服务端才处理好，这时客户端的线程没有挂掉，
     * 所以服务端仍然会将回复消息push到回复队列，但是由于客户端的请求函数已超时退出，所以客户端收不到这条回复。
     * 此时如果客户端进行了第二次RPC请求，客户端会在发送完请求消息之后收到上一次的回复消息，造成消息不同步。
     * 所以在每次RPC请求时，都去清理一下这种类型的消息
     * 
     */
    pthread_mutex_lock(&rpcRequestMutex);
    int32_t count = 0;
    if (GetQueueMsgCount(RPC_RESP_QUEUE_KEY, &count))
    {
        LOG_Warn("RPC Resp Queue has %d timeout-replied messages. Checking now.", count);
        while (count > 0)
        {
            if ((RET_ERR == GetRpcQueueMsgNoWait(RPC_RESP_QUEUE_KEY, recvData, RPC_RESP_TYPE_BASE + threadid)) && (ENOMSG == errno))
            {
                /**
                 * @brief 假设该队列总共存在n条这种客户端超时后服务端再回复的消息，在进行了m次循环判断后，该队列中已经获取不到这个类型的消息，说明剩下的n-m条消息也没有这个类型的消息了（由msgrcv的特性决定）
                 * 这时候就不需要继续判断剩下的n-m条消息，以免出现每个rpc过程都循环判断n次，占用大量资源
                 * 所以这里可以直接退出循环
                 * 
                 */
                memset(recvData, 0x00, sizeof(RpcQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);
                LOG_Warn("No more messages of desired type in RPC Resp Queue. %d timeout-replied messages remaining.", count);
                break;
            }
            memset(recvData, 0x00, sizeof(RpcQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);
            count--;
        }
    }
    /**
     * 4.清除请求队列中的超时消息
     * 当一个RPC客户端发送了一个请求并且设置了一个超时时间并且超时，这时候客户端线程执行到再次请求处时，
     * 请求队列中还会存在客户端线程的上次请求消息，这时候这条请求消息已经没有意义了（因为马上就要执行下一次请求），
     * 如果不处理的话，经过N次循环并且超时，请求队列中就会多出N条垃圾消息。
     * 所以需要每次请求之前清理队列中上一次的请求
     * 由于没有办法判断一个type类型的消息是自己还是另一个app线程请求的，所以先逐个读出来，然后判断消息中的tid，如果和自身tid不相等就push回去
     * 
     */
    count = 0;
    if (GetQueueMsgCount(RPC_REQ_QUEUE_KEY, &count))
    {
        // LOG_Debug("GetQueueMsgCount RPC_REQ_QUEUE_KEY count:%d",count);
        int32_t checkCount = 0;
        uint16_t startTid = 0;
        while (count > 0)
        {
            if ((RET_ERR != GetRpcQueueMsgNoWait(RPC_REQ_QUEUE_KEY, recvData, type)) &&
                (recvData->clienttid != threadid))
            {
                // 判断发这条消息的线程是否死掉了 如果线程死掉了那么这个消息也就没用了 不需要再push回去
                if (syscall(SYS_tkill, recvData->clienttid, 0) != 0)
                {
                    // LOG_Debug("clear the tid:%u",recvData->clienttid);
                    memset(recvData, 0x00, sizeof(RpcQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);
                    count--;
                    continue;
                }
                // 对满足：1.请求线程未死 2.请求类型为type 3.不属于自己本身的请求 这三个条件的消息进行计数
                checkCount++;
                // 如果接收到的消息的tid等于第一次处理的tid 说明已经处理完了请求队列中全部的该type类型的消息 并且又一次接收到了第一次处理时push回去的消息 就可以退出循环不用判断了
                // 如果没有这个逻辑 会重复判断0-count次 浪费资源
                if (startTid == recvData->clienttid)
                {
                    // LOG_Debug("GetQueueMsgCount RPC_REQ_QUEUE_KEY recvData->clienttid and type(%u)(0x%08lx) != threadid and type(%d)(0x%08x), and push back",
                    //           recvData->clienttid, recvData->type, threadid, type);
                    SendDataToRpcQueue(RPC_REQ_QUEUE_KEY, recvData, sizeof(RpcQueueData_t) + recvData->datalen);
                    memset(recvData, 0x00, sizeof(RpcQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);
                    break;
                }
                // 记录第一次处理的tid
                if (checkCount == 1)
                {
                    startTid = recvData->clienttid;
                }

                // LOG_Debug("GetQueueMsgCount RPC_REQ_QUEUE_KEY recvData->clienttid and type(%u)(0x%08lx) != threadid and type(%d)(0x%08x), and push back",
                //           recvData->clienttid, recvData->type, threadid, type);
                SendDataToRpcQueue(RPC_REQ_QUEUE_KEY, recvData, sizeof(RpcQueueData_t) + recvData->datalen);
            }
            memset(recvData, 0x00, sizeof(RpcQueueData_t) + SYSQUE_MSG_DATA_MAX_LEN);
            count--;
        }
    }
    pthread_mutex_unlock(&rpcRequestMutex);

    // 5.发送请求消息
    int32_t sendLen = sizeof(RpcQueueData_t) + reqLen; // 结构大小+序列化后的字节流大小
    RpcQueueData_t *queueData = g_try_malloc0(sendLen);
    if (queueData == NULL)
    {
        LOG_Error("Send reqQueMsg To Queue Faild:malloc error[%s]", g_strerror(errno));
        g_free(recvData);
        return RET_ERR;
    }
    queueData->type = type;
    queueData->subcmd = subcmd;
    queueData->clienttid = threadid;
    queueData->datalen = reqLen;
    memmove(&queueData->data, reqData, reqLen);
    SendDataToRpcQueue(RPC_REQ_QUEUE_KEY, queueData, sendLen);
    g_free(queueData);
    // 6.开启消息接收 等待server端返回消息
    int32_t ret = GetRpcQueueMsg(RPC_RESP_QUEUE_KEY, recvData, RPC_RESP_TYPE_BASE + threadid, timeOut);
    if (ret <= RET_ERR)
    {
        LOG_Error("rpc Recv Msg Error:%s", ret == RET_ERR_NORMAL_TIMEOUT ? "timeout" : "error");
        g_free(recvData);
        return ret;
    }
    if (respData == NULL)
    {
        LOG_Error("respData is null!!");
        g_free(recvData);
        return RET_ERR;
    }
    memcpy(respData, recvData->data, recvData->datalen);
    *respLen = recvData->datalen;
    g_free(recvData);
    return RET_OK;
}
