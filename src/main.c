#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"

#define BUTTON_PIN GPIO_NUM_18
#define DEBOUNCE_MS 10

static const char *TAG = "BUTTON";

static uint32_t pressedCount = 0;
static bool buttonPressed = false;

static SemaphoreHandle_t buttonSemaphore;
static int64_t interruptTimeUs = 0;
static portMUX_TYPE timeMux = portMUX_INITIALIZER_UNLOCKED;

static void buttonInterruptHandler(void *arg)
{
    const int64_t timestamp = esp_timer_get_time();

    portENTER_CRITICAL_ISR(&timeMux);
    interruptTimeUs = timestamp;
    portEXIT_CRITICAL_ISR(&timeMux);

    BaseType_t higherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(
        buttonSemaphore,
        &higherPriorityTaskWoken
    );

    if (higherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

void app_main(void)
{
    buttonSemaphore = xSemaphoreCreateBinary();

    if (buttonSemaphore == NULL) {
        ESP_LOGE(TAG, "Failed to create semaphore");
        return;
    }

    const gpio_config_t buttonConfig = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };

    ESP_ERROR_CHECK(gpio_config(&buttonConfig));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    ESP_ERROR_CHECK(
        gpio_isr_handler_add(
            BUTTON_PIN,
            buttonInterruptHandler,
            NULL
        )
    );

    ESP_LOGI(TAG, "Button initialized on GPIO %d", BUTTON_PIN);

    while (1) {
        if (xSemaphoreTake(buttonSemaphore, portMAX_DELAY) == pdTRUE) {

            int64_t timestamp;

            portENTER_CRITICAL(&timeMux);
            timestamp = interruptTimeUs;
            portEXIT_CRITICAL(&timeMux);

            // Wait for the signal to stabilize.
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));

            const int level = gpio_get_level(BUTTON_PIN);
            const int64_t nowUs = esp_timer_get_time();

            if (level == 0 && !buttonPressed)
            {
                buttonPressed = true;
                pressedCount++;

                const int64_t confirmationDelayMs = (nowUs - timestamp) / 1000;

                ESP_LOGI(
                    TAG,
                    "Button pressed: %lu | Confirmation delay: %lld ms",
                    (unsigned long)pressedCount,
                    (long long)confirmationDelayMs
                );
            } else if (level == 1) {
                buttonPressed = false;
            }
        }
    }
}