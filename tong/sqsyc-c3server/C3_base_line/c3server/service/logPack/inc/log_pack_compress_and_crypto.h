#ifndef __LOG_PACK_COMPRESS_AND_CRYPTO__
#define __LOG_PACK_COMPRESS_AND_CRYPTO__

#include "stdint.h"
#include <archive.h>
#include <archive_entry.h>

#include "log_pack_crypto_interface.h"


int CompressAndEncryptFile(Crypto_interface_t* cryptoInterface,
                           const char *inputFilePath,
                           const char *compressFilePath,
                           const char *outputFilePath,
                           const char* password);


#endif
