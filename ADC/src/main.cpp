#include <Arduino.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("ESP8266 ADC Demo");
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.printf("ADC=%d", analogRead(A0));
  Serial.println();
  delay(1000);
}