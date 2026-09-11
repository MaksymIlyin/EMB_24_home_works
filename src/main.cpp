#include <Arduino.h>

#define LED_PIN_R 47
#define LED_PIN_B 48

void setup() {
  pinMode(LED_PIN_R, OUTPUT);
  pinMode(LED_PIN_B, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN_R, 1);
  digitalWrite(LED_PIN_B, 0);
  delay(500);

  digitalWrite(LED_PIN_B, 1);
  digitalWrite(LED_PIN_R, 0);
  delay(500);
}
