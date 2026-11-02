#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sr_log.h"
#include "err_code.h"
#include "sr_engine.h"

#define MAX_PATH 256

void printMenu() {
    printf("\n===== openssl函数选择 =====\n");
    printf("0. 密钥导入\n");
    printf("1. 密钥删除\n");
    printf("2. 获取公钥\n");
    printf("3. 签名验签测试\n");
    printf("4. 私钥加密 公钥解密测试\n");
    printf("5. 公钥加密 私钥解密测试\n"); 
    printf("6. 双向认证测试\n");   
    printf("9. 其他测试\n");
    printf("q. 退出程序\n");
    printf("============================\n");
}

int switch_key_pair()
{
    int value = 0;
    int result = 0;
    
    // 从终端获取int值
    printf("请输入操作使用的密钥对: ");
    result = scanf("%d", &value);
    
    // 检查输入是否成功
    if (result != 1) {
        printf("输入错误！请输入有效的整数。\n");
        // 清空输入缓冲区
        while (getchar() != '\n');
        return 1;
    }
    
    // 清空输入缓冲区（处理多余的字符）
    while (getchar() != '\n');
    
    // 判断是否为1或2
    if (value == 1) {
        printf("您输入的是1\n");
        return CA_KEY;
    } else if (value == 2) {
        printf("您输入的是2\n");
        return CLIENT_KEY;
    } else {
        printf("您输入的%d不是1或2\n", value);
        return -1; 
    }
    
    return 0;
}
void demo_import_rsa_private()
{
  unsigned char key_data[4096] = {0};
    char filename[MAX_PATH] = {0};
    FILE *fp = NULL;
    int import_ret = 0;
    int key = switch_key_pair();
    // 从终端获取文件路径
    printf("请输入密钥文件路径: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        fprintf(stderr, "错误: 读取输入失败\n");
        return -1;
    }
    
    // 移除可能的换行符
    size_t len = strlen(filename);
    if (len > 0 && filename[len-1] == '\n') {
        filename[len-1] = '\0';
    }
    if (len > 1 && filename[len-2] == '\r') {  // 处理Windows换行符
        filename[len-2] = '\0';
    }
    
    // 检查路径是否为空
    if (strlen(filename) == 0) {
        fprintf(stderr, "错误: 文件路径不能为空\n");
        return -1;
    }
    
    // 打开文件
    fp = fopen(filename, "rb");  // 使用二进制模式
    if (fp == NULL) {
        fprintf(stderr, "错误: 无法打开文件 '%s'\n", filename);
        return -1;
    }
    
    // 读取文件内容
    size_t bytes_read = fread(key_data, 1, sizeof(key_data) - 1, fp);
    fclose(fp);
    
    if (bytes_read == 0) {
        fprintf(stderr, "错误: 文件为空或读取失败\n");
        return -1;
    }
    
    // 确保以空字符结尾
    key_data[bytes_read] = '\0';
    
    // 调用导入函数
    import_ret = ImportRSAPrivate(key_data, bytes_read, key);
    
    if (import_ret != 0) {
        fprintf(stderr, "密钥导入失败，错误码: %d\n", import_ret);
        return import_ret;
    }
    
    printf("密钥从文件 '%s' 导入成功，读取了 %zu 字节\n", filename, bytes_read);
    return 0;
}

void demo_delete_rsa_private()
{
    int key = switch_key_pair();

    //测试密钥删除
    int delete_ret = DeleteRSAPrivate(key);
    printf("DeleteRSAPrivate ret: %d\n", delete_ret);
}

void demo_get_rsa_public()
{
    int key = switch_key_pair();
    EVP_PKEY *pkey = NULL;

    if(key == -1)
    {
        printf("无效的密钥选择\n");
        return;
    }
    if(key == CA_KEY)
    {
        ENGINE_Init_ca_sirun();
        pkey = ENGINE_load_private_key(ENGINE_by_id("engine-sirun-ca"), "key-ca", NULL, NULL);
        ENGINE_DeInit_ca_sirun();
    }
    else if(key == CLIENT_KEY)
    {
        ENGINE_Init_sirun();
        pkey = ENGINE_load_private_key(ENGINE_by_id("engine-sirun"), "key-client", NULL, NULL);
        ENGINE_DeInit_sirun();
    }

    if(pkey == NULL)
    {
        printf("获取公钥失败\n");
        return;
    }
    EVP_PKEY_free(pkey);
    pkey = NULL;
}

