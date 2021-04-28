#include <Arduino.h>

#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"

#include "FS.h"
#include "LittleFS.h"

#include "resource.h"

#define BUTTON_GPIO D3
#define LED_STATE_GPIO LED_BUILTIN_AUX
#define RELAY_A_GPIO D5
#define RELAY_B_GPIO D6

#define RELAY_STATE_OFF 0
#define RELAY_STATE_ON 1
#define RELAY_STATE_DEFAULT RELAY_STATE_OFF

#define DEVICE_STATE_INIT 0
#define DEVICE_STATE_WIFI_CONNECTING 1
#define DEVICE_STATE_CONFIG 2
#define DEVICE_STATE_RELAY 4

String ssidName;
String ssidPassword;
String relayADisplayName;
String relayBDisplayName;
int deviceState = DEVICE_STATE_INIT;
int relayAState = RELAY_STATE_DEFAULT;
int relayBState = RELAY_STATE_DEFAULT;

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

void writeRelayA(int state);
void writeRelayB(int state);

bool loadWifiConfig(void);
bool saveWifiConfig(String ssid, String pasword, String displayNameA, String displayNameB);

void enterWifiRelayMode(void);
void enterWifiConfigMode(void);
// void switchToWifiConfigMode(void);

void runasStation(void);
void runasToSoftAP(void);

void onPageNotFound(void);
void onConfigHomePage(void);
void onConfigApplyPage(void);
void onRelayHomePage(void);
void onRelayAOn(void);
void onRelayAOff(void);
void onRelayBOn(void);
void onRelayBOff(void);

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
    // Write relays
    writeRelayA(RELAY_STATE_DEFAULT);
    writeRelayB(RELAY_STATE_DEFAULT);
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
  pinMode(RELAY_A_GPIO, OUTPUT);
  pinMode(RELAY_B_GPIO, OUTPUT);
  digitalWrite(RELAY_A_GPIO, LOW);
  digitalWrite(RELAY_B_GPIO, LOW);
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
//    Serial.println("Button pressed. Switch to WIFI_CONFIG mode.");
//    switchToWifiConfigMode();

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
  webserver.on("/relay_a_on", onRelayAOn);
  webserver.on("/relay_a_off", onRelayAOff);
  webserver.on("/relay_b_on", onRelayBOn);
  webserver.on("/relay_b_off", onRelayBOff);
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

// void switchToWifiConfigMode(void) {
//   Serial.println("Switching to 'Config' mode.");
//   webserver.stop();
//   enterWifiConfigMode();
// }

void runasStation(void) {
  Serial.println("WIFI run as 'STA'.");
  // if (WiFi.getMode() == WIFI_AP) {
  //   webserver.stop();
  //   WiFi.softAPdisconnect();
  // }
  // STA mode
  WiFi.mode(WIFI_STA);
  // Connect to WIFI
  Serial.printf("SSID: %s, Password: %s\r\n", ssidName.c_str(), ssidPassword.c_str());
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
  // if (WiFi.getMode() == WIFI_STA) {
  //   WiFi.disconnect();
  // }
  // SoftAP mode
  WiFi.mode(WIFI_AP);
  // Config SoftAP
  WiFi.softAPConfig(ipAddress, ipAddress, IPAddress(255, 255, 255, 0));
  WiFi.softAP("RelayX2_CFG");
  onSoftAPModeStationConnectedEvent = WiFi.onSoftAPModeStationConnected(&onSoftAPModeStationConnected);
  onSoftAPModeStationDisconnectedEvent = WiFi.onSoftAPModeStationDisconnected(&onSoftAPModeStationDisconnected);
  Serial.println("WIFI is already in 'SoftAP' mode.");
}

void writeRelayA(int state) {
  relayAState = state;
  digitalWrite(RELAY_A_GPIO, (relayAState == RELAY_STATE_OFF) ? LOW : HIGH);
}

