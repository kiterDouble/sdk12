#ifndef _CERTIFICATE_INTERFACE_H_
#define _CERTIFICATE_INTERFACE_H_

//HSM
// #define PRI_FID_01			0x0A91
// #define PUB_FID_01			0x0A01
// #define PRI_FID_02			0x0A92
// #define PUB_FID_02			0x0A02

//#define PRI_FID_01			(1)
//#define PRI_FID_02			(2)


int CertificateOfVehicleNumberChangeApplication();
int VerifyCertificateChain(const char *device_cert_path, const char *ca_chain_path);
int TeePrivateKeyImport(const char *data,uint32_t pri_len,uint16_t pri_Fid);
int CAPrivateKeyImport();
void SiRunPKIEngineInit();
void SiRunPKIEngineDeInit();
#endif
