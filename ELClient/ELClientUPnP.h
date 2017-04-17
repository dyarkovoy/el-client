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
