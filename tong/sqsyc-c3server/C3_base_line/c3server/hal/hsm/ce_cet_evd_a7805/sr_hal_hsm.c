#include "sr_hal_hsm.h"
#include "err_code.h"
#include "glib.h"

#define HSM_INI_FILE_PATH "/oemapp/config/hsm.ini"
// #define HSM_INI_FILE_PATH "/oemdata/hsm.ini"

static uint8_t fm1280_pin[3] = {0x11, 0x22, 0x33};

static int32_t HsmParserIni(void *devPath)
{
    if(devPath == NULL)
    {
        return RET_ERR;
    }
    int32_t ret = RET_ERR;
    GKeyFile *keyFile = NULL;
    GError *error = NULL;
    keyFile = g_key_file_new();

    if (!keyFile)
    {
        LOG_Error("Error new key file");
        return ret;
    }

    if (!g_key_file_load_from_file(keyFile, HSM_INI_FILE_PATH, G_KEY_FILE_NONE, &error))
    {
        LOG_Error("Error loading key file: %s", error->message);
        g_error_free(error);
        g_key_file_free(keyFile);
        return ret;
    }
    const char *section = "hsm";
    char *dev = g_key_file_get_string(keyFile, section, "dev", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to get integer value for key 'dev': %s", error->message);
        g_clear_error(&error);
        g_key_file_free(keyFile);
        return ret;
    }
    LOG_Debug("Value of key 'dev' in section 'hsm': %s", dev);
    strcpy(devPath, dev);
    g_free(dev);
    g_key_file_free(keyFile);
    ret = RET_OK;
    return ret;
}

int32_t HalHsmInit(void)
{
    int sw;
    uint16_t rlen;
    char hsmDevPath[64] = {0};
    if (RET_OK != HsmParserIni(hsmDevPath))
    {
        LOG_Error("%s HsmParserIni fail.", __FUNCTION__);
        return -1;
    }
    uint8_t rbuf[300];
    sw = fmse_open(hsmDevPath);
    if (sw < 0)
    {
        LOG_Debug("open %s fail", hsmDevPath);
        return -1;
    }
    sw = SelectFile(0xDF01, rbuf, &rlen);
    if (sw != 0x9000)
    {
        LOG_Debug("SelectFile SW : %x", sw);
        return 0xFD;
    }
    sw = VerifyPIN(fm1280_pin, 3);
    if (sw != 0x9000)
    {
        LOG_Debug("VerifyPIN SW : %x", sw);
        return 0xFC;
    }
    return 1;
}

void HalHsmDeinit(void)
{
    fmse_close();
}

int32_t HalHsmRand(unsigned char *buf, int num)
{
    if (num != 4 && num != 8 && num != 16)
    {
        // 仅支持长度4 8 16，如果输入长度不对，默认生成4字节
        num = 4;
    }
    int ret = GetChallenge(num, buf);
    if (ret != 0x9000)
    {
        LOG_Debug("GetChallenge ret = %x!", ret);
    }
    // memset(buf,0,num);
    return ret;
}

