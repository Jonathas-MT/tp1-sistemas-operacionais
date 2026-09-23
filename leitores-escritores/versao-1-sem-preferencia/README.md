# Leitores × Escritores — TP1 Sistemas Operacionais

Tema: Banco. Leitor = consulta de saldo. Escritor = depósito ou saque.
Todas as versões compartilham o mesmo TAD `Conta` (`conta.h` / `conta.c`).

## Pré-requisitos

- Compilador `gcc` (ou `clang`, no macOS)
- Rodar os comandos abaixo a partir da **raiz do repositório**

## Versão 1 — sem preferência de ordem de acesso (pode ocorrer leitura suja)

Compilar:

```bash
gcc -Wall -Wextra -o v1 leitores-escritores/versao-1-sem-preferencia/main.c leitores-escritores/conta.c common/log.c -lpthread
```

Rodar:

```bash
./v1
```

Tempo esperado de execução: ~9s (3 escritores × 3s cada, sequenciais).

## Versão 2 — preferência de escritores (não ocorre leitura suja)

Compilar:

```bash
gcc -Wall -Wextra -o v2 leitores-escritores/versao-2-preferencia-escritores/main.c leitores-escritores/conta.c common/log.c -lpthread
```

Rodar:

```bash
./v2
```

Tempo esperado de execução: ~12s (3 escritores × 1s + leitores lendo juntos por 3s no final).

## Rodar várias vezes seguidas

Útil para comparar execuções e capturar um log com leitura suja bem visível na v1:

```bash
for i in 1 2 3; do echo "=== EXECUÇÃO $i ==="; ./v1; echo; done
```

```bash
for i in 1 2 3; do echo "=== EXECUÇÃO $i ==="; ./v2; echo; done
```

## Se algo der errado

Se a compilação ou a execução falhar, copie a mensagem de erro completa e leve para quem está com essa parte do trabalho.