#include <Arduino.h>

constexpr uint8_t LED_PIN_1 = 15;
constexpr uint8_t LED_PIN_2 = 16;
constexpr uint8_t BUTTON_PIN = 21;
constexpr uint8_t BOOT_PIN = 0;

constexpr unsigned long FAST_INTERVAL = 200;
constexpr unsigned long SLOW_INTERVAL = 1000;

unsigned long previousMillis = 0;

bool led1State = false;
bool led2State = false;
bool slowMode = false;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(LED_PIN_1, OUTPUT);
    pinMode(LED_PIN_2, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BOOT_PIN, INPUT_PULLUP);

    Serial.println("GPIO was initialized");
}

void loop()
{
    const unsigned long currentMillis = millis();

    const bool bootPressed = digitalRead(BOOT_PIN) == LOW;
    const bool buttonPressed = digitalRead(BUTTON_PIN) == LOW;

    if (bootPressed & slowMode == false)
    {
        slowMode = true;
        Serial.println("Slow down");
    }

    if (buttonPressed & slowMode == true)
    {
        slowMode = false;
        Serial.println("Speed up");
    }

    const unsigned long interval =
        slowMode ? SLOW_INTERVAL : FAST_INTERVAL;

    if (currentMillis >= previousMillis + interval)
    {
        previousMillis = currentMillis;

        if (slowMode)
        {
            led1State = !led2State;
        }
        else
        {
            led1State = led2State;
        }

        led1State = !led1State;
        led2State = !led2State;

        digitalWrite(LED_PIN_1, led1State);
        digitalWrite(LED_PIN_2, led2State);
    }
}
