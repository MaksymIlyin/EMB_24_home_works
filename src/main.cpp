#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 18;

uint32_t pressedCount = 0;

volatile bool eventOccurred = false;
volatile uint32_t interruptMicros = 0;

void quickInterruptHandler() {
    interruptMicros = micros();
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
    bool hasEvent = false;
    uint32_t eventTime = 0;

    noInterrupts();
    if (eventOccurred) {
        eventTime = interruptMicros;
        eventOccurred = false;
        hasEvent = true;
    }
    interrupts();

    if (hasEvent) {
        pressedCount++;

        const uint32_t delayMicros = micros() - eventTime;

        Serial.print("Button pressed: ");
        Serial.print(pressedCount);
        Serial.print(" | Took time: ");
        Serial.print(delayMicros);
        Serial.println(" us");
    }
}
