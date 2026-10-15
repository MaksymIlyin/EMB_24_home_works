#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 18;
constexpr uint32_t DEBOUNCE = 10; // milli seconds

uint32_t pressedCount = 0;

bool buttonPressed = false;

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
        CHANGE
    );
}

void loop() {
    if (eventOccurred && digitalRead(BUTTON_PIN) == LOW && !buttonPressed)
    {
        while (digitalRead(BUTTON_PIN) == LOW) {
            if (millis() - interruptMillis >= DEBOUNCE) {
                buttonPressed = true;
                pressedCount++;

                Serial.print("Button pressed: ");
                Serial.print(pressedCount);
                Serial.print(" | Confirmation delay: ");
                Serial.print(millis() - interruptMillis);
                Serial.println(" ms");
                break;
            }
        }
    }

    if (digitalRead(BUTTON_PIN) == HIGH) {
        buttonPressed = false;
    }
}
