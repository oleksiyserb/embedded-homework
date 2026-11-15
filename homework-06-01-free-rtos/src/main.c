/*
 * Завдання
 * --------
 * Виконати ДЗ з уроку 2.3, але використовуючи функціонал RTOS,
 * а саме xTaskCreate або xTaskCreatePinnedToCore.
 *
 * Примітка: код з уроку може бути базою для виконання.
 */

#include <stdint.h>
#include <stdbool.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

typedef struct {
    const char* taskName;
    uint8_t pin;
    uint32_t interval;
    bool state;
} Led;

Led leds[3] = {
    {"led1", 15, 200, false},
    {"led2", 16, 500, false},
    {"led3", 17, 1000, false},
};

void blink_task(void* parameter)
{
    Led* led = (Led*)parameter;

    while (1) {
        led->state = !led->state;
        gpio_set_level(led->pin, led->state);
        vTaskDelay(pdMS_TO_TICKS(led->interval));
    }
}

void app_main() {
    gpio_config_t led_cfg = {
        .pin_bit_mask = (1ULL << leds[0].pin) | (1ULL << leds[1].pin) | (1ULL << leds[2].pin),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&led_cfg));

    for (size_t i = 0; i < sizeof(leds) / sizeof(leds[0]); i++) {
        xTaskCreate(blink_task, leds[i].taskName, 2048, &leds[i], 1, NULL);
    }
}