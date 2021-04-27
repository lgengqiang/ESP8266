#include <Arduino.h>
#include "ESP8266WiFi.h"
#include "ESP8266WiFiAP.h"
#include "ESP8266WebServer.h"
#include "DNSServer.h"

void onStationConnected(const WiFiEventSoftAPModeStationConnected& event) {
  Serial.println("Station connected.");
}
void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& event) {
  Serial.println("Station disconnected.");
}

const uint16_t DNS_PORT = 53;
  
ESP8266WebServer webServer;
IPAddress apIP(192, 168, 10, 1);
WiFiEventHandler onStationConnectedEventHandler;
WiFiEventHandler onStationDisconnectedEventHandler;
DNSServer dnsServer;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN_AUX, OUTPUT);

  Serial.begin(9600);
  Serial.println("ESP8266 SoftAP Demo");

  // WIFI AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("ESP8266");

  // Events
  onStationConnectedEventHandler = WiFi.onSoftAPModeStationConnected(&onStationConnected);
  onStationDisconnectedEventHandler = WiFi.onSoftAPModeStationDisconnected(&onStationDisconnected);

  // Web server
  webServer.begin(80);
  webServer.on("/", []() {
    webServer.send(200, "text/plain", "ESP8266 SoftAP Demo");
  });
  webServer.onNotFound([]() {
    webServer.send(200, "text/plain", "ESP8266 SoftAP Demo");
  });

  // DNS server
  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(DNS_PORT, "esp8266.com", apIP);
}

void loop() {
  // put your main code here, to run repeatedly:
  dnsServer.processNextRequest();
  webServer.handleClient();
}