int32_t HalHsmSHA256Update(EVP_MD_CTX *ctx, const void *data, size_t count)
{
    unsigned char *res_buf = (unsigned char *)malloc(sizeof(unsigned char) * 32);
    uint16_t res_len = 0;
    if (count <= 253)
    {
        uint8_t sbuf[256] = {0};
        sbuf[0] = 0xc1;
        sbuf[1] = count;
        memcpy(sbuf + 2, data, count);

        int ret = DataCompress(0x0102, count + 2, sbuf, res_buf, &res_len);
        if (ret != 0x9000)
        {
            LOG_Debug("SHA256 ret = %x!", ret);
            return 0;
        }
        LOG_Debug("SHA256 32byte:");
        dump_data(res_len, res_buf);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        ctx->md_data = res_buf;
#else
        EVP_DigestUpdate(ctx, res_buf, res_len);
#endif
        return 1;
    }
    else if (count >= 254 && count <= 506)
    {
        uint8_t sbuf[512] = {0};
        sbuf[0] = 0xc1;
        sbuf[1] = 0xFD;
        memcpy(sbuf + 2, data, 0xFD);
        int ret = DataCompress(0x0002, 0xFF, sbuf, res_buf, &res_len);
        if (ret != 0x9000)
        {
            LOG_Debug("SHA256 ret = %x!", ret);
            return 0;
        }

        memset(sbuf, 0x00, 512);
        sbuf[0] = 0xc1;
        sbuf[1] = count - 0xFD;
        memcpy(sbuf + 2, data + 0xFD, sbuf[1]);
        ret = DataCompress(0x0302, sbuf[1] + 2, sbuf, res_buf, &res_len);
        if (ret != 0x9000)
        {
            LOG_Debug("SHA256 ret = %x!", ret);
            return 0;
        }
        LOG_Debug("SHA256 32byte:");
        dump_data(res_len, res_buf);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        ctx->md_data = res_buf;
#else
        EVP_DigestUpdate(ctx, res_buf, res_len);
#endif
        return 1;
    }
    else if (count >= 507 && count <= 759)
    {
        // 首块
        uint8_t sbuf[512] = {0};
        sbuf[0] = 0xc1;
        sbuf[1] = 0xFD;
        memcpy(sbuf + 2, data, 0xFD);
        int ret = DataCompress(0x0002, 0xFF, sbuf, res_buf, &res_len);
        if (ret != 0x9000)
        {
            LOG_Debug("SHA256 ret = %x!", ret);
            return 0;
        }

        // 中间块
        memset(sbuf, 0x00, 512);
        sbuf[0] = 0xc1;
        sbuf[1] = 0xFD;
        memcpy(sbuf + 2, data + 0xFD, sbuf[1]);
        ret = DataCompress(0x0202, sbuf[1] + 2, sbuf, res_buf, &res_len);
        if (ret != 0x9000)
        {
            LOG_Debug("SHA256 ret = %x!", ret);
            return 0;
        }

        // 尾块
        memset(sbuf, 0x00, 512);
        sbuf[0] = 0xc1;
        sbuf[1] = count - 506;
        memcpy(sbuf + 2, data + 506, sbuf[1]);
        ret = DataCompress(0x0302, sbuf[1] + 2, sbuf, res_buf, &res_len);
        if (ret != 0x9000)
        {
            LOG_Debug("SHA256 ret = %x!", ret);
            return 0;
        }
        LOG_Debug("SHA256 32byte:");
        dump_data(res_len, res_buf);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        ctx->md_data = res_buf;
#else
        EVP_DigestUpdate(ctx, res_buf, res_len);
#endif
        return 1;
    }
    else
    {
        LOG_Debug("SHA256 data too long!");
        free(res_buf);
        return 0;
    }
}

