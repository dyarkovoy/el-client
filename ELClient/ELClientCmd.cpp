/*! \file ELClientCmd.cpp
    \brief Constructor and functions for ELClientCmd
    \author B. Runnels
    \author T. von Eicken
    \date 2016
*/
// Copyright (c) 2016 by B. Runnels and T. von Eicken

#include "ELClientCmd.h"

/*! ELClientCmd(ELClient* elc)
    @brief Constructor for ELClientCmd
*/
ELClientCmd::ELClientCmd(ELClient* elc) :_elc(elc) {}

/*! GetTime()
@brief Get time from ESP
@details Time from the ESP is unformated value of seconds
@warning If the ESP cannot connect to the NTP server or the connection NTP server is not setup, 
	then this time is the number of seconds since the last reboot of the ESP
@return <code>uint32_t</code>
	current time as number of seconds
	- since Thu Jan  1 00:00:58 UTC 1970 if ESP has time from NTP
	- since last reboot of ESP if no NTP time is available
@par Example
@code
	uint32_t t = cmd.GetTime();
	Serial.print("Time: "); Serial.println(t);
@endcode
*/
uint32_t ELClientCmd::GetTime() {
  _elc->Request(CMD_GET_TIME, 0, 0);
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
  return pkt ? pkt->value : 0;
}

/*! GetWifiInfo()
@brief Get IP address info from ESP
@details ip address, network mask, gateway ip
@return Three parameters allow returning the values looked up, specify pointer to <code>uint32_t</code> in them.
@par Example
@code
	uint32_t ip, nm, gw;
	cmd.GetWifiInfo(&ip, &nm, &gw);
@endcode
*/
void ELClientCmd::GetWifiInfo(uint32_t *ptr_ip, uint32_t *ptr_netmask, uint32_t *ptr_gateway) {
  clientCmdCb.attach(this, &ELClientCmd::wifiInfoCmdCallback);
  _elc->Request(CMD_GET_WIFI_INFO, (uint32_t)&clientCmdCb, 0);
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
  if (_elc->_debugEn) {
    _elc->_debug->println("Returning ...");
  }
  if (ptr_ip)
    *ptr_ip = ip;
  if (ptr_netmask)
    *ptr_netmask = netmask;
  if (ptr_gateway)
    *ptr_gateway = gateway;
}

/*! SetWifiInfo()
@brief Set IP address
@details ip address, network mask, gateway ip
@return 
@par Example
@code
	uint32_t ip, nm, gw;
	cmd.SetWifiInfo(ip, nm, gw);
@endcode
*/
void ELClientCmd::SetWifiInfo(uint32_t ptr_ip, uint32_t ptr_netmask, uint32_t ptr_gateway) {
  _elc->Request(CMD_SET_WIFI_INFO, 0, 0);
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
  // return pkt ? pkt->value : 0;
}


/*! wifiInfoCmdCallback()
@brief Helper function to decode the three bits of information from the packet
@details See GetWifiInfo()
@return none
*/
void ELClientCmd::wifiInfoCmdCallback(void *res) {
  ELClientResponse *resp = (ELClientResponse *)res;

  resp->popArg(&ip, sizeof(ip));
  if (_elc->_debugEn) {
    _elc->_debug->print("IP ");
    _elc->_debug->println(ip);
  }

  resp->popArg(&netmask, sizeof(netmask));
  if (_elc->_debugEn) {
    _elc->_debug->print("NM ");
    _elc->_debug->println(netmask);
  }

  resp->popArg(&gateway, sizeof(gateway));
  if (_elc->_debugEn) {
    _elc->_debug->print("GW ");
    _elc->_debug->println(gateway);
  }

  resp->popArg(&mac, sizeof(mac));
}

/*
 * FIXME this depends on having called getWifiInfo
 */
char *ELClientCmd::getMac() {
  return (char *)mac;
}

/*
 * Query the number of Access Points scanned.
 * FIXME this relies on having triggered such a scan
 */
uint32_t ELClientCmd::GetWifiApCount() {
  _elc->Request(CMD_WIFI_GET_APCOUNT, 0, 0);
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
  return pkt ? pkt->value : -1;
}

/*
 * Query the SSID of a network. Range and FIXME as with the ApCount.
 */
char * ELClientCmd::GetWifiApName(int i) {
  uint16_t	ix = i;

  clientCmdCb.attach(this, &ELClientCmd::wifiGetApNameCallback);
  _elc->Request(CMD_WIFI_GET_APNAME, (uint32_t)&clientCmdCb, 1);
  _elc->Request(&ix, 2);
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
  if (_elc->_debugEn) {
    _elc->_debug->println("Returning ...");
  }

  return ssid;
}

void ELClientCmd::wifiGetApNameCallback(void *res) {
  ELClientResponse *resp = (ELClientResponse *)res;

  if (ssid == 0) ssid = (char *)malloc(33);
  resp->popArg(ssid, 33);
  ssid[32] = '\0';
}

/*
 * Query the MQTT clientid
 */
void ELClientCmd:: mqttGetClientIdCallback(void *res) {
  ELClientResponse *resp = (ELClientResponse *)res;

  if (mqtt_clientid == 0) mqtt_clientid = (char *)malloc(33);
  resp->popArg(mqtt_clientid, 32);
  mqtt_clientid[32] = '\0';
}

char *ELClientCmd::mqttGetClientId() {
  mqttCmdCb.attach(this, &ELClientCmd::mqttGetClientIdCallback);
  _elc->Request(CMD_MQTT_GET_CLIENTID, (uint32_t)&mqttCmdCb, 0);
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
  if (_elc->_debugEn) {
    _elc->_debug->println("Returning ...");
  }

  return mqtt_clientid;
}

/*
 * Query RSSI (signal strength)
 */
int ELClientCmd::GetRSSI(int i) {
  char x = i;
  _elc->Request(CMD_WIFI_SIGNAL_STRENGTH, 0, 1);
  _elc->Request(&x, 1);
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
  return pkt ? pkt->value : 0;
}

void ELClientCmd::SelectSSID(char *ssid, char *pass) {
  _elc->Request(CMD_WIFI_SELECT_SSID, 0, 2);
  _elc->Request(ssid, strlen(ssid));
  _elc->Request(pass, strlen(pass));
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
  if (_elc->_debugEn) {
    _elc->_debug->println("Returning ...");
  }
}

void ELClientCmd::SelectSSID(int xssid, char *pass) {
  unsigned char x = xssid;
  _elc->Request(CMD_WIFI_SELECT_SSID, 0, 2);
  _elc->Request(&x, 1);
  _elc->Request(pass, strlen(pass));
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
  if (_elc->_debugEn) {
    _elc->_debug->println("Returning ...");
  }
}

char *ELClientCmd::GetSSID() {
  clientCmdCb.attach(this, &ELClientCmd::wifiGetApNameCallback);
  _elc->Request(CMD_WIFI_GET_SSID, (uint32_t)&clientCmdCb, 0);
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
  if (_elc->_debugEn) {
    _elc->_debug->println("Returning ...");
  }

  return ssid;
}

void ELClientCmd::StartScan() {
  _elc->Request(CMD_WIFI_START_SCAN, 0, 0);
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
  if (_elc->_debugEn) {
    _elc->_debug->println("Returning ...");
  }
}
