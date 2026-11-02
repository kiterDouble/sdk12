#include "sr_engine.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sr_log.h"
#include "sr_hal_hsm.h"
#include <openssl/bn.h>  
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/core_names.h>
#include "pm_server_api.h"

static const char *eng_id = "engine-sirun";
static const char *eng_name = "engine for OpenSSL(sirun)";
static const char *ca_eng_id = "engine-sirun-ca";
static const char *ca_eng_name = "engine for OpenSSL(sirun-ca)";


#define SHM_PATH    "/engine_robust_mutex_shm"
#define SHM_SIZE    4096
static int flag;

// 共享内存中的数据结构
typedef struct {
    pthread_mutex_t mutex;
    char padding[4080]; // 占位，确保大小足够
} shared_data_t;

static int shm_fd = -1;
static shared_data_t *shared = NULL;
static int get_shm_fd_cnt = 0;

// 用于保护初始化的静态互斥锁
static pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;

// 检测锁状态的函数
int GetEngineLockStatus() {
    int fd = -1;
    shared_data_t *sharedmem = NULL;
    
    // 1. 打开共享内存
    fd = shm_open(SHM_PATH, O_RDWR, 0666);
    if (fd == -1) {
        // 共享内存不存在，说明没有进程持有锁
        if (errno == ENOENT) {
            return 0; // 未加锁
        }
        LOG_Error("shm_open failed");
        return -1; // 错误
    }
    
    // 2. 映射共享内存
    sharedmem = (shared_data_t*)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (sharedmem == MAP_FAILED) {
        LOG_Error("mmap failed");
        close(fd);
        return -1;
    }
    
    // 3. 使用非阻塞方式尝试获取锁
    int result = pthread_mutex_trylock(&sharedmem->mutex);
    switch (result) {
        case 0:
            // 成功获取锁，说明之前是未锁定状态
            pthread_mutex_unlock(&sharedmem->mutex); // 立即释放
            break;
            
        case EBUSY:
            // 锁已被其他进程/线程占用
            pthread_mutex_unlock(&sharedmem->mutex); // 立即释放
            break;
            
        case EOWNERDEAD:
            // 前一个持有者死亡，锁处于不一致状态
            LOG_Debug("Lock inconsistent: previous owner died\n");
            
            // 恢复锁的一致性
            if (pthread_mutex_consistent(&sharedmem->mutex) == 0) {
                pthread_mutex_unlock(&sharedmem->mutex); // 释放锁
            }
            break;

        case EINVAL:
            // 互斥锁未初始化或已销毁
            LOG_Debug("Mutex is not properly initialized\n");
            break;
    }
    
    // 4. 清理资源
    munmap(sharedmem, SHM_SIZE);
    close(fd);
    return 0;
}



