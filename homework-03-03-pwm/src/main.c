/*
 * Завдання:
 *   1. Потенціометр керує яскравістю світлодіода.
 *   2. Потенціометр керує швидкістю вала двигуна.
 *   3. Двигун та світлодіод не впливають на роботу одне одного.
 */

#include <driver/ledc.h>
#include <esp_adc/adc_oneshot.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define LED_PIN 18
#define MOTOR_PIN 17

#define LEDC_LED_FREQUENCY 1000
#define LEDC_MOTOR_FREQUENCY 10000

#define DUTY_MAX_VALUE 1023
#define ADC_MAX_VALUE 4095

void app_main() {
    ledc_timer_config_t timer_led_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,      // Режим: низька швидкість
        .duty_resolution = LEDC_TIMER_10_BIT,   // Роздільна здатність: 10 біт (0-1023)
        .timer_num = LEDC_TIMER_0,              // Таймер номер: 0
        .freq_hz = LEDC_LED_FREQUENCY,                        // Частота: 1 kHz
        .clk_cfg = LEDC_AUTO_CLK                // Автоматичне налаштування CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_led_config));

    ledc_channel_config_t channel_led_config = {
        .gpio_num = LED_PIN,                         // GPIO піна: 18
        .speed_mode = LEDC_LOW_SPEED_MODE,      // Режим: низька швидкість
        .channel = LEDC_CHANNEL_0,              // Канал номер: 0
        .timer_sel = LEDC_TIMER_0,              // Таймер: 0 (той, що налаштовували вище)
        .duty = 0,                            // Початковий duty: 0 (0% від 1023)
        .hpoint = 0                             // Точка порівняння (зазвичай 0)
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_led_config));

    ledc_timer_config_t timer_motor_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,      // Режим: низька швидкість
        .duty_resolution = LEDC_TIMER_10_BIT,   // Роздільна здатність: 10 біт (0-1023)
        .timer_num = LEDC_TIMER_1,              // Таймер номер: 1
        .freq_hz = LEDC_MOTOR_FREQUENCY,                        // Частота: 10 kHz
        .clk_cfg = LEDC_AUTO_CLK                // Автоматичне налаштування CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_motor_config));

    ledc_channel_config_t channel_motor_config = {
        .gpio_num = MOTOR_PIN,                         // GPIO піна: 17
        .speed_mode = LEDC_LOW_SPEED_MODE,      // Режим: низька швидкість
        .channel = LEDC_CHANNEL_1,              // Канал номер: 1
        .timer_sel = LEDC_TIMER_1,              // Таймер: 1 (той, що налаштовували вище)
        .duty = 0,                            // Початковий duty: 0 (0% від 1023)
        .hpoint = 0                             // Точка порівняння (зазвичай 0)
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_motor_config));

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

    while(1) {
        int potValue;
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL_3, &potValue));
        
        int dutyCycle = potValue * DUTY_MAX_VALUE / ADC_MAX_VALUE;
        
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, dutyCycle);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, dutyCycle);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}