#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OPENSSL_API_COMPAT 10101

#include <openssl/pkcs12.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/asn1.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/obj_mac.h>
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <arpa/inet.h>
#include <glib.h>

#include "openssl_util.h"

#include "sr_log.h"

// 32位time_t最大值（用于timegm转换失败时的兜底）
#define TIME_T_32BIT_MAX 2147483647LL
#define TIME_T_32BIT_MAX_STR "2038-01-19 03:14:07"

// 添加DN字段（NID为OpenSSL预定义标识）
#define ADD_NAME_ENTRY(nid, value) \
        if (X509_NAME_add_entry_by_NID(*name, nid, MBSTRING_UTF8, (const uint8_t*)value, -1, -1, 0) != 1) { \
            LOG_Error("Add DN entry failed: %s", #nid); \
            X509_NAME_free(*name); \
            *name = NULL; \
            return -1; \
        }

/**
 * 自定义UTC时间转64位时间戳函数（替代timegm，32位系统也支持2038年后时间）
 * @param year  年份（4位，如2046）
 * @param mon   月份（1-12）
 * @param day   日期（1-31）
 * @param hour  小时（0-23）
 * @param min   分钟（0-59）
 * @param sec   秒（0-60，60为闰秒）
 * @return      成功返回UTC时间戳（秒级，int64_t）；失败返回-1
 */
static int64_t UtcToTimestamp(int year, int mon, int day, int hour, int min, int sec) {
    // 基础合法性校验（前置已校验，此处二次兜底）
    if (year < 1970 || mon < 1 || mon > 12 || day < 1 || day > 31 || 
        hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 60) {
        LOG_Error("Invalid UTC time: %d-%02d-%02d %02d:%02d:%02d", year, mon, day, hour, min, sec);
        return -1;
    }

    // 步骤1：定义各月份的天数（非闰年）
    int month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // 步骤2：计算从1970年到目标年份前一年的总天数（考虑闰年）
    int64_t total_days = 0;
    for (int y = 1970; y < year; y++) {
        // 闰年判断规则：能被4整除且不是整百年，或能被400整除
        if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) {
            total_days += 366;
        } else {
            total_days += 365;
        }
    }

    // 步骤3：计算目标年份中，到目标月份前一个月的天数（考虑闰年的2月）
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        month_days[1] = 29; // 闰年2月29天
    }
    for (int m = 1; m < mon; m++) {
        total_days += month_days[m - 1];
    }

    // 步骤4：加上目标日期的天数（day-1，因为1号是当月第0天）
    total_days += (day - 1);

    // 步骤5：转换为秒数（天*86400 + 小时*3600 + 分钟*60 + 秒）
    int64_t timestamp = total_days * 86400LL + (int64_t)hour * 3600LL + (int64_t)min * 60LL + sec;

    return timestamp;
}

/**
 * @brief 内置：提取证书 Subject 中的 CN 字段
 * @param x509 已加载的X509证书对象
 * @param out_cn 输出：存储CN的缓冲区（需提前分配，建议长度≥256）
 * @param out_cn_len 输出缓冲区长度
 * @return 0成功，-1失败
 */
