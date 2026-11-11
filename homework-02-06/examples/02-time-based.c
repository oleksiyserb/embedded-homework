// Дослід 2: подія приймається, якщо від попереднього переривання минуло >= 50 мс.
// ISR лише кладе час фронту в чергу; перевірка виконується в app_main.
// Час попереднього переривання оновлюється на КОЖНОМУ перериванні, включно з відкинутими.
// З RC: .pull_up_en = GPIO_PULLUP_DISABLE; без RC: GPIO_PULLUP_ENABLE.

#include <esp_attr.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <inttypes.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <esp_log.h>

#define BUTTON_PIN 15
#define DEBOUNCE_US 50000
#define QUEUE_LEN 32  // ponytail: якщо черга переповниться, зайві фронти губляться; збільшити, якщо raw менший за очікуваний

static QueueHandle_t edgeQueue;

static const char* TAG = "Button";

static void IRAM_ATTR onButtonInterrupt(void *arg) {
    (void)arg;
    int64_t now = esp_timer_get_time();
    BaseType_t woken = pdFALSE;
    xQueueSendFromISR(edgeQueue, &now, &woken);
    if (woken) portYIELD_FROM_ISR();
}

void app_main(void) {
    edgeQueue = xQueueCreate(QUEUE_LEN, sizeof(int64_t));

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

    uint32_t rawCount = 0;
    uint32_t acceptedCount = 0;
    int64_t previousEdge = 0;

    while (1) {
        int64_t edgeAt;
        xQueueReceive(edgeQueue, &edgeAt, portMAX_DELAY);  // спимо, доки ISR не надішле фронт

        rawCount++;
        int64_t elapsed = edgeAt - previousEdge;
        previousEdge = edgeAt;

        if (elapsed < DEBOUNCE_US) continue;

        int64_t latency = esp_timer_get_time() - edgeAt;
        acceptedCount++;

        ESP_LOGI(TAG, "Count: %" PRIu32 " (raw: %" PRIu32 "), latency: %" PRId64 " us",
                 acceptedCount, rawCount, latency);
    }
}
