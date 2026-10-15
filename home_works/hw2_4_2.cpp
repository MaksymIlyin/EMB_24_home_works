#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 18;
constexpr uint32_t DEBOUNCE_INTERVAL = 50; // 50 ms

uint32_t pressedCount = 0;

volatile bool eventOccurred = false;
volatile uint32_t interruptMillis = 0;

void quickInterruptHandler() {
    interruptMillis = millis();
    eventOccurred = true;
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(BUTTON_PIN),
        quickInterruptHandler,
        FALLING
    );
}

void loop() {
    if (eventOccurred && millis() - interruptMillis >= DEBOUNCE_INTERVAL) {
        pressedCount++;
        const uint32_t processingDelay = millis() - interruptMillis;
        Serial.print("Button pressed: ");
        Serial.print(pressedCount);
        Serial.print(" | Processing delay: ");
        Serial.print(processingDelay);
        Serial.println(" ms");
        eventOccurred = false;
    }
}

