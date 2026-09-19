# TP1 — Programação Concorrente

**Universidade Federal do Amazonas — UFAM**
**Instituto de Computação — IComp**
**Disciplina:** Sistemas Operacionais
**Professor:** João Marcos Bastos Cavalcanti
**Trabalho Prático 1**

## 📋 Sobre o projeto

Este repositório contém a implementação do **Trabalho Prático 1 da disciplina de Sistemas Operacionais**, cujo objetivo é estudar conceitos de **programação concorrente, threads e controle de concorrência utilizando semáforos e mecanismos de sincronização**.

O trabalho aborda dois problemas clássicos de concorrência:

1. **Leitores × Escritores**
2. **Produtores × Consumidores**

As implementações utilizam threads e mecanismos de sincronização para demonstrar diferentes situações de acesso a dados compartilhados, incluindo versões com e sem controle de concorrência.

---

## 🎯 Objetivos

O projeto tem como principais objetivos:

* Compreender o funcionamento de threads;
* Trabalhar com dados compartilhados entre threads;
* Utilizar semáforos, mutex e mecanismos de sincronização;
* Demonstrar regiões críticas;
* Observar situações de bloqueio e desbloqueio de threads;
* Analisar problemas causados pelo acesso concorrente;
* Comparar implementações com e sem controle de concorrência;
* Demonstrar problemas como condições de corrida e leitura inconsistente;
* Utilizar atrasos (`sleep`) para produzir diferentes interleavings entre as threads.

---

# 1. Leitores × Escritores

Nesta parte do trabalho existe uma variável compartilhada por várias threads.

São utilizadas threads de dois tipos:

* **Leitoras:** consultam e exibem os dados compartilhados;
* **Escritoras:** modificam e exibem os dados após a atualização.

Foram desenvolvidas três versões.

### Versão 1 — Sem preferência de acesso

Nesta versão não existe uma preferência explícita entre leitores e escritores.

O objetivo é observar o comportamento das threads quando diferentes operações podem ocorrer concorrentemente, incluindo a possibilidade de **leitura inconsistente ("leitura suja")** durante uma atualização.

```text
leitor ────────► leitura
escritor ─────────────► atualização
leitor ────────► leitura
```

### Versão 2 — Preferência para escritores

Nesta implementação, as threads escritoras possuem preferência sobre as leitoras.

As leitoras somente podem acessar os dados quando não existe escritor ativo ou aguardando acesso de acordo com a política implementada.

O objetivo é impedir que uma leitora observe os dados enquanto uma atualização está ocorrendo.

```text
          escritor
             │
             ▼
        ┌──────────┐
        │ REGIÃO   │
        │ CRÍTICA  │
        └──────────┘
             │
             ▼
          leitores
```

### Versão 3 — Sem controle de concorrência

Nesta versão os mecanismos de sincronização são removidos para demonstrar o problema causado pelo acesso simultâneo às informações compartilhadas.

O problema é particularmente perceptível quando várias threads escritoras tentam modificar o mesmo dado simultaneamente.

Essa versão serve como comparação com as implementações sincronizadas.

---

# 2. Produtores × Consumidores

Nesta parte do trabalho é utilizado um **buffer compartilhado**, implementado como uma estrutura do tipo vetor.

As threads produtoras inserem valores no buffer, enquanto as threads consumidoras retiram esses valores.

O acesso ao buffer precisa ser coordenado para evitar inconsistências.

## Estados do buffer

### Buffer vazio

Quando não existem elementos disponíveis, uma thread consumidora deve ser bloqueada até que algum produtor insira um novo elemento.

```text
Buffer vazio
     │
     ▼
Consumidor bloqueado
     │
     │ produtor insere
     ▼
Consumidor desbloqueado
```

### Buffer cheio

Quando o buffer está cheio, as threads produtoras devem aguardar até que uma thread consumidora retire algum elemento.

```text
Buffer cheio
     │
     ▼
Produtor bloqueado
     │
     │ consumidor retira
     ▼
Produtor desbloqueado
```

Foram desenvolvidas três versões.

### Versão 1 — Vários produtores e um consumidor

A primeira implementação utiliza:

* Várias threads produtoras;
* Uma thread consumidora;
* Um buffer compartilhado;
* Mecanismos de sincronização.

### Versão 2 — Vários produtores e vários consumidores

A segunda implementação amplia o problema para:

* Várias threads produtoras;
* Várias threads consumidoras;
* Buffer compartilhado;
* Controle de acesso concorrente;
* Bloqueio de produtores quando o buffer está cheio;
* Bloqueio de consumidores quando o buffer está vazio.

### Versão 3 — Sem controle de concorrência

Nesta implementação os mecanismos de sincronização são removidos.

O objetivo é demonstrar problemas causados pelo acesso simultâneo ao buffer, principalmente durante operações de inserção e remoção realizadas por diferentes threads.

---

# 🧵 Threads e sincronização

As implementações utilizam threads para representar os processos.

Os mecanismos de sincronização utilizados no projeto incluem:

* `pthread`;
* Semáforos;
* Mutex;
* `lock/unlock`;
* `sleep`.

Os atrasos são configuráveis para permitir diferentes velocidades entre as threads e produzir diferentes ordens de execução.

Por exemplo:

```text
Escritor 1 → sleep(3)
Escritor 2 → sleep(5)
Leitor 1   → sleep(1)
Leitor 2   → sleep(2)
```

Dessa forma, diferentes execuções podem apresentar diferentes interleavings entre as threads.

