#include <Arduino.h>
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include "FS.h"
#include "LittleFS.h"
#include "resource.h"

#define BUTTON_GPIO D3
#define LED_STATE_GPIO LED_BUILTIN_AUX
#define RELAY_GPIO D5

#define RELAY_STATE_OFF 0
#define RELAY_STATE_ON 1
#define RELAY_STATE_DEFAULT RELAY_STATE_OFF

#define DEVICE_STATE_INIT 0
#define DEVICE_STATE_WIFI_CONNECTING 1
#define DEVICE_STATE_CONFIG 2
#define DEVICE_STATE_RELAY 4

String ssidName;
String ssidPassword;
String relayDisplayName;
int deviceState = DEVICE_STATE_INIT;
int relayState = RELAY_STATE_DEFAULT;

ESP8266WebServer webserver;

IPAddress ipAddress(192, 168, 10, 1);
WiFiEventHandler onSoftAPModeStationConnectedEvent;
WiFiEventHandler onSoftAPModeStationDisconnectedEvent;

void serial_init(void);
void button_init(void);
void relay_init(void);
void led_init(void);
void misc_init(void);

void ICACHE_RAM_ATTR onButtonPressed(void);

void writeRelay(int state);

bool loadWifiConfig(void);
bool saveWifiConfig(String ssid, String pasword, String displayNameA);

void enterWifiRelayMode(void);
void enterWifiConfigMode(void);

void runasStation(void);
void runasToSoftAP(void);

void onPageNotFound(void);
void onConfigHomePage(void);
void onConfigApplyPage(void);
void onRelayHomePage(void);
void onRelayOn(void);
void onRelayOff(void);

String buildHomePageHtml(void);
String buildConfigPageHtml(void);
String buildRedirectHtml(void);

void onSoftAPModeStationConnected(const WiFiEventSoftAPModeStationConnected& event);
void onSoftAPModeStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& event);

void setup() {
  // put your setup code here, to run once:

  /* Init */
  serial_init();
  led_init();
  button_init();
  relay_init();
  misc_init();
  Serial.println("[setup()] Peripherals have been initialized.");

  /* Load WIFI configuration */
  if (loadWifiConfig() == true) {
    /* WIFI already configured */
    Serial.println("[setup()] Load WIFI config successfully, running in WIFI_RELAY mode.");
    // Write relay
    writeRelay(RELAY_STATE_DEFAULT);
    // Run on Relay(Station) mode
    enterWifiRelayMode();
  }
  else {
    /* WIFI does not been configured */
    Serial.println("[setup()] Load WIFI config failed, running in WIFI_CONFIG mode.");
    // Run on Config(SoftAP) mode
    enterWifiConfigMode();
  }
  
  Serial.println("[setup()] Finished setup processes.");
  Serial.println();
}

void loop() {
  // put your main code here, to run repeatedly:
  webserver.handleClient();
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
  if (deviceState != DEVICE_STATE_CONFIG) {
    Serial.println("Button pressed. Clear SSID configuration.");
    if (LittleFS.exists("/wifi.cfg")) {
      LittleFS.remove("/wifi.cfg");
      ESP.restart();
    }
  }
}

void enterWifiRelayMode(void) {
  Serial.println("Entering 'Relay' mode.");

  deviceState = DEVICE_STATE_WIFI_CONNECTING;

  runasStation();

  webserver.begin(80);
  webserver.on("/", onRelayHomePage);
  webserver.on("/relay_on", onRelayOn);
  webserver.on("/relay_off", onRelayOff);
  webserver.onNotFound(onPageNotFound);

  deviceState = DEVICE_STATE_RELAY;
  
  Serial.println("Device runs in 'Relay' mode.");
}

void enterWifiConfigMode(void) {
  Serial.println("Entering 'Config' mode.");

  runasToSoftAP();

  webserver.begin(80);
  webserver.on("/", onConfigHomePage);
  webserver.on("/postconfig", onConfigApplyPage);
  webserver.onNotFound(onPageNotFound);

  deviceState = DEVICE_STATE_CONFIG;
  Serial.println("Device runs in 'Config' mode.");
}

