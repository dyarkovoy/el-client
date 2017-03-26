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
 */
#include <Wire.h>

#include <ELClient.h>
#include <ELClientCmd.h>
#include <ELClientSocket.h>
#include <ELClientRest.h>

#include <TimeLib.h>
#include <Dyndns.h>
#include <PcpClient.h>

const int buffer_size = 80;
const int loop_delay = 50;

time_t		boot_time = 0;

char		buffer[buffer_size];
uint32_t	ip, nm, gw;

ELClient	esp(&Serial, &Serial);
ELClientCmd	cmd(&esp);

const char	*YOURHOSTNAME = "your-host.dyndns.org";
const char	*YOURAUTH = "azerty";

#define IP(ip, x) ((int)((ip >> (8*x)) & 0xFF))

// forward
time_t mygettime();

void setup() {
  delay(2000);

  // Serial.begin(9600);
  Serial.begin(115200);
  Serial.println("Yow !");

  // Start WiFi
  Serial.println("Starting wifi");
  while (! esp.Sync()) {
    Serial.println("ELClient sync failed");
  }

  // Get our IP address
  uint32_t ip, nm, gw;
  cmd.GetWifiInfo(&ip, &nm, &gw);

  sprintf(buffer, "IP Address %d.%d.%d.%d", IP(ip, 0), IP(ip, 1), IP(ip, 2), IP(ip, 3));
  Serial.println(buffer);

  Serial.print("Registering with no-ip.com ... ");
  Dyndns *d = new Dyndns();
  d->setHostname(YOURHOSTNAME);
  // If you don't set the address and you're behind a NAT router,
  // then no-ip will pick up the router instead, which is what you want.
  // d->setAddress("192.168.1.150");
  d->setAuth(YOURAUTH);	// base64 of user:password
  d->update();
  Serial.println("done");

  // Punch a hole through the firewall to reach us
  Serial.print("Preparing firewall ... ");
  PcpClient *pcp = new PcpClient();
  sprintf(buffer, "%d.%d.%d.%d", IP(gw, 0), IP(gw, 1), IP(gw, 2), IP(gw, 3));
  pcp->setRouter(buffer);

  // See if our router supports the PCP protocol
  IPAddress ext;
  pcp->queryRouterExternalAddress();
  int i=20;
  while (i-- > 0) {
    ext = pcp->getRouterExternalAddress();
    if (ext != IPAddress((uint32_t)0))
      break;
    delay(10);
    esp.Process();
  }
  if (ext == IPAddress((uint32_t)0)) {
    Serial.println("failed, hanging...");
    while (1) ;
  }

  pcp->addPort(80, 8765, 2 /* TCP */, -1);
  Serial.println("done");
}
 
/*********************************************************************************
 *                                                                               *
 * Loop                                                                          *
 *                                                                               *
 *********************************************************************************/
void loop() {
  esp.Process();

  time_t nowts = now();

  delay(loop_delay);
}

/*
 * Helper function because the ELClientCmd doesn't have a static function for this.
 */
time_t mygettime() {
  return cmd.GetTime();
}
