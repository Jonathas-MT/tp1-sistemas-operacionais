/*
 * v2 - Leitores x Escritores com PREFERENCIA DE ESCRITORES.
 * Leitores so acessam os dados quando nao ha nenhum escritor ativo
 * (nem esperando). Nao ocorre leitura suja.
 *
 * Algoritmo classico "segundo problema dos leitores-escritores"
 * (escritores com prioridade), com 4 semaforos:
 *   - mutex_leitores   : protege a variavel readcount
 *   - mutex_escritores : protege a variavel writecount
 *   - r                : controla a entrada de NOVOS leitores; um
 *                        escritor esperando "tranca" este semaforo
 *   - w                : acesso exclusivo aos dados (leitores em
 *                        grupo x escritor)
 *
 * Tema: Banco. Leitor = consulta de saldo. Escritor = deposito ou
 * saque. Delays (decisao da equipe): leitor 3s, escritor 1s -
 * escritor mais rapido para deixar a preferencia bem visivel nos
 * logs (leitores ficando bloqueados).
 *
 * ATENCAO (ponto a registrar no relatorio): com essa configuracao,
 * leitores podem sofrer starvation se houver escritores suficientes
 * em sequencia, ja que eles tem prioridade total sobre novos leitores.
 *
 * Semaforos NOMEADOS (sem_open/sem_close/sem_unlink) em vez de
 * anonimos (sem_init/sem_destroy): a familia anonima e deprecated no
 * macOS (Darwin nao implementa suporte completo); a nomeada funciona
 * sem warning tanto no macOS quanto no Linux. Mesma solucao aplicada
 * na v1, para manter as duas versoes consistentes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../conta.h"
#include "../../common/log.h"

#define DELAY_LEITOR    3
#define DELAY_ESCRITOR  1
#define SALDO_INICIAL   1000

#define NOME_SEM_MUTEX_LEITORES   "/tp1_rw_v2_mutex_leitores"
#define NOME_SEM_MUTEX_ESCRITORES "/tp1_rw_v2_mutex_escritores"
#define NOME_SEM_R                "/tp1_rw_v2_r"
#define NOME_SEM_W                "/tp1_rw_v2_w"

Conta conta_global;

sem_t *mutex_leitores;
sem_t *mutex_escritores;
sem_t *r;
sem_t *w;

int readcount = 0;
int writecount = 0;

typedef struct {
    int id;
    int valor; /* usado so pelos escritores: positivo = deposito, negativo = saque */
} ArgsThread;

/* Abre (criando do zero) um semaforo nomeado com valor inicial dado.
   Remove qualquer semaforo com esse nome deixado "sujo" por uma
   execucao anterior encerrada abruptamente, antes de criar o novo. */
static sem_t *abrir_semaforo(const char *nome, unsigned int valor_inicial) {
    sem_unlink(nome);
    sem_t *s = sem_open(nome, O_CREAT | O_EXCL, 0644, valor_inicial);
    if (s == SEM_FAILED) {
        perror(nome);
        exit(1);
    }
    return s;
}

void *escritor_v2(void *arg) {
    ArgsThread *a = (ArgsThread *) arg;
    log_evento(a->id, "ESCRITOR", "CRIADA");

    sem_wait(mutex_escritores);
    writecount++;
    if (writecount == 1) {
        log_evento(a->id, "ESCRITOR", "BLOQUEANDO novos leitores (r)");
        sem_wait(r);
    }
    sem_post(mutex_escritores);

    log_evento(a->id, "ESCRITOR", "BLOQUEADA (aguardando acesso exclusivo)");
    sem_wait(w);
    log_evento(a->id, "ESCRITOR", "ENTROU na regiao critica");

    if (a->valor >= 0) {
        conta_depositar(&conta_global, a->valor);
    } else {
        conta_sacar(&conta_global, -a->valor);
    }
    sleep(DELAY_ESCRITOR);

    log_evento_valor(a->id, "ESCRITOR", "SAIU da regiao critica",
                      conta_consultar(&conta_global));
    sem_post(w);

    sem_wait(mutex_escritores);
    writecount--;
    if (writecount == 0) {
        sem_post(r);
        log_evento(a->id, "ESCRITOR", "LIBEROU novos leitores (r)");
    }
    sem_post(mutex_escritores);

    log_evento(a->id, "ESCRITOR", "FINALIZADA");
    free(a);
    return NULL;
}

void *leitor_v2(void *arg) {
    ArgsThread *a = (ArgsThread *) arg;
    log_evento(a->id, "LEITOR", "CRIADA");

    log_evento(a->id, "LEITOR", "BLOQUEADA (aguardando escritor ativo/esperando)");
    sem_wait(r);
    sem_wait(mutex_leitores);
    readcount++;
    if (readcount == 1) {
        sem_wait(w); /* primeiro leitor bloqueia escritores */
    }
    sem_post(mutex_leitores);
    sem_post(r);

    log_evento(a->id, "LEITOR", "ENTROU na regiao critica");
    int saldo = conta_consultar(&conta_global);
    sleep(DELAY_LEITOR);
    log_evento_valor(a->id, "LEITOR", "SAIU da regiao critica", saldo);

    sem_wait(mutex_leitores);
    readcount--;
    if (readcount == 0) {
        sem_post(w); /* ultimo leitor libera escritores */
    }
    sem_post(mutex_leitores);

    log_evento(a->id, "LEITOR", "FINALIZADA");
    free(a);
    return NULL;
}

int main(void) {
    /* ------------------------------------------------------------
     * ENTRADA TEMPORARIA PARA TESTE.
     * O Jonathas vai substituir este bloco pela entrada gerada
     * aleatoriamente (gerador.h/gerador.c). Por enquanto uso valores
     * fixos para eu compilar e testar a v2 isoladamente.
     * ------------------------------------------------------------ */
    int n_leitores = 4;
    int n_escritores = 3;
    int valores_escritores[] = {100, -50, 200};

    conta_criar(&conta_global, SALDO_INICIAL);
    mutex_leitores   = abrir_semaforo(NOME_SEM_MUTEX_LEITORES, 1);
    mutex_escritores = abrir_semaforo(NOME_SEM_MUTEX_ESCRITORES, 1);
    r                = abrir_semaforo(NOME_SEM_R, 1);
    w                = abrir_semaforo(NOME_SEM_W, 1);

    int total = n_leitores + n_escritores;
    pthread_t threads[total];
    int idx = 0;

    for (int i = 0; i < n_escritores; i++) {
        ArgsThread *a = malloc(sizeof(ArgsThread));
        a->id = i + 1;
        a->valor = valores_escritores[i];
        pthread_create(&threads[idx++], NULL, escritor_v2, a);
    }
    for (int i = 0; i < n_leitores; i++) {
        ArgsThread *a = malloc(sizeof(ArgsThread));
        a->id = i + 1;
        pthread_create(&threads[idx++], NULL, leitor_v2, a);
    }

    for (int i = 0; i < total; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_close(mutex_leitores);
    sem_close(mutex_escritores);
    sem_close(r);
    sem_close(w);
    sem_unlink(NOME_SEM_MUTEX_LEITORES);
    sem_unlink(NOME_SEM_MUTEX_ESCRITORES);
    sem_unlink(NOME_SEM_R);
    sem_unlink(NOME_SEM_W);

    printf("\nSaldo final: %d\n", conta_consultar(&conta_global));
    return 0;
}
