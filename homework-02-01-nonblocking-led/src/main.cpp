// Концепції Embedded C++:

// volatile, const / constexpr, static
// Відмова від динамічної пам’яті та важких контейнерів STL
// Superloop-архітектура
// Безпечна робота з периферією (LED, кнопка)
// 1. [x] Переписати “класичний Arduino blink” у Embedded C++ — виконано.
//    [x] Використати enum class для стану LED (On / Off).
//    [x] Використати constexpr для номера піну та інтервалу блимання.
//    [x] Уникнути глобальних змінних (крім мінімально необхідних для стану/ISR).
//        Об'єкт led — локальний static усередині getLed().
//    [x] Не використовувати delay(); реалізувати неблокуючий код у superloop за допомогою millis().
//    [x] Створити клас Led з методами init() та set(LedState state).
// 2. [x] Використати static const / constexpr для параметрів — виконано.
// [x] Створити клас-конфігурацію або структуру з static const / constexpr для додаткових налаштувань.
//     Config містить static constexpr LED_OUT та BLINK_INTERVAL.
// [x] Повністю усунути «магічні числа», винісши їх у константи.
// 3. [x] Виміряти час виконання superloop (Опційно) — виконано.

// [x] Виміряти час однієї ітерації loop() без delay().
//     micros() вимірює логіку LED без накопичення статистики та друку звіту.
// [x] Виводити середній час або показники у Serial Monitor кожні 1000 ітерацій.
// 4. Додати кнопку з обробкою через переривання (опційно)
// [x] Під'єднати кнопку до GPIO та налаштувати переривання (attachInterrupt)
// [x] Використати volatile bool button прапорець для фіксації події натискання в ISR.
// [x] Змінювати режим роботи LED при кожному натисканні: Блимання → Постійно увімкнено → Постійно вимкнено → Блимання.
// [x] Зробити ISR мінімальним (без викликів Serial.print чи затримок).
// [x] Всю логіку перемикання режимів та антидребезгу (debouncing) обробляти в loop().

#include <Arduino.h>
#include "Led.h"

struct Config
{
    static constexpr uint8_t LED_OUT{15};
    static constexpr uint8_t BUTTON{16};
    static constexpr uint32_t BLINK_INTERVAL{1000};
    static constexpr uint8_t DEBOUNCE_MS{40};
    static constexpr uint32_t BAUD_RATE{9600};
};

enum class LedMode
{
    Blink,
    AlwaysOn,
    AlwaysOff
};

// If we don't need global variable as in task use this:
Led &getLed()
{
    static Led led{Config::LED_OUT};
    return led;
}

// ISR: прапорець означає зміну рівня, включно з відпусканням кнопки.
volatile bool buttonChanged{false};

void ARDUINO_ISR_ATTR onButtonChanged()
{
    buttonChanged = true;
}

void setup()
{
    Serial.begin(Config::BAUD_RATE);

    getLed().init();

    pinMode(Config::BUTTON, INPUT_PULLUP);
    attachInterrupt(Config::BUTTON, onButtonChanged, CHANGE);

    Serial.println("Початок вимірювання");
}

void loop()
{
    static LedMode mode{LedMode::Blink};

    constexpr uint16_t REPORT_EVERY_ITERATIONS{1000};
    static uint16_t count{0};
    static uint32_t totalDurationUs{0};

    const uint32_t start = micros();

    static uint32_t lastToggleTime{0};

    const uint32_t now = millis();
    const uint32_t elapsedTime = now - lastToggleTime;

    if (mode == LedMode::Blink && elapsedTime >= Config::BLINK_INTERVAL)
    {
        const Led::LedState ledState =
        (getLed().getState() == Led::LedState::OFF)
            ? Led::LedState::ON
            : Led::LedState::OFF;

        getLed().set(ledState);
        lastToggleTime = now;
    }

    static uint32_t lastDebounceTime{0};
    static int lastButtonReading{HIGH};
    static int stableButtonState{HIGH};

    // ISR не має встановити прапорець між його читанням та очищенням.
    noInterrupts();
    const bool hasButtonEvent = buttonChanged;
    buttonChanged = false;
    interrupts();

    const int buttonReading = digitalRead(Config::BUTTON);
    if (hasButtonEvent || buttonReading != lastButtonReading)
    {
        lastDebounceTime = now;
        lastButtonReading = buttonReading;
    }

    const uint32_t elapsedDebounce = now - lastDebounceTime;

    if (buttonReading != stableButtonState &&
        elapsedDebounce >= Config::DEBOUNCE_MS)
    {
        stableButtonState = buttonReading;

        // INPUT_PULLUP: лише підтверджений LOW означає нове натискання.
        if (stableButtonState == LOW)
        {
            switch (mode)
            {
            case LedMode::Blink:
                mode = LedMode::AlwaysOn;
                getLed().set(Led::LedState::ON);
                break;
            case LedMode::AlwaysOn:
                mode = LedMode::AlwaysOff;
                getLed().set(Led::LedState::OFF);
                break;
            case LedMode::AlwaysOff:
                mode = LedMode::Blink;
                getLed().set(Led::LedState::ON);
                lastToggleTime = now;
                break;
            }
        }
    }

    const uint32_t duration = micros() - start;
    totalDurationUs += duration;
    ++count;

    if (count >= REPORT_EVERY_ITERATIONS)
    {
        Serial.print("Середній час ітерації, мкс: ");
        Serial.println(
            static_cast<double>(totalDurationUs) / count, 2
        );

        totalDurationUs = 0;
        count = 0;
    }
}
