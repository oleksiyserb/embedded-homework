/*
 * Завдання
 *
 * 1. Під'єднайте LDR до ESP32 у вигляді подільника напруги та під'єднайте
 *    його вихід до одного з каналів ADC1.
 * 2. Під'єднайте світлодіод до будь-якого доступного GPIO.
 * 3. Реалізуйте зчитування значень з ADC у режимі oneshot.
 * 4. Реалізуйте Simple Moving Average (SMA) для фільтрації значень
 *    освітленості.
 * 5. Використовуючи відфільтровані дані:
 *    - коли стає темно — світлодіод вмикається;
 *    - коли стає світло — світлодіод вимикається.
 * 6. Світлодіод не повинен мерехтіти при незначних коливаннях освітленості.
 *    Реалізуйте два пороги (гістерезис) для запобігання мерехтінню.
 */

#include <stdio.h>
#include <esp_adc/adc_oneshot.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>

#define SMA_COUNT 10

#define UPPER_RANGE 2500
#define LOWER_RANGE 1500

#define LED_PIN 15

static const char* TAG = "ldr";

void app_main(void)
{
    gpio_config_t led_cfg = {
        .pin_bit_mask = 1ULL << LED_PIN,
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&led_cfg));

    // Create ADC1 unit
    adc_oneshot_unit_handle_t adc_handle;

    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_handle));

    // Config Channel
    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_3, &chan_cfg));

    // Pre init
    int samples[SMA_COUNT] = {0};
    int sum = 0;
    int first_raw = 0;
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL_3, &first_raw));

    for (size_t i = 0; i < SMA_COUNT; i++) {
        samples[i] = first_raw;
        sum = sum + samples[i];
    }

    int j = 0;

    // Read in while
    while(1) {
        int raw;
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL_3, &raw));

        // Simple Moving Average (SMA) N = 10
        sum = sum - samples[j];
        samples[j] = raw;
        sum = sum + samples[j];
        if (j < SMA_COUNT - 1) {
            j++;
        } else {
            j = 0;
        }

        const int average = sum / SMA_COUNT;
        ESP_LOGI(TAG, "raw = %d", raw);
        ESP_LOGI(TAG, "SMA = %d", average);

        if (average >= UPPER_RANGE) {
            gpio_set_level(LED_PIN, 0);
        } else if (average <= LOWER_RANGE) {
            gpio_set_level(LED_PIN, 1);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}