int engine_lock(void) {
    if ((NULL == shared) || (MAP_FAILED == shared)) {
        LOG_Debug("Shared memory not initialized");
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

int engine_unlock(void) {
    if (!shared) {
        LOG_Debug("shared NULL");
        return -1;
    }

    int ret = pthread_mutex_unlock(&shared->mutex);
    if (ret == 0) {
        LOG_Debug("pthread_mutex_unlock success");
        LOG_Debug("pthread_mutex_unlock success %d",ret);
        return 0;  // 正常释放
    }
    LOG_Debug("pthread_mutex_unlock  %d",ret);
    return -1;
}

int engine_lock_init(void) {

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
        shared = (shared_data_t*)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
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
        shared = (shared_data_t*)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
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

    int lock_ret = engine_lock();
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

void engine_lock_cleanup(void) 
{
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

/*
将hash编码为ASN.1 DER数据
hash[in] : 源数据
hashlen[in] : 源数据长度
der[out] : 编码后的数据，传入NULL时，可以获得编码后的数据长度
derlen[out] : 编码后的数据长度
type[in] : 声明哈希的算法ID，NID_sha256 NID_sha256WithRSAEncryption type不一样会导致生成的der头不一样，比如:
NID_sha256：der: 30 31 30 0d 06 09 60 86 48 01 65 03 04 02 01 05 00 04 20 + 32byte sha256
NID_sha256WithRSAEncryption：der: 30 31 30 0d 06 09 2a 86 48 86 f7 0d 01 01 0b 05 00 04 20 + 32byte sha256
在生成csr文件的过程中，需要使用NID_sha256WithRSAEncryption算法，因为openssl req -in test.csr -text解析出来的Signature Algorithm: sha256WithRSAEncryption
*/
int RSA_sig_to_der(const unsigned char *hash, unsigned int hashlen, unsigned char *der, unsigned int *derlen, int type)
{
    int ret = -1;
    X509_SIG *xsig;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
#else
	X509_ALGOR *algor;
	ASN1_OCTET_STRING *pdigest;
#endif
    do
    {
        // 不同于SM2算法的签名转换，此定义已在OpenSSL中声明
        // SM2的签名数据需要自己做声明
        xsig = X509_SIG_new();
        if (!xsig)
        {
            break;
        }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
        if (!X509_ALGOR_set0(xsig->algor, OBJ_nid2obj(type), V_ASN1_NULL, NULL))
        {
            break;
        }
        if (!ASN1_OCTET_STRING_set(xsig->digest, hash, hashlen))
        {
            break;
        }
#else
        // 获得algor和pdigest在xsig中的地址，从而设置xsig的值
        X509_SIG_getm(xsig, &algor, &pdigest);
        if (!X509_ALGOR_set0(algor, OBJ_nid2obj(type), V_ASN1_NULL, NULL))
        {
            break;
        }
        if (!ASN1_OCTET_STRING_set(pdigest, hash, hashlen))
        {
            break;
        }
#endif
        if (der)
        {
            *derlen = i2d_X509_SIG(xsig, &der);
        }
        else
        {
            *derlen = i2d_X509_SIG(xsig, NULL);
        }
        ret = 0;
    } while (0);

    if (xsig)
    {
        X509_SIG_free(xsig);
    }
    return ret;
}

/*
将ASN1编码数据转换为hash
pbDer[in] : ASN1编码数据
uiDerLen[in] : ASN1编码数据长度
hash[out] : 哈希数据
hashlen[out] : 哈希数据长度
*/
int RSA_sig_to_hash(const unsigned char *pbDer, unsigned int uiDerLen, unsigned char *hash, unsigned int *hashlen)
{
    int ret = -1;
    X509_SIG *xsig = NULL;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
#else
    const ASN1_OCTET_STRING *doct = NULL;
#endif
    const unsigned char *data = NULL;
    int datalen = 0;

    do
    {
        xsig = d2i_X509_SIG(NULL, &pbDer, uiDerLen);
        if (!xsig)
        {
            break;
        }
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        datalen = ASN1_STRING_length(xsig->digest);
        if (datalen <= 0)
        {
            break;
        }
        data = ASN1_STRING_data(xsig->digest);
        if (!data)
        {
            break;
        }
#else
        // 这里没有对哈希算法进行解析，如需解析，把NULL变为变量再解析
        X509_SIG_get0(xsig, NULL, &doct);

        datalen = ASN1_STRING_length(doct);
        if (datalen <= 0)
        {
            break;
        }
        data = ASN1_STRING_get0_data(doct);
        if (!data)
        {
            break;
        }
#endif

        memcpy(hash, data, datalen);
        *hashlen = datalen;
        ret = 0;
    } while (0);

    if (xsig)
    {
        X509_SIG_free(xsig);
    }
    return ret;
}

int RSA_signature(const unsigned char *m, unsigned int m_length,
                  unsigned char *sigret, unsigned int *siglen, RSA *rsa)
{
    int ret = 0;
    LOG_Debug("RSA_signature m_length: %d", m_length);
    unsigned char hash[SHA256_DIGEST_LENGTH] = {0};
    
    // 先用openssl接口算sha256
    SHA256(m, m_length, hash);
    // 签名，会将原始数据传给 my_rsa_sign 进行签名
    ret = RSA_sign(0, hash, SHA256_DIGEST_LENGTH, sigret, siglen, rsa);
   
    return  ret;// my_rsa_sign
}

int RSA_verifySignature(const unsigned char *m, unsigned int m_length,
                        const unsigned char *sigbuf, unsigned int siglen, RSA *rsa)
{
    int ret = 0;
    unsigned char hash[SHA256_DIGEST_LENGTH] = {0};
    // 先用openssl接口算原始消息sha256
   
    SHA256(m, m_length, hash);
    // 验签 会调用 my_rsa_verify 进行验签
    ret = RSA_verify(0, hash, SHA256_DIGEST_LENGTH, sigbuf, siglen, rsa); // my_rsa_verify
    
    return ret;
}

static int myengine_init(ENGINE *e)
{
    int ret = 0;
    LOG_Debug("myengine_init");
    engine_lock_init();
    ret = HalHsmInit();
    engine_lock_cleanup();
    return ret;
}
static int myengine_finish(ENGINE *e)
{
    LOG_Debug("myengine_finish");
    HalHsmDeinit();
    return 1;
}
static int myengine_destroy(ENGINE *e)
{
    LOG_Debug("myengine_destroy");
    return 1;
}

static int myrand_status(void)
{
    LOG_Debug("myrand_status");
    return 1;
}

static int myrand_bytes(unsigned char *buf, int num)
{
    int ret = 0;
    LOG_Debug("myrand_bytes number:%d", num);
    //engine_lock_init();
    ret = HalHsmRand(buf, num);
    //engine_lock_cleanup();
    return ret;
}

static const RAND_METHOD rand_method =
    {
        NULL,
        myrand_bytes,
        NULL,
        NULL,
        NULL,
        myrand_status};

#if OPENSSL_VERSION_NUMBER < 0x10100000L
static int my_engine_sha256_update(EVP_MD_CTX *ctx, const void *data, size_t count);
static int my_engine_sha256_init(EVP_MD_CTX *ctx)
{
    ctx->update = &my_engine_sha256_update;

    LOG_Debug("initialized! SHA256");
    return 1;
}

static int my_engine_sha256_update(EVP_MD_CTX *ctx, const void *data, size_t count)
{
    LOG_Debug("SHA256 update ! size:%d", count);
    return HalHsmSHA256Update(ctx, data, count);
}

static int my_engine_sha256_final(EVP_MD_CTX *ctx, unsigned char *md)
{
    LOG_Debug("SHA256 final size of EVP_MD: %d", sizeof(EVP_MD));
    memcpy(md, ctx->md_data, ctx->digest->md_size);
    return 1;
}

int my_engine_sha256_copy(EVP_MD_CTX *to, const EVP_MD_CTX *from)
{
    LOG_Debug("Copy SHA256");
    if (to->md_data && from->md_data)
    {
        memcpy(to->md_data, from->md_data, sizeof(*from->md_data));
    }
    return 1;
}

static int my_engine_sha256_clean(EVP_MD_CTX *ctx)
{
    LOG_Debug("SHA256 cleanup");
    if (ctx->md_data)
        memset(ctx->md_data, 0, ctx->digest->md_size);
    return 1;
}

// 摘要算法 digest
// typedef struct env_md_st EVP_MD
static EVP_MD my_sha256_engine = {
    NID_sha256,
    0,
    32,
    EVP_MD_FLAG_DIGALGID_ABSENT,
    my_engine_sha256_init,
    my_engine_sha256_update,
    my_engine_sha256_final,
    my_engine_sha256_copy,
    my_engine_sha256_clean,
    NULL,
    NULL,
    {0},
    64,
    32,
    NULL};
#else
#endif

static int my_rsa_pub_enc(int flen, const unsigned char *from,
                          unsigned char *to, RSA *rsa, int padding)
{
    LOG_Debug("my_rsa_pub_enc start-------------------------------------------------------------------------------------------------------------------");
    int rsabits = 2048;
    engine_lock_init();
    // 需要传入的rsa里面有公钥n和e，用来判断bit长度，判断用RSA1024、1280、2048哪种方式进行加密
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (rsa != NULL && rsa->n != NULL)
    {
        rsabits = RSA_size(rsa) * 8;
    }
#else
    if (rsa != NULL)
    {
        const BIGNUM *n = NULL;
        RSA_get0_key(rsa, &n, NULL, NULL);
        if (n != NULL)
        {
            rsabits = RSA_size(rsa) * 8;
        }
    }
#endif
    // pkcs随机数 先生成256个非0随机数备用
    unsigned char rand_buf[256] = {0};
    int err = RAND_bytes(rand_buf, 256);
    LOG_Debug("RAND_bytes ret : %x", err);
    for (int i = 0; i < 256; i++)
    {
        // 保持数据没有0
        if (rand_buf[i] == 0x00)
            rand_buf[i] = 0xff;
    }

    int ret = HalHsmRSAPubEnc(PUB_FID_01,flen, rsabits, rand_buf, from, to);
    engine_lock_cleanup();
    LOG_Debug("my_rsa_pub_enc end-------------------------------------------------------------------------------------------------------------------");
    return ret;
}
static int my_rsa_pub_dec(int flen, const unsigned char *from,
                          unsigned char *to, RSA *rsa, int padding)
{
    // 1. 参数校验
    if (flen <= 0 || from == NULL || to == NULL || rsa == NULL) {
        return -1;
    }

    engine_lock_init();
    int rsa_size = RSA_size(rsa);
    if (flen != rsa_size) {
        engine_lock_cleanup();
        return -1;  // 输入长度必须等于 RSA 模数长度
    }

    // 2. 获取公钥参数 (n, e)
    const BIGNUM *n = NULL;
    const BIGNUM *e = NULL;
    const BIGNUM *d = NULL;  // 虽然公钥解密不用 d，但可以用来判断是否为私钥
    RSA_get0_key(rsa, &n, &e, &d);

    if (n == NULL || e == NULL) {
        engine_lock_cleanup();
        return -1;
    }

    // 3. 创建 BIGNUM 上下文（用于临时变量）
    BN_CTX *ctx = BN_CTX_new();
    if (ctx == NULL) {
        engine_lock_cleanup();
        return -1;
    }

    // 4. 将密文 from 转为 BIGNUM (c)
    BIGNUM *c = BN_bin2bn(from, flen, NULL);
    if (c == NULL) {
        BN_CTX_free(ctx);
        engine_lock_cleanup();
        return -1;
    }

    // 5. 分配明文 BIGNUM (m)
    BIGNUM *m = BN_new();
    if (m == NULL) {
        BN_free(c);
        BN_CTX_free(ctx);
        engine_lock_cleanup();
        return -1;
    }

    // 6. 执行模幂运算：m = c^e mod n
    if (!BN_mod_exp(m, c, e, n, ctx)) {
        // 模幂失败
        BN_free(m);
        BN_free(c);
        BN_CTX_free(ctx);
        engine_lock_cleanup();
        return -1;
    }

    // 7. 将结果 m 写入 to 缓冲区（大端序）
    int mod_len = BN_num_bytes(n);  // 应该等于 RSA_size(rsa)
    int m_len = BN_bn2bin(m, to);   // 写入实际字节

    // 8. 填充处理：PKCS#1 v1.5 要求输出长度 = RSA_size
    // 如果 m_len < mod_len，前面补 0
    if (m_len < mod_len) {
        // 将数据向前移动，前面补 0
        memmove(to + (mod_len - m_len), to, m_len);
        memset(to, 0, mod_len - m_len);
        m_len = mod_len;
    } else if (m_len > mod_len) {
        // 理论上不会发生，因为 m < n
        BN_free(m);
        BN_free(c);
        BN_CTX_free(ctx);
        engine_lock_cleanup();
        return -1;
    }

    // 9. 清理资源
    BN_free(m);
    BN_free(c);
    BN_CTX_free(ctx);
    engine_lock_cleanup();
    // 10. 返回明文长度
    return m_len;  // 成功
}
static int my_rsa_priv_enc(int flen, const unsigned char *from,
                           unsigned char *to, RSA *rsa, int padding)
{
    // LOG_DebugHex(from, flen, "my_rsa_priv_enc");
    LOG_Debug("[RSA] my_rsa_priv_enc called, padding=%d,flen=%d", padding,flen);
    // LOG_Debug("my_rsa_priv_enc start! -------------------------------------------------------------------------------------------------------------------");
    
    //LOG_Debug("my_rsa_priv_enc start-------------------------------------------------------------------------------------------------------------------");
    int rsabits = 2048;
    
    engine_lock_init();
    // 需要传入的rsa里面有公钥n和e，用来判断bit长度，判断用RSA1024、1280、2048哪种方式进行加密
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (rsa != NULL && rsa->n != NULL)
    {
        rsabits = RSA_size(rsa) * 8;
    }
#else
    if (rsa != NULL)
    {
        const BIGNUM *n = NULL;
        RSA_get0_key(rsa, &n, NULL, NULL);
        if (n != NULL)
        {
            rsabits = RSA_size(rsa) * 8;
        }
    }
#endif


    int ret = HalHsmRSAPrivEnc(PRI_FID_01,flen, rsabits, from, to);
    engine_lock_cleanup();
    LOG_Debug("my_rsa_priv_enc ret = %d", ret);
    //LOG_Debug("my_rsa_priv_enc end! -------------------------------------------------------------------------------------------------------------------");
    return ret;

}
static int my_rsa_priv_dec(int flen, const unsigned char *from,
                           unsigned char *to, RSA *rsa, int padding)
{
    LOG_Debug("my_rsa_priv_dec start! -------------------------------------------------------------------------------------------------------------------");
    int rsabits = 2048;
    engine_lock_init();
    // 需要传入的rsa里面有公钥n和e，用来判断bit长度，判断用RSA1024、1280、2048哪种方式进行加密
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (rsa != NULL && rsa->n != NULL)
    {
        rsabits = RSA_size(rsa) * 8;
    }
#else
    if (rsa != NULL)
    {
        const BIGNUM *n = NULL;
        RSA_get0_key(rsa, &n, NULL, NULL);
        if (n != NULL)
        {
            rsabits = RSA_size(rsa) * 8;
        }
    }
#endif
    int ret = HalHsmRSAPrivDec(PRI_FID_01,flen, rsabits, from, to);
    engine_lock_cleanup();
    LOG_Debug("my_rsa_priv_dec end! -------------------------------------------------------------------------------------------------------------------");
    return ret;
}

static int my_rsa_sign(int type, const unsigned char *m, unsigned int m_length,
                       unsigned char *sigret, unsigned int *siglen, const RSA *rsa)
{
    // 传入的已经是一个sha256了
    LOG_Debug("my_rsa_sign start,m_length:%d,type:%d-------------------------------------------------------------------------------------------------", m_length, type);

    // 对sha256进行签名进行ASN.1编码,加上杂凑算法的国际OID,得到一串稍微长一点的DER数据
    unsigned char der[128] = {0};
    unsigned int derlen = 0;
    engine_lock_init();
    int ret = RSA_sig_to_der(m, m_length, der, &derlen, NID_sha256WithRSAEncryption); // 在32byte sha256前加入编码
    if (ret != 0)
    {
        engine_lock_cleanup();
        return -1;
    }
    // 使用RSA私钥计算签名
    int rsabits = 2048;
    // 需要传入的rsa里面有公钥n和e，用来判断bit长度，判断用RSA1024、1280、2048哪种方式进行加密
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (rsa != NULL && rsa->n != NULL)
    {
        rsabits = RSA_size(rsa) * 8;
    }
#else
    if (rsa != NULL)
    {
        const BIGNUM *n = NULL;
        RSA_get0_key(rsa, &n, NULL, NULL);
        if (n != NULL)
        {
            rsabits = RSA_size(rsa) * 8;
        }
    }
#endif
    ret = HalHsmRSASign(PRI_FID_01,rsabits, der, derlen, sigret, siglen);
    engine_lock_cleanup();
    LOG_Debug("my_rsa_sign end---------------------------------------------------------------------------------------------------------");
    return ret;
}

static int my_rsa_verify(int dtype,
                         const unsigned char *digest, size_t digest_len,
                         const unsigned char *sig, size_t sig_len,
                         RSA *rsa) {
    // 参数检查
    if (!digest || !sig || !rsa || digest_len == 0 || sig_len == 0) {
        return 0;
    }

    LOG_Debug("dtype = %d", dtype);
    //LOG_DebugHex(digest, digest_len, "digest");
    engine_lock_init();
    const BIGNUM *n = NULL;
    RSA_get0_key(rsa, &n, NULL, NULL);
    if (!n) {
        engine_lock_cleanup();
        return 0;
    }
    engine_lock_cleanup();
    //LOG_DebugHex(sig, sig_len, "sig");

    unsigned char plain[256] = {0};
    int plain_len = RSA_public_decrypt(sig_len, sig, plain, rsa, RSA_PKCS1_PADDING);

    //LOG_DebugHex(plain, plain_len, "plain");

    if(memcmp(digest, plain + plain_len - digest_len, digest_len) == 0)
    {
        LOG_Debug("my_rsa_verify success");
    }
    else
    {
        LOG_Debug("my_rsa_verify failed");
    }
    

    return 1;
}


static int my_rsa_init(RSA *rsa)
{
    LOG_Debug("my_rsa_init");
    return 1;
}

static int my_rsa_finish(RSA *rsa)
{
    LOG_Debug("my_rsa_finish");
    return 1;
}

static int my_rsa_keygen(RSA *rsa, int bits, BIGNUM *e, BN_GENCB *cb)
{
#if 0
    BIGNUM *n = NULL;
    BIGNUM *d = NULL;
    int ret = HalHsmRSAKeygen(bits,e,&n,&d);
    if(ret == -1)
    {
        return -1;
    }
    // LOG_Debug("n:");
    // BN_print_fp(stdout, n);
    // LOG_Debug("");
    // LOG_Debug("e:");
    // BN_print_fp(stdout, e);
    // LOG_Debug("");
    if (!n || !d)
    {
        BN_free(n);
        BN_free(d);
        return 0; // 转换失败
    }
    // 公钥指数e，加密芯片支持e=65537或3 默认限制为65537

    // 将生成的密钥设置到 RSA 结构体中
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    rsa->n = BN_dup(n);
    rsa->e = BN_dup(e);
    rsa->d = BN_dup(d);
    BN_free(n);
    BN_free(d);
#else
    if (RSA_set0_key(rsa, n, e, NULL) != 1)
    {
        LOG_Debug("Failed to set RSA key");
    }
#endif
#endif
    return 1;
}


/**
 * @brief 从PUB_FID_01（0A01）读取公钥，如果没生成过公钥，返回的数据都是0
 * 
 * @param rsa 
 * @param bits 
 * @return int 
 */
int RSA_getPubKey(u_int16_t key_Fid,RSA *rsa, int bits)
{

    BIGNUM *n = NULL;
    BIGNUM *e = BN_new();
    // 公钥指数 65537
    BN_set_word(e, RSA_F4);
    int ret = HalHsmRSAGetPubKey(key_Fid,bits,&n);
    if(ret == -1)
    {
        BN_free(e);
        return -1;
    }
    // 公钥指数e，加密芯片支持e=65537或3 默认限制为65537
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    rsa->n = BN_dup(n);
    rsa->e = BN_dup(e);

    BN_free(n);
    BN_free(e);
#else
    //如果没有生成过公钥，n是全0，RSA_set0_key也不会失败
    if (RSA_set0_key(rsa, n, e, NULL) != 1)
    {
        LOG_Debug("Failed to set RSA key");
        BN_free(n);
        BN_free(e);
        return -1;
    }
#endif
    if (RSA_size(rsa) == 0)
    {
        LOG_Debug("RSA_getPubKey:RSA not keygen");
        return -1;
    }

    return 0;

}




#if OPENSSL_VERSION_NUMBER < 0x10100000L
const RSA_METHOD rsa_method = {
    "rsa_engine_test",
    my_rsa_pub_enc,
    my_rsa_pub_dec,
    my_rsa_priv_enc,
    my_rsa_priv_dec,
    NULL,
    NULL,
    my_rsa_init,
    my_rsa_finish,
    ENGINE_METHOD_RSA | RSA_FLAG_SIGN_VER,
    NULL,
    my_rsa_sign,
    my_rsa_verify,
    my_rsa_keygen};
#else
RSA_METHOD *rsa_method = NULL;
int RSA_meth_init(void)
{
    // rsa_method = RSA_meth_new("rsa_engine_sirun", ENGINE_METHOD_RSA | RSA_FLAG_SIGN_VER);
    rsa_method = RSA_meth_new("rsa_engine_sirun", 0);
    if (rsa_method == NULL)
    {
        return -1;
    }
    RSA_meth_set_pub_enc(rsa_method, my_rsa_pub_enc);
    RSA_meth_set_pub_dec(rsa_method, my_rsa_pub_dec);
    RSA_meth_set_priv_enc(rsa_method, my_rsa_priv_enc);
    RSA_meth_set_priv_dec(rsa_method, my_rsa_priv_dec);
    RSA_meth_set_init(rsa_method, my_rsa_init);
    RSA_meth_set_finish(rsa_method, my_rsa_finish);
    RSA_meth_set_sign(rsa_method, my_rsa_sign);
    RSA_meth_set_verify(rsa_method, my_rsa_verify);
    RSA_meth_set_keygen(rsa_method, my_rsa_keygen);
    return 0;
}
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L
static int my_digest_ids[] = {NID_sha256};
static int my_engine_digest_selector(ENGINE *e, const EVP_MD **digest,
                                     const int **nids, int nid)
{
    int ok = 1;
    if (!digest)
    {
        *nids = my_digest_ids;
        LOG_Debug(" Digest is empty! Nid:%d", nid);
        return 2;
    }
    LOG_Debug("Digest no %d requested", nid);
    if (nid == NID_sha256)
    {
        *digest = &my_sha256_engine;
    }
    else
    {
        ok = 0;
        *digest = NULL;
    }
    return ok;
}
#else
#endif

//aes的暂时不支持，此为测试代码
#if 0 
#define AESNI_MIN_ALIGN 16
#define AESNI_ALIGN(x) \
    ((void *)(((unsigned long)(x) + AESNI_MIN_ALIGN - 1) & ~(AESNI_MIN_ALIGN - 1)))
static int aesni_cipher_nids[] = {
    NID_aes_128_ecb,
    NID_aes_128_cbc,

    NID_aes_192_ecb,
    NID_aes_192_cbc,

    NID_aes_256_ecb,
    NID_aes_256_cbc,
};
static int aesni_cipher_nids_num =
    (sizeof(aesni_cipher_nids) / sizeof(aesni_cipher_nids[0]));

typedef struct
{
    AES_KEY ks;
    unsigned int _pad1[3];
} AESNI_KEY;

int aesni_set_encrypt_key(const unsigned char *userKey, int bits,
                          AES_KEY *key)
{
    LOG_Debug("aesni_set_encrypt_key");
    memcpy((void *)key, userKey, bits / 8);
    return 1;
}

int aesni_set_decrypt_key(const unsigned char *userKey, int bits,
                          AES_KEY *key)
{
    LOG_Debug("aesni_set_encrypt_key");
    memcpy((void *)key, userKey, bits / 8);
    return 1;
}

void aesni_ecb_encrypt(const unsigned char *in,
                       unsigned char *out,
                       size_t length,
                       const AES_KEY *key,
                       int enc)
{
    LOG_Debug("aesni_ecb_encrypt");
}

void aesni_cbc_encrypt(const unsigned char *in,
                       unsigned char *out,
                       size_t length,
                       const AES_KEY *key,
                       unsigned char *ivec, int enc)
{
    LOG_Debug("aesni_cbc_encrypt");
}

static int aesni_cipher_cbc(EVP_CIPHER_CTX *ctx, unsigned char *out,
                            const unsigned char *in, size_t inl)
{
    // 参考ecb实现
    return 1;
}

static int aesni_cipher_ecb(EVP_CIPHER_CTX *ctx, unsigned char *out,
                            const unsigned char *in, size_t inl)
{
    uint8_t sbuf[256] = {0};
    uint16_t slen = 0, sw = 0;
    // AES_KEY *key = (void*)(ctx->cipher_data);
    LOG_Debug("aesni_cipher_ecb:size:%d,key:", inl);
    for (int i = 0; i < ctx->key_len; i++)
    {
        // LOG_Debug("%x ", ((unsigned char *)key)[i]);
    }
    // LOG_Debug("");
    memcpy(sbuf + 4, in, inl);
    slen = inl + 4;
    uint16_t outlen = 0;
    if (ctx->encrypt)
    {
        // 进行加密操作
        // 实现自己的 AES-256-CBC 加密逻辑
        // 将结果写入 out
        // memcpy(out, in, inl);
        sbuf[3] = *((unsigned char *)ctx->app_data);
        sw = DataEnDecrypt(0x0040, slen, sbuf, out, &outlen);
        if (sw != 0x9000)
        {
            LOG_Debug("DataEnDecrypt sw : %x", sw);
            return 3;
        }
        LOG_Debug("aesni_cbc_encrypt size:%d", inl);
    }
    else
    {
        // 进行解密操作
        // 实现自己的 AES-256-CBC 解密逻辑
        // 将结果写入 out
        // memcpy(out, in, inl);
        sbuf[3] = *((unsigned char *)(ctx->app_data) + 1);
        sw = DataEnDecrypt(0x00c0, slen, sbuf, out, &outlen);
        if (sw != 0x9000)
        {
            LOG_Debug("DataEnDecrypt sw : %x", sw);
            return 3;
        }
        LOG_Debug("aesni_cbc_decrypt size:%d", inl);
    }
    return 1;
}

static int
aesni_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *user_key,
               const unsigned char *iv, int enc)
{
    LOG_Debug("aesni_init_key");
    uint8_t sbuf[256] = {0}, rbuf[8] = {0};
    uint16_t slen = 0, rlen = 0, sw = 0;
    AES_KEY *key = (void *)(ctx->cipher_data);
    memcpy((void *)key, user_key, ctx->key_len);
    sbuf[0] = 0xc1;
    sbuf[1] = ctx->key_len;
    memcpy(sbuf + 2, user_key, ctx->key_len);
    slen = ctx->key_len + 2;
    // AES 大端数据格式
    uint16_t p1p2 = 0x0710;
    if (ctx->key_len == 192 / 8)
    {
        p1p2 = 0x4710;
    }
    else if (ctx->key_len == 256 / 8)
    {
        p1p2 = 0x8710;
    }
    else
    {
    }
    sw = ImportSessionKey(p1p2, slen, sbuf, rbuf, &rlen);
    if (sw != 0x9000)
    {
        LOG_Debug("ImportSessionKey sw : %x", sw);
        return 2;
    }
    if (ctx->app_data == NULL)
    {
        ctx->app_data = malloc(sizeof(unsigned char) * 2);
    }

    if (enc == 0)
    {
        // ret = aesni_set_decrypt_key(user_key, ctx->key_len * 8, key);

        // 保存解密的对称会话密钥的索引号
        *((unsigned char *)(ctx->app_data) + 1) = rbuf[0];
    }
    else
    {
        // 加密
        //  ret = aesni_set_encrypt_key(user_key, ctx->key_len * 8, key);
        // 保存加密的对称会话密钥的索引号
        *((unsigned char *)ctx->app_data) = rbuf[0];
    }
    LOG_Debug("aesni_init_key:key:");
    for (int i = 0; i < ctx->key_len; i++)
    {
        // LOG_Debug("%x ", ((unsigned char *)key)[i]);
    }
    // LOG_Debug("");

    // if (ret < 0)
    // {
    //     EVPerr(EVP_F_AESNI_INIT_KEY, EVP_R_AES_KEY_SETUP_FAILED);
    //     return 0;
    // }

    // if (ctx->cipher->flags & EVP_CIPH_CUSTOM_IV)
    // {
    //     if (iv != NULL)
    //         memcpy(ctx->iv, iv, ctx->cipher->iv_len);
    //     else
    //     {
    //         EVPerr(EVP_F_AESNI_INIT_KEY, EVP_R_AES_IV_SETUP_FAILED);
    //         return 0;
    //     }
    // }

    return 1;
}

#define AES_BLOCK_SIZE 16

#define EVP_CIPHER_block_size_ECB AES_BLOCK_SIZE
#define EVP_CIPHER_block_size_CBC AES_BLOCK_SIZE
/* Declaring so many ciphers by hand would be a pain.
   Instead introduce a bit of preprocessor magic :-) */
#define DECLARE_AES_EVP(ksize, lmode, umode)            \
    static const EVP_CIPHER aesni_##ksize##_##lmode = { \
        NID_aes_##ksize##_##lmode,                      \
        EVP_CIPHER_block_size_##umode,                  \
        ksize / 8,                                      \
        AES_BLOCK_SIZE,                                 \
        0 | EVP_CIPH_##umode##_MODE,                    \
        aesni_init_key,                                 \
        aesni_cipher_##lmode,                           \
        NULL,                                           \
        sizeof(AESNI_KEY),                              \
        EVP_CIPHER_set_asn1_iv,                         \
        EVP_CIPHER_get_asn1_iv,                         \
        NULL,                                           \
        NULL}

DECLARE_AES_EVP(128, ecb, ECB);
DECLARE_AES_EVP(128, cbc, CBC);

DECLARE_AES_EVP(192, ecb, ECB);
DECLARE_AES_EVP(192, cbc, CBC);

DECLARE_AES_EVP(256, ecb, ECB);
DECLARE_AES_EVP(256, cbc, CBC);

static int
aesni_ciphers(ENGINE *e, const EVP_CIPHER **cipher,
              const int **nids, int nid)
{
    /* No specific cipher => return a list of supported nids ... */
    if (!cipher)
    {
        *nids = aesni_cipher_nids;
        return aesni_cipher_nids_num;
    }

    /* ... or the requested "cipher" otherwise */
    switch (nid)
    {
    case NID_aes_128_ecb:
        *cipher = &aesni_128_ecb;
        break;
    case NID_aes_128_cbc:
        *cipher = &aesni_128_cbc;
        break;
    case NID_aes_192_ecb:
        *cipher = &aesni_192_ecb;
        break;
    case NID_aes_192_cbc:
        *cipher = &aesni_192_cbc;
        break;
    case NID_aes_256_ecb:
        *cipher = &aesni_256_ecb;
        break;
    case NID_aes_256_cbc:
        *cipher = &aesni_256_cbc;
        break;
    default:
        /* Sorry, we don't support this NID */
        *cipher = NULL;
        return 0;
    }
    return 1;
}

/*绑定函数*/
static int bind_pp(ENGINE *e)
{
    if (!ENGINE_set_id(e, eng_id) ||
        !ENGINE_set_name(e, eng_name) ||
        !ENGINE_set_RAND(e, &rand_method) ||
        !ENGINE_set_destroy_function(e, myengine_destroy) ||
        !ENGINE_set_init_function(e, myengine_init) ||
        !ENGINE_set_finish_function(e, myengine_finish) ||
        !ENGINE_set_RSA(e, &rsa_method) ||
        !ENGINE_set_digests(e, &my_engine_digest_selector) || // 设置摘要选择器函数
        !ENGINE_set_ciphers(e, aesni_ciphers))
        return 0;
    // RAND_set_rand_method(&rand_method);
    return 1;
}
#endif
static EVP_PKEY *my_load_privkey(ENGINE *eng, const char *key_id,
                                 UI_METHOD *ui_method, void *callback_data) 
{
    // 根据key_id获取密钥（从HSM/文件/数据库等）
    if(NULL == key_id)
    {
        LOG_Error("get privkey key_id is NULL"); 
        return NULL;       
    }
    LOG_Debug("get privkey key_id:%s ", key_id);
    
    engine_lock_init();
    // 1. 从HSM/加密芯片获取RSA公钥
    RSA *rsa = RSA_new();
    if (!rsa)
    {
        LOG_Error("RSA_new failed");
        engine_lock_cleanup();
        return NULL;
    }

    int ret = RSA_getPubKey(PUB_FID_01,rsa, 2048);

    if (ret < 0)
    {
        LOG_Error("RSA not keygen");
        RSA_free(rsa);
        engine_lock_cleanup();
        return NULL;
    }
    // 2. 将RSA包装为EVP_PKEY
    EVP_PKEY *pkey = EVP_PKEY_new();
    if (!pkey)
    {
        LOG_Error("EVP_PKEY_new failed");
        RSA_free(rsa);
        engine_lock_cleanup();
        return NULL;
    }
    if (EVP_PKEY_assign_RSA(pkey, rsa) != 1)
    {
        LOG_Error("EVP_PKEY_assign_RSA failed");
        EVP_PKEY_free(pkey);
        RSA_free(rsa);
        engine_lock_cleanup();
        return NULL;
    }
    LOG_Debug("get privkey key_id:%s SUCCESS %p", key_id, pkey);
    engine_lock_cleanup();
    return pkey;
}


/*绑定函数*/
static int bind_pp(ENGINE *e)
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L
#else
    RSA_meth_init();
#endif
    if (!ENGINE_set_id(e, eng_id) ||
        !ENGINE_set_name(e, eng_name) ||
        !ENGINE_set_RAND(e, &rand_method) ||
        !ENGINE_set_destroy_function(e, myengine_destroy) ||
        !ENGINE_set_init_function(e, myengine_init) ||
        !ENGINE_set_finish_function(e, myengine_finish) ||
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        !ENGINE_set_RSA(e, &rsa_method) ||
        !ENGINE_set_digests(e, &my_engine_digest_selector))
#else
        !ENGINE_set_RSA(e, rsa_method) || 
        !ENGINE_set_load_privkey_function(e, &my_load_privkey))
#endif
        return 0;
    // RAND_set_rand_method(&rand_method);
    return 1;
}

