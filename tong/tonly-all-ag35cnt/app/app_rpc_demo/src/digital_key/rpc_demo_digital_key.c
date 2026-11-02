#include <stdint.h>
#include <unistd.h>

#include "sr_log.h"
#include "rpc_digital_key_api.h"

static void usage(const char* argv0){
    printf("usage: %s [-h] [-t testSection] \n", argv0);
    printf("testSection digitalkey query state      : 0  \n");
    printf("            digitalkey update firmware  : 1 fwpath  \n");
}

static void UpdateBleProgressCallBack(const DigitalKeyUpgradeProgress_t *progressData, const uint16_t dataSize, void* userdata)
{
    if(dataSize != sizeof(DigitalKeyUpgradeProgress_t)){
        printf("dataSize:%d error \n",dataSize);
        return;
    }

    printf("progress: %d/%d userData:%p \n",progressData->progress,progressData->total, userdata);
}

static int parse_test_args(int argc, char *argv[], char *test_section_arg) {
    
    if (test_section_arg == NULL) {
        printf("错误:-t 选项后必须指定 testSection  \n");
        usage(argv[0]);
        return -1;
    }
    int test_section = atoi(test_section_arg);
    int ret = 0;
 
    switch (test_section) {
        case 0:
            {
                DigitalKeyState_t keyState = {0};
                printf("执行测试:digitalkey query state \n");
                ret = RpcDigitalKeyQueryState(&keyState);
                printf("RpcDigitalKeyQueryState ret:%d  \n", ret);
                if (ret == 0){
                    printf("        advertising:%d, connected:%d, bonded:%d, pairing:%d, termiating:%d, disk:%d, pmState:%d, mtu:%d\n",
                                keyState.bleState.state.advertising,
                                keyState.bleState.state.connected,
                                keyState.bleState.state.bonded,
                                keyState.bleState.state.pairing,
                                keyState.bleState.state.termiating,
                                keyState.bleState.state.disk,
                                keyState.bleState.state.pmState,
                                keyState.bleState.state.mtu);

                    printf("        shankhand:%d, calibration:%d\n", keyState.bleState.mode.shankhand, keyState.bleState.mode.calibration);

                    printf("        fwVersion:[%s] \n",keyState.bleState.fwVersion);

                    printf("        bleMac:[%02X:%02X:%02X:%02X:%02X:%02X] \n",
                                keyState.bleState.bleMac[0], keyState.bleState.bleMac[1],
                                keyState.bleState.bleMac[2], keyState.bleState.bleMac[3],
                                keyState.bleState.bleMac[4], keyState.bleState.bleMac[5]);

                    printf("        connectedMac:[%02X:%02X:%02X:%02X:%02X:%02X] \n",
                                keyState.bleState.connectedMac[0], keyState.bleState.connectedMac[1],
                                keyState.bleState.connectedMac[2], keyState.bleState.connectedMac[3],
                                keyState.bleState.connectedMac[4], keyState.bleState.connectedMac[5]);

                    for (int i =0; i<keyState.bleState.pairedMacNum; i++){
                        printf("        pairedMac[0x%02X]: addrType:%d [%02X:%02X:%02X:%02X:%02X:%02X] \n", i, keyState.bleState.pairedMac[i].addrType,
                                keyState.bleState.pairedMac[i].mac[0], keyState.bleState.pairedMac[i].mac[1],
                                keyState.bleState.pairedMac[i].mac[2], keyState.bleState.pairedMac[i].mac[3],
                                keyState.bleState.pairedMac[i].mac[4], keyState.bleState.pairedMac[i].mac[5]);
                    }
                    printf("        otaState:%d \n", keyState.otaState);
                }
            }
            break;
        case 1:
            {
                int idx = 0;
                const char* fw_path = NULL;

                for (int i = 0; i < argc; i++) {
                    if (argv[i] == test_section_arg) {
                        idx = i;
                        break;
                    }
                }
                if (idx + 1 >= argc) {
                    printf("错误:-t 1 后必须传入固件路径参数  \n");
                    usage(argv[0]);
                    return -1;
                }
                fw_path = argv[idx + 1];
                if (fw_path == NULL || strlen(fw_path) == 0) {
                    printf("错误:固件路径不能为空  \n");
                    return -1;
                }
                printf("执行测试:digitalkey update firmware, fw_path=%s\n", fw_path);
                ret = RpcDigitalKeyUpdateBleFirmware(fw_path, 75, UpdateBleProgressCallBack, (void*) 0x1200);
                printf("RpcDigitalKeyUpdateBleFirmware ret:%d  \n", ret);
            }
            break;
        case 2:
            {
                int idx = 0;
                const char* key_Id = NULL;

                for (int i = 0; i < argc; i++) {
                    if (argv[i] == test_section_arg) {
                        idx = i;
                        break;
                    }
                }
                if (idx + 1 >= argc) {
                    printf("错误:-t 2 后必须传入keyId  \n");
                    usage(argv[0]);
                    return -1;
                }
                key_Id = argv[idx + 1];
                if (key_Id == NULL || strlen(key_Id) == 0) {
                    printf("错误keyId不能为空  \n");
                    return -1;
                }
                printf("执行测试:digitalkey del key, key_Id=%s\n", key_Id);
                ret = RpcDigitalKeyDelKey(key_Id);
                printf("RpcDigitalKeyDelKey ret:%d  \n", ret);
            }
            break;
        default:
            printf("错误:无效的 testSection %d  \n", test_section);
            usage(argv[0]);
            return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int opt;
    int ret = 0;
   
    StartQueueMsgRcvThread("app_rpc_demo");

    while ((opt = getopt(argc, argv, "ht:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            usage(argv[0]);
            exit(0);
            break;
        
        case 't':
            ret = parse_test_args(argc, argv, optarg);
            if (ret != 0) {
                exit(1);
            }
            break;
        case '?':
            printf("错误:无效的选项 -%c  \n", optopt);
            usage(argv[0]);
            exit(1);
            break;
        }
    }

    // 未传入任何有效选项时提示
    if (optind == 1) {
        printf("错误:未指定任何有效选项  \n");
        usage(argv[0]);
        exit(1);
    }

    return 0;
}
