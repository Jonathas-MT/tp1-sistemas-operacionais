# Leitores × Escritores — v3 (sem controle de concorrência)

Mesmo tema das versões 1 e 2 (Banco: leitor = consulta de saldo, escritor =
depósito/saque), mas **sem nenhum semáforo, mutex ou outra forma de
espera**. O objetivo é mostrar o problema do acesso simultâneo ao saldo
compartilhado — e não resolvê-lo.

## Por que o problema aparece

`saldo += valor` parece uma operação única, mas são três passos: ler o
saldo, calcular o novo valor e gravar. Se dois escritores leem o mesmo
saldo antes de qualquer um gravar, o último a gravar apaga a atualização
do outro ("atualização perdida"). Numa máquina real essa janela dura
nanossegundos; aqui o escritor faz os três passos separados com uma pausa
entre a leitura e a gravação (`DELAY_ESCRITOR`), para o erro aparecer em
toda execução, não só de vez em quando.

O programa também inclui mensagens que só servem para apontar o erro no
log (ex.: avisar quando vai sobrescrever a atualização de outro escritor);
elas não corrigem nada.

## Pré-requisitos

- Compilador `gcc` (ou `clang`, no macOS)
- Rodar o comando abaixo a partir da **raiz do repositório**

## Compilar

```bash
gcc -Wall -Wextra -o v3_le leitores-escritores/versao-3-sem-concorrencia/main.c leitores-escritores/conta.c common/log.c common/gerador.c -lpthread
```

## Rodar

```bash
./v3_le
```

Entrada gerada aleatoriamente (mesmo gerador das versões 1 e 2). Para
repetir a mesma entrada, passe a semente impressa no início da execução:

```bash
./v3_le 106457680
```

No final, o programa compara o **saldo esperado** (saldo inicial + soma dos
valores dos escritores) com o **saldo obtido**, e avisa se houve
inconsistência. Como depende do escalonamento das threads, o resultado
pode variar a cada execução — rode algumas vezes seguidas se a primeira
não mostrar o problema.

```bash
for i in 1 2 3; do echo "=== EXECUÇÃO $i ==="; ./v3_le; echo; done
```

