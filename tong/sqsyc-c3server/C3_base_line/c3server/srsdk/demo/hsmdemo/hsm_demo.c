#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define OPENSSL_API_COMPAT 10101

#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/x509.h>

#include "sr_log.h"
#include "sr_ipc.h"
#include "rpc_hsm_api.h"
#include "openssl_util.h"


#define HSM_DEMO_KEY_ID 0x82

#define DEMO_PKI_CRT    "-----BEGIN CERTIFICATE-----\n"  \
                        "MIIFLTCCBBWgAwIBAgIJAIkW2Hrg+8GUMA0GCSqGSIb3DQEBCwUAME8xDzANBgNV\n"  \
                        "BAMMBlJvb3RDQTELMAkGA1UEBhMCQ04xEDAOBgNVBAgMB0ppYW5nU3UxDTALBgNV\n"  \
                        "BAcMBFd1WGkxDjAMBgNVBAoMBVNpUnVuMB4XDTI2MDUxNTA4MDM1N1oXDTQ2MDUx\n"  \
                        "NTIzNTk1OVowWzEbMBkGA1UEAwwSVEJPWFRTSU1VTEFURVMwMDAyMQswCQYDVQQG\n"  \
                        "EwJDTjEQMA4GA1UECAwHSmlhbmdTdTENMAsGA1UEBwwEV3VYaTEOMAwGA1UECgwF\n"  \
                        "U2lSdW4wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCLtsqdLUuJbKOp\n"  \
                        "l+gCkPjlDEu8wm3VsettnAzOJwVte3aTZljbJhaTlhVopOQIZE7Uu0X+GxVUktAP\n"  \
                        "EqGvKuPSxtHhlu50PMuJbCu3wAx8LdyUxLExhUFCGMe5/ZJHzgKKiuFJLH6WQ0wt\n"  \
                        "Y44klLjfl7IZpZpP4N5vsVQUPGq8S5CSjOT9jnTtphJLmpUGgqm1lRDYF6pLWBLy\n"  \
                        "OOlnqOHFCeNa5T1qr3KLBglwqd80feJWZ+GCoIfeItsxRgjHYm2Pfypv8DumNzLE\n"  \
                        "gO3m+wxtzzyxCFzDRq1/SlWFPuBZsLOOTs6hB6oZcYRHjhdEKsUMQOtvHFjqWHSM\n"  \
                        "YXOdtM0VAgMBAAGjggH+MIIB+jAOBgNVHQ8BAf8EBAMCB4AwCQYDVR0TBAIwADAT\n"  \
                        "BgNVHSUEDDAKBggrBgEFBQcDAjCCAYYGA1UdEQSCAX0wggF5oIIBdQYIKwYBBAHg\n" \
                        "OQGgggFnDIIBY3VuaXF1ZU5hbWU6U0p3c3k3cmRxOFhyYkpRTERzTFR6aE12V29Y\n" \
                        "TkRRT0JIcS85L2N6Si81clFwU1pDNEowVG0yV09JMWZEQnlZNW9kTU9DYWVpYmZO\n" \
                        "ajVoUHVRek1TNmdWazhEaU8zRzZJODBkWWxObDF6OVVVMm9QNm40WmhLM3pFOUg2\n" \
                        "c3JUNEhoeVhERUliemhnZkxHWDVhbEdOTFdZZGU3U1Y2SUhreVoxU08rMkNKRHNo\n" \
                        "UFB4WWF3MXhac2x1MnFmby9mS0dPcFBpd25iam5iTllvZlRHZDdZM1Bhd2lzS1I5\n" \
                        "ajNQU2YwZWRUbVJZZExSckNBOGtIcXdZa2FBVlNxM1NHc2dTSzNuRTJDb3NxQWNO\n" \
                        "MTlIbEVNV3ZEb3MxV0VDRWlsdjNiUEZ1RG5jQnNDcWRWM1RVTWtzeWFmcEVZK3gy\n" \
                        "dFUrVjg5YktWVnZIOFhUZjZkUGdRNC80YlhRPT0wHQYDVR0OBBYEFLf7V+W5rWDk\n" \
                        "bjNsIf4QbVUsibkeMB8GA1UdIwQYMBaAFFxIk1bsP2G7DIFPvL2y9ahcNS5vMA0G\n" \
                        "CSqGSIb3DQEBCwUAA4IBAQB+yplQ8db+3MvYuEosd5GhOb1GZXc2g3VufptipL4X\n" \
                        "N19sjZEiDq7IysJZ/MTkRNqZPAsdaXvokKkQTSJ3UO7jgi7LzRiCfjd3ediZzMy4\n" \
                        "z7XBuokBAFnErVG0ephNtQROlxEVGBq1R58qCFdpv3fFqfLICZchbH4OvqkFi8Ui\n" \
                        "spwYefxKaoHQeUKnl0E4rLwBAHfVWbYeuoyHDMBPqvDtRR1zsu45bLJrlPXs/IfZ\n" \
                        "LvFkr+rt55X8EKENBbwchokXaUqnz9HYd2c3FvtsuaXwADGI0dYu1YqX48d8AybU\n" \
                        "1o+MuKg1yGGmvXXhd/AMiNTBJ3cttE8/xz3fJXAYf8Kv\n" \
                        "-----END CERTIFICATE-----\n"


#define DEMO_PKI_KEY    "-----BEGIN PRIVATE KEY-----\n" \
                        "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCLtsqdLUuJbKOp\n" \
                        "l+gCkPjlDEu8wm3VsettnAzOJwVte3aTZljbJhaTlhVopOQIZE7Uu0X+GxVUktAP\n" \
                        "EqGvKuPSxtHhlu50PMuJbCu3wAx8LdyUxLExhUFCGMe5/ZJHzgKKiuFJLH6WQ0wt\n" \
                        "Y44klLjfl7IZpZpP4N5vsVQUPGq8S5CSjOT9jnTtphJLmpUGgqm1lRDYF6pLWBLy\n" \
                        "OOlnqOHFCeNa5T1qr3KLBglwqd80feJWZ+GCoIfeItsxRgjHYm2Pfypv8DumNzLE\n" \
                        "gO3m+wxtzzyxCFzDRq1/SlWFPuBZsLOOTs6hB6oZcYRHjhdEKsUMQOtvHFjqWHSM\n" \
                        "YXOdtM0VAgMBAAECggEAFe1OuENLFxrPRscZ+QkinuQCoPHMxhyBy/aRRet23Rti\n" \
                        "3ThTWrWMFoaX/o/eX/jKLHXlwr8iZl8SO0Nbz7+F+OcOOn7zFpgSA5PP+qsv6m9S\n" \
                        "LqD+BD0bJFH6SpTi19Ubrf2k5NSQW2ndc0QsI06aAzFuGLtWUBIFJvdoMDLZN4rT\n" \
                        "Fdm+luKvChuWof4CphCfP6ngPBU4vSvezeGBGzTdUtAdl5FXEZayjYtj91jYxTkE\n" \
                        "AjPm0//TNL5jbVoh0m9E0tM+CGr/E7cPdKJ0cQSgZ4JmVET+7jV6pAeQEZWwj/DD\n" \
                        "7HaT+opXygMbF5ZnYN66kTt2Rwns/iVdRcwyjAkDQQKBgQDDKJHktc+IqB+2wwIQ\n" \
                        "gHDf5nSLLKxBFvPjQ40b8yBHbUGcZMUlOZxyTxJkZ2EnDcJvUN0Huei9qnIVtsz/\n" \
                        "LYW3+FyClsF/2NwupZ1kSlMecu/J7XYPWeaE9rZ9jKnio0g9fVAA9cGAH+82tWPL\n" \
                        "KWs6IXEGnBXP1TReWFrOjcH/NQKBgQC3RT95LWTAH3pqxr22HS3ExjnZgrbK/2+5\n" \
                        "HYiFqz3CfC9CbU9bKI4wAN2eBiPkfXPGRXK7a+OaQDHxc7a/F8mv+bh+UP/awmNp\n" \
                        "eljgc6YjtntMHnJzj96M7Lo4fQyn2nCUGirCcONR5p7caz0ApTru9g+1xVf/auBH\n" \
                        "Dp4wE4fyYQKBgQC1oI9PHMn99AI39Ht6Yh8qse8rC7NwjG0EyHyOU5JqB0u1wq8b\n" \
                        "+ucYF3EVkoT/dLXNJaPfqezqf7PXc56LqpE1a2alfqqtUmVw4F9/1BBRWUTDWEW0\n" \
                        "+cUpX/btF7mQHMiBuZt4mT+x/4aBEwXCMVUC79aiASfnmhQ37eDtNU8kNQKBgFiE\n" \
                        "DJ0VTJTyXyjVJmmf/CwO5X/lfy3aY3aQChEtXN3jJtw5eVt44fvTXzZkls9TvCgm\n" \
                        "ALz+5eXpXXXVwozXC2WAAhiYPIrG2kbmfaiikrqq9zATgInKEd4Us/h2hTdI7JjL\n" \
                        "bp2M8m3rKg0hikwWgblKhNnxH4hQcVRAfiyQMlfhAoGAU7hBDbr+i8Y4gL/baZcG\n" \
                        "bQnjET4segcCO2TwcoXIqcOmzUMstVIFl0OGr+mCax9hwiJhlTJ6Jxj36ONNQYVC\n" \
                        "LbtdASWO+XF67qvOHYbRoAYC9bZO8LIZvLP+czNRxSK+mp2bQXGf6g+xr/G6aUXi\n" \
                        "o2oJi0yfnhBh0k1P3itnLPg=\n" \
                        "-----END PRIVATE KEY-----\n"




