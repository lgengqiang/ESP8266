#include <Arduino.h>

void ICACHE_RAM_ATTR buttonHandler(void);

void setup() {
  // put your setup code here, to run once:

  // Buildin LED
  pinMode(LED_BUILTIN_AUX, OUTPUT);

  // Serial, baud: 9600
  Serial.begin(9600);
  Serial.println("NodeMCU with 'FLASH' button.");

  // 'FLASH' key with interrupt
  pinMode(D3, INPUT_PULLUP);
  attachInterrupt(D3, buttonHandler, FALLING);
}

void loop() {
  // put your main code here, to run repeatedly:
}

void ICACHE_RAM_ATTR buttonHandler(void) {
  Serial.println("'FLASH' button was been pressed.");
  digitalWrite(LED_BUILTIN_AUX, digitalRead(LED_BUILTIN_AUX) == LOW ? HIGH : LOW);
}