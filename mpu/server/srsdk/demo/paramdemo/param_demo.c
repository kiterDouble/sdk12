#include "sr_param.h"
#include "sr_sdk.h"
#include "sr_log.h"
#include "err_code.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

int main()
{

    ParamInfo_t data = {0};
    int ret = 0;
    LOG_INIT("demo");

    InitSRSdk();
    while (1)
    {
        data.id = 1;
        ret = GetParam(&data);

        LOG_Debug("param:start GetParam");
        LOG_Debug("param->id:%d", data.id);
        LOG_Debug("param->name:%s", data.name);
        LOG_Debug("param->type:%d", data.type);
        LOG_Debug("param->min:%d", data.min);
        LOG_Debug("param->max:%d", data.max);
        LOG_Debug("param->paramSources:%d", data.paramSources);
        LOG_Debug("param->rpcWriteEnable:%d", data.rpcWriteEnable);
        LOG_Debug("param->value:%s", data.value);
        LOG_Debug("param:return:%d", ret);

        sleep(10);

        ret = UpdateParam(1, (uint8_t *)"888888888888888", 15);
        LOG_Debug("param:start UpdateParam");
        LOG_Debug("param:return:%d", ret);

        sleep(10);

        data.id = 101;
        strcpy((char *)data.name, "userparam1");
        data.type = 0;
        data.min = 1;
        data.max = 10;
        data.paramSources = 1;
        data.rpcWriteEnable = 1;
        strcpy((char *)data.value, "paramtestvalue1");
        ret = AddUserParam(data);
        LOG_Debug("param:start AddUserParam");
        LOG_Debug("param:return:%d", ret);

        sleep(10);

        ret = DeleteUserParam(101);
        LOG_Debug("param:start DeleteUserParam");
        LOG_Debug("param:return:%d", ret);

        sleep(10);
    }

    return 0;
}