static void usage(const char *argv0)
{
    printf("用法: %s [选项]\n", argv0);
    printf("选项:\n");
    printf("  -h              显示帮助信息\n");
    printf("  -t <测试编号>    运行指定测试\n");
    printf("  0 - 通过RPC调用服务器端（需要hsm运行）\n");
    printf("  1 - 查询HSM中密钥信息（需要hsm运行）\n");
    printf("  2 - HSM密钥生成（需要hsm运行）\n");
    printf("  3 - OPENSSL加密，HSM解密（需要hsm运行）\n");
    printf("  4 - HSM签名，OPENSSL验签（需要hsm运行）\n");
    printf("  5 - OPENSSL密钥生成\n");
    printf("  6 - HSM加密，OPENSSL解密（需要hsm运行）\n");
    printf("  7 - OPENSSL签名，HSM验签（需要hsm运行）\n");
    printf("  8 - 导入导出公钥（需要hsm运行）\n");
    printf("  9 - 导入私钥（需要hsm运行）\n");
    printf("  10 - 导入秘钥对（需要hsm运行）\n");
    printf("  100 keyID - 删除秘钥\n");

    printf("  101 - 导入导出证书（需要hsm运行）\n");
    printf("  102 - 查询证书（需要hsm运行）\n");
    printf("  200 crtId - 删除证书（需要hsm运行）\n");    


    printf("  1000 - 查询PKI证书和秘钥安装状态（需要hsm运行）\n");
    printf("  1001 - 安装PKI证书和秘钥（需要hsm运行）\n");
    printf("  1002 - 清除PKI证书和秘钥安装（需要hsm运行）\n");
    printf("  1003 crtSubjectCN - PKI证书和秘钥自检（需要hsm运行）\n");

    printf("  3000 - 获取HSM工作状态（需要hsm运行）\n");

}

static void StringToByte(char* source, uint8_t* dest, int sourceLen)
{
	int i = 0;
	uint8_t highByte, lowByte;

	for (i = 0; i < sourceLen; i += 2)
	{
		highByte = toupper(source[i]);
		lowByte  = toupper(source[i + 1]);

		if (highByte > 0x39)
			highByte -= 0x37;
		else
			highByte -= 0x30;

		if (lowByte > 0x39)
			lowByte -= 0x37;
		else
			lowByte -= 0x30;

		dest[i / 2] = (highByte << 4) | lowByte;
	}
	return ;
}

static void print_hex(const char *prefix, const uint8_t *data, int len)
{
    if (prefix) printf("%s", prefix);
    for (int i = 0; i < len; i++) printf("%02X", data[i]);
    printf("\n");
}

static const char* key_alg_to_string(HsmKeyAlg_e alg)
{
    switch (alg) {
        case E_HSM_KEY_ALG_RSA:   return "RSA";
        case E_HSM_KEY_ALG_SM2:   return "SM2";
        case E_HSM_KEY_ALG_ECC:   return "ECC";
        case E_HSM_KEY_ALG_AES:   return "AES";
        case E_HSM_KEY_ALG_SM4:   return "SM4";
        case E_HSM_KEY_ALG_3DES:  return "3DES";
        default:                  return "UNKNOWN";
    }
}

static const char* key_type_to_string(HsmKeyType_e type)
{
    switch (type) {
        case E_HSM_KEY_TYPE_PUBLIC:   return "PUBLIC";
        case E_HSM_KEY_TYPE_PRIVATE:  return "PRIVATE";
        case E_HSM_KEY_TYPE_KEY_PAIR: return "KEY_PAIR";
        case E_HSM_KEY_TYPE_SECRET:   return "SECRET";
        default:                      return "UNKNOWN";
    }
}

static void print_key_list(QueryKeyResult_t *result)
{
    if (result == NULL || result->validKeyCount == 0) return;
    printf("  validKeyCount=%u\n", result->validKeyCount);
    for (int i = 0; i < result->validKeyCount; i++) {
        KeyInfo_t *k = &result->keyList[i];
        printf("  KeyId=0x%02X: alg=%s, type=%s, bits=%u\n",
               k->keyId, key_alg_to_string(k->keyAlg), key_type_to_string(k->keyType), k->keyBits);
    }
}

