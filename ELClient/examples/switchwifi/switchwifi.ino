#include <Wire.h>

#include <ELClient.h>
#include <ELClientRest.h>
#include <ELClientCmd.h>
#include <ELClientUPnP.h>
#include <ELClientMqtt.h>

#include <TimeLib.h>
#include <Dyndns.h>
#include <UpnpIgd.h>

// This contains the WiFi SSID and passwords
#include "secret.h"

const int buffer_size = 80;
const int loop_delay = 50;

time_t		boot_time = 0, nowts, change_time;
char		buffer[buffer_size];
uint32_t	ip, nm, gw;

int		busy = 1;

ELClient		esp(&Serial, &Serial);
ELClientCmd		cmd(&esp);
ELClientUPnP		upnp(&esp);
ELClientMqtt		mqtt(&esp);

// Forward
void mqData(void *response);
void MqttSetup(), MqttSubscribe(), MqttHostname();
void WifiStatusCallback(void *response);
void NoIP();
void Punch();

// Reboot the Arduino
void (*resetFunc) (void) = 0;	// Declare reset function @ address 0

#define IP(ip, x) ((int)((ip >> (8*x)) & 0xFF))

// Helper function because the ELClientCmd doesn't have a static function for this.
time_t mygettime() {
  return cmd.GetTime();
}

void resetCb(void) {
  Serial.println("EL-Client (re)starting");
}

void setup() {
  // delay(2000);
  Serial.begin(115200);
  Serial.print("Starting wifi ");

  esp.resetCb = resetCb;

  while (! esp.Sync()) {
    Serial.print(".");
  }
  Serial.println(" ok");

  // Put me on the home network, this can save me from reflashing the ESP completely
  cmd.SelectSSID(HOME_SSID, HOME_PASS);

  Serial.print("Get IP address ");
  // Share our IP address
  uint32_t ip, nm, gw;
  cmd.GetWifiInfo(&ip, &nm, &gw);

  // After SelectSSID, wait for valid IP
  while (IP(ip, 0) == 0) {
    Serial.print(".");
    delay(1000);
    cmd.GetWifiInfo(&ip, &nm, &gw);
  }
  Serial.print(" ok -> ");

  sprintf(buffer, "IP %d.%d.%d.%d", IP(ip, 0), IP(ip, 1), IP(ip, 2), IP(ip, 3));
  Serial.println(buffer);
  mqtt.publish("espmega", (char *)buffer);

  Serial.print("MQTT ... ");
  MqttSetup();
  MqttSubscribe();
  // MqttHostname();
  Serial.println("done");

  // Time
  setSyncProvider(mygettime);	// Set the function to get time from the ESP's esp-link
  if (timeStatus() != timeSet)
    Serial.println("RTC failure");
  else {
    Serial.print("RTC ok, ");      
    boot_time = now();
    sprintf(buffer, "%02d:%02d:%02d %02d/%02d/%04d",
      hour(), minute(), second(), day(), month(), year());
    Serial.println(buffer); 
  }

  NoIP();

  // Punch();

  // Get status changes after initial connect
  // esp.wifiCb.attach(WifiStatusCallback);

  // Allow for status change function to work
  busy = 0;
}
 
void Act() {
  change_time = nowts;
  Serial.print("Act() - ");
  sprintf(buffer, "%02d:%02d:%02d", hour(), minute(), second());
  Serial.println(buffer);

  // Select a network
  cmd.SelectSSID(REMOTE_SSID, REMOTE_PASS);
}

void Reset() {
  Serial.print("Reset() - ");
  sprintf(buffer, "%02d:%02d:%02d", hour(), minute(), second());
  Serial.println(buffer);

  // Select a network
  cmd.SelectSSID(HOME_SSID, HOME_PASS);
}

void Report() {
  // Get our IP address
  uint32_t ip, nm, gw;
  cmd.GetWifiInfo(&ip, &nm, &gw);

  sprintf(buffer, ", IP %d.%d.%d.%d, %02d:%02d:%02d",
    IP(ip, 0), IP(ip, 1), IP(ip, 2), IP(ip, 3),
    hour(), minute(), second());
  mqtt.publish("espmega", (char *)buffer);
}

void loop() {
  esp.Process();
  nowts = now();

  if (change_time != 0 && (nowts - change_time) > 110) {
    Report();
  } else if (change_time != 0 && (nowts - change_time) > 120) {
    Reset();
    change_time = 0;
  }
  delay(loop_delay);
}

void MqttSetup() {
  mqtt.setup();
}

void MqttSubscribe() {
  // Subscribe to MQTT messages
  mqtt.subscribe("/wifi/query");
  mqtt.subscribe("/wifi/select");
  mqtt.dataCb.attach(mqData);
}

