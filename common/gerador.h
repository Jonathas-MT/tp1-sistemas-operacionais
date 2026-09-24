#ifndef GERADOR_H
#define GERADOR_H

/*
 * Gerador de entrada ALEATORIA (numerica) para os programas.
 * Responsavel: Jonathas
 *
 * Sorteia, a cada execucao: quantidades de threads e os valores
 * numericos que serao usados (saldo inicial e valores dos escritores).
 *
 * As faixas sorteadas ficam nos #define abaixo; para testar outra
 * configuracao, basta mudar aqui. Valores sao multiplos de GER_PASSO
 * so para o log ficar legivel (ex.: +120, -80).
 *
 * Cuidado ao editar as faixas: com SALDO_MIN = 2000, ate 5 escritores
 * sacando no maximo 300 cada (-1500), o saldo nunca fica negativo.
 */

/* ---- Leitores x Escritores ---- */
#define GER_LEITORES_MIN     3
#define GER_LEITORES_MAX     6
#define GER_ESCRITORES_MIN   2
#define GER_ESCRITORES_MAX   5
#define GER_SALDO_MIN        2000
#define GER_SALDO_MAX        5000
#define GER_DEPOSITO_MAX     500   /* valor de deposito sorteado: 1..MAX  */
#define GER_SAQUE_MAX        300   /* valor de saque sorteado: -1..-MAX   */

/* ---- Produtores x Consumidores ---- */
#define GER_PRODUTORES_MIN   2
#define GER_PRODUTORES_MAX   4
#define GER_ITENS_MIN        3     /* itens por produtor */
#define GER_ITENS_MAX        5
#define GER_CONSUMIDORES_MIN 2
#define GER_CONSUMIDORES_MAX 3

#define GER_PASSO            10

typedef struct {
    int  n_leitores;
    int  n_escritores;
    int  saldo_inicial;
    int *valores_escritores;   /* vetor com n_escritores posicoes;
                                  positivo = deposito, negativo = saque */
} ConfigLE;

typedef struct {
    int n_produtores;
    int itens_por_produtor;
    int n_consumidores;
} ConfigPC;

/* Inicializa o sorteio. Se semente == 0, usa uma semente baseada no
   horario e no PID. Devolve a semente usada (imprima-a no log: passando
   a mesma semente de volta, a mesma entrada e gerada de novo). */
unsigned int gerador_iniciar(unsigned int semente);

ConfigLE gerador_leitores_escritores(void);
void     gerador_liberar_le(ConfigLE *cfg);   /* libera valores_escritores */

ConfigPC gerador_produtores_consumidores(void);

#endif