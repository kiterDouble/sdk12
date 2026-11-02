#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
// #include "sr_ipc.h"
// #include "sr_sdk.h"
// #include "https.h"
#include "sr_log.h"
// #include "time_status_api.h"
// #include "time_status.h"
// #include "https_interface.h"
// #include "system_interface.h"
// #include "thread_interface.h"
#include "sr_engine.h"
// #include "file_interface.h"
// #include "certificate_interface.h"
#include <openssl/engine.h>
#include <openssl/ssl.h>
#include <openssl/engine.h>
#include "pki_sdk.h"

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#define SHM_PATH    "/pki_engine_robust_mutex_shm"
#define SHM_SIZE    4096

// 共享内存中的数据结构
typedef struct {
    pthread_mutex_t mutex;
    char padding[4080]; // 占位，确保大小足够
} pki_shared_data_t;

static int shm_fd = -1;
static pki_shared_data_t *shared = NULL;
static int get_shm_fd_cnt = 0;

// 用于保护初始化的静态互斥锁
static pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;

static int engine_lock(void) {
    if ((NULL == shared) || (MAP_FAILED == shared)) {
        LOG_Error("Shared memory not initialized");
        return -1;
    }

    int ret = pthread_mutex_lock(&shared->mutex);
    if (ret == 0) {
        LOG_Debug("pthread_mutex_lock success");
        return 0;  // 正常获取
    }

    LOG_Debug("pthread_mutex_lock failed: %s", strerror(ret));

    if (ret == EOWNERDEAD) {
        LOG_Debug("Previous owner died, recovering mutex...");
        int recover = pthread_mutex_consistent(&shared->mutex);
        if (recover != 0) {
            LOG_Debug("Failed to make mutex consistent: %s", strerror(recover));
            pthread_mutex_unlock(&shared->mutex);  // 尝试解锁
            return -1;
        }

        return 0;
    }

    return -1;
}

static int engine_unlock(void) {
    if (!shared) {
        return -1;
    }

    int ret = pthread_mutex_unlock(&shared->mutex);
    if (ret == 0) {
        LOG_Debug("pthread_mutex_unlock success");
        return 0;  // 正常释放
    }
    return -1;
}

static int engine_lock_init(void) {

    int lock_ret;
    LOG_Debug("engine_lock_init begin");
    // 加线程锁
    pthread_mutex_lock(&init_mutex);

    LOG_Debug("engine_lock_init pthread_mutex locked");

    if (shm_fd == -1)
    {
        shm_fd = shm_open(SHM_PATH, O_RDWR | O_CREAT, 0666);
        if (shm_fd == -1) {
            LOG_Debug("shm_open failed");
            pthread_mutex_unlock(&init_mutex);
            return -1;
        }
    }

    // 获取共享内存大小
    struct stat sb;
    if (fstat(shm_fd, &sb) == -1) {
        LOG_Debug("fstat failed: %s\n", strerror(errno));
        goto fail;
    }

    if (sb.st_size == 0) {
        // 首次创建，初始化大小和 mutex
        if (ftruncate(shm_fd, SHM_SIZE) == -1) {
            LOG_Debug("ftruncate failed: %s\n", strerror(errno));
            goto fail;
        }

        // 首次映射共享内存
        shared = (pki_shared_data_t*)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if ((NULL == shared) || (shared == MAP_FAILED)) 
        {
            LOG_Debug("mmap failed: %s\n", strerror(errno));
            goto fail;
        }

        // 初始化 robust mutex
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);  // 关键：健壮性

        if (pthread_mutex_init(&shared->mutex, &attr) != 0) {
            LOG_Debug("pthread_mutex_init failed");
            munmap(shared, SHM_SIZE);
            shared = NULL;
            pthread_mutexattr_destroy(&attr);
            goto fail;
        }
        pthread_mutexattr_destroy(&attr);

    } 
        
    if ((NULL == shared) || (shared == MAP_FAILED)) 
    {
        shared = (pki_shared_data_t*)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (shared == MAP_FAILED) 
        {
            get_shm_fd_cnt++;
            LOG_Debug("mmap failed: %s\n", strerror(errno));
            goto fail;
        }
        else
        {
            get_shm_fd_cnt = 0;
        }
    }

    lock_ret = engine_lock();
    LOG_Debug("engine_lock  = %d", lock_ret);
    if(0 != lock_ret)
    {
        goto fail;
    }

    LOG_Debug("engine_lock_init end");

    pthread_mutex_unlock(&init_mutex);

    return 0;

