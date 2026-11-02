/*************************************************
 Copyright © 2025 SiRun (Beijing) . All rights reserved.
 File Name: securityHal.h
 Author:
 Created Time:
 Description:
 Others:
*************************************************/
#ifndef _SECURITYHAL_H
#define _SECURITYHAL_H

/****************************** include ***************************************/
#include <stdint.h>

/****************************** Macro Definitions ******************************/
/****************************** Type Definitions ******************************/
/****************************** Global Variables ******************************/
/****************************** Function Declarations *************************/
/****************************** Public Function Implementations ***************/
int16_t SecurityHalInit(void);
void SecurityHalSetMode(uint8_t wakeMode);
void SecurityHalTestMain(void);

#endif  // _SECURITYHAL_H