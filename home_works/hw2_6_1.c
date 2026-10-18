
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"

#define BUTTON_PIN GPIO_NUM_18

static const char *TAG = "BUTTON";

static uint32_t pressedCount = 0;
static int64_t interruptTimeUs = 0;

static SemaphoreHandle_t buttonSemaphore;

static portMUX_TYPE timeMux = portMUX_INITIALIZER_UNLOCKED;

static void IRAM_ATTR buttonInterruptHandler(void *arg)
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
        ESP_LOGE(TAG, "Failed to create button semaphore");
        return;
    }

    const gpio_config_t buttonConfig = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };

    ESP_ERROR_CHECK(gpio_config(&buttonConfig));
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_IRAM));

    ESP_ERROR_CHECK(
        gpio_isr_handler_add(
            BUTTON_PIN,
            buttonInterruptHandler,
            NULL
        )
    );

    ESP_LOGI(TAG, "Button interrupt initialized on GPIO %d", (int)BUTTON_PIN);

    while (1) {
        if (xSemaphoreTake(buttonSemaphore, portMAX_DELAY) == pdTRUE)
        {
            int64_t timestamp;

            portENTER_CRITICAL(&timeMux);
            timestamp = interruptTimeUs;
            portEXIT_CRITICAL(&timeMux);

            pressedCount++;

            const int64_t delayUs =
                esp_timer_get_time() - timestamp;

            ESP_LOGI(
                TAG,
                "Button pressed: %lu | Took time: %lld us",
                (unsigned long)pressedCount,
                (long long)delayUs
            );
        }
    }
}
