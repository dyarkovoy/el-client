/*! \file ELClientCmd.h
    \brief Definitions for ELClientCmd
	\note Miscellaneous commands
*/
// Miscellaneous commands

#ifndef _EL_CLIENT_CMD_H_
#define _EL_CLIENT_CMD_H_

#include <Arduino.h>
#include "ELClient.h"
#include "FP.h"

class ELClientCmd {
  public:
    // Constructor
    ELClientCmd(ELClient* elc);
    // Get the current time in seconds since the epoch, 0 if the time is unknown
    uint32_t GetTime();
    void GetWifiInfo(uint32_t *, uint32_t *, uint32_t *);
    void SetWifiInfo(uint32_t, uint32_t, uint32_t);
    uint32_t GetWifiApCount();
    char * GetWifiApName(int);
    char *getMac();
    char *mqttGetClientId();
    int GetRSSI(int);		// Current signal strength if <0, or selected network's rssi
    void SelectSSID(char *, char *);
    void SelectSSID(int, char *);
    char *GetSSID();
    void StartScan();

  private:
    ELClient* _elc; /**< ELClient instance */
    FP<void, void*> clientCmdCb; /**< Pointer to external callback function */
    void wifiInfoCmdCallback(void *resp);
    uint32_t ip, netmask, gateway;
    uint8_t mac[6];

    char *ssid;
    void wifiGetApNameCallback(void *);

    FP<void, void*> mqttCmdCb; /**< Pointer to external callback function */
    void mqttGetClientIdCallback(void *);
    char *mqtt_clientid;
};
#endif
