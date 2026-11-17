#include <cstdint>

#include "esp_err.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "esp_adc/adc_oneshot.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace {

constexpr char kTag[] = "PWM_CONTROL";

// ADC configuration
constexpr adc_channel_t kChannel = ADC_CHANNEL_3; // GPIO4
constexpr adc_atten_t kAttenuation = ADC_ATTEN_DB_12;
constexpr adc_bitwidth_t kBitwidth = ADC_BITWIDTH_12;

// GPIO configuration
constexpr gpio_num_t kLedPin = GPIO_NUM_18;
constexpr gpio_num_t kMotorPin = GPIO_NUM_17;

// PWM configuration
constexpr ledc_timer_t kPwmTimer = LEDC_TIMER_0;

constexpr ledc_channel_t kLedChannel = LEDC_CHANNEL_0;
constexpr ledc_channel_t kMotorChannel = LEDC_CHANNEL_1;

constexpr ledc_mode_t kPwmMode = LEDC_LOW_SPEED_MODE;

constexpr uint32_t kPwmFrequency = 5000;
constexpr ledc_timer_bit_t kPwmResolution = LEDC_TIMER_12_BIT;

constexpr uint16_t kMaxDuty = 4095;

constexpr TickType_t kPeriodTicks = pdMS_TO_TICKS(100);

adc_oneshot_unit_handle_t adc = nullptr;

void initializeAdc()
{
    adc_oneshot_unit_init_cfg_t unitConfig{};
    unitConfig.unit_id = ADC_UNIT_1;
    unitConfig.ulp_mode = ADC_ULP_MODE_DISABLE;

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(&unitConfig, &adc)
    );

    adc_oneshot_chan_cfg_t channelConfig{};
    channelConfig.atten = kAttenuation;
    channelConfig.bitwidth = kBitwidth;

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc,
            kChannel,
            &channelConfig
        )
    );
}

void initializePwm()
{
    // PWM timer
    ledc_timer_config_t timerConfig{};
    timerConfig.speed_mode = kPwmMode;
    timerConfig.timer_num = kPwmTimer;
    timerConfig.duty_resolution = kPwmResolution;
    timerConfig.freq_hz = kPwmFrequency;
    timerConfig.clk_cfg = LEDC_AUTO_CLK;

    ESP_ERROR_CHECK(
        ledc_timer_config(&timerConfig)
    );

    // LED PWM channel
    ledc_channel_config_t ledConfig{};
    ledConfig.gpio_num = kLedPin;
    ledConfig.speed_mode = kPwmMode;
    ledConfig.channel = kLedChannel;
    ledConfig.timer_sel = kPwmTimer;
    ledConfig.duty = 0;
    ledConfig.hpoint = 0;

    ESP_ERROR_CHECK(
        ledc_channel_config(&ledConfig)
    );

    // Motor PWM channel
    ledc_channel_config_t motorConfig{};
    motorConfig.gpio_num = kMotorPin;
    motorConfig.speed_mode = kPwmMode;
    motorConfig.channel = kMotorChannel;
    motorConfig.timer_sel = kPwmTimer;
    motorConfig.duty = 0;
    motorConfig.hpoint = 0;

    ESP_ERROR_CHECK(
        ledc_channel_config(&motorConfig)
    );
}

void setPwmDuty(
    ledc_channel_t channel,
    uint16_t duty
)
{
    ESP_ERROR_CHECK(
        ledc_set_duty(
            kPwmMode,
            channel,
            duty
        )
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(
            kPwmMode,
            channel
        )
    );
}

} // namespace

extern "C" void app_main()
{
    initializeAdc();
    initializePwm();

    configASSERT(kPeriodTicks > 0);

    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        int raw = 0;

        const esp_err_t readError =
            adc_oneshot_read(
                adc,
                kChannel,
                &raw
            );

        if (readError == ESP_OK) {

            // ADC 0..4095 -> PWM 0..4095
            const uint16_t duty = static_cast<uint16_t>(raw);

            // LED brightness
            setPwmDuty(kLedChannel, duty);

            // Motor speed
            setPwmDuty(kMotorChannel, duty);

        } else {
            ESP_LOGE(
                kTag,
                "ADC read failed: %s",
                esp_err_to_name(readError)
            );
        }

        vTaskDelayUntil(
            &lastWake,
            kPeriodTicks
        );
    }
}
