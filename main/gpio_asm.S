    .section .text
    .global gpio_config_set_asm
    .type gpio_config_set_asm, @function

/*
 * FUNCAO: gpio_config_set_asm(pin, mode, state)
 *
 * Parametros recebidos do C (ABI Xtensa):
 *   a2 = pin   -> numero do GPIO (ex.: 2, 4, 5...)
 *   a3 = mode  -> 0 = entrada, qualquer valor != 0 = saida
 *   a4 = state -> 0 = nivel baixo (LOW), != 0 = nivel alto (HIGH)
 *
 */
gpio_config_set_asm:
    /* Prologo da funcao: abre frame de pilha de 16 bytes */
    entry   a1, 16

    /* a5 comeca com 1 e vira mascara do pino: a5 = (1 << pin) */
    movi    a5, 1
    ssl     a2               /* configura deslocamento com o valor de pin */
    sll     a5, a5           /* efetiva o shift: 1 << pin */

    /* Se mode == 0, vai para configuracao de entrada */
    beqz    a3, set_input_mode

    /* mode != 0: configura pino como saida */
    movi    a6, 0x3FF44024    /* GPIO_ENABLE_W1TS_REG (seta bit de enable) */
    s32i    a5, a6, 0         /* escreve mascara: habilita saida desse GPIO */
    j       set_level         /* segue para definir nivel logico */

/* mode == 0: configura pino como entrada */
set_input_mode:
    movi    a6, 0x3FF44028    /* GPIO_ENABLE_W1TC_REG (limpa bit de enable) */
    s32i    a5, a6, 0         /* escreve mascara: desabilita saida */

set_level:
    /* Se state == 0, vai para LOW */
    beqz    a4, set_low

    /* state != 0: coloca pino em HIGH */
    movi    a6, 0x3FF44008    /* GPIO_OUT_W1TS_REG (seta bit de saida) */
    s32i    a5, a6, 0         /* escreve mascara: nivel alto no pino */
    j       done

/* state == 0: coloca pino em LOW */
set_low:
    movi    a6, 0x3FF4400C    /* GPIO_OUT_W1TC_REG (limpa bit de saida) */
    s32i    a5, a6, 0         /* escreve mascara: nivel baixo no pino */

done:
    /* Epilogo/retorno da funcao (fecha janela de registradores) */
    retw
