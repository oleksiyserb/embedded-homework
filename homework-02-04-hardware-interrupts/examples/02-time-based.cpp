// Дослід 2: проміжок від попереднього переривання >= 50 мс.
// Скопіювати у src/main.cpp. З RC: INPUT; без RC: INPUT_PULLUP.
#include <Arduino.h>

constexpr uint8_t BUTTON_PIN{15};
constexpr uint32_t BAUD_RATE{9600};
constexpr uint8_t BUTTON_INPUT_MODE{INPUT};
constexpr uint32_t DEBOUNCE{50000};
constexpr uint8_t FIRST_ITERATION{1};

volatile uint32_t count{0};
volatile uint32_t started{0};
volatile uint32_t previousStarted{0};
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void ARDUINO_ISR_ATTR onButtonInterrupt() {
    portENTER_CRITICAL_ISR(&mux);
    previousStarted = started;
    started = micros();
    count++;
    portEXIT_CRITICAL_ISR(&mux);
}

void setup() {
    Serial.begin(BAUD_RATE);
    pinMode(BUTTON_PIN, BUTTON_INPUT_MODE);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButtonInterrupt, FALLING);
}

void loop() {
    static uint32_t lastPrintedCount{0};
    static uint32_t acceptedCount{0};
    portENTER_CRITICAL(&mux);
    uint32_t currentCount = count;
    uint32_t eventStarted = started;
    uint32_t previousEventStarted = previousStarted;
    portEXIT_CRITICAL(&mux);

    if (currentCount == lastPrintedCount) return;
    lastPrintedCount = currentCount;
    uint32_t elapsed = eventStarted - previousEventStarted;

    // Час попереднього переривання оновлюється в ISR навіть для відкинутих подій.
    // Зберігається лише остання пара часів: це не черга всіх переривань.
    if (currentCount == FIRST_ITERATION || elapsed >= DEBOUNCE) {
        uint32_t latency = micros() - eventStarted;
        acceptedCount++;
        Serial.println("Count: ");
        Serial.println(acceptedCount);
        Serial.println("Latency, us: ");
        Serial.println(latency);
    }
}
