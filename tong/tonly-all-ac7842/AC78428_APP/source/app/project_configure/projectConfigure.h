#ifndef _PROJECT_CONFIGURE_H
#define _PROJECT_CONFIGURE_H

#include <stdint.h>

#if 1 //sq
void ProjectConfigGetGacEcuPartNumber_F187(uint8_t *pData, uint16_t *pLength);
void ProjectConfigGetSupplierId_F18A(uint8_t *pData, uint16_t *pLength);
void ProjectConfigGetSupplierHdVersion_F193(uint8_t *pSerialNumber, uint16_t *pLength); 
void ProjectConfigGetChipID_F1B2(uint8_t *pData, uint16_t *pLength);
void ProjectConfigGetPublicKey_F1B3(uint8_t *pData, uint16_t *pLength);
void ProjectConfigGetTerminalActiveState_F300(uint8_t *pData, uint16_t *pLength);
void ProjectConfigGetInternalSwVersion_1214(uint8_t *pData, uint16_t *pLength);
void ProjectConfigGetExternalSwVersion_1215(uint8_t *pData, uint16_t *pLength);
void ProjectConfigHWVersion_1221(uint8_t *pData, uint16_t *pLength);

// #else
int16_t ProjectConfigGetDiagSoftwareVersion(uint8_t *pVersion, uint32_t *pLength);
int16_t ProjectConfigGetSoftwareNumber(uint8_t *pNumber, uint16_t *pLength);
void ProjectConfigGetSoftwareVersion(uint8_t *pVersion, uint32_t *pLength);
void ProjectConfigGetHardNumber(uint8_t *pNumber, uint16_t *pLength);
void ProjectConfigGetPartNumber(uint8_t *pPartNumber, uint32_t *pLength);
int16_t ProjectConfigGetSupplierIdentifier(uint8_t *pSupplierId, uint32_t *pLength);
int16_t ProjectConfigGetHardwareVersion(uint8_t *pVersion, uint32_t *pLength);
int16_t ProjectConfigGetPartName(uint8_t *pPartName, uint32_t *pLength);

int16_t ProjectConfigGetPartNameBootSoftwareVersion(uint8_t *pData, uint32_t *pLength);
int16_t ProjectConfigGetCustomVersion(uint8_t *pData, uint32_t *pLength);

//_cxl
void ProjectConfigGetGacDiagParamVersion_F10B(uint8_t *pData, uint16_t *pLength);
void ProjectConfigGetGacSparePartNumber_F17F(uint8_t *pData, uint16_t *pLength);
void ProjectConfigGetBootSwPartNumber_F180(uint8_t *pData, uint16_t *pLength);
void ProjectConfigGetGacEcuPartNumber_F187(uint8_t *pData, uint16_t *pLength);
void ProjectConfigGetEcuSwVersion_F189(uint8_t *pData, uint16_t *pLength);
void ProjectConfigSiRunSwVersion_C100(uint8_t *pData, uint16_t *pLength);
void ProjectConfigGetVisteonPartNumber_F18E(uint8_t *pData, uint16_t *pLength);
void ProjectConfigGetSupplierSwVersion_F195(uint8_t *pData, uint16_t *pLength);
void ProjectConfigGetSupplierId_F18A(uint8_t *pData, uint16_t *pLength);
void ProjectConfigGetAppSoftwareVersion_B279(uint8_t *pVersion, uint16_t *pLength);
void ProjectConfigGetGacEcuSerialNumber_F18C(uint8_t *pSerialNumber, uint16_t *pLength);
//void ProjectConfigGetGacEcuSerialNumber_F18C(uint8_t *pSerialNumber, uint16_t *pLength);
void ProjectConfigGetSupplierHdVersion_F193(uint8_t *pSerialNumber, uint16_t *pLength); // F193_cxl
#endif 
/**
 * @brief Get the combined MCU and MPU version
 * @param pVersion Pointer to store the version string
 * @param pLength Pointer to store the version string length
 * @return 0 on success
 */
int16_t ProjectConfigGetMcuMpuTotalVersion(uint8_t *pVersion, uint16_t *pLength);

/**
 * @brief Set the MPU system version
 * @param pMpuVersion Pointer to the new MPU version string
 * @param versionLen Length of the version string
 * @return 0 on success, -1 if input is invalid
 */
int16_t ProjectConfigSetMpuVersion(const uint8_t *pMpuVersion, uint16_t versionLen);

#endif //