int32_t HalHsmRSAPubEnc(int32_t flen, int32_t rsabits, const unsigned char *rand_buf, const unsigned char *from, unsigned char *to)
{
    int sw;
    uint16_t rlen;
    uint8_t rbuf[300] = {0};
    uint8_t inbuf[270] = {0};
    // c0 02+公钥文件fid c1 820100+输入数据块
    memcpy(inbuf, "\xc0\x02\x0A\x01\xc1\x82\x01\x00", 8);
    inbuf[2] = (uint8_t)((uint32_t)FILE_ID_11 >> 24);
    inbuf[3] = (uint8_t)((uint32_t)FILE_ID_11 >> 16);
    uint16_t p1p2 = 0;
    if (rsabits == 1024 || rsabits == 1280)
    {
        // memcpy(inbuf+7, from, flen);
        // 对数据进行pkcs1填充 注意：flen最大128-11=117字节或160-11=149字节
        inbuf[7] = 0x00;
        inbuf[8] = 0x02;
        memcpy(inbuf + 9, rand_buf, 128 - flen - 3);
        inbuf[128 - flen - 3 + 2 + 7] = 0x00;
        memcpy(inbuf + 7 + 128 - flen, from, flen);
        // p1 = 0x40（0100表示回送结果 0000表示唯一块） p2 = 0x02 （表示rsa2048） lc表示长度（数据长度0x80+inbuf最开始的7个字节）
        p1p2 = 0x4000;
        uint16_t in_len = 0x87;
        inbuf[5] = 0x81;
        inbuf[6] = 0x80;
        if (rsabits == 1280)
        {
            p1p2 += 0x0001;
            in_len = 0xA7;
            inbuf[6] = 0xA0;
            memcpy(inbuf + 9, rand_buf, 160 - flen - 3);
            inbuf[160 - flen - 3 + 2 + 7] = 0x00;
            memcpy(inbuf + 7 + 160 - flen, from, flen);
        }
        sw = RSAPubKeyCal(p1p2, in_len, inbuf, rbuf, &rlen);
        LOG_Debug("RSAPubKeyCal:sw=%04x,rlen=%d", sw, rlen);
        if (sw != 0x9000)
            return 1;
        LOG_Debug("RSA1024/1280 PUB key enc:");
        // dump_data(rlen, rbuf);
        memcpy(to, rbuf, rlen);
    }
    else // rsa2048
    {
        // memcpy(inbuf + 8, from, flen);
        // 对数据进行pkcs1填充 注意：m_length最大256-11=245字节
        inbuf[8] = 0x00;
        inbuf[9] = 0x02;
        memcpy(inbuf + 10, rand_buf, 256 - flen - 3);
        inbuf[256 - flen - 3 + 2 + 8] = 0x00;
        memcpy(inbuf + 8 + 256 - flen, from, flen);
        // p1 = 0x41（0100表示回送结果 0001表示首块） p2 = 0x02 （表示rsa2048） lc=0x88表示长度为136（128数据+inbuf最开始的8个字节）
        sw = RSAPubKeyCal(0x4102, 0x88, inbuf, rbuf, &rlen);
        LOG_Debug("RSAPubKeyCal:sw=%04x,rlen=%d", sw, rlen);
        if (sw != 0x9000)
            return 1;
        ////p1 = 0x43 （0100表示回送结果 0011表示尾块） p2 = 0x02 （表示rsa2048） lc=0x80表示长度为128
        // 尾块返回响应报文数据域，也就是全部的256字节公钥签名后的数据
        sw = RSAPubKeyCal(0x4302, 0x80, (uint8_t *)(inbuf + 0x88), rbuf, &rlen);
        LOG_Debug("RSAPubKeyCal2:sw=%04x,rlen=%d", sw, rlen);
        if (sw != 0x9000)
            return 1;
        LOG_Debug("RSA2048 PUB key enc:");
        // dump_data(rlen, rbuf);
        memcpy(to, rbuf, rlen);
    }
    return rlen;
}

int32_t HalHsmRSAPrivDec(int32_t flen, int32_t rsabits, const unsigned char *from, unsigned char *to)
{
    int sw;
    uint16_t rlen;
    uint8_t rbuf[300] = {0};
    uint8_t inbuf[270] = {0};

    // c2 02+私钥文件fid(0999) c1 820100+输入数据块
    memcpy(inbuf, "\xc2\x02\x0A\x91\xc1\x82\x01\x00", 8);
    inbuf[2] = (uint8_t)((uint32_t)FILE_ID_11 >> 8);
    inbuf[3] = (uint8_t)((uint32_t)FILE_ID_11);
    uint16_t p1p2 = 0;
    if (rsabits == 1024 || rsabits == 1280)
    {
        memcpy(inbuf + 7, from, flen);
        // p1 = 0x40（0100表示回送结果 0000表示唯一块） p2 = 0x02 （表示rsa2048） lc表示长度（数据长度0x80+inbuf最开始的7个字节）
        p1p2 = 0x4000;
        uint16_t in_len = 0x87;
        inbuf[5] = 0x81;
        inbuf[6] = 0x80;
        if (rsabits == 1280)
        {
            p1p2 += 0x0001;
            in_len = 0xA7;
            inbuf[6] = 0xA0;
        }
        sw = RSAPriKeyCal(p1p2, in_len, inbuf, rbuf, &rlen);
        LOG_Debug("RSAPriKeyCal:sw=%04x,rlen=%d", sw, rlen);
        if (sw != 0x9000)
            return 1;
        LOG_Debug("RSA1024/1280 PRIC key dec:");
        // dump_data(rlen, rbuf);
    }
    else // rsa2048
    {
        memcpy(inbuf + 8, from, flen);
        // RSAPriKeyCal操作时，加密芯片根据传入的文件id去自己从自己内部查询私钥去操作
        // p1 = 0x41（0100表示回送结果 0001表示首块） p2 = 0x02 （表示rsa2048） lc=0x88表示长度为136（128数据+inbuf最开始的8个字节）
        sw = RSAPriKeyCal(0x4102, 0x88, inbuf, rbuf, &rlen);
        LOG_Debug("RSAPriKeyCal:sw=%04x,rlen=%d", sw, rlen);
        if (sw != 0x9000)
            return 1;
        // p1 = 0x43 （0100表示回送结果 0011表示尾块） p2 = 0x02 （表示rsa2048） lc=0x80表示长度为128
        // 尾块返回响应报文数据域，也就是全部的256字节私钥签名数据
        sw = RSAPriKeyCal(0x4302, 0x80, (uint8_t *)(inbuf + 0x88), rbuf, &rlen);
        LOG_Debug("RSAPriKeyCal2:sw=%04x,rlen=%d", sw, rlen);
        if (sw != 0x9000)
            return 1;
        LOG_Debug("RSA2048 PRIV key dec:");
        // dump_data(rlen, rbuf);
    }
    // 解密后的数据也是pkcs1填充的，所以需要找到第二个0x00出现的位置，从第二个0x00之后才是签名前的数据
    int count = 0;
    for (size_t i = 0; i < rlen; i++)
    {
        if (rbuf[i] == 0x00)
        {
            count++;
            if (count == 2)
            {
                memcpy(to, rbuf + i + 1, rlen - i - 1);
                return rlen - i - 1;
            }
        }
    }
    return 0;
}

