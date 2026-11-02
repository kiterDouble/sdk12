#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "sr_log.h"
#include "sr_ipc.h"
#include "rpc_property_api.h"

#define MODULE_NAME "tbox_property_get"
#define QUERY_LENGTH 10

static int PropertyRpcGetHandle(const char *name)
{
    char value[PROPERTY_RPC_VALUE_MAX_LEN] = {0};
    int32_t ret = 0;

    ret = RpcPropertyGet(name, "", value, sizeof(value));
    if ((ret == 0) && (strlen(value) > 0))
    {
        fprintf(stdout, "%.*s\n", PROPERTY_RPC_VALUE_MAX_LEN - 1, value);
        return 0;
    }
    else
    {
        fprintf(stdout, "Entry not found %.*s\n\n", PROPERTY_RPC_KEY_MAX_LEN - 1, name);
        return -1;
    }
}

static int PropertyRpcQueryHandle(void)
{
    PropertyItem_t items[QUERY_LENGTH] = {0};
    int32_t ret = 0;
    int32_t offset = 0;
    int32_t number = 0;
    int32_t size = 0;
    int32_t total = 0;

    while (1)
    {
        memset(items, 0, sizeof(items));
        number = 0;

        ret = RpcPropertyQuery(offset, QUERY_LENGTH, items, QUERY_LENGTH, &number, &size);
        if (ret != 0)
        {
            LOG_Error("RpcPropertyQuery failed: %d", ret);
            break;
        }

        if (size < 0)
        {
            LOG_Error("Invalid size: %d", size);
            ret = -1;
            break;
        }

        if (number <= 0)
        {
            break;
        }

        for (int32_t i = 0; i < number; i++)
        {
            fprintf(stdout, "[%.*s]: [%.*s]\n",
                    PROPERTY_RPC_KEY_MAX_LEN, items[i].key,
                    PROPERTY_RPC_VALUE_MAX_LEN, items[i].value);
        }

        if (total > INT32_MAX - number || offset > INT32_MAX - number)
        {
            LOG_Error("Offset/total overflow");
            ret = -1;
            break;
        }

        total += number;
        offset += number;

        if (size == 0 || total >= size)
        {
            break;
        }
    }

    return (int)ret;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    char *propertyName = NULL;

    if (argc > 1)
    {
        if (argv[1] == NULL || strlen(argv[1]) >= PROPERTY_RPC_KEY_MAX_LEN)
        {
            fprintf(stdout, "property name too long or invalid\n");
            exit(-1);
        }
        propertyName = argv[1];
    }

    LOG_INIT(MODULE_NAME);
    LOG_Info("%s starting...", MODULE_NAME);

    ret = CreatRpcQueue();
    if (ret != 0)
    {
        LOG_Error("CreatRpcQueue failed: %d", ret);
        exit(-1);
    }

    if (propertyName != NULL)
    {
        ret = PropertyRpcGetHandle(propertyName);
    }
    else
    {
        ret = PropertyRpcQueryHandle();
    }

    exit(ret);
}
