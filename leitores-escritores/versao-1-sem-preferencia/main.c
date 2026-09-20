/*
 * v1 - Leitores x Escritores SEM preferencia de ordem de acesso.
 * Pode ocorrer "leitura suja".
 *
 * Interpretacao adotada pela equipe (ver docs/definicoes_equipe):
 *   - Escritores se sincronizam ENTRE SI (um mutex de escrita), para
 *     nao haver dois escritores corrompendo o saldo ao mesmo tempo.
 *   - Leitores NAO esperam em nenhum semaforo: leem o saldo livremente,
 *     mesmo que um escritor esteja no meio de uma atualizacao.
 *   - E exatamente essa ausencia de barreira leitor-escritor que
 *     permite a leitura suja pedida pelo enunciado.
 *
 * Tema: Banco. Leitor = consulta de saldo. Escritor = deposito ou saque.
 * Delays (decisao da equipe): leitor 3s, escritor 3s (nenhum tem
 * preferencia por velocidade tambem).
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "conta.h"
#include "../common/log.h"

#define DELAY_LEITOR    3
#define DELAY_ESCRITOR  3
#define SALDO_INICIAL   1000

Conta conta_global;
sem_t mutex_escrita; /* protege escritores entre si */

typedef struct {
    int id;
    int valor; /* usado so pelos escritores: positivo = deposito, negativo = saque */
} ArgsThread;

void *escritor_v1(void *arg) {
    ArgsThread *a = (ArgsThread *) arg;
    log_evento(a->id, "ESCRITOR", "CRIADA");

    log_evento(a->id, "ESCRITOR", "BLOQUEADA (aguardando outro escritor)");
    sem_wait(&mutex_escrita);
    log_evento(a->id, "ESCRITOR", "ENTROU na regiao critica");

    if (a->valor >= 0) {
        conta_depositar(&conta_global, a->valor);
    } else {
        conta_sacar(&conta_global, -a->valor);
    }
    sleep(DELAY_ESCRITOR); /* simula o tempo da operacao; janela onde
                               pode ocorrer leitura suja */

    log_evento_valor(a->id, "ESCRITOR", "SAIU da regiao critica",
                      conta_consultar(&conta_global));
    sem_post(&mutex_escrita);

    log_evento(a->id, "ESCRITOR", "FINALIZADA");
    free(a);
    return NULL;
}

void *leitor_v1(void *arg) {
    ArgsThread *a = (ArgsThread *) arg;
    log_evento(a->id, "LEITOR", "CRIADA");

    /* Sem sem_wait aqui de proposito: o leitor entra mesmo que um
       escritor esteja ativo, o que pode gerar leitura suja. */
    log_evento(a->id, "LEITOR", "ENTROU na regiao critica");
    int saldo = conta_consultar(&conta_global);
    sleep(DELAY_LEITOR);

    log_evento_valor(a->id, "LEITOR", "SAIU da regiao critica", saldo);
    log_evento(a->id, "LEITOR", "FINALIZADA");
    free(a);
    return NULL;
}

int main(void) {
    /* ------------------------------------------------------------
     * ENTRADA TEMPORARIA PARA TESTE.
     * O Jonathas vai substituir este bloco pela entrada gerada
     * aleatoriamente (gerador.h/gerador.c): quantidade de leitores,
     * quantidade de escritores e valores dos escritores. Por
     * enquanto uso valores fixos para eu compilar e testar a v1
     * isoladamente.
     * ------------------------------------------------------------ */
    int n_leitores = 4;
    int n_escritores = 3;
    int valores_escritores[] = {100, -50, 200};

    conta_criar(&conta_global, SALDO_INICIAL);
    sem_init(&mutex_escrita, 0, 1);

    int total = n_leitores + n_escritores;
    pthread_t threads[total];
    int idx = 0;

    for (int i = 0; i < n_escritores; i++) {
        ArgsThread *a = malloc(sizeof(ArgsThread));
        a->id = i + 1;
        a->valor = valores_escritores[i];
        pthread_create(&threads[idx++], NULL, escritor_v1, a);
    }
    for (int i = 0; i < n_leitores; i++) {
        ArgsThread *a = malloc(sizeof(ArgsThread));
        a->id = i + 1;
        pthread_create(&threads[idx++], NULL, leitor_v1, a);
    }

    for (int i = 0; i < total; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&mutex_escrita);
    printf("\nSaldo final: %d\n", conta_consultar(&conta_global));
    return 0;
}
