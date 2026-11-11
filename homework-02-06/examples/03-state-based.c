// Дослід 3: ISR лише ставить прапорець; app_main приймає натискання, якщо пін LOW,
// і блокує повтори до 50 мс стабільного відпускання (HIGH без жодного фронту).
// З RC: .pull_up_en = GPIO_PULLUP_DISABLE; без RC: GPIO_PULLUP_ENABLE.

#include <esp_attr.h>
#include <freertos/FreeRTOS.h>
#include <inttypes.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/task.h>
#include <stdbool.h>

#define BUTTON_PIN 15
#define DEBOUNCE 50000

static volatile int64_t started = 0;
static volatile bool interruptPending = false;
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

static const char* TAG = "Button";

static void IRAM_ATTR onButtonInterrupt(void* arg) {
    (void)arg;
    portENTER_CRITICAL_ISR(&mux);
    if (!interruptPending) {
        started = esp_timer_get_time();
        interruptPending = true;
    }
    portEXIT_CRITICAL_ISR(&mux);
}

void app_main(void) {
    gpio_config_t button_cfg = {
        .pin_bit_mask = 1ULL << BUTTON_PIN,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_NEGEDGE
    };

    ESP_ERROR_CHECK(gpio_config(&button_cfg));

    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(BUTTON_PIN, onButtonInterrupt, NULL));

    uint32_t acceptedCount = 0;
    bool pressAccepted = false;
    bool releaseTiming = false;
    int64_t releaseStarted = 0;

    while (1) {
        vTaskDelay(1);

        portENTER_CRITICAL(&mux);
        bool hasEvent = interruptPending;
        int64_t eventStarted = started;
        interruptPending = false;
        portEXIT_CRITICAL(&mux);

        bool isPressed = gpio_get_level(BUTTON_PIN) == 0;
        const int64_t now = esp_timer_get_time();
        if (pressAccepted) {
            if (isPressed || hasEvent) {
                releaseTiming = false;
            } else if (!releaseTiming) {
                releaseStarted = now;
                releaseTiming = true;
            } else if (now - releaseStarted >= DEBOUNCE) {
                pressAccepted = false;
                releaseTiming = false;
            }
            continue;
        }

        if (hasEvent && isPressed) {
            int64_t latency = now - eventStarted;
            pressAccepted = true;
            releaseTiming = false;
            acceptedCount++;

            ESP_LOGI(TAG, "Count: %" PRIu32 ", latency: %" PRId64 " us", acceptedCount, latency);
        }
    }
}