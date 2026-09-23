#include <stdio.h>
#include <stdlib.h>
#include "buffer.h"

Buffer *buffer_criar(int capacidade) {
    if (capacidade <= 0) return NULL;

    Buffer *b = malloc(sizeof(Buffer));
    if (b == NULL) return NULL;

    b->dados = malloc(capacidade * sizeof(int));
    if (b->dados == NULL) {
        free(b);
        return NULL;
    }

    b->capacidade = capacidade;
    b->entrada = 0;
    b->saida = 0;
    b->quantidade = 0;
    return b;
}

void buffer_destruir(Buffer *b) {
    if (b == NULL) return;
    free(b->dados);
    free(b);
}

int buffer_cheio(const Buffer *b) {
    return b->quantidade == b->capacidade;
}

int buffer_vazio(const Buffer *b) {
    return b->quantidade == 0;
}

void buffer_inserir(Buffer *b, int valor) {
    if (buffer_cheio(b)) {
        fprintf(stderr, "ERRO: tentativa de inserir em buffer cheio\n");
        return;
    }
    b->dados[b->entrada] = valor;
    b->entrada = (b->entrada + 1) % b->capacidade;
    b->quantidade++;
}

int buffer_retirar(Buffer *b) {
    if (buffer_vazio(b)) {
        fprintf(stderr, "ERRO: tentativa de retirar de buffer vazio\n");
        return -1;
    }
    int valor = b->dados[b->saida];
    b->saida = (b->saida + 1) % b->capacidade;
    b->quantidade--;
    return valor;
}

void buffer_exibir(const Buffer *b) {
    printf("        buffer: [ ");
    for (int i = 0; i < b->capacidade; i++) {
        int distancia = (i - b->saida + b->capacidade) % b->capacidade;
        if (distancia < b->quantidade)
            printf("%d", b->dados[i]);
        else
            printf("_");
        if (i < b->capacidade - 1) printf(" | ");
    }
    printf(" ]\n");
}
