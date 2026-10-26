#include <Arduino.h>
#include "Led.h"

Led::Led(uint8_t pin) : m_pin(pin)
{
}

void Led::init()
{
    pinMode(m_pin, OUTPUT);
    setState(State::ON);
}

void Led::setState(State state)
{
    m_state = state;
    digitalWrite(m_pin, state == State::ON ? HIGH : LOW);
}