#ifndef __MTD_INTERFACE_H__
#define __MTD_INTERFACE_H__


#ifdef __cplusplus
extern "C" {
#endif




#define     TIME_DIFF_20    20*60 
#define     TIME_DIFF_60    60*60 
#define     TIME_DIFF_0    0 
#define     REMOVESRLOG                             "rm -rf /usrdata/log/sr.log"

long SpatialProcessingFunction(long size);

#ifdef __cplusplus
}
#endif
#endif