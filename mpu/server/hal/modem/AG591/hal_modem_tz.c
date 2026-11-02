#include "hal_modem_tz.h"
#include <qtee/qteecrypto.h>


static pid_t* tee_flag()
{
  static pid_t flag = 0;
  static pid_t pid = 0;

  if(flag != 0)
  {
    LOG_Error("%s tee_debug not incomplete access, pid[%d] tid[%d]", __FUNCTION__, pid, flag);
    //寄希望于调用者的错误处理可恢复异常
    //如不能，继续卡死
    flag = 0;
    return NULL;
  }

  flag = gettid();
  pid = getpid();
  LOG_Debug("%s tee_debug tee access, pid[%d] tid[%d]", __FUNCTION__, pid, flag);

  return &flag;
}

static void reset_flag(pid_t *flag)
{
  if(flag)
  {
    *flag = 0;
  }

  LOG_Debug("%s tee_debug complete access, pid[%d] tid[%d]", __FUNCTION__, getpid(), gettid());
}

int32_t HalModemTzWrite(const uint8_t *data, uint32_t dataLen,const char *fileName)
{
    int ret;

    if (data == NULL || fileName == NULL || dataLen == 0)
    {
        return -1; // 参数错误
    }

    LOG_Debug("%s tee_debug qtee_ss_write", __FUNCTION__);
    pid_t *flag = NULL;
    flag = tee_flag();
    if(!flag)
    {
      LOG_Error("%s tee_debug incomplete, current pid[%d] tid[%d]", __FUNCTION__, getpid(), gettid());
//      return QTEE_ERR_FAILURE;
    }

    ret = qtee_ss_write(SS_PARTITION, fileName, SS_PROCESS, data, dataLen);    
    reset_flag(flag);

    return ret;
}

int32_t HalModemTzRead(const char *filename, uint8_t *outbuf, uint32_t *outlen)
{
    int ret;
    if (filename == NULL || outbuf == NULL || outlen == NULL || *outlen == 0)
    {
        return -1; // 参数错误
    }

    LOG_Debug("%s tee_debug qtee_ss_read", __FUNCTION__);
    pid_t *flag = NULL;
    flag = tee_flag();
    if(!flag)
    {
      LOG_Error("%s tee_debug incomplete, current pid[%d] tid[%d]", __FUNCTION__, getpid(), gettid());
 //     return QTEE_ERR_FAILURE;
    }

    ret = qtee_ss_read(SS_PARTITION, filename, SS_PROCESS, outbuf, outlen);    
    reset_flag(flag);

    return ret;
}

int32_t HalModemTzDelete(const char *filename)
{
    int ret;
    if (filename == NULL)
    {
        return -1; // 参数错误
    }

    LOG_Debug("%s tee_debug qtee_ss_delete", __FUNCTION__);
    pid_t *flag = NULL;
    flag = tee_flag();
    if(!flag)
    {
      LOG_Error("%s tee_debug incomplete, current pid[%d] tid[%d]", __FUNCTION__, getpid(), gettid());
//      return QTEE_ERR_FAILURE;
    }

    ret = qtee_ss_delete(SS_PARTITION, filename, SS_PROCESS);    
    reset_flag(flag);

    return ret;
}


int32_t HalModemTzQuery(const char *filename)
{
    int ret;
    if (filename == NULL)
    {
        return -1; // 参数错误
    }

    LOG_Debug("%s tee_debug qtee_ss_query", __FUNCTION__);
    pid_t *flag = NULL;
    flag = tee_flag();
    if(!flag)
    {
      LOG_Error("%s tee_debug incomplete, current pid[%d] tid[%d]", __FUNCTION__, getpid(), gettid());
  //    return QTEE_ERR_FAILURE;
    }

    ret = qtee_ss_query(SS_PARTITION, filename, SS_PROCESS);    
    reset_flag(flag);

    return ret;
}

