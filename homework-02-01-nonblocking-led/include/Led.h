#pragma once
#include <Arduino.h>

class Led
{
public:
    enum class LedState
    {
        OFF,
        ON
    };

    Led(uint8_t pin);

    void init();
    void set(LedState state);

    LedState getState() const { return m_state; };
private:
    LedState m_state{LedState::OFF};
    uint8_t m_pin;
};