#include "watchdogHal.h"
#include "Wdg_Hal.h"

/* WDG timeout = 500ms (LSI ~128KHz, 64000/128 = 500ms) */
#define WDOG_TIMEOUT_MS               (500U)
#define WDOG_WINDOW_MS                (0U)

const Wdg_HalConfigType WDOG_Config = {
    .ClockSource = WDG_CLOCK_LSI,                               /* Select LSI as clock source */
    .TimeoutValue = WDOG_TIMEOUT_MS,                            /* Watchdog timeout: 500ms */
    .WindowValue = WDOG_WINDOW_MS,                              /* Window mode disabled (0) */
    .Config = WDG_CONFIG_LP_EN | WDG_CONFIG_DBG_EN,             /* Enable in LP mode + Debug mode */
};

void WatchdogHal_Init(void)
{
    Wdg_Hal_Init(&WDOG_Config);
    Wdg_Hal_Feed();
}