int32_t HalModemTzReadPubkeyToQteersa(const char *filename, QTEE_RSA *qtee_rsa)
{
    int32_t ret = 0;
    uint8_t *out;
    uint32_t outlen = QTEE_SS_MAXSIZE;
    out = malloc(outlen);
    LOG_Debug("%s tee_debug malloc outlen: %d", __FUNCTION__, out);
    SRTEE_COND_CHK((out == NULL), -1, err_exit);

    ret = HalModemTzRead(filename, out, &outlen);
    SRTEE_COND_CHK((ret != 0), ret, err_free_out);

    memcpy(qtee_rsa, out, outlen);
    LOG_Debug("%s tee_debug set_qtee_pk_type", __FUNCTION__);
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

    LOG_Debug("%s QTEE_RSA_init", __FUNCTION__);
    keyblob = QTEE_RSA_init();
    SRTEE_COND_CHK((keyblob == NULL), -1, err_exit);

    ret = HalModemTzReadPubkeyToQteersa(keypair_filename, keyblob);
    SRTEE_COND_CHK((ret != 0), ret, err_rsa_deinit);

    LOG_Debug("%s qtee_delete_rsakey", __FUNCTION__);
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

    LOG_Debug("%s QTEE_RSA_init", __FUNCTION__);
    qtee_rsa = QTEE_RSA_init();
    SRTEE_COND_CHK((qtee_rsa == NULL),-2,err_exit);

    pid_t *flag = NULL;
    flag = tee_flag();
    if(!flag)
    {
      LOG_Error("%s tee_debug incomplete, current pid[%d] tid[%d]", __FUNCTION__, getpid(), gettid());
   //   goto err_rsa_deinit;
    }
    // 提取RSA密钥中的公钥信息，并将公钥以特定格式写入Tee安全存储区，供后续使用
    ret = pkcs1_rsapub_to_qtee(rsa,qtee_rsa);
    reset_flag(flag);
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

    LOG_Debug("%s tee_debug RSA_new", __FUNCTION__);
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
    LOG_Debug("%s tee_debug QTEE_GENRSA_CTX_init", __FUNCTION__);
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

    LOG_Debug("%s qtee_gen_rsakey", __FUNCTION__);
    ret = qtee_gen_rsakey(&ctx);
    SRTEE_COND_CHK((ret != 0), ret, err_free_outbuf);

    // 将公钥的QLRSA格式写入Tee安全存储区
    ret = HalModemTzWrite((const uint8_t *)ctx.outbuf, ctx.outlen, keypair_filename);
    SRTEE_COND_CHK((ret != 0), ret, err_free_outbuf);

    // 提取公钥信息返回
    LOG_Debug("%s pkcs1_qtee_to_rsapub", __FUNCTION__);
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


    LOG_Debug("%s tee_debug QTEE_RSA_CTX_init", __FUNCTION__);
    QTEE_RSA_CTX_init(&ctx);

    LOG_Debug("%s tee_debug QTEE_CTX_init", __FUNCTION__);
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

    pid_t *flag = NULL;
    flag = tee_flag();
    if(!flag)
    {
      LOG_Error("%s tee_debug incomplete, current pid[%d] tid[%d]", __FUNCTION__, getpid(), gettid());
    //  goto err_rsa_deinit;
    }

    ret = qtee_rsa_sign(&ctx);
    reset_flag(flag);

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
    LOG_Debug("%s tee_debug ", __FUNCTION__);
    RSA *rsa = NULL;
    QTEE_RSA *qtee_rsa = NULL;
    int32_t ret = 0;

    LOG_Debug("%s tee_debug RSA_new", __FUNCTION__);
    rsa = RSA_new();
    SRTEE_COND_CHK((rsa == NULL), -1, err_exit);

    LOG_Debug("%s tee_debug QTEE_RSA_init", __FUNCTION__);
    qtee_rsa = QTEE_RSA_init();
    SRTEE_COND_CHK((qtee_rsa == NULL), -2, err_free_rsa);

    ret = HalModemTzReadPubkeyToQteersa(keypair_filename, qtee_rsa);
    SRTEE_COND_CHK((ret != 0), ret,  err_free_qtee);

    pid_t *flag = NULL;
    flag = tee_flag();
    if(!flag)
    {
      LOG_Error("%s tee_debug incomplete, current pid[%d] tid[%d]", __FUNCTION__, getpid(), gettid());
     // goto err_free_qtee;
    }

    ret = pkcs1_qtee_to_rsapub(qtee_rsa, rsa);
    reset_flag(flag);

    SRTEE_COND_CHK((ret != 0), ret, err_free_qtee);

    LOG_Debug("%s tee_debug QTEE_RSA_deinit", __FUNCTION__);
    QTEE_RSA_deinit(qtee_rsa);
    return rsa;

err_free_qtee:
    LOG_Debug("%s tee_debug QTEE_RSA_deinit", __FUNCTION__);
    QTEE_RSA_deinit(qtee_rsa);
err_free_rsa:
    LOG_Debug("%s tee_debug QTEE_RSA_free", __FUNCTION__);
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


    LOG_Debug("%s tee_debug QTEE_RSA_CTX_init", __FUNCTION__);
    QTEE_RSA_CTX_init(&ctx);

    LOG_Debug("%s tee_debug QTEE_RSA_init", __FUNCTION__);
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

    pid_t *flag = NULL;
    flag = tee_flag();
    if(!flag)
    {
      LOG_Error("%s tee_debug incomplete, current pid[%d] tid[%d]", __FUNCTION__, getpid(), gettid());
      //goto err_rsa_deinit;
    }

    ret = qtee_rsa_sign(&ctx);
    reset_flag(flag);

    SRTEE_COND_CHK((ret != 0), ret, err_rsa_deinit);

    ret = ctx.outlen;
err_rsa_deinit:
    LOG_Debug("%s tee_debug QTEE_RSA_deinit", __FUNCTION__);
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

    pid_t *flag = NULL;
    flag = tee_flag();
    if(!flag)
    {
      LOG_Error("%s tee_debug incomplete, current pid[%d] tid[%d]", __FUNCTION__, getpid(), gettid());
      //goto err_rsa_deinit;
    }

    ret = qtee_rsa_decrypt(&ctx);
    reset_flag(flag);

    SRTEE_COND_CHK((ret != 0), ret, err_rsa_deinit);
    ret = ctx.outlen;
err_rsa_deinit:
    LOG_Debug("%s tee_debug QTEE_RSA_deinit", __FUNCTION__);
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


    LOG_Debug("%s tee_debug QTEE_RSA_CTX_init", __FUNCTION__);
    QTEE_RSA_CTX_init(&ctx);

    LOG_Debug("%s tee_debug QTEE_RSA_init", __FUNCTION__);
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
    pid_t *flag = NULL;
    flag = tee_flag();
    if(!flag)
    {
      LOG_Error("%s tee_debug incomplete, current pid[%d] tid[%d]", __FUNCTION__, getpid(), gettid());
     // goto err_rsa_deinit;
    }
    ret = qtee_rsa_encrypt(&ctx);
    reset_flag(flag);

    SRTEE_COND_CHK((ret != 0), ret, err_rsa_deinit);

    ret = ctx.outlen;
err_rsa_deinit:
    QTEE_RSA_deinit(qtee_rsa);
err_exit:
    return ret;   
}

int32_t HalModemTzGetPrivateKeyState(const char *keypair_filename)
{
    LOG_Debug("%s tee_debug", __FUNCTION__);
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
