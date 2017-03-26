
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
 */
#ifndef _INCLUDE_PCPCLIENT_H_
#define _INCLUDE_PCPCLIENT_H_

#include <IPAddress.h>

class PcpClient {
public:
  PcpClient();
  ~PcpClient();

  void setLocalIP(IPAddress local);
  void setRouter(const char *hostname);
  void queryRouterExternalAddress();
  IPAddress getRouterExternalAddress();
  void addPort(int16_t localport, int16_t remoteport, int8_t protocol, int32_t lifetime);
  void deletePort(int16_t localport, int8_t protocol);

private:
  IPAddress local, external;
  char *router_name;
  ELClientSocket *sock;
  const int	pcp_client_port = 5350,
  		pcp_server_port = 5351;
  static void udpCb(uint8_t, uint8_t, uint16_t, char *);

  void (PcpClient::*catcher)(uint16_t, char *);
  void catchRouterExternalAddress(uint16_t len, char *data);
  void catchAddPort(uint16_t, char *);
  void catchDeletePort(uint16_t, char *);
};

struct PcpPacketExternalAddressRequest {
  uint8_t version;		// 0
  uint8_t opcode;		// 0
};

struct PcpPacketExternalAddressReply {
  uint8_t version;		// 0
  uint8_t opcode;		// Reply should be 128
  uint16_t result;
  uint32_t seconds;
  uint32_t address;
};

struct PcpPacketMappingRequest {
  uint8_t version;		// 0
  uint8_t opcode;		// 1 for UDP, 2 for TCP
  uint16_t reserved;
  int16_t internal_port;
  uint16_t external_port;
  uint32_t lifetime;		// in seconds
};

struct PcpPacketMappingReply {
  uint8_t version;		// 0
  uint8_t opcode;		// 128 + query-opcode
  uint16_t result;
  uint32_t seconds;
  int16_t internal_port;
  uint16_t external_port;
  uint32_t lifetime;		// in seconds
};
#endif
