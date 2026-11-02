#ifndef _MQTT_INTERFACE_H_
#define _MQTT_INTERFACE_H_





int TboxConnectSend();
int CycleSend();
int UploadSoftwareVersion(int *https_ret);

#endif