// RPC 模式测试函数
static void test_hsm_api_full(void)
{
    int ret = -1;
    uint8_t pubKeyData[512] = {0};
    uint32_t pubKeyDataBufSize = sizeof(pubKeyData);
    uint8_t signData[] = "Data to be signed";
    uint8_t signature[256] = {0};
    uint32_t signLen = sizeof(signature);
    uint8_t plaintext[] = "Hello, HSM! This is a test message for RSA encryption.";
    uint8_t ciphertext[1024] = {0};
    uint32_t cipherLen = sizeof(ciphertext);
    uint8_t decResult[256] = {0};
    uint32_t decLen = sizeof(decResult);
    uint8_t randomBuf[4096] = {0};
    uint32_t testLen = 2048;
    QueryKeyResult_t keyResult = {0};
    uint8_t exportBuf[1024] = {0};
    uint32_t exportBufLen = sizeof(exportBuf);
    char s_appkey[1300]={0};
    uint8_t importBuf[1024] = {0};
    uint32_t s_appkey_len = 0;
    EVP_PKEY *pubKey = NULL;
    uint8_t *importBufDer = NULL;
    int importBufDerLen = 0;
    printf("\n========== 测试: HSM API完整流程 (RPC模式) ==========\n");

    // 测试密钥对生成 (返回 X.509 SubjectPublicKeyInfo DER)
    printf("\n[1/10] 测试 RpcHsmGenKey()...\n");
    ret = RpcHsmGenKey(E_ALG_RSA2048, HSM_DEMO_KEY_ID, pubKeyData, &pubKeyDataBufSize);
    if (ret != 0) { printf("失败! ret=%d\n", ret); return; }
    print_hex("pubKeyData (X.509 DER HEX): ", pubKeyData, (int)pubKeyDataBufSize);
    printf("GenKey 成功! pubKeyDataBufSize=%u\n", pubKeyDataBufSize);

    // 测试签名
    printf("\n[2/10] 测试 RpcHsmSign()...\n");
    ret = RpcHsmSign(E_ALG_RSA2048, HSM_DEMO_KEY_ID, E_DIGIST_SHA256, signData, sizeof(signData) - 1, signature, &signLen);
    if (ret != 0) { printf("失败! ret=%d\n", ret); return; }
    print_hex("signature (HEX): ", signature, (int)signLen);
    printf("Sign 成功! signLen=%u\n", signLen);

    // 测试验签
    printf("\n[3/10] 测试 RpcHsmVerify()...\n");
    ret = RpcHsmVerify(E_ALG_RSA2048, HSM_DEMO_KEY_ID, E_DIGIST_SHA256, signData, sizeof(signData) - 1, signature, signLen);
    if (ret != 0) { printf("失败! ret=%d\n", ret); return; }
    printf("Verify 成功!\n");

    // 测试加密
    printf("\n[4/10] 测试 RpcHsmEncrypt()...\n");
    ret = RpcHsmEncrypt(E_ALG_RSA2048, HSM_DEMO_KEY_ID, E_RSA_SIGN_PADDING_PKCS1, plaintext, (uint32_t)sizeof(plaintext) - 1, ciphertext, &cipherLen);
    if (ret != 0) { printf("失败! ret=%d\n", ret); return; }
    print_hex("ciphertext (HEX): ", ciphertext, (int)cipherLen);
    printf("Encrypt 成功! cipherLen=%u\n", cipherLen);

    // 测试解密
    printf("\n[5/10] 测试 RpcHsmDecrypt()...\n");
    ret = RpcHsmDecrypt(E_ALG_RSA2048, HSM_DEMO_KEY_ID, E_RSA_SIGN_PADDING_PKCS1, ciphertext, cipherLen, decResult, &decLen);
    if (ret != 0 || decLen != sizeof(plaintext) - 1 || memcmp(decResult, plaintext, decLen) != 0)
    { printf("失败! ret=%d\n", ret); return; }
    printf("解密内容 (ASCII): %.*s\n", decLen, decResult);
    printf("Decrypt 成功! decLen=%u\n", decLen);

    // 随机数测试
    printf("\n[6/10] 测试 RpcHsmRandom()...\n");
    ret = RpcHsmRandom(testLen, randomBuf);
    if (ret != 0) { printf("随机数生成失败! ret=%d\n", ret); return; }
    print_hex("random (HEX): ", randomBuf, (int)testLen);
    printf("随机数生成成功! len=%u\n", testLen);

    // 密钥查询测试
    printf("\n[7/10] 测试 RpcHsmQueryKey()...\n");
    ret = RpcHsmQueryKey(&keyResult);
    if (ret != 0) { printf("密钥查询失败! ret=%d\n", ret); return; }
    printf("密钥查询成功! validKeyCount=%u\n", keyResult.validKeyCount);
    print_key_list(&keyResult);

    // 密钥导出测试 (返回 X.509 SubjectPublicKeyInfo DER)
    printf("\n[8/10] 测试 RpcHsmExportKey()...\n");
    ret = RpcHsmExportKey(E_ALG_RSA2048, HSM_DEMO_KEY_ID, E_KEY_TYPE_PUB, exportBuf, &exportBufLen);
    if (ret != 0) { printf("密钥导出失败! ret=%d\n", ret); return; }
    print_hex("exportBuf (X.509 DER HEX): ", exportBuf, (int)exportBufLen);
    printf("密钥导出成功! exportBufLen=%u\n", exportBufLen);

    // 密钥删除测试
    printf("\n[9/10] 测试 RpcHsmDeleteKey()...\n");
    ret = RpcHsmDeleteKey(HSM_DEMO_KEY_ID);
    if (ret != 0) { printf("密钥删除失败! ret=%d\n", ret); return; }
    printf("密钥删除成功!\n");
    printf("密钥删除后查询密钥:\n");
    ret = RpcHsmQueryKey(&keyResult);
    if (ret != 0) { printf("密钥查询失败! ret=%d\n", ret); return; }
    printf("密钥查询成功! 有效密钥数量: %u\n", keyResult.validKeyCount);    
    print_key_list(&keyResult);

    // 密钥导入测试 (导入 X.509 SubjectPublicKeyInfo DER)
    printf("\n[10/10] 测试 RpcHsmImportKey()...\n");

    // 优先从 /oemdata/openssl_public.pem 读取公钥并转换为 X.509 DER
    ret = OpensslLoadRsaPublicKey("/oemdata/openssl_public.pem", &pubKey);
    if (ret == 0 && pubKey != NULL) {
        importBufDerLen = i2d_PUBKEY(pubKey, &importBufDer);
        EVP_PKEY_free(pubKey);
        if (importBufDerLen <= 0) {
            printf("公钥转换为 X.509 DER 失败!\n");
            OPENSSL_free(importBufDer);
            return;
        }
        printf("从 /oemdata/openssl_public.pem 加载并转换为 X.509 DER 公钥, 长度=%d\n", importBufDerLen);
    } else {
        // fallback: 使用硬编码的 X.509 DER 公钥
        strcpy(s_appkey, "30820122300d06092a864886f70d01010105000382010f003082010a02820101008bb6ca9d2d4b896ca3a997e80290f8e50c4bbcc26dd5b1eb6d9c0cce27056d7b76936658db261693961568a4e408644ed4bb45fe1b155492d00f12a1af2ae3d2c6d1e196ee743ccb896c2bb7c00c7c2ddc94c4b13185414218c7b9fd9247ce028a8ae1492c7e96434c2d638e2494b8df97b219a59a4fe0de6fb154143c6abc4b90928ce4fd8e74eda6124b9a950682a9b59510d817aa4b5812f238e967a8e1c509e35ae53d6aaf728b060970a9df347de25667e182a087de22db314608c7626d8f7f2a6ff03ba63732c480ede6fb0c6dcf3cb1085cc346ad7f4a55853ee059b0b38e4ecea107aa197184478e17442ac50c40eb6f1c58ea58748c61739db4cd150203010001");
        s_appkey_len = strlen(s_appkey);
        importBufDerLen = s_appkey_len / 2;
        importBufDer = importBuf;
        StringToByte(s_appkey, importBufDer, s_appkey_len);
        printf("使用硬编码 X.509 DER fallback 公钥, 长度=%d\n", importBufDerLen);
    }

    ret = RpcHsmImportKey(E_ALG_RSA2048, HSM_DEMO_KEY_ID, E_KEY_TYPE_PUB, importBufDer, (uint32_t)importBufDerLen);
    if (importBufDer != importBuf) OPENSSL_free(importBufDer);
    if (ret != 0) { printf("密钥导入失败! ret=%d\n", ret); return; }
    printf("密钥导入成功!\n");
    printf("密钥导入后查询密钥:\n");
    ret = RpcHsmQueryKey(&keyResult);
    if (ret != 0) { printf("密钥查询失败! ret=%d\n", ret); return; }
    printf("密钥查询成功! 有效密钥数量: %u\n", keyResult.validKeyCount);    
    print_key_list(&keyResult);

    printf("\n========== HSM API测试完成 ==========\n");
}

static void test_hsm_query_key(void)
{
    QueryKeyResult_t keyResult = {0};
    int ret = 0;
    ret = RpcHsmQueryKey(&keyResult);
    if (ret != 0) { printf("密钥查询失败! ret=%d\n", ret); return; }
    printf("密钥查询成功! validKeyCount=%u\n", keyResult.validKeyCount);
    print_key_list(&keyResult);
}

static void test_gen_key(void)
{
    int ret = 0;
    // 测试密钥对生成 (返回 X.509 SubjectPublicKeyInfo DER)
    uint8_t pubKeyData[512] = {0};
    uint32_t pubKeyDataBufSize = sizeof(pubKeyData);
    EVP_PKEY *pubKey = NULL;
    printf("\n 测试 RpcHsmGenKey()...\n");
    ret = RpcHsmGenKey(E_ALG_RSA2048, 0x83, pubKeyData, &pubKeyDataBufSize);
    if (ret != 0) { printf("失败! ret=%d\n", ret); return; }
    print_hex("pubKeyData (X.509 DER HEX): ", pubKeyData, (int)pubKeyDataBufSize);
    printf("GenKey 成功! pubKeyDataBufSize=%u\n", pubKeyDataBufSize);

    // 保存公钥到文件 (从 X.509 DER 解析后直接写入 PEM)
    uint8_t *p = pubKeyData;
    pubKey = d2i_PUBKEY(NULL, (const unsigned char **)&p, (long)pubKeyDataBufSize);
    if (pubKey) {
        FILE *file = fopen("/oemdata/test_pub.pem", "w");
        if (file) {
            if (PEM_write_PUBKEY(file, pubKey)) {
                printf("公钥已保存到 /oemdata/test_pub.pem\n");
            } else {
                printf("保存公钥到 PEM 失败!\n");
            }
            fclose(file);
        } else {
            printf("打开文件失败!\n");
        }
        EVP_PKEY_free(pubKey);
    } else {
        printf("解析 X.509 DER 公钥失败!\n");
    }
}