---

# 📂 Organização do projeto

```text
.
├── leitores-escritores/
│   ├── versao-1-sem-preferencia/
│   ├── versao-2-preferencia-escritores/
│   └── versao-3-sem-concorrencia/
│
├── produtores-consumidores/
│   ├── versao-1-varios-produtores-1-consumidor/
│   ├── versao-2-varios-produtores-varios-consumidores/
│   └── versao-3-sem-concorrencia/
│
├── relatorio/
│   └── relatorio.pdf
│
├── exemplos/
│   ├── leitores-escritores/
│   └── produtores-consumidores/
│
└── README.md
```

---

# ⚙️ Requisitos

Para executar os programas, é necessário possuir:

* GCC ou G++;
* Biblioteca POSIX Threads (`pthread`);
* Suporte a semáforos POSIX;
* Linux, WSL ou outro ambiente compatível com POSIX.

Verifique a instalação do compilador:

```bash
gcc --version
```

ou:

```bash
g++ --version
```

---

# 🚀 Compilação e execução

Cada versão possui seu próprio código-fonte e pode ser compilada utilizando `gcc` ou `g++`, dependendo da implementação.

Exemplo:

```bash
gcc main.c -o programa -pthread
```

Execução:

```bash
./programa
```

Caso sejam utilizados semáforos POSIX, a compilação poderá exigir opções adicionais dependendo do ambiente.

---

# 📊 Saída do programa

As implementações apresentam informações sobre o estado das threads durante a execução.

Entre as informações exibidas estão:

* Criação da thread;
* Tipo da thread;
* Entrada na região crítica;
* Saída da região crítica;
* Bloqueio;
* Desbloqueio;
* Dados lidos;
* Dados modificados;
* Valores produzidos;
* Valores consumidos;
* Estado atual do buffer;
* Finalização da thread.

Exemplo conceitual:

```text
[THREAD 01] Criada - LEITOR
[THREAD 02] Criada - ESCRITOR

[THREAD 01] Entrando na região crítica
[THREAD 01] Leu valor: 100
[THREAD 01] Saindo da região crítica

[THREAD 02] Entrando na região crítica
[THREAD 02] Atualizou valor: 100 -> 150
[THREAD 02] Saindo da região crítica

[THREAD 01] Finalizada
[THREAD 02] Finalizada
```

No problema Produtor × Consumidor:

```text
[PRODUTOR 01] Produzindo valor: 25
[BUFFER] [25, _, _, _, _]

[CONSUMIDOR 01] Consumindo valor: 25
[BUFFER] [_, _, _, _, _]

[PRODUTOR 02] Produzindo valor: 42
[BUFFER] [42, _, _, _, _]
```

---

# 🧪 Comparação das versões

| Problema                  | Versão | Característica                          |
| ------------------------- | ------ | --------------------------------------- |
| Leitores × Escritores     | 1      | Sem preferência de acesso               |
| Leitores × Escritores     | 2      | Preferência para escritores             |
| Leitores × Escritores     | 3      | Sem controle de concorrência            |
| Produtores × Consumidores | 1      | Vários produtores e 1 consumidor        |
| Produtores × Consumidores | 2      | Vários produtores e vários consumidores |
| Produtores × Consumidores | 3      | Sem controle de concorrência            |

---

# 📝 Relatório

O relatório do trabalho apresenta:

* Descrição das soluções adotadas;
* Estratégias de sincronização;
* Estrutura das threads;
* Funcionamento dos semáforos/mutex;
* Exemplos de execução;
* Análise dos resultados;
* Comparação entre as versões;
* Problemas observados nas versões sem controle de concorrência;
* Considerações finais.

O relatório está disponível em:

```text
relatorio/relatorio.pdf
```

---

# 👥 Equipe

| Nome                | Matrícula |
| ------------------- | --------- |
| Nome do estudante 1 | XXXXXXXX  |
| Nome do estudante 2 | XXXXXXXX  |
| Nome do estudante 3 | XXXXXXXX  |

---

# 📚 Referências

As fontes utilizadas durante o desenvolvimento do trabalho devem ser registradas nesta seção, incluindo:

* Livros;
* Slides da disciplina;
* Documentação oficial;
* Sites;
* Artigos;
* Assistentes de IA;
* Outras fontes consultadas.

Exemplos de documentação:

* POSIX Threads (`pthread`);
* Semáforos POSIX;
* Documentação da linguagem C/C++;
* Material disponibilizado pelo professor.

> As fontes efetivamente utilizadas no desenvolvimento devem ser listadas no relatório e neste repositório, conforme solicitado no enunciado do trabalho.

---

# 📅 Informações da atividade

**Disciplina:** Sistemas Operacionais
**Instituição:** Universidade Federal do Amazonas — UFAM
**Instituto:** Instituto de Computação — IComp
**Professor:** João Marcos Bastos Cavalcanti
**Trabalho:** Trabalho Prático 1
**Tema:** Programação Concorrente
**Data de entrega:** 05/10/2026

---

## 📌 Status

🚧 **Em desenvolvimento**

* [ ] Leitores × Escritores — Versão 1
* [ ] Leitores × Escritores — Versão 2
* [ ] Leitores × Escritores — Versão 3
* [ ] Produtores × Consumidores — Versão 1
* [ ] Produtores × Consumidores — Versão 2
* [ ] Produtores × Consumidores — Versão 3
* [ ] Testes
* [ ] Exemplos de execução
* [ ] Relatório
* [ ] Revisão final
