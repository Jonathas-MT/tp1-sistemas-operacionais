/*
 * v3 - Produtores x Consumidores SEM CONTROLE DE CONCORRENCIA.
 * VARIOS produtores e VARIOS consumidores.
 *
 * Objetivo: mostrar onde e quando o buffer se corrompe quando nao ha
 * semaforo nem mutex. Aqui nenhuma thread espera por outra:
 *   - o produtor NAO espera vaga (nao ha semaforo "vazio");
 *   - o consumidor NAO espera item (nao ha semaforo "cheio");
 *   - nao ha exclusao mutua sobre o buffer (nao ha mutex).
 *
 * Problemas que podem aparecer no log (quais e quantos varia a cada
 * execucao, ja que depende do escalonamento das threads; a corrida nos
 * indices e o underflow aparecem sempre, o overflow e o consumo em
 * duplicidade dependem de os produtores ficarem a frente dos
 * consumidores ou de dois consumidores lerem o mesmo indice):
 *   1) CORRIDA nos indices: dois produtores leem o mesmo "entrada" e
 *      gravam na MESMA posicao; um valor apaga o outro. Idem para dois
 *      consumidores lendo o mesmo "saida" (o mesmo valor e consumido
 *      duas vezes). Tambem se perde incremento de "quantidade".
 *   2) OVERFLOW: com o buffer cheio, o produtor continua e SOBRESCREVE
 *      dados que ninguem consumiu ainda.
 *   3) UNDERFLOW: com o buffer vazio, o consumidor continua e le uma
 *      posicao vazia (lixo), avancando "saida" e deixando "quantidade"
 *      negativa.
 *
 * Para que a corrida aconteca em TODA execucao (e nao so de vez em
 * quando), inserir/retirar sao feitos em passos separados, com uma pausa
 * curta (JANELA_CORRIDA_MS) entre "ler o indice" e "usar o indice".
 * Em hardware real essa janela existe, so que e minuscula. A logica e
 * escrita direto sobre os campos do Buffer (a struct e publica no
 * buffer.h) porque buffer_inserir/buffer_retirar do TAD fazem tudo de uma
 * vez e ainda barram overflow/underflow, o que esconderia o problema.
 * Por isso tambem nao uso buffer_exibir: ele so mostra as posicoes
 * "validas" segundo "quantidade", que e justamente o campo corrompido.
 * exibir_bruto() mostra o conteudo REAL do vetor.
 *
 * Encerramento: sem semaforos nao da para usar sentinela com seguranca,
 * entao cada produtor produz uma quantidade fixa e cada consumidor faz
 * uma quantidade fixa de retiradas (total de itens dividido entre os
 * consumidores). O programa sempre termina, mesmo que o buffer esteja
 * corrompido.
 *
 * Contabilidade: um vetor "consumos" registra quantas vezes cada valor
 * foi consumido, e no final o programa lista o que foi perdido e o que foi
 * consumido em duplicidade. Os contadores usam operacoes atomicas
 * para que a propria contabilidade nao seja fonte de erro.
 *
 * Delays (mesmos da v1/v2): produtor 1s, consumidor 2s.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include "../buffer.h"
#include "../../common/log.h"
#include "../../common/gerador.h"

#define CAPACIDADE_BUFFER    5
#define DELAY_PRODUTOR       1     /* segundos entre cada item produzido */
#define DELAY_CONSUMIDOR     2     /* segundos entre cada item consumido */
#define JANELA_CORRIDA_MS    300   /* pausa entre ler o indice e usa-lo */
#define SLOT_LIVRE           -1    /* marca de posicao vazia no vetor */
#define MAX_VALOR            10000 /* valores sao id*100+i; ver consumos[] */

Buffer *buffer_global;

static int consumos[MAX_VALOR];    /* quantas vezes cada valor foi consumido */
static int sobrescritas = 0;       /* produtor gravou em posicao ocupada     */
static int leituras_vazias = 0;    /* consumidor leu posicao vazia           */
static int consumos_duplicados = 0;/* valor consumido mais de uma vez        */

typedef struct {
    int id;
    int quantidade_itens;
} ArgsProdutor;

typedef struct {
    int id;
    int quantidade_retiradas;
} ArgsConsumidor;