void demo_rsa_sign_data()
{
    int key = switch_key_pair();
    EVP_PKEY *pkey = NULL;

    if(key == -1)
    {
        printf("无效的密钥选择\n");
        return;
    }
    if(key == CA_KEY)
    { 
        ENGINE_Init_ca_sirun();
        pkey = ENGINE_load_private_key(ENGINE_by_id("engine-sirun-ca"), "key-ca", NULL, NULL);
    }
    else if(key == CLIENT_KEY)
    { 
        ENGINE_Init_sirun();
        pkey = ENGINE_load_private_key(ENGINE_by_id("engine-sirun"), "key-client", NULL, NULL);
    }
    if(pkey == NULL)
    {
        printf("获取公钥失败\n");
        return;
    }

    RSA *rsa = EVP_PKEY_get1_RSA(pkey);
    if (!rsa) {
        return 0; // 不是 RSA 密钥或获取失败
    }

    //签名/验证签名
    unsigned char plaintextsign[] = "Hello, World Hello, World 123456789 abcdefg sign-verify";
    unsigned char ciphertext1[1024]={0};
    unsigned int signlen = sizeof(ciphertext1);

    //签名
    RSA_signature(plaintextsign, sizeof(plaintextsign)-1, ciphertext1, &signlen, rsa);
    // 打印签名后的数据
    printf("sign data: ");
    for (int i = 0; i < signlen; i++)
    {
        printf("%02x ", ciphertext1[i]);
    }
    printf("\n");

    //验签
    int verify_ret = RSA_verifySignature(plaintextsign, sizeof(plaintextsign)-1, ciphertext1, signlen, rsa);
    printf("verify_ret:%d\n",verify_ret);

    if(key == CA_KEY)
    { 
        ENGINE_DeInit_ca_sirun();
    }
    else if(key == CLIENT_KEY)
    { 
        ENGINE_DeInit_sirun();
    }

    RSA_free(rsa);
    rsa = NULL;
    EVP_PKEY_free(pkey);

    pkey = NULL;
}


void demo_rsa_priv_enc()
{
    int key = switch_key_pair();
    EVP_PKEY *pkey = NULL;

    if(key == -1)
    {
        printf("无效的密钥选择\n");
        return;
    }
    if(key == CA_KEY)
    { 
        ENGINE_Init_ca_sirun();
        pkey = ENGINE_load_private_key(ENGINE_by_id("engine-sirun-ca"), "key-ca", NULL, NULL);
    }
    else if(key == CLIENT_KEY)
    { 
        ENGINE_Init_sirun();
        pkey = ENGINE_load_private_key(ENGINE_by_id("engine-sirun"), "key-client", NULL, NULL);
    }
    if(pkey == NULL)
    {
        printf("获取公钥失败\n");
        return;
    }

    RSA *rsa = EVP_PKEY_get1_RSA(pkey);
    if (!rsa) {
        return 0; // 不是 RSA 密钥或获取失败
    }

    unsigned char hash[32] = {
        0x3A, 0xF7, 0x9C, 0x1E, 0xD2, 0x8B, 0x5A, 0xE3,
        0x04, 0x7F, 0xC6, 0xB1, 0x29, 0x3D, 0x90, 0x5B,
        0x12, 0x89, 0xAD, 0x47, 0xF0, 0x2C, 0x6E, 0x81,
        0x38, 0x9F, 0xD4, 0x0B, 0x75, 0xA2, 0xE6, 0x1C
    };
    unsigned char output[256] = {0};

    // // 2. 进行PKCS#1 v1.5填充
    // int ret = RSA_padding_add_PKCS1_type_1(output, 256,
    //                                        digest_info, digest_info_len);
    memcpy(&output[256-32], hash, 32);
    unsigned char encdata[256]={0};
    unsigned int encnlen = sizeof(encdata);
    // RSA私钥加密测试
    RSA_private_encrypt(sizeof(output), output, encdata, rsa, RSA_NO_PADDING);

    // 打印签名后的数据
    printf("enc data: ");
    for (int i = 0; i < encnlen; i++)
    {
        printf("%02x ", encdata[i]);
    }
    printf("\n");

    // RSA公钥解密测试
    unsigned char decdata[256]={0};
    unsigned int decnlen = sizeof(decdata);
    RSA_public_decrypt(encnlen, encdata, decdata, rsa, RSA_PKCS1_PADDING);

    // 打印解密后的数据
    printf("dec data: ");
    for (int i = 0; i < decnlen; i++)
    {
        printf("%02x ", decdata[i]);
    }
    printf("\n");

    if(key == CA_KEY)
    { 
        ENGINE_DeInit_ca_sirun();
    }
    else if(key == CLIENT_KEY)
    { 
        ENGINE_DeInit_sirun();
    }

    RSA_free(rsa);
    rsa = NULL;
    EVP_PKEY_free(pkey);

    pkey = NULL;
}


