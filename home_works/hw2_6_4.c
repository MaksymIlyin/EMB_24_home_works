#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"

#define BUTTON_PIN GPIO_NUM_18
#define POLL_INTERVAL_MS 10
#define DEBOUNCE_INTERVAL_MS 50

static const char *TAG = "BUTTON";

typedef enum {
    BUTTON_RELEASED,
    BUTTON_PRESSED
} ButtonState;

static ButtonState buttonState = BUTTON_RELEASED;

static uint32_t pressedCount = 0;
static int64_t lastPollUs = 0;
static int64_t lastStateChangeUs = 0;
static int64_t pressDetectedUs = 0;

static int lastRawState = 1;

void app_main(void)
{
    const gpio_config_t buttonConfig = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&buttonConfig));

    lastRawState = gpio_get_level(BUTTON_PIN);

    const int64_t startUs = esp_timer_get_time();
    lastStateChangeUs = startUs;
    lastPollUs = startUs;

    while (1)
    {
        const int64_t nowUs = esp_timer_get_time();
        const int rawState = gpio_get_level(BUTTON_PIN);

        if (rawState != lastRawState)
        {
            lastRawState = rawState;
            lastStateChangeUs = nowUs;

            if (rawState == 0)
            {
                pressDetectedUs = nowUs;
            }
        }

        if (nowUs - lastStateChangeUs >= DEBOUNCE_INTERVAL_MS * 1000) {

            if (rawState == 0 &&
                buttonState == BUTTON_RELEASED) {

                buttonState = BUTTON_PRESSED;
                pressedCount++;

                const uint32_t debounceDelay =
                    (uint32_t)((nowUs - pressDetectedUs) / 1000);

                ESP_LOGI(
                    TAG,
                    "Button pressed: %lu | Debounce delay: %lu ms",
                    (unsigned long)pressedCount,
                    (unsigned long)debounceDelay
                );

            } else if (rawState == 1 &&
                    buttonState == BUTTON_PRESSED) {

                buttonState = BUTTON_RELEASED;
                ESP_LOGI(TAG, "Button released");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
    }
}