static void pausa_ms(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long) (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

/* Mostra o conteudo REAL do vetor, sem confiar em "quantidade".
   Monta a linha inteira antes e imprime numa unica chamada, para outra
   thread nao intercalar o log no meio da linha. */
static void exibir_bruto(const Buffer *b) {
    char linha[256];
    int n = 0;
    n += snprintf(linha + n, sizeof(linha) - n, "        buffer: [ ");
    for (int i = 0; i < b->capacidade && n < (int) sizeof(linha) - 64; i++) {
        if (b->dados[i] == SLOT_LIVRE)
            n += snprintf(linha + n, sizeof(linha) - n, "_");
        else
            n += snprintf(linha + n, sizeof(linha) - n, "%d", b->dados[i]);
        if (i < b->capacidade - 1)
            n += snprintf(linha + n, sizeof(linha) - n, " | ");
    }
    snprintf(linha + n, sizeof(linha) - n, " ]  (entrada=%d saida=%d quantidade=%d)\n",
             b->entrada, b->saida, b->quantidade);
    fputs(linha, stdout);
    fflush(stdout);
}

/* Insercao SEM protecao: le indice -> pausa -> grava -> atualiza indices. */
static void inserir_sem_controle(Buffer *b, int id, int valor) {
    char msg[192];

    int pos = b->entrada;        /* le o indice */
    int q = b->quantidade;       /* le a quantidade */
    pausa_ms(JANELA_CORRIDA_MS); /* outra thread pode agir aqui */

    if (b->dados[pos] != SLOT_LIVRE) {
        snprintf(msg, sizeof(msg),
                 "PROBLEMA: posicao %d ainda guarda o valor %d (nao consumido); "
                 "SOBRESCREVENDO com %d - dado PERDIDO", pos, b->dados[pos], valor);
        log_evento(id, "PRODUTOR", msg);
        __atomic_fetch_add(&sobrescritas, 1, __ATOMIC_SEQ_CST);
    }

    b->dados[pos] = valor;
    b->entrada = (pos + 1) % b->capacidade;
    b->quantidade = q + 1;       /* usa a quantidade lida ANTES da pausa */
}

/* Retirada SEM protecao. Devolve o valor lido (SLOT_LIVRE se era lixo). */
static int retirar_sem_controle(Buffer *b, int id) {
    char msg[192];

    int pos = b->saida;          /* le o indice */
    int q = b->quantidade;       /* le a quantidade */
    pausa_ms(JANELA_CORRIDA_MS); /* outra thread pode agir aqui */

    int valor = b->dados[pos];
    if (valor == SLOT_LIVRE) {
        snprintf(msg, sizeof(msg),
                 "PROBLEMA: posicao %d esta VAZIA; consumindo mesmo assim (buffer vazio)", pos);
        log_evento(id, "CONSUMIDOR", msg);
        __atomic_fetch_add(&leituras_vazias, 1, __ATOMIC_SEQ_CST);
    } else if (valor >= 0 && valor < MAX_VALOR) {
        int antes = __atomic_fetch_add(&consumos[valor], 1, __ATOMIC_SEQ_CST);
        if (antes > 0) {
            snprintf(msg, sizeof(msg),
                     "PROBLEMA: o valor %d JA foi consumido por outro consumidor "
                     "(consumo em duplicidade)", valor);
            log_evento(id, "CONSUMIDOR", msg);
            __atomic_fetch_add(&consumos_duplicados, 1, __ATOMIC_SEQ_CST);
        }
    }

    b->dados[pos] = SLOT_LIVRE;
    b->saida = (pos + 1) % b->capacidade;
    b->quantidade = q - 1;       /* usa a quantidade lida ANTES da pausa */
    return valor;
}

void *tarefa_produtor(void *arg) {
    ArgsProdutor *a = (ArgsProdutor *) arg;
    char msg[96];
    log_evento(a->id, "PRODUTOR", "CRIADA");

    for (int i = 1; i <= a->quantidade_itens; i++) {
        int valor = a->id * 100 + i;

        log_evento(a->id, "PRODUTOR", "ENTROU na regiao critica (SEM protecao)");
        inserir_sem_controle(buffer_global, a->id, valor);
        snprintf(msg, sizeof(msg), "inseriu o valor %d no buffer", valor);
        log_evento(a->id, "PRODUTOR", msg);
        exibir_bruto(buffer_global);
        log_evento(a->id, "PRODUTOR", "SAIU da regiao critica");

        if (i < a->quantidade_itens) sleep(DELAY_PRODUTOR);
    }

    log_evento(a->id, "PRODUTOR", "FINALIZADA");
    free(a);
    return NULL;
}

void *tarefa_consumidor(void *arg) {
    ArgsConsumidor *a = (ArgsConsumidor *) arg;
    char msg[96];
    log_evento(a->id, "CONSUMIDOR", "CRIADA");

    for (int k = 1; k <= a->quantidade_retiradas; k++) {
        log_evento(a->id, "CONSUMIDOR", "ENTROU na regiao critica (SEM protecao)");
        int valor = retirar_sem_controle(buffer_global, a->id);
        if (valor == SLOT_LIVRE)
            snprintf(msg, sizeof(msg), "retirou LIXO (posicao vazia)");
        else
            snprintf(msg, sizeof(msg), "retirou o valor %d do buffer", valor);
        log_evento(a->id, "CONSUMIDOR", msg);
        exibir_bruto(buffer_global);
        log_evento(a->id, "CONSUMIDOR", "SAIU da regiao critica");

        if (k < a->quantidade_retiradas) sleep(DELAY_CONSUMIDOR);
    }

    log_evento(a->id, "CONSUMIDOR", "FINALIZADA");
    free(a);
    return NULL;
}

int main(int argc, char **argv) {
    /* Entrada gerada aleatoriamente (common/gerador.c). Para repetir uma
       execucao, passe a semente impressa: ./pc_v3 <semente>
       Obs.: os valores sao produtor*100+i, entao precisa de
       n_produtores*100 + itens_por_produtor < MAX_VALOR. */
    unsigned int semente = gerador_iniciar(argc > 1 ? (unsigned int) strtoul(argv[1], NULL, 10) : 0);
    ConfigPC cfg = gerador_produtores_consumidores();

    int n_produtores = cfg.n_produtores;
    int itens_por_produtor = cfg.itens_por_produtor;
    int n_consumidores = cfg.n_consumidores;

    if (n_produtores * 100 + itens_por_produtor >= MAX_VALOR) {
        fprintf(stderr, "configuracao grande demais para MAX_VALOR\n");
        return 1;
    }

    int total_itens = n_produtores * itens_por_produtor;

    printf("Semente: %u (passe-a como argumento para repetir esta execucao)\n", semente);
    printf("Produtores: %d | Itens por produtor: %d (total %d) | Consumidores: %d\n\n",
           n_produtores, itens_por_produtor, total_itens, n_consumidores);

    buffer_global = buffer_criar(CAPACIDADE_BUFFER);
    if (buffer_global == NULL) {
        fprintf(stderr, "erro ao criar o buffer\n");
        return 1;
    }
    for (int i = 0; i < buffer_global->capacidade; i++) {
        buffer_global->dados[i] = SLOT_LIVRE;
    }

    pthread_t threads_produtores[n_produtores];
    pthread_t threads_consumidores[n_consumidores];

    for (int i = 0; i < n_produtores; i++) {
        ArgsProdutor *a = malloc(sizeof(ArgsProdutor));
        a->id = i + 1;
        a->quantidade_itens = itens_por_produtor;
        pthread_create(&threads_produtores[i], NULL, tarefa_produtor, a);
    }
    for (int i = 0; i < n_consumidores; i++) {
        ArgsConsumidor *a = malloc(sizeof(ArgsConsumidor));
        a->id = i + 1;
        /* divide o total de retiradas; o ultimo consumidor fica com o resto */
        a->quantidade_retiradas = total_itens / n_consumidores;
        if (i == n_consumidores - 1) {
            a->quantidade_retiradas += total_itens % n_consumidores;
        }
        pthread_create(&threads_consumidores[i], NULL, tarefa_consumidor, a);
    }

    for (int i = 0; i < n_produtores; i++) {
        pthread_join(threads_produtores[i], NULL);
    }
    for (int i = 0; i < n_consumidores; i++) {
        pthread_join(threads_consumidores[i], NULL);
    }

    /* ---------------- relatorio final ---------------- */
    int nunca_consumidos = 0;
    printf("\n===== RESULTADO =====\n");
    printf("Itens produzidos: %d | retiradas feitas pelos consumidores: %d\n",
           total_itens, total_itens);

    printf("Valores produzidos e NUNCA consumidos (sobrescritos ou esquecidos no buffer): ");
    for (int p = 1; p <= n_produtores; p++) {
        for (int i = 1; i <= itens_por_produtor; i++) {
            int v = p * 100 + i;
            if (consumos[v] == 0) {
                printf("%d ", v);
                nunca_consumidos++;
            }
        }
    }
    if (nunca_consumidos == 0) printf("(nenhum)");
    printf("\n");

    int itens_nos_slots = 0;
    for (int i = 0; i < buffer_global->capacidade; i++) {
        if (buffer_global->dados[i] != SLOT_LIVRE) itens_nos_slots++;
    }

    printf("Sobrescritas de dado nao consumido:   %d\n", sobrescritas);
    printf("Leituras de posicao vazia (lixo):     %d\n", leituras_vazias);
    printf("Consumos em duplicidade:              %d\n", consumos_duplicados);
    printf("Buffer no final (esperado: todo vazio e quantidade = 0):\n");
    exibir_bruto(buffer_global);
    printf("Itens realmente nas posicoes: %d | quantidade registrada: %d\n",
           itens_nos_slots, buffer_global->quantidade);

    if (nunca_consumidos > 0 || sobrescritas > 0 || leituras_vazias > 0 ||
        consumos_duplicados > 0 || buffer_global->quantidade != 0) {
        printf("INCONSISTENTE: o buffer foi corrompido por falta de controle de concorrencia.\n");
    } else {
        printf("Desta vez nao houve corrupcao visivel (execute de novo ou aumente JANELA_CORRIDA_MS).\n");
    }

    buffer_destruir(buffer_global);
    return 0;
}