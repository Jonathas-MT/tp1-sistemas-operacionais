#ifndef CONTA_H
#define CONTA_H

/*
 * TAD Conta
 * Responsavel: Beatriz
 *
 * Guarda apenas o dado (saldo) e as operacoes basicas, SEM nenhuma
 * sincronizacao. A sincronizacao (semaforos) fica nos programas
 * (rw_v1_sem_preferencia.c, rw_v2_preferencia_escritores.c), em volta
 * das chamadas a este TAD - conforme decidido pela equipe.
 */

typedef struct {
    int saldo;
} Conta;

void conta_criar(Conta *c, int saldo_inicial);
int  conta_consultar(Conta *c);
void conta_depositar(Conta *c, int valor);
void conta_sacar(Conta *c, int valor);

#endif
