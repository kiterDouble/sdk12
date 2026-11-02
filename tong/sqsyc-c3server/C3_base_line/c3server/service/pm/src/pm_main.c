#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <glib.h>

#include "cJSON.h"
#include "pm_device_control.h"
#include "sr_log.h"
#include "err_code.h"
#include "pm_sched.h"

int main()
{
    LOG_INIT("pmServer");
    LOG_Debug("pm server start !.");

    GMainLoop *mainLoop = g_main_loop_new(NULL, FALSE);
    
	PmSchedInit();
	
    g_main_loop_run(mainLoop);

	PmSchedExit();

}

