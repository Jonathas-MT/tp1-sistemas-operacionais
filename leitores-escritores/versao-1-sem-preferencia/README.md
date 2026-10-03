# Leitores × Escritores — TP1 Sistemas Operacionais

Tema: Banco. Leitor = consulta de saldo. Escritor = depósito ou saque.
Todas as versões compartilham o mesmo TAD `Conta` (`conta.h` / `conta.c`) e
a mesma entrada gerada aleatoriamente por `common/gerador.c`.

## Pré-requisitos

- Compilador `gcc` (ou `clang`, no macOS)
- Rodar os comandos abaixo a partir da **raiz do repositório**

## Versão 1 — sem preferência de ordem de acesso (pode ocorrer leitura suja)

Compilar:

```bash
gcc -Wall -Wextra -o v1 leitores-escritores/versao-1-sem-preferencia/main.c leitores-escritores/conta.c common/log.c common/gerador.c -lpthread
```

Rodar:

```bash
./v1
```

A cada execução, o gerador sorteia a quantidade de leitores, de escritores,
o saldo inicial e o valor de cada escritor, e imprime a semente usada. Para
repetir a mesma entrada depois, passe a semente como argumento:

```bash
./v1 42
```

## Versão 2 — preferência de escritores (não ocorre leitura suja)

Compilar:

```bash
gcc -Wall -Wextra -o v2 leitores-escritores/versao-2-preferencia-escritores/main.c leitores-escritores/conta.c common/log.c common/gerador.c -lpthread
```

Rodar:

```bash
./v2
```

Mesma entrada aleatória da v1 (também aceita uma semente como argumento).

## Rodar várias vezes seguidas

Útil para comparar execuções e capturar um log com leitura suja bem visível na v1:

```bash
for i in 1 2 3; do echo "=== EXECUÇÃO $i ==="; ./v1; echo; done
```

```bash
for i in 1 2 3; do echo "=== EXECUÇÃO $i ==="; ./v2; echo; done
```

