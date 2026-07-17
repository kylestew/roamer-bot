#include "board.h"

#include "stm32f1xx.h"

enum {
    HSI_CLOCK_HZ = 8000000U,
    PLL_CLOCK_HZ = 48000000U,
    HSE_STARTUP_TIMEOUT = 1000000U,
};

static const uint32_t LED_PIN = 1UL << 13U;
static const uint32_t MOTOR_OUTPUT_PINS =
    (1UL << 6U) | (1UL << 7U) | (1UL << 12U) |
    (1UL << 13U) | (1UL << 14U) | (1UL << 15U);

static volatile uint32_t milliseconds;

static void gpio_configure_push_pull_output(GPIO_TypeDef *gpio, uint32_t pin)
{
    volatile uint32_t *configuration_register;
    uint32_t shift;

    if (pin < 8U) {
        configuration_register = &gpio->CRL;
        shift = pin * 4U;
    } else {
        configuration_register = &gpio->CRH;
        shift = (pin - 8U) * 4U;
    }

    uint32_t configuration = *configuration_register;
    configuration &= ~(0xFUL << shift);
    configuration |= 0x2UL << shift; /* 2 MHz, general-purpose push-pull. */
    *configuration_register = configuration;
}

static void safe_gpio_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
    (void)RCC->APB2ENR;

    /* Establish safe output levels before changing any pin to output mode. */
    GPIOB->BSRR = MOTOR_OUTPUT_PINS << 16U;
    GPIOC->BSRR = LED_PIN; /* PC13 LED is active-low, so high is off. */

    gpio_configure_push_pull_output(GPIOB, 6U);  /* DRV_R_PH */
    gpio_configure_push_pull_output(GPIOB, 7U);  /* DRV_R_EN */
    gpio_configure_push_pull_output(GPIOB, 12U); /* DRV_L_SLEEP_N */
    gpio_configure_push_pull_output(GPIOB, 13U); /* DRV_L_PH */
    gpio_configure_push_pull_output(GPIOB, 14U); /* DRV_L_EN */
    gpio_configure_push_pull_output(GPIOB, 15U); /* DRV_R_SLEEP_N */
    gpio_configure_push_pull_output(GPIOC, 13U); /* LED_HEARTBEAT */
}

static bool wait_for_bits(volatile uint32_t *reg, uint32_t mask, uint32_t expected)
{
    for (uint32_t remaining = HSE_STARTUP_TIMEOUT; remaining > 0U; --remaining) {
        if ((*reg & mask) == expected) {
            return true;
        }
        __NOP();
    }

    return false;
}

static bool clock_init(void)
{
    RCC->CR |= RCC_CR_HSEON;
    if (!wait_for_bits(&RCC->CR, RCC_CR_HSERDY, RCC_CR_HSERDY)) {
        RCC->CR &= ~RCC_CR_HSEON;
        SystemCoreClock = HSI_CLOCK_HZ;
        return false;
    }

    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_0;

    uint32_t clock_configuration = RCC->CFGR;
    clock_configuration &= ~(RCC_CFGR_SW | RCC_CFGR_HPRE | RCC_CFGR_PPRE1 |
                             RCC_CFGR_PPRE2 | RCC_CFGR_ADCPRE |
                             RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE |
                             RCC_CFGR_PLLMULL | RCC_CFGR_USBPRE);
    clock_configuration |= RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_ADCPRE_DIV4 |
                           RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE_HSE_DIV2 |
                           RCC_CFGR_PLLMULL6 | RCC_CFGR_USBPRE;
    RCC->CFGR = clock_configuration;

    RCC->CR |= RCC_CR_PLLON;
    if (!wait_for_bits(&RCC->CR, RCC_CR_PLLRDY, RCC_CR_PLLRDY)) {
        RCC->CR &= ~(RCC_CR_PLLON | RCC_CR_HSEON);
        FLASH->ACR = 0U;
        SystemCoreClock = HSI_CLOCK_HZ;
        return false;
    }

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    if (!wait_for_bits(&RCC->CFGR, RCC_CFGR_SWS, RCC_CFGR_SWS_PLL)) {
        RCC->CFGR &= ~RCC_CFGR_SW;
        SystemCoreClock = HSI_CLOCK_HZ;
        return false;
    }

    SystemCoreClock = PLL_CLOCK_HZ;
    return true;
}

static void systick_init(bool external_clock_ready)
{
    const uint32_t ticks_per_millisecond =
        external_clock_ready ? (PLL_CLOCK_HZ / 1000U) : (HSI_CLOCK_HZ / 1000U);

    milliseconds = 0U;
    SysTick->LOAD = ticks_per_millisecond - 1U;
    SysTick->VAL = 0U;
    NVIC_SetPriority(SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
}

bool board_init(void)
{
    __disable_irq();
    safe_gpio_init();
    const bool external_clock_ready = clock_init();
    systick_init(external_clock_ready);
    __enable_irq();

    return external_clock_ready;
}

void board_led_set(bool on)
{
    GPIOC->BSRR = on ? (LED_PIN << 16U) : LED_PIN;
}

uint32_t board_millis(void)
{
    return milliseconds;
}

void SysTick_Handler(void)
{
    ++milliseconds;
}
