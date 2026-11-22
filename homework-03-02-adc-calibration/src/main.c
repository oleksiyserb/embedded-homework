/*
 * Завдання
 *
 * 1. Підключіть потенціометр (~10 кОм) у вигляді подільника напруги.
 *    Візьміть до уваги опорну напругу, атенюацію та розрядність
 *    (їх також можна додати до таблиці для інформації).
 * 2. Налаштуйте АЦП та зчитуйте значення кожні ~100 мс.
 * 3. Зробіть кілька вимірювань. Для кожного вимірювання:
 *    - зчитайте RAW значення АЦП та обчисліть напругу за формулою;
 *    - зчитайте напругу за допомогою каліброваного АЦП;
 *    - обчисліть похибку (%) між обчисленою та каліброваною напругою.
 * 4. Виведіть у серійну консоль таблицю, наприклад:
 *
 *     RAW   U_manual(mV)   U_cali(mV)   Error(%)
 *    ------------------------------------------
 *     512       412.1          398        3.55
 *    1024       824.2          801        2.89
 *    2048      1648.4         1589        3.74
 *    3072      2472.6         2398        3.10
 *
 * Опційно
 *
 * - Побудуйте графіки.
 * - Знайдіть ділянки нелінійності (візуально).
 */

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_ll_adc.h"  // VREFINT_CAL_ADDR, __LL_ADC_CALC_* макроси

// Потенціометр: PA1 = ADC1_IN1 (PA0 на Blackpill зайнятий кнопкою KEY)
#define POT_PIN     GPIO_PIN_1
#define POT_CHANNEL ADC_CHANNEL_1

#define VDDA_NOMINAL_MV 3300
#define ADC_MAX         __LL_ADC_DIGITAL_SCALE(LL_ADC_RESOLUTION_12B)

// Консоль: USART1, PA9 = TX, PA10 = RX, 115200 8N1 (USB-UART адаптер)

static ADC_HandleTypeDef hadc1;
static UART_HandleTypeDef huart1;

void SysTick_Handler(void) { HAL_IncTick(); }

static void error_handler(void) {
    __disable_irq();
    while (1) {}
}

// printf -> USART1
int _write(int fd, char *ptr, int len) {
    (void)fd;
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}

static void uart_init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {
        .Pin = GPIO_PIN_9 | GPIO_PIN_10,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_PULLUP,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = GPIO_AF7_USART1,
    };
    HAL_GPIO_Init(GPIOA, &gpio);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK) error_handler();
}

static void adc_init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {
        .Pin = POT_PIN,
        .Mode = GPIO_MODE_ANALOG,
        .Pull = GPIO_NOPULL,
    };
    HAL_GPIO_Init(GPIOA, &gpio);

    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;  // 16 МГц / 4 = 4 МГц (макс. 36)
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;           // 0..4095
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;              // oneshot
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) error_handler();
}

// Один oneshot-замір вибраного каналу (POT_CHANNEL або ADC_CHANNEL_VREFINT).
// 480 циклів семплування: VREFINT вимагає >= 10 мкс, для потенціометра теж не завадить.
static uint32_t adc_read(uint32_t channel) {
    ADC_ChannelConfTypeDef ch = {
        .Channel = channel,
        .Rank = 1,
        .SamplingTime = ADC_SAMPLETIME_480CYCLES,
    };
    if (HAL_ADC_ConfigChannel(&hadc1, &ch) != HAL_OK) error_handler();

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    uint32_t raw = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return raw;
}

int main(void) {
    HAL_Init();  // тактування за замовчуванням: HSI 16 МГц, SysTick 1 мс
    uart_init();
    adc_init();

    printf("\r\nADC1: 12 bit, Vref = VDDA, no attenuation (0..VDDA)\r\n");
    printf(" RAW   U_manual(mV)   U_cali(mV)   Error(%%)   VDDA(mV)\r\n");
    printf("-------------------------------------------------------\r\n");

    while (1) {
        uint32_t raw = adc_read(POT_CHANNEL);
        uint32_t vrefint_raw = adc_read(ADC_CHANNEL_VREFINT);
        
        float u_manual = raw * (float)VDDA_NOMINAL_MV / ADC_MAX;
        uint32_t vdda = __LL_ADC_CALC_VREFANALOG_VOLTAGE(vrefint_raw, LL_ADC_RESOLUTION_12B);
        uint32_t u_cali = __LL_ADC_CALC_DATA_TO_VOLTAGE(vdda, raw, LL_ADC_RESOLUTION_12B);

        printf("%4" PRIu32 "   %12.1f   %10" PRIu32 "   ", raw, u_manual, u_cali);
        if (u_cali == 0) {
            printf("%8s", "-");
        } else {
            float error_value = fabsf(u_manual - u_cali) / u_cali * 100.0f;
            printf("%8.2f", error_value);
        }
        printf("   %8" PRIu32 "\r\n", vdda);

        HAL_Delay(100);
    }
}