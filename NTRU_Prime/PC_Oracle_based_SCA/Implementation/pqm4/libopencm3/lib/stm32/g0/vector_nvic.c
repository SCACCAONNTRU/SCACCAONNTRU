/* This file is part of the libopencm3 project.
 *
 * It was generated by the irq2nvic_h script.
 *
 * This part needs to get included in the compilation unit where
 * blocking_handler gets defined due to the way #pragma works.
 */


/** @defgroup CM3_nvic_isrdecls_STM32G0 User interrupt service routines (ISR) defaults for STM32 G0 series
    @ingroup CM3_nvic_isrdecls

    @{*/

void wwdg_isr(void) __attribute__((weak, alias("blocking_handler")));
void pvd_isr(void) __attribute__((weak, alias("blocking_handler")));
void rtc_isr(void) __attribute__((weak, alias("blocking_handler")));
void flash_isr(void) __attribute__((weak, alias("blocking_handler")));
void rcc_isr(void) __attribute__((weak, alias("blocking_handler")));
void exti0_1_isr(void) __attribute__((weak, alias("blocking_handler")));
void exti2_3_isr(void) __attribute__((weak, alias("blocking_handler")));
void exti4_15_isr(void) __attribute__((weak, alias("blocking_handler")));
void ucpd1_ucpd2_isr(void) __attribute__((weak, alias("blocking_handler")));
void dma1_channel1_isr(void) __attribute__((weak, alias("blocking_handler")));
void dma1_channel2_3_isr(void) __attribute__((weak, alias("blocking_handler")));
void dma1_channel4_7_dmamux_isr(void) __attribute__((weak, alias("blocking_handler")));
void adc_comp_isr(void) __attribute__((weak, alias("blocking_handler")));
void tim1_brk_up_trg_com_isr(void) __attribute__((weak, alias("blocking_handler")));
void tim1_cc_isr(void) __attribute__((weak, alias("blocking_handler")));
void tim2_isr(void) __attribute__((weak, alias("blocking_handler")));
void tim3_isr(void) __attribute__((weak, alias("blocking_handler")));
void tim6_dac_lptim1_isr(void) __attribute__((weak, alias("blocking_handler")));
void tim7_lptim2_isr(void) __attribute__((weak, alias("blocking_handler")));
void tim14_isr(void) __attribute__((weak, alias("blocking_handler")));
void tim15_isr(void) __attribute__((weak, alias("blocking_handler")));
void tim16_isr(void) __attribute__((weak, alias("blocking_handler")));
void tim17_isr(void) __attribute__((weak, alias("blocking_handler")));
void i2c1_isr(void) __attribute__((weak, alias("blocking_handler")));
void i2c2_isr(void) __attribute__((weak, alias("blocking_handler")));
void spi1_isr(void) __attribute__((weak, alias("blocking_handler")));
void spi2_isr(void) __attribute__((weak, alias("blocking_handler")));
void usart1_isr(void) __attribute__((weak, alias("blocking_handler")));
void usart2_isr(void) __attribute__((weak, alias("blocking_handler")));
void usart3_usart4_lpuart1_isr(void) __attribute__((weak, alias("blocking_handler")));
void cec_isr(void) __attribute__((weak, alias("blocking_handler")));
void aes_rng_isr(void) __attribute__((weak, alias("blocking_handler")));

/**@}*/

/* Initialization template for the interrupt vector table. This definition is
 * used by the startup code generator (vector.c) to set the initial values for
 * the interrupt handling routines to the chip family specific _isr weak
 * symbols. */

#define IRQ_HANDLERS \
    [NVIC_WWDG_IRQ] = wwdg_isr, \
    [NVIC_PVD_IRQ] = pvd_isr, \
    [NVIC_RTC_IRQ] = rtc_isr, \
    [NVIC_FLASH_IRQ] = flash_isr, \
    [NVIC_RCC_IRQ] = rcc_isr, \
    [NVIC_EXTI0_1_IRQ] = exti0_1_isr, \
    [NVIC_EXTI2_3_IRQ] = exti2_3_isr, \
    [NVIC_EXTI4_15_IRQ] = exti4_15_isr, \
    [NVIC_UCPD1_UCPD2_IRQ] = ucpd1_ucpd2_isr, \
    [NVIC_DMA1_CHANNEL1_IRQ] = dma1_channel1_isr, \
    [NVIC_DMA1_CHANNEL2_3_IRQ] = dma1_channel2_3_isr, \
    [NVIC_DMA1_CHANNEL4_7_DMAMUX_IRQ] = dma1_channel4_7_dmamux_isr, \
    [NVIC_ADC_COMP_IRQ] = adc_comp_isr, \
    [NVIC_TIM1_BRK_UP_TRG_COM_IRQ] = tim1_brk_up_trg_com_isr, \
    [NVIC_TIM1_CC_IRQ] = tim1_cc_isr, \
    [NVIC_TIM2_IRQ] = tim2_isr, \
    [NVIC_TIM3_IRQ] = tim3_isr, \
    [NVIC_TIM6_DAC_LPTIM1_IRQ] = tim6_dac_lptim1_isr, \
    [NVIC_TIM7_LPTIM2_IRQ] = tim7_lptim2_isr, \
    [NVIC_TIM14_IRQ] = tim14_isr, \
    [NVIC_TIM15_IRQ] = tim15_isr, \
    [NVIC_TIM16_IRQ] = tim16_isr, \
    [NVIC_TIM17_IRQ] = tim17_isr, \
    [NVIC_I2C1_IRQ] = i2c1_isr, \
    [NVIC_I2C2_IRQ] = i2c2_isr, \
    [NVIC_SPI1_IRQ] = spi1_isr, \
    [NVIC_SPI2_IRQ] = spi2_isr, \
    [NVIC_USART1_IRQ] = usart1_isr, \
    [NVIC_USART2_IRQ] = usart2_isr, \
    [NVIC_USART3_USART4_LPUART1_IRQ] = usart3_usart4_lpuart1_isr, \
    [NVIC_CEC_IRQ] = cec_isr, \
    [NVIC_AES_RNG_IRQ] = aes_rng_isr
