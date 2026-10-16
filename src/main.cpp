#include <Arduino.h>

constexpr uint8_t FAN_PIN  = 18;

constexpr ulong PERIOD_SECONDS = 6;
constexpr ulong ON_SECONDS = 2;
constexpr ulong OFF_SECONDS = (PERIOD_SECONDS - ON_SECONDS);

constexpr bool FAN_ON_LEVEL = true;

hw_timer_t* timer = nullptr;

volatile uint32_t elapsedSeconds = 0;
volatile bool fanIsOn = false;


void IRAM_ATTR onTimer()
{
    elapsedSeconds += 1;

    const uint32_t currentDuration = fanIsOn ? ON_SECONDS : OFF_SECONDS;

    if (elapsedSeconds >= currentDuration)
    {
        elapsedSeconds = 0;
        fanIsOn = !fanIsOn;

        const uint32_t level = fanIsOn ? FAN_ON_LEVEL : !FAN_ON_LEVEL;

        digitalWrite(FAN_PIN, level);
    }
}

void setup() {
    pinMode(FAN_PIN, OUTPUT);

    digitalWrite(FAN_PIN, FAN_ON_LEVEL);

    timer = timerBegin(1000000);

    if (timer == nullptr) {
        digitalWrite(FAN_PIN, !FAN_ON_LEVEL);
        return;
    }

    timerAttachInterrupt(timer, &onTimer);

    timerAlarm(timer, 1000000, true, 0);
}

void loop() {
}
