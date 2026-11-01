#include <Arduino.h>
#include "Led.h"

Led::Led(uint8_t pin) : m_pin(pin)
{
}

void Led::init()
{
    pinMode(m_pin, OUTPUT);
    set(LedState::ON);
}

void Led::set(LedState state)
{
    m_state = state;
    digitalWrite(m_pin, state == LedState::ON ? HIGH : LOW);
}