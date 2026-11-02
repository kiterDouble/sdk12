#ifndef _FILE_INTERFACE_H_
#define _FILE_INTERFACE_H_

#include <stdio.h>  


//#define LOCK_FILE "lockfile.lock"
#define  ROOTCACERTCHAINFILE               "/oemapp/cert/certificate_chain_client.pem"
#define  DEVICECACERTCHAINFILE               "/oemapp/cert/certificate_chain.pem"


#define  DEVICECERTFILE               "/oemdata/cert/device.pem"
#define  DEVICEKEYFILE                "/oemdata/cert/private.pem"


#define  CERTFILE               "/oemapp/cert/certificate.pem"
#define  KEYFILE                "/oemapp/cert/private_key.key"
#define  KEYFILEFLAG                "/oemdata/cert/private.txt"
#define  FILE_MAX_SIZE (20) // 20MB


#define  FILE_CHECK   "/oemdata/cert/check.txt"

int OpenWriteFile(const char *filepath , char *data,size_t datalen);
int OpenReadFile(const char *filepath , char *data,int datalen);
int WriteFile(FILE *file , unsigned char *data,size_t datalen);
double GetFileSize(const char *filename);
int writeIntToFile(const char *filepath, int data);
int readIntFromFile(const char *filepath, int *data);
int writeIntWithTimestampToFile(const char *filepath, int flag) ;
int ReadSpecificLine(const char *filename, int lineNumber, char *lineContent, size_t maxLen);

#endif
