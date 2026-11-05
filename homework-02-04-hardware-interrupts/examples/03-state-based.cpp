// Дослід 3: одна дія до стабільного відпускання протягом 50 мс.
// Скопіювати у src/main.cpp. З RC: INPUT; без RC: INPUT_PULLUP.
#include <Arduino.h>

constexpr uint8_t BUTTON_PIN{15};
constexpr uint32_t BAUD_RATE{9600};
constexpr uint8_t BUTTON_INPUT_MODE{INPUT};
constexpr uint32_t RELEASE_DEBOUNCE_US{50000};

volatile bool interruptPending{false};
volatile uint32_t started{0};
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void ARDUINO_ISR_ATTR onButtonInterrupt() {
    portENTER_CRITICAL_ISR(&mux);
    if (!interruptPending) {
        started = micros();
        interruptPending = true;
    }
    portEXIT_CRITICAL_ISR(&mux);
}

void setup() {
    Serial.begin(BAUD_RATE);
    pinMode(BUTTON_PIN, BUTTON_INPUT_MODE);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButtonInterrupt, FALLING);
}

void loop() {
    static uint32_t acceptedCount{0};
    static bool pressAccepted{false};
    static bool releaseTiming{false};
    static uint32_t releaseStarted{0};

    portENTER_CRITICAL(&mux);
    bool hasEvent = interruptPending;
    uint32_t eventStarted = started;
    interruptPending = false;
    portEXIT_CRITICAL(&mux);

    bool isPressed = digitalRead(BUTTON_PIN) == LOW;
    uint32_t now = micros();
    if (pressAccepted) {
        if (isPressed || hasEvent) {
            releaseTiming = false;
        } else if (!releaseTiming) {
            releaseStarted = now;
            releaseTiming = true;
        } else if (now - releaseStarted >= RELEASE_DEBOUNCE_US) {
            pressAccepted = false;
            releaseTiming = false;
        }
        return;
    }

    if (hasEvent && isPressed) {
        uint32_t latency = micros() - eventStarted;
        pressAccepted = true;
        releaseTiming = false;
        acceptedCount++;
        Serial.println("Count: ");
        Serial.println(acceptedCount);
        Serial.println("Latency, us: ");
        Serial.println(latency);
    }
}
