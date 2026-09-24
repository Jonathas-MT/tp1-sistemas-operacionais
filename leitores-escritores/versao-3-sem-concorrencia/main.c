/*
 * v3 - Leitores x Escritores SEM CONTROLE DE CONCORRENCIA.
 *
 * Objetivo: mostrar o problema do acesso simultaneo ao dado
 * compartilhado. Aqui NAO existe semaforo, mutex ou qualquer outra
 * forma de espera: nenhuma thread espera por outra.
 *
 * Por que o problema aparece:
 *   "saldo += valor" parece uma operacao unica, mas na pratica sao tres
 *   passos: (1) LER o saldo, (2) CALCULAR o novo saldo, (3) GRAVAR.
 *   Se dois escritores executam isso ao mesmo tempo, ambos leem o MESMO
 *   saldo antigo e o ultimo a gravar apaga a atualizacao do outro
 *   ("atualizacao perdida" / lost update).
 *
 *   Numa maquina real essa janela entre a leitura e a gravacao e de
 *   nanossegundos, entao o erro apareceria so de vez em quando. Para o
 *   problema ficar visivel em TODA execucao, o escritor faz os tres
 *   passos separados e coloca o sleep(DELAY_ESCRITOR) entre a leitura e a
 *   gravacao, simulando o tempo de processamento da operacao. Essa
 *   e a mesma ideia do sleep dentro da regiao critica nas v1 e v2.
 *
 * Como o TAD Conta so oferece depositar/sacar (que fazem os tres passos
 * de uma vez), a gravacao e feita direto no campo conta_global.saldo.
 * A struct Conta e publica no conta.h, entao isso compila normalmente.
 *
 * Instrumentacao (so para mostrar o erro no log, nao corrige nada):
 *   - antes de gravar, o escritor confere se o saldo mudou desde que ele
 *     leu; se mudou, avisa que vai sobrescrever a atualizacao de outro.
 *   - o leitor avisa quando le enquanto ha escritor ativo.
 *   - no final, o programa compara o saldo esperado com o obtido.
 *
 * Tema: Banco. Leitor = consulta de saldo. Escritor = deposito/saque.
 * Delays: leitor 3s e escritor 3s (ajuste os #define abaixo para testar
 * qualquer configuracao de atraso para cada tipo de thread).
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include "../conta.h"
#include "../../common/log.h"
#include "../../common/gerador.h"

#define DELAY_LEITOR          3
#define DELAY_ESCRITOR        3     /* tempo entre LER e GRAVAR o saldo */
#define INTERVALO_CRIACAO_MS  500   /* espaco entre a criacao de cada thread */

Conta conta_global;

/* Quantos escritores estao "no meio" de uma atualizacao. Serve apenas
   para o leitor avisar no log; usa operacao atomica para que o proprio
   contador nao seja fonte de erro. */
static int escritores_ativos = 0;

typedef struct {
    int id;
    int valor; /* usado so pelos escritores: positivo = deposito, negativo = saque */
} ArgsThread;