static void test_decrypt(void)
{
    int ret = 0;
    uint8_t plaintext[] = "Hello, HSM! This is a test message for RSA encryption.";
    uint8_t ciphertext[1024] = {0};
    int cipherLen = sizeof(ciphertext);
    uint8_t decResult[1024] = {0};
    uint32_t decLen = sizeof(decResult);
    OpensslLibraryInit();

    // 测试openssl公钥加密
    printf("\n[1/2] 测试 OpensslPublicKeyEncrypt()...\n");
    ret = OpensslPublicKeyEncrypt("/oemdata/test_pub.pem", RSA_PADDING_PKCS1_V15, plaintext, sizeof(plaintext) - 1, ciphertext, &cipherLen);
    if (ret != 0) { printf("失败! ret=%d\n", ret); return; }
    print_hex("ciphertext (HEX): ", ciphertext, (int)cipherLen);
    printf("Encrypt 成功! cipherLen=%u\n", cipherLen);

    // 测试hsm私钥解密
    printf("\n[2/2] 测试 RpcHsmDecrypt()...\n");
    ret = RpcHsmDecrypt(E_ALG_RSA2048, 0x83, E_RSA_SIGN_PADDING_PKCS1, ciphertext, cipherLen, decResult, &decLen);
    if (ret != 0) { printf("解密失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    printf("解密内容 (ASCII): %.*s\n", decLen, decResult);

    // 调用release
    OpensslLibraryRelease();
}

static void test_sign(void)
{
    int ret = 0;
    uint8_t signData[] = "Data to be signed";
    uint8_t signature[256] = {0};
    uint32_t signLen = sizeof(signature);
    EVP_PKEY *pubKey = NULL;
    OpensslLibraryInit();

    // HSM签名
    printf("\n[1/3] 测试 RpcHsmSign()...\n");
    ret = RpcHsmSign(E_ALG_RSA2048, 0x83, E_DIGIST_SHA256, signData, sizeof(signData) - 1, signature, &signLen);
    if (ret != 0) { printf("签名失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    print_hex("signature (HEX): ", signature, (int)signLen);
    printf("HSM Sign 成功! signLen=%u\n", signLen);

    // 加载公钥
    printf("\n[2/3] 加载公钥...\n");
    ret = OpensslLoadRsaPublicKey("/oemdata/test_pub.pem", &pubKey);
    if (ret != 0) { printf("加载公钥失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    printf("加载公钥成功!\n");

    // 验证openssl公钥签名
    printf("\n[3/3] 测试 OpensslRsaVerifyWithSHA256()...\n");
    // strcpy((char*)signData, "yqgeuqhb1237l aj");
    ret = OpensslRsaVerifyWithSHA256(pubKey, signData, sizeof(signData) - 1, signature, signLen);
    EVP_PKEY_free(pubKey);
    if (ret == 0) {
        printf("  签名验证成功! ret=%d\n", ret);
    } else if (ret == 1) {
        printf("  签名验证失败! ret=%d\n", ret);
    } else {
        printf("  验证出错! ret=%d\n", ret);
    }

    // 调用release
    OpensslLibraryRelease();
}

static void test_openssl_gen_key(void)
{
    const char *priv_key_path = "/oemdata/openssl_private.pem";
    const char *pub_key_path = "/oemdata/openssl_public.pem";
    EVP_PKEY *pkey = NULL;
    int ret = 0;

    OpensslLibraryInit();

    // 生成 RSA 密钥对
    printf("\n[1/3] 生成 RSA 2048 密钥对...\n");
    ret = OpensslGenerateRsaPrivateKey(&pkey, 2048);
    if (ret != 0) { printf("  生成密钥失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    printf("  生成密钥成功!\n");

    // 保存私钥
    printf("\n[2/3] 保存私钥到 %s ...\n", priv_key_path);
    ret = OpensslSaveRsaPrivateKey(pkey, priv_key_path);
    if (ret != 0) { printf("  保存私钥失败! ret=%d\n", ret); EVP_PKEY_free(pkey); OpensslLibraryRelease(); return; }
    printf("  保存私钥成功!\n");

    // 保存公钥
    printf("\n[3/3] 保存公钥到 %s ...\n", pub_key_path);
    ret = OpensslSaveRsaPublicKey(pkey, pub_key_path);
    if (ret != 0) { printf("  保存公钥失败! ret=%d\n", ret); EVP_PKEY_free(pkey); OpensslLibraryRelease(); return; }
    printf("  保存公钥成功!\n");

    // 释放资源
    EVP_PKEY_free(pkey);
    OpensslLibraryRelease();
    printf("\n密钥生成测试完成!\n");
}

static void test_encrypt(void)
{
    int ret = 0;
    EVP_PKEY *pubKey = NULL;
    uint8_t *importBufDer = NULL;
    int importBufDerLen = 0;
    uint8_t plaintext[] = "Hello, HSM! This is a test message for HSM encryption and openssl decrypt.";
    uint8_t ciphertext[1024] = {0};
    uint32_t cipherLen = sizeof(ciphertext);
    EVP_PKEY *privKey = NULL;
    uint8_t decrypted[1024] = {0};
    uint16_t decryptedLen = sizeof(decrypted);
    OpensslLibraryInit();

    // 从pem文件读取公钥
    printf("\n[1/5] 加载公钥...\n");
    ret = OpensslLoadRsaPublicKey("/oemdata/openssl_public.pem", &pubKey);
    if (ret != 0) { printf("加载公钥失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    printf("加载公钥成功!\n");

    // 将公钥转换为 X.509 SubjectPublicKeyInfo DER
    printf("\n[2/5] 转换 RSA 公钥为 X.509 DER...\n");

    importBufDerLen = i2d_PUBKEY(pubKey, &importBufDer);
    EVP_PKEY_free(pubKey);
    if (importBufDerLen <= 0) { printf("转换 RSA 公钥为 X.509 DER 失败!\n"); OPENSSL_free(importBufDer); OpensslLibraryRelease(); return; }
    printf("  X.509 DER 公钥数据长度: %d 字节\n", importBufDerLen);

    // 导入公钥到 HSM
    printf("\n[3/5] 导入公钥到 HSM...\n");
    ret = RpcHsmImportKey(E_ALG_RSA2048, 0x84, E_KEY_TYPE_PUB, importBufDer, (uint32_t)importBufDerLen);
    OPENSSL_free(importBufDer);
    if (ret != 0) { printf("导入公钥失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    printf("导入公钥成功!\n");

    // HSM加密
    printf("\n[4/5] 测试 RpcHsmEncrypt()...\n");
    ret = RpcHsmEncrypt(E_ALG_RSA2048, 0x84, E_RSA_SIGN_PADDING_PKCS1, plaintext, (uint32_t)sizeof(plaintext) - 1, ciphertext, &cipherLen);
    if (ret != 0) { printf("加密失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    print_hex("ciphertext (HEX): ", ciphertext, (int)cipherLen);
    printf("  加密成功! cipherLen=%u\n", cipherLen);

    // 使用 OpenSSL 私钥解密
    printf("\n[5/5] 使用 OpenSSL 私钥解密...\n");
    ret = OpensslLoadRsaPrivateKey("/oemdata/openssl_private.pem", &privKey);
    if (ret != 0) { printf("加载私钥失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    printf("  加载私钥成功!\n");

    // HSM解密
    ret = OpensslPrivateKeyDecryptEx(privKey, RSA_PADDING_PKCS1_V15, ciphertext, (uint16_t)cipherLen, decrypted, &decryptedLen);
    EVP_PKEY_free(privKey);
    if (ret != 0) { printf("解密失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    printf("  解密成功! decryptedLen=%u\n", decryptedLen);
    printf("  decrypted: %.*s\n", decryptedLen, decrypted);

    // 调用release
    OpensslLibraryRelease();
}

static void test_verify(void)
{
    int ret = 0;
    EVP_PKEY *pubKey = NULL;
    uint8_t *importBufDer = NULL;
    int importBufDerLen = 0;
    uint8_t signData[] = "Hello, HSM! This is a test message for openssl signature and HSM verify.";
    uint8_t signature[256] = {0};
    uint16_t signLen = sizeof(signature);
    EVP_PKEY *privKey = NULL;
    OpensslLibraryInit();

    // 从pem文件读取公钥
    printf("\n[1/5] 加载公钥...\n");
    ret = OpensslLoadRsaPublicKey("/oemdata/openssl_public.pem", &pubKey);
    if (ret != 0) { printf("加载公钥失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    printf("加载公钥成功!\n");

    // 将公钥转换为 X.509 SubjectPublicKeyInfo DER
    printf("\n[2/5] 转换 RSA 公钥为 X.509 DER...\n");

    importBufDerLen = i2d_PUBKEY(pubKey, &importBufDer);
    EVP_PKEY_free(pubKey);
    if (importBufDerLen <= 0) { printf("转换 RSA 公钥为 X.509 DER 失败!\n"); OPENSSL_free(importBufDer); OpensslLibraryRelease(); return; }
    printf("  X.509 DER 公钥数据长度: %d 字节\n", importBufDerLen);

    // 导入公钥到 HSM
    printf("\n[3/5] 导入公钥到 HSM...\n");
    ret = RpcHsmImportKey(E_ALG_RSA2048, 0x84, E_KEY_TYPE_PUB, importBufDer, (uint32_t)importBufDerLen);
    OPENSSL_free(importBufDer);
    if (ret != 0) { printf("导入公钥失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    printf("导入公钥成功!\n");

    // 使用 OpenSSL 私钥签名
    printf("\n[4/5] 使用 OpenSSL 私钥签名...\n");

    // 加载 OpenSSL 私钥
    ret = OpensslLoadRsaPrivateKey("/oemdata/openssl_private.pem", &privKey);
    if (ret != 0) { printf("加载私钥失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    printf("  加载私钥成功!\n");

    ret = OpensslRsaSignWithSHA256(privKey, signData, (int)sizeof(signData) - 1, signature, &signLen);
    EVP_PKEY_free(privKey);
    if (ret != 0) { printf("签名失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    print_hex("signature (HEX): ", signature, (int)signLen);
    printf("  签名成功! signLen=%u\n", signLen);

    // 使用 HSM 公钥验签
    printf("\n[5/5] 使用 HSM 公钥验签...\n");
    // strcpy((char*)signData, "use error data");
    ret = RpcHsmVerify(E_ALG_RSA2048, 0x84, E_DIGIST_SHA256, signData, (uint32_t)sizeof(signData) - 1, signature, signLen);
    if (ret != 0) { printf("验签失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    printf("验签成功!\n");

    // 调用release
    OpensslLibraryRelease();
}

static void test_import_private_key(void)
{
    int ret = 0;
    int pkcs8Len = 0;
    uint32_t decLen = 0;
    uint32_t signLen = 0;
    int cipherLen = 0;
    EVP_PKEY *privKey = NULL;
    EVP_PKEY *pubKey = NULL;
    PKCS8_PRIV_KEY_INFO *p8 = NULL;
    uint8_t *pkcs8Der = NULL;
    uint8_t plaintext[] = "Hello, HSM! This is a test for private key import.";
    uint8_t ciphertext[1024] = {0};
    uint8_t decResult[1024] = {0};
    uint8_t signData[] = "Data to be signed by imported private key";
    uint8_t signature[256] = {0};

    int cipherLenInt = sizeof(ciphertext);
    OpensslLibraryInit();

    printf("\n========== 测试: 导入私钥验签 ==========\n");

    // [1/6] 加载 OpenSSL 私钥
    printf("\n[1/6] 加载 OpenSSL 私钥...\n");
    ret = OpensslLoadRsaPrivateKey("/oemdata/openssl_private.pem", &privKey);
    if (ret != 0) { printf("加载私钥失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    printf("  加载私钥成功!\n");

    // [2/6] 将私钥转换为 PKCS#8 PrivateKeyInfo DER
    printf("\n[2/6] 转换 RSA 私钥为 PKCS#8 DER...\n");
    p8 = EVP_PKEY2PKCS8(privKey);
    if (!p8) { printf("EVP_PKEY2PKCS8 失败!\n"); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    pkcs8Len = i2d_PKCS8_PRIV_KEY_INFO(p8, &pkcs8Der);
    PKCS8_PRIV_KEY_INFO_free(p8);
    if (pkcs8Len <= 0) { printf("转换 PKCS#8 DER 失败!\n"); OPENSSL_free(pkcs8Der); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    printf("  PKCS#8 DER 私钥数据长度: %d 字节\n", pkcs8Len);

    // [3/6] 导入私钥到 HSM
    printf("\n[3/6] 导入私钥到 HSM...\n");
    ret = RpcHsmImportKey(E_ALG_RSA2048, 0x85, E_KEY_TYPE_PRI, pkcs8Der, (uint32_t)pkcs8Len);
    OPENSSL_free(pkcs8Der);
    if (ret != 0) { printf("导入私钥失败! ret=%d\n", ret); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    printf("  导入私钥成功!\n");

    // [4/6] 软件公钥加密，HSM 私钥解密
    printf("\n[4/6] 软件公钥加密，HSM 私钥解密...\n");
    ret = OpensslPublicKeyEncrypt("/oemdata/openssl_public.pem", RSA_PADDING_PKCS1_V15, plaintext, sizeof(plaintext) - 1, ciphertext, &cipherLenInt);
    if (ret != 0) { printf("公钥加密失败! ret=%d\n", ret); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    cipherLen = (uint32_t)cipherLenInt;
    printf("  公钥加密成功! cipherLen=%d\n", cipherLenInt);

    decLen = sizeof(decResult);
    ret = RpcHsmDecrypt(E_ALG_RSA2048, 0x85, E_RSA_SIGN_PADDING_PKCS1, ciphertext, cipherLen, decResult, &decLen);
    if (ret != 0 || decLen != sizeof(plaintext) - 1 || memcmp(decResult, plaintext, decLen) != 0) {
        printf("HSM 私钥解密失败! ret=%d\n", ret);
        EVP_PKEY_free(privKey);
        OpensslLibraryRelease();
        return;
    }
    printf("  HSM 私钥解密成功! decLen=%u, data=%.*s\n", decLen, decLen, decResult);

    // [5/6] HSM 私钥签名
    printf("\n[5/6] HSM 私钥签名...\n");
    signLen = sizeof(signature);
    ret = RpcHsmSign(E_ALG_RSA2048, 0x85, E_DIGIST_SHA256, signData, sizeof(signData) - 1, signature, &signLen);
    if (ret != 0) { printf("HSM 私钥签名失败! ret=%d\n", ret); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    print_hex("signature (HEX): ", signature, (int)signLen);
    printf("  HSM 私钥签名成功! signLen=%u\n", signLen);

    // [6/6] 软件公钥验签
    printf("\n[6/6] 软件公钥验签...\n");
    ret = OpensslLoadRsaPublicKey("/oemdata/openssl_public.pem", &pubKey);
    if (ret != 0) { printf("加载公钥失败! ret=%d\n", ret); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    printf("  加载公钥成功!\n");

    ret = OpensslRsaVerifyWithSHA256(pubKey, signData, sizeof(signData) - 1, signature, signLen);
    EVP_PKEY_free(pubKey);
    if (ret != 0) { printf("公钥验签失败! ret=%d\n", ret); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    printf("  公钥验签成功!\n");

    EVP_PKEY_free(privKey);
    OpensslLibraryRelease();
    printf("\n========== 导入私钥验签测试完成 ==========\n");
}

static void test_import_key_pair(void)
{
    int ret = 0;
    int pkcs8Len = 0;
    uint32_t decLen = 0;
    uint32_t signLen = 0;
    uint32_t exportBufLen = 0;
    uint32_t cipherLen = 0;
    EVP_PKEY *privKey = NULL;
    EVP_PKEY *pubKey = NULL;
    PKCS8_PRIV_KEY_INFO *p8 = NULL;
    uint8_t *pkcs8Der = NULL;
    uint8_t exportBuf[1024] = {0};
    uint8_t *origPubDer = NULL;
    int origPubDerLen = 0;
    uint8_t plaintext[] = "Hello, HSM! Key pair import test.";
    uint8_t ciphertext[1024] = {0};
    uint8_t decResult[1024] = {0};
    uint8_t signData[] = "Data to be signed by key pair";
    uint8_t signature[256] = {0};

    int cipherLenInt2 = sizeof(ciphertext);
    uint16_t decLen16 = (uint16_t)sizeof(decResult);
    uint16_t signLen16 = (uint16_t)sizeof(signature);
    OpensslLibraryInit();

    printf("\n========== 测试: 导入密钥对 ==========\n");

    // [1/8] 加载 OpenSSL 私钥
    printf("\n[1/8] 加载 OpenSSL 私钥...\n");
    ret = OpensslLoadRsaPrivateKey("/oemdata/openssl_private.pem", &privKey);
    if (ret != 0) { printf("加载私钥失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    printf("  加载私钥成功!\n");

    // [2/8] 将私钥转换为 PKCS#8 PrivateKeyInfo DER 并导入 HSM
    printf("\n[2/8] 转换密钥对为 PKCS#8 DER 并导入 HSM...\n");
    p8 = EVP_PKEY2PKCS8(privKey);
    if (!p8) { printf("EVP_PKEY2PKCS8 失败!\n"); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    pkcs8Len = i2d_PKCS8_PRIV_KEY_INFO(p8, &pkcs8Der);
    PKCS8_PRIV_KEY_INFO_free(p8);
    if (pkcs8Len <= 0) { printf("转换 PKCS#8 DER 失败!\n"); OPENSSL_free(pkcs8Der); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    printf("  PKCS#8 DER 密钥对数据长度: %d 字节\n", pkcs8Len);

    ret = RpcHsmImportKey(E_ALG_RSA2048, 0x86, E_KEY_TYPE_PRI_PUB_PAIR, pkcs8Der, (uint32_t)pkcs8Len);
    OPENSSL_free(pkcs8Der);
    if (ret != 0) { printf("导入密钥对失败! ret=%d\n", ret); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    printf("  导入密钥对成功!\n");

    // [3/8] 软件公钥加密，HSM 私钥解密
    printf("\n[3/8] 软件公钥加密，HSM 私钥解密...\n");
    ret = OpensslPublicKeyEncrypt("/oemdata/openssl_public.pem", RSA_PADDING_PKCS1_V15, plaintext, sizeof(plaintext) - 1, ciphertext, &cipherLenInt2);
    if (ret != 0) { printf("公钥加密失败! ret=%d\n", ret); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    cipherLen = (uint32_t)cipherLenInt2;
    printf("  公钥加密成功! cipherLen=%d\n", cipherLenInt2);

    decLen = sizeof(decResult);
    ret = RpcHsmDecrypt(E_ALG_RSA2048, 0x86, E_RSA_SIGN_PADDING_PKCS1, ciphertext, cipherLen, decResult, &decLen);
    if (ret != 0 || decLen != sizeof(plaintext) - 1 || memcmp(decResult, plaintext, decLen) != 0) {
        printf("HSM 私钥解密失败! ret=%d\n", ret);
        EVP_PKEY_free(privKey);
        OpensslLibraryRelease();
        return;
    }
    printf("  HSM 私钥解密成功! decLen=%u, data=%.*s\n", decLen, decLen, decResult);

    // [4/8] HSM 签名，软件验签
    printf("\n[4/8] HSM 签名，软件验签...\n");
    signLen = sizeof(signature);
    ret = RpcHsmSign(E_ALG_RSA2048, 0x86, E_DIGIST_SHA256, signData, sizeof(signData) - 1, signature, &signLen);
    if (ret != 0) { printf("HSM 签名失败! ret=%d\n", ret); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    print_hex("signature (HEX): ", signature, (int)signLen);
    printf("  HSM 签名成功! signLen=%u\n", signLen);

    ret = OpensslLoadRsaPublicKey("/oemdata/openssl_public.pem", &pubKey);
    if (ret != 0) { printf("加载公钥失败! ret=%d\n", ret); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    ret = OpensslRsaVerifyWithSHA256(pubKey, signData, sizeof(signData) - 1, signature, signLen);
    EVP_PKEY_free(pubKey);
    if (ret != 0) { printf("软件公钥验签失败! ret=%d\n", ret); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    printf("  软件公钥验签成功!\n");

    // [5/8] HSM 加密，软件解密
    printf("\n[5/8] HSM 加密，软件解密...\n");
    signLen = sizeof(signature); // reuse as plaintext len tracker not needed
    cipherLen = sizeof(ciphertext);
    ret = RpcHsmEncrypt(E_ALG_RSA2048, 0x86, E_RSA_SIGN_PADDING_PKCS1, plaintext, sizeof(plaintext) - 1, ciphertext, &cipherLen);
    if (ret != 0) { printf("HSM 加密失败! ret=%d\n", ret); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    printf("  HSM 加密成功! cipherLen=%d\n", cipherLen);

    ret = OpensslLoadRsaPrivateKey("/oemdata/openssl_private.pem", &pubKey);
    if (ret != 0) { printf("加载私钥失败! ret=%d\n", ret); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    ret = OpensslPrivateKeyDecryptEx(pubKey, RSA_PADDING_PKCS1_V15, ciphertext, (uint16_t)cipherLen, decResult, &decLen16);
    EVP_PKEY_free(pubKey);
    if (ret != 0 || decLen16 != sizeof(plaintext) - 1 || memcmp(decResult, plaintext, decLen16) != 0) {
        printf("软件私钥解密失败! ret=%d\n", ret);
        EVP_PKEY_free(privKey);
        OpensslLibraryRelease();
        return;
    }
    printf("  软件私钥解密成功! decLen=%u, data=%.*s\n", decLen16, decLen16, decResult);

    // [6/8] 软件签名，HSM 验签
    printf("\n[6/8] 软件签名，HSM 验签...\n");
    ret = OpensslLoadRsaPrivateKey("/oemdata/openssl_private.pem", &pubKey);
    if (ret != 0) { printf("加载私钥失败! ret=%d\n", ret); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    ret = OpensslRsaSignWithSHA256(pubKey, signData, (int)sizeof(signData) - 1, signature, &signLen16);
    EVP_PKEY_free(pubKey);
    if (ret != 0) { printf("软件签名失败! ret=%d\n", ret); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    print_hex("signature (HEX): ", signature, signLen16);
    printf("  软件签名成功! signLen=%u\n", signLen16);

    ret = RpcHsmVerify(E_ALG_RSA2048, 0x86, E_DIGIST_SHA256, signData, sizeof(signData) - 1, signature, signLen);
    if (ret != 0) { printf("HSM 验签失败! ret=%d\n", ret); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    printf("  HSM 验签成功!\n");

    // [7/8] 导出 HSM 公钥
    printf("\n[7/8] 导出 HSM 公钥...\n");
    exportBufLen = sizeof(exportBuf);
    ret = RpcHsmExportKey(E_ALG_RSA2048, 0x86, E_KEY_TYPE_PUB, exportBuf, &exportBufLen);
    if (ret != 0) { printf("导出公钥失败! ret=%d\n", ret); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    printf("  导出公钥成功! exportBufLen=%u\n", exportBufLen);

    // [8/8] 比对公钥
    printf("\n[8/8] 比对公钥...\n");
    ret = OpensslLoadRsaPublicKey("/oemdata/openssl_public.pem", &pubKey);
    if (ret != 0) { printf("加载公钥失败! ret=%d\n", ret); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }
    origPubDerLen = i2d_PUBKEY(pubKey, &origPubDer);
    EVP_PKEY_free(pubKey);
    if (origPubDerLen <= 0) { printf("转换原始公钥为 X.509 DER 失败!\n"); OPENSSL_free(origPubDer); EVP_PKEY_free(privKey); OpensslLibraryRelease(); return; }

    // 比对 X.509 DER 公钥
    if (exportBufLen == (uint32_t)origPubDerLen && memcmp(exportBuf, origPubDer, exportBufLen) == 0) {
        printf("  公钥比对通过! X.509 DER 完全一致!\n");
    } else {
        printf("  公钥比对失败! exportLen=%u, origLen=%d\n", exportBufLen, origPubDerLen);
    }
    OPENSSL_free(origPubDer);

    EVP_PKEY_free(privKey);
    OpensslLibraryRelease();
    printf("\n========== 导入密钥对测试完成 ==========\n");
}

static void test_import_export_key(void)
{
    int ret = 0;
    EVP_PKEY *pubKey = NULL;
    uint8_t *importBufDer = NULL;
    int importBufDerLen = 0;
    uint8_t exportBuf[1024] = {0};
    uint32_t exportBufLen = sizeof(exportBuf);
    EVP_PKEY *exportedPubKey = NULL;
    OpensslLibraryInit();

    // 从文件读取公钥
    printf("\n[1/5] 加载公钥...\n");
    ret = OpensslLoadRsaPublicKey("/oemdata/openssl_public.pem", &pubKey);
    if (ret != 0) { printf("加载公钥失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    printf("加载公钥成功!\n");

    // 将公钥转换为 X.509 SubjectPublicKeyInfo DER
    printf("\n[2/5] 转换 RSA 公钥为 X.509 DER...\n");

    importBufDerLen = i2d_PUBKEY(pubKey, &importBufDer);
    EVP_PKEY_free(pubKey);
    if (importBufDerLen <= 0) { printf("转换 RSA 公钥为 X.509 DER 失败!\n"); OPENSSL_free(importBufDer); OpensslLibraryRelease(); return; }
    printf("  X.509 DER 公钥数据长度: %d 字节\n", importBufDerLen);

    // 导入公钥到 HSM
    printf("\n[3/5] 导入公钥到 HSM...\n");
    ret = RpcHsmImportKey(E_ALG_RSA2048, 0x84, E_KEY_TYPE_PUB, importBufDer, (uint32_t)importBufDerLen);
    OPENSSL_free(importBufDer);
    if (ret != 0) { printf("导入公钥失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    printf("导入公钥成功!\n");

    // 导出公钥
    printf("\n[4/5] 导出公钥...\n");
    ret = RpcHsmExportKey(E_ALG_RSA2048, 0x84, E_KEY_TYPE_PUB, exportBuf, &exportBufLen);
    if (ret != 0) { printf("导出公钥失败! ret=%d\n", ret); OpensslLibraryRelease(); return; }
    print_hex("exportBuf (X.509 DER HEX): ", exportBuf, (int)exportBufLen);
    printf("  导出成功! exportBufLen=%u\n", exportBufLen);

    // 解析导出的 X.509 DER 公钥，保存到 PEM 文件
    printf("\n[5/5] 保存导出的公钥到文件...\n");
    {
        uint8_t *p = exportBuf;
        exportedPubKey = d2i_PUBKEY(NULL, (const unsigned char **)&p, (long)exportBufLen);
        if (!exportedPubKey) {
            printf("  解析 X.509 DER 公钥失败!\n");
        } else {
            FILE *file = fopen("/oemdata/hsm_exported_pub.pem", "w");
            if (!file) {
                printf("  打开文件失败!\n");
            } else {
                if (PEM_write_PUBKEY(file, exportedPubKey)) {
                    printf("  公钥已保存到 /oemdata/hsm_exported_pub.pem\n");
                } else {
                    printf("  保存公钥到 PEM 失败!\n");
                }
                fclose(file);
            }
            EVP_PKEY_free(exportedPubKey);
        }
    }

    // 调用release
    OpensslLibraryRelease();
}

static void test_hsm_api_delete(int idx)
{
    int ret = 0;
    ret = RpcHsmDeleteKey(idx&0xffff);
    printf("删除 key:%0x ret = %d \n", idx, ret);
}

static void test_import_export_crt(void)
{
    int ret = 0;
    uint16_t crtId = 0x0001;
    uint8_t testData[] = {0x30, 0x82, 0x01, 0x00, 0x02, 0x82, 0x01, 0x01,
                          0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11,
                          0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    uint32_t testDataLen = sizeof(testData);
    uint8_t exportBuf[1024] = {0};
    uint32_t exportBufLen = sizeof(exportBuf);

    printf("\n========== 测试: 导入导出证书 ==========\n");

    printf("\n[1/3] 导入证书到 HSM (crtId=0x%04X, len=%u)...\n", crtId, testDataLen);
    ret = RpcHsmImportCrt(crtId, testData, testDataLen);
    if (ret != 0) { printf("导入证书失败! ret=%d\n", ret); return; }
    printf("导入证书成功!\n");

    printf("\n[2/3] 导出证书...\n");
    ret = RpcHsmExportCrt(crtId, exportBuf, &exportBufLen);
    if (ret != 0) { printf("导出证书失败! ret=%d\n", ret); return; }
    print_hex("导出数据 (HEX): ", exportBuf, (int)exportBufLen);
    printf("导出证书成功! len=%u\n", exportBufLen);

    printf("\n[3/3] 验证数据一致性...\n");
    if (exportBufLen == testDataLen && memcmp(exportBuf, testData, testDataLen) == 0) {
        printf("数据验证通过: 导入/导出数据一致!\n");
    } else {
        printf("数据验证失败: 导入/导出数据不一致!\n");
        printf("  期望长度: %u, 实际长度: %u\n", testDataLen, exportBufLen);
    }

    printf("\n========== 导入导出证书测试完成 ==========\n");
}

static void test_query_crt(void)
{
    int ret = 0;
    QueryCrtResult_t crtResult = {0};

    printf("\n========== 测试: 查询证书 ==========\n");

    ret = RpcHsmQueryCrt(&crtResult);
    if (ret != 0) { printf("查询证书失败! ret=%d\n", ret); return; }

    printf("查询证书成功! validCrtCount=%u\n", crtResult.validCrtCount);
    if (crtResult.validCrtCount > 0) {
        printf("证书列表:\n");
        for (int i = 0; i < crtResult.validCrtCount; i++) {
            printf("  [%d] FID=0x%04X\n", i, crtResult.crtList[i]);
        }
    } else {
        printf("  (无证书)\n");
    }

    printf("\n========== 查询证书测试完成 ==========\n");
}

static void test_delete_crt(uint16_t crtId)
{
    int ret = 0;
    printf("\n========== 测试: 删除证书 ==========\n");
    printf("删除证书 crtId=0x%04X...\n", crtId);
    ret = RpcHsmDeleteCrt(crtId);
    if (ret != 0) { printf("删除证书失败! ret=%d\n", ret); return; }
    printf("删除证书成功!\n");
    printf("\n========== 删除证书测试完成 ==========\n");
}

static int ImportPkiKeyFromPem(const char *pemKey, uint16_t keyId)
{
    int ret = -1;
    EVP_PKEY *pkey = NULL;
    PKCS8_PRIV_KEY_INFO *p8 = NULL;
    uint8_t *pkcs8Der = NULL;
    int pkcs8Len = 0;
    BIO *bio = NULL;

    do {
        bio = BIO_new_mem_buf(pemKey, -1);
        if (!bio) {
            printf("BIO_new_mem_buf 失败!\n");
            break;
        }

        pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
        if (!pkey) {
            printf("PEM_read_bio_PrivateKey 失败!\n");
            break;
        }

        p8 = EVP_PKEY2PKCS8(pkey);
        if (!p8) {
            printf("EVP_PKEY2PKCS8 失败!\n");
            break;
        }

        pkcs8Len = i2d_PKCS8_PRIV_KEY_INFO(p8, &pkcs8Der);
        if (pkcs8Len <= 0) {
            printf("转换 PKCS#8 DER 失败!\n");
            break;
        }

        ret = RpcHsmImportKey(E_ALG_RSA2048, keyId, E_KEY_TYPE_PRI_PUB_PAIR,
                              pkcs8Der, (uint32_t)pkcs8Len);
    } while (0);

    if (p8) PKCS8_PRIV_KEY_INFO_free(p8);
    if (pkcs8Der) OPENSSL_free(pkcs8Der);
    if (pkey) EVP_PKEY_free(pkey);
    if (bio) BIO_free(bio);

    return ret;
}

static void test_pki_query_status(void)
{
    int ret = 0;
    QueryKeyResult_t keyResult = {0};
    QueryCrtResult_t crtResult = {0};

    printf("\n========== 测试: 查询PKI证书和秘钥安装状态 ==========\n");

    printf("\n[1/2] 查询证书 (crtId=0x%04X)...\n", CRTID_DIGITALKEY_PKI);
    ret = RpcHsmQueryCrt(&crtResult);
    if (ret != 0) {
        printf("查询证书失败! ret=%d\n", ret);
    } else {
        printf("查询证书成功! validCrtCount=%u\n", crtResult.validCrtCount);
        int found = 0;
        for (int i = 0; i < crtResult.validCrtCount; i++) {
            if (crtResult.crtList[i] == CRTID_DIGITALKEY_PKI) {
                found = 1;
                break;
            }
        }
        printf("  PKI证书(0x%04X): %s\n", CRTID_DIGITALKEY_PKI, found ? "已安装" : "未安装");
    }

    printf("\n[2/2] 查询密钥 (keyId=0x%04X)...\n", KEYID_DIGITALKEY_PKI_KEY);
    ret = RpcHsmQueryKey(&keyResult);
    if (ret != 0) {
        printf("查询密钥失败! ret=%d\n", ret);
    } else {
        printf("查询密钥成功! validKeyCount=%u\n", keyResult.validKeyCount);
        int found = 0;
        for (int i = 0; i < keyResult.validKeyCount; i++) {
            if (keyResult.keyList[i].keyId == KEYID_DIGITALKEY_PKI_KEY) {
                found = 1;
                break;
            }
        }
        printf("  PKI密钥(0x%04X): %s\n", KEYID_DIGITALKEY_PKI_KEY, found ? "已安装" : "未安装");
    }

    printf("\n========== 查询PKI安装状态完成 ==========\n");
}

static void test_pki_install(void)
{
    int ret = 0;

    printf("\n========== 测试: 安装PKI证书和秘钥 ==========\n");

    printf("\n[1/2] 安装PKI证书 (crtId=0x%04X)...\n", CRTID_DIGITALKEY_PKI);
    ret = RpcHsmImportCrt(CRTID_DIGITALKEY_PKI, (const uint8_t *)DEMO_PKI_CRT, strlen(DEMO_PKI_CRT));
    if (ret != 0) {
        printf("安装证书失败! ret=%d\n", ret);
    } else {
        printf("安装证书成功!\n");
    }

    printf("\n[2/2] 安装PKI密钥 (keyId=0x%04X)...\n", KEYID_DIGITALKEY_PKI_KEY);
    ret = ImportPkiKeyFromPem(DEMO_PKI_KEY, KEYID_DIGITALKEY_PKI_KEY);
    if (ret != 0) {
        printf("安装密钥失败! ret=%d\n", ret);
    } else {
        printf("安装密钥成功!\n");
    }

    printf("\n========== 安装PKI证书和秘钥完成 ==========\n");
}

static void test_pki_clear(void)
{
    int ret = 0;

    printf("\n========== 测试: 清除PKI证书和秘钥安装 ==========\n");

    printf("\n[1/2] 删除PKI证书 (crtId=0x%04X)...\n", CRTID_DIGITALKEY_PKI);
    ret = RpcHsmDeleteCrt(CRTID_DIGITALKEY_PKI);
    if (ret != 0) {
        printf("删除证书失败! ret=%d\n", ret);
    } else {
        printf("删除证书成功!\n");
    }

    printf("\n[2/2] 删除PKI密钥 (keyId=0x%04X)...\n", KEYID_DIGITALKEY_PKI_KEY);
    ret = RpcHsmDeleteKey(KEYID_DIGITALKEY_PKI_KEY);
    if (ret != 0) {
        printf("删除密钥失败! ret=%d\n", ret);
    } else {
        printf("删除密钥成功!\n");
    }

    printf("\n========== 清除PKI证书和秘钥完成 ==========\n");
}

static void test_pki_self_check(const char *subjectCN)
{
    int ret = -1;
    int fail = 0;
    int crtFound = 0;
    int keyFound = 0;
    X509 *cert = NULL;
    EVP_PKEY *certPubKey = NULL;
    char cnBuf[256] = {0};
    uint8_t cipherBuf[256] = {0};
    uint8_t plainBuf[256] = {0};
    uint8_t crtBuf[2048] = {0};
    uint8_t hsmPubKeyBuf[512] = {0};
    uint8_t *crtPubKeyDer = NULL;
    int crtPubKeyDerLen = 0;
    uint32_t subjectCNLen = 0;
    uint32_t crtBufLen = sizeof(crtBuf);
    uint32_t hsmCipherLen = 0;
    uint32_t hsmPlainLen = 0;
    uint32_t hsmPubKeyLen = sizeof(hsmPubKeyBuf);
    BIO *bio = NULL;

    printf("\n========== 测试: PKI证书和秘钥自检 ==========\n");

    do {

        if (!subjectCN) {
            printf("错误: subjectCN 为 NULL\n");
            fail = 1;
            break;
        }
        subjectCNLen = strlen(subjectCN);
        printf("输入 SubjectCN: %s\n", subjectCN);

        /* 步骤1: 检查 HSM 中是否已安装证书和密钥 */
        printf("\n[1/5] 检查 HSM 中证书/密钥是否存在...\n");
        crtFound = RpcHsmQueryCrtExists(CRTID_DIGITALKEY_PKI);
        keyFound = RpcHsmQueryKeyExists(KEYID_DIGITALKEY_PKI_KEY);
        if (crtFound != 1 || keyFound != 1) {
            printf("  证书或密钥未安装! crtFound=%d keyFound=%d\n", crtFound, keyFound);
            fail = 1;
            break;
        }
        printf("  证书和密钥均已安装\n");

        /* 步骤2: 从 HSM 导出证书并验证合法性 */
        printf("\n[2/5] 验证证书合法性...\n");
        crtBufLen = sizeof(crtBuf);
        memset(crtBuf, 0, sizeof(crtBuf));
        ret = RpcHsmExportCrt(CRTID_DIGITALKEY_PKI, crtBuf, &crtBufLen);
        if (ret != 0) {
            printf("  导出证书失败! ret=%d\n", ret);
            fail = 1;
            break;
        }
        ret = OpensslLoadX509RsaFromPemBuffer((const char *)crtBuf, (int)crtBufLen, &cert, &certPubKey);
        if (ret != 0 || !cert || !certPubKey) {
            printf("  OpensslLoadX509RsaFromPemBuffer 失败! ret=%d\n", ret);
            fail = 1;
            break;
        }
        printf("  证书合法\n");

        /* 步骤3: 验证 SubjectCN */
        printf("\n[3/5] 验证 SubjectCN...\n");
        ret = OpensslLoadCrtSubjectCN(cert, cnBuf, sizeof(cnBuf));
        if (ret != 0) {
            printf("  提取 CN 失败!\n");
            fail = 1;
            break;
        }
        if (strcmp(cnBuf, subjectCN) != 0) {
            printf("  CN 不匹配! 期望:%s 实际:%s\n", subjectCN, cnBuf);
            fail = 1;
            break;
        }
        printf("  SubjectCN 匹配: %s\n", cnBuf);

        /* 步骤4: HSM 公钥加密 -> HSM 私钥解密 */
        printf("\n[4/5] HSM 加密解密测试...\n");
        hsmCipherLen = sizeof(cipherBuf);
        ret = RpcHsmEncrypt(E_ALG_RSA2048, KEYID_DIGITALKEY_PKI_KEY, E_RSA_SIGN_PADDING_PKCS1,
                            (uint8_t *)subjectCN, subjectCNLen, cipherBuf, &hsmCipherLen);
        if (ret != 0) {
            printf("  HSM 加密失败! ret=%d\n", ret);
            fail = 1;
            break;
        }
        printf("  HSM 加密成功! cipherLen=%u\n", hsmCipherLen);

        hsmPlainLen = sizeof(plainBuf);
        ret = RpcHsmDecrypt(E_ALG_RSA2048, KEYID_DIGITALKEY_PKI_KEY, E_RSA_SIGN_PADDING_PKCS1,
                            cipherBuf, hsmCipherLen, plainBuf, &hsmPlainLen);
        if (ret != 0) {
            printf("  HSM 解密失败! ret=%d\n", ret);
            fail = 1;
            break;
        }
        if (hsmPlainLen != subjectCNLen || memcmp(plainBuf, subjectCN, subjectCNLen) != 0) {
            printf("  HSM 解密数据不匹配!\n");
            fail = 1;
            break;
        }
        printf("  HSM 解密成功! 数据一致\n");

        /* 步骤5: 比对 DEMO_PKI_CRT 公钥与 HSM 导出公钥的 X.509 DER 是否一致 */
        printf("\n[5/5] 比对证书公钥与 HSM 公钥 (X.509 DER)...\n");

        // 从 HSM 导出公钥 (X.509 SubjectPublicKeyInfo DER)
        hsmPubKeyLen = sizeof(hsmPubKeyBuf);
        memset(hsmPubKeyBuf, 0, sizeof(hsmPubKeyBuf));
        ret = RpcHsmExportKey(E_ALG_RSA2048, KEYID_DIGITALKEY_PKI_KEY, E_KEY_TYPE_PUB, hsmPubKeyBuf, &hsmPubKeyLen);
        if (ret != 0) {
            printf("  导出 HSM 公钥失败! ret=%d\n", ret);
            fail = 1;
            break;
        }

        // 将证书公钥转换为 X.509 SubjectPublicKeyInfo DER
        crtPubKeyDerLen = i2d_PUBKEY(certPubKey, &crtPubKeyDer);
        if (crtPubKeyDerLen <= 0) {
            printf("  证书公钥转换为 X.509 DER 失败!\n");
            fail = 1;
            break;
        }

        // 比对 X.509 DER 公钥
        if ((uint32_t)crtPubKeyDerLen != hsmPubKeyLen || memcmp(crtPubKeyDer, hsmPubKeyBuf, hsmPubKeyLen) != 0) {
            printf("  公钥 X.509 DER 不匹配! crtLen=%d hsmLen=%u\n", crtPubKeyDerLen, hsmPubKeyLen);
            fail = 1;
            break;
        }
        printf("  公钥 X.509 DER 比对一致\n");

        printf("\n========== PKI自检全部通过 ==========\n");
    } while (0);

    if (fail) {
        printf("\n========== PKI自检失败 ==========\n");
    }

    if (bio) BIO_free(bio);
    if (cert) X509_free(cert);
    if (certPubKey) EVP_PKEY_free(certPubKey);
    if (crtPubKeyDer) OPENSSL_free(crtPubKeyDer);
}

static void test_get_work_state(void)
{
    int ret = 0;
    uint8_t state = 0;

    printf("\n========== 测试: 获取HSM工作状态 ==========\n");
    ret = RpcHsmGetWorkState(&state);
    if (ret != 0) {
        printf("获取HSM工作状态失败! ret=%d\n", ret);
        return;
    }

    printf("HSM 工作状态: ");
    switch (state) {
        case E_HSM_WORK_STATE_NORMAL:
            printf("Normal (0x00)\n");
            break;
        case E_HSM_WORK_STATE_FAULT:
            printf("Fault (0x01)\n");
            break;
        case E_HSM_WORK_STATE_UNKNOWN:
            printf("Unknown (0x02)\n");
            break;
        default:
            printf("未知状态 (0x%02X)\n", state);
            break;
    }
    printf("========== 获取HSM工作状态完成 ==========\n");
}

static int parse_test_args(int argc, char *argv[], const char *optarg)
{
    int test_case = atoi(optarg);

    switch (test_case) {
        case 0: test_hsm_api_full();break;
        case 1: test_hsm_query_key(); break;
        case 2: test_gen_key(); break;
        case 3: test_decrypt(); break;
        case 4: test_sign(); break;
        case 5: test_openssl_gen_key(); break;
        case 6: test_encrypt(); break;
        case 7: test_verify(); break;
        case 8: test_import_export_key(); break;
        case 9: test_import_private_key(); break;
        case 10: test_import_key_pair(); break;
        case 100:
            {
                int idx = 0;
                int deleteKeyId = 0;

                for (int i = 0; i < argc; i++) {
                    if (argv[i] == optarg) {
                        idx = i;
                        break;
                    }
                }
                if (idx + 1 >= argc) {
                        printf("错误:-t 100 后必须传入keyID  \n");
                        usage(argv[0]);
                        return -1;
                }
                deleteKeyId = atoi(argv[idx + 1]);
                test_hsm_api_delete(deleteKeyId);
                break;
            }
        case 101: test_import_export_crt(); break;
        case 102: test_query_crt(); break;
        case 200:
            {
                int idx = 0;
                int deleteCrtId = 0;

                for (int i = 0; i < argc; i++) {
                    if (argv[i] == optarg) {
                        idx = i;
                        break;
                    }
                }
                if (idx + 1 >= argc) {
                        printf("错误:-t 200 后必须传入crtId  \n");
                        usage(argv[0]);
                        return -1;
                }
                deleteCrtId = atoi(argv[idx + 1]);
                test_delete_crt((uint16_t)deleteCrtId);
                break;
            }
        case 1000: test_pki_query_status(); break;
        case 1001: test_pki_install(); break;
        case 1002: test_pki_clear(); break;
        case 1003:
            {
                int idx = 0;
                const char *subjectCN = NULL;
                for (int i = 0; i < argc; i++) {
                    if (argv[i] == optarg) {
                        idx = i;
                        break;
                    }
                }
                if (idx + 1 >= argc) {
                    printf("错误:-t 1003 后必须传入crtSubjectCN\n");
                    usage(argv[0]);
                    return -1;
                }
                subjectCN = argv[idx + 1];
                test_pki_self_check(subjectCN);
                break;
            }
        case 3000: test_get_work_state(); break;
        default:
            printf("错误: 无效的测试编号 %d\n", test_case);
            return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int opt = 0;
    int ret = 0;
    int test_specified = 0;

    LOG_INIT("demo");
    LOG_Info("OpenSSL Engine RPC Client starting...");

    // 解析命令行参数
    while ((opt = getopt(argc, argv, "ht:m:")) != -1) {
        switch (opt) {
            case 'h':
                usage(argv[0]);
                exit(0);
                break;
            case 't':
                ret = parse_test_args(argc, argv, optarg);
                test_specified = 1;
                break;
            default:
                printf("错误: 无效的选项 -%c\n", optopt);
                usage(argv[0]);
                exit(1);
                break;
        }
    }

    if (!test_specified) {
        printf("错误: 未指定测试编号\n");
        usage(argv[0]);
        exit(1);
    }

    return ret;
}