void MqttHostname() {
  // Query MQTT hostname
  Serial.print("hostname {");
  char *mqtt_clientid = cmd.mqttGetClientId();
  Serial.print(mqtt_clientid);
  Serial.print("} ");
}

// This catches commands via MQTT
void mqData(void *response) {
  ELClientResponse *res = (ELClientResponse *)response;
  String topic = res->popString();
  String data = res->popString();

  Serial.print("MQTT query(");
  Serial.print(topic);
  Serial.print(") data (");
  Serial.print(data);
  Serial.println(")");

  if (strcmp(data.c_str(), "back") == 0) {
    mqtt.publish("espmega", "Changing to home network");
    Reset();
  } else if (strcmp(data.c_str(), "remote") == 0) {
    mqtt.publish("espmega", "Changing to remote AP");
    Act();
  } else if (strcmp(data.c_str(), "reboot") == 0) {
    mqtt.publish("espmega", "Rebooting Arduino ...");
    resetFunc();
  } else if (strcmp(data.c_str(), "ping") == 0) {
    mqtt.publish("espmega", "Hello, cruel world !");
  } else if (strcmp(data.c_str(), "noip") == 0) {
    NoIP();
  } else if (strcmp(data.c_str(), "punch") == 0) {
    mqtt.publish("espmega", "Punching a hole through the firewall");
    Punch();
  } else {
    mqtt.publish("espmega", "Changing to remote AP");
    Act();
  }
}

// This can be used to detect connectivity changes
void WifiStatusCallback(void *response) {
  ELClientResponse *res = (ELClientResponse *)response;

  if (busy == 1)
    return;
  busy = 1;

  NoIP();

  if (res->argc() == 1) {
    uint8_t status;
    res->popArg(&status, 1);

    if (status == STATION_GOT_IP) {
      Serial.println("Network status change\n");
#if 1
      // Share our IP address
      uint32_t ip, nm, gw;
      cmd.GetWifiInfo(&ip, &nm, &gw);

      sprintf(buffer, ", IP %d.%d.%d.%d", IP(ip, 0), IP(ip, 1), IP(ip, 2), IP(ip, 3));
      Serial.println(buffer);
#endif
    }
  }

  busy = 0;
}

void NoIP() {
  Serial.print("Registering with no-ip.com ... ");
  Dyndns *d = new Dyndns();
  d->setHostname(NOIP_HOSTNAME);
  d->setAuth(NOIP_AUTH);	// base64 of user:password
  d->update();
  Serial.println("done");
}

int in_punch = 0;

void Punch() {
  int i;

  if (in_punch)
    return;
  in_punch++;

  cmd.GetWifiInfo(&ip, &nm, &gw);
  sprintf(buffer, "Preparing punching : local %d.%d.%d.%d %u remote %u", 
    IP(ip, 0), IP(ip, 1), IP(ip, 2), IP(ip, 3), 80, 43000);
  mqtt.publish("espmega", buffer); delay(500);
  // UPnP
  Serial.print("UPnP SCAN ");
  upnp.begin();
			mqtt.publish("espmega", "after upnp.begin()"); delay(500);
  uint32_t	igd;
#if 1
  i=0;
  while ((igd = upnp.scan()) == 0 && i++ < 5) {
    delay(1000);
    Serial.print(".");
  }
  Serial.print(" done, router is at ");
#else
  while ((igd = upnp.scan()) == 0) {
    delay(10000);
    Serial.print(".");
  }
  Serial.print(" done, router is at ");
#endif
  sprintf(buffer, "%d.%d.%d.%d, ", IP(igd, 0), IP(igd, 1), IP(igd, 2), IP(igd, 3));
  Serial.print(buffer);
#if 1
  uint32_t	ext = upnp.getExternalAddress();
  while (ext == 0) {
    delay(1000);
    ext = upnp.getExternalAddress();
  }
  sprintf(buffer, "external IP %d.%d.%d.%d", IP(ext, 0), IP(ext, 1), IP(ext, 2), IP(ext, 3));
  Serial.println(buffer);
  mqtt.publish("espmega", buffer); delay(500);

  Serial.print("Signal strength : ");
  int rssi = cmd.GetRSSI(-1);
  Serial.println(rssi);
#endif
  Serial.print("UPnP Add Port mapping ... ");
			mqtt.publish("espmega", "add port mapping ... "); delay(500);
  // upnp.add(0xC0A80196, 0x1234, 0x8765);
  upnp.add(ip, 80, 43000);
  sprintf(buffer, "Punching : local %d.%d.%d.%d %u remote %u", 
    IP(ip, 0), IP(ip, 1), IP(ip, 2), IP(ip, 3), 80, 43000);
  mqtt.publish("espmega", buffer); delay(500);
  Serial.println("done");

  in_punch--;
}
