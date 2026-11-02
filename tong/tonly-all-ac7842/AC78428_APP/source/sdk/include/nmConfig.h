/*************************************************
  Copyright ? 2025 SiRun (AnHui). All rights reserved.
  File Name: nmConfig.h
  Author: 
  Created Time: 
  Description: Network Management Protocol Configuration Header
               This file provides unified configuration for switching between
               OSEK NM and AUTOSAR NM protocols across all modules.
  Others:
*************************************************/
#ifndef _NM_CONFIG_H
#define _NM_CONFIG_H

/****************************** Protocol Selection ******************************/
#define NM_PROTOCOL_AUTOSAR  1   // Use AUTOSAR NM protocol
#define NM_PROTOCOL_OSEK     0   // Use OSEK NM protocol

// Select which NM protocol to use (uncomment only one)
// #define NM_PROTOCOL_TYPE  NM_PROTOCOL_AUTOSAR   // Alternative: AUTOSAR
#define NM_PROTOCOL_TYPE     NM_PROTOCOL_OSEK     // Default: OSEK

#endif //_NM_CONFIG_H