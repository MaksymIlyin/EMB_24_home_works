#include <stdint.h>

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

constexpr gpio_num_t RED_PIN    = GPIO_NUM_15;
constexpr gpio_num_t YELLOW_PIN = GPIO_NUM_16;
constexpr gpio_num_t GREEN_PIN  = GPIO_NUM_17;

constexpr uint8_t RED_MASK    = 1U << 0;
constexpr uint8_t YELLOW_MASK = 1U << 1;
constexpr uint8_t GREEN_MASK  = 1U << 2;

struct TrafficState {
    uint8_t pin_mask;
    uint32_t duration_ms;
    bool blink;
};

enum class State: uint8_t {
    GREEN,
    GREEN_BLINK,
    YELLOW,
    RED,
    RED_YELLOW,
    COUNT
};

constexpr TrafficState STATES[] = {
    { GREEN_MASK,                10000, false },
    { GREEN_MASK,                 2000, true  },
    { YELLOW_MASK,                3000, false },
    { RED_MASK,                  10000, false },
    { RED_MASK | YELLOW_MASK,     3000, false }
};

constexpr gpio_num_t PINS[] = {
    RED_PIN,
    YELLOW_PIN,
    GREEN_PIN
};

constexpr uint8_t PIN_MASKS[] = {
    RED_MASK,
    YELLOW_MASK,
    GREEN_MASK
};

constexpr uint32_t BLINK_INTERVAL_MS = 250;
constexpr uint32_t LOOP_INTERVAL_MS = 10;

static State current_state = State::GREEN;

static int64_t state_start_us = 0;
static int64_t last_blink_us = 0;

static bool blink_on = true;

static void set_lights(uint8_t mask)
{
    for (uint8_t i = 0; i < 3; ++i)
    {
        const bool enabled = (mask & PIN_MASKS[i]) != 0;
        ESP_ERROR_CHECK(gpio_set_level(PINS[i], enabled ? 1 : 0));
    }
}

static void init_gpio()
{
    gpio_config_t config = {};

    config.pin_bit_mask =
        (1ULL << RED_PIN) |
        (1ULL << YELLOW_PIN) |
        (1ULL << GREEN_PIN);

    config.mode = GPIO_MODE_OUTPUT;
    config.pull_up_en = GPIO_PULLUP_DISABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_DISABLE;

    ESP_ERROR_CHECK(gpio_config(&config));

    set_lights(0);
}

static void next_state()
{
    uint8_t next = static_cast<uint8_t>(current_state) + 1;

    if (next >= static_cast<uint8_t>(State::COUNT))
    {
        next = 0;
    }

    current_state = static_cast<State>(next);

    const int64_t now = esp_timer_get_time();

    state_start_us = now;
    last_blink_us = now;
    blink_on = true;

    set_lights(STATES[static_cast<uint8_t>(current_state)].pin_mask);
}

static void update_state()
{
    const int64_t now = esp_timer_get_time();

    const TrafficState& state = STATES[static_cast<uint8_t>(current_state)];

    if (now - state_start_us >= static_cast<int64_t>(state.duration_ms) * 1000)
    {
        next_state();
        return;
    }

    if (state.blink && now - last_blink_us >= static_cast<int64_t>(BLINK_INTERVAL_MS) * 1000)
    {
        last_blink_us = now;
        blink_on = !blink_on;

        set_lights(blink_on ? state.pin_mask : 0);
    }
}

extern "C" void app_main(void)
{
    init_gpio();

    state_start_us = esp_timer_get_time();
    last_blink_us = state_start_us;

    set_lights(STATES[static_cast<uint8_t>(current_state)].pin_mask);

    while (true) {
        update_state();
        vTaskDelay(pdMS_TO_TICKS(LOOP_INTERVAL_MS));
    }
}
