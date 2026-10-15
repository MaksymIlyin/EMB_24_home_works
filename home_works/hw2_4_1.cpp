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
    if (eventOccurred)
    {
        pressedCount++;
        const uint32_t delayMicros = micros() - interruptMicros;
        Serial.print("Button pressed: ");
        Serial.print(pressedCount);
        Serial.print(" | Took time: ");
        Serial.print(delayMicros);
        Serial.println(" us");
        eventOccurred = false;
    }
}
