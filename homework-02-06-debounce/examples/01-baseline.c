// Дослід 1: без програмного debounce (ESP-IDF).
// З RC: .pull_up_en = GPIO_PULLUP_DISABLE; без RC (внутрішня підтяжка): GPIO_PULLUP_ENABLE.

#include <esp_attr.h>
#include <freertos/FreeRTOS.h>
#include <stdio.h>
#include <inttypes.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/task.h>

static const char* TAG = "button";

#define BUTTON_PIN 15

static volatile uint32_t count = 0;
static volatile int64_t started = 0;
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

static void IRAM_ATTR onButtonInterrupt(void *arg) {
    (void)arg;
    portENTER_CRITICAL_ISR(&mux);
    started = esp_timer_get_time();
    count++;
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

    uint32_t lastPrintedCount = 0;

    while (1) {
        vTaskDelay(1);

        portENTER_CRITICAL(&mux);
        uint32_t currentCount = count;
        int64_t eventStarted = started;
        portEXIT_CRITICAL(&mux);

        if (currentCount == lastPrintedCount) continue;
        int64_t latency = esp_timer_get_time() - eventStarted;
        lastPrintedCount = currentCount;

        ESP_LOGI(TAG, "Count: %" PRIu32 ", latency: %" PRId64 " us", currentCount, latency);
    }
}