static ENGINE *engine_sirun_new(void)
{
    ENGINE *e = ENGINE_new();
    if (!e)
        return NULL;
    if (!bind_pp(e))
    {
        ENGINE_free(e);
        return NULL;
    }
    return e;
}
int ENGINE_Init_sirun()
{
    int ret = 0;
    // 初始化 OpenSSL 引擎系统
    ENGINE_load_builtin_engines();
    ENGINE_register_all_complete();

    ENGINE *e = engine_sirun_new();
    if (!e)
    {
        LOG_Error("e null");
        return -1;
    }
    ret = ENGINE_add(e);
    if(ret != 1)
    {
        LOG_Error("engine add fail");
        ENGINE_free(e);
        return -1;
    }
    // ERR_add_error(e);
    display_engine_list();
    // ENGINE *e = NULL;
    // e = ENGINE_by_id(eng_id);
    // if(!e)
    // {
    //     LOG_Debug("err load engine failed");
    //     return 0;
    // }

    ret = ENGINE_init(e);         // 会调用 myengine_init 函数
    if(ret != 1)
    {
        LOG_Error("engine init fail");
        ENGINE_remove(e);
        ENGINE_free(e);
        return -1;
    }
    ret = ENGINE_register_RSA(e); // 会调用my_rsa_init函数
    if(ret != 1)
    {
        LOG_Error("engine register rsa fail");
        ENGINE_remove(e);
        ENGINE_free(e);
        return -1;
    }
    // ENGINE_set_default_ciphers(e);//使能后调用的就是用户实现的ciphers (aes)算法，不加这一句的话调用的就是openssl的默认实现
    // fmse rand长度仅支持4 8 16 所以不用它的
    // ENGINE_set_default_RAND(e);
    // 使用自己的sha256 engine会使产生的csr文件解析出错。。
    // ENGINE_set_default_digests(e);
    return 0;
}
void ENGINE_DeInit_sirun()
{

    ENGINE *e = NULL;
    e = ENGINE_by_id(eng_id);
    if (!e)
    {
        LOG_Debug("err load engine failed");
        return;
    }
    ENGINE_unregister_RSA(e);
    // 清理引擎资源
    //ENGINE_finish(e); // myengine_finish
    if (ENGINE_get_finish_function(e))
    {
        ENGINE_get_finish_function(e)(e);
    }else{
        LOG_Error("ENGINE_get_finish_function NULL");
    }

    ENGINE_remove(e);
    ENGINE_free(e);
    LOG_Debug("ENGINE_DeInit_sirun");
}

