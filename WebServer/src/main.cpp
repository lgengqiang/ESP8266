#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define SSID_NAME "ESP8266"
#define SSID_PASSWORD "12345678"
#define LED_GPIO LED_BUILTIN_AUX

void onHomePage();
void onLedOn();
void onLedOff();
void onPageNotFound();
String buildHtml();
String buildRedirectHtml();

ESP8266WebServer webServer;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  pinMode(LED_GPIO, OUTPUT);

  /* Connect to WIFI */
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID_NAME, SSID_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  /* WIFI connected */
  Serial.print("WIFI connected : ");
  Serial.print(WiFi.SSID());
  Serial.print(", IP :");
  Serial.println(WiFi.localIP());

  /* ESP8266 Web Server */
  Serial.println("Starting web server.");
  webServer.begin(80);
  webServer.on("/", onHomePage);
  webServer.on("/led_on", onLedOn);
  webServer.on("/led_off", onLedOff);
  webServer.onNotFound(onPageNotFound);
  Serial.println("Web server started.");
}

void loop() {
  // put your main code here, to run repeatedly:
  webServer.handleClient();
}

void onHomePage() {
  webServer.send(200, "text/html", buildHtml());
}

void onLedOn() {
  Serial.println("LED ON");
  digitalWrite(LED_GPIO, LOW);
  webServer.send(200, "text/html", buildRedirectHtml());
}

void onLedOff() {
  Serial.println("LED OFF");
  digitalWrite(LED_GPIO, HIGH);
  webServer.send(200, "text/html", buildRedirectHtml());
}

void onPageNotFound() {
  webServer.send(404, "text/plain", "Page Not Found!");
}

String buildHtml() {
  String str = "<!DOCTYPE HTML>";
  str += "<html>";
  str += "<head><title>ESP8266 Web Server Demo</title></head>";
  str += "<body>";
  if (digitalRead(LED_GPIO) == HIGH) {
    str += "<p><a href=\"/led_on\">Turn ON</a></p>";
  } else {
    str += "<p><a href=\"/led_off\">Turn OFF</a></p>";
  }
  str += "</body>";
  str += "</html>";
  return str;
}

String buildRedirectHtml() {
  String str = "<!DOCTYPE HTML>";
  str += "<html>";
  str += "<meta http-equiv=\"refresh\" content=\"0; url=../\"";
  str += "<head><title></title></head>";
  str += "<body></body>";
  str += "</html>";
  return str;
}