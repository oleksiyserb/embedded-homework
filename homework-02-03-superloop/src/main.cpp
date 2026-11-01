// Завдання
// Керувати кількома незалежними процесами в одному loop() без блокування.
// Є 3 світлодіоди:
// LED1 блимає кожні 200 мс
// LED2 блимає кожні 500 мс
// LED3 блимає кожні 1000 мс
// Вимоги
// ❌ Заборонено використовувати delay()
// ❌ Заборонено використовувати while() для очікування часу у loop() 
// ✅ Дозволено використовувати millis()

#include <Arduino.h>

struct Led
{
    uint8_t pin;
    uint32_t interval;
    uint32_t previous;
    bool state;
};

Led leds[] = {
    {15, 200, 0, false},
    {16, 500, 0, false},
    {17, 1000, 0, false},
};

void setup() {
    for (auto& led: leds) {
        pinMode(led.pin, OUTPUT);
        digitalWrite(led.pin, LOW);
    }
}

void loop() {
    const uint32_t now = millis();

    for (auto& led: leds) {
        if (now - led.previous >= led.interval) {
            led.previous = now;
            led.state = !led.state;
            digitalWrite(led.pin, led.state ? HIGH : LOW);
        }
    }
}