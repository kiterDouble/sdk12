#include "hal_modem_tz.h"
#include <qtee/qteecrypto.h>

int32_t HalModemTzWrite(const uint8_t *data, uint32_t dataLen,const char *fileName)
{
    int ret;
    if (data == NULL || fileName == NULL || dataLen == 0)
    {
        return -1; // 参数错误
    }
    ret = qtee_ss_write(SS_PARTITION, fileName, SS_PROCESS, data, dataLen);    
    return ret;
}

int32_t HalModemTzRead(const char *filename, uint8_t *outbuf, uint32_t *outlen)
{
    int ret;
    if (filename == NULL || outbuf == NULL || outlen == NULL || *outlen == 0)
    {
        return -1; // 参数错误
    }
    ret = qtee_ss_read(SS_PARTITION, filename, SS_PROCESS, outbuf, outlen);    
    return ret;
}

int32_t HalModemTzDelete(const char *filename)
{
    int ret;
    if (filename == NULL)
    {
        return -1; // 参数错误
    }
    ret = qtee_ss_delete(SS_PARTITION, filename, SS_PROCESS);    
    return ret;
}


int32_t HalModemTzQuery(const char *filename)
{
    int ret;
    if (filename == NULL)
    {
        return -1; // 参数错误
    }
    ret = qtee_ss_query(SS_PARTITION, filename, SS_PROCESS);    
    return ret;
}

int32_t HalModemTzReadPubkeyToQteersa(const char *filename, QTEE_RSA *qtee_rsa)
{
    int32_t ret = 0;
    uint8_t *out;
    uint32_t outlen = QTEE_SS_MAXSIZE;
    out = malloc(outlen);
    SRTEE_COND_CHK((out == NULL), -1, err_exit);

    ret = HalModemTzRead(filename, out, &outlen);
    SRTEE_COND_CHK((ret != 0), ret, err_free_out);

    memcpy(qtee_rsa, out, outlen);
    set_qtee_pk_type(qtee_rsa, QTEE_RSA_FLAGS_SRC_TZ);

err_free_out:
    free(out);
err_exit:
    return ret;
}

int32_t HalModemTzRSADelKey(const char *keypair_filename)
{
    QTEE_RSA *keyblob = NULL;
    int32_t ret = 0;

    keyblob = QTEE_RSA_init();
    SRTEE_COND_CHK((keyblob == NULL), -1, err_exit);

    ret = HalModemTzReadPubkeyToQteersa(keypair_filename, keyblob);
    SRTEE_COND_CHK((ret != 0), ret, err_rsa_deinit);

    ret = qtee_delete_rsakey(keyblob);
    SRTEE_COND_CHK((ret != 0), ret, err_rsa_deinit);

    // 删除存储的公钥文件，保持状态一致
    HalModemTzDelete(keypair_filename);
err_rsa_deinit:
    QTEE_RSA_deinit(keyblob);
err_exit:
    return ret;    
}

int32_t HalModemTzImportKey(RSA *rsa,const char *keypair_filename)
{
    int32_t ret = 0;
    QTEE_RSA *qtee_rsa;
    
    SRTEE_COND_CHK((rsa == NULL || keypair_filename == NULL),-1,err_exit);

    qtee_rsa = QTEE_RSA_init();
    SRTEE_COND_CHK((qtee_rsa == NULL),-2,err_exit);

    // 提取RSA密钥中的公钥信息，并将公钥以特定格式写入Tee安全存储区，供后续使用
    ret = pkcs1_rsapub_to_qtee(rsa,qtee_rsa);
    SRTEE_COND_CHK((ret != 0),-3,err_rsa_deinit);

    ret = HalModemTzWrite((const uint8_t *)qtee_rsa, get_qtee_rsa_size(), keypair_filename);
    SRTEE_COND_CHK((ret != 0),-4,err_rsa_deinit);

    // 清空qtee_rsa缓存
    memset(qtee_rsa, 0, get_qtee_rsa_size());

    // 将密钥对转为Tee可识别的格式并导入Tee中，完成密钥对的导入
    ret = pkcs1_rsapri_to_qtee(rsa,qtee_rsa);
    SRTEE_COND_CHK((ret != 0 || qtee_rsa == NULL),-5,err_del_pub);

    QTEE_GENRSA_CTX ctx;
    QTEE_GENRSA_CTX_init(&ctx);
    ctx.inbuf = (uint8_t *)qtee_rsa;
    ctx.inlen = get_qtee_rsa_size();
    ret = qtee_import_rsakey(&ctx);
    SRTEE_COND_CHK((ret == 0 ),0,err_rsa_deinit);

    // 导入失败后删除刚才写入的公钥文件，保持状态一致
err_del_pub:
    HalModemTzDelete(keypair_filename);
err_rsa_deinit:
    QTEE_RSA_deinit(qtee_rsa);
err_exit:
    return ret;
}


