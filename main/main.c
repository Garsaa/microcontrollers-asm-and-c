#include <stdint.h>

/* Funcao Assembly que configura direcao e nivel de um GPIO */
void gpio_config_set_asm(uint32_t pin, uint32_t mode, uint32_t state);

/* Funcao Assembly de atraso por busy-wait (ms aproximado) */
void delay_ms_asm(uint32_t ms);

void app_main(void)
{
    while (1) {
        /* 1) Liga o LED azul (GPIO2 em modo saida, estado HIGH) */
        gpio_config_set_asm(25, 1, 1);

        while(1){
            delay_ms_asm(2000);
        }
        /* 2) Espera ~2 segundos */
    
        /* 3) Desliga o LED azul (GPIO2 em modo saida, estado LOW) */
        // gpio_config_set_asm(25, 1, 0);

        /* 4) Espera ~5 segundos */
        // delay_ms_asm(5000);
    
        /* 5) Liga novamente */
        // gpio_config_set_asm(25, 1, 1);
    }
}
