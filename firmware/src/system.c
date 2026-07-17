#include "stm32f1xx.h"

enum {
    HSI_CLOCK_HZ = 8000000U,
    HSE_CLOCK_HZ = 16000000U,
};

uint32_t SystemCoreClock = HSI_CLOCK_HZ;

void SystemInit(void)
{
    SystemCoreClock = HSI_CLOCK_HZ;
    SCB->VTOR = FLASH_BASE;
}

void SystemCoreClockUpdate(void)
{
    static const uint8_t ahb_prescaler_shift[16] = {
        0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
        1U, 2U, 3U, 4U, 6U, 7U, 8U, 9U,
    };
    uint32_t system_clock;

    switch (RCC->CFGR & RCC_CFGR_SWS) {
    case RCC_CFGR_SWS_HSE:
        system_clock = HSE_CLOCK_HZ;
        break;

    case RCC_CFGR_SWS_PLL: {
        uint32_t pll_input = HSI_CLOCK_HZ / 2U;
        if ((RCC->CFGR & RCC_CFGR_PLLSRC) != 0U) {
            pll_input = HSE_CLOCK_HZ;
            if ((RCC->CFGR & RCC_CFGR_PLLXTPRE) != 0U) {
                pll_input /= 2U;
            }
        }

        uint32_t multiplier = ((RCC->CFGR & RCC_CFGR_PLLMULL) >> 18U) + 2U;
        if (multiplier > 16U) {
            multiplier = 16U;
        }
        system_clock = pll_input * multiplier;
        break;
    }

    case RCC_CFGR_SWS_HSI:
    default:
        system_clock = HSI_CLOCK_HZ;
        break;
    }

    const uint32_t prescaler_index = (RCC->CFGR & RCC_CFGR_HPRE) >> 4U;
    SystemCoreClock = system_clock >> ahb_prescaler_shift[prescaler_index];
}
