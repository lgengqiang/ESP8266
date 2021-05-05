#include <Arduino.h>
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include "FS.h"
#include "LittleFS.h"
#include "resource.h"

#define RELAY_STATE_OFF 0
#define RELAY_STATE_ON 1
#define RELAY_STATE_DEFAULT RELAY_STATE_OFF

#define BUTTON_GPIO D3
#define LED_STATE_GPIO LED_BUILTIN
#define RELAY_GPIO D5

uint32_t perviousMillis = 0;

String ssidName;
String ssidPassword;
String relayDisplayName;
int relayState = RELAY_STATE_DEFAULT;

ESP8266WebServer webserver;

IPAddress ipAddress(192, 168, 10, 1);

WiFiEventHandler onSoftAPModeStationConnectedEvent;
WiFiEventHandler onSoftAPModeStationDisconnectedEvent;
WiFiEventHandler onStationModeConnectedEvent;
WiFiEventHandler onStationModeGotIPEvent;
WiFiEventHandler onStationModeDisconnectedEvent;

void serial_init(void);
void button_init(void);
void relay_init(void);
void led_init(void);
void misc_init(void);

void ICACHE_RAM_ATTR onButtonPressed(void);

void writeRelay(int state);

bool loadWifiConfig(void);
bool saveWifiConfig(String ssid, String pasword, String relayDisplayName);

void onPageNotFound(void);
void onStatusPage(void);
void onConfigHomePage(void);
void onConfigApplyPage(void);
void onRelayHomePage(void);
void onRelayOn(void);
void onRelayOff(void);

String buildStatusPageHtml(void);
String buildConfigPageHtml(void);
String buildHomePageHtml(void);
String buildRedirectHtml(void);

void onSoftAPModeStationConnected(const WiFiEventSoftAPModeStationConnected& event);
void onSoftAPModeStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& event);

void onStationModeConnected(const WiFiEventStationModeConnected& event);
void onStationModeGotIP(const WiFiEventStationModeGotIP& event);
void onStationModeDisconnected(const WiFiEventStationModeDisconnected& event);

void setup() {
  // put your setup code here, to run once:

  /* Peripherals */
  serial_init();
  led_init();
  relay_init();
  misc_init();
  Serial.println("[Setup] Peripherals have been initialized.");

  digitalWrite(LED_STATE_GPIO, LOW);

  /* Load WIFI config */
  if (loadWifiConfig() == true) {
    /* WIFI already configured */
    Serial.println("[Setup] Load configuration successfully.");
  }
  else {
    /* WIFI does not been configured */
    Serial.println("[Setup] Load configuration failed.");
  }

  /* Web Server */
  webserver.begin(80);
  webserver.on("/", onRelayHomePage);
  webserver.on("/relay_on", onRelayOn);
  webserver.on("/relay_off", onRelayOff);
  webserver.on("/config", onConfigHomePage);
  webserver.on("/postconfig", onConfigApplyPage);
  webserver.on("/status", onStatusPage);
  webserver.onNotFound(onPageNotFound);

  /* WIFI */
  WiFi.mode(WIFI_AP_STA);
  //WiFi.hostname(relayDisplayName);
  onSoftAPModeStationConnectedEvent = WiFi.onSoftAPModeStationConnected(&onSoftAPModeStationConnected);
  onSoftAPModeStationDisconnectedEvent = WiFi.onSoftAPModeStationDisconnected(&onSoftAPModeStationDisconnected);
  onStationModeConnectedEvent = WiFi.onStationModeConnected(&onStationModeConnected);
  onStationModeGotIPEvent = WiFi.onStationModeGotIP(&onStationModeGotIP);
  onStationModeDisconnectedEvent = WiFi.onStationModeDisconnected(&onStationModeDisconnected);

  /* WIFI SoftAP */
  WiFi.softAPConfig(ipAddress, ipAddress, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SOFTAP_SSID_NAME);

  /* WIFI Station */
  WiFi.begin(ssidName, ssidPassword);

  /* Finished */
  Serial.println("[Setup] Finished setup.");
  Serial.println();

  perviousMillis = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  webserver.handleClient();

  uint32_t currentMillis = millis();
  if ((currentMillis - perviousMillis) > 500) {
    perviousMillis = currentMillis;
    digitalWrite(LED_STATE_GPIO, (digitalRead(LED_STATE_GPIO) == LOW) ? HIGH : LOW);
  }
}

void serial_init(void) {
  Serial.begin(9600);
}

void button_init(void) {
  pinMode(BUTTON_GPIO, INPUT_PULLUP);
  attachInterrupt(BUTTON_GPIO, onButtonPressed, FALLING);
}

void relay_init(void) {
  pinMode(RELAY_GPIO, OUTPUT);
  digitalWrite(RELAY_GPIO, LOW);
}

void led_init(void) {
  pinMode(LED_STATE_GPIO, OUTPUT);
  digitalWrite(LED_STATE_GPIO, LOW);
}

void misc_init(void) {
  LittleFS.begin();
}

void ICACHE_RAM_ATTR onButtonPressed(void) {
  Serial.println("Button pressed. Reset configuration.");
  if (LittleFS.exists("/wifi.cfg")) {
    LittleFS.remove("/wifi.cfg");
    ESP.restart();
  }
}

void writeRelay(int state) {
  relayState = state;
  digitalWrite(RELAY_GPIO, (relayState == RELAY_STATE_OFF) ? LOW : HIGH);
}

