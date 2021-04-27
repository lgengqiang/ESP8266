#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiServer.h>
#include <WiFiClient.h>

#define SSID_NAME "ESP8266"
#define SSID_PASSWORD "12345678"
#define LED_GPIO LED_BUILTIN_AUX

WiFiServer wifiServer(80);

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_GPIO, OUTPUT);
  Serial.begin(9600);

  /* Connecting to WIFI */
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID_NAME, SSID_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  /* WIFI Connected */
  Serial.print("WIFI connected : ");
  Serial.print(WiFi.SSID());
  Serial.print(", IP :");
  Serial.println(WiFi.localIP());

  /* WiFi Server */
  wifiServer.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  WiFiClient client = wifiServer.available();
  if (!client) {
    return;
  }

  String request = client.readString();

  Serial.print("New client : ");
  Serial.println(request);

  client.flush();

  if (request.indexOf("/led_on") != -1)
  {
    Serial.println("LED ON");
    digitalWrite(LED_GPIO, LOW);
  }
  if (request.indexOf("/led_off") != -1)
  {
    Serial.println("LED OFF");
    digitalWrite(LED_GPIO, HIGH);
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head><title>ESP8266 WiFi Server Demo</title></head>");
  client.println("<body>");
  if (digitalRead(LED_GPIO) == HIGH) {
    client.println("<p><a href=\"/led_on\">Turn ON</a></p>");
  } else {
    client.println("<p><a href=\"/led_off\">Turn OFF</a></p>");
  }
  client.println("</body>");
  client.println("</html>");
  client.stop();
}