static int OpensslGetCertSubjectCN(X509* x509, char* out_cn, int out_cn_len) {
    if (x509 == NULL || out_cn == NULL || out_cn_len <= 0) {
        LOG_Error("Invalid input parameters");
        return -1;
    }

    // 清空输出缓冲区
    memset(out_cn, 0, out_cn_len);

    // 1. 获取证书Subject名称
    X509_NAME* subject_name = X509_get_subject_name(x509);
    if (subject_name == NULL) {
        LOG_Error("Get cert subject name failed: %s", ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }

    // 2. 定位CN字段（NID_commonName对应CN）
    int cn_idx = X509_NAME_get_index_by_NID(subject_name, NID_commonName, -1);
    if (cn_idx < 0) {
        LOG_Error("Cert subject has no CN field");
        return -1;
    }

    X509_NAME_ENTRY* cn_entry = X509_NAME_get_entry(subject_name, cn_idx);
    if (cn_entry == NULL) {
        LOG_Error("Get CN entry failed: %s", ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }

    // 3. 获取CN的ASN1字符串
    ASN1_STRING* cn_asn1 = X509_NAME_ENTRY_get_data(cn_entry);
    if (cn_asn1 == NULL) {
        LOG_Error("Get CN ASN1 string failed: %s", ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }

    // 4. 转换ASN1_STRING到UTF8（适配OpenSSL 3.0.5正确签名）
    uint8_t* utf8_buf = NULL; // 函数会自动分配内存
    int cn_len = ASN1_STRING_to_UTF8(&utf8_buf, cn_asn1);
    if (cn_len < 0 || utf8_buf == NULL) {
        LOG_Error("Convert CN to UTF8 failed: %s", ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }

    // 5. 拷贝到输出缓冲区（防止缓冲区溢出）
    int copy_len = (cn_len < out_cn_len - 1) ? cn_len : (out_cn_len - 1);
    memcpy(out_cn, utf8_buf, copy_len);
    out_cn[copy_len] = '\0'; // 确保字符串终止

    // 6. 释放函数分配的内存（关键！避免内存泄漏）
    OPENSSL_free(utf8_buf);

    return 0;
}


/**
 * @brief 内置：手动对比RSA公钥和私钥的模数+公钥指数（核心匹配逻辑）
 * @param certPubkey 证书公钥
 * @param pkey 私钥
 * @return 0=匹配，1=不匹配，-1=失败
 */
static int OpensslRsaPubkeyMatch(EVP_PKEY* certPubkey, EVP_PKEY* pkey) {
    if (certPubkey == NULL || pkey == NULL) {
        LOG_Error("Invalid input pkey");
        return -1;
    }

    // 转换为RSA对象（兼容所有版本）
    RSA* rsa_cert = EVP_PKEY_get1_RSA(certPubkey);
    RSA* rsa_key = EVP_PKEY_get1_RSA(pkey);
    if (rsa_cert == NULL || rsa_key == NULL) {
        LOG_Error("Convert EVP_PKEY to RSA failed: %s", ERR_error_string(ERR_get_error(), NULL));
        RSA_free(rsa_cert);
        RSA_free(rsa_key);
        return -1;
    }

    // 获取证书公钥的模数(n)和指数(e)
    const BIGNUM* cert_n = RSA_get0_n(rsa_cert);
    const BIGNUM* cert_e = RSA_get0_e(rsa_cert);
    // 获取私钥对应的模数(n)和指数(e)
    const BIGNUM* key_n = RSA_get0_n(rsa_key);
    const BIGNUM* key_e = RSA_get0_e(rsa_key);

    // 校验参数有效性
    if (cert_n == NULL || cert_e == NULL || key_n == NULL || key_e == NULL) {
        LOG_Error("Get RSA n/e failed");
        RSA_free(rsa_cert);
        RSA_free(rsa_key);
        return -1;
    }

    // 对比模数（核心：模数必须完全一致）
    int n_cmp = BN_cmp(cert_n, key_n);
    // 对比公钥指数（通常都是65537）
    int e_cmp = BN_cmp(cert_e, key_e);

    // 释放资源
    RSA_free(rsa_cert);
    RSA_free(rsa_key);

    // 模数和指数都相等则匹配
    return (n_cmp == 0 && e_cmp == 0) ? 0 : 1;
}


/**
 * @brief 辅助函数：仅检查X509证书是否过期（只判断notAfter，不判断notBefore）
 * @param x509 已加载的X509证书对象
 * @param is_expired 传出：1=过期（当前时间>notAfter），0=未过期
 * @return 0成功，-1失败
 */
int OpensslCheckCertExpiredOnly(X509* x509, uint8_t* is_expired) {
    if (x509 == NULL || is_expired == NULL) {
        LOG_Error("Invalid parameters");
        return -1;
    }

    // 获取当前时间（秒级，UTC时间，与证书时间对齐）
    char buf_not_after[24] = {0};
    char buf_not_before[24] = {0};
    int64_t t_after;
    int64_t t_before;
    time_t now = time(NULL);
    int64_t now64 = (int64_t)now;

    const ASN1_TIME* not_after = X509_get0_notAfter(x509);
    if (not_after == NULL) {
        LOG_Error("Get cert notAfter fail: %s", ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }

    const ASN1_TIME* not_before = X509_get0_notBefore(x509);
    if (not_before == NULL) {
        LOG_Error("Get cert notBefore fail: %s", ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }    

    // 自定义转换ASN1_TIME到time_t（替代ASN1_TIME_to_time_t）
    if (OpensslAsn1TimeToTime(&t_after, not_after, buf_not_after, sizeof(buf_not_after)) != 0) {
        LOG_Error("Convert cert notAfter to time_t fail");
        return -1;
    }
    LOG_Warn("Cert not_after (%s)", buf_not_after);

    if (OpensslAsn1TimeToTime(&t_before, not_before, buf_not_before, sizeof(buf_not_before)) != 0) {
        LOG_Error("Convert cert notBefore to time_t fail");
        return -1;
    }
    LOG_Warn("Cert not_before (%s)", buf_not_before);

    // 仅判断：当前时间 > 过期时间 → 视为过期；否则未过期（忽略未生效）
    if (now64 > t_after) {
#if __WORDSIZE == 32        
        LOG_Error("Cert expired (notAfter: %lld, now: %lld)", t_after, now64);
#else   
        LOG_Error("Cert expired (notAfter: %ld, now: %ld)", t_after, now64);
#endif
        *is_expired = 1;
    } else {
        *is_expired = 0;
    }

    return 0;
}

static int OpensslBuildCsrSubject(X509_NAME** name,
    const char* country,  const char* state, const char* city, const char* org, const char* unit,
    const char* vin, const char* tboxSn)
{
    char cn[64] = {0};

    if (name == NULL) {
        LOG_Error("Invalid parameter (name is NULL)");
        return -1;
    }

    *name = X509_NAME_new();
    if (*name == NULL) {
        LOG_Error("Create X509_NAME failed");
        
        return -1;
    }

    snprintf(cn, sizeof(cn), "%s_%s", vin, tboxSn);
    ADD_NAME_ENTRY(NID_countryName, country);
    ADD_NAME_ENTRY(NID_stateOrProvinceName, state);
    ADD_NAME_ENTRY(NID_localityName, city);
    ADD_NAME_ENTRY(NID_organizationName, org);
    ADD_NAME_ENTRY(NID_organizationalUnitName, unit);
    ADD_NAME_ENTRY(NID_commonName, cn);

    #undef ADD_NAME_ENTRY

    return 0;
}

/** 初始化 openssl 库 **/
void OpensslLibraryInit()
{
    LOG_Warn("Openssl Version: %s - %d.%d.%d(0x%lx)", OPENSSL_VERSION_TEXT,
            OPENSSL_VERSION_MAJOR, OPENSSL_VERSION_MINOR, OPENSSL_VERSION_PATCH,
            OPENSSL_VERSION_NUMBER);
    OpenSSL_add_all_algorithms();
}

/** 释放 openssl 库 **/
void OpensslLibraryRelease()
{
    LOG_Warn("Openssl Release");
    EVP_cleanup();
}

/** 获取TLS协议版本 **/
uint32_t OpensslGetTlsVersion(SSL *ssl)
{
    if (!ssl) return 0;
    const char *version = SSL_get_version(ssl);
    if (strstr(version, "TLSv1.3")) return TLS1_3_VERSION;
    if (strstr(version, "TLSv1.2")) return TLS1_2_VERSION;
    if (strstr(version, "TLSv1.1")) return TLS1_1_VERSION;
    if (strstr(version, "TLSv1"))   return TLS1_VERSION;
    if (strstr(version, "SSLv3"))   return SSL3_VERSION;
    return 0;
}

/**
 * 转换ASN1_TIME到int64_t时间戳（替换原timegm方案，32位系统支持2038年后时间）
 */
int OpensslAsn1TimeToTime(int64_t* out_time, const ASN1_TIME* asn1_time, char* buf, size_t buf_len) {
    // 1. 核心参数校验
    if (out_time == NULL || asn1_time == NULL) {
        LOG_Error("Invalid core param: out_time=%p, asn1_time=%p", out_time, asn1_time);
        return -1;
    }
    // 2. buf缓冲区校验
    if (buf != NULL && buf_len < 24) {
        LOG_Error("Buffer too small (need ≥24, got %zu)", buf_len);
        return -1;
    }

    int type = asn1_time->type;
    const uint8_t* data = asn1_time->data;
    int len = asn1_time->length;

    int year = 0, mon = 0, day = 0, hour = 0, min = 0, sec = 0;

    // 步骤1：解析ASN1_TIME（兼容UTCTIME/GENERALIZEDTIME）
    if (type == V_ASN1_UTCTIME) { // 2位年份：YYMMDDHHMMSS[Z/+/-HHMM]
        if (len < 10) {
            LOG_Error("UTCTIME len too short: %d", len);
            return -1;
        }
        // 解析2位年份（OpenSSL规则：00-49=2000-2049，50-99=1950-1999）
        year = (data[0] - '0') * 10 + (data[1] - '0');
        year = (year < 50) ? 2000 + year : 1900 + year;
        mon = (data[2] - '0') * 10 + (data[3] - '0');
        day = (data[4] - '0') * 10 + (data[5] - '0');
        hour = (data[6] - '0') * 10 + (data[7] - '0');
        min = (data[8] - '0') * 10 + (data[9] - '0');
        sec = (len >= 12) ? ((data[10] - '0') * 10 + (data[11] - '0')) : 0;
    } else if (type == V_ASN1_GENERALIZEDTIME) { // 4位年份：YYYYMMDDHHMMSS[Z/+/-HHMM]
        if (len < 12) {
            LOG_Error("GENERALIZEDTIME len too short: %d", len);
            return -1;
        }
        // 解析4位年份
        year = (data[0] - '0') * 1000 + (data[1] - '0') * 100 + (data[2] - '0') * 10 + (data[3] - '0');
        mon = (data[4] - '0') * 10 + (data[5] - '0');
        day = (data[6] - '0') * 10 + (data[7] - '0');
        hour = (data[8] - '0') * 10 + (data[9] - '0');
        min = (data[10] - '0') * 10 + (data[11] - '0');
        sec = (len >= 14) ? ((data[12] - '0') * 10 + (data[13] - '0')) : 0;
    } else {
        LOG_Error("Unsupported ASN1_TIME type: %d", type);
        return -1;
    }

    // 步骤2：基础时间合法性校验
    if (mon < 1 || mon > 12 || day < 1 || day > 31 || hour > 23 || min > 59 || sec > 60) {
        LOG_Error("Invalid time: %d-%02d-%02d %02d:%02d:%02d", year, mon, day, hour, min, sec);
        return -1;
    }

    // 步骤3：调用自定义函数计算64位时间戳（核心替换：不再用timegm）
    int64_t timestamp = UtcToTimestamp(year, mon, day, hour, min, sec);
    if (timestamp == -1) {
        LOG_Error("Custom UTC to timestamp fail: %d-%02d-%02d %02d:%02d:%02d", year, mon, day, hour, min, sec);
        return -1;
    }
    *out_time = timestamp;

    // 步骤4：格式化可读字符串（若buf非空）
    if (buf != NULL) {
        snprintf(buf, buf_len, "%04d-%02d-%02d %02d:%02d:%02d",
                 year, mon, day, hour, min, sec);
    }

    return 0;
}

/** 打包 SHA256 DigestInfo  **/
int OpensslBuildSha256DigestInfo(const uint8_t *digest, uint8_t *digestInfo)
{
    if (!digest || !digestInfo) return 0;

    // SHA256 DigestInfo 固定头（19字节）
    const uint8_t sha256_digest_info_head[] = {
        0x30, 0x31, 0x30, 0x0D, 0x06, 0x09, 0x60, 0x86, 0x48,
        0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20
    };
    const int head_len = sizeof(sha256_digest_info_head);
    const int digestLen = 32; // SHA256固定长度
    const int total_len = head_len + digestLen; // 51字节

    memcpy(digestInfo, sha256_digest_info_head, head_len);
    memcpy(digestInfo + head_len, digest, digestLen);

    return total_len;
}

/**
 * @brief 从PEM格式内存缓冲区加载X509证书（必选），可选提取RSA公钥
 * @param pemBuffer PEM格式证书的内存缓冲区（NULL结尾或指定长度）
 * @param bufferLen pemBuffer的长度（字节数，传0则自动按strlen计算）
 * @param[out] pClientCert 输出：加载成功的X509证书对象（必须非NULL！调用者需自行X509_free释放）
 * @param[out] pRsa 输出：从证书提取的RSA公钥对象（可选，传NULL则不提取；非NULL时调用者需自行RSA_free释放）
 * @return 0-失败，1-成功
 * @note 1. pClientCert 是必传参数，不允许为NULL；pRsa 为可选参数，可传NULL跳过RSA提取；
 *       2. 仅当 pRsa 非NULL时，才执行RSA公钥提取逻辑，提取失败则整体返回失败；
 *       3. 兼容OpenSSL 3.0+，使用EVP_PKEY_get1_RSA（非NULL时需手动释放RSA）；
 *       4. 失败时所有已分配资源会自动释放，避免内存泄漏。
 */
int OpensslLoadCrtSubjectCN(X509* x509, char* SubjectCN, int SubjectCNMaxSize)
{
    int ret = 1;
    do {
        if (!x509 || !SubjectCN || SubjectCNMaxSize <= 0) break;
        X509_NAME *subj = X509_get_subject_name(x509);
        if (!subj) break;
        if (X509_NAME_get_text_by_NID(subj, NID_commonName, SubjectCN, SubjectCNMaxSize) <= 0) break;
        ret = 0;
    } while (0);
    return ret;
}

int OpensslLoadX509RsaFromPemBuffer(const char *pemBuffer, int pemBufferLen, X509 **pClientCert, EVP_PKEY **certPubKey)
{
    if (pemBuffer == NULL || pClientCert == NULL || pemBufferLen <= 0) {
        LOG_Error("invalid param");
        return -1;
    }

    /* 初始化输出参数（避免野指针）*/ 
    *pClientCert = NULL;
    if (certPubKey != NULL) {
        *certPubKey = NULL;
    }

    BIO *memBio = BIO_new_mem_buf(pemBuffer, pemBufferLen);
    if (memBio == NULL) {
        LOG_Error("create memory BIO failed:%s", ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }

    X509 *clientCert = PEM_read_bio_X509(memBio, NULL, NULL, NULL);
    BIO_free(memBio); // 内存BIO用完即释放，不依赖后续逻辑
    if (clientCert == NULL) {
        LOG_Error("load pem data failed:%s", ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }

    if (certPubKey != NULL) {
        *certPubKey = X509_get_pubkey(clientCert);
        if (*certPubKey == NULL) {
            LOG_Error("load pubkey failed:%s", ERR_error_string(ERR_get_error(), NULL));
            X509_free(clientCert);
            return -1;
        }
        LOG_Warn("Load cert and pubKey success!!!");
    } else {
        LOG_Warn("Load cert success!!!");
    }

    *pClientCert = clientCert;
    return 0;
}

/** 加载秘钥文件 **/
int OpensslLoadRsaPrivateKey(const char *keyPath,EVP_PKEY** privKey)
{
    FILE *keyFileFs = NULL;
    
    if (keyPath == NULL || privKey == NULL) {
        LOG_Error("Invalid parameter");
        return -1;
    }

    keyFileFs = fopen(keyPath, "r");

    if (!keyFileFs) {
        LOG_Error("Unable to open private key file: %s", keyPath);
        *privKey = NULL;
        return -1;
    }

    *privKey = PEM_read_PrivateKey(keyFileFs, NULL, NULL, NULL);

    fclose(keyFileFs);

   if (*privKey == NULL) {
        LOG_Error("Load RSA private key failed: %s(%s)", keyPath, ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }

    if (EVP_PKEY_get_id(*privKey) != EVP_PKEY_RSA) {
        LOG_Error("Key is not RSA type (key type: %d)", EVP_PKEY_get_id(*privKey));
        EVP_PKEY_free(*privKey);
        *privKey = NULL;
        return -1;
    }

    return 0;
}

/** 加载秘钥pemBuffer  **/
int OpensslLoadRsaPrivateKeyFromBuffer(const char *keyPemBuf, int keyPemSize, EVP_PKEY** privKey)
{
    BIO *keyBio = NULL;
    
    if (keyPemBuf == NULL || keyPemSize <= 0 || privKey == NULL) {
        LOG_Error("Invalid parameter");
        return -1;
    }

    // 创建内存 BIO，读取 PEM 数据
    keyBio = BIO_new_mem_buf(keyPemBuf, keyPemSize);
    if (keyBio == NULL) {
        LOG_Error("Failed to create memory BIO");
        *privKey = NULL;
        return -1;
    }

    // 从 BIO 读取私钥
    *privKey = PEM_read_bio_PrivateKey(keyBio, NULL, NULL, NULL);
    BIO_free(keyBio);

    if (*privKey == NULL) {
        LOG_Error("Load RSA private key from buffer failed: %s", ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }

    // 检查是否为 RSA 密钥
    if (EVP_PKEY_get_id(*privKey) != EVP_PKEY_RSA) {
        LOG_Error("Key is not RSA type (key type: %d)", EVP_PKEY_get_id(*privKey));
        EVP_PKEY_free(*privKey);
        *privKey = NULL;
        return -1;
    }

    return 0;
}

/** 
 * @brief 加载PEM格式的RSA类型X509证书
 * @param certPath 证书文件路径（PEM格式：-----BEGIN CERTIFICATE-----）
 * @param cert 输出参数，存储加载后的X509证书对象（调用者需自行释放X509_free）
 * @return 成功返回证书中RSA公钥的位数（如2048/4096），失败返回-1
 */
int OpensslLoadRsaCertificate(const char *certPath, X509 **cert)
{
    FILE *certFileFs = NULL;
    EVP_PKEY *pubKey = NULL;  // 从证书提取的公钥

    // 1. 参数合法性校验（和私钥函数保持一致风格）
    if (certPath == NULL || cert == NULL) {
        LOG_Error("Invalid parameter (certPath=%p, cert=%p)", certPath, cert);
        return -1;
    }
    // 初始化输出参数，避免野指针
    *cert = NULL;

    // 2. 打开证书文件
    certFileFs = fopen(certPath, "r");
    if (!certFileFs) {
        LOG_Error("Unable to open certificate file: %s, reason: %s", certPath, strerror(errno));
        return -1;
    }

    // 3. 读取PEM格式X509证书
    *cert = PEM_read_X509(certFileFs, NULL, NULL, NULL);
    // 立即关闭文件（文件句柄仅用于读取，读取完成后释放）
    fclose(certFileFs);
    certFileFs = NULL;

    // 4. 校验证书是否加载成功
    if (*cert == NULL) {
        LOG_Error("Load X509 certificate failed: %s, OpenSSL error: %s", 
                  certPath, ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }

    // 5. 从证书提取公钥
    pubKey = X509_get_pubkey(*cert);
    if (pubKey == NULL) {
        LOG_Error("Extract public key from certificate failed: %s", certPath);
        X509_free(*cert);
        *cert = NULL;
        return -1;
    }

    // 6. 校验公钥是否为RSA类型（和私钥函数的RSA类型校验呼应）
    if (EVP_PKEY_get_id(pubKey) != EVP_PKEY_RSA) {
        LOG_Error("Certificate public key is not RSA type (key type: %d)", EVP_PKEY_get_id(pubKey));
        EVP_PKEY_free(pubKey);
        X509_free(*cert);
        *cert = NULL;
        return -1;
    }

    EVP_PKEY_free(pubKey);

    return 0;
}

/**
 * @brief 从 RSA 私钥中提取公钥并导出为 DER 格式（二进制）
 * @param privkey 输入的 RSA 私钥（EVP_PKEY* 类型，必须是 RSA 类型）
 * @param pubkeyDer 输出的 DER 格式公钥缓冲区（NULL 时仅返回所需大小）
 * @param pubkeyDerBufferSize 输入：缓冲区大小；输出：实际所需/使用的大小（二进制长度，无终止符）
 * @return 0 成功，非 0 失败（错误码见注释）
 * 错误码说明：
 *   0: 成功
 *  -1: 私钥为 NULL
 *  -2: pubkeyDerBufferSize 指针为 NULL
 *  -3: 缓冲区大小无效（<=0 且 pubkeyDer 非 NULL）
 *  -4: 无法从私钥提取公钥（或非 RSA 类型）
 *  -5: 无法创建内存 BIO
 *  -6: 无法将公钥编码为 DER 格式
 *  -7: 缓冲区空间不足
 */
int OpensslExportRsaPubKeyDerFromPrivKey(EVP_PKEY* privkey, uint8_t* pubkeyDer, int* pubkeyDerBufferSize) {
    // 步骤1：参数合法性检查
    if (privkey == NULL) {
        return -1; // 私钥无效
    }
    if (pubkeyDerBufferSize == NULL) {
        return -2; // 缓冲区大小指针无效
    }
    if (pubkeyDer != NULL && *pubkeyDerBufferSize <= 0) {
        return -3; // 缓冲区大小无效
    }

    EVP_PKEY* pubkey = NULL;
    RSA* rsa_priv = NULL;
    RSA* rsa_pub = NULL;
    BIO* mem_bio = NULL;
    int ret = 0;
    long der_len = 0;

    do {
        int pubkey_from_provider = 0;

        // 步骤2：从 EVP_PKEY 中提取 RSA 私钥结构（兼容 1.1.x/3.0.x）
        rsa_priv = EVP_PKEY_get1_RSA(privkey);
        if (rsa_priv == NULL) {
            // 尝试通过 EVP_PKEY_get_bn_param 获取 n/e（provider key 路径）
            BIGNUM *bn_n = NULL;
            BIGNUM *bn_e = NULL;
            if (EVP_PKEY_get_bn_param(privkey, OSSL_PKEY_PARAM_RSA_N, &bn_n) &&
                EVP_PKEY_get_bn_param(privkey, OSSL_PKEY_PARAM_RSA_E, &bn_e)) {
                rsa_pub = RSA_new();
                if (rsa_pub && RSA_set0_key(rsa_pub, bn_n, bn_e, NULL)) {
                    bn_n = NULL;
                    bn_e = NULL;
                    pubkey = EVP_PKEY_new();
                    if (pubkey && EVP_PKEY_set1_RSA(pubkey, rsa_pub)) {
                        pubkey_from_provider = 1;
                    }
                }
                if (bn_n) BN_free(bn_n);
                if (bn_e) BN_free(bn_e);
            }
            if (!pubkey_from_provider) {
                ret = -4;
                break;
            }
        } else {
            // 步骤3：从 RSA 私钥中提取公钥组件（n=模数，e=公钥指数）
            const BIGNUM* n = NULL;
            const BIGNUM* e = NULL;
            RSA_get0_key(rsa_priv, &n, &e, NULL); // 1.1.x/3.0.x 通用接口
            if (n == NULL || e == NULL) {
                ret = -4;
                break;
            }

            // 步骤4：创建 RSA 公钥结构并设置公钥组件
            rsa_pub = RSA_new();
            if (rsa_pub == NULL) {
                ret = -4;
                break;
            }
            // 设置公钥模数和指数（注意：RSA_set0_key 会接管 n/e 的所有权，无需手动释放）
            if (!RSA_set0_key(rsa_pub, BN_dup(n), BN_dup(e), NULL)) {
                ret = -4;
                break;
            }

            // 步骤5：将 RSA 公钥包装为 EVP_PKEY（兼容 1.1.x/3.0.x）
            pubkey = EVP_PKEY_new();
            if (pubkey == NULL || !EVP_PKEY_set1_RSA(pubkey, rsa_pub)) {
                ret = -4;
                break;
            }
        }

        // 步骤6：创建内存 BIO
        mem_bio = BIO_new(BIO_s_mem());
        if (mem_bio == NULL) {
            ret = -5;
            break;
        }

        // 步骤7：将公钥编码为 DER 格式写入 BIO（1.1.x/3.0.x 通用）
        if (!i2d_PUBKEY_bio(mem_bio, pubkey)) {
            ret = -6;
            break;
        }

        // 步骤8：获取 DER 数据长度（二进制，无终止符）
        der_len = BIO_get_mem_data(mem_bio, NULL);
        int required_size = (int)der_len;

        // 步骤9：缓冲区逻辑处理
        if (pubkeyDer == NULL) {
            *pubkeyDerBufferSize = required_size;
            ret = 0;
            break;
        }
        if (*pubkeyDerBufferSize < required_size) {
            *pubkeyDerBufferSize = required_size;
            ret = -7;
            break;
        }

        // 步骤10：复制 DER 数据到输出缓冲区
        memset(pubkeyDer, 0, *pubkeyDerBufferSize);
        BIO_read(mem_bio, pubkeyDer, der_len);
        *pubkeyDerBufferSize = required_size;
        ret = 0;

    } while (0);

    // 步骤11：释放所有资源（按创建逆序释放）
    if (mem_bio != NULL) {
        BIO_free_all(mem_bio);
    }
    if (pubkey != NULL) {
        EVP_PKEY_free(pubkey);
    }
    if (rsa_pub != NULL) {
        RSA_free(rsa_pub);
    }
    if (rsa_priv != NULL) {
        RSA_free(rsa_priv);
    }

    return ret;
}

/**
 * @brief 使用 RSA 私钥（EVP_PKEY）进行 SHA256withRSA 签名
 * @param pkiKey 输入：RSA 私钥（EVP_PKEY* 类型，必须是 RSA 私钥）
 * @param data 输入：待签名的原始数据
 * @param dataLen 输入：待签名数据的长度（字节）
 * @param signature 输出：签名结果缓冲区（NULL 时仅返回所需缓冲区大小）
 * @param signatureLen 输入：缓冲区大小；输出：实际签名长度（字节）
 * @return 0 成功，非 0 失败（错误码见注释）
 * 错误码说明：
 *   0: 成功
 *  -1: 私钥无效（NULL 或非 RSA 类型）
 *  -2: 待签名数据/长度无效
 *  -3: signatureLen 指针为 NULL
 *  -4: 无法创建签名上下文
 *  -5: 签名初始化失败（算法/私钥不匹配）
 *  -6: 数据哈希更新失败
 *  -7: 签名生成失败
 *  -8: 缓冲区空间不足
 */
int OpensslRsaSignWithSHA256(EVP_PKEY* pkiKey, const uint8_t* data, int dataLen, uint8_t* signature, uint16_t* signatureLen) {
    // 步骤1：参数合法性检查
    if (pkiKey == NULL || EVP_PKEY_id(pkiKey) != EVP_PKEY_RSA) {
        return -1; // 私钥无效或非 RSA 类型
    }
    if (data == NULL || dataLen <= 0) {
        return -2; // 待签名数据无效
    }
    if (signatureLen == NULL) {
        return -3; // 签名长度指针无效
    }

    EVP_MD_CTX* md_ctx = NULL;
    int ret = 0;
    size_t sig_len = 0; // OpenSSL 接口使用 size_t 类型存储长度

    do {
        // 步骤2：创建签名上下文（兼容 1.1.x/3.0.x）
        md_ctx = EVP_MD_CTX_new();
        if (md_ctx == NULL) {
            ret = -4;
            break;
        }

        // 步骤3：初始化 SHA256withRSA 签名
        // EVP_sha256() 指定哈希算法为 SHA256，OpenSSL 自动组合 RSA 签名
        // 第三个参数传 NULL 使用默认填充（PKCS#1 v1.5，符合 SHA256withRSA 标准）
        if (!EVP_DigestSignInit(md_ctx, NULL, EVP_sha256(), NULL, pkiKey)) {
            ret = -5;
            break;
        }

        // 步骤4：传入待签名数据（支持分块调用，适合大文件）
        if (!EVP_DigestSignUpdate(md_ctx, data, dataLen)) {
            ret = -6;
            break;
        }

        // 步骤5：获取签名所需长度（先传 NULL 仅计算长度）
        if (!EVP_DigestSignFinal(md_ctx, NULL, &sig_len)) {
            ret = -7;
            break;
        }
        uint16_t required_size = (uint16_t)sig_len;

        // 步骤6：处理输出缓冲区逻辑
        if (signature == NULL) {
            // 仅返回所需缓冲区大小
            *signatureLen = required_size;
            ret = 0;
            break;
        }
        // 检查缓冲区大小是否足够
        if (*signatureLen < required_size) {
            *signatureLen = required_size; // 输出所需大小
            ret = -8;
            break;
        }

        // 步骤7：生成最终签名并写入缓冲区
        if (!EVP_DigestSignFinal(md_ctx, signature, &sig_len)) {
            ret = -7;
            break;
        }
        *signatureLen = (int)sig_len; // 输出实际签名长度
        ret = 0;

    } while (0);

    // 步骤8：释放所有资源
    if (md_ctx != NULL) {
        EVP_MD_CTX_free(md_ctx);
    }

    // 可选：打印 OpenSSL 错误信息（调试用）
    if (ret != 0) {
        
    }

    return ret;
}

/**
 * @brief 生成RSA密钥对（基于OpenSSL EVP接口，符合最佳实践）
 * @param pkey 输出参数，存储生成的RSA密钥对（调用前若指向非NULL，需先释放）
 * @param bits RSA密钥位数（推荐2048/4096，必须≥1024）
 * @return 成功返回0，失败返回-1
 */
int OpensslGenerateRsaPrivateKey(EVP_PKEY **pkey, int bits)
{
    EVP_PKEY_CTX *ctx = NULL;

    // ========== 1. 入参合法性校验 ==========
    // 校验pkey指针是否为NULL
    if (pkey == NULL) {
        LOG_Error("Invalid parameter: pkey pointer is NULL");
        return -1;
    }
    // 校验密钥位数（仅要求≥1024，放宽原2的幂限制，符合OpenSSL规范）
    if (bits < 1024) {
        LOG_Error("Invalid RSA key bits: %d (must be >= 1024)", bits);
        return -1;
    }
    // 若pkey已有指向的EVP_PKEY对象，先释放（避免内存泄漏）
    if (*pkey != NULL) {
        EVP_PKEY_free(*pkey);
        *pkey = NULL;
    }

    // ========== 2. 创建并初始化密钥生成上下文 ==========
    // 创建RSA类型的EVP_PKEY_CTX
    ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx) {
        LOG_Error("EVP_PKEY_CTX_new_id failed: %s", ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }

    // 初始化密钥生成上下文
    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        LOG_Error("EVP_PKEY_keygen_init failed: %s", ERR_error_string(ERR_get_error(), NULL));
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    // ========== 3. 设置RSA密钥参数（最佳实践） ==========
    // 设置密钥位数
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0) {
        LOG_Error("EVP_PKEY_CTX_set_rsa_keygen_bits failed: %s", ERR_error_string(ERR_get_error(), NULL));
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    // ========== 4. 生成RSA密钥对 ==========
    if (EVP_PKEY_keygen(ctx, pkey) <= 0) {
        LOG_Error("EVP_PKEY_keygen failed: %s", ERR_error_string(ERR_get_error(), NULL));
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    LOG_Info("Successfully generate RSA key pair with %d bits", bits);
    
    // ========== 5. 清理资源并返回成功 ==========
    EVP_PKEY_CTX_free(ctx);
    return 0; // 生成成功，返回0
}

/**
 * @brief 生成 CSR
 * @return 0: 成功，-1: 参数错误，-2: 构建DN失败，-3: 创建CSR失败，-4: 签名失败
 */
/**
 * @brief Generate CSR
 *
 * @param[out] csr
 * @param[in]  pkey
 *
 * @return
 *  0   success
 * -1   invalid parameter
 * -2   build subject failed
 * -3   create csr failed
 * -4   set version failed
 * -5   set subject failed
 * -6   set public key failed
 * -7   sign csr failed
 */
int OpensslGenerateCsr(X509_REQ** csr,EVP_PKEY* pkey,const char* country,const char* state,const char* city,const char* org,const char* unit,const char* vin,const char* tboxSn)
{
    int ret = -1;

    X509_NAME* subject = NULL;
    X509_REQ* csrPos = NULL;

    if (csr == NULL || pkey == NULL) {
        LOG_Error("Invalid parameter");
        return -1;
    }

    *csr = NULL;

    do {

        /* Build DN */
        ret = OpensslBuildCsrSubject(
            &subject,
            country,
            state,
            city,
            org,
            unit,
            vin,
            tboxSn);

        if (ret != 0) {
            LOG_Error("Build CSR subject failed");
            ret = -2;
            break;
        }

        /* Create CSR */
        csrPos = X509_REQ_new();
        if (csrPos == NULL) {
            LOG_Error("X509_REQ_new failed");
            ret = -3;
            break;
        }

        /* Version */
        if (X509_REQ_set_version(csrPos, 0L) != 1) {
            LOG_Error("X509_REQ_set_version failed");
            ret = -4;
            break;
        }

        /* Subject */
        if (X509_REQ_set_subject_name(csrPos, subject) != 1) {
            LOG_Error("X509_REQ_set_subject_name failed");
            ret = -5;
            break;
        }

        /* Public key */
        if (X509_REQ_set_pubkey(csrPos, pkey) != 1) {
            LOG_Error("X509_REQ_set_pubkey failed");
            ret = -6;
            break;
        }

        /* Sign CSR */
        if (X509_REQ_sign(csrPos, pkey, EVP_sha256()) <= 0) {
            LOG_Error("X509_REQ_sign failed");
            ret = -7;
            break;
        }

        *csr = csrPos;
        csrPos = NULL;

        ret = 0;

    } while (0);

    if (subject != NULL) {
        X509_NAME_free(subject);
        subject = NULL;
    }

    if (csrPos != NULL) {
        X509_REQ_free(csrPos);
        csrPos = NULL;
    }

    return ret;
}


/**
 * @brief 检查证书加载状态、与私钥匹配性、是否过期
 * @param pkiKey      传入的私钥对象（EVP_PKEY类型）
 * @param crtPath     证书文件路径（PEM格式）
 * @param isMatch     输出参数：1=证书与私钥匹配，0=不匹配/验证失败
 * @param isExpired   输出参数：1=证书已过期，0=未过期（含未生效）
 * @return int        0=证书加载成功；-1=文件打开失败；-2=证书解析失败；-3=参数非法
 */
int OpensslCheckCrt(EVP_PKEY* pkiKey, const char* crtPath, uint8_t* isMatch, uint8_t* isExpired)
{
    // 1. 参数合法性检查
    if (pkiKey == NULL || crtPath == NULL || isMatch == NULL || isExpired == NULL)
    {
        LOG_Error( "Error: Invalid input parameters");
        return -3;
    }

    // 初始化输出参数（默认值）
    *isMatch = 0;
    *isExpired = 0;

    BIO* bio = NULL;
    X509* cert = NULL;
    int ret = -1;

    do {
        // 2. 打开证书文件（PEM格式）
        bio = BIO_new_file(crtPath, "r");
        if (bio == NULL)
        {
            LOG_Error( "Error: Failed to open certificate file: %s", crtPath);
             // 打印OpenSSL具体错误信息
            ret = -1;
            break;
        }

        // 3. 从文件中读取X509证书对象
        cert = PEM_read_bio_X509(bio, NULL, NULL, NULL);
        if (cert == NULL)
        {
            LOG_Error( "Error: Failed to parse certificate file");
            
            ret = -2;
            break;
        }

        // 4. 验证证书与私钥是否匹配
        int matchRet = X509_check_private_key(cert, pkiKey);
        if (matchRet == 1)
        {
            *isMatch = 1;
        }
        else if (matchRet == -1)
        {
            LOG_Error( "Warning: Failed to check private key match");
            
        }

        // 5. 检查证书是否过期（仅判断“是否过期”，不区分“未生效”）
        // X509_cmp_current_time返回值：
        //  0 = 当前时间 ≤ 证书过期时间（未过期）
        // -1 = 当前时间 > 证书过期时间（已过期）
        //  1 = 当前时间 < 证书生效时间（未生效）
        const ASN1_TIME* notAfter = X509_get0_notAfter(cert); // OpenSSL 3.0推荐API
        int expireRet = X509_cmp_current_time(notAfter);
        if (expireRet == -1)
        {
            *isExpired = 1; // 仅当“已过期”时置1，未生效/有效期内均置0
        }

        // 证书加载成功
        ret = 0;
    } while (0);

    // 6. 释放资源（避免内存泄漏）
    if (cert != NULL)
    {
        X509_free(cert);
    }
    if (bio != NULL)
    {
        BIO_free(bio);
    }

    return ret;
}



/**
 * @brief 适配多填充方式的RSA私钥解密函数
 * @param pkiKey 已加载的RSA私钥（无密码保护）
 * @param padding_type 加密端使用的填充方式（必须精准匹配）
 * @param in 密文数据（原始二进制，非Base64/十六进制）
 * @param inLen 密文长度（必须等于RSA密钥大小，如2048位=256字节）
 * @param outPlain 明文输出缓冲区
 * @param outPlainSize 入：缓冲区大小；出：实际明文长度
 * @return 0成功，-1失败
 */
int OpensslPrivateKeyDecryptEx(EVP_PKEY* pkiKey, RSA_PADDING_TYPE padding_type, uint8_t* in, uint16_t inLen, uint8_t* outPlain, uint16_t* outPlainSize)
{
    // 入参校验
    if (pkiKey == NULL || in == NULL || inLen == 0 || outPlain == NULL || outPlainSize == NULL) {
        LOG_Error("Invalid input parameters");
        return -1;
    }

    // 1. 校验密钥类型（必须是RSA）
    if (EVP_PKEY_get_id(pkiKey) != EVP_PKEY_RSA) {
        LOG_Error("Key type is not RSA");
        return -1;
    }

    // 2. 校验密文长度（必须等于RSA密钥模数长度）
    int key_bits = EVP_PKEY_get_bits(pkiKey);
    if (key_bits <= 0) {
        LOG_Error("Failed to get RSA key bits");
        return -1;
    }
    size_t rsa_key_size = key_bits / 8;
    if (inLen != rsa_key_size) {
        LOG_Error("Ciphertext length error! Need %zu bytes, got %d bytes (key bits: %d)",
                  rsa_key_size, inLen, key_bits);
        return -1;
    }

    // 3. 创建解密上下文
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_from_pkey(NULL, pkiKey, NULL);
    if (ctx == NULL) {
        LOG_Error("Create EVP_PKEY_CTX fail: %s", ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }

    // 4. 初始化解密上下文
    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        LOG_Error("Init decrypt fail: %s", ERR_error_string(ERR_get_error(), NULL));
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    // 5. 精准设置填充方式（核心：与加密端完全一致！）
    switch (padding_type) {
        case RSA_PADDING_PKCS1_V15:
            if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
                LOG_Error("Set PKCS1 v1.5 padding fail: %s", ERR_error_string(ERR_get_error(), NULL));
                EVP_PKEY_CTX_free(ctx);
                return -1;
            }
            break;
        case RSA_PADDING_OAEP_SHA256:
            if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0 ||
                EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) <= 0 ||
                EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256()) <= 0) {
                LOG_Error("Set OAEP-SHA256 padding fail: %s", ERR_error_string(ERR_get_error(), NULL));
                EVP_PKEY_CTX_free(ctx);
                return -1;
            }
            break;
        case RSA_PADDING_OAEP_SHA1:
            if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0 ||
                EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha1()) <= 0 ||
                EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha1()) <= 0) {
                LOG_Error("Set OAEP-SHA1 padding fail: %s", ERR_error_string(ERR_get_error(), NULL));
                EVP_PKEY_CTX_free(ctx);
                return -1;
            }
            break;
        case RSA_PADDING_NO_PADDING:
            if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_NO_PADDING) <= 0) {
                LOG_Error("Set no padding fail: %s", ERR_error_string(ERR_get_error(), NULL));
                EVP_PKEY_CTX_free(ctx);
                return -1;
            }
            break;
        default:
            LOG_Error("Unsupported padding type: %d", padding_type);
            EVP_PKEY_CTX_free(ctx);
            return -1;
    }

    // 6. 获取明文长度
    size_t plain_len = *outPlainSize;
    if (EVP_PKEY_decrypt(ctx, NULL, &plain_len, in, inLen) <= 0) {
        LOG_Error("Get plaintext length fail: %s", ERR_error_string(ERR_get_error(), NULL));
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    // 7. 检查缓冲区大小
    if (plain_len > *outPlainSize) {
        LOG_Error("Out buffer too small! Need %zu bytes, got %d bytes", plain_len, *outPlainSize);
        *outPlainSize = plain_len;
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    // 8. 执行解密
    if (EVP_PKEY_decrypt(ctx, outPlain, &plain_len, in, inLen) <= 0) {
        LOG_Error("Decrypt fail: %s", ERR_error_string(ERR_get_error(), NULL));
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    // 9. 设置实际明文长度并清理资源
    *outPlainSize = plain_len;
    EVP_PKEY_CTX_free(ctx);
    return 0;
}


/**
 * @brief AES-ECB-PKCS5Padding 加密（兼容Java的aesEncrypt）
 * @param in 待加密的明文数据
 * @param inLen 明文长度（任意长度）
 * @param aesKey AES密钥（仅支持16/24/32字节，对应128/192/256位）
 * @param keyLen 密钥长度
 * @param out 加密后的密文缓冲区（需提前分配，建议大小：inLen + 16）
 * @param outLen 传出：实际密文长度
 * @return 0成功，-1失败
 */
int OpensslAesEcbPkcs5Encrypt(uint8_t* in, uint16_t inLen, uint8_t* aesKey, uint16_t keyLen, uint8_t* out, uint16_t* outLen)
{
    // 入参合法性校验
    if (in == NULL || inLen == 0 || aesKey == NULL || out == NULL || outLen == NULL) {
        LOG_Error("Invalid input parameters");
        return -1;
    }

    // AES密钥长度校验（仅支持16/24/32字节）
    const EVP_CIPHER* cipher = NULL;
    if (keyLen == 16) {
        cipher = EVP_aes_128_ecb();
    } else if (keyLen == 24) {
        cipher = EVP_aes_192_ecb();
    } else if (keyLen == 32) {
        cipher = EVP_aes_256_ecb();
    } else {
        LOG_Error("AES key length error! Only support 16/24/32 bytes, got %u bytes", keyLen);
        return -1;
    }

    // 1. 创建并初始化加密上下文
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        LOG_Error("Create EVP_CIPHER_CTX fail: %s", ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }

    // 2. 初始化加密上下文（ECB模式无IV，最后一个参数传NULL）
    if (EVP_EncryptInit_ex(ctx, cipher, NULL, aesKey, NULL) != 1) {
        LOG_Error("Init encrypt ctx fail: %s", ERR_error_string(ERR_get_error(), NULL));
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    // 3. 启用PKCS7填充（对应Java的PKCS5Padding）
    EVP_CIPHER_CTX_set_padding(ctx, 1);

    // 4. 执行加密（分块处理）
    int len = 0;
    int cipher_len = 0;
    if (EVP_EncryptUpdate(ctx, out, &len, in, inLen) != 1) {
        LOG_Error("Encrypt update fail: %s", ERR_error_string(ERR_get_error(), NULL));
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    cipher_len = len;

    // 5. 完成加密（处理最后一块填充）
    if (EVP_EncryptFinal_ex(ctx, out + len, &len) != 1) {
        LOG_Error("Encrypt final fail: %s", ERR_error_string(ERR_get_error(), NULL));
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    cipher_len += len;

    // 6. 设置输出长度
    *outLen = cipher_len;
    
    // 释放上下文资源
    EVP_CIPHER_CTX_free(ctx);
    return 0;
}

/**
 * @brief AES-ECB-PKCS5Padding 解密（兼容Java的aesDecrypt）
 * @param in 待解密的密文数据
 * @param inLen 密文长度（必须是16的整数倍）
 * @param aesKey AES密钥（仅支持16/24/32字节）
 * @param keyLen 密钥长度
 * @param out 解密后的明文缓冲区（需提前分配，建议大小：inLen）
 * @param outLen 传出：实际明文长度
 * @return 0成功，-1失败
 */
int OpensslAesEcbPkcs5Decrypt(uint8_t* in, uint16_t inLen, uint8_t* aesKey, uint16_t keyLen, uint8_t* out, uint16_t* outLen)
{
    // 入参合法性校验
    if (in == NULL || inLen == 0 || aesKey == NULL || out == NULL || outLen == NULL) {
        LOG_Error("Invalid input parameters");
        return -1;
    }

    // 密文长度校验（ECB模式密文必须是16的整数倍）
    if (inLen % 16 != 0) {
        LOG_Error("Ciphertext length error! Must be multiple of 16, got %u bytes", inLen);
        return -1;
    }

    // AES密钥长度校验
    const EVP_CIPHER* cipher = NULL;
    if (keyLen == 16) {
        cipher = EVP_aes_128_ecb();
    } else if (keyLen == 24) {
        cipher = EVP_aes_192_ecb();
    } else if (keyLen == 32) {
        cipher = EVP_aes_256_ecb();
    } else {
        LOG_Error("AES key length error! Only support 16/24/32 bytes, got %u bytes", keyLen);
        return -1;
    }

    // 1. 创建并初始化解密上下文
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        LOG_Error("Create EVP_CIPHER_CTX fail: %s", ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }

    // 2. 初始化解密上下文（ECB模式无IV）
    if (EVP_DecryptInit_ex(ctx, cipher, NULL, aesKey, NULL) != 1) {
        LOG_Error("Init decrypt ctx fail: %s", ERR_error_string(ERR_get_error(), NULL));
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    // 3. 启用PKCS7填充（对应Java的PKCS5Padding）
    EVP_CIPHER_CTX_set_padding(ctx, 1);

    // 4. 执行解密（分块处理）
    int len = 0;
    int plain_len = 0;
    if (EVP_DecryptUpdate(ctx, out, &len, in, inLen) != 1) {
        LOG_Error("Decrypt update fail: %s", ERR_error_string(ERR_get_error(), NULL));
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plain_len = len;

    // 5. 完成解密（处理填充）
    if (EVP_DecryptFinal_ex(ctx, out + len, &len) != 1) {
        LOG_Error("Decrypt final fail: %s", ERR_error_string(ERR_get_error(), NULL));
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plain_len += len;

    // 6. 设置输出长度并清理资源
    *outLen = plain_len;
    EVP_CIPHER_CTX_free(ctx);
    return 0;
}

/**
 * @brief 从内存缓冲区校验PEM证书和私钥（含CN校验、过期校验）
 * @param crtPemBuf PEM证书缓冲区（ASCII格式）
 * @param crtPemSize 证书缓冲区长度
 * @param keyPemBuf PEM私钥缓冲区（ASCII格式）
 * @param keyPemSize 私钥缓冲区长度
 * @param subjectCN 期望的证书Subject CN值（NULL则跳过CN校验）
 * @return 0=匹配+未过期+CN匹配，-1=参数/加载错误，1=密钥不匹配，2=证书过期，3=CN不匹配
 */
int OpensslRsaCheckKeyAndCrtBuffer(const char* crtPemBuf, int crtPemSize, const char* keyPemBuf, int keyPemSize, const char* subjectCN) {
    // 入参校验
    if (crtPemBuf == NULL || crtPemSize <= 0 || keyPemBuf == NULL || keyPemSize <= 0) {
        LOG_Error("Invalid input parameters (NULL or size <=0)");
        return -1;
    }

    // 资源初始化
    X509* x509 = NULL;
    EVP_PKEY* pkey = NULL;
    EVP_PKEY* certPubkey = NULL;
    BIO* crt_bio = NULL;
    BIO* key_bio = NULL;
    uint8_t cert_expired = 0;
    int match_ret = -1;
    unsigned long ssl_err = 0;
    char cert_cn[256] = {0}; // 存储证书实际CN

    // 1. 加载PEM证书（从内存缓冲区）
    crt_bio = BIO_new_mem_buf(crtPemBuf, crtPemSize);
    if (crt_bio == NULL) {
        ssl_err = ERR_get_error();
        LOG_Error("Create cert BIO failed: %s", ERR_error_string(ssl_err, NULL));
        return -1;
    }
    x509 = PEM_read_bio_X509(crt_bio, NULL, NULL, NULL);
    if (x509 == NULL) {
        ssl_err = ERR_get_error();
        LOG_Error("Load cert from buffer failed: %s", ERR_error_string(ssl_err, NULL));
        BIO_free(crt_bio);
        return -1;
    }

    // 2. 校验证书CN（若传入subjectCN不为NULL）
    if (subjectCN != NULL && strlen(subjectCN) > 0) {
        if (OpensslGetCertSubjectCN(x509, cert_cn, sizeof(cert_cn)) != 0) {
            LOG_Error("Extract cert CN failed");
            X509_free(x509);
            BIO_free(crt_bio);
            return -1;
        }
        LOG_Warn("crt subjectCN:%s", cert_cn);
        if (strcmp(cert_cn, subjectCN) != 0) {
            LOG_Error("Cert CN not match (expect: %s, actual: %s)", subjectCN, cert_cn);
            X509_free(x509);
            BIO_free(crt_bio);
            return 3; // CN不匹配
        }
    }

    // 3. 检查证书是否过期
    if (OpensslCheckCertExpiredOnly(x509, &cert_expired) != 0) {
        LOG_Error("Check cert expired failed");
        X509_free(x509);
        BIO_free(crt_bio);
        return -1;
    }

    // 4. 加载PEM私钥（从内存缓冲区）
    key_bio = BIO_new_mem_buf(keyPemBuf, keyPemSize);
    if (key_bio == NULL) {
        ssl_err = ERR_get_error();
        LOG_Error("Create key BIO failed: %s", ERR_error_string(ssl_err, NULL));
        X509_free(x509);
        BIO_free(crt_bio);
        return -1;
    }
    pkey = PEM_read_bio_PrivateKey(key_bio, NULL, NULL, NULL); // 无密码私钥
    if (pkey == NULL) {
        ssl_err = ERR_get_error();
        LOG_Error("Load key from buffer failed: %s", ERR_error_string(ssl_err, NULL));
        X509_free(x509);
        BIO_free(crt_bio);
        BIO_free(key_bio);
        return -1;
    }

    // 5. 提取证书公钥并校验类型
    certPubkey = X509_get_pubkey(x509);
    if (certPubkey == NULL) {
        ssl_err = ERR_get_error();
        LOG_Error("Extract cert public key failed: %s", ERR_error_string(ssl_err, NULL));
        X509_free(x509);
        EVP_PKEY_free(pkey);
        BIO_free(crt_bio);
        BIO_free(key_bio);
        return -1;
    }
    if (EVP_PKEY_get_id(certPubkey) != EVP_PKEY_RSA || EVP_PKEY_get_id(pkey) != EVP_PKEY_RSA) {
        LOG_Error("Key type not RSA (cert: %d, key: %d)", EVP_PKEY_get_id(certPubkey), EVP_PKEY_get_id(pkey));
        X509_free(x509);
        EVP_PKEY_free(pkey);
        EVP_PKEY_free(certPubkey);
        BIO_free(crt_bio);
        BIO_free(key_bio);
        return -1;
    }

    // 6. 校验证书与私钥是否匹配
    match_ret = OpensslRsaPubkeyMatch(certPubkey, pkey);
    if (match_ret < 0) {
        LOG_Error("Check key-crt match failed");
        X509_free(x509);
        EVP_PKEY_free(pkey);
        EVP_PKEY_free(certPubkey);
        BIO_free(crt_bio);
        BIO_free(key_bio);
        return -1;
    }

    // 7. 最终结果判断并清理资源
    int ret;
    if (match_ret != 0) {
        ret = 1; // 密钥不匹配
        LOG_Error("key-crt is not match");
    } else if (cert_expired) {
        ret = 2; // 证书过期
        LOG_Error("crt is expired");
    } else {
        ret = 0; // 全部校验通过
    }

    // 释放所有资源
    X509_free(x509);
    EVP_PKEY_free(pkey);
    EVP_PKEY_free(certPubkey);
    BIO_free(crt_bio);
    BIO_free(key_bio);

    return ret;
}

/**
 * @brief 从文件路径校验PEM证书和私钥（含CN校验、过期校验）
 * @param crtPemPath PEM证书文件路径
 * @param keyPemPath PEM私钥文件路径
 * @param subjectCN 期望的证书Subject CN值（NULL则跳过CN校验）
 * @return 0=匹配+未过期+CN匹配，-1=参数/加载错误，1=密钥不匹配，2=证书过期，3=CN不匹配
 */
int OpensslRsaCheckKeyAndCrtFile(const char* crtPemPath, const char* keyPemPath, const char* subjectCN) {
    // 入参校验
    if (crtPemPath == NULL || keyPemPath == NULL) {
        LOG_Error("Invalid input parameters (NULL)");
        return -1;
    }

    // 资源初始化
    X509* x509 = NULL;
    EVP_PKEY* pkey = NULL;
    EVP_PKEY* certPubkey = NULL;
    FILE* crt_fp = NULL;
    FILE* key_fp = NULL;
    uint8_t cert_expired = 0;
    int match_ret = -1;
    unsigned long ssl_err = 0;
    char cert_cn[256] = {0}; // 存储证书实际CN

    // 1. 加载PEM证书（从文件）
    crt_fp = fopen(crtPemPath, "r");
    if (crt_fp == NULL) {
        LOG_Error("Open cert file %s failed: %s", crtPemPath, strerror(errno));
        return -1;
    }
    x509 = PEM_read_X509(crt_fp, NULL, NULL, NULL);
    if (x509 == NULL) {
        ssl_err = ERR_get_error();
        LOG_Error("Load cert from file %s failed: %s", crtPemPath, ERR_error_string(ssl_err, NULL));
        fclose(crt_fp);
        return -1;
    }

    // 2. 校验证书CN（若传入subjectCN不为NULL）
    if (subjectCN != NULL && strlen(subjectCN) > 0) {
        if (OpensslGetCertSubjectCN(x509, cert_cn, sizeof(cert_cn)) != 0) {
            LOG_Error("Extract cert CN failed");
            X509_free(x509);
            fclose(crt_fp);
            return -1;
        }
        LOG_Warn("crt subjectCN:%s", cert_cn);
        if (strcmp(cert_cn, subjectCN) != 0) {
            LOG_Error("Cert CN not match (expect: %s, actual: %s)", subjectCN, cert_cn);
            X509_free(x509);
            fclose(crt_fp);
            return 3; // CN不匹配
        }
    }

    // 3. 检查证书是否过期
    if (OpensslCheckCertExpiredOnly(x509, &cert_expired) != 0) {
        LOG_Error("Check cert expired failed");
        X509_free(x509);
        fclose(crt_fp);
        return -1;
    }

    // 4. 加载PEM私钥（从文件）
    key_fp = fopen(keyPemPath, "r");
    if (key_fp == NULL) {
        LOG_Error("Open key file %s failed: %s", keyPemPath, strerror(errno));
        X509_free(x509);
        fclose(crt_fp);
        return -1;
    }
    pkey = PEM_read_PrivateKey(key_fp, NULL, NULL, NULL); // 无密码私钥
    if (pkey == NULL) {
        ssl_err = ERR_get_error();
        LOG_Error("Load key from file %s failed: %s", keyPemPath, ERR_error_string(ssl_err, NULL));
        X509_free(x509);
        fclose(crt_fp);
        fclose(key_fp);
        return -1;
    }

    // 5. 提取证书公钥并校验类型
    certPubkey = X509_get_pubkey(x509);
    if (certPubkey == NULL) {
        ssl_err = ERR_get_error();
        LOG_Error("Extract cert public key failed: %s", ERR_error_string(ssl_err, NULL));
        X509_free(x509);
        EVP_PKEY_free(pkey);
        fclose(crt_fp);
        fclose(key_fp);
        return -1;
    }
    if (EVP_PKEY_get_id(certPubkey) != EVP_PKEY_RSA || EVP_PKEY_get_id(pkey) != EVP_PKEY_RSA) {
        LOG_Error("Key type not RSA (cert: %d, key: %d)", EVP_PKEY_get_id(certPubkey), EVP_PKEY_get_id(pkey));
        X509_free(x509);
        EVP_PKEY_free(pkey);
        EVP_PKEY_free(certPubkey);
        fclose(crt_fp);
        fclose(key_fp);
        return -1;
    }

    // 6. 校验证书与私钥是否匹配
    match_ret = OpensslRsaPubkeyMatch(certPubkey, pkey);
    if (match_ret < 0) {
        LOG_Error("Check key-crt match failed");
        X509_free(x509);
        EVP_PKEY_free(pkey);
        EVP_PKEY_free(certPubkey);
        fclose(crt_fp);
        fclose(key_fp);
        return -1;
    }

    // 7. 最终结果判断并清理资源
    int ret;
    if (match_ret != 0) {
        ret = 1; // 密钥不匹配
        LOG_Error("key-crt is not match");
    } else if (cert_expired) {
        ret = 2; // 证书过期
        LOG_Error("crt is expired");
    } else {
        ret = 0; // 全部校验通过
    }

    // 释放所有资源
    X509_free(x509);
    EVP_PKEY_free(pkey);
    EVP_PKEY_free(certPubkey);
    fclose(crt_fp);
    fclose(key_fp);

    return ret;
}

/**
 * @brief 从X509证书的SAN扩展中提取所有有效的DNS域名和IP地址（适配多条目）
 * @param serverCrt 待解析的X509证书指针（非NULL）
 * @param dns_list 输出参数：存储所有DNS域名的GList链表（外部传入NULL，函数内部填充）
 * @param ip_list  输出参数：存储所有IP地址的GList链表（外部传入NULL，函数内部填充）
 * @return 0-成功，1-入参非法，2-SAN解析失败（无SAN扩展不返回错误）
 * @note 1. 链表节点存储的字符串由g_malloc分配，外部需调用g_list_free_full(dns_list, g_free)释放；
 *       2. 支持IPv4/IPv6，IP地址以字符串形式存储；
 *       3. 空的/无效的SAN条目会被跳过，仅保留有效数据。
 */
int OpensslX509ExtractHostAndIPFromSAN(X509 *serverCrt, GList **dns_list, GList **ip_list)
{
    // ========== 变量定义 ==========
    STACK_OF(GENERAL_NAME) *san_list = NULL;
    int i = 0;
    GENERAL_NAME *gn = NULL;
    ASN1_IA5STRING *dns_name = NULL;
    uint8_t *utf8_buf = NULL;
    int dns_len = 0;
    ASN1_OCTET_STRING *ip_octet = NULL;
    const uint8_t *ip_data = NULL;
    int ip_bin_len = 0;
    char ip_str[INET6_ADDRSTRLEN] = {0};
    struct in_addr ipv4;
    struct in6_addr ipv6;

    // ========== 1. 入参合法性检查 ==========
    if (serverCrt == NULL || dns_list == NULL || ip_list == NULL) {
        LOG_Warn("入参非法：serverCrt/dns_list/ip_list不能为NULL");
        return 1; // 入参非法
    }

    // 初始化链表（确保外部传入的链表初始为NULL，避免重复添加）
    *dns_list = NULL;
    *ip_list = NULL;

    // ========== 2. 解析SAN扩展 ==========
    san_list = X509_get_ext_d2i(serverCrt, NID_subject_alt_name, NULL, NULL);
    if (san_list == NULL) {
        return 0; // 无SAN扩展不算错误，返回空链表
    }

    // ========== 3. 遍历SAN列表，收集所有有效DNS/IP ==========
    for (i = 0; i < sk_GENERAL_NAME_num(san_list); i++) {
        gn = sk_GENERAL_NAME_value(san_list, i);
        if (gn == NULL) {
            continue;
        }

        // -------------- 处理DNS域名（GEN_DNS） --------------
        if (gn->type == GEN_DNS) {
            dns_name = gn->d.dNSName;
            if (dns_name == NULL || ASN1_STRING_length(dns_name) == 0) {
                continue;
            }

            // 转换为UTF8编码
            dns_len = ASN1_STRING_to_UTF8(&utf8_buf, (ASN1_STRING *)dns_name);
            if (dns_len < 0 || utf8_buf == NULL) {
                LOG_Warn("DNS条目[%d]UTF8转换失败，跳过", i);
                if (utf8_buf != NULL) OPENSSL_free(utf8_buf);
                utf8_buf = NULL;
                continue;
            }

            // 分配内存并添加到DNS链表（外部释放）
            char *dns_str = g_malloc(dns_len + 1); // 预留'\0'位置
            memcpy(dns_str, utf8_buf, dns_len);
            dns_str[dns_len] = '\0';
            *dns_list = g_list_append(*dns_list, dns_str);

            // 释放临时内存
            OPENSSL_free(utf8_buf);
            utf8_buf = NULL;
        }

        // -------------- 处理IP地址（GEN_IPADD） --------------
        else if (gn->type == GEN_IPADD) {
            ip_octet = gn->d.iPAddress;
            if (ip_octet == NULL || ASN1_STRING_length(ip_octet) == 0) {
                continue;
            }

            // 提取二进制IP数据并转换为字符串
            ip_data = ASN1_STRING_get0_data(ip_octet);
            ip_bin_len = ASN1_STRING_length(ip_octet);
            if (ip_bin_len == 4) { // IPv4
                memcpy(&ipv4.s_addr, ip_data, 4);
                strcpy(ip_str, inet_ntoa(ipv4));
            } else if (ip_bin_len == 16) { // IPv6
                memcpy(&ipv6.s6_addr, ip_data, 16);
                inet_ntop(AF_INET6, &ipv6, ip_str, INET6_ADDRSTRLEN);
            } else {
                LOG_Warn("IP条目[%d]长度非法（%d），跳过", i, ip_bin_len);
                continue;
            }

            // 分配内存并添加到IP链表（外部释放）
            char *ip_str_dup = g_strdup(ip_str); // g_strdup = g_malloc + strcpy
            *ip_list = g_list_append(*ip_list, ip_str_dup);
        }
    }

    // ========== 4. 释放资源 ==========
    if (utf8_buf != NULL) OPENSSL_free(utf8_buf);
    if (san_list != NULL) sk_GENERAL_NAME_pop_free(san_list, GENERAL_NAME_free);

    return 0; // 成功
}


/**
* 使用公钥进行 RSA 加密
* @param pubKeyPath PEM格式公钥文件路径
* @param in 明文数据
* @param inLen 明文长度
* @param out 密文输出缓冲区
* @param outLen 密文长度（输入为缓冲区大小，输出为实际加密长度）
* @return 0成功，-1失败
*/
int OpensslPublicKeyEncrypt(const char *pubKeyPath, RSA_PADDING_TYPE padding_type, const uint8_t *in, int inLen, uint8_t *out, int *outLen)
{
    if (pubKeyPath == NULL || in == NULL || out == NULL || outLen == NULL) {
        LOG_Error("OpensslPublicKeyEncrypt: invalid parameters");
        return -1;
    }

    // 加载公钥
    EVP_PKEY *pkey = EVP_PKEY_new();
    if (pkey == NULL) {
        LOG_Error("OpensslPublicKeyEncrypt: EVP_PKEY_new failed");
        return -1;
    }

    FILE *fp = fopen(pubKeyPath, "r");
    if (fp == NULL) {
        LOG_Error("OpensslPublicKeyEncrypt: failed to open public key file: %s", pubKeyPath);
        EVP_PKEY_free(pkey);
        return -1;
    }

    RSA *rsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL);
    fclose(fp);

    if (rsa == NULL) {
        LOG_Error("OpensslPublicKeyEncrypt: PEM_read_RSA_PUBKEY failed");
        EVP_PKEY_free(pkey);
        return -1;
    }

    if (EVP_PKEY_set1_RSA(pkey, rsa) != 1) {
        LOG_Error("OpensslPublicKeyEncrypt: EVP_PKEY_set1_RSA failed");
        RSA_free(rsa);
        EVP_PKEY_free(pkey);
        return -1;
    }
    RSA_free(rsa);

    // 使用 EVP 接口加密
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (ctx == NULL) {
        LOG_Error("OpensslPublicKeyEncrypt: EVP_PKEY_CTX_new failed");
        EVP_PKEY_free(pkey);
        return -1;
    }

    if (EVP_PKEY_encrypt_init(ctx) != 1) {
        LOG_Error("OpensslPublicKeyEncrypt: EVP_PKEY_encrypt_init failed");
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return -1;
    }

    // 设置填充方式（核心：与解密端一致）
    switch (padding_type) {
        case RSA_PADDING_PKCS1_V15:
            if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
                LOG_Error("Set PKCS1 v1.5 padding fail: %s", ERR_error_string(ERR_get_error(), NULL));
                EVP_PKEY_CTX_free(ctx);
                EVP_PKEY_free(pkey);
                return -1;
            }
            break;
        case RSA_PADDING_OAEP_SHA256:
            if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0 ||
                EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) <= 0 ||
                EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256()) <= 0) {
                LOG_Error("Set OAEP-SHA256 padding fail: %s", ERR_error_string(ERR_get_error(), NULL));
                EVP_PKEY_CTX_free(ctx);
                EVP_PKEY_free(pkey);
                return -1;
            }
            break;
        case RSA_PADDING_OAEP_SHA1:
            if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0 ||
                EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha1()) <= 0 ||
                EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha1()) <= 0) {
                LOG_Error("Set OAEP-SHA1 padding fail: %s", ERR_error_string(ERR_get_error(), NULL));
                EVP_PKEY_CTX_free(ctx);
                EVP_PKEY_free(pkey);
                return -1;
            }
            break;
        case RSA_PADDING_NO_PADDING:
            if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_NO_PADDING) <= 0) {
                LOG_Error("Set no padding fail: %s", ERR_error_string(ERR_get_error(), NULL));
                EVP_PKEY_CTX_free(ctx);
                EVP_PKEY_free(pkey);
                return -1;
            }
            break;
        default:
            LOG_Error("Unsupported padding type: %d", padding_type);
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            return -1;
    }

    // 执行加密
    size_t outlen_sz = (size_t)*outLen;
    if (EVP_PKEY_encrypt(ctx, out, &outlen_sz, in, inLen) != 1) {
        LOG_Error("OpensslPublicKeyEncrypt: EVP_PKEY_encrypt failed");
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return -1;
    }
    *outLen = (int)outlen_sz;

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);

    LOG_Info("OpensslPublicKeyEncrypt: success, encrypted %d bytes to %d bytes", inLen, *outLen);
    return 0;
}

/**
 * @brief 使用 RSA 公钥进行 SHA256withRSA 验签
 * @param pubKey 公钥（EVP_PKEY 类型）
 * @param data 原始数据
 * @param dataLen 原始数据长度
 * @param signature 签名数据
 * @param signatureLen 签名数据长度
 * @return 0: 验证成功，1: 验证失败（签名无效），其他: 参数错误或内部错误
 */
int OpensslRsaVerifyWithSHA256(EVP_PKEY* pubKey, const uint8_t *data, int dataLen, const uint8_t *signature, uint16_t signatureLen) 
{
    // 参数合法性检查
    if (pubKey == NULL || EVP_PKEY_id(pubKey) != EVP_PKEY_RSA) {
        LOG_Error("OpensslRsaVerifyWithSHA256: invalid public key or not RSA type");
        return -1;
    }
    if (data == NULL || dataLen <= 0) {
        LOG_Error("OpensslRsaVerifyWithSHA256: invalid data");
        return -2;
    }
    if (signature == NULL || signatureLen == 0) {
        LOG_Error("OpensslRsaVerifyWithSHA256: invalid signature");
        return -3;
    }

    EVP_MD_CTX* md_ctx = NULL;
    int ret = 0;

    // 创建验签上下文
    md_ctx = EVP_MD_CTX_new();
    if (md_ctx == NULL) {
        LOG_Error("OpensslRsaVerifyWithSHA256: EVP_MD_CTX_new failed");
        ret = -4;
        goto cleanup;
    }

    // 初始化验签（EVP_sha256 指定哈希算法，pubKey 为公钥）
    if (!EVP_DigestVerifyInit(md_ctx, NULL, EVP_sha256(), NULL, pubKey)) {
        LOG_Error("OpensslRsaVerifyWithSHA256: EVP_DigestVerifyInit failed");
        ret = -5;
        goto cleanup;   
    }

    // 传入原始数据
    if (!EVP_DigestVerifyUpdate(md_ctx, data, dataLen)) {
        LOG_Error("OpensslRsaVerifyWithSHA256: EVP_DigestVerifyUpdate failed");
        ret = -6;
        goto cleanup;
    }

    // 执行最终验签
    if (!EVP_DigestVerifyFinal(md_ctx, signature, signatureLen)) {
        LOG_Error("OpensslRsaVerifyWithSHA256: signature verification failed");
        ret = 1; 
        goto cleanup;
    }

    LOG_Info("OpensslRsaVerifyWithSHA256: signature verification succeeded");
    ret = 0;  

cleanup:
    if (md_ctx != NULL) {
        EVP_MD_CTX_free(md_ctx);
    }

    return ret;
}

/**
 * @brief 从 PEM 文件加载 RSA 公钥
 * @param pubKeyPath PEM 公钥文件路径
 * @param pubKey 输出参数，加载后的公钥
 * @return 0 成功，负数 失败
 */
 int OpensslLoadRsaPublicKey(const char *pubKeyPath, EVP_PKEY **pubKey)
 {
     FILE *keyFileFs = NULL;
 
     if (pubKeyPath == NULL || pubKey == NULL) {
         LOG_Error("OpensslLoadRsaPublicKey: invalid parameter");
         return -1;
     }
 
     keyFileFs = fopen(pubKeyPath, "r");
     if (!keyFileFs) {
         LOG_Error("OpensslLoadRsaPublicKey: unable to open public key file: %s", pubKeyPath);
         return -1;
     }
 
     *pubKey = PEM_read_PUBKEY(keyFileFs, NULL, NULL, NULL);
     fclose(keyFileFs);
 
     if (*pubKey == NULL) {
         LOG_Error("OpensslLoadRsaPublicKey: load public key failed: %s", ERR_error_string(ERR_get_error(), NULL));
         return -1;
     }
 
     if (EVP_PKEY_get_id(*pubKey) != EVP_PKEY_RSA) {
         LOG_Error("OpensslLoadRsaPublicKey: key is not RSA type");
         EVP_PKEY_free(*pubKey);
         *pubKey = NULL;
         return -1;
     }
 
     return 0;
 }


/**
 * @brief 将 RSA 私钥保存到 PEM 文件
 * @param privKey 私钥
 * @param filePath 保存路径
 * @return 0: 成功，负数: 失败
 */
int OpensslSaveRsaPrivateKey(EVP_PKEY *privKey, const char *filePath)
{
    if (privKey == NULL || filePath == NULL) {
        LOG_Error("OpensslSaveRsaPrivateKey: invalid parameter");
        return -1;
    }

    FILE *fp = fopen(filePath, "w");
    if (fp == NULL) {
        LOG_Error("OpensslSaveRsaPrivateKey: failed to open file %s", filePath);
        return -2;
    }

    if (!PEM_write_PrivateKey(fp, privKey, NULL, NULL, 0, NULL, NULL)) {
        LOG_Error("OpensslSaveRsaPrivateKey: PEM_write_PrivateKey failed");
        fclose(fp);
        return -3;
    }

    fclose(fp);
    LOG_Info("OpensslSaveRsaPrivateKey: saved to %s", filePath);
    return 0;
}

/**
 * @brief 从私钥导出公钥，并把公钥保存到 PEM 文件
 * @param privKey 私钥
 * @param filePath 保存路径
 * @return 0: 成功，负数: 失败
 */
int OpensslSaveRsaPublicKey(EVP_PKEY *privKey, const char *filePath)
{
    if (privKey == NULL || filePath == NULL) {
        LOG_Error("OpensslSaveRsaPublicKey: invalid parameter");
        return -1;
    }

    // 从私钥导出公钥
    EVP_PKEY *pubKey = NULL;
    int ret = OpensslExtractPublicKey(privKey, &pubKey);
    if (ret != 0 || pubKey == NULL) {
        LOG_Error("OpensslSaveRsaPublicKey: failed to extract public key");
        return -2;
    }

    FILE *fp = fopen(filePath, "w");
    if (fp == NULL) {
        LOG_Error("OpensslSaveRsaPublicKey: failed to open file %s", filePath);
        EVP_PKEY_free(pubKey);
        return -3;
    }

    if (!PEM_write_PUBKEY(fp, pubKey)) {
        LOG_Error("OpensslSaveRsaPublicKey: PEM_write_PUBKEY failed");
        fclose(fp);
        EVP_PKEY_free(pubKey);
        return -4;
    }

    fclose(fp);
    EVP_PKEY_free(pubKey);
    LOG_Info("OpensslSaveRsaPublicKey: saved to %s", filePath);
    return 0;
}

/**
 * @brief 从私钥导出公钥
 * @param privKey 私钥
 * @param pubKey 输出参数，导出的公钥
 * @return 0: 成功，负数: 失败
 */
int OpensslExtractPublicKey(EVP_PKEY *privKey, EVP_PKEY **pubKey)
{
    if (privKey == NULL || pubKey == NULL) {
        LOG_Error("OpensslExtractPublicKey: invalid parameter");
        return -1;
    }

    *pubKey = EVP_PKEY_dup(privKey);
    if (*pubKey == NULL) {
        LOG_Error("OpensslExtractPublicKey: EVP_PKEY_dup failed");
        return -2;
    }

    return 0;
}

 /**
 * @brief 从RSA秘钥中导出e和n
 * @param pkey 输入参数，RSA私钥或公钥
 * @param eBuf 输出参数，导出的e值（4字节大端，无前导零）
 * @param nBuf 输出参数，导出的n值缓冲区（大端，无前导零）
 * @param nLen nBuf缓冲区大小（输入）
 * @return 0: 成功，-1: 失败
 */
int OpensslExtracRsaPubKey(EVP_PKEY *pkey, uint8_t *outBuf, uint32_t *outLen)
{
    const BIGNUM *bn_n = NULL, *bn_e = NULL;
    int ret = 0;
    RSA *rsa = NULL;
    int nbytes = 0, ebytes = 0;
    uint32_t offset = 0;

    do {
        if (!pkey || !outBuf || !outLen) {
            ret = -1;
            break;
        }

        if (EVP_PKEY_id(pkey) != EVP_PKEY_RSA) {
            ret = -1;
            break;
        }

        rsa = (RSA*)EVP_PKEY_get0_RSA(pkey);
        if (!rsa) {
            ret = -1;
            break;
        }

        RSA_get0_key(rsa, &bn_n, &bn_e, NULL);
        if (!bn_n || !bn_e) {
            ret = -1;
            break;
        }

        nbytes = BN_num_bytes(bn_n);
        ebytes = BN_num_bytes(bn_e);

        uint32_t expectedLen = 4 + nbytes;
        if (*outLen < expectedLen) {
            *outLen = expectedLen;
            ret = -1;
            break;
        }

        if (ebytes > 4) {
            ret = -1;
            break;
        }
        memset(outBuf + offset, 0, 4);
        BN_bn2bin(bn_e, outBuf + offset + (4 - ebytes));
        offset += 4;

        BN_bn2bin(bn_n, outBuf + offset);
        offset += nbytes;

        *outLen = offset;
        ret = 0;
    } while(0);

    return ret;
}

int OpensslExtracRsaPriKey(EVP_PKEY *pkey, uint8_t *outBuf, uint32_t *outLen)
{
    const BIGNUM *bn_e = NULL, *bn_p = NULL, *bn_q = NULL;
    const BIGNUM *bn_dmp1 = NULL, *bn_dmq1 = NULL, *bn_iqmp = NULL;
    int ret = 0;
    RSA *rsa = NULL;
    int ebytes = 0, pbytes = 0, qbytes = 0;
    int dpbytes = 0, dqbytes = 0, invqbytes = 0;
    int halfLen = 0;
    uint32_t offset = 0;

    do {
        if (!pkey || !outBuf || !outLen) {
            ret = -1;
            break;
        }

        if (EVP_PKEY_id(pkey) != EVP_PKEY_RSA) {
            ret = -1;
            break;
        }

        rsa = (RSA*)EVP_PKEY_get0_RSA(pkey);
        if (!rsa) {
            ret = -1;
            break;
        }

        RSA_get0_key(rsa, NULL, &bn_e, NULL);
        RSA_get0_factors(rsa, &bn_p, &bn_q);
        RSA_get0_crt_params(rsa, &bn_dmp1, &bn_dmq1, &bn_iqmp);

        if (!bn_e || !bn_p || !bn_q || !bn_dmp1 || !bn_dmq1 || !bn_iqmp) {
            ret = -1;
            break;
        }

        pbytes = BN_num_bytes(bn_p);
        qbytes = BN_num_bytes(bn_q);
        halfLen = pbytes > qbytes ? pbytes : qbytes;

        uint32_t expectedLen = 4 + 5 * halfLen;
        if (*outLen < expectedLen) {
            *outLen = expectedLen;
            ret = -1;
            break;
        }

        ebytes = BN_num_bytes(bn_e);
        if (ebytes > 4) {
            ret = -1;
            break;
        }
        memset(outBuf + offset, 0, 4);
        BN_bn2bin(bn_e, outBuf + offset + (4 - ebytes));
        offset += 4;

        memset(outBuf + offset, 0, halfLen);
        BN_bn2bin(bn_p, outBuf + offset + (halfLen - pbytes));
        offset += halfLen;

        memset(outBuf + offset, 0, halfLen);
        BN_bn2bin(bn_q, outBuf + offset + (halfLen - qbytes));
        offset += halfLen;

        dpbytes = BN_num_bytes(bn_dmp1);
        memset(outBuf + offset, 0, halfLen);
        BN_bn2bin(bn_dmp1, outBuf + offset + (halfLen - dpbytes));
        offset += halfLen;

        dqbytes = BN_num_bytes(bn_dmq1);
        memset(outBuf + offset, 0, halfLen);
        BN_bn2bin(bn_dmq1, outBuf + offset + (halfLen - dqbytes));
        offset += halfLen;

        invqbytes = BN_num_bytes(bn_iqmp);
        memset(outBuf + offset, 0, halfLen);
        BN_bn2bin(bn_iqmp, outBuf + offset + (halfLen - invqbytes));
        offset += halfLen;

        *outLen = offset;
        ret = 0;
    } while(0);

    return ret;
}

int OpensslExtracRsaKeyPair(EVP_PKEY *pkey, uint8_t *outBuf, uint32_t *outLen)
{
    const BIGNUM *bn_n = NULL, *bn_e = NULL;
    const BIGNUM *bn_p = NULL, *bn_q = NULL;
    const BIGNUM *bn_dmp1 = NULL, *bn_dmq1 = NULL, *bn_iqmp = NULL;
    int ret = 0;
    RSA *rsa = NULL;
    int nbytes = 0, ebytes = 0, pbytes = 0, qbytes = 0;
    int dpbytes = 0, dqbytes = 0, invqbytes = 0;
    int halfLen = 0;
    uint32_t offset = 0;

    do {
        if (!pkey || !outBuf || !outLen) {
            ret = -1;
            break;
        }

        if (EVP_PKEY_id(pkey) != EVP_PKEY_RSA) {
            ret = -1;
            break;
        }

        rsa = (RSA*)EVP_PKEY_get0_RSA(pkey);
        if (!rsa) {
            ret = -1;
            break;
        }

        RSA_get0_key(rsa, &bn_n, &bn_e, NULL);
        RSA_get0_factors(rsa, &bn_p, &bn_q);
        RSA_get0_crt_params(rsa, &bn_dmp1, &bn_dmq1, &bn_iqmp);

        if (!bn_n || !bn_e || !bn_p || !bn_q || !bn_dmp1 || !bn_dmq1 || !bn_iqmp) {
            ret = -1;
            break;
        }

        nbytes = BN_num_bytes(bn_n);
        pbytes = BN_num_bytes(bn_p);
        qbytes = BN_num_bytes(bn_q);
        halfLen = pbytes > qbytes ? pbytes : qbytes;

        uint32_t expectedLen = 4 + nbytes + 5 * halfLen;
        if (*outLen < expectedLen) {
            *outLen = expectedLen;
            ret = -1;
            break;
        }

        ebytes = BN_num_bytes(bn_e);
        if (ebytes > 4) {
            ret = -1;
            break;
        }
        memset(outBuf + offset, 0, 4);
        BN_bn2bin(bn_e, outBuf + offset + (4 - ebytes));
        offset += 4;

        BN_bn2bin(bn_n, outBuf + offset);
        offset += nbytes;

        memset(outBuf + offset, 0, halfLen);
        BN_bn2bin(bn_p, outBuf + offset + (halfLen - pbytes));
        offset += halfLen;

        memset(outBuf + offset, 0, halfLen);
        BN_bn2bin(bn_q, outBuf + offset + (halfLen - qbytes));
        offset += halfLen;

        dpbytes = BN_num_bytes(bn_dmp1);
        memset(outBuf + offset, 0, halfLen);
        BN_bn2bin(bn_dmp1, outBuf + offset + (halfLen - dpbytes));
        offset += halfLen;

        dqbytes = BN_num_bytes(bn_dmq1);
        memset(outBuf + offset, 0, halfLen);
        BN_bn2bin(bn_dmq1, outBuf + offset + (halfLen - dqbytes));
        offset += halfLen;

        invqbytes = BN_num_bytes(bn_iqmp);
        memset(outBuf + offset, 0, halfLen);
        BN_bn2bin(bn_iqmp, outBuf + offset + (halfLen - invqbytes));
        offset += halfLen;

        *outLen = offset;
        ret = 0;
    } while(0);

    return ret;
}

int OpensslExtracRsaKeyPairFromBuffer(const char *keyPemBuf, int keyPemSize, uint8_t *outBuf, uint32_t *outLen)
{
    int ret = -1;
    EVP_PKEY *pkey = NULL;

    do {
        if (!keyPemBuf || !outBuf || !outLen) {
            break;
        }

        ret = OpensslLoadRsaPrivateKeyFromBuffer(keyPemBuf, keyPemSize, &pkey);
        if (ret != 0 || pkey == NULL) {
            break;
        }

        ret = OpensslExtracRsaKeyPair(pkey, outBuf, outLen);
        if (ret != 0) {
            break;
        }
    }while(0);

    /** 可以不判断 NULL **/
    EVP_PKEY_free(pkey);
    pkey = NULL;

    return ret;
}

int OpensslRsaEncrypt(EVP_PKEY *pkey, int padding, const uint8_t *plaintext, int plaintextLen, uint8_t *ciphertext, int *ciphertextLen)
{
    int ret = -1;
    EVP_PKEY_CTX *ctx = NULL;

    do {
        if (!pkey || !plaintext || !ciphertext || !ciphertextLen || *ciphertextLen == 0) {
            break;
        }

        ctx = EVP_PKEY_CTX_new(pkey, NULL);
        if (!ctx)
            break;

        if (EVP_PKEY_encrypt_init(ctx) <= 0)
            break;

        if (EVP_PKEY_CTX_set_rsa_padding(ctx, padding) <= 0)
            break;

        if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) <= 0)
            break;

        if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256()) <= 0)
            break;

        if (EVP_PKEY_encrypt(ctx, (uint8_t *)ciphertext, (size_t*) ciphertextLen, (const uint8_t*)plaintext, (size_t)plaintextLen) <= 0)
            break;

        ret = 0;
    } while (0);

    if (ctx)
        EVP_PKEY_CTX_free(ctx);

    return ret;
}

