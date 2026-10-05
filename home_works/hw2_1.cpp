#include <Arduino.h>

enum class LedState
{
    Off,
    On
};

struct Config
{
    static constexpr uint8_t LED_PIN = 17;
    static constexpr uint32_t BLINK_INTERVAL_MS = 500;
};

class Led
{
public:
    void init()
    {
        pinMode(Config::LED_PIN, OUTPUT);
    }

    void set(LedState state)
    {
        digitalWrite(
            Config::LED_PIN,
            state == LedState::On ? HIGH : LOW
        );
    }
};

Led led;

void setup()
{
    led.init();
    led.set(LedState::Off);
}

void loop()
{
    static LedState state = LedState::Off;
    static uint32_t previousMillis = 0;

    const uint32_t currentMillis = millis();

    if (currentMillis - previousMillis >= Config::BLINK_INTERVAL_MS)
    {
        previousMillis = currentMillis;

        state = (state == LedState::Off)
            ? LedState::On
            : LedState::Off;

        led.set(state);
    }
}