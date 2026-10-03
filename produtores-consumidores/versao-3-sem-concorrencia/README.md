# Produtores × Consumidores — v3 (sem controle de concorrência)

Vários produtores e vários consumidores acessando o mesmo buffer de 5
posições, **sem semáforos nem mutex**: o produtor não espera vaga e o
consumidor não espera item. O objetivo é mostrar onde e quando o buffer
se corrompe.

## Problemas que podem aparecer no log

- **Corrida nos índices**: dois produtores leem o mesmo índice de entrada
  e gravam na mesma posição (um valor apaga o outro); o mesmo vale para
  dois consumidores lendo o mesmo índice de saída (consumo em duplicidade).
- **Overflow**: com o buffer cheio, o produtor continua e sobrescreve
  dados ainda não consumidos.
- **Underflow**: com o buffer vazio, o consumidor continua e lê uma
  posição vazia, podendo deixar o contador de itens negativo.

Quais problemas aparecem — e quantos — varia a cada execução, pois depende
do escalonamento das threads escolhido pelo sistema operacional. Para que
a corrida apareça em toda execução (e não só de vez em quando), inserir e
retirar são feitos em passos separados, com uma pausa curta
(`JANELA_CORRIDA_MS`) entre ler o índice e usá-lo.

Como não há semáforos, não é seguro usar o valor sentinela das versões 1 e
2 para avisar o fim: aqui cada produtor produz uma quantidade fixa e cada
consumidor faz uma quantidade fixa de retiradas, de modo que o programa
sempre termina, mesmo com o buffer corrompido.

## Pré-requisitos

- Compilador `gcc` (ou `clang`, no macOS)
- Rodar o comando abaixo a partir da **raiz do repositório**

## Compilar

```bash
gcc -Wall -Wextra -o v3_pc produtores-consumidores/versao-3-sem-concorrencia/main.c produtores-consumidores/buffer.c common/log.c common/gerador.c -lpthread
```

## Rodar

```bash
./v3_pc
```

Entrada gerada aleatoriamente (mesmo gerador das versões 1 e 2). Para
repetir a mesma entrada:

```bash
./v3_pc 2049731252
```

No final, o programa mostra os valores produzidos e nunca consumidos, o
número de sobrescritas, de leituras de posição vazia e de consumos em
duplicidade, comparando com o que seria esperado sem corrupção.

```bash
for i in 1 2 3; do echo "=== EXECUÇÃO $i ==="; ./v3_pc; echo; done
```

