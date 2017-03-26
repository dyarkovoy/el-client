/*
 * Copyright (c) 2017 Danny Backx
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
 * This class allows you to "punch a hole" in the NAT router/firewall typically used.
 *
 * A local port on the esp-link can be remapped to some other port number on the router.
 * Someone accessing that port number can thereby get access to the esp-link even through
 * the NAT router.
 *
 * Protocol used : PCP (Port Control Protocol), RFC 6887, which is a superset of
 * NAT-PMP (NAT Port Mapping Protocol), RFC 6886.
 *
 * Obviously (grin) this is a minimal implementation.
 */
#include <Arduino.h>

#include <ELClient.h>
#include <ELClientSocket.h>

#include "PcpClient.h"
#include "IPAddress.h"

extern ELClient esp;
static PcpClient *self;

PcpClient::PcpClient() {
  sock = new ELClientSocket(&esp);
  self = this;
  catcher = 0;
}

PcpClient::~PcpClient() {
  if (sock) {
    delete sock;
    sock = 0;
  }
}

void PcpClient::setLocalIP(IPAddress local) {
  this->local = local;
}

void PcpClient::setRouter(const char *hostname) {
  router_name = (char *)hostname;
}

void PcpClient::queryRouterExternalAddress() {
  struct PcpPacketExternalAddressRequest p;
  p.version = p.opcode = 0;
  external = IPAddress((uint32_t)0);

  catcher = &PcpClient::catchRouterExternalAddress;

  int err = sock->begin(router_name, pcp_server_port, SOCKET_UDP, udpCb);
  sock->send((char *)&p);
}

IPAddress PcpClient::getRouterExternalAddress() {
  return external;
}

void PcpClient::catchRouterExternalAddress(uint16_t len, char *data) {
  if (len == sizeof(PcpPacketExternalAddressReply)) {
    PcpPacketExternalAddressReply *p = (PcpPacketExternalAddressReply *)data;
    if (p->opcode != 0x80)
      return;
    if (p->version != 0 && p->version != 2)	// NAT-PMP & PCP
      return;
    // Looks right now
    external = p->address;
  } else
    return; // silently

  // Success, so switch ourselves off
  catcher = 0;
}

void PcpClient::addPort(int16_t localport, int16_t remoteport, int8_t protocol, int32_t lifetime) {
  struct PcpPacketMappingRequest p;
  p.version = 0;
  p.opcode = protocol;
  p.reserved = 0;
  p.internal_port = localport;
  p.external_port = remoteport;
  p.lifetime = lifetime;

  catcher = &PcpClient::catchAddPort;

  int err = sock->begin(router_name, pcp_server_port, SOCKET_UDP, udpCb);
  sock->send((char *)&p);
}

void PcpClient::catchAddPort(uint16_t len, char *data) {
#if 0
  if (len == sizeof(PcpPacketMappingReply)) {
    PcpPacketMappingReply *p = (PcpPacketMappingReply *)data;
    if (p->opcode != 0x81 && p->opcode != 0x82)
      return;
    if (p->version != 0 && p->version != 2)	// NAT-PMP & PCP
      return;
    // Looks right now
    // Nothing to do really
  } else
    return; // silently
#endif
  // Success, so switch ourselves off
  catcher = 0;
}

void PcpClient::deletePort(int16_t localport, int8_t protocol) {
  struct PcpPacketMappingRequest p;
  p.version = 0;
  p.opcode = protocol;
  p.reserved = 0;
  p.internal_port = localport;
  p.external_port = 0;
  p.lifetime = 0;

  catcher = &PcpClient::catchDeletePort;

  int err = sock->begin(router_name, pcp_server_port, SOCKET_UDP, udpCb);
  sock->send((char *)&p);
}

void PcpClient::catchDeletePort(uint16_t len, char *data) {
  // Nothing to do ?

  // Success, so switch ourselves off
  catcher = 0;
}

void PcpClient::udpCb(uint8_t resp_type, uint8_t client_num, uint16_t len, char *data) {
  if (len > 0 && resp_type == USERCB_RECV) {
    if (self && self->catcher)
      // (self .* catcher)(len, data);
      (self ->* ((PcpClient*)self)->PcpClient::catcher)(len, data);
  }
}

/* Note for later :
 * Syntax in a static member function :
 *  (self ->* ((PcpClient*)self)->PcpClient::catcher)(1, "xx");
 * Syntax in a member function :
 *  (this ->* ((PcpClient*)this)->PcpClient::catcher)(1, "xx");
 *
 * See also http://stackoverflow.com/questions/990625/c-function-pointer-class-member-to-non-static-member-function
 */