fail:
    if (shm_fd != -1) 
    {   
        close(shm_fd);
        shm_fd = -1;
    }
    if (shared && shared != MAP_FAILED) {
        munmap(shared, SHM_SIZE);
        shared = NULL;
    }
    if (get_shm_fd_cnt >= 20) {
        get_shm_fd_cnt = 0;
        shm_unlink(SHM_PATH); // 清理失败创建
    }
    LOG_Debug("engine_lock_init failed end");
    pthread_mutex_unlock(&init_mutex);
    return -1;
}

static void engine_lock_cleanup(void) {
    LOG_Debug("engine_lock_cleanup begin");
    int ret = engine_unlock();
    if (ret != 0) {
        // 注意：不要 destroy robust mutex，除非你是创建者且确定无其他进程使用
        // 一般只 unmap
        if (shared && shared != MAP_FAILED) 
        {
            munmap(shared, SHM_SIZE);
            shared = NULL;
        }
          
        if (shm_fd != -1) {
            close(shm_fd);
            shm_fd = -1;
        }
    }

    LOG_Debug("engine_lock_cleanup end,ret= %d", ret);
}


int PkiCertificateAndKeyFullyFunctionalFunctions()
{
    int ret = 0;
    int fd = open("/oemdata/cert/check.txt", O_CREAT, 0644); 
    if (fd == -1) {
        LOG_Error("Error creating or opening file");
        return -1; 
    }

    close(fd); 
    for(int i =0;i<25;i++)
    {
        if(access("/oemdata/cert/check.txt",F_OK) != 0)
        {
            return 0;
        }else{
            sleep(1);
        }
        LOG_Debug("PkiCertificateAndKeyFullyFunctionalFunctions count:%d",i);
    }
    return -1;
}

int CertificateInspection()
{
    int ret = 0;
    LOG_INIT("pki_sdk");
    engine_lock_init();
    LOG_Debug("CertificateInspection start");
    ret = PkiCertificateAndKeyFullyFunctionalFunctions();
    if(ret < 0)
    {
        LOG_Error("PkiCertificateAndKeyFullyFunctionalFunctions fail");
        engine_lock_cleanup();
        return -1;
    }
    LOG_Debug("CertificateInspection end");
    engine_lock_cleanup();
    return 0;
}

void SiRunPKIEngineInit()
{
    int ret = 0;
    ENGINE *engine = NULL;
    LOG_Debug("SiRunPKIEngineInit start ");
    engine = ENGINE_by_id("engine-sirun");
    if (!engine)
    {
        LOG_Debug("ENGINE_by_id null");
        ret = ENGINE_Init_sirun();
        if(ret != 0)
        {
            LOG_Error("ENGINE Init sirun fail");
            return ;
        }
    }
    // else{
    //     SiRunPKIEngineDeInit();
    //     LOG_Debug("ENGINE_by_id NULL");
    //     ret = ENGINE_Init_sirun();
    //     if(ret != 0)
    //     {
    //         LOG_Error("ENGINE Init sirun fail");
    //         return ;
    //     }
    // }

    return ;
}

void SiRunPKIEngineDeInit()
{
    ENGINE *engine = NULL;
    engine = ENGINE_by_id("engine-sirun");
    if (engine)
    {
        LOG_Debug("engine true");
        ENGINE_DeInit_sirun();
    }
}