void demo_rsa_pub_enc()
{
    int key = switch_key_pair();
    EVP_PKEY *pkey = NULL;

    if(key == -1)
    {
        printf("无效的密钥选择\n");
        return;
    }
    if(key == CA_KEY)
    { 
        ENGINE_Init_ca_sirun();
        pkey = ENGINE_load_private_key(ENGINE_by_id("engine-sirun-ca"), "key-ca", NULL, NULL);
    }
    else if(key == CLIENT_KEY)
    { 
        ENGINE_Init_sirun();
        pkey = ENGINE_load_private_key(ENGINE_by_id("engine-sirun"), "key-client", NULL, NULL);
    }
    if(pkey == NULL)
    {
        printf("获取公钥失败\n");
        return;
    }

    RSA *rsa = EVP_PKEY_get1_RSA(pkey);
    if (!rsa) {
        return 0; // 不是 RSA 密钥或获取失败
    }
    
    // 使用引擎进行 RSA 加密
    unsigned char plaintext[] = "Hello, World Hello, World 123456789 abcdefg en-de";
    unsigned char ciphertext[256]={0};
    unsigned char plaintext1[256]={0};
    int len = RSA_public_encrypt(sizeof(plaintext)-1, plaintext, ciphertext, rsa, RSA_PKCS1_PADDING);//调用 my_rsa_pub_enc
    // 打印加密后的数据
    printf("Encrypted data: ");
    for (int i = 0; i < len; i++)
    {
        printf("%02x ", ciphertext[i]);
    }
    printf("\n");
    len=RSA_private_decrypt(len, ciphertext, plaintext1, rsa, RSA_PKCS1_PADDING);//调用 my_rsa_priv_dec

    // 打印解密后的数据
    printf("decrypted data: ");
    for (int i = 0; i < len; i++) {
        printf("%02x ", plaintext1[i]);
    }
    printf("\n");
    printf("ciphertext=%s\n",plaintext1);


    if(key == CA_KEY)
    { 
        ENGINE_DeInit_ca_sirun();
    }
    else if(key == CLIENT_KEY)
    { 
        ENGINE_DeInit_sirun();
    }

    RSA_free(rsa);
    rsa = NULL;
    EVP_PKEY_free(pkey);

    pkey = NULL;
}