// E固定65537
RSA *HalModemTzRSAKeygen(int32_t bits, BIGNUM *e, const char *keypair_filename)
{
    RSA *rsa = NULL;
    int32_t ret = 0;
    // uint32_t exponent = 0;

    rsa = RSA_new();
    SRTEE_COND_CHK((rsa == NULL),-1,err_exit);

    // 1、检测公钥文件是否存在
    if(HalModemTzQuery(keypair_filename) == 0)
    {
        // 公钥文件存在，清除密钥对
        ret = HalModemTzRSADelKey(keypair_filename);

        // 删除公钥文件
        ret = HalModemTzDelete(keypair_filename);

    }

    QTEE_GENRSA_CTX ctx;
    QTEE_GENRSA_CTX_init(&ctx);

    // 转换公钥指数为 uint32_t
    // exponent = BN_get_word(e);
    // SRTEE_COND_CHK((exponent == (BN_ULONG)-1), -1, err_exit);

    // 默认65537
    ctx.pub_e_type = QTEE_RSA_E_TYPE_65537;
    ctx.rsabits = bits;
    ctx.outlen = 4128;
    ctx.outbuf = malloc(ctx.outlen);
    SRTEE_COND_CHK((ctx.outbuf == NULL), -2, err_free_outbuf);

    ret = qtee_gen_rsakey(&ctx);
    SRTEE_COND_CHK((ret != 0), ret, err_free_outbuf);

    // 将公钥的QLRSA格式写入Tee安全存储区
    ret = HalModemTzWrite((const uint8_t *)ctx.outbuf, ctx.outlen, keypair_filename);
    SRTEE_COND_CHK((ret != 0), ret, err_free_outbuf);

    // 提取公钥信息返回
    ret = pkcs1_qtee_to_rsapub((QTEE_RSA *)ctx.outbuf, rsa);
    SRTEE_COND_CHK((ret != 0),ret,err_free_outbuf);

err_free_outbuf:
    free(ctx.outbuf);
err_exit:
    return rsa;
}


int32_t HalModemTzRSASign(int32_t rsabits, unsigned char *der, unsigned int derlen, unsigned char *sigret, unsigned int *siglen, const char *keypair_filename)
{
    QTEE_RSA *qtee_rsa;
    QTEE_RSA_CTX ctx;
    int ret = -1;

    SRTEE_COND_CHK((der == NULL),-1,err_exit);
    SRTEE_COND_CHK((sigret == NULL),-2,err_exit);


    QTEE_RSA_CTX_init(&ctx);

    qtee_rsa = QTEE_RSA_init();
    SRTEE_COND_CHK((qtee_rsa == NULL),-3,err_exit);

    ctx.padding_type = QTEE_RSA_PAD_PKCS1_V1_5_SIG;
    ctx.sigmode = QTEE_RSA_NORMAL_WORLD_SHA256; 
    
    ret = HalModemTzReadPubkeyToQteersa(keypair_filename, qtee_rsa);
    SRTEE_COND_CHK((ret != 0), ret, err_rsa_deinit);
    ctx.inbuf = der;
    ctx.inlen = derlen;
    ctx.inkey = qtee_rsa;
    ctx.outlen = rsabits / 8;
    ctx.outbuf = sigret;
    LOG_Debug("HalModemTzRSASign %d ,%d", ctx.inlen, ctx.outlen);
    ret = qtee_rsa_sign(&ctx);
    SRTEE_COND_CHK((ret != 0), ret, err_rsa_deinit);


    ret = ctx.outlen;
    *siglen = ret;

err_rsa_deinit:
    QTEE_RSA_deinit(qtee_rsa);
err_exit:
    return ret;   
}



RSA *HalModemTzRSAGetPubKey(const char *keypair_filename)
{
    RSA *rsa = NULL;
    QTEE_RSA *qtee_rsa = NULL;
    int32_t ret = 0;

    rsa = RSA_new();
    SRTEE_COND_CHK((rsa == NULL), -1, err_exit);

    qtee_rsa = QTEE_RSA_init();
    SRTEE_COND_CHK((qtee_rsa == NULL), -2, err_free_rsa);

    ret = HalModemTzReadPubkeyToQteersa(keypair_filename, qtee_rsa);
    SRTEE_COND_CHK((ret != 0), ret,  err_free_qtee);

    ret = pkcs1_qtee_to_rsapub(qtee_rsa, rsa);
    SRTEE_COND_CHK((ret != 0), ret, err_free_qtee);

    QTEE_RSA_deinit(qtee_rsa);
    return rsa;

err_free_qtee:
    QTEE_RSA_deinit(qtee_rsa);
err_free_rsa:
    RSA_free(rsa);
err_exit:
    return NULL;
}


