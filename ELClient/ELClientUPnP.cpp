/*
 * Copyright (c) 2016, 2017 Danny Backx
 *
 * License (MIT license):
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *   THE SOFTWARE.
 *
 * This contains code to "punch a hole" in a NAT firewall, so a device on the inside
 * network can be accessed from the outside by parties that know how to do so.
 *
 * This is basically a poor man's version of some of the UPnP protocols.
 */
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
  _elc->Request(CMD_UPNP_QUERY_EXTERNAL_ADDRESS, 0, 0);
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
  return pkt ? pkt->value : 0;
}