void print_connection_ciphers(SSL *ssl) {
    if (!ssl) return;
    
    // 获取当前使用的加密套件
    const SSL_CIPHER *cipher = SSL_get_current_cipher(ssl);
    if (cipher) {
        // 基本算法信息 [citation:3][citation:8]
        LOG_Info("=== 当前连接使用的算法 ===");
        LOG_Info("Cipher: %s", SSL_CIPHER_get_name(cipher));
        LOG_Info("Protocol: %s", SSL_get_cipher_version(ssl));
        
        // 获取算法位数
        int alg_bits;
        int bits = SSL_CIPHER_get_bits(cipher, &alg_bits);
        LOG_Info("Secret bits: %d, Algorithm bits: %d", bits, alg_bits);
        
        // 详细的算法分解 [citation:3]
        LOG_Info("Key Exchange: %s", 
                 SSL_CIPHER_get_kx_nid(cipher) != NID_undef ? 
                 OBJ_nid2sn(SSL_CIPHER_get_kx_nid(cipher)) : "unknown");
        
        LOG_Info("Authentication: %s", 
                 SSL_CIPHER_get_auth_nid(cipher) != NID_undef ? 
                 OBJ_nid2sn(SSL_CIPHER_get_auth_nid(cipher)) : "unknown");
        
        // 获取加密算法和MAC算法
        int cipher_nid = SSL_CIPHER_get_cipher_nid(cipher);
        int digest_nid = SSL_CIPHER_get_digest_nid(cipher);
        
        if (cipher_nid != NID_undef) {
            LOG_Info("Encryption: %s", OBJ_nid2sn(cipher_nid));
            // 检查是否为AEAD模式 [citation:3]
            if (SSL_CIPHER_is_aead(cipher)) {
                LOG_Info("AEAD mode (no separate MAC)");
            }
        }
        
        if (digest_nid != NID_undef) {
            LOG_Info("MAC: %s", OBJ_nid2sn(digest_nid));
        }
        
        // 获取握手使用的哈希算法 [citation:3]
        const EVP_MD *handshake_md = SSL_CIPHER_get_handshake_digest(cipher);
        if (handshake_md) {
            LOG_Info("Handshake digest: %s", EVP_MD_name(handshake_md));
        }
        
        // 完整的算法描述
        char desc[256];
        if (SSL_CIPHER_description(cipher, desc, sizeof(desc))) {
            // 去除换行符
            char *nl;
            if ((nl = strchr(desc, '\n')) != NULL) *nl = '\0';
            LOG_Info("Full description: %s", desc);
        }
    }
}

// 定义调试回调函数
void ssl_debug_callback(const SSL *ssl, int where, int ret) {
    const char *str;
    int w = where & ~SSL_ST_MASK;
    static int handshake_completed = 0;

    if(w & SSL_ST_CONNECT)
        str = "SSL_connect";
    else if(w & SSL_ST_ACCEPT)
        str = "SSL_accept";
    else
        str = "undefined";

    if(where & SSL_CB_LOOP) {
        LOG_Info("%s:%s", str, SSL_state_string_long(ssl));
    } else if(where & SSL_CB_ALERT) {
        const char *alert_type = SSL_alert_type_string_long(ret);
        const char *alert_desc = SSL_alert_desc_string_long(ret);
        
        LOG_Info("SSL alert: %s - %s", alert_type, alert_desc);
        
    } else if(where & SSL_CB_HANDSHAKE_DONE) {
        handshake_completed = 1;
        LOG_Info("Handshake completed");
        print_connection_ciphers(ssl); // 握手完成后打印算法信息
        // 握手完成后检查证书验证结果
        long verify_result = SSL_get_verify_result(ssl);
        if (verify_result != X509_V_OK) {
            LOG_Info("Certificate verification failed: %s",
                    X509_verify_cert_error_string(verify_result));
        }
    } else if(where & SSL_CB_EXIT) {
        LOG_Info("SSL_CB_EXIT ,ret = %d",ret);
        if(ret <= 0) {
            const char *state_str = SSL_state_string_long(ssl);
            LOG_Info("%s:SSL_CB_EXIT,error in %s", str, state_str);
        }
    }
}

