#include <Arduino.h>
#include <ELClient.h>
#include <ELClientCmd.h>

// Forward declarations
void wifiCb(void *response);

// Initialize a connection to esp-link using the normal hardware serial port both for
// SLIP and for debug messages.
ELClient esp(&Serial, &Serial);
ELClientCmd cmd(&esp);

boolean wifiConnected = false;

void setup() {
  Serial.begin(9600);	// Match the esplink config (Arduino serial <-> ESP)
  delay(3000);
  Serial.println("ELClient test : show IP information");

  esp.wifiCb.attach(wifiCb); // wifi status change callback, optional (delete if not desired)
  bool ok;
  do {
    ok = esp.Sync();      // sync up with esp-link, blocks for up to 2 seconds
    if (!ok) Serial.println("EL-Client sync failed!");
  } while(!ok);
  Serial.println("EL-Client synced!");

  uint32_t ip, nm, gw;
  cmd.GetWifiInfo(&ip, &nm, &gw);

  // FIX ME in reverse
  Serial.print("IP Address ");
  Serial.print((ip & 0x000000FF));
  Serial.print(".");
  Serial.print((ip & 0x0000FF00) >> 8);
  Serial.print(".");
  Serial.print((ip & 0x00FF0000) >> 16);
  Serial.print(".");
  Serial.print((ip & 0xFF000000) >> 24);
  Serial.println("");

  // FIX ME in reverse
  Serial.print("Network mask ");
  Serial.print((nm & 0x000000FF));
  Serial.print(".");
  Serial.print((nm & 0x0000FF00) >> 8);
  Serial.print(".");
  Serial.print((nm & 0x00FF0000) >> 16);
  Serial.print(".");
  Serial.print((nm & 0xFF000000) >> 24);
  Serial.println("");

  // FIX ME in reverse
  Serial.print("IP Gateway ");
  Serial.print((gw & 0x000000FF));
  Serial.print(".");
  Serial.print((gw & 0x0000FF00) >> 8);
  Serial.print(".");
  Serial.print((gw & 0x00FF0000) >> 16);
  Serial.print(".");
  Serial.print((gw & 0xFF000000) >> 24);
  Serial.println("");

  char *mac = cmd.getMac();
  Serial.print("MAC Address : ");
  for (int i=0; i<6; i++) {
    char buf[4];
    if (i < 5)
      sprintf(buf, "%02X:", 0xFF & mac[i]);
    else
      sprintf(buf, "%02X", 0xFF & mac[i]);
    Serial.print(buf);
  }
  Serial.println("");
}

void loop() {
  delay(100);
}

// Callback made from esp-link to notify of wifi status changes
// Here we print something out and set a global flag
void wifiCb(void *response) {
  ELClientResponse *res = (ELClientResponse*)response;
  if (res->argc() == 1) {
    uint8_t status;
    res->popArg(&status, 1);

    if(status == STATION_GOT_IP) {
      Serial.println("WIFI CONNECTED");
      wifiConnected = true;
    } else {
      Serial.print("WIFI NOT READY: ");
      Serial.println(status);
      wifiConnected = false;
    }
  }
}

