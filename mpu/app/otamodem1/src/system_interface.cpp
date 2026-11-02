#include <stdio.h>
#include<iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "sr_log.h"
#include "system_interface.h"

int GetSystemRetValue(const char *cmd)
{
    pid_t status;
    if(NULL == cmd)
    {
        return -1;
    }
    LOG_Debug("%s exe start", cmd);
    status = system(cmd);
    if(-1 == status)
    {
        return -1;
    }

    if(!WIFEXITED(status))
    {
        return -1;
    }
    return WEXITSTATUS(status);
}
