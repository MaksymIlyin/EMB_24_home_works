#include <Arduino.h>
#include <cmath>
#include <cstdint>

constexpr uint8_t ADC_PIN = 16;
constexpr uint8_t OUT_PIN = 17;
constexpr uint8_t ADC_BITS = 12;
constexpr uint32_t SERIAL_BAUD = 115200;

constexpr uint8_t BUTTON = 5;
constexpr uint8_t RGB_LED = 48;

constexpr uint32_t SAMPLE_DELAY_MS = 200;
constexpr uint16_t LIGHT_THRESHOLD_RAW = 1800;

constexpr uint16_t ADC_MAX = static_cast<uint16_t>((1u << ADC_BITS) - 1u);
constexpr uint16_t U_REF = 3300;

bool outState = false;

uint8_t state = 0;
constexpr uint8_t STATE_AUTO = 0;
constexpr uint8_t STATE_OFF = 1;
constexpr uint8_t STATE_ON = 2;

void IRAM_ATTR button_isr() {
    state++;
    if (state > STATE_ON)
        state = 0;
}

void setColor(uint8_t r, uint8_t g, uint8_t b)
{
    // neopixelWrite expects GRB
    neopixelWrite(RGB_LED, g, r, b);
}

void setup() {
    Serial.begin(SERIAL_BAUD);
    pinMode(OUT_PIN, OUTPUT);
    pinMode(BUTTON, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON), button_isr, FALLING);
    analogReadResolution(ADC_BITS);
    analogSetAttenuation(ADC_11db);
}

void loop() {
    const uint16_t raw = analogRead(ADC_PIN);
    const bool isBright = raw >= LIGHT_THRESHOLD_RAW;

    outState = isBright ? false: true;
    Serial.print(state);
    Serial.print(" | raw=");
    Serial.print(raw);
    Serial.print(" | light=");
    Serial.println(isBright ? "bright" : "dark");

    if (state == STATE_AUTO)
    {
        setColor(50, 0, 0); // green light
        digitalWrite(OUT_PIN, outState);
    }
    else if (state == STATE_OFF)
    {
        setColor(0, 50, 0); // red light
        digitalWrite(OUT_PIN, false);
    }
    else
    {
        setColor(0, 0, 50); // blue light
        digitalWrite(OUT_PIN, true);
    }

    delay(SAMPLE_DELAY_MS);
}
