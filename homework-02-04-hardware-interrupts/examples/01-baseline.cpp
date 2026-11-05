// Дослід 1: без програмного debounce. Скопіювати цей файл у src/main.cpp.
// З RC: INPUT; без RC (внутрішня підтяжка): INPUT_PULLUP.
#include <Arduino.h>

constexpr uint8_t BUTTON_PIN{15};
constexpr uint32_t BAUD_RATE{9600};
constexpr uint8_t BUTTON_INPUT_MODE{INPUT};

volatile uint32_t count{0};
volatile uint32_t started{0};
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void ARDUINO_ISR_ATTR onButtonInterrupt() {
    portENTER_CRITICAL_ISR(&mux);
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
    portENTER_CRITICAL(&mux);
    uint32_t currentCount = count;
    uint32_t eventStarted = started;
    portEXIT_CRITICAL(&mux);

    if (currentCount == lastPrintedCount) return;
    uint32_t latency = micros() - eventStarted;
    lastPrintedCount = currentCount;
    Serial.println("Count: ");
    Serial.println(currentCount);
    Serial.println("Latency, us: ");
    Serial.println(latency);
}
