#include "Arduino.h"

#define LED_PIN 4

// PWM settings
#define PWM_FREQ 200
#define PWM_RESOLUTION 8
#define DELAY 5

const int CH = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 PWM Fade Demo");

  // Attaching PIN to PMV
  ledcSetup(CH, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_PIN, CH);
}

void loop() {
  // --- Fade In ---
  Serial.println("Fading IN...");

  for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle++) {
    ledcWrite(CH, dutyCycle);
    delay(DELAY);
  }

  delay(500);

  // --- Fade Out ---
  Serial.println("Fading OUT...");
  for (int dutyCycle = 255; dutyCycle >= 0; dutyCycle--) {
    ledcWrite(CH, dutyCycle);
    delay(DELAY); // Швидкість зміни
  }

  delay(500);
}