int32_t HalHsmRSASign(int32_t rsabits, unsigned char *der, unsigned int derlen,
                                    unsigned char *sigret, unsigned int *siglen)
{
    // 使用RSA私钥计算签名
    int sw;
    uint16_t rlen;
    uint8_t rbuf[300] = {0};
    uint8_t inbuf[270] = {0};

    // c2 02+私钥文件fid(0999) c1 820100+输入数据块
    memcpy(inbuf, "\xc2\x02\x0A\x91\xc1\x82\x01\x00", 8);
    inbuf[2] = (uint8_t)((uint32_t)FILE_ID_11 >> 8);
    inbuf[3] = (uint8_t)((uint32_t)FILE_ID_11);
    uint16_t p1p2 = 0;
    if (rsabits == 1024 || rsabits == 1280)
    {
        // memcpy(inbuf+7, m, m_length);
        // 对数据进行pkcs1填充 注意：m_length最大128-11=117字节或160-11=149字节
        inbuf[7] = 0x00;
        inbuf[8] = 0x01;
        memset(inbuf + 9, 0xff, 128 - derlen - 3);
        inbuf[128 - derlen - 3 + 2 + 7] = 0x00;
        memcpy(inbuf + 7 + 128 - derlen, der, derlen);
        // p1 = 0x40（0100表示回送结果 0000表示唯一块） p2 = 0x02 （表示rsa2048） lc表示长度（数据长度0x80+inbuf最开始的7个字节）
        p1p2 = 0x4000;
        uint16_t in_len = 0x87;
        inbuf[5] = 0x81;
        inbuf[6] = 0x80;
        if (rsabits == 1280)
        {
            p1p2 += 0x0001;
            in_len = 0xA7;
            inbuf[6] = 0xA0;
            memset(inbuf + 9, 0xff, 160 - derlen - 3);
            inbuf[160 - derlen - 3 + 2 + 7] = 0x00;
            memcpy(inbuf + 7 + 160 - derlen, der, derlen);
        }
        sw = RSAPriKeyCal(p1p2, in_len, inbuf, rbuf, &rlen);
        LOG_Debug("RSAPriKeyCal:sw=%04x,rlen=%d", sw, rlen);
        if (sw != 0x9000)
            return -1;
        LOG_Debug("sign data:");
        // dump_data(rlen, rbuf);
        memcpy(sigret, rbuf, rlen);
        *siglen = rlen;
    }
    else // rsa2048
    {
        // memcpy(inbuf + 8, m, m_length);
        // 对数据进行pkcs1填充 注意：m_length最大256-11=245字节
        inbuf[8] = 0x00;
        inbuf[9] = 0x01;
        memset(inbuf + 10, 0xff, 256 - derlen - 3);
        inbuf[256 - derlen - 3 + 2 + 8] = 0x00;
        memcpy(inbuf + 8 + 256 - derlen, der, derlen);
        // RSAPriKeyCal操作时，加密芯片根据传入的文件id去自己从自己内部查询私钥去操作
        // p1 = 0x41（0100表示回送结果 0001表示首块） p2 = 0x02 （表示rsa2048） lc=0x88表示长度为136（128数据+inbuf最开始的8个字节）
        sw = RSAPriKeyCal(0x4102, 0x88, inbuf, rbuf, &rlen);
        LOG_Debug("RSAPriKeyCal:sw=%04x,rlen=%d", sw, rlen);
        if (sw != 0x9000)
            return -1;
        // p1 = 0x43 （0100表示回送结果 0011表示尾块） p2 = 0x02 （表示rsa2048） lc=0x80表示长度为128
        // 尾块返回响应报文数据域，也就是全部的256字节私钥签名数据
        sw = RSAPriKeyCal(0x4302, 0x80, (uint8_t *)(inbuf + 0x88), rbuf, &rlen);
        LOG_Debug("RSAPriKeyCal2:sw=%04x,rlen=%d", sw, rlen);
        if (sw != 0x9000)
            return -1;
        LOG_Debug("sign data:");
        dump_data(rlen, rbuf);
        memcpy(sigret, rbuf, rlen);
        *siglen = rlen;
    }
    return rlen;
}

