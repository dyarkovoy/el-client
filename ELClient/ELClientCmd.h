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
    char *getMac();

  private:
    ELClient* _elc; /**< ELClient instance */
    FP<void, void*> clientCmdCb; /**< Pointer to external callback function */
    void wifiInfoCmdCallback(void *resp);
    uint32_t ip, netmask, gateway;
    uint8_t mac[6];
};
#endif
