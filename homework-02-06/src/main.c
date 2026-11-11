// Задача
//
// Реалізувати завдання до уроку 2.4 за допомогою ESP-IDF.
// Якщо було реалізовано на ESP-IDF раніше - зараховується і тут.

#include <freertos/FreeRTOS.h>
#include <inttypes.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/task.h>
#include <stdbool.h>

#define BUTTON_PIN 15
#define DEBOUNCE_US 50000

typedef enum {
    BUTTON_RELEASED,
    BUTTON_CONFIRMING_PRESSED,
    BUTTON_PRESSED,
    BUTTON_CONFIRMING_RELEASED
} button_state_t;

static const char* TAG = "Button";

void app_main(void) {
    gpio_config_t button_cfg = {
        .pin_bit_mask = 1ULL << BUTTON_PIN,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&button_cfg));

    uint32_t acceptedCount = 0;
    button_state_t state = BUTTON_RELEASED;
    int64_t stateStarted = 0;

    while (1) {
        vTaskDelay(1);

        const int64_t now = esp_timer_get_time();

        bool isPressed = gpio_get_level(BUTTON_PIN) == 0;

        switch (state) {
            case BUTTON_RELEASED:
                if (isPressed) {
                    stateStarted = now;
                    state = BUTTON_CONFIRMING_PRESSED;
                }
                break;

            case BUTTON_CONFIRMING_PRESSED:
                if (!isPressed) {
                    state = BUTTON_RELEASED;
                } else if (now - stateStarted >= DEBOUNCE_US) {
                    int64_t latency = now - stateStarted;
                    state = BUTTON_PRESSED;
                    acceptedCount++;

                    ESP_LOGI(TAG, "Count: %" PRIu32 ", latency: %" PRId64 " us", acceptedCount, latency);
                }
                break;

            case BUTTON_PRESSED:
                if (!isPressed) {
                    stateStarted = now;
                    state = BUTTON_CONFIRMING_RELEASED;
                }
                break;

            case BUTTON_CONFIRMING_RELEASED:
                if (isPressed) {
                    state = BUTTON_PRESSED;
                } else if (now - stateStarted >= DEBOUNCE_US) {
                    state = BUTTON_RELEASED;
                }
                break;
        }
    }
}