/*
 * ЗАДАЧА
 * ======
 * Реалізувати керування витяжним вентилятором, який кожну годину
 * вмикається на 15 хвилин, після чого автоматично вимикається.
 *
 * Вимоги:
 *   [x] Вентилятор = двигун або реле
 *   [x] Таймери мають працювати незалежно від коду в циклі
 *   [x] Не використовувати delay()
 *   [x] Цикл повторюється без участі loop() / task
 *   [x] Час роботи і період задаються змінними або макросами
 *
 * Для налагодження можна:
 *   [x] замінити 1 годину на кілька секунд (20 с)
 *   [x] замінити 15 хв на пропорційно менший відрізок часу (5 с)
 *
 * Опційно:
 *   [ ] Watchdog
 *   [ ] Логування станів
 *   [ ] Захист від повторного запуску
 *   [x] LED-індикатори, що дублюють поточний стан
 */

#include <Arduino.h>

constexpr uint64_t RELAY_INTERVAL{20000}; // 1H = 3 600 000 milliseconds | 20s - test
constexpr uint64_t WORK_INTERVAL{5000}; // 15m = 900 000 milliseconds | 5s - test
static_assert(RELAY_INTERVAL >  WORK_INTERVAL, "Relay interval must be larger than work interval");

constexpr uint8_t RELAY_PIN{15};
constexpr uint8_t GREEN_LED{16};
constexpr uint8_t RED_LED{17};

constexpr uint32_t MICROS_IN_MILLIS{1000};

hw_timer_t* timer = nullptr;

volatile bool isRelayActive{false};

void IRAM_ATTR onTimer()
{
    if (!isRelayActive) {
        digitalWrite(RELAY_PIN, HIGH);
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(RED_LED, LOW);

        isRelayActive = true;
        timerAlarmWrite(timer, WORK_INTERVAL * MICROS_IN_MILLIS, true);
    } else {
        digitalWrite(RELAY_PIN, LOW);
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(RED_LED, HIGH);

        isRelayActive = false;
        timerAlarmWrite(timer, (RELAY_INTERVAL - WORK_INTERVAL) * MICROS_IN_MILLIS, true);
    }
}

void setup() {
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    digitalWrite(RELAY_PIN, LOW);

    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, RELAY_INTERVAL * MICROS_IN_MILLIS,  true);

    timerAlarmEnable(timer);
}

void loop() {
}