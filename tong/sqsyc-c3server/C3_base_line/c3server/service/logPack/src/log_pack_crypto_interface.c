#include "log_pack_crypto_interface.h"

/**
 * 可用加密接口注册表，NULL 结尾。
 * 新增算法时直接在此数组中添加对应接口指针即可。
 */
Crypto_interface_t* g_cryptoInterfaces[] = {
    (Crypto_interface_t*) &g_des3_crypto_interface,
    (Crypto_interface_t*) &g_aes128_crypto_interface,
    NULL,
};