int32_t HalModemTzRSAPrivEnc(int32_t rsabits, unsigned char *der, unsigned int derlen, unsigned char *sigret, unsigned int padding, const char *keypair_filename)
{
    QTEE_RSA *qtee_rsa;
    QTEE_RSA_CTX ctx;
    int ret = -1;

    SRTEE_COND_CHK((der == NULL),-1,err_exit);
    SRTEE_COND_CHK((sigret == NULL),-2,err_exit);


    QTEE_RSA_CTX_init(&ctx);

    qtee_rsa = QTEE_RSA_init();
    SRTEE_COND_CHK((qtee_rsa == NULL),-3,err_exit);
    if (padding == RSA_NO_PADDING)
    {
        ctx.padding_type = QTEE_RSA_NO_PAD;
    }
    else
    {
        SRTEE_COND_CHK(1, -4, err_rsa_deinit); // 不支持的填充方式
    }
    ctx.sigmode = QTEE_RSA_NORMAL_WORLD_SHA256; 
    ret = HalModemTzReadPubkeyToQteersa(keypair_filename, qtee_rsa);
    SRTEE_COND_CHK((ret != 0), ret, err_rsa_deinit);
    ctx.inbuf = der;
    ctx.inlen = derlen;
    ctx.inkey = qtee_rsa;
    ctx.outlen = rsabits / 8;
    ctx.outbuf = sigret;
    LOG_Debug("HalModemTzRSAPrivEnc %d ,%d,%d", ctx.inlen, ctx.outlen,ctx.padding_type);
    ret = qtee_rsa_sign(&ctx);
    SRTEE_COND_CHK((ret != 0), ret, err_rsa_deinit);

    ret = ctx.outlen;
err_rsa_deinit:
    QTEE_RSA_deinit(qtee_rsa);
err_exit:
    return ret;   
}


int32_t  HalModemTzRSAPrivDec(int32_t flen, int32_t rsabits, const unsigned char *from, unsigned char *to, const char *keypair_filename)
{
    QTEE_RSA *qtee_rsa;
    QTEE_RSA_CTX ctx;
    int ret = -1;

    SRTEE_COND_CHK((from == NULL),-1,err_exit);
    SRTEE_COND_CHK((to == NULL),-2,err_exit);

    QTEE_RSA_CTX_init(&ctx);
    qtee_rsa = QTEE_RSA_init();
    SRTEE_COND_CHK((qtee_rsa == NULL),-3,err_exit);

    ret = HalModemTzReadPubkeyToQteersa(keypair_filename, qtee_rsa);
    SRTEE_COND_CHK((ret != 0), ret, err_rsa_deinit);

    ctx.padding_type = QTEE_RSA_PAD_PKCS1_V1_5_ENC;
    ctx.inbuf = from;
    ctx.inlen = flen;
    ctx.inkey = qtee_rsa;
    ctx.outlen = ctx.inlen;
    ctx.outbuf = to;

    ret = qtee_rsa_decrypt(&ctx);
    SRTEE_COND_CHK((ret != 0), ret, err_rsa_deinit);
    ret = ctx.outlen;
err_rsa_deinit:
    QTEE_RSA_deinit(qtee_rsa);
err_exit:
    return ret;
}


int32_t HalModemTzRSAPubEnc(RSA *rsa, unsigned char *der, unsigned int derlen, unsigned char *sigret, unsigned int padding)
{
    QTEE_RSA *qtee_rsa;
    QTEE_RSA_CTX ctx;
    int ret = -1;

    SRTEE_COND_CHK((der == NULL),-1,err_exit);
    SRTEE_COND_CHK((sigret == NULL),-2,err_exit);


    QTEE_RSA_CTX_init(&ctx);

    qtee_rsa = QTEE_RSA_init();
    SRTEE_COND_CHK((qtee_rsa == NULL),-3,err_exit);
    if (padding == RSA_NO_PADDING)
    {
        ctx.padding_type = QTEE_RSA_NO_PAD;
    }
    else if(padding == RSA_PKCS1_PADDING)
    {
        ctx.padding_type = QTEE_RSA_PAD_PKCS1_V1_5_ENC;
    }
    else
    {
        SRTEE_COND_CHK(1, -4, err_rsa_deinit); // 不支持的填充方式
    }

    ret = pkcs1_rsapub_to_qtee(rsa,qtee_rsa);
    set_qtee_pk_type(qtee_rsa, QTEE_RSA_FLAGS_NULL);
    SRTEE_COND_CHK((ret != 0),-3,err_rsa_deinit);

    ctx.inbuf = der;
    ctx.inlen = derlen;
    ctx.inkey = qtee_rsa;
    ctx.outlen = 256;
    ctx.outbuf = sigret;
    LOG_Debug("HalModemTzRSAPrivEnc %d ,%d,%d", ctx.inlen, ctx.outlen,ctx.padding_type);
    ret = qtee_rsa_encrypt(&ctx);
    SRTEE_COND_CHK((ret != 0), ret, err_rsa_deinit);

    ret = ctx.outlen;
err_rsa_deinit:
    QTEE_RSA_deinit(qtee_rsa);
err_exit:
    return ret;   
}

int32_t HalModemTzGetPrivateKeyState(const char *keypair_filename)
{
    int32_t ret = 0;
    ret = HalModemTzQuery(keypair_filename);
    if (ret == 0)
    {
        // 文件存在，说明密钥对已生成
        return 0; // 成功
    }
    else
    {
        return -1; 
    }
}