#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BUTTON_COUNT 3
#define DEBOUNCE_MS 200
#define BIT_U32(index) (1UL << (index))

/*
 * Equivalente, no ESP32, ao exemplo com PCINT1/PORTC do ATmega328P.
 *
 * No ATmega, varios pinos do mesmo grupo PCINT compartilham o mesmo vetor de
 * interrupcao, entao a ISR precisa testar qual pino gerou o pedido. Aqui no
 * ESP-IDF cada GPIO registra o mesmo handler e passa seu indice por argumento,
 * mantendo a mesma ideia: uma ISR compartilhada atende varios botoes.
 */
static const gpio_num_t button_gpios[BUTTON_COUNT] = {
    GPIO_NUM_26,
    GPIO_NUM_27,
    GPIO_NUM_14,
};

static const gpio_num_t led_gpios[BUTTON_COUNT] = {
    GPIO_NUM_25,
    GPIO_NUM_32,
    GPIO_NUM_33,
};

static bool led_on[BUTTON_COUNT];
static volatile uint32_t changed_buttons;
static volatile uint32_t pressed_buttons;

static uint64_t build_pin_mask(const gpio_num_t *pins)
{
    uint64_t mask = 0;

    for (uint32_t i = 0; i < BUTTON_COUNT; i++) {
        mask |= 1ULL << pins[i];
    }

    return mask;
}

static void IRAM_ATTR portc_change_isr(void *arg)
{
    uint32_t index = (uint32_t)arg;
    uint32_t bit = BIT_U32(index);

    /*
     * A interrupcao e por mudanca de estado, entao apertar e soltar o botao
     * geram pedidos. Para alternar o LED apenas no aperto, a ISR guarda se o
     * pino estava em nivel 0 no momento do evento. O botao usa pull-up interno:
     * solto = 1, pressionado = 0.
     */
    changed_buttons |= bit;
    if (gpio_get_level(button_gpios[index]) == 0) {
        pressed_buttons |= bit;
    } else {
        pressed_buttons &= ~bit;
    }

    /*
     * Desabilita temporariamente este GPIO para reduzir o efeito do ruido do
     * contato. O atraso de 200 ms e feito na task principal, porque no ESP-IDF
     * uma ISR deve ser curta e nao deve chamar vTaskDelay().
     */
    gpio_intr_disable(button_gpios[index]);
}

void app_main(void)
{
    gpio_config_t led_config = {
        .pin_bit_mask = build_pin_mask(led_gpios),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&led_config);

    for (uint32_t i = 0; i < BUTTON_COUNT; i++) {
        gpio_set_level(led_gpios[i], 0);
    }

    gpio_config_t button_config = {
        .pin_bit_mask = build_pin_mask(button_gpios),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    gpio_config(&button_config);

    gpio_install_isr_service(0);
    for (uint32_t i = 0; i < BUTTON_COUNT; i++) {
        gpio_isr_handler_add(button_gpios[i], portc_change_isr, (void *)i);
    }

    while (1) {
        uint32_t pending = changed_buttons;

        for (uint32_t i = 0; i < BUTTON_COUNT; i++) {
            uint32_t bit = BIT_U32(i);

            if ((pending & bit) == 0) {
                continue;
            }

            if ((pressed_buttons & bit) != 0) {
                led_on[i] = !led_on[i];
                gpio_set_level(led_gpios[i], led_on[i]);
            }

            /*
             * Debounce: espera o contato estabilizar antes de aceitar outra
             * mudanca neste botao. Sem isso, o ruido mecanico pode gerar varias
             * interrupcoes e deixar o estado final do LED imprevisivel.
             */
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));

            changed_buttons &= ~bit;
            pressed_buttons &= ~bit;
            gpio_intr_enable(button_gpios[i]);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
