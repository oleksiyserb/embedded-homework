// Концепції Embedded C++:

// volatile, const / constexpr, static
// Відмова від динамічної пам’яті та важких контейнерів STL
// Superloop-архітектура
// Безпечна робота з периферією (LED, кнопка)
// 1. Переписати “класичний Arduino blink” у Embedded C++
//// Використати enum class для стану LED (On / Off).
//// Використати constexpr для номера піну та інтервалу блимання.
//// Уникнути глобальних змінних (крім мінімально необхідних для стану/ISR).
//// Не використовувати delay(); реалізувати неблокуючий код у superloop за допомогою millis().
//// Створити клас Led з методами init() та set(LedState state).
// 2. Використати static const / constexpr для параметрів

// Створити клас-конфігурацію або структуру з static const / constexpr для додаткових налаштувань.
// Повністю усунути «магічні числа», винісши їх у константи.
// 3. Виміряти час виконання superloop (Опційно)

// Виміряти час однієї ітерації loop() без delay().
// Виводити середній час або показники у Serial Monitor кожні 1000 ітерацій.
// 4. Додати кнопку з обробкою через переривання (опційно)

// Під'єднати кнопку до GPIO та налаштувати переривання (attachInterrupt)
// Використати volatile bool button прапорець для фіксації події натискання в ISR.
// Змінювати режим роботи LED при кожному натисканні: Блимання → Постійно увімкнено → Постійно вимкнено → Блимання.
// Зробити ISR мінімальним (без викликів Serial.print чи затримок).
// Всю логіку перемикання режимів та антидребезгу (debouncing) обробляти в loop().

#include <Arduino.h>
#include "Led.h"

constexpr uint8_t LED_OUT{15};
constexpr unsigned long BLINK_INTERVAL{1000};

// Initialize LED
Led led{LED_OUT};

// If we don't need global variable as in task use this:
// Led& getLed()
// {
//     static Led led{LED_OUT};
//     return led;
// }

void setup()
{
    led.init();
}

void loop()
{
    static unsigned long lastToggleTime{0};

    unsigned long now = millis();
    unsigned long elapsedTime = now - lastToggleTime;

    if (elapsedTime >= BLINK_INTERVAL)
    {
        const Led::State ledState = (led.getState() == Led::State::OFF) ? Led::State::ON : Led::State::OFF;
        led.setState(ledState);
        lastToggleTime = now;
    }
}