#include <Arduino.h>

constexpr uint8_t PIN_OUT = 18;
constexpr uint8_t PIN_IN = 15;

constexpr uint32_t INTERVAL_MS = 2000;
constexpr uint32_t DEBOUNCE_US = 5000;
constexpr uint8_t N = 10;

volatile bool event = false;
volatile uint32_t edgeTime = 0;

uint32_t switchTime = 0;
uint32_t previousMillis = 0;

bool relayOn = false;
bool waiting = false;

uint32_t onSum = 0;
uint32_t offSum = 0;
uint8_t onCount = 0;
uint8_t offCount = 0;

void IRAM_ATTR contactISR() {
    if (!event)
    {
        edgeTime = micros();
        event = true;
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(PIN_OUT, OUTPUT);
    pinMode(PIN_IN, INPUT_PULLUP);
    digitalWrite(PIN_OUT, LOW);

    attachInterrupt(
        digitalPinToInterrupt(PIN_IN),
        contactISR,
        CHANGE
    );
}

void loop() {
    uint32_t now = millis();

    if (!waiting &&
        (onCount < N || offCount < N) &&
        now - previousMillis >= INTERVAL_MS)
    {
        previousMillis = now;
        relayOn = !relayOn;

        noInterrupts();
        event = false;
        interrupts();

        switchTime = micros();
        digitalWrite(PIN_OUT, relayOn ? HIGH : LOW);
        waiting = true;
    }

    if (waiting && event)
    {
        uint32_t edge;

        noInterrupts();
        edge = edgeTime;
        interrupts();

        if (micros() - edge >= DEBOUNCE_US) {
            uint8_t state = digitalRead(PIN_IN);

            if (state == (relayOn ? LOW : HIGH)) {
                uint32_t elapsed = edge - switchTime;

                if (relayOn && onCount < N) {
                    onSum += elapsed;
                    onCount++;

                    Serial.print("ON #");
                    Serial.print(onCount);
                    Serial.print(": ");
                    Serial.print(elapsed);
                    Serial.println(" us");
                } else if (!relayOn && offCount < N) {
                    offSum += elapsed;
                    offCount++;

                    Serial.print("OFF #");
                    Serial.print(offCount);
                    Serial.print(": ");
                    Serial.print(elapsed);
                    Serial.println(" us");
                }

                waiting = false;
            }

            noInterrupts();
            event = false;
            interrupts();
        }
    }

    if (onCount == N && offCount == N) {
        Serial.print("Average ON: ");
        Serial.print(onSum / N);
        Serial.println(" us");

        Serial.print("Average OFF: ");
        Serial.print(offSum / N);
        Serial.println(" us");

        while (true) {}
    }
}
