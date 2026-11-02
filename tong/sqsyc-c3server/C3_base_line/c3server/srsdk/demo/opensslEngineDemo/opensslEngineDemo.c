#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sr_log.h"
#include "err_code.h"
#include "sr_engine.h"

int main()
{
    LOG_INIT("demo");
    //加载引擎
    ENGINE_Init_sirun();

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
    
    //注销
    ENGINE_DeInit_sirun();
}