void demo_rsa_connect()
{
   int key = switch_key_pair();
    EVP_PKEY *pkey = NULL;

    if(key == -1)
    {
        printf("无效的密钥选择\n");
        return;
    }
    if(key == CA_KEY)
    { 
        ENGINE_Init_ca_sirun();
        pkey = ENGINE_load_private_key(ENGINE_by_id("engine-sirun-ca"), "key-ca", NULL, NULL);
    }
    else if(key == CLIENT_KEY)
    { 
        ENGINE_Init_sirun();
        pkey = ENGINE_load_private_key(ENGINE_by_id("engine-sirun"), "key-client", NULL, NULL);
    }
    if(pkey == NULL)
    {
        printf("获取公钥失败\n");
        return;
    }


   // 初始化 OpenSSL（OpenSSL 1.0.2 必须手动调用）
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    int sockfd;
    struct sockaddr_in server_addr;
    SSL_CTX* ctx = NULL;
    SSL* ssl = NULL;
    int ret;

    // 创建 SSL 上下文 (客户端模式)
    ctx = SSL_CTX_new(TLSv1_2_method()); 
    if (!ctx) {
        fprintf(stderr, "Failed to create SSL context\n");
        ERR_print_errors_fp(stderr);
        return 1;
    }

    SSL_CTX_set1_sigalgs_list(ctx, "RSA+SHA256");

    SSL_CTX_set_info_callback(ctx, ssl_debug_callback);
    // 设置客户端证书
    if (SSL_CTX_use_certificate_file(ctx, "/oemapp/test/certificate.pem", SSL_FILETYPE_PEM) <= 0) {
        LOG_Error("SSL_CTX_use_certificate_file fail \n");
        return 1;
    }

        // 可选：设置CA证书以验证服务器证书
    if (SSL_CTX_load_verify_locations(ctx, "/oemapp/test/certificate_chain.pem", NULL) <= 0) {
        LOG_Error("SSL_CTX_load_verify_locations fail \n");
        return 1;
    }

    if (!SSL_CTX_use_PrivateKey(ctx, pkey))
    {
        EVP_PKEY_free(pkey);   // 使用失败，释放
        LOG_Error("Failed to use private key: %s",
        ERR_error_string(ERR_get_error(), NULL));
        pkey = NULL;
        return 1;
    }

    // 验证私钥是否与证书匹配
    if (SSL_CTX_check_private_key(ctx) != 1) {
        LOG_Error("SSL_CTX_check_private_key fail \n");
        return 1;
    }

    // 创建 TCP 套接字并连接到服务器
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        SSL_CTX_free(ctx);
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8083);
    server_addr.sin_addr.s_addr = inet_addr("14.103.161.194");

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to connect to server");
        close(sockfd);
        SSL_CTX_free(ctx);
        return 1;
    }

    printf("Connected to server\n");

    // 创建 SSL 对象并绑定到套接字
    ssl = SSL_new(ctx);
    if (!ssl) {
        fprintf(stderr, "Failed to create SSL object\n");
        ERR_print_errors_fp(stderr);
        close(sockfd);
        SSL_CTX_free(ctx);
        return 1;
    }

    SSL_set_fd(ssl, sockfd);


    // 执行 TLS 握手
    ret = SSL_connect(ssl);
    if (ret <= 0) {
        fprintf(stderr, "TLS handshake failed: %d\n", ret);
        ERR_print_errors_fp(stderr);

        // 获取详细的验证错误
        int verify_result = SSL_get_verify_result(ssl);
        printf("Certificate verification result: %s (code: %d)\n",
               X509_verify_cert_error_string(verify_result), verify_result);

        close(sockfd);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        return 1;
    }

    printf("TLS handshake successful\n");







    if(key == CA_KEY)
    { 
        ENGINE_DeInit_ca_sirun();
    }
    else if(key == CLIENT_KEY)
    { 
        ENGINE_DeInit_sirun();
    }


    EVP_PKEY_free(pkey);
    pkey = NULL;
}