EVP_PKEY *SiRunPKIEvpPkeyStruct()
{
    EVP_PKEY *ssl_pubKey = NULL;
    if(NULL == ssl_pubKey)
    {
        ENGINE *engine = NULL;
        engine = ENGINE_by_id("engine-sirun");
        if (!engine)
        {
            LOG_Error("ENGINE_by_id failed");
            return NULL;
        }
        
        ssl_pubKey = ENGINE_load_private_key(engine, "private key", NULL, NULL);
    
        if (!ssl_pubKey)
        {
            LOG_Error("Failed to load private key");
            ssl_pubKey = NULL;
            return NULL;
        }
    }

    return ssl_pubKey;
}

int SiRunPKISSLCTXUsePrivateKey(SSL_CTX *ctx)
{
    engine_lock_init();
    LOG_Debug("SiRunPKISSLCTXUsePrivateKey start... ");
    display_engine_list();
    EVP_PKEY *s_pubKey = NULL;
    if(NULL == s_pubKey)
    {
        display_engine_list();
        ENGINE *engine = NULL;
        engine = ENGINE_by_id("engine-sirun");
        if (!engine)
        {
            LOG_Error("ENGINE_by_id failed");
            engine_lock_cleanup();
            return -1;
        }

        s_pubKey = ENGINE_load_private_key(engine, "cert request", NULL, NULL);
    
        if (!s_pubKey)
        {
            LOG_Error("Failed to load private key");
            s_pubKey = NULL;
            engine_lock_cleanup();
            return -1;
        }
    

        if (!SSL_CTX_use_PrivateKey(ctx, s_pubKey))
        {
            EVP_PKEY_free(s_pubKey);   // 使用失败，释放
            LOG_Error("Failed to use private key: %s",
            ERR_error_string(ERR_get_error(), NULL));
            s_pubKey = NULL;
            engine_lock_cleanup();
            return -1;
        }

        if(!SSL_CTX_set1_client_sigalgs_list(ctx,"RSA+SHA256"))
        {
            LOG_Error("SSL_CTX_set1_client_sigalgs_list fail");
            EVP_PKEY_free(s_pubKey);   // 使用失败，释放
            s_pubKey = NULL;
            engine_lock_cleanup();
            return -1;
        }

        EVP_PKEY_free(s_pubKey);
    }
    engine_lock_cleanup();
    LOG_Debug("SiRunPKISSLCTXUsePrivateKey  end");
    return 0;

}


/**
 * @brief RSA私钥解密
 * @param ciphertext 密文数据
 * @param pt_len 明文长度
 * @param plaintext 明文数据
 * @param pt_len 明文长度
 * @param padding 填充模式 
 * @return 密文长度（失败返回-1）
 */

int SiRunPKIRSAPrivateDec(const unsigned char *ciphertext,int ct_len, unsigned char *plaintext,int *pt_len,int padding)
{
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = NULL;
    size_t len;
    int ret = 0;

    if((ciphertext == NULL) || (plaintext == NULL))
    {
        LOG_Error( "ciphertext NULL or plaintext NULL");
        return -1;
    }
    engine_lock_init();
    display_engine_list();
    LOG_Debug("SiRunPKIRSAPrivateDec start ct_len = %d...",ct_len);
    
    pkey = SiRunPKIEvpPkeyStruct();
    if (!pkey) {
        LOG_Error( "无法加载私钥\n");
        engine_lock_cleanup();
        return -1;
    }

    // 创建解密上下文
    ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx || EVP_PKEY_decrypt_init(ctx) <= 0) {
        LOG_Error("上下文初始化失败\n");
        EVP_PKEY_free(pkey);
        engine_lock_cleanup();
        return -1;
    }

    // 设置填充模式
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, padding) <= 0) {
        LOG_Error("设置填充模式失败\n");
        EVP_PKEY_free(pkey);
        EVP_PKEY_CTX_free(ctx);
        engine_lock_cleanup();
        return -1;
    }

    // 执行解密（首次调用获取缓冲区大小）
    if (EVP_PKEY_decrypt(ctx, NULL, &len, ciphertext, ct_len) <= 0) {
        LOG_Error("解密长度获取失败\n");
        EVP_PKEY_free(pkey);
        EVP_PKEY_CTX_free(ctx);
        engine_lock_cleanup();
        return -1;
    }

    // 实际解密
    if (EVP_PKEY_decrypt(ctx, plaintext, &len, ciphertext, ct_len) <= 0) {
        LOG_Error("解密失败\n");
        ERR_print_errors_fp(stderr);
        EVP_PKEY_free(pkey);
        EVP_PKEY_CTX_free(ctx);
        engine_lock_cleanup();
        return -1;
    }
    *pt_len = len;
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    engine_lock_cleanup();
    return (int)0;
}




