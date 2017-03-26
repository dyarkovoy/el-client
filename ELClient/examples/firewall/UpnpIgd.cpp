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
 * Protocol used : UPNP.
 *
 * This implementation is minimalistic : it can just create one packet type.
 */
#include <Arduino.h>

#include <ELClient.h>
#include <ELClientSocket.h>

#include "UpnpIgd.h"

extern ELClient esp;
char *location;

UpnpIgd::UpnpIgd() {
  udpSocket = 0;
}

UpnpIgd::~UpnpIgd() {
}

void UpnpIgd::udpCb(uint8_t resp_type, uint8_t client_num, uint16_t len, char *data) {
  Serial.print("udpCb len ");
  Serial.print(len);
  Serial.print(" resp_type ");
  Serial.println(resp_type);

  if (len > 0 && resp_type == USERCB_RECV) {
    // Filter LOCATION: line
    char *p, *q;
    for (p=data; p<data+len; p++)
      if (*p == 'L' && strncmp(p, "LOCATION:", 9) == 0) {
        for (q=p+8; *q && *q != '\r' && *q != '\n'; q++) ;
	int len = q-p+1;
	location = (char *)malloc(len);
	strncpy(location, p, len-1);
	location[len] = 0;
	Serial.print("GOT ");
	Serial.println(location);
      }
  }
}

void UpnpIgd::discoverGateway() {
  char *msg = "M-SEARCH * HTTP/1.1\r\n"
	"HOST: 239.255.255.250:1900\r\n"
  	"ST: urn:schemas-upnp-org:device:InternetGatewayDevice:1\r\n"
  	"MAN: \"ssdp:discover\"\r\n"
  	"MX: 2\r\n";

  udpSocket = new ELClientSocket(&esp);
  int err = udpSocket->begin("239.255.255.250", 1900, SOCKET_UDP, udpCb);
  udpSocket->send(msg);
  esp.Process();
#if 0
  delay(10);
  udpSocket->send(msg);
  delay(10);
  udpSocket->send(msg);
  delay(10);
  udpSocket->send(msg);
  delay(50);
  udpSocket->send(msg);
#endif
}

/*

34 : UDP multicast to 239.255.255.250:1900

M-SEARCH * HTTP/1.1
HOST: 239.255.255.250:1900
ST: urn:schemas-upnp-org:device:InternetGatewayDevice:1
MAN: "ssdp:discover"
MX: 2

35 : UDP reply

HTTP/1.1 200 OK
CACHE-CONTROL:max-age=1800
EXT:
LOCATION:http://192.168.1.1:8000/o8ee3npj36j/IGD/upnp/IGD.xml
SERVER:MediaAccess TG 789Ovn Xtream 10.A.0.I UPnP/1.0 (9C-97-26-26-44-DE)
ST:urn:schemas-upnp-org:device:InternetGatewayDevice:1
USN:uuid:c5eb6a02-c0b8-5afe-83da-3965c9516822::urn:schemas-upnp-org:device:InternetGatewayDevice:1

114

POST /o8ee3npj36j/IGD/upnp/control/igd/wanpppc_1_1_1 HTTP/1.1
Host: 192.168.1.1:8000
User-Agent: Ubuntu/16.04, UPnP/1.1, MiniUPnPc/2.0
Content-Length: 286
Content-Type: text/xml
SOAPAction: "urn:schemas-upnp-org:service:WANPPPConnection:1#GetExternalIPAddress"
Connection: Close
Cache-Control: no-cache
Pragma: no-cache

<?xml version="1.0"?>
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><s:Body><u:GetExternalIPAddress xmlns:u="urn:schemas-upnp-org:service:WANPPPConnection:1"></u:GetExternalIPAddress></s:Body></s:Envelope>

117

<?xml version="1.0"?>
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
 <s:Body>
<m:GetExternalIPAddressResponse xmlns:m="urn:schemas-upnp-org:service:WANPPPConnection:1"><NewExternalIPAddress>213.49.166.224</NewExternalIPAddress></m:GetExternalIPAddressResponse> </s:Body>
</s:Envelope>

125

POST /o8ee3npj36j/IGD/upnp/control/igd/wanpppc_1_1_1 HTTP/1.1
Host: 192.168.1.1:8000
User-Agent: Ubuntu/16.04, UPnP/1.1, MiniUPnPc/2.0
Content-Length: 594
Content-Type: text/xml
SOAPAction: "urn:schemas-upnp-org:service:WANPPPConnection:1#AddPortMapping"
Connection: Close
Cache-Control: no-cache
Pragma: no-cache

<?xml version="1.0"?>
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><s:Body><u:AddPortMapping xmlns:u="urn:schemas-upnp-org:service:WANPPPConnection:1"><NewRemoteHost></NewRemoteHost><NewExternalPort>9876</NewExternalPort><NewProtocol>TCP</NewProtocol><NewInternalPort>80</NewInternalPort><NewInternalClient>192.168.1.176</NewInternalClient><NewEnabled>1</NewEnabled><NewPortMappingDescription>libminiupnpc</NewPortMappingDescription><NewLeaseDuration>0</NewLeaseDuration></u:AddPortMapping></s:Body></s:Envelope>

127+129

HTTP/1.0 200 OK
Connection: close
Date: Sat, 25 Mar 2017 16:54:41 GMT
Server: MediaAccess TG 789Ovn Xtream 10.A.0.I UPnP/1.0 (9C-97-26-26-44-DE)
Content-Length: 300
Content-Type: text/xml; charset="utf-8"
EXT:

<?xml version="1.0"?>
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
 <s:Body>
<m:AddPortMappingResponse xmlns:m="urn:schemas-upnp-org:service:WANPPPConnection:1"></m:AddPortMappingResponse> </s:Body>
</s:Envelope>


252

POST /o8ee3npj36j/IGD/upnp/control/igd/wanpppc_1_1_1 HTTP/1.1
Host: 192.168.1.1:8000
User-Agent: Ubuntu/16.04, UPnP/1.1, MiniUPnPc/2.0
Content-Length: 380
Content-Type: text/xml
SOAPAction: "urn:schemas-upnp-org:service:WANPPPConnection:1#DeletePortMapping"
Connection: Close
Cache-Control: no-cache
Pragma: no-cache

<?xml version="1.0"?>
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><s:Body><u:DeletePortMapping xmlns:u="urn:schemas-upnp-org:service:WANPPPConnection:1"><NewRemoteHost></NewRemoteHost><NewExternalPort>9876</NewExternalPort><NewProtocol>TCP</NewProtocol></u:DeletePortMapping></s:Body></s:Envelope>


314,316,318
<m:DeletePortMappingResponse xmlns:m="urn:schemas-upnp-org:service:WANPPPConnection:1"></m:DeletePortMappingResponse> </s:Body>
</s:Envelope>


/* */
