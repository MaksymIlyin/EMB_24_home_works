#include <Arduino.h>

static constexpr uint8_t LED_PIN1 = 16;
static constexpr uint8_t LED_PIN2 = 17;
static constexpr uint8_t LED_PIN3 = 18;
const uint32_t SPEEDS[3] = { 200, 500, 1000 };

enum class LedState
{
    Off,
    On
};

class Led
{
private:
    uint8_t ledPin;

public:
    Led(uint8_t pLedPin)
    {
        ledPin = pLedPin;
    }
    void init()
    {
        pinMode(ledPin, OUTPUT);
    }

    void set(LedState state)
    {
        digitalWrite(
            ledPin,
            state == LedState::On ? HIGH : LOW
        );
    }
};

Led led1(LED_PIN1);
Led led2(LED_PIN2);
Led led3(LED_PIN3);

void setup()
{
    Serial.begin(115200);
    led1.init();
    led1.set(LedState::Off);
    led2.init();
    led2.set(LedState::Off);
    led3.init();
    led3.set(LedState::Off);
}

void loop()
{
    static LedState state1 = LedState::Off;
    static LedState state2 = LedState::Off;
    static LedState state3 = LedState::Off;
    static uint32_t previousMillis1 = 0;
    static uint32_t previousMillis2 = 0;
    static uint32_t previousMillis3 = 0;

    const uint32_t currentMillis = millis();

    if (currentMillis - previousMillis1 >= SPEEDS[0])
    {
        previousMillis1 = currentMillis;

        state1 = (state1 == LedState::Off)
            ? LedState::On
            : LedState::Off;

        led1.set(state1);
    }
    if (currentMillis - previousMillis2 >= SPEEDS[1])
    {
        previousMillis2 = currentMillis;

        state2 = (state2 == LedState::Off)
            ? LedState::On
            : LedState::Off;

        led2.set(state2);
    }
    if (currentMillis - previousMillis3 >= SPEEDS[2])
    {
        previousMillis3 = currentMillis;

        state3 = (state3 == LedState::Off)
            ? LedState::On
            : LedState::Off;

        led3.set(state3);
    }
}
