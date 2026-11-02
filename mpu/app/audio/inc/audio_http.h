#ifndef _AUDIO_HTTP_H_
#define _AUDIO_HTTP_H_
#include <stdint.h>
#include <glib.h>
#include "include/civetweb.h"


#define  AUDIO_POST_SET_LICENSE_NAME    "/api/set/license/name"
#define  AUDIO_POST_GET_ACTIVE_STA      "/api/get/active/status"

void *AudioHttpThread(void *argv);

int32_t AudioBackupLicenseFile(const char *fileName);

#endif