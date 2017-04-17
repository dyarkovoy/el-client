#ifndef _EL_CLIENT_UPNP_H_
#define _EL_CLIENT_UPNP_H_

#include <Arduino.h>
#include "ELClient.h"

class ELClientUPnP {
  public:
    ELClientUPnP(ELClient *e);

    int begin();

    uint32_t scan();
    void add(uint32_t local_ip, uint16_t local_port, uint16_t remote_port);
    void remove(uint16_t remote_port);
    uint32_t getExternalAddress();

  private:
    ELClient *_elc;
};
#endif // _EL_CLIENT_UPNP_H_
