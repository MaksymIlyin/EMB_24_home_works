#include <array>
#include <cstdint>

#include "esp_err.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace {

constexpr char kTag[] = "SMA";

constexpr adc_channel_t kChannel = ADC_CHANNEL_3; // GPIO4, ADC1
constexpr gpio_num_t kLedPin = GPIO_NUM_18;

constexpr int kPeriodMs = 100;

constexpr std::size_t kWindowSize = 10;

constexpr int kDarkThreshold = 1800;
constexpr int kLightThreshold = 2200;

class SimpleMovingAverage {
public:
    int update(int value)
    {
        if (count_ < kWindowSize) {
            buffer_[index_] = value;
            sum_ += value;
            count_++;
        } else {
            sum_ -= buffer_[index_];
            buffer_[index_] = value;
            sum_ += value;
        }

        index_ = (index_ + 1) % kWindowSize;

        return static_cast<int>(sum_ / count_);
    }

private:
    std::array<int, kWindowSize> buffer_{};

    std::size_t index_ = 0;
    std::size_t count_ = 0;

    int64_t sum_ = 0;
};

} // namespace

extern "C" void app_main()
{
    // ADC initialization
    adc_oneshot_unit_handle_t adc = nullptr;

    adc_oneshot_unit_init_cfg_t unitConfig{};
    unitConfig.unit_id = ADC_UNIT_1;
    unitConfig.ulp_mode = ADC_ULP_MODE_DISABLE;

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(&unitConfig, &adc)
    );

    adc_oneshot_chan_cfg_t channelConfig{};
    channelConfig.atten = ADC_ATTEN_DB_12;
    channelConfig.bitwidth = ADC_BITWIDTH_12;

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc,
            kChannel,
            &channelConfig
        )
    );

    // LED initialization
    const gpio_config_t ledConfig = {
        .pin_bit_mask = (1ULL << kLedPin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&ledConfig));
    ESP_ERROR_CHECK(gpio_set_level(kLedPin, 0));

    // SMA initialization
    SimpleMovingAverage sma;

    bool ledOn = false;

    const TickType_t periodTicks =
        pdMS_TO_TICKS(kPeriodMs);

    configASSERT(periodTicks > 0);

    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        int raw = 0;

        const esp_err_t readError =
            adc_oneshot_read(adc, kChannel, &raw);

        if (readError == ESP_OK) {
            const int filtered = sma.update(raw);

            // Hysteresis
            if (!ledOn && filtered < kDarkThreshold) {
                ledOn = true;
                gpio_set_level(kLedPin, 1);
            } else if (ledOn && filtered > kLightThreshold) {
                ledOn = false;
                gpio_set_level(kLedPin, 0);
            }

            ESP_LOGI(
                kTag,
                "raw=%d | SMA=%d | LED=%s",
                raw,
                filtered,
                ledOn ? "ON" : "OFF"
            );
        } else {
            ESP_LOGW(
                kTag,
                "ADC read failed: %s",
                esp_err_to_name(readError)
            );
        }

        vTaskDelayUntil(&lastWake, periodTicks);
    }
}