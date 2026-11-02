
#include <stdio.h>
#include<iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "sr_log.h"
#include "md5_interface.h"


int CalculateMd5(const char *filename, unsigned char output[MD5_DIGEST_LENGTH]) 
{

    unsigned char buffer[1024];
    size_t bytes;

    if(!filename)
    {
        LOG_Error("CalculateMd5 file NULL ");
        return -1;
    }

    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        LOG_Error("Unable to open file %s",filename);
        return -1;
    }

    MD5_CTX md5_ctx;
    MD5_Init(&md5_ctx);

    while ((bytes = fread(buffer, 1, sizeof(buffer), file)) != 0) {
        MD5_Update(&md5_ctx, buffer, bytes);
    }

    MD5_Final(output, &md5_ctx);
    fclose(file);

    return 0;

}