int32_t HalHsmRSAVerify(int32_t rsabits, uint16_t *rlen, uint8_t* rbuf,
                                    const unsigned char *sigbuf, unsigned int siglen)
{
    int sw;
    uint8_t inbuf[270] = {0};

    // c0 02+公钥文件fid c1 820100+输入数据块
    memcpy(inbuf, "\xc0\x02\x0A\x01\xc1\x82\x01\x00", 8);
    inbuf[2] = (uint8_t)((uint32_t)FILE_ID_11 >> 24);
    inbuf[3] = (uint8_t)((uint32_t)FILE_ID_11 >> 16);
    uint16_t p1p2 = 0;
    if (rsabits == 1024 || rsabits == 1280)
    {
        memcpy(inbuf + 7, sigbuf, siglen);
        // p1 = 0x40（0100表示回送结果 0000表示唯一块） p2 = 0x02 （表示rsa2048） lc表示长度（数据长度0x80+inbuf最开始的7个字节）
        p1p2 = 0x4000;
        uint16_t in_len = 0x87;
        inbuf[5] = 0x81;
        inbuf[6] = 0x80;
        if (rsabits == 1280)
        {
            p1p2 += 0x0001;
            in_len = 0xA7;
            inbuf[6] = 0xA0;
        }
        sw = RSAPubKeyCal(p1p2, in_len, inbuf, rbuf, rlen);
        LOG_Debug("RSAPubKeyCal:sw=%04x,rlen=%d", sw, *rlen);
        if (sw != 0x9000)
            return -1;
        LOG_Debug("verify data:");
        dump_data(*rlen, rbuf);
    }
    else // rsa2048
    {
        memcpy(inbuf + 8, sigbuf, siglen);
        // p1 = 0x41（0100表示回送结果 0001表示首块） p2 = 0x02 （表示rsa2048） lc=0x88表示长度为136（128数据+inbuf最开始的8个字节）
        sw = RSAPubKeyCal(0x4102, 0x88, inbuf, rbuf, rlen);
        LOG_Debug("RSAPubKeyCal:sw=%04x,rlen=%d", sw, *rlen);
        if (sw != 0x9000)
            return -1;
        ////p1 = 0x43 （0100表示回送结果 0011表示尾块） p2 = 0x02 （表示rsa2048） lc=0x80表示长度为128
        // 尾块返回响应报文数据域，也就是全部的256字节公钥签名后的数据
        sw = RSAPubKeyCal(0x4302, 0x80, (uint8_t *)(inbuf + 0x88), rbuf, rlen);
        LOG_Debug("RSAPubKeyCal2:sw=%04x,rlen=%d", sw, *rlen);
        if (sw != 0x9000)
            return -1;
        LOG_Debug("verify data:");
        dump_data(*rlen, rbuf);
    }
    return 0;
}

