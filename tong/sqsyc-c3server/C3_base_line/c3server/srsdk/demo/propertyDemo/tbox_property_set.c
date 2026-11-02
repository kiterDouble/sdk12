#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sr_log.h"
#include "sr_ipc.h"
#include "rpc_property_api.h"

#define MODULE_NAME "tbox_property_set"

static void usage(const char *argv0)
{
    fprintf(stdout, "usage: %s <key> <value>\n", argv0 ? argv0 : "tbox_property_set");
}

int main(int argc, char *argv[])
{
    int ret = 0;
    char *propertyName = NULL;
    char *propertyValue = NULL;

    if (argc != 3)
    {
        usage(argv[0]);
        exit(-1);
    }

    if (argv[1] == NULL || argv[2] == NULL ||
        strlen(argv[1]) >= PROPERTY_RPC_KEY_MAX_LEN ||
        strlen(argv[2]) >= PROPERTY_RPC_VALUE_MAX_LEN)
    {
        fprintf(stdout, "invalid key or value\n");
        exit(-1);
    }

    propertyName = argv[1];
    propertyValue = argv[2];

    LOG_INIT(MODULE_NAME);
    LOG_Info("%s starting...", MODULE_NAME);

    ret = CreatRpcQueue();
    if (ret != 0)
    {
        LOG_Error("CreatRpcQueue failed: %d", ret);
        exit(-1);
    }

    ret = RpcPropertySet(propertyName, propertyValue);
    if (ret != 0)
    {
        fprintf(stdout, "could not set property\n");
    }

    exit(ret);
}