bool loadWifiConfig(void) {
  File file = LittleFS.open("/wifi.cfg", "r");
  if (!file) {
    Serial.println("Failed to read configuration.");
    return false;
  }

  String cfg = file.readString();
  file.close();

  int index = cfg.indexOf("\r\n");
  ssidName = cfg.substring(0, index);
  cfg.remove(0, index + 2);

  index = cfg.indexOf("\r\n");
  ssidPassword = cfg.substring(0, index);
  cfg.remove(0, index + 2);

  relayDisplayName = cfg;

  Serial.println("Loaded configuration.");
  Serial.printf("    SSID: %s\r\n", ssidName.c_str());
  Serial.printf("    Password: %s\r\n", ssidPassword.c_str());
  Serial.printf("    RelayDisplayName: %s\r\n", relayDisplayName.c_str());
  
  if (ssidName.isEmpty() || ssidPassword.isEmpty()) {
    return false;
  }

  if (relayDisplayName.isEmpty()) {
    relayDisplayName = RELAY_DEFAULT_NAME;
  }

  return true;
}

bool saveWifiConfig(String ssid, String pasword, String relayDisplayName) {
  File file = LittleFS.open("/wifi.cfg", "w");
  if (!file) {
    Serial.println("Failed to write configuration.");
    return false;
  }
  
  file.println(ssid);
  file.println(pasword);
  file.println(relayDisplayName);
  file.flush();
  file.close();
  return true;
}

void onPageNotFound(void) {
  Serial.print("[WebServer] Page not found: ");
  Serial.println(webserver.uri());
}

void onStatusPage(void) {
  Serial.println("[WebServer] Opening 'Status' page.");
  webserver.send(200, "text/html", buildStatusPageHtml());
}

void onConfigHomePage(void) {
  Serial.println("[WebServer] Opening configuration page.");
  webserver.send(200, "text/html", buildConfigPageHtml());
}

void onConfigApplyPage(void) {
  Serial.println("[WebServer] New configuration arrived.");
  if (webserver.method() != HTTP_POST) {
    Serial.println("[Web_CFG] Only allow POST.");
    webserver.send(404, "text/plain", "Method not allow");
    return;
  }

  if (!webserver.hasArg("SSID") || !webserver.hasArg("Password") || !webserver.hasArg("RelayDisplayName")) {
    Serial.println("[WebServer] Arguments does not valid.");
    webserver.send(404, "text/plain", "Method not allow");
    return;
  }

  ssidName = webserver.arg("SSID");
  ssidPassword = webserver.arg("Password");
  relayDisplayName = webserver.arg("RelayDisplayName");
  Serial.printf("SSID: %s, Password: %s\r\n", ssidName.c_str(), ssidPassword.c_str());
  saveWifiConfig(ssidName, ssidPassword, relayDisplayName);

  webserver.send(200, "text/html", buildRedirectHtml());

  WiFi.disconnect(false);
  WiFi.begin(ssidName, ssidPassword);
}

void onRelayHomePage(void) {
  Serial.println("Opening relay page.");
  webserver.send(200, "text/html", buildHomePageHtml());
}

void onRelayOn(void) {
  Serial.println("Relay -> ON");
  writeRelay(RELAY_STATE_ON);
  webserver.send(200, "text/html", buildRedirectHtml());
}

void onRelayOff(void) {
  Serial.println("Relay -> OFF");
  writeRelay(RELAY_STATE_OFF);
  webserver.send(200, "text/html", buildRedirectHtml());
}

String buildStatusPageHtml(void) {
  String str = String(STATUS_PAGE);
  str.replace(STATUS_STA_STATUS, (WiFi.status() == WL_CONNECTED) ? STATUS_CONNECTED : STATUS_NOT_CONNECT);
  str.replace(STATUS_SSID_NAME, (WiFi.status() == WL_CONNECTED) ? WiFi.SSID() : NOT_AVAILABLE);
  str.replace(STATUS_STA_IP_ADDRESS, (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : NOT_AVAILABLE);
  str.replace(STATUS_AP_SSID, SOFTAP_SSID_NAME);
  str.replace(STATUS_AP_IP_ADDRESS, WiFi.softAPIP().toString());
  return str;
}

String buildConfigPageHtml(void) {
  return CONFIG_PAGE;
}

String buildHomePageHtml(void) {
  String str = String(RELAY_PAGE);
  str.replace(RELAY_NAME, relayDisplayName);
  str.replace(RELAY_STATE, (relayState == RELAY_STATE_OFF) ? "ON" : "OFF");
  str.replace(RELAY_HREF, (relayState == RELAY_STATE_OFF) ? "relay_on" : "relay_off");
  return str;
}

String buildRedirectHtml(void) {
  return REDIRECT_PAGE;
}

void onSoftAPModeStationConnected(const WiFiEventSoftAPModeStationConnected& event) {
  Serial.printf("[SoftAP] Device connected, MAC: %02X-%02X-%02X-%02X-%02X-%02X\r\n", event.mac[0], event.mac[1], event.mac[2], event.mac[3], event.mac[4], event.mac[5]);
}

void onSoftAPModeStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& event) {
  Serial.printf("[SoftAP] Device disconnected, MAC: %02X-%02X-%02X-%02X-%02X-%02X\r\n", event.mac[0], event.mac[1], event.mac[2], event.mac[3], event.mac[4], event.mac[5]);
}

void onStationModeConnected(const WiFiEventStationModeConnected& event) {
  Serial.printf("WIFI(STA) connected. SSID: %s\r\n", event.ssid.c_str());
}

void onStationModeGotIP(const WiFiEventStationModeGotIP& event) {
  Serial.printf("WIFI(STA) got IP: %s\r\n", event.ip.toString().c_str());
}

void onStationModeDisconnected(const WiFiEventStationModeDisconnected& event) {
  Serial.println("WIFI(STA) disconnected.");
}