int32_t HalHsmRSAKeygen(int32_t bits, BIGNUM *e, BIGNUM **n, BIGNUM **d)
{
    int sw;
    int ret;
    uint16_t rlen;
    uint8_t rbuf[300];
    uint8_t inbuf[300];
    BIGNUM *three = BN_new();
    BN_set_word(three, 3);
    // gen rsa2048 key pair
    // RSA2048 : 02(RSA2048) 00（E为65537）
    uint16_t rsabits = (bits == 2048 ? RSA2048 : (bits == 1280 ? RSA1280 : (bits == 1024 ? RSA1024 : RSA2048)));
    if (BN_cmp(e, three) == 0)
    {
        rsabits = rsabits | 0x0001;
    }
    sw = GenRSAKeyPair(rsabits, FILE_ID_11, rbuf, &rlen);
    LOG_Debug("GenRSAKeyPair:sw=%04x", sw);
    dump_data(rlen, rbuf);
    if (sw != 0x9000)
        return -1;

    // 选择公钥文件
    ret = SelectFile(PUB_FID_01, rbuf, &rlen);
    if (ret != 0x9000)
    {
        LOG_Debug("SelectFile ret = %x!", ret);
        return -1;
    }
    unsigned char raw_n[256] = {0};

    if (bits == 1280)
    {
        sw = ReadBinary(0, 0xA6, rbuf, &rlen); // 166个字节(160+2+4)
        LOG_Debug("ReadBinary:sw=%04x,rlen=%d", sw, rlen);
        if (sw != 0x9000)
            return -1;
        memcpy(inbuf, rbuf, 0xA6);
        LOG_Debug("RSA1280 public key:");
        dump_data(166, inbuf);
        memcpy(raw_n, inbuf + 2, 160);
        *n = BN_bin2bn(raw_n, 160, NULL);
    }
    else if (bits == 1024)
    {
        sw = ReadBinary(0, 0x86, rbuf, &rlen); // 134个字节(128+2+4)
        LOG_Debug("ReadBinary:sw=%04x,rlen=%d", sw, rlen);
        if (sw != 0x9000)
            return -1;
        memcpy(inbuf, rbuf, 0x86);
        LOG_Debug("RSA1280 public key:");
        dump_data(134, inbuf);
        memcpy(raw_n, inbuf + 2, 128);
        *n = BN_bin2bn(raw_n, 128, NULL);
    }
    else
    {
        // key len:256+6=262
        // 上面selete了公钥文件 所以这里直接读
        // p1和p2都是0 表示从文件首地址开始读，le为0xE0，也就是读224个字节
        sw = ReadBinary(0, 0xE0, rbuf, &rlen);
        LOG_Debug("ReadBinary:sw=%04x,rlen=%d", sw, rlen);
        if (sw != 0x9000)
            return -1;
        memcpy(inbuf, rbuf, 0xe0);
        // p1 = 0  p2 = 0xE0 表示从文件开始+0xE0的位置开始读，读0x26也就是38个字节
        sw = ReadBinary(0x00E0, 0x26, rbuf, &rlen);
        LOG_Debug("ReadBinary:sw=%04x,rlen=%d", sw, rlen);
        if (sw != 0x9000)
            return -1;
        memcpy(inbuf + 0xe0, rbuf, 0x26);
        /*SE RSA2048 PUB KEY Format:
        Length(2B)+Key(256B)+Exp(4B)!!!*/
        LOG_Debug("RSA2048 public key:");
        // 这里的262个字节的公钥就是n、e、d中的n吗（e就是65537）
        dump_data(262, inbuf);

        memcpy(raw_n, inbuf + 2, 256);
        // 将原始数据转换为 OpenSSL 的 BIGNUM 格式
        *n = BN_bin2bn(raw_n, 256, NULL);
    }
    // 私钥d 从加密芯片中拿不到
    *d = BN_new();
    return 0;
}

