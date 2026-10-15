#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 18;
constexpr uint32_t POLL_INTERVAL = 5;
constexpr uint32_t DEBOUNCE_INTERVAL = 50;

enum class ButtonState {
    RELEASED,
    PRESSED
};

ButtonState buttonState = ButtonState::RELEASED;

uint32_t pressedCount = 0;
uint32_t lastPollMillis = 0;
uint32_t lastStateChangeMillis = 0;
uint32_t pressDetectedMillis = 0;

int lastRawState = HIGH;

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    lastRawState = digitalRead(BUTTON_PIN);
    lastStateChangeMillis = millis();
    lastPollMillis = millis();
}

void loop() {
    if (millis() - lastPollMillis < POLL_INTERVAL) {
        return;
    }
    lastPollMillis = millis();

    const int rawState = digitalRead(BUTTON_PIN);

    if (rawState != lastRawState) {
        lastRawState = rawState;
        lastStateChangeMillis = millis();

        if (rawState == LOW) {
            pressDetectedMillis = millis();
        }
    }

    if (millis() - lastStateChangeMillis < DEBOUNCE_INTERVAL) {
        return;
    }

    if (rawState == LOW && buttonState == ButtonState::RELEASED) {
        buttonState = ButtonState::PRESSED;
        pressedCount++;

        const uint32_t debounceDelay = millis() - pressDetectedMillis;

        Serial.print("Button pressed: ");
        Serial.print(pressedCount);
        Serial.print(" | Debounce delay: ");
        Serial.print(debounceDelay);
        Serial.println(" ms");
    } else if (rawState == HIGH && buttonState == ButtonState::PRESSED)
    {
        buttonState = ButtonState::RELEASED;
        Serial.println("Button released");
    }
}
