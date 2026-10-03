# Produtores × Consumidores — v1 (vários produtores, 1 consumidor)

Buffer compartilhado de 5 posições (TAD `Buffer`, em
`produtores-consumidores/buffer.h` / `buffer.c`), acessado por N threads
produtoras e 1 thread consumidora. A entrada (quantidade de produtores e
itens por produtor) é gerada aleatoriamente por `common/gerador.c`.

## Sincronização usada

- `sem_t vazio` — semáforo genérico, conta posições livres no buffer.
- `sem_t cheio` — semáforo genérico, conta itens disponíveis no buffer.
- `pthread_mutex_t mutex` — garante que só uma thread mexe no buffer por vez.

Quando todos os produtores terminam, a `main` insere um valor especial
(`VALOR_SENTINELA = -1`) no buffer, para o consumidor saber que pode parar.

## Pré-requisitos

- Compilador `gcc` (ou `clang`, no macOS)
- Rodar os comandos abaixo a partir da **raiz do repositório**

## Compilar

```bash
gcc -Wall -Wextra -o v1_pc produtores-consumidores/versao-1-varios-produtores-1-consumidor/main.c produtores-consumidores/buffer.c common/log.c common/gerador.c -lpthread
```

## Rodar

```bash
./v1_pc
```

A cada execução, o gerador sorteia a quantidade de produtores e de itens
por produtor (esta versão sempre tem 1 consumidor), e imprime a semente
usada. Para repetir a mesma entrada, passe a semente como argumento:

```bash
./v1_pc 123
```

Produtor produz a cada 1s, consumidor consome a cada 2s — como o
consumidor é mais lento, o buffer costuma encher e os produtores ficam
bloqueados algumas vezes (aparece no log como
`BLOQUEADA (buffer cheio, aguardando vaga)`).

## Rodar várias vezes seguidas

```bash
for i in 1 2 3; do echo "=== EXECUÇÃO $i ==="; ./v1_pc; echo; done
```
