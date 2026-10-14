#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 18;
constexpr uint32_t DEBOUNCE = 50; // micro seconds

volatile bool eventOccurred = false;

uint32_t pressedCount = 0;
uint32_t pressStartMicros= 0;

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
        pressStartMicros = micros();

        while (digitalRead(BUTTON_PIN) == LOW) {
            if (micros() - pressStartMicros >= DEBOUNCE) {
                buttonPressed = true;
                pressedCount++;

                Serial.print("Button pressed: ");
                Serial.print(pressedCount);
                Serial.print(" | Confirmation delay: ");
                Serial.print(micros() - pressStartMicros);
                Serial.println(" us");

                break;
            }
        }
    }

    if (digitalRead(BUTTON_PIN) == HIGH) {
        buttonPressed = false;
    }
}
