# Домашня робота 2.6: debounce кнопки на ESP-IDF

Завдання уроку 2.4 ([ТЗ і результати Arduino-версії](../homework-02-04-hardware-interrupts/README.md)) перенесено з Arduino Framework на **ESP-IDF**. Усі чотири способи обробки кнопки реалізовано на C через драйвер `driver/gpio.h`, `esp_timer` і FreeRTOS.

## Обладнання та збірка

| Параметр | Значення |
| --- | --- |
| Плата | Adafruit Feather ESP32-S3 (без PSRAM) |
| Кнопка | GPIO15, активний LOW (натиснута = 0) |
| Підтяжка | без RC: внутрішня (`GPIO_PULLUP_ENABLE`); з RC: зовнішня 10 кОм, `GPIO_PULLUP_DISABLE` |
| Фреймворк | ESP-IDF через PlatformIO (`framework = espidf`) |
| Тік FreeRTOS | 100 Гц (`CONFIG_FREERTOS_HZ=100`), тобто `vTaskDelay(1)` = 10 мс |
| Лог | `ESP_LOGI`, 115200 бод |

Варіанти лежать у [examples/](examples/). Щоб запустити один із них, скопіюйте файл у `src/main.c`, а тоді виконайте `pio run -t upload -t monitor`. Зараз у `src/main.c` лежить варіант 4 (polling).

## Реалізації

### 1. Без debounce: [01-baseline.c](examples/01-baseline.c)

Переривання спрацьовує на спадному фронті (`GPIO_INTR_NEGEDGE`). ISR збільшує лічильник і запам'ятовує час події під `portENTER_CRITICAL_ISR`. Друк виконується в `app_main`, поза ISR: раз на тік вона перевіряє, чи змінився лічильник.

### 2. Debounce за часом: [02-time-based.c](examples/02-time-based.c)

ISR лише кладе час фронту (`esp_timer_get_time()`) у чергу FreeRTOS (`xQueueSendFromISR`). `app_main` чекає на черзі (`xQueueReceive`) і перевіряє кожен фронт:

- подія приймається, якщо від **попереднього переривання** минуло ≥ 50 мс;
- час попереднього переривання **оновлюється на кожному перериванні**, зокрема на відкинутих.

Черга гарантує, що головний цикл бачить кожен фронт. В Arduino-версії зберігалася лише остання пара часів, і перший фронт пачки міг загубитися. У лог виводиться і прийнятий лічильник, і `raw`, тобто всі переривання.

### 3. Debounce за станом: [03-state-based.c](examples/03-state-based.c)

ISR тільки ставить прапорець `interruptPending` і запам'ятовує час першого фронту. Решту робить автомат у `app_main`, який перевіряє стан раз на тік:

| Стан | Умова переходу |
| --- | --- |
| IDLE → PRESSED | була подія **і** пін зараз LOW, тоді Count++ |
| PRESSED → IDLE | пін HIGH **50 мс поспіль** і за цей час не було жодного фронту |

У стані PRESSED усі фронти ігноруються, тож брязкіт під час відпускання не викликає дії.

### 4. Polling: [04-polling.c](examples/04-polling.c)

Переривання вимкнені (`GPIO_INTR_DISABLE`). Кнопка опитується раз на тік (**10 мс**) машиною з чотирьох станів:

```
RELEASED → CONFIRMING_PRESSED → PRESSED → CONFIRMING_RELEASED → RELEASED
```

Кожен перехід у «натиснута» або «відпущена» підтверджується **50 мс** стабільного рівня. Дія (Count++) виконується лише під час переходу в `PRESSED`. Тут `latency` рахується від першого опитування, яке побачило LOW, тому вона завжди становить приблизно 50 мс.

## Відмінності від Arduino-версії (2.4)

| Arduino | ESP-IDF |
| --- | --- |
| `attachInterrupt(..., FALLING)` | `gpio_config` + `gpio_install_isr_service` + `gpio_isr_handler_add` |
| `micros()` (`uint32_t`, переповнюється за ~71 хв) | `esp_timer_get_time()` (`int64_t`) |
| `loop()` без пауз | задача `app_main` з `vTaskDelay(1)`; у варіанті 2 вона блокується на черзі |
| `pinMode(INPUT / INPUT_PULLUP)` | поля `.pull_up_en` / `.pull_down_en` у `gpio_config_t` |
| `Serial.println` | `ESP_LOGI` |
| Polling кожні 5 мс | Polling кожні 10 мс: це мінімальний період `vTaskDelay` при тіку 100 Гц |

## Перевірка

Усі чотири варіанти зібрано через `pio run` (ESP-IDF, `adafruit_feather_esp32s3_nopsram`) без попереджень у коді проєкту. Схема RC-фільтра, вимірювання й висновки щодо методів наведено у [звіті 2.4](../homework-02-04-hardware-interrupts/REPORT.md).