void writeRelayB(int state) {
  relayBState = state;
  digitalWrite(RELAY_B_GPIO, (relayBState == RELAY_STATE_OFF) ? LOW : HIGH);
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
  cfg.remove(0, index + 3);

  index = cfg.indexOf("\r\n");
  ssidPassword = cfg.substring(0, index);
  cfg.remove(0, index + 3);

  index = cfg.indexOf("\r\n");
  relayADisplayName = cfg.substring(0, index);
  cfg.remove(0, index + 3);

  relayBDisplayName = cfg;

  if (ssidName.isEmpty() || ssidPassword.isEmpty()) {
    return false;
  }

  if (relayADisplayName.isEmpty()) {
    relayADisplayName = RELAY_A_DEFAULT_NAME;
  }

  if (relayBDisplayName.isEmpty()) {
    relayBDisplayName = RELAY_B_DEFAULT_NAME;
  }

  return true;
  // int index = cfg.indexOf("\r\n");
  // if (index != -1) {
  //   ssidName = cfg.substring(0, index);
  //   ssidPassword = cfg.substring(index + 2, cfg.length());
  //   return true;
  // } else {
  //   return false;
  // }
}

bool saveWifiConfig(String ssid, String pasword, String displayNameA, String displayNameB) {
  File file = LittleFS.open("/wifi.cfg", "w");
  if (!file) {
    Serial.println("Failed to write WIFI config file.");
    return false;
  }
  
  file.println(ssid);
  file.println(pasword);
  file.println(displayNameA);
  file.print(displayNameB);
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

  if (!webserver.hasArg("SSID") || !webserver.hasArg("Password") || !webserver.hasArg("DisplayNameA") || !webserver.hasArg("DisplayNameB")) {
    Serial.println("[Web_CFG] Arguments does not valid.");
    webserver.send(404, "text/plain", "Method not allow");
    return;
  }

  ssidName = webserver.arg("SSID");
  ssidPassword = webserver.arg("Password");
  relayADisplayName = webserver.arg("DisplayNameA");
  relayBDisplayName = webserver.arg("DisplayNameB");
  Serial.printf("SSID: %s, Password: %s\r\n", ssidName.c_str(), ssidPassword.c_str());
  saveWifiConfig(ssidName, ssidPassword, relayADisplayName, relayBDisplayName);

  webserver.send(200, "text/html", buildRedirectHtml());

  delay(5000);

  ESP.restart();
}

void onRelayHomePage(void) {
  Serial.println("Opening relay page.");
  webserver.send(200, "text/html", buildHomePageHtml());
}

void onRelayAOn(void) {
  Serial.println("Relay A -> ON");
  writeRelayA(RELAY_STATE_ON);
  webserver.send(200, "text/html", buildRedirectHtml());
}

void onRelayAOff(void) {
  Serial.println("Relay A -> OFF");
  writeRelayA(RELAY_STATE_OFF);
  webserver.send(200, "text/html", buildRedirectHtml());
}

void onRelayBOn(void) {
  Serial.println("Relay B -> ON");
  writeRelayB(RELAY_STATE_ON);
  webserver.send(200, "text/html", buildRedirectHtml());
}

void onRelayBOff(void) {
  Serial.println("Relay B -> OFF");
  writeRelayB(RELAY_STATE_OFF);
  webserver.send(200, "text/html", buildRedirectHtml());
}

String buildHomePageHtml(void) {
  String str = String(RELAY_PAGE);
  str.replace(RELAY_A_NAME, "Relay A");
  str.replace(RELAY_A_STATE, (relayAState == RELAY_STATE_OFF) ? "ON" : "OFF");
  str.replace(RELAY_B_NAME, "Relay B");
  str.replace(RELAY_B_STATE, (relayBState == RELAY_STATE_OFF) ? "ON" : "OFF");
  return str;
}

String buildConfigPageHtml(void) {
  return CONFIG_PAGE;
}

String buildRedirectHtml(void) {
  return REDIRECT_PAGE;
}

void onSoftAPModeStationConnected(const WiFiEventSoftAPModeStationConnected& event) {
  Serial.printf("Station connected, MAC: %2X-%2X-%2X-%2X-%2X-%2X\r\n", event.mac[0], event.mac[1], event.mac[2], event.mac[3], event.mac[4], event.mac[5]);
}

void onSoftAPModeStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& event) {
  Serial.printf("Station disconnected, MAC: %2X-%2X-%2X-%2X-%2X-%2X\r\n", event.mac[0], event.mac[1], event.mac[2], event.mac[3], event.mac[4], event.mac[5]);
}
