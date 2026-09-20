#include "conta.h"

void conta_criar(Conta *c, int saldo_inicial) {
    c->saldo = saldo_inicial;
}

int conta_consultar(Conta *c) {
    return c->saldo;
}

void conta_depositar(Conta *c, int valor) {
    c->saldo += valor;
}

void conta_sacar(Conta *c, int valor) {
    c->saldo -= valor;
}
