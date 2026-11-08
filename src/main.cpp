
#include <cstdint>
#include <cstdio>

#include "esp_err.h"
#include "esp_log.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace {

constexpr char kTag[] = "ADC_CAL";

constexpr adc_channel_t kChannel = ADC_CHANNEL_3; // GPIO4
constexpr adc_atten_t kAttenuation = ADC_ATTEN_DB_12;
constexpr adc_bitwidth_t kBitwidth = ADC_BITWIDTH_12;

constexpr int kPeriodMs = 100;

constexpr double kManualFullScaleMv = 3100.0;
constexpr double kMaxRaw = 4095.0;

adc_oneshot_unit_handle_t adc = nullptr;
adc_cali_handle_t calibration = nullptr;

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

void initializeCalibration()
{
    adc_cali_curve_fitting_config_t calibrationConfig{};

    calibrationConfig.unit_id = ADC_UNIT_1;
    calibrationConfig.chan = kChannel;
    calibrationConfig.atten = kAttenuation;
    calibrationConfig.bitwidth = kBitwidth;

    ESP_ERROR_CHECK(
        adc_cali_create_scheme_curve_fitting(
            &calibrationConfig,
            &calibration
        )
    );
}

} // namespace

extern "C" void app_main()
{
    initializeAdc();
    initializeCalibration();

    printf("\n");
    printf(" RAW | U_manual(mV) | U_cali(mV) | Error(%%)\n");
    printf("------------------------------------------\n");

    const TickType_t periodTicks =
        pdMS_TO_TICKS(kPeriodMs);

    configASSERT(periodTicks > 0);

    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        int raw = 0;
        int calibratedMv = 0;

        const esp_err_t readError =
            adc_oneshot_read(adc, kChannel, &raw);

        if (readError == ESP_OK) {

            // 1. Manual voltage calculation
            const double manualMv =
                (raw / kMaxRaw) * kManualFullScaleMv;

            // 2. Calibrated voltage
            const esp_err_t calibrationError =
                adc_cali_raw_to_voltage(
                    calibration,
                    raw,
                    &calibratedMv
                );

            if (calibrationError == ESP_OK) {

                // 3. Error calculation
                double errorPercent = 0.0;

                if (calibratedMv != 0) {
                    errorPercent =
                        (manualMv - calibratedMv) /
                        calibratedMv * 100.0;
                }

                printf(
                    "%4d | %12.1f | %10d | %7.2f\n",
                    raw,
                    manualMv,
                    calibratedMv,
                    errorPercent
                );

            } else {
                ESP_LOGE(
                    kTag,
                    "Calibration failed: %s",
                    esp_err_to_name(calibrationError)
                );
            }

        } else {
            ESP_LOGE(
                kTag,
                "ADC read failed: %s",
                esp_err_to_name(readError)
            );
        }

        vTaskDelayUntil(&lastWake, periodTicks);
    }
}