static void pausa_ms(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long) (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

void *escritor_v3(void *arg) {
    ArgsThread *a = (ArgsThread *) arg;
    char msg[192];

    log_evento(a->id, "ESCRITOR", "CRIADA");
    log_evento(a->id, "ESCRITOR", "ENTROU na regiao critica (SEM protecao)");
    __atomic_fetch_add(&escritores_ativos, 1, __ATOMIC_SEQ_CST);

    /* passo 1: LER */
    int lido = conta_consultar(&conta_global);
    snprintf(msg, sizeof(msg), "LEU saldo = %d (vai aplicar %+d)", lido, a->valor);
    log_evento(a->id, "ESCRITOR", msg);

    /* passo 2: CALCULAR (com o valor lido, que pode ficar desatualizado) */
    int novo = lido + a->valor;
    sleep(DELAY_ESCRITOR); /* janela onde outros escritores/leitores acessam */

    /* verificacao so para o log: alguem mexeu no saldo enquanto eu calculava? */
    int atual = conta_consultar(&conta_global);
    if (atual != lido) {
        snprintf(msg, sizeof(msg),
                 "PROBLEMA: saldo mudou de %d para %d durante minha operacao; "
                 "vou SOBRESCREVER a atualizacao de outro escritor", lido, atual);
        log_evento(a->id, "ESCRITOR", msg);
    }

    /* passo 3: GRAVAR */
    conta_global.saldo = novo;
    snprintf(msg, sizeof(msg), "GRAVOU saldo = %d (calculado a partir do saldo %d lido antes)",
             novo, lido);
    log_evento(a->id, "ESCRITOR", msg);

    __atomic_fetch_sub(&escritores_ativos, 1, __ATOMIC_SEQ_CST);
    log_evento_valor(a->id, "ESCRITOR", "SAIU da regiao critica",
                      conta_consultar(&conta_global));
    log_evento(a->id, "ESCRITOR", "FINALIZADA");
    free(a);
    return NULL;
}

void *leitor_v3(void *arg) {
    ArgsThread *a = (ArgsThread *) arg;
    char msg[128];

    log_evento(a->id, "LEITOR", "CRIADA");
    log_evento(a->id, "LEITOR", "ENTROU na regiao critica (SEM protecao)");

    int saldo = conta_consultar(&conta_global);
    int ativos = __atomic_load_n(&escritores_ativos, __ATOMIC_SEQ_CST);
    log_evento_valor(a->id, "LEITOR", "LEU o saldo", saldo);
    if (ativos > 0) {
        snprintf(msg, sizeof(msg),
                 "ATENCAO: leu com %d escritor(es) ativo(s); o dado pode estar "
                 "desatualizado (leitura suja)", ativos);
        log_evento(a->id, "LEITOR", msg);
    }
    sleep(DELAY_LEITOR);

    log_evento_valor(a->id, "LEITOR", "SAIU da regiao critica", saldo);
    log_evento(a->id, "LEITOR", "FINALIZADA");
    free(a);
    return NULL;
}

int main(int argc, char **argv) {
    /* Entrada gerada aleatoriamente (common/gerador.c). Para repetir uma
       execucao, passe a semente impressa: ./le_v3 <semente> */
    unsigned int semente = gerador_iniciar(argc > 1 ? (unsigned int) strtoul(argv[1], NULL, 10) : 0);
    ConfigLE cfg = gerador_leitores_escritores();

    int n_leitores = cfg.n_leitores;
    int n_escritores = cfg.n_escritores;
    int *valores_escritores = cfg.valores_escritores;

    conta_criar(&conta_global, cfg.saldo_inicial);

    int esperado = cfg.saldo_inicial;
    for (int i = 0; i < n_escritores; i++) {
        esperado += valores_escritores[i];
    }

    printf("Semente: %u (passe-a como argumento para repetir esta execucao)\n", semente);
    printf("Leitores: %d | Escritores: %d | Valores dos escritores:", n_leitores, n_escritores);
    for (int i = 0; i < n_escritores; i++) {
        printf(" %+d", valores_escritores[i]);
    }
    printf("\n");
    printf("Saldo inicial: %d | Saldo esperado se nao houvesse problema: %d\n\n",
           cfg.saldo_inicial, esperado);

    int total = n_leitores + n_escritores;
    pthread_t threads[total];
    int idx = 0;

    /* Cria escritores e leitores alternados, com um pequeno intervalo,
       para que leitores cheguem enquanto escritores estao no meio da
       operacao. */
    int maximo = n_leitores > n_escritores ? n_leitores : n_escritores;
    for (int i = 0; i < maximo; i++) {
        if (i < n_escritores) {
            ArgsThread *a = malloc(sizeof(ArgsThread));
            a->id = i + 1;
            a->valor = valores_escritores[i];
            pthread_create(&threads[idx++], NULL, escritor_v3, a);
            pausa_ms(INTERVALO_CRIACAO_MS);
        }
        if (i < n_leitores) {
            ArgsThread *a = malloc(sizeof(ArgsThread));
            a->id = i + 1;
            a->valor = 0;
            pthread_create(&threads[idx++], NULL, leitor_v3, a);
            pausa_ms(INTERVALO_CRIACAO_MS);
        }
    }

    for (int i = 0; i < total; i++) {
        pthread_join(threads[i], NULL);
    }

    int obtido = conta_consultar(&conta_global);
    printf("\n===== RESULTADO =====\n");
    printf("Saldo esperado: %d\n", esperado);
    printf("Saldo obtido:   %d\n", obtido);
    if (obtido != esperado) {
        printf("INCONSISTENTE: o saldo final difere do esperado em %+d, porque atualizacoes foram\n"
               "perdidas/sobrescritas por falta de controle de concorrencia.\n",
               obtido - esperado);
    } else {
        printf("Desta vez o saldo bateu por coincidencia (execute de novo ou aumente DELAY_ESCRITOR).\n");
    }
    gerador_liberar_le(&cfg);
    return 0;
}