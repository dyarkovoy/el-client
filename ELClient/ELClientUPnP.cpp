#include "ELClientUPnP.h"

ELClientUPnP::ELClientUPnP(ELClient *e)
{
  _elc = e;
}

int ELClientUPnP::begin() {
  _elc->Request(CMD_UPNP_BEGIN, 0, 0);
  _elc->Request();
}

/*
 * Query for a NAT router on the network (via UPnP multicast).
 * Return its IP address if found.
 *
 * This assumes we find only one (rather safe for home use). FIXME ?
 * This allows an application to call the scan function in a loop awaiting non-null reply.
 */
uint32_t ELClientUPnP::scan() {
  _elc->Request(CMD_UPNP_SCAN, 0, 0);
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
  return pkt ? pkt->value : 0;
}

/*
 * Add a port mapping to the firewall / home router.
 */
void ELClientUPnP::add(uint32_t local_ip, uint16_t local_port, uint16_t remote_port) {
  _elc->Request(CMD_UPNP_ADD_PORT, 0, 3);
  _elc->Request(&local_ip, 4);
  _elc->Request(&local_port, 2);
  _elc->Request(&remote_port, 2);
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
}

void ELClientUPnP::remove(uint16_t remote_port) {
  _elc->Request(CMD_UPNP_REMOVE_PORT, 0, 1);
  _elc->Request(&remote_port, 2);
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
}

/*
 * Query the external IP address of the router
 */
uint32_t ELClientUPnP::getExternalAddress() {
}
