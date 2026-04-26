    .section .text
    .global delay_ms_asm
    .type delay_ms_asm, @function

/*
 * FUNCAO: delay_ms_asm(ms)
 *
 * Parametro:
 *   a2 = quantidade de milissegundos (ms)
 *
 */
delay_ms_asm:
    /* Prologo da funcao: abre frame de pilha */
    entry   a1, 16

    /* Se ms == 0, nao ha nada para esperar */
    beqz    a2, done

/* Loop externo: repete uma vez por "1 ms aproximado" */
outer_loop:
    /* Contador interno que consome ciclos de CPU */
    movi    a3, 80000

/* Loop interno: decrementa ate zero */
inner_loop:
    addi    a3, a3, -1       /* a3-- */
    bnez    a3, inner_loop   /* enquanto a3 != 0, continua */

    addi    a2, a2, -1       /* ms restante-- */
    bnez    a2, outer_loop   /* enquanto ms != 0, repete */

done:
    /* Retorno da funcao */
    retw
