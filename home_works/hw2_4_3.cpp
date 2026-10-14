#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 18;

uint32_t pressedCount = 0;
uint32_t lastAcceptedMicros = 0;

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
        CHANGE
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

    if (!hasEvent) {
        return;
    }

    if (digitalRead(BUTTON_PIN) != LOW) {
        Serial.println("Ignored: button released");
        return;
    }

    const uint32_t intervalMicros =
        eventTime - lastAcceptedMicros;

    lastAcceptedMicros = eventTime;
    pressedCount++;

    const uint32_t processingDelay = micros() - eventTime;

    Serial.print("Button pressed: ");
    Serial.print(pressedCount);

    Serial.print(" | Interval: ");
    Serial.print(intervalMicros);
    Serial.print(" us");

    Serial.print(" | Processing delay: ");
    Serial.print(processingDelay);
    Serial.println(" us");
}