int GenX509ReqFile(int version, const char *subjectCN, const char *subjectOU, const char *subjectO, const char *subjectL, const char *subjectST, const char *subjectC,
                   const char *subhectEMail, RSA *rsa, const char *fileName)
{
    X509_REQ *x509_req = NULL;
    X509_NAME *x509_name = NULL;
    int nVersion = version;
    x509_req = X509_REQ_new();
    x509_name = X509_REQ_get_subject_name(x509_req);
    X509_REQ_set_version(x509_req, nVersion);
    X509_NAME_add_entry_by_txt(x509_name, "CN", MBSTRING_ASC, (const unsigned char *)subjectCN, -1, -1, 0); // 证书所请求的域名 主题通用名称
    X509_NAME_add_entry_by_txt(x509_name, "OU", MBSTRING_ASC, (const unsigned char *)subjectOU, -1, -1, 0); // 组织单位
    X509_NAME_add_entry_by_txt(x509_name, "O", MBSTRING_ASC, (const unsigned char *)subjectO, -1, -1, 0);   // 组织
    X509_NAME_add_entry_by_txt(x509_name, "L", MBSTRING_ASC, (const unsigned char *)subjectL, -1, -1, 0);   // 城市
    X509_NAME_add_entry_by_txt(x509_name, "ST", MBSTRING_ASC, (const unsigned char *)subjectST, -1, -1, 0); // 省
    X509_NAME_add_entry_by_txt(x509_name, "C", MBSTRING_ASC, (const unsigned char *)subjectC, -1, -1, 0);   // 国家
    X509_NAME_add_entry_by_txt(x509_name, "emailAddress", MBSTRING_ASC, (const unsigned char *)subhectEMail, -1, -1, 0);
    EVP_PKEY *pKey = NULL;
    pKey = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(pKey, rsa);
    int ret = X509_REQ_set_pubkey(x509_req, pKey);
    if (ret != 1)
    {
        LOG_Debug("MakeLocalCsrSSLApi X509_REQ_set_pubkey err");
    }

    // my_rsa_sign 签名，会将原始数据做sha256运算后，将32byte sha256 传给my_rsa_sign进行签名
    if (0 >= X509_REQ_sign(x509_req, pKey, EVP_sha256()))
    {
        LOG_Debug("failed to sign the certificate");
        X509_REQ_free(x509_req);
        x509_req = NULL;
    }

    FILE *pFile = NULL;
    pFile = fopen(fileName, "w");
    if (pFile == NULL)
    {
        LOG_Debug("fopen failed");
    }
    PEM_write_X509_REQ(pFile, x509_req);
    fclose(pFile);
    return 1;
}

