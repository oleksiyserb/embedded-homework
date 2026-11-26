#pragma once

#include <stddef.h>
#include <stdint.h>

// Частоти нот першої октави, Гц
#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_C5 523
#define REST    0       // Пауза: не нота, а тиша

// Тривалості в тіках плеєра (1 тік = 50 мс)
#define EIGHTH  4       // 200 мс
#define QUARTER 8       // 400 мс
#define HALF    16      // 800 мс
#define WHOLE   32      // 1600 мс

// Одна нота мелодії: що грати і скільки тіків
typedef struct {
    uint32_t freq_hz;
    uint8_t ticks;      // Має бути >= 2: останній тік ноти завжди тиша
} note_t;

// Мелодія: назва для логу і масив нот
typedef struct {
    const char *name;
    const note_t *notes;
    size_t length;
} melody_t;

// Плейлист з усіх мелодій завдання
extern const melody_t melodies[];
extern const size_t melodies_count;