// int rsa_encrypt(const char *pubkey_path, 
//                const unsigned char *plaintext, int pt_len,
//                unsigned char *ciphertext, 
//                int padding) {
//     RSA *rsa = NULL;
//     FILE *fp = fopen(pubkey_path, "r");
//     if (!fp) {
//         perror("打开公钥文件失败");
//         return -1;
//     }

//     // 从PEM文件读取公钥
//     rsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL);
//     fclose(fp);
//     if (!rsa) {
//         fprintf(stderr, "解析公钥失败\n");
//         ERR_print_errors_fp(stderr);
//         return -1;
//     }

//     // 检查明文长度
//     int max_len = RSA_size(rsa) - (padding == RSA_PKCS1_PADDING ? 11 : 42);
//     if (pt_len > max_len) {
//         fprintf(stderr, "明文过长（最大允许%d字节）\n", max_len);
//         RSA_free(rsa);
//         return -1;
//     }

//     // 执行加密
//     int ciphertext_len = RSA_public_encrypt(pt_len, plaintext, ciphertext, rsa, padding);
//     if (ciphertext_len == -1) {
//         fprintf(stderr, "加密失败\n");
//         ERR_print_errors_fp(stderr);
//     }

//     RSA_free(rsa);
//     return ciphertext_len;
// }

/**
 * @brief RSA公钥加密
 * @param plaintext 明文数据
 * @param pt_len 明文长度
 * @param ciphertext 密文输出缓冲区
 * @param pt_len 密文长度
 * @param padding 填充模式  暂时仅支持RSA_PKCS1_PADDING
 * @return 成功返回0/失败返回-1
 */
int SiRunPKIRSAPublicEnc(const unsigned char *plaintext, int pt_len,unsigned char *ciphertext,int *ct_len,int padding) 
{
    // 1. 加载公钥
    int ret = 0;
    EVP_PKEY *pkey;
    engine_lock_init();
    display_engine_list();
    LOG_Debug("SiRunPKIRSAPublicEnc start ...");
    
    pkey = SiRunPKIEvpPkeyStruct();
    if (!pkey) {
        LOG_Error("无法加载私钥\n");
        engine_lock_cleanup();
        return -1;
    }

    // 2. 创建加密上下文
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, NULL); // 关键：传入引擎
    if (!ctx || EVP_PKEY_encrypt_init(ctx) <= 0) {
        EVP_PKEY_free(pkey);
        engine_lock_cleanup();
        return -1;
    }

    // 3. 设置PKCS#1 v1.5填充
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, padding) <= 0) {
        EVP_PKEY_free(pkey);
        EVP_PKEY_CTX_free(ctx);
        engine_lock_cleanup();
        return -1;
    }

    // 4. 获取输出缓冲区大小
    if (EVP_PKEY_encrypt(ctx, NULL, (size_t*)ct_len, plaintext, pt_len) <= 0) {
        EVP_PKEY_free(pkey);
        EVP_PKEY_CTX_free(ctx);
        engine_lock_cleanup();
        return -1;
    }

    // 5. 执行加密
    if (EVP_PKEY_encrypt(ctx, ciphertext, (size_t*)ct_len, plaintext, pt_len) <= 0) {

        EVP_PKEY_free(pkey);
        EVP_PKEY_CTX_free(ctx);
        engine_lock_cleanup();
        return -1;
    }

    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    LOG_Debug("SiRunPKIRSAPublicEnc end ...");
    engine_lock_cleanup();
    return (int)0;
}
