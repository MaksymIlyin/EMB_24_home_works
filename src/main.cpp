#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "esp_err.h"
#include "esp_log.h"

// ==================== Configuration ====================

#define BUZZER_GPIO GPIO_NUM_12

#define PWM_MODE        LEDC_LOW_SPEED_MODE
#define PWM_TIMER       LEDC_TIMER_0
#define PWM_CHANNEL     LEDC_CHANNEL_0
#define PWM_RESOLUTION  LEDC_TIMER_10_BIT

#define PWM_DUTY        512
#define NOTE_DELAY_MS   50
#define MELODY_PAUSE_MS 2000

static const char *TAG = "BUZZER";

// ==================== Musical Notes ====================

#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523

// ==================== Note Structure ====================

typedef struct {
    uint32_t frequency;
    uint32_t duration_ms;
} note_t;

// ==================== Melody ====================

// Twinkle Twinkle Little Star

static const note_t melody[] = {
    {NOTE_C4, 500},
    {NOTE_C4, 500},
    {NOTE_G4, 500},
    {NOTE_G4, 500},
    {NOTE_A4, 500},
    {NOTE_A4, 500},
    {NOTE_G4, 1000},

    {NOTE_F4, 500},
    {NOTE_F4, 500},
    {NOTE_E4, 500},
    {NOTE_E4, 500},
    {NOTE_D4, 500},
    {NOTE_D4, 500},
    {NOTE_C4, 1000},

    {NOTE_G4, 500},
    {NOTE_G4, 500},
    {NOTE_F4, 500},
    {NOTE_F4, 500},
    {NOTE_E4, 500},
    {NOTE_E4, 500},
    {NOTE_D4, 1000},

    {NOTE_G4, 500},
    {NOTE_G4, 500},
    {NOTE_F4, 500},
    {NOTE_F4, 500},
    {NOTE_E4, 500},
    {NOTE_E4, 500},
    {NOTE_D4, 1000},

    {NOTE_C4, 500},
    {NOTE_C4, 500},
    {NOTE_G4, 500},
    {NOTE_G4, 500},
    {NOTE_A4, 500},
    {NOTE_A4, 500},
    {NOTE_G4, 1000},

    {NOTE_F4, 500},
    {NOTE_F4, 500},
    {NOTE_E4, 500},
    {NOTE_E4, 500},
    {NOTE_D4, 500},
    {NOTE_D4, 500},
    {NOTE_C4, 1000}
};

#define MELODY_LENGTH (sizeof(melody) / sizeof(melody[0]))

// ==================== PWM Initialization ====================

static void pwm_init(void)
{
    ledc_timer_config_t timer_config = {};

    timer_config.speed_mode = PWM_MODE;
    timer_config.timer_num = PWM_TIMER;
    timer_config.duty_resolution = PWM_RESOLUTION;
    timer_config.freq_hz = NOTE_C4;
    timer_config.clk_cfg = LEDC_AUTO_CLK;

    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {};

    channel_config.gpio_num = BUZZER_GPIO;
    channel_config.speed_mode = PWM_MODE;
    channel_config.channel = PWM_CHANNEL;
    channel_config.timer_sel = PWM_TIMER;
    channel_config.duty = 0;
    channel_config.hpoint = 0;

    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));

    ESP_LOGI(TAG, "LEDC initialized");
}

// ==================== Stop Sound ====================

static void stop_sound(void)
{
    ESP_ERROR_CHECK(
        ledc_set_duty(PWM_MODE, PWM_CHANNEL, 0)
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(PWM_MODE, PWM_CHANNEL)
    );
}

// ==================== Play Note ====================

static void play_note(uint32_t frequency, uint32_t duration_ms)
{
    if (frequency == 0) {
        stop_sound();
        vTaskDelay(pdMS_TO_TICKS(duration_ms));
        return;
    }

    ESP_ERROR_CHECK(
        ledc_set_freq(PWM_MODE, PWM_TIMER, frequency)
    );

    ESP_ERROR_CHECK(
        ledc_set_duty(PWM_MODE, PWM_CHANNEL, PWM_DUTY)
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(PWM_MODE, PWM_CHANNEL)
    );

    vTaskDelay(pdMS_TO_TICKS(duration_ms));

    stop_sound();

    vTaskDelay(pdMS_TO_TICKS(NOTE_DELAY_MS));
}

// ==================== Play Melody ====================

static void play_melody(const note_t *notes, size_t length)
{
    for (size_t i = 0; i < length; i++) {
        ESP_LOGI(
            TAG,
            "Note: %lu Hz, duration: %lu ms",
            (unsigned long)notes[i].frequency,
            (unsigned long)notes[i].duration_ms
        );

        play_note(
            notes[i].frequency,
            notes[i].duration_ms
        );
    }

    stop_sound();
}

// ==================== Main ====================

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting buzzer synthesizer");

    pwm_init();

    while (1) {
        play_melody(melody, MELODY_LENGTH);

        ESP_LOGI(TAG, "Melody finished");

        vTaskDelay(pdMS_TO_TICKS(MELODY_PAUSE_MS));
    }
}
