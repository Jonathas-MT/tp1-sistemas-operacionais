# Produtores × Consumidores — v2 (vários produtores, vários consumidores)

Buffer compartilhado de 5 posições (TAD `Buffer`, em
`produtores-consumidores/buffer.h` / `buffer.c`), acessado por N threads
produtoras e N threads consumidoras.

## Sincronização usada

- `sem_t vazio` — semáforo genérico, conta posições livres no buffer.
- `sem_t cheio` — semáforo genérico, conta itens disponíveis no buffer.
- `pthread_mutex_t mutex` — garante que só uma thread mexe no buffer por vez.

Quando todos os produtores terminam, a `main` insere um valor especial
(`VALOR_SENTINELA = -1`) no buffer **uma vez para cada consumidor**, para
que cada um saiba que pode parar (sem ficar esperando para sempre).

## Pré-requisitos

- Compilador `gcc` (ou `clang`, no macOS)
- Rodar os comandos abaixo a partir da **raiz do repositório**

## Compilar

```bash
gcc -Wall -Wextra -o v2_pc produtores-consumidores/versao-2-varios-produtores-varios-consumidores/main.c produtores-consumidores/buffer.c common/log.c -lpthread
```

## Rodar

```bash
./v2_pc
```

Configuração atual de teste (valores fixos, temporários — ver comentário
"ENTRADA TEMPORARIA" no início do `main()`): 3 produtores × 4 itens cada
(12 itens no total), 2 consumidores. Produtor produz a cada 1s, cada
consumidor consome a cada 2s. Com 2 consumidores ativos dá para ver os
dois cenários de bloqueio: buffer cheio (produtores bloqueados) no
começo, e buffer vazio (consumidores bloqueados) perto do fim, quando os
produtores já terminaram.

Tempo esperado de execução: ~20s.

## Rodar várias vezes seguidas

```bash
for i in 1 2 3; do echo "=== EXECUÇÃO $i ==="; ./v2_pc; echo; done
```

## Diferença em relação à v1

A única mudança estrutural é ter `n_consumidores` threads consumidoras em
vez de 1, e enviar um sinal de encerramento por consumidor (em vez de um
único sinal). A lógica de sincronização (`vazio`, `cheio`, `mutex`) é
idêntica à v1.

## Se algo der errado

Se a compilação ou a execução falhar, copie a mensagem de erro completa
e leve para quem está com essa parte do trabalho.
