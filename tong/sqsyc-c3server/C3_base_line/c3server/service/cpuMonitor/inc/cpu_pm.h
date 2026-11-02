#ifndef CPU_PM_H
#define CPU_PM_H

#include <stdint.h>
#include "cpu_info.h"

int32_t CpuPmStateMonitorInit(void* data);
int32_t CpuPmStateMonitorDeinit(void* data);

#endif /* CPU_PM_H */