int SaveRSAPubKeyFile(RSA *rsa, char *fileName)
{
    FILE *file = fopen(fileName, "w");
    if (!file)
    {
        LOG_Debug("Error opening file!");
        return -1;
    }
    if (!PEM_write_RSA_PUBKEY(file, rsa))
    {
        LOG_Debug("Error writing RSA public key!");
        return -1;
    }
    fclose(file);
    return 1;
}

int LoadRSAPubKey(RSA **rsa, char *fileName)
{
    FILE *fp = fopen(fileName, "rb");
    if (!fp)
    {
        LOG_Debug("Error: Failed to open public key file.");
        return -1;
    }

    *rsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL);
    fclose(fp);

    if (!*rsa)
    {
        LOG_Debug("Error: Failed to read public key from PEM file.");
        return -1;
    }
    return 1;
}

void display_engine_list()
{
    ENGINE *h;
    int loop;

    h = ENGINE_get_first();
    loop = 0;
    LOG_Debug("start:");
    while (h)
    {
        LOG_Debug("engine %i, id = \"%s\", name = \"%s\"",
                  loop++, ENGINE_get_id(h), ENGINE_get_name(h));
        h = ENGINE_get_next(h);
    }
    LOG_Debug("end of list");
    ENGINE_free(h);
}

void opensslHandleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}


/*
将hash编码为ASN.1 DER数据
hash[in] : 源数据
hashlen[in] : 源数据长度
der[out] : 编码后的数据，传入NULL时，可以获得编码后的数据长度
derlen[out] : 编码后的数据长度
type[in] : 声明哈希的算法ID，NID_sha256 NID_sha256WithRSAEncryption type不一样会导致生成的der头不一样，比如:
NID_sha256：der: 30 31 30 0d 06 09 60 86 48 01 65 03 04 02 01 05 00 04 20 + 32byte sha256
NID_sha256WithRSAEncryption：der: 30 31 30 0d 06 09 2a 86 48 86 f7 0d 01 01 0b 05 00 04 20 + 32byte sha256
在生成csr文件的过程中，需要使用NID_sha256WithRSAEncryption算法，因为openssl req -in test.csr -text解析出来的Signature Algorithm: sha256WithRSAEncryption
*/
int RSA_sig_to_der_ca(const unsigned char *hash, unsigned int hashlen, unsigned char *der, unsigned int *derlen, int type)
{
    int ret = -1;
    X509_SIG *xsig;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
#else
	X509_ALGOR *algor;
	ASN1_OCTET_STRING *pdigest;
#endif
    do
    {
        // 不同于SM2算法的签名转换，此定义已在OpenSSL中声明
        // SM2的签名数据需要自己做声明
        xsig = X509_SIG_new();
        if (!xsig)
        {
            break;
        }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
        if (!X509_ALGOR_set0(xsig->algor, OBJ_nid2obj(type), V_ASN1_NULL, NULL))
        {
            break;
        }
        if (!ASN1_OCTET_STRING_set(xsig->digest, hash, hashlen))
        {
            break;
        }
#else
        // 获得algor和pdigest在xsig中的地址，从而设置xsig的值
        X509_SIG_getm(xsig, &algor, &pdigest);
        if (!X509_ALGOR_set0(algor, OBJ_nid2obj(type), V_ASN1_NULL, NULL))
        {
            break;
        }
        if (!ASN1_OCTET_STRING_set(pdigest, hash, hashlen))
        {
            break;
        }
#endif
        if (der)
        {
            *derlen = i2d_X509_SIG(xsig, &der);
        }
        else
        {
            *derlen = i2d_X509_SIG(xsig, NULL);
        }
        ret = 0;
    } while (0);

    if (xsig)
    {
        X509_SIG_free(xsig);
    }
    return ret;
}

/*
将ASN1编码数据转换为hash
pbDer[in] : ASN1编码数据
uiDerLen[in] : ASN1编码数据长度
hash[out] : 哈希数据
hashlen[out] : 哈希数据长度
*/
int RSA_sig_to_hash_ca(const unsigned char *pbDer, unsigned int uiDerLen, unsigned char *hash, unsigned int *hashlen)
{
    int ret = -1;
    X509_SIG *xsig = NULL;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
#else
    const ASN1_OCTET_STRING *doct = NULL;
#endif
    const unsigned char *data = NULL;
    int datalen = 0;

    do
    {
        xsig = d2i_X509_SIG(NULL, &pbDer, uiDerLen);
        if (!xsig)
        {
            break;
        }
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        datalen = ASN1_STRING_length(xsig->digest);
        if (datalen <= 0)
        {
            break;
        }
        data = ASN1_STRING_data(xsig->digest);
        if (!data)
        {
            break;
        }
#else
        // 这里没有对哈希算法进行解析，如需解析，把NULL变为变量再解析
        X509_SIG_get0(xsig, NULL, &doct);

        datalen = ASN1_STRING_length(doct);
        if (datalen <= 0)
        {
            break;
        }
        data = ASN1_STRING_get0_data(doct);
        if (!data)
        {
            break;
        }
#endif

        memcpy(hash, data, datalen);
        *hashlen = datalen;
        ret = 0;
    } while (0);

    if (xsig)
    {
        X509_SIG_free(xsig);
    }
    return ret;
}

