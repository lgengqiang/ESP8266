#include <Arduino.h>
#include "FS.h"
#include "LittleFS.h"

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  Serial.println("File read & write via LittleFS.");

  LittleFS.begin();
  
  File file = LittleFS.open("/test.txt", "w");
  if (!file) {
    Serial.println("Failed to open file.");
  }

  file.println("Hello ESP8266");
  file.print("Also welcome LittleFS");
  file.flush();
  file.close();
  
  Serial.println("File created.");

  /* -------------------- */
  file = LittleFS.open("/test.txt", "r");
  if (!file) {
    Serial.println("Failed to open file.");
  }

  String text = file.readString();

  int index = text.indexOf("\r\n");
  Serial.print(text.substring(0, index));
  Serial.print(text.substring(index, text.length() - 1));

  file.close();
  
  Serial.println("File read.");
  
  /* -------------------- */
}

void loop() {
  // put your main code here, to run repeatedly:
}