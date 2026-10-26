#pragma once
#include <Arduino.h>

class Led
{
public:
    enum class State
    {
        OFF,
        ON
    };

    Led(uint8_t pin);

    void init();
    void setState(State state);

    State getState() const { return m_state; };
private:
    State m_state{State::OFF};
    uint8_t m_pin;
};