int RSA_signature_ca(const unsigned char *m, unsigned int m_length,
                  unsigned char *sigret, unsigned int *siglen, RSA *rsa)
{
    LOG_Debug("RSA_signature_ca m_length: %d", m_length);
    unsigned char hash[SHA256_DIGEST_LENGTH] = {0};
    int ret = 0;
    // 先用openssl接口算sha256
    
    SHA256(m, m_length, hash);
    // 签名，会将原始数据传给 my_rsa_sign 进行签名
    ret = RSA_sign(0, hash, SHA256_DIGEST_LENGTH, sigret, siglen, rsa); // my_rsa_sign
    
    return ret;
}

int RSA_verifySignature_ca(const unsigned char *m, unsigned int m_length,
                        const unsigned char *sigbuf, unsigned int siglen, RSA *rsa)
{
    unsigned char hash[SHA256_DIGEST_LENGTH] = {0};
    int ret = 0;
    // 先用openssl接口算原始消息sha256
    
    SHA256(m, m_length, hash);
    // 验签 会调用 my_rsa_verify 进行验签
    ret = RSA_verify(0, hash, SHA256_DIGEST_LENGTH, sigbuf, siglen, rsa); // my_rsa_verify
    
    return ret;
}

static int ca_myengine_init(ENGINE *e)
{
    LOG_Debug("ca_myengine_init");
    int ret = 0;
    engine_lock_init();
    ret = HalHsmInit();
    engine_lock_cleanup();
    return ret;
}
static int ca_myengine_finish(ENGINE *e)
{
    LOG_Debug("ca_myengine_finish");
    HalHsmDeinit();
    
    return 1;
}
static int ca_myengine_destroy(ENGINE *e)
{
    LOG_Debug("ca_myengine_destroy");
    return 1;
}

static int ca_myrand_status(void)
{
    LOG_Debug("ca_myrand_status");
    return 1;
}

static int ca_myrand_bytes(unsigned char *buf, int num)
{
    LOG_Debug("ca_myrand_bytes number:%d", num);
    int ret = 0;
    //engine_lock_init();
    ret = HalHsmRand(buf, num);
    //engine_lock_cleanup();
    return ret;
}

static const RAND_METHOD ca_rand_method =
    {
        NULL,
        ca_myrand_bytes,
        NULL,
        NULL,
        NULL,
        ca_myrand_status};

#if OPENSSL_VERSION_NUMBER < 0x10100000L
static int ca_my_engine_sha256_update(EVP_MD_CTX *ctx, const void *data, size_t count);
static int ca_my_engine_sha256_init(EVP_MD_CTX *ctx)
{
    ctx->update = &ca_my_engine_sha256_update;

    LOG_Debug("ca_my_engine_sha256_init initialized! SHA256");
    return 1;
}

static int ca_my_engine_sha256_update(EVP_MD_CTX *ctx, const void *data, size_t count)
{
    LOG_Debug("SHA256 update ! size:%d", count);
    return HalHsmSHA256Update(ctx, data, count);
}

static int ca_my_engine_sha256_final(EVP_MD_CTX *ctx, unsigned char *md)
{
    LOG_Debug("SHA256 final size of EVP_MD: %d", sizeof(EVP_MD));
    memcpy(md, ctx->md_data, ctx->digest->md_size);
    return 1;
}

int ca_my_engine_sha256_copy(EVP_MD_CTX *to, const EVP_MD_CTX *from)
{
    LOG_Debug("Copy SHA256");
    if (to->md_data && from->md_data)
    {
        memcpy(to->md_data, from->md_data, sizeof(*from->md_data));
    }
    return 1;
}

static int ca_my_engine_sha256_clean(EVP_MD_CTX *ctx)
{
    LOG_Debug("SHA256 cleanup");
    if (ctx->md_data)
        memset(ctx->md_data, 0, ctx->digest->md_size);
    return 1;
}

// 摘要算法 digest
// typedef struct env_md_st EVP_MD
static EVP_MD ca_my_sha256_engine = {
    NID_sha256,
    0,
    32,
    EVP_MD_FLAG_DIGALGID_ABSENT,
    ca_my_engine_sha256_init,
    ca_my_engine_sha256_update,
    ca_my_engine_sha256_final,
    ca_my_engine_sha256_copy,
    ca_my_engine_sha256_clean,
    NULL,
    NULL,
    {0},
    64,
    32,
    NULL};
#else
#endif

static int ca_my_rsa_pub_enc(int flen, const unsigned char *from,
                          unsigned char *to, RSA *rsa, int padding)
{
    LOG_Debug("ca_my_rsa_pub_enc start-------------------------------------------------------------------------------------------------------------------");
    int rsabits = 2048;
    engine_lock_init();
    // 需要传入的rsa里面有公钥n和e，用来判断bit长度，判断用RSA1024、1280、2048哪种方式进行加密
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (rsa != NULL && rsa->n != NULL)
    {
        rsabits = RSA_size(rsa) * 8;
    }
#else
    if (rsa != NULL)
    {
        const BIGNUM *n = NULL;
        RSA_get0_key(rsa, &n, NULL, NULL);
        if (n != NULL)
        {
            rsabits = RSA_size(rsa) * 8;
        }
    }
#endif
    // pkcs随机数 先生成256个非0随机数备用
    unsigned char rand_buf[256] = {0};
    int err = RAND_bytes(rand_buf, 256);
    LOG_Debug("RAND_bytes ret : %x", err);
    for (int i = 0; i < 256; i++)
    {
        // 保持数据没有0
        if (rand_buf[i] == 0x00)
            rand_buf[i] = 0xff;
    }

    int ret = HalHsmRSAPubEnc(PUB_FID_02,flen, rsabits, rand_buf, from, to);
    engine_lock_cleanup();
    LOG_Debug("ca_my_rsa_pub_enc end-------------------------------------------------------------------------------------------------------------------");
    return ret;
}
static int ca_my_rsa_pub_dec(int flen, const unsigned char *from,
                          unsigned char *to, RSA *rsa, int padding)
{
    // 1. 参数校验
    if (flen <= 0 || from == NULL || to == NULL || rsa == NULL) {
        return -1;
    }
    engine_lock_init();
    int rsa_size = RSA_size(rsa);
    if (flen != rsa_size) {
        engine_lock_cleanup();
        return -1;  // 输入长度必须等于 RSA 模数长度
    }

    // 2. 获取公钥参数 (n, e)
    const BIGNUM *n = NULL;
    const BIGNUM *e = NULL;
    const BIGNUM *d = NULL;  // 虽然公钥解密不用 d，但可以用来判断是否为私钥
    RSA_get0_key(rsa, &n, &e, &d);

    if (n == NULL || e == NULL) {
        engine_lock_cleanup();
        return -1;
    }

    // 3. 创建 BIGNUM 上下文（用于临时变量）
    BN_CTX *ctx = BN_CTX_new();
    if (ctx == NULL) {
        engine_lock_cleanup();
        return -1;
    }

    // 4. 将密文 from 转为 BIGNUM (c)
    BIGNUM *c = BN_bin2bn(from, flen, NULL);
    if (c == NULL) {
        BN_CTX_free(ctx);
        engine_lock_cleanup();
        return -1;
    }

    // 5. 分配明文 BIGNUM (m)
    BIGNUM *m = BN_new();
    if (m == NULL) {
        BN_free(c);
        BN_CTX_free(ctx);
        engine_lock_cleanup();
        return -1;
    }

    // 6. 执行模幂运算：m = c^e mod n
    if (!BN_mod_exp(m, c, e, n, ctx)) {
        // 模幂失败
        BN_free(m);
        BN_free(c);
        BN_CTX_free(ctx);
        engine_lock_cleanup();
        return -1;
    }

    // 7. 将结果 m 写入 to 缓冲区（大端序）
    int mod_len = BN_num_bytes(n);  // 应该等于 RSA_size(rsa)
    int m_len = BN_bn2bin(m, to);   // 写入实际字节

    // 8. 填充处理：PKCS#1 v1.5 要求输出长度 = RSA_size
    // 如果 m_len < mod_len，前面补 0
    if (m_len < mod_len) {
        // 将数据向前移动，前面补 0
        memmove(to + (mod_len - m_len), to, m_len);
        memset(to, 0, mod_len - m_len);
        m_len = mod_len;
    } else if (m_len > mod_len) {
        // 理论上不会发生，因为 m < n
        BN_free(m);
        BN_free(c);
        BN_CTX_free(ctx);
        engine_lock_cleanup();
        return -1;
    }

    // 9. 清理资源
    BN_free(m);
    BN_free(c);
    BN_CTX_free(ctx);
    engine_lock_cleanup();
    // 10. 返回明文长度
    return m_len;  // 成功
}
static int ca_my_rsa_priv_enc(int flen, const unsigned char *from,
                           unsigned char *to, RSA *rsa, int padding)
{
    //LOG_DebugHex(from, flen, "ca_my_rsa_priv_enc");
    LOG_Debug("[RSA] ca_my_rsa_priv_enc called, padding=%d,flen=%d", padding,flen);
    //LOG_Debug("ca_my_rsa_priv_enc start! -------------------------------------------------------------------------------------------------------------------");
    
    //LOG_Debug("ca_my_rsa_priv_enc start-------------------------------------------------------------------------------------------------------------------");
    int rsabits = 2048;
    engine_lock_init();
    // 需要传入的rsa里面有公钥n和e，用来判断bit长度，判断用RSA1024、1280、2048哪种方式进行加密
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (rsa != NULL && rsa->n != NULL)
    {
        rsabits = RSA_size(rsa) * 8;
    }
#else
    if (rsa != NULL)
    {
        const BIGNUM *n = NULL;
        RSA_get0_key(rsa, &n, NULL, NULL);
        if (n != NULL)
        {
            rsabits = RSA_size(rsa) * 8;
        }
    }
#endif


    int ret = HalHsmRSAPrivEnc(PRI_FID_02,flen, rsabits, from, to);
    engine_lock_cleanup();
    LOG_Debug("ca_my_rsa_priv_enc ret = %d", ret);
    //LOG_Debug("ca_my_rsa_priv_enc end! -------------------------------------------------------------------------------------------------------------------");
    return ret;
}
static int ca_my_rsa_priv_dec(int flen, const unsigned char *from,
                           unsigned char *to, RSA *rsa, int padding)
{
    LOG_Debug("ca_my_rsa_priv_dec start! -------------------------------------------------------------------------------------------------------------------");
    int rsabits = 2048;
    engine_lock_init();
    // 需要传入的rsa里面有公钥n和e，用来判断bit长度，判断用RSA1024、1280、2048哪种方式进行加密
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (rsa != NULL && rsa->n != NULL)
    {
        rsabits = RSA_size(rsa) * 8;
    }
#else
    if (rsa != NULL)
    {
        const BIGNUM *n = NULL;
        RSA_get0_key(rsa, &n, NULL, NULL);
        if (n != NULL)
        {
            rsabits = RSA_size(rsa) * 8;
        }
    }
#endif
    int ret = HalHsmRSAPrivDec(PRI_FID_02,flen, rsabits, from, to);
    engine_lock_cleanup();
    LOG_Debug("ca_my_rsa_priv_dec end! -------------------------------------------------------------------------------------------------------------------");
    return ret;
}