void runasStation(void) {
  Serial.println("WIFI run as 'STA'.");
  // STA mode
  WiFi.mode(WIFI_STA);
  // Connect to WIFI
  WiFi.begin(ssidName, ssidPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  /* Connected */
  Serial.print("WIFI connected. SSID: ");
  Serial.print(WiFi.SSID());
  Serial.print(", IP: ");
  Serial.println(WiFi.localIP());
}

void runasToSoftAP(void) {
  Serial.println("WIFI run as 'SoftAP'.");
  // SoftAP mode
  WiFi.mode(WIFI_AP);
  // Config SoftAP
  WiFi.softAPConfig(ipAddress, ipAddress, IPAddress(255, 255, 255, 0));
  WiFi.softAP("RelayCFG");
  onSoftAPModeStationConnectedEvent = WiFi.onSoftAPModeStationConnected(&onSoftAPModeStationConnected);
  onSoftAPModeStationDisconnectedEvent = WiFi.onSoftAPModeStationDisconnected(&onSoftAPModeStationDisconnected);
  Serial.println("WIFI is already in 'SoftAP' mode.");
}

void writeRelay(int state) {
  relayState = state;
  digitalWrite(RELAY_GPIO, (relayState == RELAY_STATE_OFF) ? LOW : HIGH);
}

bool loadWifiConfig(void) {
  File file = LittleFS.open("/wifi.cfg", "r");
  if (!file) {
    Serial.println("Failed to read WIFI config file.");
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

  Serial.println("Loaded WIFI configuration.");
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

bool saveWifiConfig(String ssid, String pasword, String displayName) {
  File file = LittleFS.open("/wifi.cfg", "w");
  if (!file) {
    Serial.println("Failed to write WIFI config file.");
    return false;
  }
  
  file.println(ssid);
  file.println(pasword);
  file.println(displayName);
  file.flush();
  file.close();
  return true;
}

void onPageNotFound(void) {
  Serial.print("[WebServer] Page not found: ");
  Serial.println(webserver.uri());
}

void onConfigHomePage(void) {
  Serial.println("[Web_CFG] Opening configuration page.");
  webserver.send(200, "text/html", buildConfigPageHtml());
}

void onConfigApplyPage(void) {
  Serial.println("[Web_CFG] New configuration arrived.");
  if (webserver.method() != HTTP_POST) {
    Serial.println("[Web_CFG] Only allow POST.");
    webserver.send(404, "text/plain", "Method not allow");
    return;
  }

  if (!webserver.hasArg("SSID") || !webserver.hasArg("Password") || !webserver.hasArg("RelayDisplayName")) {
    Serial.println("[Web_CFG] Arguments does not valid.");
    webserver.send(404, "text/plain", "Method not allow");
    return;
  }

  ssidName = webserver.arg("SSID");
  ssidPassword = webserver.arg("Password");
  relayDisplayName = webserver.arg("RelayDisplayName");
  Serial.printf("SSID: %s, Password: %s\r\n", ssidName.c_str(), ssidPassword.c_str());
  saveWifiConfig(ssidName, ssidPassword, relayDisplayName);

  webserver.send(200, "text/html", buildRedirectHtml());

  delay(5000);

  ESP.restart();
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

String buildHomePageHtml(void) {
  String str = String(RELAY_PAGE);
  str.replace(RELAY_NAME, relayDisplayName);
  str.replace(RELAY_STATE, (relayState == RELAY_STATE_OFF) ? "ON" : "OFF");
  str.replace(RELAY_HREF, (relayState == RELAY_STATE_OFF) ? "relay_on" : "relay_off");
  return str;
}

String buildConfigPageHtml(void) {
  return CONFIG_PAGE;
}

String buildRedirectHtml(void) {
  return REDIRECT_PAGE;
}

void onSoftAPModeStationConnected(const WiFiEventSoftAPModeStationConnected& event) {
  Serial.printf("Station connected, MAC: %02X-%02X-%02X-%02X-%02X-%02X\r\n", event.mac[0], event.mac[1], event.mac[2], event.mac[3], event.mac[4], event.mac[5]);
}

void onSoftAPModeStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& event) {
  Serial.printf("Station disconnected, MAC: %02X-%02X-%02X-%02X-%02X-%02X\r\n", event.mac[0], event.mac[1], event.mac[2], event.mac[3], event.mac[4], event.mac[5]);
}
