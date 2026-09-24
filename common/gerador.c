#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "gerador.h"

/* Sorteia um inteiro em [min, max]. So e chamada pela thread principal,
   antes de criar as outras threads (rand() nao e thread-safe). */
static int sortear(int min, int max) {
    return min + rand() % (max - min + 1);
}

/* Sorteia um multiplo de GER_PASSO em [min, max]. */
static int sortear_multiplo(int min, int max) {
    return GER_PASSO * sortear(min / GER_PASSO, max / GER_PASSO);
}

unsigned int gerador_iniciar(unsigned int semente) {
    if (semente == 0) {
        semente = (unsigned int) time(NULL) ^ ((unsigned int) getpid() << 16);
        if (semente == 0) semente = 1;
    }
    srand(semente);
    return semente;
}

ConfigLE gerador_leitores_escritores(void) {
    ConfigLE cfg;
    cfg.n_leitores     = sortear(GER_LEITORES_MIN, GER_LEITORES_MAX);
    cfg.n_escritores   = sortear(GER_ESCRITORES_MIN, GER_ESCRITORES_MAX);
    cfg.saldo_inicial  = sortear_multiplo(GER_SALDO_MIN, GER_SALDO_MAX);

    cfg.valores_escritores = malloc(cfg.n_escritores * sizeof(int));
    if (cfg.valores_escritores == NULL) {
        exit(1);
    }
    for (int i = 0; i < cfg.n_escritores; i++) {
        if (rand() % 2 == 0) {
            cfg.valores_escritores[i] = sortear_multiplo(GER_PASSO, GER_DEPOSITO_MAX);
        } else {
            cfg.valores_escritores[i] = -sortear_multiplo(GER_PASSO, GER_SAQUE_MAX);
        }
    }
    return cfg;
}

void gerador_liberar_le(ConfigLE *cfg) {
    free(cfg->valores_escritores);
    cfg->valores_escritores = NULL;
}

ConfigPC gerador_produtores_consumidores(void) {
    ConfigPC cfg;
    cfg.n_produtores        = sortear(GER_PRODUTORES_MIN, GER_PRODUTORES_MAX);
    cfg.itens_por_produtor  = sortear(GER_ITENS_MIN, GER_ITENS_MAX);
    cfg.n_consumidores      = sortear(GER_CONSUMIDORES_MIN, GER_CONSUMIDORES_MAX);
    return cfg;
}