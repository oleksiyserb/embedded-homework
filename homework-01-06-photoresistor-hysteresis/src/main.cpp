#include <Arduino.h>

#define ADC_PIN 16
#define PIN_OUT 15
#define THRESHOLD_DARK 2000
#define THRESHOLD_LIGHT 3000

void setup() {
  pinMode(ADC_PIN, INPUT);
  pinMode(PIN_OUT, OUTPUT);
}

void loop() {
  int adcValue = analogRead(ADC_PIN);

  if (adcValue < THRESHOLD_DARK) {
    digitalWrite(PIN_OUT, HIGH);
  }

  if (adcValue > THRESHOLD_LIGHT) {
    digitalWrite(PIN_OUT, LOW);
  }
}