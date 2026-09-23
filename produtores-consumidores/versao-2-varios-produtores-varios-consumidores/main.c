/*
 * v2 - Produtores x Consumidores: VARIOS produtores e VARIOS consumidores.
 *
 * Mesma logica de sincronizacao da v1 (sem_t vazio / sem_t cheio /
 * mutex). A diferenca esta em ter N consumidores em vez de 1.
 *
 * Sinal de encerramento: depois que todos os produtores terminam, a
 * main insere um VALOR_SENTINELA no buffer para CADA consumidor (um
 * "bilhete de saida" por consumidor). Assim cada consumidor recebe
 * exatamente um sinal e nenhum fica esperando para sempre.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "../buffer.h"
#include "../../common/log.h"

#define CAPACIDADE_BUFFER      5
#define DELAY_PRODUTOR         1
#define DELAY_CONSUMIDOR       2
#define VALOR_SENTINELA        -1

Buffer *buffer_global;
sem_t vazio;
sem_t cheio;
pthread_mutex_t mutex;

typedef struct {
    int id;
    int quantidade_itens;
} ArgsProdutor;

void *tarefa_produtor(void *arg) {
    ArgsProdutor *a = (ArgsProdutor *) arg;
    log_evento(a->id, "PRODUTOR", "CRIADA");

    for (int i = 1; i <= a->quantidade_itens; i++) {
        int valor = a->id * 100 + i;
        char msg[64];

        if (sem_trywait(&vazio) != 0) {
            log_evento(a->id, "PRODUTOR", "BLOQUEADA (buffer cheio, aguardando vaga)");
            sem_wait(&vazio);
        }

        pthread_mutex_lock(&mutex);
        log_evento(a->id, "PRODUTOR", "ENTROU na regiao critica");

        buffer_inserir(buffer_global, valor);
        snprintf(msg, sizeof(msg), "inseriu o valor %d no buffer", valor);
        log_evento(a->id, "PRODUTOR", msg);
        buffer_exibir(buffer_global);

        log_evento(a->id, "PRODUTOR", "SAIU da regiao critica");
        pthread_mutex_unlock(&mutex);
        sem_post(&cheio);

        sleep(DELAY_PRODUTOR);
    }

    log_evento(a->id, "PRODUTOR", "FINALIZADA");
    free(a);
    return NULL;
}

void *tarefa_consumidor(void *arg) {
    int id = *(int *) arg;
    log_evento(id, "CONSUMIDOR", "CRIADA");

    while (1) {
        if (sem_trywait(&cheio) != 0) {
            log_evento(id, "CONSUMIDOR", "BLOQUEADA (buffer vazio, aguardando item)");
            sem_wait(&cheio);
        }

        pthread_mutex_lock(&mutex);
        log_evento(id, "CONSUMIDOR", "ENTROU na regiao critica");

        int valor = buffer_retirar(buffer_global);

        if (valor == VALOR_SENTINELA) {
            log_evento(id, "CONSUMIDOR", "recebeu sinal de encerramento");
            buffer_exibir(buffer_global);
            log_evento(id, "CONSUMIDOR", "SAIU da regiao critica");
            pthread_mutex_unlock(&mutex);
            sem_post(&vazio);
            break;
        }

        char msg[64];
        snprintf(msg, sizeof(msg), "retirou o valor %d do buffer", valor);
        log_evento(id, "CONSUMIDOR", msg);
        buffer_exibir(buffer_global);

        log_evento(id, "CONSUMIDOR", "SAIU da regiao critica");
        pthread_mutex_unlock(&mutex);
        sem_post(&vazio);

        sleep(DELAY_CONSUMIDOR);
    }

    log_evento(id, "CONSUMIDOR", "FINALIZADA");
    free(arg);
    return NULL;
}

int main(void) {
    /* ENTRADA TEMPORARIA PARA TESTE (ver comentario na v1). */
    int n_produtores = 3;
    int itens_por_produtor = 4;  /* total de itens = 3 x 4 = 12 */
    int n_consumidores = 2;

    buffer_global = buffer_criar(CAPACIDADE_BUFFER);
    sem_init(&vazio, 0, CAPACIDADE_BUFFER);
    sem_init(&cheio, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    pthread_t threads_produtores[n_produtores];
    pthread_t threads_consumidores[n_consumidores];

    for (int i = 0; i < n_produtores; i++) {
        ArgsProdutor *a = malloc(sizeof(ArgsProdutor));
        a->id = i + 1;
        a->quantidade_itens = itens_por_produtor;
        pthread_create(&threads_produtores[i], NULL, tarefa_produtor, a);
    }
    for (int i = 0; i < n_consumidores; i++) {
        int *id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&threads_consumidores[i], NULL, tarefa_consumidor, id);
    }

    for (int i = 0; i < n_produtores; i++) {
        pthread_join(threads_produtores[i], NULL);
    }

    /* um sinal de encerramento POR CONSUMIDOR */
    for (int i = 0; i < n_consumidores; i++) {
        sem_wait(&vazio);
        pthread_mutex_lock(&mutex);
        buffer_inserir(buffer_global, VALOR_SENTINELA);
        pthread_mutex_unlock(&mutex);
        sem_post(&cheio);
    }
    log_evento(0, "MAIN", "todos os produtores terminaram; sinais de encerramento enviados");

    for (int i = 0; i < n_consumidores; i++) {
        pthread_join(threads_consumidores[i], NULL);
    }

    printf("\nBuffer final: ");
    buffer_exibir(buffer_global);

    sem_destroy(&vazio);
    sem_destroy(&cheio);
    pthread_mutex_destroy(&mutex);
    buffer_destruir(buffer_global);
    return 0;
}