int OpensslRsaDecrypt(EVP_PKEY *pkey, int padding, const uint8_t *ciphertext, int ciphertextLen, uint8_t *plaintext, int *plaintextLen)
{
    int ret = -1;
    EVP_PKEY_CTX *ctx = NULL;

    do {
        if (!pkey || !ciphertext || !plaintext || !plaintextLen || *plaintextLen == 0) {
            break;
        }

        ctx = EVP_PKEY_CTX_new(pkey, NULL);
        if (!ctx)
            break;

        if (EVP_PKEY_decrypt_init(ctx) <= 0)
            break;

        if (EVP_PKEY_CTX_set_rsa_padding(ctx, padding) <= 0)
            break;
        if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) <= 0)
            break;
        if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256()) <= 0)
            break;

        if (EVP_PKEY_decrypt(ctx, (uint8_t *)plaintext, (size_t*)plaintextLen, (const uint8_t *)ciphertext, (size_t)ciphertextLen) <= 0)
            break;

        ret = 0;
    } while (0);

    if (ctx)
        EVP_PKEY_CTX_free(ctx);

    return ret;
}

int OpensslRsaSign(EVP_PKEY *pkey, const uint8_t *plain, uint32_t plainLen, uint8_t *signBuf, uint32_t *signBufLen)
{
    uint8_t hash[SHA256_DIGEST_LENGTH] = {0};
    int ret = -1;

    const RSA *rsa = EVP_PKEY_get0_RSA(pkey);
    if (!rsa){
        return -1;
    }

    SHA256(plain, plainLen, hash);
    ret = RSA_sign(NID_sha256, hash, SHA256_DIGEST_LENGTH, signBuf, signBufLen, (RSA *)rsa);

    return ret ==1 ? 0 : -1;
}