int main()
{
    LOG_INIT("demo");
    //加载引擎

    char buffer[1024] = {0};
    char choice = 0;
    do {
        printMenu();
        printf("请选择功能（输入编号）： ");
        fgets(buffer, sizeof(buffer), stdin);
        choice = buffer[0];
        printf("int put choice:%c\n", choice);
        switch (choice) {
            case '0':
                {
                    demo_import_rsa_private();
                }
                break;
            case '1':
                {
                    demo_delete_rsa_private();
                }
                break;
            case '2':
                {
                    demo_get_rsa_public();
                }
                break;
            case '3':
                {
                    //签名测试
                    demo_rsa_sign_data();
                }
                break;
            case '4':
                {
                    //RSA私钥加密测试
                    demo_rsa_priv_enc();
                }
                break;
            case '5':
                {
                    //RSA公钥加密测试
                    demo_rsa_pub_enc();
                }
                break;
            case '6':
                {
                    //双向认证测试
                    demo_rsa_connect();
                }
                break;
            case '9':
                {
                    RSA *rsa = RSA_new();
                    BIGNUM *bn = BN_new();
                    //公钥指数 65537
                    BN_set_word(bn, RSA_F4);
                    RSA_generate_key_ex(rsa, 2048, bn, NULL);//调用 my_rsa_keygen
                    printf("RSA Key Pair:\n");
                    RSA_print_fp(stdout, rsa, 0);

                    //保存公钥文件
                    SaveRSAPubKeyFile(rsa,"/oemdata/test.pem");

                    RSA *rsa_load = NULL;
                    //读取公钥文件
                    LoadRSAPubKey(&rsa_load,"/oemdata/test.pem");
                    printf("RSA load Key Pair:\n");
                    RSA_print_fp(stdout, rsa_load, 0);

                    //生成csr文件
                    GenX509ReqFile(202403,"sirun.test1.test2.test3.test4","SIRUNTIANLANG INC. CERT","SIRUNTIANLANG","BeiJing","BeiJing","CN","jiangze.jia@sirun.net",rsa_load,"/oemdata/test.csr");

                    // 使用引擎进行 RSA 加密
                    unsigned char plaintext[] = "Hello, World Hello, World 123456789 abcdefg en-de";
                    unsigned char ciphertext[256]={0};
                    unsigned char plaintext1[256]={0};
                    int len = RSA_public_encrypt(sizeof(plaintext)-1, plaintext, ciphertext, rsa_load, RSA_PKCS1_PADDING);//调用 my_rsa_pub_enc
                    // 打印加密后的数据
                    printf("Encrypted data: ");
                    for (int i = 0; i < len; i++)
                    {
                        printf("%02x ", ciphertext[i]);
                    }
                    printf("\n");
                    len=RSA_private_decrypt(len, ciphertext, plaintext1, rsa_load, RSA_PKCS1_PADDING);//调用 my_rsa_priv_dec

                    // 打印解密后的数据
                    printf("decrypted data: ");
                    for (int i = 0; i < len; i++) {
                        printf("%02x ", plaintext1[i]);
                    }
                    printf("\n");
                    printf("ciphertext=%s\n",plaintext1);

                    //签名/验证签名
                    unsigned char plaintextsign[] = "Hello, World Hello, World 123456789 abcdefg sign-verify";
                    unsigned char ciphertext1[256]={0};
                    unsigned int signlen = 0;

                    //签名
                    RSA_signature(plaintextsign, sizeof(plaintextsign)-1, ciphertext1, &signlen, rsa_load);//my_rsa_sign
                    // 打印签名后的数据
                    printf("sign data: ");
                    for (int i = 0; i < signlen; i++)
                    {
                        printf("%02x ", ciphertext1[i]);
                    }
                    printf("\n");

                    //验签
                    int verify_ret = RSA_verifySignature(plaintextsign, sizeof(plaintextsign)-1, ciphertext1, signlen, rsa_load);//my_rsa_verify
                    printf("verify_ret:%d\n",verify_ret);
                }
                break;
            case 'q':
                printf("退出程序\n");
                break;
            default:
                printf("无效的选择，请重新输入。\n");
                break;            
        }                
    } while (choice != 'q');

}