static int ca_my_rsa_sign(int type, const unsigned char *m, unsigned int m_length,
                       unsigned char *sigret, unsigned int *siglen, const RSA *rsa)
{
    // 传入的已经是一个sha256了
    LOG_Debug("ca_my_rsa_sign start,m_length:%d,type:%d-------------------------------------------------------------------------------------------------", m_length, type);

    // 对sha256进行签名进行ASN.1编码,加上杂凑算法的国际OID,得到一串稍微长一点的DER数据
    unsigned char der[128] = {0};
    unsigned int derlen = 0;
    engine_lock_init();
    int ret = RSA_sig_to_der(m, m_length, der, &derlen, NID_sha256WithRSAEncryption); // 在32byte sha256前加入编码
    if (ret != 0)
    {
        return -1;
    }
    // 使用RSA私钥计算签名
    int rsabits = 2048;
    // 需要传入的rsa里面有公钥n和e，用来判断bit长度，判断用RSA1024、1280、2048哪种方式进行加密
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (rsa != NULL && rsa->n != NULL)
    {
        rsabits = RSA_size(rsa) * 8;
    }
#else
    if (rsa != NULL)
    {
        const BIGNUM *n = NULL;
        RSA_get0_key(rsa, &n, NULL, NULL);
        if (n != NULL)
        {
            rsabits = RSA_size(rsa) * 8;
        }
    }
#endif
    ret = HalHsmRSASign(PRI_FID_02,rsabits, der, derlen, sigret, siglen);
    engine_lock_cleanup();
    LOG_Debug("ca_my_rsa_sign end---------------------------------------------------------------------------------------------------------");
    return ret;
}
static int ca_my_rsa_verify(int dtype,
                         const unsigned char *digest, size_t digest_len,
                         const unsigned char *sig, size_t sig_len,
                         RSA *rsa) {
    // 参数检查
    if (!digest || !sig || !rsa || digest_len == 0 || sig_len == 0) {
        return 0;
    }

    LOG_Debug("dtype = %d", dtype);
    //LOG_DebugHex(digest, digest_len, "digest");
    engine_lock_init();
    const BIGNUM *n = NULL;
    RSA_get0_key(rsa, &n, NULL, NULL);
    if (!n) {
        return 0;
    }
    engine_lock_cleanup();
    //LOG_DebugHex(sig, sig_len, "sig");

    unsigned char plain[256] = {0};
    int plain_len = RSA_public_decrypt(sig_len, sig, plain, rsa, RSA_PKCS1_PADDING);

    //LOG_DebugHex(plain, plain_len, "plain");

    if(memcmp(digest, plain + plain_len - digest_len, digest_len) == 0)
    {
        LOG_Debug("ca_my_rsa_verify success");
    }
    else
    {
        LOG_Debug("ca_my_rsa_verify failed");
    }

    
    return 1;
}

static int ca_my_rsa_init(RSA *rsa)
{
    LOG_Debug("ca_my_rsa_init");
    return 1;
}

static int ca_my_rsa_finish(RSA *rsa)
{
    LOG_Debug("ca_my_rsa_finish");
    return 1;
}

static int ca_my_rsa_keygen(RSA *rsa, int bits, BIGNUM *e, BN_GENCB *cb)
{
#if 0
    BIGNUM *n = NULL;
    BIGNUM *d = NULL;
    int ret = HalHsmRSAKeygen(bits,e,&n,&d);
    if(ret == -1)
    {
        return -1;
    }
    // LOG_Debug("n:");
    // BN_print_fp(stdout, n);
    // LOG_Debug("");
    // LOG_Debug("e:");
    // BN_print_fp(stdout, e);
    // LOG_Debug("");
    if (!n || !d)
    {
        BN_free(n);
        BN_free(d);
        return 0; // 转换失败
    }
    // 公钥指数e，加密芯片支持e=65537或3 默认限制为65537

    // 将生成的密钥设置到 RSA 结构体中
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    rsa->n = BN_dup(n);
    rsa->e = BN_dup(e);
    rsa->d = BN_dup(d);
    BN_free(n);
    BN_free(d);
#else
    if (RSA_set0_key(rsa, n, e, NULL) != 1)
    {
        LOG_Debug("Failed to set RSA key");
    }
#endif
#endif
    return 1;
}


#if OPENSSL_VERSION_NUMBER < 0x10100000L
const RSA_METHOD ca_rsa_method = {
    "ca_rsa_engine_test",
    ca_my_rsa_pub_enc,
    ca_my_rsa_pub_dec,
    ca_my_rsa_priv_enc,
    ca_my_rsa_priv_dec,
    NULL,
    NULL,
    ca_my_rsa_init,
    ca_my_rsa_finish,
    ENGINE_METHOD_RSA | RSA_FLAG_SIGN_VER,
    NULL,
    ca_my_rsa_sign,
    ca_my_rsa_verify,
    ca_my_rsa_keygen};
#else
RSA_METHOD *ca_rsa_method = NULL;
int RSA_ca_meth_init(void)
{
    // rsa_method = RSA_meth_new("rsa_engine_sirun", ENGINE_METHOD_RSA | RSA_FLAG_SIGN_VER);
    ca_rsa_method = RSA_meth_new("rsa_engine_sirun_ca", 0);
    if (ca_rsa_method == NULL)
    {
        return -1;
    }
    RSA_meth_set_pub_enc(ca_rsa_method, ca_my_rsa_pub_enc);
    RSA_meth_set_pub_dec(ca_rsa_method, ca_my_rsa_pub_dec);
    RSA_meth_set_priv_enc(ca_rsa_method, ca_my_rsa_priv_enc);
    RSA_meth_set_priv_dec(ca_rsa_method, ca_my_rsa_priv_dec);
    RSA_meth_set_init(ca_rsa_method, ca_my_rsa_init);
    RSA_meth_set_finish(ca_rsa_method, ca_my_rsa_finish);
    RSA_meth_set_sign(ca_rsa_method, ca_my_rsa_sign);
    RSA_meth_set_verify(ca_rsa_method, ca_my_rsa_verify);
    RSA_meth_set_keygen(ca_rsa_method, ca_my_rsa_keygen);
    return 0;
}
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L
static int ca_my_digest_ids[] = {NID_sha256};
static int ca_my_engine_digest_selector(ENGINE *e, const EVP_MD **digest,
                                     const int **nids, int nid)
{
    int ok = 1;
    if (!digest)
    {
        *nids = ca_my_digest_ids;
        LOG_Debug(" Digest is empty! Nid:%d", nid);
        return 2;
    }
    LOG_Debug("Digest no %d requested", nid);
    if (nid == NID_sha256)
    {
        *digest = &ca_my_sha256_engine;
    }
    else
    {
        ok = 0;
        *digest = NULL;
    }
    return ok;
}
#else
#endif