int OpensslRsaVerify(EVP_PKEY *pkey, const uint8_t *plain, uint32_t plainLen,const uint8_t *signBuf, uint32_t signBufLen)
{
    uint8_t hash[SHA256_DIGEST_LENGTH] = {0};
    int ret = -1;

    const RSA *rsa = EVP_PKEY_get0_RSA(pkey);
    if (!rsa){
        return -1;
    }

    SHA256(plain, plainLen, hash);
    ret = RSA_verify(NID_sha256, hash, SHA256_DIGEST_LENGTH, signBuf, signBufLen, (RSA *)rsa);

    return ret ==1 ? 0 : -1;
}


int OpensslGetRandom(uint8_t* buf, int len)
{
    if (buf == NULL || len <= 0) {
        return -1;
    }
    if (RAND_bytes(buf, len) != 1) {
        LOG_Error("RAND_bytes failed: %s", ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }
    return 0;
}

int OpensslX509NameToString(X509_NAME *name, char *buf, size_t buf_size)
{
    if (name == NULL || buf == NULL || buf_size == 0) {
        return -1;
    }
    BIO *bio = BIO_new(BIO_s_mem());
    if (bio == NULL) {
        return -1;
    }
    int ret = X509_NAME_print_ex(bio, name, 0, XN_FLAG_ONELINE);
    if (ret < 0) {
        BIO_free(bio);
        return -1;
    }
    int len = BIO_read(bio, buf, (int)(buf_size - 1));
    BIO_free(bio);
    if (len < 0) {
        return -1;
    }
    buf[len] = '\0';
    return 0;
}
