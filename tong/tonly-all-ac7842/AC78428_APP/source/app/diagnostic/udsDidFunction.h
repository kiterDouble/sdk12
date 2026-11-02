#ifndef _UDS_DID_FUNCTION_H
#define _UDS_DID_FUNCTION_H

#include <stdint.h>
#include "stdbool.h"

uint8_t UdsDidGetManufactoryMode(void);
bool UdsCheckSecurityAccessValid(uint8_t requiredLevel);
#if 1 //sq
// serive 22 read function
int16_t Service22ReadFingerprintEthernetF10A(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadEcuPartNumberF187(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadSupplierIdF18A(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadManufactureDateF18B(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadSerialNumberF18C(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadVINF190(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadSupplierECUHardwareVersionF193(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadSupplierECUSoftwareVersionF195(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadVehicleManufactureDateF1B7(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadSWWrittenStatusF200(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadConfigurationDataVersionF1B0(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadPINF1B1(uint8_t *pData, uint16_t *pLength);
int16_t ServiceReadManufactoryMode(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadChipIdF1B2(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadPublicKeyF1B3(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadTerminalActiveStateF300(uint8_t *pData, uint16_t *pLength);
// EOL DID
int16_t Service22ReadLowVoltBatteryTypeF201(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadVoltPlatformF202(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadEthRoleF203(uint8_t *pData, uint16_t *pLength);

// service 2E write function
int16_t Service2EWriteFingerprintEthernetF10A(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteAppSoftwareFingerprint(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteVINF190(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteManufactureDateF1B7(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWritePINF1B1(uint8_t *pData, uint16_t dataLength);
// EOL DID
int16_t Service2EWriteLowVoltBatteryTypeF201(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteVoltPlatformF202(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteEthRoleF203(uint8_t *pData, uint16_t dataLength);

//EOL Write function
int16_t ServiceWriteManufactureDateF18B(uint8_t *pData, uint16_t dataLength);
int16_t ServiceWriteSerialNumberF18C(uint8_t *pData, uint16_t dataLength);
int16_t ServiceWriteManufactoryMode(uint8_t *pData, uint16_t dataLength);
#else
// serive 22 read function
int16_t Service22ReadPartNumber(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadSupplierId(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadHardwareNumber(uint8_t *pData, uint16_t *pLength);
// int16_t Service22ReadSupplierHardwareNumber(uint8_t *pData,uint16_t *pLength);
int16_t Service22ReadSoftwareNumber(uint8_t *pData, uint16_t *pLength);
// int16_t Service22ReadBootSoftwareVersion(uint8_t *pData,uint16_t *pLength);
int16_t Service22ReadAppSoftwareVersion(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadHardwareVersion(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadSoftwareVersion(uint8_t *pData, uint16_t *pLength);
// int16_t Service22ReadSupplierSoftwareNumber(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadEcuMask(uint8_t *pData,uint16_t *pLength);
int16_t Service22ReadManufactureDate(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadPartName(uint8_t *pData, uint16_t *pLength);
// int16_t Service22ReadESKey(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadIpAddress(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadIpPort(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadPublicKey(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadTboxEcallNumber(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadTboxBcallNumber(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadTboxIcallNumber(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadTspCallNumber1(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadTspCallNumber2(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadTspCallNumber3(uint8_t *pData,uint16_t *pLength);
int16_t Service22ReadVIN(uint8_t *pData, uint16_t *pLength);
// int16_t Service22ReadBID(uint8_t *pData,uint16_t *pLength);
int16_t Service22ReadSerialNumber(uint8_t *pData, uint16_t *pLength); // F18F
int16_t Service22ReadICCID(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadIMEI(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadIMSI(uint8_t *pData, uint16_t *pLength);
// int16_t Service22ReadTboxPreKey(uint8_t *pData,uint16_t *pLength);
int16_t Service22ReadTboxCallNumber(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadRegisterFlag(uint8_t *pData, uint16_t *pLength);
// int16_t Service22ReadKeyType(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadTboxMsgNumber1(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadTboxMsgNumber2(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadCanUploadCycle(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadTboxModel(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadCarModeType(uint8_t *pData,uint16_t *pLength);
int16_t Service22ReadSecurityVersion(uint8_t *pData, uint16_t *pLength);

int16_t Service22ReadWorkingMode(uint8_t *pData, uint16_t *pLength);

int16_t Service22ReadFingerPrint(uint8_t *pData, uint16_t *pLength);
/************add 22 *******************************/
int16_t Service22ReadSubnetConfigListOnHighSpeedCan(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadVehicleManufacturingDate(uint8_t *pData, uint16_t *pLength);
// int16_t Service22ReadVariantcode(uint8_t *pData,uint16_t *pLength);

int16_t Service22ReadEOLconfig(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadNationIpAddress(uint8_t *pData, uint16_t *pLength);
// int16_t Service22ReadNationIpPort(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadGovernmentIpAddress(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadGovernmentIpPort(uint8_t *pData,uint16_t *pLength);

// int16_t Service22ReadNationIpAddress_DIG(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadGovernmentIpAddress_DIG(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadIpAddress_DIG(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadInternalBatteryStatus(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadGNSS_test(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadNetWorkProvider_test(uint8_t* buf, uint16_t *len);
int16_t Service22ReadPhoneSignalTest(uint8_t *pData, uint16_t *pLength);
/************add 22 *******************************/
int16_t Service22ReadBatteryStatus(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadKL30Voltage(uint8_t *pData, uint16_t *pLength);
// int16_t Service22ReadKL30ConnectStatus(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadBatteryVoltage(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadBatteryTemperature(uint8_t *pData,uint16_t *pLength);

// int16_t Service22ReadGNSSSLocationStatus(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadGNSSStatus(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadGNSSSAntenaStatus(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadGNSS_LatitudeLongitude(uint8_t *pData,uint16_t *pLength);

// int16_t Service22ReadGPRSModeStatus(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadGPRSNetWorkStatus(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadNADPrimaryAntenaStatus(uint8_t *pData,uint16_t *pLength);

// int16_t Service22ReadCANBUSCommunicationStatus(uint8_t *pData,uint16_t *pLength);
int16_t Service22ReadUploadFrequency(uint8_t *pData, uint16_t *pLength);
// int16_t Service22ReadStorageMemoryUsages(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadStorageMemoryStatus(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadBackupBatterySoc(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadBackupBatteryStatus(uint8_t *pData,uint16_t *pLength);

int16_t Service22ReadPhoneSignal(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadNetWorkProvider(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadNetWorkType(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadApnNumber(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadCpuFlashCapacity(uint8_t *pData, uint16_t *pLength);

// int16_t Service22ReadBatteryTemperature(uint8_t *pData,uint16_t *pLength);
int16_t Service22ReadGNSS(uint8_t *pData, uint16_t *pLength);
//
// int16_t Service22ReadTboxTemperature(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadSystemTime(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadNetworkMCC(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadNetworkMNC(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadConnectionType(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadModemTemperature(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadGNSSLiveTime(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadGNSSLiveHeigth(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadGNSSLiveHDOP(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadGNSSLiveSpeed(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadGNSSLiveHeading(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadGNSSCurrentNumOfSatallitesUsed(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadGNSSCurrentNumOfSatallitesInView(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadBackupBatteryTemperature(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadBackupBatteryAgeCoefficient(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadBackupBatteryVoltage(uint8_t *pData,uint16_t *pLength);

// int16_t Service22ReadIMMOStatus(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadPowerIn(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadEepromCheck(uint8_t *pData,uint16_t *pLength);
int16_t Service22ReadDiagnosticCanReport(uint8_t *pData, uint16_t *pLength);

// test
// int16_t Service22ReadPinIN_1Status(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadPinIN_2Status(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadPinKL15Status(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadPinSRSStatus(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadPinEcallStatus(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadBatteryChargeState(uint8_t *pData,uint16_t *pLength);
int16_t Service22ReadAppSoftwareFingerprint(uint8_t *pData, uint16_t *pLength);
// int16_t Service22ReadUCUType(uint8_t *pData,uint16_t *pLength);

// int16_t Service22ReadPhoneKeyOffLineUseTimes(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadNFCCardReaderID(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadSEID(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadProlinesChannelKey3(uint8_t *pData,uint16_t *pLength);
// int16_t Service22ReadBleMac(uint8_t *pData,uint16_t *pLength);

// service 2E write function
// int16_t Service2EWriteProlinesChannelKey(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWritePhoneKeyOffLineUseTimes(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteNFCCardID(uint8_t *pData,uint16_t dataLength);

int16_t Service2EWritePartNumber(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteSupplierId(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteHardwareVersion(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteSoftwareVersion(uint8_t *pData, uint16_t dataLength);
// int16_t Service2EWriteEcuMask(uint8_t *pData,uint16_t dataLength);
int16_t Service2EWriteManufactureDate(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWritePartName(uint8_t *pData, uint16_t dataLength);

int16_t Service2EWriteESKey(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteESKeyByEol(uint8_t *pData, uint16_t dataLength);
// int16_t Service2EWriteIpAddress(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteIpPort(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWritePublicKey(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteTboxEcallNumber(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteTboxBcallNumber(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteTboxIcallNumber(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteTSPCallNumber1(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteTSPCallNumber2(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteTSPCallNumber3(uint8_t *pData,uint16_t dataLength);
int16_t Service2EWriteVIN(uint8_t *pData, uint16_t dataLength);
// int16_t Service2EWriteBID(uint8_t *pData,uint16_t dataLength);
int16_t Service2EWriteSerialNumber(uint8_t *pData, uint16_t dataLength);
// int16_t Service2EWriteICCID(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteIMEI(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteIMSI(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteTboxPreKey(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteTboxCallNumber(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteTboxRegisterFlag(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteKeyType(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteTboxMsgNumber1(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteTboxMsgNumber2(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteCanUploadCycle(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteTboxModel(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteCarModeType(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteSecurityVersion(uint8_t *pData,uint16_t dataLength);

int16_t Service2EWriteAppSoftwareFingerprint(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteFingerPrint(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteSubnetConfigListOnHighSpeedCan(uint8_t *pData, uint16_t dataLength);

int16_t Service2EWriteWorkingMode(uint8_t *pData, uint16_t dataLength);
// ADD 2e
int16_t Service2EWriteNationIpAddress(uint8_t *pData, uint16_t dataLength);
// int16_t Service2EWriteNationIpPort(uint8_t *pData,uint16_t dataLength);
int16_t Service2EWriteGovernmentIpAddress(uint8_t *pData, uint16_t dataLength);
// int16_t Service2EWriteGovernmentIpPort(uint8_t *pData,uint16_t dataLength);
int16_t Service2EWriteRepairShopCodeOrTestSN(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteInstallationDate(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteEOLconfig(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteEOLconfigByEol(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteVariantcode(uint8_t *pData, uint16_t dataLength);

// int16_t Service2EWriteIpAddress_DIG(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteNationIpAddress_DIG(uint8_t *pData,uint16_t dataLength);
// int16_t Service2EWriteGovernmentIpAddress_DIG(uint8_t *pData,uint16_t dataLength);

int16_t Service2EWriteHardwareNumber(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteSoftwareNumber(uint8_t *pData, uint16_t dataLength);

int16_t Service2EWriteDiagnosticCanReport(uint8_t *pData, uint16_t dataLength);

int16_t Service2EWriteESKey(uint8_t *pData, uint16_t dataLength);                       // 0x010D_cxl
int16_t Service2EWriteApn1(uint8_t *pData, uint16_t dataLength);                        // 0x011B_cxl
int16_t Service2EWriteIp1Addr(uint8_t *pData, uint16_t dataLength);                     // 0x011C_cxl
int16_t Service2EWriteTspPort(uint8_t *pData, uint16_t dataLength);                     // 0x011D_cxl
int16_t Service2EWriteTspTimeout(uint8_t *pData, uint16_t dataLength);                  // 0xB209_cxl
int16_t Service2EWriteECallNum1(uint8_t *pData, uint16_t dataLength);                   // 0x011F_cxl
int16_t Service2EWriteBCallNum1(uint8_t *pData, uint16_t dataLength);                   // 0x0124_cxl
int16_t Service2EWriteOfficialServiceNum(uint8_t *pData, uint16_t dataLength);          // 0xB20A_cxl
int16_t Service2EWriteEmergencyAsNum(uint8_t *pData, uint16_t dataLength);              // 0xB20B_cxl
int16_t Service2EWriteTboxPhoneNum(uint8_t *pData, uint16_t dataLength);                // 0x0129_cxl
int16_t Service2EWriteTspFlowTimeout(uint8_t *pData, uint16_t dataLength);              // 0xB20C_cxl
int16_t Service2EWriteTspAckTime(uint8_t *pData, uint16_t dataLength);                  // 0xB20D_cxl
int16_t Service2EWriteApn2(uint8_t *pData, uint16_t dataLength);                        // 0x013C_cxl
int16_t Service2EWriteIp2Addr(uint8_t *pData, uint16_t dataLength);                     // 0xB20E_cxl
int16_t Service2EWritePort2(uint8_t *pData, uint16_t dataLength);                       // 0xB20F_cxl
int16_t Service2EWriteIp3Addr(uint8_t *pData, uint16_t dataLength);                     // 0x105F_cxl
int16_t Service2EWriteLongConnCycle(uint8_t *pData, uint16_t dataLength);               // 0xB211_cxl
int16_t Service2EWriteNormalUploadPeriod(uint8_t *pData, uint16_t dataLength);          // 0xB229_cxl
int16_t Service2EWriteAlarmUploadPeriod(uint8_t *pData, uint16_t dataLength);           // 0xB22A_cxl
int16_t Service2EWriteTspDomainLen(uint8_t *pData, uint16_t dataLength);                // 0xB22B_cxl
int16_t Service2EWriteTspDomain(uint8_t *pData, uint16_t dataLength);                   // 0xB22C_cxl
int16_t Service2EWriteTspPortNumeric(uint8_t *pData, uint16_t dataLength);              // 0xB22D_cxl
int16_t Service2EWritePublicTspDomain(uint8_t *pData, uint16_t dataLength);             // 0x1014_cxl
int16_t Service2EWriteApn1Type(uint8_t *pData, uint16_t dataLength);                    // 0xB247_cxl
int16_t Service2EWriteApn2Type(uint8_t *pData, uint16_t dataLength);                    // 0xB248_cxl
int16_t Service2EWriteTspConnectSetting(uint8_t *pData, uint16_t dataLength);           // 0xB258_cxl
int16_t Service2EWriteEmmcDeleteControl(uint8_t *pData, uint16_t dataLength);           // 0xB259_cxl
int16_t Service2EWritePkiApply(uint8_t *pData, uint16_t dataLength);                    // 0xB261_cxl
int16_t Service2EWriteTspDomain1(uint8_t *pData, uint16_t dataLength);                  // 0x031C_cxl
int16_t Service2EWriteApn3(uint8_t *pData, uint16_t dataLength);                        // 0x1061_cxl
int16_t Service2EWriteOtaDomainName(uint8_t *pData, uint16_t dataLength);               // 0xB2CB_cxl
int16_t Service2EWriteOtaPort(uint8_t *pData, uint16_t dataLength);                     // 0xB2CC_cxl
int16_t Service2EWritePkiDomainName(uint8_t *pData, uint16_t dataLength);               // 0x1052_Cxl
int16_t Service2EWritePkiPort(uint8_t *pData, uint16_t dataLength);                     // 0x1053_Cxl
int16_t Service2EWriteTspPort3(uint8_t *pData, uint16_t dataLength);                    // 0x105E_cxl
int16_t Service2EWriteLogDomainName(uint8_t *pData, uint16_t dataLength);               // 0xB275_cxl
int16_t Service2EWriteLogPort(uint8_t *pData, uint16_t dataLength);                     // 0xB276_cxl
int16_t Service2EWriteLogConnectionCommand(uint8_t *pData, uint16_t dataLength);        // 0xB277_cxl
int16_t Service2EWriteEthernetConfig(uint8_t *pData, uint16_t dataLength);              // 0xB212_cxl
int16_t Service2EWritePkiCertCycle(uint8_t *pData, uint16_t dataLength);                // 0xB278_cxl
int16_t Service2EWriteLogAccount(uint8_t *pData, uint16_t dataLength);                  // 0xB27C_cxl
int16_t Service2EWriteLogPassword(uint8_t *pData, uint16_t dataLength);                 // 0xB27D_cxl
int16_t Service2EWriteLogUploadChannel(uint8_t *pData, uint16_t dataLength);            // 0xB27F_cxl
int16_t Service2EWriteLogDomainFront(uint8_t *pData, uint16_t dataLength);              // 0xB280_cxl
int16_t Service2EWriteLogDomainLater(uint8_t *pData, uint16_t dataLength);              // 0xB281_cxl
int16_t Service2EWriteLogPath(uint8_t *pData, uint16_t dataLength);                     // 0xB282_cxl
int16_t Service2EWriteLogPath_P1(uint8_t *pData, uint16_t dataLength);                  // 0xB283_cxl
int16_t Service2EWriteLogPath_P2(uint8_t *pData, uint16_t dataLength);                  // 0xB284_cxl
int16_t Service2EWriteLogPath_P3(uint8_t *pData, uint16_t dataLength);                  // 0xB285_cxl
int16_t Service2EWriteLogPath_P4(uint8_t *pData, uint16_t dataLength);                  // 0xB286_cxl
int16_t Service2EWriteApn3Type(uint8_t *pData, uint16_t dataLength);                    // 0xB28B_cxl
int16_t Service2EWriteSensitiveDataSet(uint8_t *pData, uint16_t dataLength);            // 0xB2C5_cxl
int16_t Service2EWriteOtaRtcWakeupTime(uint8_t *pData, uint16_t dataLength);            // 0xB2C6_cxl
int16_t Service2EWriteBuryingPointSwitch(uint8_t *pData, uint16_t dataLength);          // 0xB2C7_cxl
int16_t Service2EWriteAdbStatus(uint8_t *pData, uint16_t dataLength);                   // 0xB296_cxl
int16_t Service2EWriteConnectionTestAddr(uint8_t *pData, uint16_t dataLength);          // 0xB297_cxl
int16_t Service2EWriteDataBuriedPointDomain(uint8_t *pData, uint16_t dataLength);       // 0xB289_cxl
int16_t Service2EWriteDataBuriedPointDomain_P1(uint8_t *pData, uint16_t dataLength);    // 0xB29C_cxl
int16_t Service2EWriteDataBuriedPointDomain_P2(uint8_t *pData, uint16_t dataLength);    // 0xB29D_cxl
int16_t Service2EWriteGnssGalaxy(uint8_t *pData, uint16_t dataLength);                  // 0xB2C9_cxl
int16_t Service2EWriteOtaBookingTime(uint8_t *pData, uint16_t dataLength);              // 0xB2CA_cxl
int16_t Service2EWriteOtaSwAcqTime(uint8_t *pData, uint16_t dataLength);                // 0xB2CD_cxl
int16_t Service2EWriteJtagStatus(uint8_t *pData, uint16_t dataLength);                  // 0xB2E5_cxl
int16_t Service2EWriteFunctionConfig(uint8_t *pData, uint16_t dataLength);              // 0xB2E6_cxl
int16_t Service2EWriteTransportMode(uint8_t *pData, uint16_t dataLength);               // 0xB2B4_cxl
int16_t Service2EWriteManufactoryMode(uint8_t *pData, uint16_t dataLength);             // 0x0110_cxl
int16_t Service2EWriteDtcSettingControl(uint8_t *pData, uint16_t dataLength);           // 0x0120_cxl
int16_t Service2EWriteICCID(uint8_t *pData, uint16_t dataLength);                       // 0x0111_cxl
int16_t Service2EWriteIMEI(uint8_t *pData, uint16_t dataLength);                        // 0x010E_cxl
int16_t Service2EWriteIMSI(uint8_t *pData, uint16_t dataLength);                        // 0x010F_cxl
int16_t Service2EWriteRtcWakeupSet(uint8_t *pData, uint16_t dataLength);                // 0xB208_cxl
int16_t Service2EWrite4GModuleRsetTime(uint8_t *pData, uint16_t dataLength);            // 0xB25F_cxl
int16_t Service2EWriteReprogramCounter(uint8_t *pData, uint16_t dataLength);            // 0x0200_cxl
int16_t Service2EWriteReprogramAttemptCounter(uint8_t *pData, uint16_t dataLength);     // 0x0201_cxl
int16_t Service2EWriteTcpdumpSwitch(uint8_t *pData, uint16_t dataLength);               // 0xB2C8_cxl
int16_t Service2EWriteTspPlatformConnectionStatus(uint8_t *pData, uint16_t dataLength); // 0xB28E_cxl
int16_t Service2EWritePKIapply(uint8_t *pData, uint16_t dataLength);                    // 0xB261_cxl
int16_t Service2ETboxCurrentRunningMode(uint8_t *pData, uint16_t dataLength);           // 0xB25c_cxl
int16_t Service2EWriteGacEcuPartNumber(uint8_t *pData, uint16_t dataLength);              // 0xF187
// service 31 start routinue function
// int16_t Service31CheckPreFlashConditon(uint8_t *pDataIn,uint16_t lengthIn,uint8_t *pDataOut,uint16_t *pLengthOut);
int16_t Service31StopApplication(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
void Service31StopApplicationSetDefault(void);
// int16_t Service31WriteDiagnosticCanReport01(uint8_t *pData,uint16_t dataLength,uint8_t *pDataOut,uint16_t *pLengthOut);
// int16_t Service31WriteDiagnosticCanReport02(uint8_t *pData,uint16_t dataLength,uint8_t *pDataOut,uint16_t *pLengthOut);
// int16_t Service31WriteDiagnosticCanReport03(uint8_t *pData,uint16_t dataLength,uint8_t *pDataOut,uint16_t *pLengthOut);
void Service31WriteDiagnosticCanReportSetDefault(void);
// 28,85
// uint8_t Service28CommunicationControl(uint8_t ContolType, uint8_t SubService);
// uint8_t Service85DtcControl(uint8_t subFunction);
// void Service10ResetTboxStatusUpdate(void);
//

// void CheckEolConfigureState(void);
void EolConfigureInit(void);
// const uint8_t *GetVehicleTypeConfigure(void);

// void DiagCanReportValueInit(void);
// void CheckCanReportState(void);

void VinInit(void);
void CheckVinCyleProcess(void);

void EskkeyCheckInit(void);
// void CheckEskValidtateDtc(void);

/*====================Great Wall=================*/
/*read and write*/
int16_t Service2EWriteVehicleNetworkConfigurationF1A1(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteVehicleNetworkConfigurationF1B0(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteECUlevelNetworkConfigurationDataIdentifierF1B1(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteVehicleModelF112(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteFingerprintF1F0(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteFingerprintEthernetF0FF(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteVehicleSoftwareVersionF1AB(uint8_t *pData, uint16_t dataLength);
// int16_t Service2EWriteWorkingModeF1C2(uint8_t *pData,uint16_t dataLength);
int16_t Service2EWriteMaintenanceModeFD00(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteCertificateF1CB(uint8_t *pData, uint16_t dataLength);
int16_t Service2EWriteManufactureDateF18B(uint8_t *pData, uint16_t dataLength);

int16_t Service2EWriteTripRestAA11(uint8_t *pData, uint16_t dataLength);

/*just write*/
int16_t Service2EWriteSecOCKeyF1C7(uint8_t *pData, uint16_t dataLength); // SecOC��Կ HEX

int16_t Service2EWriteSK(uint8_t *pData, uint16_t dataLength);

/*Read*/
int16_t Service22ReadFingerprintEthernetF0FF(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadVehicleModelF112(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadVehicleNetworkConfigurationF1A1(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadCANmatrixVersionF1A2(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadVehicleSoftwareVersionF1AB(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadVehicleNetworkConfigurationF1B0(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadECUlevelNetworkConfigurationDataIdentifierF1B1(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadVehicleManufacturerECUSoftwareVersionNumberDataIdentifierF189(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadNodeAddressF1B3(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadCompID1F1B4(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadNameF1B5(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadToolVersionF1B6(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadCompID2F1B9(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadSoftwareInformationF1BC(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadHardwareInformationF1BD(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadCalibrationSoftwareVersionF1C0(uint8_t *pData, uint16_t *pLength);
// int16_t Service22ReadWorkingModeF1C2(uint8_t *pData,uint16_t *pLength);
int16_t Service22ReadUINF1C5(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadSecocKeyCC77(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadSecOCVersionNumberF1C8(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadCsrF1C9(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadPkiStateF1CA(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadCertificateSerialNumberF1CC(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadCsrHashF1CD(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadSecocVersion(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadFingerprintF1F0(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadECUTypeF1D9(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadMaintenanceModeFD00(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadGPSRNetworkStatus600D(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadGPSRNetworkSignalStrength600E(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadSK(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadTripRestAA11(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadEmmcStateAA01(uint8_t *pData, uint16_t *pLength);
int16_t Service22ReadGacDiagParamVersion(uint8_t *pData, uint16_t *pLength);                                           // 0xF10B_cxl
int16_t Service22ReadGacSparePartNumber(uint8_t *pData, uint16_t *pLength);                                            // 0xF17F_cxl
int16_t Service22ReadBootSoftwarePartNumber(uint8_t *pData, uint16_t *pLength);                                        // 0xF180_cxl
int16_t Service22ReadGacEcuPartNumber(uint8_t *pData, uint16_t *pLength);                                              // 0xF187_cxl
int16_t Service22ReadVehicleManufacturerECUSoftwareVersionNumberDataIdentifierF189(uint8_t *pData, uint16_t *pLength); // 0xF189_cxl
int16_t Service22ReadGacVisteonInHousePartNumber(uint8_t *pData, uint16_t *pLength);                                   // 0xF18E_cxl
int16_t Service22ReadSupplierECUHardwareVersion(uint8_t *pData, uint16_t *pLength);                                    // 0xF193_cxl
int16_t Service22ReadSupplierECUSoftwareVersion(uint8_t *pData, uint16_t *pLength);                                    // 0xF195_cxl
int16_t Service22ReadPowerVoltage(uint8_t *pData, uint16_t *pLength);                                                  // 0x1000_cxl
int16_t Service22ReadSupplierId(uint8_t *pData, uint16_t *pLength);                                                    // 0xF18A_cxl
int16_t Service22Read4G3GStatus(uint8_t *pData, uint16_t *pLength);                                                    // 0xB201_cxl
int16_t Service22ReadGpsStatus(uint8_t *pData, uint16_t *pLength);                                                     // 0xB202_cxl
int16_t Service22ReadSystemTime(uint8_t *pData, uint16_t *pLength);                                                    // 0xB203_cxl
int16_t Service22ReadBcallEcallKeyStatus(uint8_t *pData, uint16_t *pLength);                                           // 0xB204_cxl
int16_t Service22ReadVehicleSpeed(uint8_t *pData, uint16_t *pLength);                                                  // 0xB205_cxl
int16_t Service22ReadEngineMotorSpeed(uint8_t *pData, uint16_t *pLength);                                              // 0xB206_cxl
int16_t Service22ReadESKey(uint8_t *pData, uint16_t *pLength);                                                         // 0x010D_cxl
int16_t Service22ReadESKWrittenStatus(uint8_t *pData, uint16_t *pLength);                                              // 0x0204_cxl
int16_t Service22ReadIMEI(uint8_t *pData, uint16_t *pLength);                                                          // 0x010E_cxl
int16_t Service22ReadApn1(uint8_t *pData, uint16_t *pLength);                                                          // 0x011B_cxl
int16_t Service22ReadIp1Addr(uint8_t *pData, uint16_t *pLength);                                                       // 0x011C_cxl
int16_t Service22ReadTspPort(uint8_t *pData, uint16_t *pLength);                                                       // 0x011D_cxl
int16_t Service22ReadTspTimeout(uint8_t *pData, uint16_t *pLength);                                                    // 0xB209_cxl
int16_t Service22ReadECallNum1(uint8_t *pData, uint16_t *pLength);                                                     // 0x011F_cxl
int16_t Service22ReadBCallNum1(uint8_t *pData, uint16_t *pLength);                                                     // 0x0124_cxl
int16_t Service22ReadOfficialServiceNum(uint8_t *pData, uint16_t *pLength);                                            // 0xB20A_cxl
int16_t Service22ReadEmergencyAsNum(uint8_t *pData, uint16_t *pLength);                                                // 0xB20B_cxl
int16_t Service22ReadTboxPhoneNum(uint8_t *pData, uint16_t *pLength);                                                  // 0x0129_cxl
int16_t Service22ReadTspFlowTimeout(uint8_t *pData, uint16_t *pLength);                                                // 0xB20C_cxl
int16_t Service22ReadTspAckTime(uint8_t *pData, uint16_t *pLength);                                                    // 0xB20D_cxl
int16_t Service22ReadApn2(uint8_t *pData, uint16_t *pLength);                                                          // 0x013C_cxl
int16_t Service22ReadIp2Addr(uint8_t *pData, uint16_t *pLength);                                                       // 0xB20E_cxl
int16_t Service22ReadPort2(uint8_t *pData, uint16_t *pLength);                                                         // 0xB20F_cxl
int16_t Service22Read4GNetworkStatus(uint8_t *pData, uint16_t *pLength);                                               // 0xB210_cxl
int16_t Service22ReadIp3Addr(uint8_t *pData, uint16_t *pLength);                                                       // 0x105F_cxl
int16_t Service22ReadLongConnCycle(uint8_t *pData, uint16_t *pLength);                                                 // 0xB211_cxl
int16_t Service22ReadEthernetConfig(uint8_t *pData, uint16_t *pLength);                                                // 0xB212_cxl
int16_t Service22ReadNormalUploadPeriod(uint8_t *pData, uint16_t *pLength);                                            // 0xB229_cxl
int16_t Service22ReadAlarmUploadPeriod(uint8_t *pData, uint16_t *pLength);                                             // 0xB22A_cxl
int16_t Service22ReadTspDomainLen(uint8_t *pData, uint16_t *pLength);                                                  // 0xB22B_cxl
int16_t Service22ReadTspDomain(uint8_t *pData, uint16_t *pLength);                                                     // 0xB22C_cxl
int16_t Service22ReadTspPortNumeric(uint8_t *pData, uint16_t *pLength);                                                // 0xB22D_cxl
int16_t Service22ReadPublicTspDomain(uint8_t *pData, uint16_t *pLength);                                               // 0x1014_cxl
int16_t Service22ReadTboxWakeupSource(uint8_t *pData, uint16_t *pLength);                                              // 0xB245_cxl
int16_t Service22ReadApn1Type(uint8_t *pData, uint16_t *pLength);                                                      // 0xB247_cxl
int16_t Service22ReadApn2Type(uint8_t *pData, uint16_t *pLength);                                                      // 0xB248_cxl
int16_t Service22ReadGpsSatelliteInfo(uint8_t *pData, uint16_t *pLength);                                              // 0xB249_cxl
int16_t Service22ReadEmmcState(uint8_t *pData, uint16_t *pLength);                                                     // 0xB24A_cxl
int16_t Service22Read4gFirmwareVersion(uint8_t *pData, uint16_t *pLength);                                             // 0xB24C_cxl
int16_t Service22Read4gHardwareVersion(uint8_t *pData, uint16_t *pLength);                                             // 0xB24D_cxl
int16_t Service22Read4gFunctionState(uint8_t *pData, uint16_t *pLength);                                               // 0xB24E_cxl
int16_t Service22ReadEthernetId(uint8_t *pData, uint16_t *pLength);                                                    // 0xB256_cxl
int16_t Service22ReadTspConnectSetting(uint8_t *pData, uint16_t *pLength);                                             // 0xB258_cxl
int16_t Service22ReadEmmcDeleteStatus(uint8_t *pData, uint16_t *pLength);                                              // 0xB259_cxl
int16_t Service22Read4gAppVersion(uint8_t *pData, uint16_t *pLength);                                                  // 0xB25B_cxl
int16_t Service22ReadPkiApply(uint8_t *pData, uint16_t *pLength);                                                      // 0xB261_cxl
int16_t Service22ReadPkiCertStatus(uint8_t *pData, uint16_t *pLength);                                                 // 0xB262_cxl
int16_t Service22ReadGnssAntennaVoltage(uint8_t *pData, uint16_t *pLength);                                            // 0xB266_cxl
int16_t Service22ReadOpenCpuTemperature(uint8_t *pData, uint16_t *pLength);                                            // 0xB271_cxl
int16_t Service22ReadLogDomainName(uint8_t *pData, uint16_t *pLength);                                                 // 0xB275_cxl
int16_t Service22ReadLogPort(uint8_t *pData, uint16_t *pLength);                                                       // 0xB276_cxl
int16_t Service22ReadLogConnectionCommand(uint8_t *pData, uint16_t *pLength);                                          // 0xB277_cxl
int16_t Service22ReadPkiCertCycle(uint8_t *pData, uint16_t *pLength);                                                  // 0xB278_cxl
int16_t Service22ReadPkiSdkVersion(uint8_t *pData, uint16_t *pLength);                                                 // 0xB279_cxl
int16_t Service22ReadLogAccount(uint8_t *pData, uint16_t *pLength);                                                    // 0xB27C_cxl
int16_t Service22ReadLogPassword(uint8_t *pData, uint16_t *pLength);                                                   // 0xB27D_cxl
int16_t Service22ReadLogUploadChannel(uint8_t *pData, uint16_t *pLength);                                              // 0xB27F_cxl
int16_t Service22ReadLogDomainFront(uint8_t *pData, uint16_t *pLength);                                                // 0xB280_cxl
int16_t Service22ReadLogDomainLater(uint8_t *pData, uint16_t *pLength);                                                // 0xB281_cxl
int16_t Service22ReadLogPath(uint8_t *pData, uint16_t *pLength);                                                       // 0xB282_cxl
int16_t Service22ReadLogPath_P1(uint8_t *pData, uint16_t *pLength);                                                    // 0xB283_cxl
int16_t Service22ReadLogPath_P2(uint8_t *pData, uint16_t *pLength);                                                    // 0xB284_cxl
int16_t Service22ReadLogPath_P3(uint8_t *pData, uint16_t *pLength);                                                    // 0xB285_cxl
int16_t Service22ReadLogPath_P4(uint8_t *pData, uint16_t *pLength);                                                    // 0xB286_cxl
int16_t Service22ReadPwmSignal(uint8_t *pData, uint16_t *pLength);                                                     // 0xB287_cxl
int16_t Service22ReadTspPort3(uint8_t *pData, uint16_t *pLength);                                                      // 0x105E_cxl
int16_t Service22ReadApn3(uint8_t *pData, uint16_t *pLength);                                                          // 0x1061_cxl
int16_t Service22ReadApn3Type(uint8_t *pData, uint16_t *pLength);                                                      // 0xB28B_cxl
int16_t Service22ReadSensitiveDataSet(uint8_t *pData, uint16_t *pLength);                                              // 0xB2C5_cxl
int16_t Service22ReadOtaRtcWakeupTime(uint8_t *pData, uint16_t *pLength);                                              // 0xB2C6_cxl
int16_t Service22ReadBuryingPointSwitch(uint8_t *pData, uint16_t *pLength);                                            // 0xB2C7_cxl
int16_t Service22ReadTspConnectionStatus(uint8_t *pData, uint16_t *pLength);                                           // 0xB28E_cxl
int16_t Service22ReadGpsDiagInfo(uint8_t *pData, uint16_t *pLength);                                                   // 0xB290_cxl
int16_t Service22ReadAdbStatus(uint8_t *pData, uint16_t *pLength);                                                     // 0xB296_cxl
int16_t Service22ReadConnectionTestAddr(uint8_t *pData, uint16_t *pLength);                                            // 0xB297_cxl
int16_t Service22ReadDataBuriedPointDomain(uint8_t *pData, uint16_t *pLength);                                         // 0xB289_cxl
int16_t Service22ReadDataBuriedPointDomain_P1(uint8_t *pData, uint16_t *pLength);                                      // 0xB29C_cxl
int16_t Service22ReadDataBuriedPointDomain_P2(uint8_t *pData, uint16_t *pLength);                                      // 0xB29D_cxl
int16_t Service22ReadGnssGalaxy(uint8_t *pData, uint16_t *pLength);                                                    // 0xB2C9_cxl
int16_t Service22ReadOtaBookingTime(uint8_t *pData, uint16_t *pLength);                                                // 0xB2CA_cxl
int16_t Service22ReadOtaSwAcqTime(uint8_t *pData, uint16_t *pLength);                                                  // 0xB2CD_cxl
int16_t Service22ReadJtagStatus(uint8_t *pData, uint16_t *pLength);                                                    // 0xB2E5_cxl
int16_t Service22ReadFunctionConfig(uint8_t *pData, uint16_t *pLength);                                                // 0xB2E6_cxl
int16_t Service22ReadEthLineFaultInfo(uint8_t *pData, uint16_t *pLength);                                              // 0xB257_cxl
int16_t Service22ReadOtaDomainName(uint8_t *pData, uint16_t *pLength);                                                 // 104F_cxl B2CB_cxl
int16_t Service22ReadOtaPort(uint8_t *pData, uint16_t *pLength);                                                       // 1050_cxl B2CC_cxl
int16_t Service22ReadPkiDomainName(uint8_t *pData, uint16_t *pLength);                                                 // 1052_cxl
int16_t Service22ReadPkiPort(uint8_t *pData, uint16_t *pLength);                                                       // 1053_cxl
int16_t Service22ReadTransportMode(uint8_t *pData, uint16_t *pLength);                                                 // B2B4_cxl
int16_t Service22ReadManufactoryMode(uint8_t *pData, uint16_t *pLength);                                               // 0110_cxl
int16_t Service22ReadKeySt(uint8_t *pData, uint16_t *pLength);                                                         // 0xB2B5_cxl
int16_t Service22Read4gResetCount(uint8_t *pData, uint16_t *pLength);                                                  // B260_cxl
int16_t Service22ReadDtcSettingControl(uint8_t *pData, uint16_t *pLength);                                             // 0x0120_cxl
int16_t Service22ReadActiveDiagnosticSession(uint8_t *pData, uint16_t *pLength);                                       // 0xF186_cxl
int16_t Service22ReadTspDomain1(uint8_t *pData, uint16_t *pLength);                                                    // 0x031C_cxl
int16_t Service22ReadVehicleMode(uint8_t *pData, uint16_t *pLength);                                                   // 0x5001_cxl
int16_t Service22ReadSpiCommunicationBetweenMCU(uint8_t *pData, uint16_t *pLength);                                    // 0xB26B_cxl
#endif
#if 1 //sq
int16_t Service31StartRoutineCheckProgramPreConditions(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);  // 0xDF01: 检查编程预条件
int16_t Service31StartRoutineCheckProgramIntegrity(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);   // 0xDF02: 编程完整性检查
int16_t Service31StartRoutineEraseMemory(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);            // 0xFF00: 启动擦除内存
int16_t Service31StartRoutineCheckProgrammingDependencies(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);  // 0xFF01: 编程依赖性检查
void Service31StopApplicationSetDefault(void);
void Service31WriteDiagnosticCanReportSetDefault(void);
void EskkeyCheckInit(void);
/*31 01*/
int16_t Service31StartRoutineDdrTest(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
/*31 03*/
int16_t Service31ResultRoutineDdrTest(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
#else
/*31 01*/
int16_t Service31StartRoutineWritePIN(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31StartRoutineWriteSK(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31StartRoutineVerifyPIN(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31StartRoutineVerifySK(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31StartRoutineGetLearn(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31StartRoutineVerifyEolPIN(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31StartRoutineClearPINSK(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31StartRoutinePKI(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);

int16_t Service31StartRoutineDiagDefault(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31StartRoutineDTC(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31StartRoutineJTAG(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31StartRoutineReadEthStatus(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31StartRoutineEthMode(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31StartRoutineEthTestMode(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31StartRoutineCableDiag(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31StartRoutinePkiTest(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31StartRoutineDdrTest(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
/*31 03*/
int16_t Service31RequestRoutineResultsReadPIN(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31RequestRoutineResultsReadSK(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31RequestRoutineResultsVerifyPIN(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31RequestRoutineResultsVerifySK(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31RequestRoutineResultsGetLearn(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31RequestRoutineResultsVerifyEolPIN(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31RequestRoutineResultsClearPINSK(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31RequestRoutineResultsPKI(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31RequestRoutineResultsDiagDefault(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31RequestRoutineResultsJTAG(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31ResultRoutineEthMode(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31ResultRoutineEthTestMode(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31ResultRoutinePkiTest(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
int16_t Service31ResultRoutineDdrTest(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
// service 31 start routinue function
int16_t Service31StopApplication(uint8_t *pDataIn, uint16_t lengthIn, uint8_t *pDataOut, uint16_t *pLengthOut);
void Service31StopApplicationSetDefault(void);
void Service31WriteDiagnosticCanReportSetDefault(void);
void EolConfigureInit(void);
void VinInit(void);
void CheckVinCyleProcess(void);
void EskkeyCheckInit(void);
#endif
#endif //_UDS_DID_FUNCTION_H
