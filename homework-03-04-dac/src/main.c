/*
 * Домашнє завдання: Програвання звуку на зумері (PWM)
 *
 * Вимоги до здачі:
 *   - Код і результат виконання на GitHub.
 *   - Назва репозиторію (або гілка чи коміт меседж) повинна містити
 *     номер модуля і номер уроку.
 *   - Посилання на репозиторій в LMS.
 *   - Обрати тільки одне завдання, не обов'язково робити обидва.
 *
 * Завдання 1
 *   Реалізувати неблокуючий аудіо-плеєр для зумера, який:
 *     - працює через PWM (LEDC);
 *     - використовує фіксований часовий тік, наприклад 50 мс;
 *     - не блокує виконання програми;
 *     - не використовує FreeRTOS task та delay.
 *
 * Завдання 2
 *   Реалізувати синтезатор на 2, 3 або більше нот для можливості зіграти
 *   просту мелодію. Приклади мелодій:
 *
 *     Baby Shark                   C D E | C D E | C D E
 *     Jingle Bells                 E E E | E E E | E G C D E
 *     We Will Rock You             C C D | C C D | C C D E
 *     Twinkle Twinkle Little Star  C C G G | A A G
 *                       спрощено:  C C D D | E E D
 *     Old MacDonald                G G G D E E D
 *                       спрощено:  C C C D E E D
 */

#include <driver/ledc.h>
#include <esp_log.h>
#include <esp_timer.h>

#include "melodies.h"

static const char *TAG = "player";

#define BUZZER_PIN 18

#define BUZZER_SPEED_MODE LEDC_LOW_SPEED_MODE
#define BUZZER_TIMER LEDC_TIMER_0
#define BUZZER_CHANNEL LEDC_CHANNEL_0

#define BUZZER_DUTY_RESOLUTION LEDC_TIMER_10_BIT
#define BUZZER_DUTY_SOUND 512   // 50% від 1023: найгучніший звук
#define BUZZER_DUTY_SILENCE 0

// Тік плеєра: кожні 50 мс плеєр робить один крок
#define TICK_US (50 * 1000)

// Пауза між мелодіями в плейлисті
#define MELODY_GAP_TICKS 40     // 2 с

static void buzzer_play(uint32_t freq_hz) {
    // Міняємо частоту таймера. Роздільна здатність та сама,
    // тому duty 512 і далі означає 50%
    ESP_ERROR_CHECK(ledc_set_freq(BUZZER_SPEED_MODE, BUZZER_TIMER, freq_hz));

    ESP_ERROR_CHECK(ledc_set_duty(BUZZER_SPEED_MODE, BUZZER_CHANNEL, BUZZER_DUTY_SOUND));
    ESP_ERROR_CHECK(ledc_update_duty(BUZZER_SPEED_MODE, BUZZER_CHANNEL));
}

static void buzzer_silence(void) {
    // Таймер далі тікає, але на піні постійний 0, тож мембрана не рухається
    ESP_ERROR_CHECK(ledc_set_duty(BUZZER_SPEED_MODE, BUZZER_CHANNEL, BUZZER_DUTY_SILENCE));
    ESP_ERROR_CHECK(ledc_update_duty(BUZZER_SPEED_MODE, BUZZER_CHANNEL));
}

// Стан плеєра: між тіками живе тут, а не на стеку
static size_t melody_index = 0; // Яка мелодія з плейлиста грає
static size_t note_index = 0;   // Яку ноту грати наступною
static uint8_t ticks_left = 0;  // Скільки тіків лишилось поточній ноті чи паузі

// Викликається таймером кожні TICK_US. Робить один крок і одразу виходить
static void player_tick(void *arg) {
    if (ticks_left == 0) {
        const melody_t *melody = &melodies[melody_index];

        if (note_index == melody->length) {
            // Мелодія скінчилась: пауза, потім наступна з плейлиста по колу
            buzzer_silence();
            ticks_left = MELODY_GAP_TICKS;
            melody_index = (melody_index + 1) % melodies_count;
            note_index = 0;
        } else {
            if (note_index == 0) {
                ESP_LOGI(TAG, "Граю: %s", melody->name);
            }

            const note_t *note = &melody->notes[note_index];

            if (note->freq_hz == REST) {
                buzzer_silence();
            } else {
                buzzer_play(note->freq_hz);
            }

            ticks_left = note->ticks;
            note_index++;
        }
    }

    if (ticks_left == 1) {
        // Останній тік ноти: "відпускаємо клавішу", щоб C C не злились
        buzzer_silence();
    }

    ticks_left--;
}

void app_main() {
    ledc_timer_config_t timer_config = {
        .speed_mode = BUZZER_SPEED_MODE,
        .duty_resolution = BUZZER_DUTY_RESOLUTION,
        .timer_num = BUZZER_TIMER,
        .freq_hz = NOTE_A4,                 // Частота ШІМ = висота ноти
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {
        .gpio_num = BUZZER_PIN,
        .speed_mode = BUZZER_SPEED_MODE,
        .channel = BUZZER_CHANNEL,
        .timer_sel = BUZZER_TIMER,
        .duty = BUZZER_DUTY_SILENCE,        // Стартуємо в тиші
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));

    esp_timer_create_args_t timer_args = {
        .callback = player_tick,
        .name = "player_tick"
    };
    esp_timer_handle_t tick_timer;
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, TICK_US));

    // app_main завершується: далі все робить таймер, процесор вільний
}