static EVP_PKEY *ca_my_load_privkey(ENGINE *eng, const char *key_id,
                                 UI_METHOD *ui_method, void *callback_data) 
{
    // 根据key_id获取密钥（从HSM/文件/数据库等）
    if(NULL == key_id)
    {
        LOG_Error("ca_my_load_privkey get privkey key_id is NULL"); 
        return NULL;       
    }
    LOG_Debug("ca_my_load_privkey get privkey key_id:%s ", key_id);
    engine_lock_init();

    // 1. 从HSM/加密芯片获取RSA公钥
    RSA *rsa = RSA_new();
    if (!rsa)
    {
        LOG_Error("ca_my_load_privkey RSA_new failed");
        engine_lock_cleanup();
        return NULL;
    }

    int ret = RSA_getPubKey(PUB_FID_02,rsa, 2048);

    if (ret < 0)
    {
        LOG_Error("ca_my_load_privkey RSA not keygen");
        RSA_free(rsa);
        engine_lock_cleanup();
        return NULL;
    }
    // 2. 将RSA包装为EVP_PKEY
    EVP_PKEY *pkey = EVP_PKEY_new();
    if (!pkey)
    {
        LOG_Error("ca_my_load_privkey EVP_PKEY_new failed");
        RSA_free(rsa);
        engine_lock_cleanup();
        return NULL;
    }
    if (EVP_PKEY_assign_RSA(pkey, rsa) != 1)
    {
        LOG_Error("ca_my_load_privkey EVP_PKEY_assign_RSA failed");
        EVP_PKEY_free(pkey);
        RSA_free(rsa);
        engine_lock_cleanup();
        return NULL;
    }
    LOG_Debug("ca_my_load_privkey get privkey key_id:%s SUCCESS %p", key_id, pkey);
    engine_lock_cleanup();
    return pkey;
}


/*绑定函数*/
static int ca_bind_pp(ENGINE *e)
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L
#else
    RSA_ca_meth_init();
#endif
    if (!ENGINE_set_id(e, ca_eng_id) ||
        !ENGINE_set_name(e, ca_eng_name) ||
        !ENGINE_set_RAND(e, &ca_rand_method) ||
        !ENGINE_set_destroy_function(e, ca_myengine_destroy) ||
        !ENGINE_set_init_function(e, ca_myengine_init) ||
        !ENGINE_set_finish_function(e, ca_myengine_finish) ||
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        !ENGINE_set_RSA(e, &ca_rsa_method) ||
        !ENGINE_set_digests(e, &ca_my_engine_digest_selector))
#else
        !ENGINE_set_RSA(e, ca_rsa_method) || 
        !ENGINE_set_load_privkey_function(e, &ca_my_load_privkey))
#endif
        return 0;
    // RAND_set_rand_method(&rand_method);
    return 1;
}

static ENGINE *ca_engine_sirun_new(void)
{
    ENGINE *e = ENGINE_new();
    if (!e)
        return NULL;
    if (!ca_bind_pp(e))
    {
        ENGINE_free(e);
        return NULL;
    }
    return e;
}
int ENGINE_Init_ca_sirun()
{
    int ret = 0;
    // 初始化 OpenSSL 引擎系统
    ENGINE_load_builtin_engines();
    ENGINE_register_all_complete();

    ENGINE *e = ca_engine_sirun_new();
    if (!e)
    {
        LOG_Error("e null");
        return -1;
    }
    ret = ENGINE_add(e);
    if(ret != 1)
    {
        LOG_Error("engine add fail");
        ENGINE_free(e);
        return -1;
    }
    // ERR_add_error(e);
    display_engine_list();
    // ENGINE *e = NULL;
    // e = ENGINE_by_id(eng_id);
    // if(!e)
    // {
    //     LOG_Debug("err load engine failed");
    //     return 0;
    // }

    ret = ENGINE_init(e);         // 会调用 myengine_init 函数
    if(ret != 1)
    {
        LOG_Error("engine init fail");
        ENGINE_remove(e);
        ENGINE_free(e);
        return -1;
    }
    ret = ENGINE_register_RSA(e); // 会调用my_rsa_init函数
    if(ret != 1)
    {
        LOG_Error("engine register rsa fail");
        ENGINE_remove(e);
        ENGINE_free(e);
        return -1;
    }
    // ENGINE_set_default_ciphers(e);//使能后调用的就是用户实现的ciphers (aes)算法，不加这一句的话调用的就是openssl的默认实现
    // fmse rand长度仅支持4 8 16 所以不用它的
    // ENGINE_set_default_RAND(e);
    // 使用自己的sha256 engine会使产生的csr文件解析出错。。
    // ENGINE_set_default_digests(e);
    return 0;
}
void ENGINE_DeInit_ca_sirun()
{
    ENGINE *e = NULL;
    e = ENGINE_by_id(ca_eng_id);
    if (!e)
    {
        LOG_Debug("ENGINE_DeInit_ca_sirun err load engine failed");
        return;
    }
    ENGINE_unregister_RSA(e);
    //ENGINE_finish(e); // myengine_finish
    // 清理引擎资源
     if (ENGINE_get_finish_function(e))
    {
        ENGINE_get_finish_function(e)(e);
    }else{
        LOG_Error("ENGINE_get_finish_function NULL");
    }
    ENGINE_remove(e);
    ENGINE_free(e);
    LOG_Debug("ENGINE_DeInit_ca_sirun");
}



void display_engine_ca_list()
{
    ENGINE *h;
    int loop;

    h = ENGINE_get_first();
    loop = 0;
    LOG_Debug("start:");
    while (h)
    {
        LOG_Debug("engine %i, id = \"%s\", name = \"%s\"",
                  loop++, ENGINE_get_id(h), ENGINE_get_name(h));
        h = ENGINE_get_next(h);
    }
    LOG_Debug("end of list");
    ENGINE_free(h);
}

void caopensslHandleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}


int HsmInit()
{
    LOG_Debug("HsmInit");
    engine_lock_init();
    int i = 0;
    for (i = 0; i < 3; i++)
    {
        if(1 != HalHsmInit())
        {
            LOG_Debug("HalHsmInit fail");
            sleep(1);
        }
        else
        {
            engine_lock_cleanup();
            return 0;
        }
    }

    engine_lock_cleanup();
    return -1;
}


void HsmDeInit()
{
    LOG_Debug("HsmDeInit");
    HalHsmDeinit();

}

int32_t ImportRSAPrivate(const char *rsa,uint16_t pub_Fid,uint16_t pri_Fid)
{
    int ret = 0;
    engine_lock_init();
    ret = HalHsmImportRSAPrivate(rsa,pub_Fid,pri_Fid);
    engine_lock_cleanup();
    return ret;
}


void HsmEnterSleepMode()
{
    //检查锁的状态
    //GetEngineLockStatus()
    if(flag != 0)
    {   
        LOG_Debug("HSM  Sleeping Mode");
        return;
    }
    engine_lock_init();
    LOG_Debug("HsmEnterSleepMode start");
    if(1 != HalHsmInit())
    {
        LOG_Debug("HsmEnterSleepMode HSM inti fail");
        HsmDeInit();
        engine_lock_cleanup();
        return;
    }
    
    sleep(1);
    int ret =  HalHsmEnterSleepMode();
    if(ret == -1)
    {
        LOG_Error("%s :Hsm Enter Sleep Mode failed.",__FUNCTION__);
    }else{
        flag = 1;
    }
    HsmDeInit();
    engine_lock_cleanup();
    
}

void HsmEnterNormalMode()
{
    int status = 0;
    int ret = 0;
    LOG_Debug("HsmEnterNormalMode start");
    PmSetSleepEvent("PKI_SDK");
    engine_lock_init();
    status = system("echo 0  > /sys/class/gpio/gpio102/value");
    if (status == 0)
    {
        LOG_Debug("%s : echo 0  > /sys/class/gpio/gpio102/value success.",__FUNCTION__);
    } else {
        LOG_Error("%s : echo 0  > /sys/class/gpio/gpio102/value failed.",__FUNCTION__);
    }
    usleep(10000);
    status = system("echo 1  > /sys/class/gpio/gpio102/value");
    if (status == 0)
    {
        LOG_Debug("%s : echo 1  > /sys/class/gpio/gpio102/value success.",__FUNCTION__);
    } else {
        LOG_Error("%s : echo 1  > /sys/class/gpio/gpio102/value failed.",__FUNCTION__);
    }
    usleep(10000);
    flag = 0;
    engine_lock_cleanup();
    if(1 != HalHsmInit())
    {
        LOG_Error("HalHsmInit fail");
          
    }
    HsmDeInit();
    PmCleanSleepEvent("PKI_SDK");
    LOG_Debug("HsmEnterNormalMode end ");
}




void HsmRestGpioIinit()
{
    int status = 0;
    LOG_Debug("HsmRestGpioIinit start",__FUNCTION__);
    status = system("echo 102 > /sys/class/gpio/export");
    if (status == 0)
    {
        LOG_Debug("%s : echo 102 > /sys/class/gpio/export success.",__FUNCTION__);
    } else {
        LOG_Error("%s : echo 102 > /sys/class/gpio/export failed.",__FUNCTION__);
    }
    status = system("echo out  > /sys/class/gpio/gpio102/direction");
    if (status == 0)
    {
        LOG_Debug("%s :echo out  > /sys/class/gpio/gpio102/direction success.",__FUNCTION__);
    } else {
        LOG_Error("%s : echo out  > /sys/class/gpio/gpio102/direction failed.",__FUNCTION__);
    }
    status = system("echo 1  > /sys/class/gpio/gpio102/value");
    if (status == 0)
    {
        LOG_Debug("%s : echo 1  > /sys/class/gpio/gpio102/value success.",__FUNCTION__);
    } else {
        LOG_Error("%s : echo 1  > /sys/class/gpio/gpio102/value failed.",__FUNCTION__);
    }

}
