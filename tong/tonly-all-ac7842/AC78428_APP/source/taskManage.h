#ifndef _TASK_MANAGE_H
#define _TASK_MANAGE_H

/* Enable CAN-to-CPU SPI transparent forwarding test mode.
 * 0: normal 4-channel CAN collection + SPI forwarding to SOC (default)
 * 1: run CanToCpuTest simulation loop inside TaskVehicleDataToCpu */
#define TASK_TEST_ENABLE    0

void TasksStart(void);











#endif//_TASK_MANAGE_H