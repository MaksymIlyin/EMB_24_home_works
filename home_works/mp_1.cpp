#include <Arduino.h>
#include <cmath>
#include <cstdint>

constexpr uint8_t ADC_PIN = 16;
constexpr uint8_t OUT_PIN = 17;
constexpr uint8_t ADC_BITS = 12;
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t SAMPLE_DELAY_MS = 200;
constexpr uint16_t LIGHT_THRESHOLD_RAW = 1800;

constexpr uint16_t ADC_MAX = static_cast<uint16_t>((1u << ADC_BITS) - 1u);
constexpr uint16_t U_REF = 3300;

bool outState = false;

void setup() {
    Serial.begin(SERIAL_BAUD);
    pinMode(OUT_PIN, OUTPUT);
    analogReadResolution(ADC_BITS);
    analogSetAttenuation(ADC_11db);
}

void loop() {
    const uint16_t raw = analogRead(ADC_PIN);
    const bool isBright = raw >= LIGHT_THRESHOLD_RAW;

    outState = isBright ? false: true;
    digitalWrite(OUT_PIN, outState);
    Serial.print("raw=");
    Serial.print(raw);
    Serial.print(" | light=");
    Serial.println(isBright ? "bright" : "dark");

    delay(SAMPLE_DELAY_MS);
}
