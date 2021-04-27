#include <Arduino.h>
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"

// #define SSID_NAME "LIANG_RT-AC86U"
// #define SSID_PASSWORD "7562614990"
#define SSID_NAME "1701_2.4G"
#define SSID_PASSWORD "12345678"

ESP8266WebServer webServer;

void handleRoot(void);
void handlePostPlainText(void);
void handlePostForm(void);
void handleNotFound(void);
String prepareHomePage(void);
String prepareRedirectPage(void);

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN_AUX, OUTPUT);

  Serial.begin(9600);
  Serial.println("POST demo.");
  
  // Connect to WIFI
  WiFi.begin(SSID_NAME, SSID_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // WIFI connected
  Serial.print("WIFI connected. IP: ");
  Serial.println(WiFi.localIP());

  // Setup web server
  webServer.begin(80);
  webServer.on("/", handleRoot);
  webServer.on("/postplaintext/", handlePostPlainText);
  webServer.on("/postform/", handlePostForm);
  webServer.onNotFound(handleNotFound);

  Serial.println("Web server started.");
}

void loop() {
  // put your main code here, to run repeatedly:
  webServer.handleClient();
}


void handleRoot(void) {
  webServer.send(200, "text/html", prepareHomePage());
  Serial.println("New client.");
}

void handleNotFound(void) {
  Serial.print("Page not found : ");
  Serial.println(webServer.uri());
  webServer.send(200, "text/plain", "Page not found!");
}

void handlePostPlainText(void) {
  if (webServer.method() != HTTP_POST) {
    Serial.println("Method is not allowed.");
  }
  else {
    webServer.send(200, "text/html", prepareRedirectPage());
    Serial.println("HTTP Post(Plain Text) result:");
    for (uint8_t i = 0; i < webServer.args(); i++) {
      Serial.print(webServer.argName(i));
      Serial.print(" : ");
      Serial.println(webServer.arg(i));
    }
  }
}

void handlePostForm(void) {
  if (webServer.method() != HTTP_POST) {
    Serial.println("Method is not allowed.");
  }
  else {
    webServer.send(200, "text/html", prepareRedirectPage());
    Serial.println("HTTP Post(Form) result:");
    for (uint8_t i = 0; i < webServer.args(); i++) {
      Serial.print(webServer.argName(i));
      Serial.print(" : ");
      Serial.println(webServer.arg(i));
    }
  }
}

String prepareHomePage(void) {
  String response = "<!DOCTYPE HTML>";
  response += "<html>";
  response += "<head><title>HTTP POST Request Demo</title></head>";
  response += "<body>";
  response += "<form method =\"post\" enctype=\"text/plain\" action=\"/postplaintext/\">";
  response += "<input type=\"text\" name=\"Name\" value=\"your name\"><br/>";
  response += "<input type=\"text\" name=\"Email\" value=\"your email\"><br/>";
  response += "<input type=\"submit\" value=\"Post Plain Text\">";
  response += "</form>";
  response += "<form method =\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/postform/\">";
  response += "<input type=\"text\" name=\"Name\" value=\"Enter your name\"><br/>";
  response += "<input type=\"text\" name=\"Email\" value=\"Enter your email\"><br/>";
  response += "<input type=\"submit\" value=\"Post Form\">";
  response += "</form>";
  response += "</body>";
  response += "</html>";

  return response;
}

String prepareRedirectPage(void) {
  String response = "<!DOCTYPE html>";
  response += "<html>";
  response += "<meta http-equiv=\"refresh\" content=\"0; url=../\" />";
  response += "<head><title>ESP8266</title></head>";
  response += "<body>";
  response += "<p>Submit successfully!</p>";
  response += "</body>";
  response += "</html>";

  return response;
}