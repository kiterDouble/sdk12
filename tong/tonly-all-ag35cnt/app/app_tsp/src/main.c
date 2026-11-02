#include "tsp_app.h"
#include "tsp_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "sr_sdk.h"
#include "sr_ipc.h"
#include "sr_log.h"

#define DEFAULT_CONF_FILE "/oemapp/config/tlyApp.conf"

static void print_usage(const char *prog)
{
    printf("Usage: %s [options]\n", prog);
    printf("Options:\n");
    printf("  -c, --config FILE   Configuration file (default: %s)\n", DEFAULT_CONF_FILE);
    printf("  -h, --help          Show this help\n");
    printf("  -v, --version       Show version\n");
}

static void print_version(void)
{
    printf("tspApp - Tonly IOT Platform Client\n");
    printf("Version: 1.0.0\n");
    printf("Build: %s %s\n", __DATE__, __TIME__);
}

int main(int argc, char *argv[])
{
    const char *conf_file = DEFAULT_CONF_FILE;

    static struct option long_opts[] = {
        {"config",  required_argument, NULL, 'c'},
        {"help",    no_argument,       NULL, 'h'},
        {"version", no_argument,       NULL, 'v'},
        {NULL, 0, NULL, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "c:hv", long_opts, NULL)) != -1) {
        switch (opt) {
        case 'c':
            conf_file = optarg;
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        case 'v':
            print_version();
            return 0;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    LOG_INIT("tspApp");

    InitSRSdk();

    /* IPC初始化和UDS消息订阅在tly_app_init中处理 */

    printf("Starting tspApp with config: %s\n", conf_file);

    tly_app_t app;
    int rc = tly_app_init(&app, conf_file);
    if (rc != 0) {
        fprintf(stderr, "Failed to initialize (rc=%d)\n", rc);
        return 1;
    }

    rc = tly_app_run(&app);

    tly_app_destroy(&app);
    printf("tspApp exited (rc=%d)\n", rc);
    return (rc == 0) ? 0 : 1;
}
