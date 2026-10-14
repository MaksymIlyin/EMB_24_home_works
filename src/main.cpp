#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 18;
uint8_t pressedCount = 0;


volatile boolean eventOccurred = false;

void quickInterruptHandler() {
    eventOccurred = true;
}
void setup() {
    Serial.begin(115200);
    delay(1000);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), quickInterruptHandler, FALLING);
}

void loop() {
    const uint32_t startMicros = micros();
    if (eventOccurred) {
        pressedCount++;
        const uint32_t delayMicros = micros() - startMicros;
        Serial.print("Button pressed: ");
        Serial.print(pressedCount);
        Serial.print("| Took time: ");
        Serial.print(delayMicros);
        Serial.println(" μs");
        eventOccurred = false;
    }
}
