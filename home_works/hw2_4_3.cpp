#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 18;
constexpr uint32_t DEBOUNCE_MS = 50;

volatile bool eventOccurred = false;

uint32_t pressedCount = 0;
uint32_t pressStartMillis = 0;

bool buttonPressed = false;

void quickInterruptHandler() {
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
    bool hasEvent;

    noInterrupts();
    hasEvent = eventOccurred;
    eventOccurred = false;
    interrupts();

    if (hasEvent && digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
        pressStartMillis = millis();

        while (digitalRead(BUTTON_PIN) == LOW) {
            if (millis() - pressStartMillis >= DEBOUNCE_MS) {
                buttonPressed = true;
                pressedCount++;

                Serial.print("Button pressed: ");
                Serial.print(pressedCount);
                Serial.print(" | Confirmation delay: ");
                Serial.print(millis() - pressStartMillis);
                Serial.println(" ms");

                break;
            }
        }
    }

    if (digitalRead(BUTTON_PIN) == HIGH) {
        buttonPressed = false;
    }
}
