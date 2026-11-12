/*
 * Задача:
 *   Використовуючи код із заняття, додати стани відповідно до ПДР України.
 *
 * Станом на 12 січня 2026 року в Україні правила дорожнього руху (ПДР)
 * чітко регламентують послідовність сигналів світлофора для забезпечення
 * безпеки.
 *
 * Стандартна послідовність перемикання:
 *   1. Зелений               - дозволяє рух.
 *   2. Зелений миготливий    - попереджає, що час дії дозволеного сигналу
 *                              закінчується і скоро буде ввімкнено
 *                              заборонний сигнал.
 *   3. Жовтий                - забороняє рух і попереджає про майбутню
 *                              зміну сигналів.
 *   4. Червоний              - забороняє рух.
 *   5. Червоний + жовтий     - попереджають про наступне ввімкнення
 *                              зеленого сигналу (рух все ще заборонено).
 *   6. Зелений               - цикл повторюється.
 *
 * Особливі випадки:
 *   - Жовтий миготливий      - дозволяє рух і інформує про наявність
 *                              нерегульованого перехрестя або
 *                              пішохідного переходу.
 */
#include <freertos/FreeRTOS.h>
#include <inttypes.h>
#include <driver/gpio.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <stdbool.h>

#define RED_LED 17
#define YELLOW_LED 16
#define GREEN_LED 15

#define GREEN_TIME_MS 10000
#define GREEN_BLINK_TIME_MS 3000
#define YELLOW_TIME_MS 3000
#define RED_TIME_MS 10000
#define RED_YELLOW_TIME_MS 2000
#define BLINK_PERIOD_MS 500

typedef enum {
    GREEN_LIGHT,
    GREEN_BLINK,
    YELLOW_LIGHT,
    RED_LIGHT,
    RED_YELLOW_LIGHT,
    // YELLOW_BLINK
} traffic_light_t;

static traffic_light_t currentState = GREEN_LIGHT;
static int64_t lastToggledTimeMs = 0;
static bool isStateEntered = false;

static void set_lights(uint32_t red, uint32_t yellow, uint32_t green)
{
    gpio_set_level(RED_LED, red);
    gpio_set_level(YELLOW_LED, yellow);
    gpio_set_level(GREEN_LED, green);
}

static void change_state(traffic_light_t state, int64_t timeMs)
{
    lastToggledTimeMs = timeMs;
    currentState = state;
    isStateEntered = false;
}

void app_main(void)
{
    gpio_config_t led_cfg = {
        .pin_bit_mask = (1ULL << RED_LED) | (1ULL << YELLOW_LED) | (1ULL << GREEN_LED),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&led_cfg));

    gpio_set_level(RED_LED, 0);
    gpio_set_level(YELLOW_LED, 0);
    gpio_set_level(GREEN_LED, 0);

    lastToggledTimeMs = esp_timer_get_time() / 1000;

    while(1)
    {
        vTaskDelay(1);

        const int64_t currentTimeMs = esp_timer_get_time() / 1000;

        switch (currentState) {
            case GREEN_LIGHT: {
                if (!isStateEntered) {
                    isStateEntered = true;
                    set_lights(0, 0, 1);
                }

                if (currentTimeMs - lastToggledTimeMs > GREEN_TIME_MS) {
                    change_state(GREEN_BLINK, currentTimeMs);
                }
                break;
            }
            case GREEN_BLINK: {
                if (!isStateEntered) {
                    isStateEntered = true;

                    set_lights(0, 0, 0);
                }

                const uint32_t greenLevel = ((currentTimeMs - lastToggledTimeMs) / BLINK_PERIOD_MS) % 2;
                gpio_set_level(GREEN_LED, greenLevel);

                if (currentTimeMs - lastToggledTimeMs > GREEN_BLINK_TIME_MS) {
                    change_state(YELLOW_LIGHT, currentTimeMs);
                }
                break;
            }
            case YELLOW_LIGHT: {
                if (!isStateEntered) {
                    isStateEntered = true;

                    set_lights(0, 1, 0);
                }

                if (currentTimeMs - lastToggledTimeMs > YELLOW_TIME_MS) {
                    change_state(RED_LIGHT, currentTimeMs);
                }
                break;
            }
            case RED_LIGHT: {
                if (!isStateEntered) {
                    isStateEntered = true;

                    set_lights(1, 0, 0);
                }

                if (currentTimeMs - lastToggledTimeMs > RED_TIME_MS) {
                    change_state(RED_YELLOW_LIGHT, currentTimeMs);
                }
                break;
            }
            case RED_YELLOW_LIGHT: {
                if (!isStateEntered) {
                    isStateEntered = true;

                    set_lights(1, 1, 0);
                }

                if (currentTimeMs - lastToggledTimeMs > RED_YELLOW_TIME_MS) {
                    change_state(GREEN_LIGHT, currentTimeMs);
                }
                break;
            }
        }
    }
}
