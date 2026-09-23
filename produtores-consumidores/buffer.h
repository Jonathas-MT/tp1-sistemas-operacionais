#ifndef BUFFER_H
#define BUFFER_H

/*
 * TAD Buffer — Produtores x Consumidores
 *
 * Guarda apenas o dado (o vetor circular) e as operacoes basicas,
 * SEM nenhuma sincronizacao — igual ao TAD Conta dos Leitores x
 * Escritores. A sincronizacao (semaforos + mutex) fica nos main.c
 * de cada versao, em volta das chamadas a este TAD.
 *
 * CAPACIDADE_BUFFER fica em cada main.c (cada versao pode testar um
 * tamanho de buffer diferente), por isso buffer_criar recebe a
 * capacidade como parametro.
 */

typedef struct {
    int *dados;      /* vetor circular com as posicoes do buffer      */
    int capacidade;  /* quantas posicoes existem                      */
    int entrada;     /* indice onde o PROXIMO dado sera colocado      */
    int saida;       /* indice de onde o PROXIMO dado sera retirado   */
    int quantidade;  /* quantos dados estao no buffer agora           */
} Buffer;

Buffer *buffer_criar(int capacidade);
void    buffer_destruir(Buffer *b);
void    buffer_inserir(Buffer *b, int valor);
int     buffer_retirar(Buffer *b);
int     buffer_cheio(const Buffer *b);
int     buffer_vazio(const Buffer *b);
void    buffer_exibir(const Buffer *b);   /* ex.: [ 12 | 7 | _ | _ | _ ] */

#endif
