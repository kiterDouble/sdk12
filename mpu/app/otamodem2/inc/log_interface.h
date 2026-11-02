#ifndef __LOG_INTERFACE_H__
#define __LOG_INTERFACE_H__


#ifdef __cplusplus
extern "C" {
#endif

#define     DIRECTORY                           "/usrdata/log/logstore"
#define     FILE_EXTENSION                      ".tgz"
#define     BOOTUP_LOG                          "/usrdata/log/bootup.txt"
#define     DNS_LOG                             "/usrdata/log/dns_err.log"
#define     MEASUREMENTS_LOG                    "/usrdata/log/measurements.log"
#define     STATISTICS_LOG                      "/usrdata/log/statistics.log"
#define     TRACKING_LOG                        "/usrdata/log/tracking.log"
#define     SR_LOG                              "/usrdata/log/sr.log"
#define     LOGSTORE                            "/oemdata/log/logstore"


int LogProcessingFunction(long timediff);
void RemoveMiscLog();
#ifdef __cplusplus
}
#endif
#endif