// Домашнє завдання: усунення брязкоту контактів кнопки
//
// Код і результати — на GitHub, посилання на репозиторій — у LMS.
// Використовувати Arduino Framework або ESP-IDF.
//
// 1. [x] Базова реалізація (без debounce) — виконано
// - Налаштувати GPIO interrupt на FALLING.
// - При кожному interrupt збільшувати лічильник, виводити його в лог поза ISR.
// Очікується: кілька interrupt на одне натискання; можливі й при відпусканні.
//
// 2. Time-based debounce
// - Ігнорувати події, якщо від попереднього interrupt минуло < 50 мс.
// - Реалізувати перевірку поза ISR.
// Очікується: менше хибних спрацювань, але можливі при відпусканні.
//
// 3. State-based debounce
// - Interrupt лише сигналізує про подію.
// - У loop() / task приймати її, якщо кнопка досі натиснута.
// - Повторне натискання дозволяти після стабільного відпускання.
// Очікується: одна реакція на натискання; відпускання не викликає дії.
//
// 4. Polling + debounce (без interrupts)
// - Опитувати кнопку кожні 5–10 мс.
// - Реалізувати debounce як машину станів.
// Очікується: стабільна поведінка з дещо більшою затримкою.
//
// 5. Hardware debounce
// - Додати RC-фільтр: 100 нФ, 100 Ом; підтягувальний резистор — 10 кОм.
// - Повторити завдання 1–4, порівняти результати з RC і без RC.
//
// 6. Порівняльна таблиця
// Для однакової кількості натискань заповнити таблицю без RC та з RC.
//
// Метод        | Хибні спрацювання | Середня затримка, мкс | Складність | Коментар
// -------------|------------------|----------------------|------------|---------------------------------------
// Без debounce | 2                | 5.25                 | низька     | Без RC: 10 натискань, 12 спрацювань
// Time-based   | 2                | 3.42                 | низька     | Без RC: 10 натискань, 12 спрацювань; зайві спрацювання при відпусканні
// State-based  | 0                | 4.60                 | середня    | Без RC: 20 натискань, 20 спрацювань; зайвих спрацювань не зафіксовано
// Polling      | 0                | 50001                | середня    | Без RC: 20 натискань, 20 спрацювань; затримка за 19 повними вимірами
// Hardware RC  | 35               | 23.73                | середня    | З RC (100 нФ): 4 натискання, 39 переривань; 11 повних вимірів затримки
// Time-based   | 9                | 3.63                 | низька     | З RC: 10 натискань, 19 спрацювань; 19 вимірів затримки
// State-based  | 0                | 4.60                 | середня    | З RC: 10 натискань, 10 спрацювань; 10 вимірів затримки
// Polling      | 0                | 50001                | середня    | З RC: 20 натискань, 20 спрацювань; 20 повних вимірів затримки
//
// Затримка: від запису часу в ISR до обробки в loop().
// Для Polling: від першого зчитаного LOW до підтвердження натискання;
// час до першого опитування (до 5 мс за регулярного loop()) сюди не входить.
// Середнє: без debounce і Time-based — по 12 вимірів; State-based — 20 вимірів.
// Polling без RC — 19 повних вимірів (останній обірваний); з RC — 20; усі по 50001 мкс.
//
// Hardware RC — базова реалізація з RC, без програмного debounce.
// Її програмне вимірювання затримки не враховує затримку самого RC-фільтра.

#include <Arduino.h>

constexpr uint8_t BUTTON_PIN{15};
constexpr uint32_t BAUD_RATE{9600};
constexpr uint32_t POLL_INTERVAL_US{5000};
constexpr uint32_t DEBOUNCE_US{50000};

enum class ButtonState {
    Released,
    ConfirmingPress,
    Pressed,
    ConfirmingRelease
};

void setup() {
    Serial.begin(BAUD_RATE);

    // Зовнішня підтяжка 10 кОм до 3,3 В уже підключена в RC-схемі.
    pinMode(BUTTON_PIN, INPUT);
}

void loop() {
    static uint32_t acceptedCount{0};
    static ButtonState state{ButtonState::Released};
    static uint32_t lastPoll{0};
    static uint32_t stateStarted{0};

    uint32_t now = micros();
    if (now - lastPoll < POLL_INTERVAL_US) return;
    lastPoll = now;

    bool isPressed = digitalRead(BUTTON_PIN) == LOW;

    switch (state) {
        case ButtonState::Released:
            if (isPressed) {
                stateStarted = now;
                state = ButtonState::ConfirmingPress;
            }
            break;

        case ButtonState::ConfirmingPress:
            if (!isPressed) {
                state = ButtonState::Released;
            } else if (now - stateStarted >= DEBOUNCE_US) {
                uint32_t latency = micros() - stateStarted;
                state = ButtonState::Pressed;
                acceptedCount++;

                Serial.println("Count: ");
                Serial.println(acceptedCount);
                Serial.println("Latency, us: ");
                Serial.println(latency);
            }
            break;

        case ButtonState::Pressed:
            if (!isPressed) {
                stateStarted = now;
                state = ButtonState::ConfirmingRelease;
            }
            break;

        case ButtonState::ConfirmingRelease:
            if (isPressed) {
                state = ButtonState::Pressed;
            } else if (now - stateStarted >= DEBOUNCE_US) {
                state = ButtonState::Released;
            }
            break;
    }
}
