# TP1 — Programação Concorrente

**Universidade Federal do Amazonas — UFAM**
**Instituto de Computação — IComp**
**Disciplina:** Sistemas Operacionais
**Professor:** João Marcos Bastos Cavalcanti
**Trabalho Prático 1**

---

## 📋 Sobre o projeto

Este repositório contém a implementação do Trabalho Prático 1 da disciplina de Sistemas Operacionais. O objetivo é estudar programação concorrente em C por meio de threads POSIX (`pthread`), semáforos, mutexes e estruturas de dados compartilhadas.

O trabalho aborda dois problemas clássicos:

1. **Leitores × Escritores:** várias threads consultam ou modificam o saldo de uma conta bancária compartilhada.
2. **Produtores × Consumidores:** threads produtoras inserem itens em um buffer circular, enquanto threads consumidoras retiram esses itens.

Para cada problema, foram implementadas três versões, com diferentes estratégias de sincronização. As versões controladas permitem observar a coordenação entre threads; as versões sem controle demonstram problemas decorrentes do acesso concorrente.

O projeto também utiliza geração pseudoaleatória de entradas, atrasos artificiais e registros de eventos para facilitar a análise do comportamento das threads.

---

## 🎯 Objetivos

* Compreender a criação, a execução e a finalização de threads com `pthread`;
* Trabalhar com dados compartilhados entre threads;
* Aplicar semáforos POSIX e mutexes;
* Implementar exclusão mútua e coordenação entre operações concorrentes;
* Observar bloqueios e desbloqueios de threads;
* Analisar leitura suja, atualizações perdidas e condições de corrida;
* Compreender o funcionamento de um buffer circular limitado;
* Investigar problemas de overflow, underflow, sobrescrita de dados e consumo duplicado;
* Comparar estratégias de sincronização e seus efeitos sobre a consistência e a ordem de execução.

---

# 1. Leitores × Escritores

O recurso compartilhado é uma conta bancária representada pelo TAD `Conta`, implementado em `leitores-escritores/conta.h` e `leitores-escritores/conta.c`.

As threads são divididas em dois tipos:

* **Leitoras:** consultam e exibem o saldo da conta.
* **Escritoras:** realizam depósitos ou saques, utilizando valores positivos ou negativos, respectivamente.

O TAD `Conta` implementa as operações sobre o saldo, mas não possui sincronização interna. Cada versão define sua própria política de concorrência.

### Versão 1 — Acesso sem preferência

Arquivo principal: `leitores-escritores/versao-1-sem-preferencia/main.c`

Esta versão utiliza o semáforo POSIX nomeado `mutex_escrita`, inicializado com valor 1, para garantir que somente uma thread escritora modifique o saldo por vez.

As leitoras, porém, não adquirem esse semáforo. Assim, podem consultar o saldo enquanto uma escritora está realizando uma operação.

Essa estratégia permite observar **leituras sujas**, nas quais uma leitora pode acessar um estado intermediário da operação de escrita. Diferentemente da versão sem sincronização, esta versão ainda impede que duas escritoras executem simultaneamente a atualização protegida.

O atraso configurado é de 3 segundos tanto para leitores quanto para escritores. Ele ajuda a tornar a sobreposição das operações mais visível, mas não garante que uma leitura suja ocorra em todas as execuções.

### Versão 2 — Preferência para escritores

Arquivo principal: `leitores-escritores/versao-2-preferencia-escritores/main.c`

Esta versão implementa uma política de preferência para escritores utilizando quatro semáforos POSIX nomeados:

| Semáforo           | Responsabilidade                                                                     |
| ------------------ | ------------------------------------------------------------------------------------ |
| `mutex_leitores`   | Protege o contador `readcount`, que registra leitores ativos.                        |
| `mutex_escritores` | Protege o contador `writecount`, que registra escritores ativos ou aguardando.       |
| `r`                | Controla a entrada de novos leitores quando existem escritores ativos ou aguardando. |
| `w`                | Controla o acesso ao saldo compartilhado.                                            |

Várias leitoras podem consultar o saldo simultaneamente quando não há escritores ativos ou aguardando. A primeira leitora do grupo bloqueia o acesso exclusivo ao saldo, e a última leitora libera esse acesso.

Quando chega o primeiro escritor, a entrada de novos leitores é bloqueada. Os escritores atualizam o saldo de forma exclusiva, e a entrada de leitores é reaberta quando o último escritor termina.

Essa política evita que leitores observem o saldo durante uma escrita, mas pode causar **starvation de leitores** caso escritores continuem chegando sem intervalo.

Os atrasos configurados são de 3 segundos para leitores e 1 segundo para escritores.

### Versão 3 — Sem controle de concorrência

Arquivo principal: `leitores-escritores/versao-3-sem-concorrencia/main.c`

Esta versão remove os mecanismos de sincronização para demonstrar as consequências do acesso concorrente ao saldo.

As operações das escritoras são divididas em etapas: leitura do saldo atual, cálculo do novo valor, espera artificial e gravação do resultado. Essa separação amplia a oportunidade de duas escritoras trabalharem com o mesmo saldo antigo.

O principal problema demonstrado é a **atualização perdida**: uma escritora pode gravar um resultado calculado a partir de um saldo desatualizado, sobrescrevendo a atualização realizada por outra escritora.

O programa também registra situações em que leitoras consultam o saldo enquanto existem escritores em operação e compara o saldo esperado com o saldo obtido ao final.

Os acessos compartilhados sem sincronização constituem uma condição de corrida e, em C, podem resultar em comportamento indefinido. Os atrasos artificiais servem para evidenciar o problema, não para corrigi-lo.

---

# 2. Produtores × Consumidores

O recurso compartilhado é um buffer circular de tamanho fixo, implementado pelo TAD `Buffer`, nos arquivos `produtores-consumidores/buffer.h` e `produtores-consumidores/buffer.c`.

O buffer possui **5 posições** e oferece operações de inserção, remoção e consulta de estado. O TAD não realiza sincronização interna: cada programa é responsável por controlar o acesso concorrente.

### Semáforos e exclusão mútua

Nas versões sincronizadas, são utilizados dois semáforos genéricos e um mutex:

| Mecanismo | Função                                                                                    |
| --------- | ----------------------------------------------------------------------------------------- |
| `vazio`   | Conta as posições livres do buffer. É inicializado com a capacidade do buffer, igual a 5. |
| `cheio`   | Conta os itens disponíveis para consumo. É inicializado com 0.                            |
| `mutex`   | Garante que somente uma thread por vez manipule o buffer.                                 |

Antes de inserir um item, o produtor aguarda uma posição livre por meio de `vazio`. Depois de inserir o item, sinaliza `cheio`.

Antes de retirar um item, o consumidor aguarda um item disponível por meio de `cheio`. Depois da remoção, sinaliza `vazio`.

A ordem de aquisição é importante: primeiro a thread aguarda a condição correspondente e depois adquire o mutex. Assim, uma thread bloqueada à espera de uma vaga ou de um item não mantém o mutex retido, impedindo que a outra parte do sistema altere o estado do buffer.

Para registrar os bloqueios nos logs, as versões sincronizadas utilizam `sem_trywait` antes da espera bloqueante com `sem_wait`. Isso permite registrar quando uma thread precisa aguardar porque o buffer está cheio ou vazio.

### Versão 1 — Vários produtores e um consumidor

Arquivo principal: `produtores-consumidores/versao-1-varios-produtores-1-consumidor/main.c`

Nesta versão:

* Várias threads produtoras inserem itens no buffer;
* Uma única thread consumidora retira os itens;
* Os semáforos `vazio` e `cheio` coordenam a disponibilidade de posições e itens;
* O mutex protege as operações sobre o buffer circular;
* O consumidor termina depois que todos os produtores finalizam e um sinal de encerramento é inserido no buffer.

O sinal de encerramento é representado pelo valor reservado `VALOR_SENTINELA = -1`. A thread principal espera a finalização de todos os produtores e, em seguida, insere esse valor utilizando o protocolo de sincronização do buffer.

Os atrasos configurados são de 1 segundo por item produzido e 2 segundos por item consumido. Essa diferença favorece a observação de bloqueios de produtores quando o buffer fica cheio.

### Versão 2 — Vários produtores e vários consumidores

Arquivo principal: `produtores-consumidores/versao-2-varios-produtores-varios-consumidores/main.c`

Esta versão mantém a mesma estratégia de sincronização da versão 1, mas permite várias threads consumidoras.

O uso de semáforos genéricos permite que produtores e consumidores diferentes sinalizem a disponibilidade de posições e itens. O mutex continua protegendo o buffer contra acessos simultâneos.

Como existem vários consumidores, a thread principal insere **um valor sentinela para cada consumidor**, depois que todos os produtores terminam. Dessa forma, cada consumidor pode receber seu sinal de encerramento sem que os demais permaneçam bloqueados indefinidamente.

Dependendo da quantidade de produtores, consumidores e dos atrasos, podem ocorrer bloqueios tanto por buffer cheio quanto por buffer vazio.

Os atrasos configurados são de 1 segundo para produtores e 2 segundos para consumidores.

### Versão 3 — Sem controle de concorrência

Arquivo principal: `produtores-consumidores/versao-3-sem-concorrencia/main.c`

Esta versão não utiliza semáforos nem mutex para coordenar as operações no buffer.

O objetivo é demonstrar os seguintes problemas:

* **Corridas nos índices:** produtores podem utilizar a mesma posição de entrada, ou consumidores podem utilizar a mesma posição de saída;
* **Overflow:** produtores continuam inserindo quando o buffer está cheio, sobrescrevendo itens ainda não consumidos;
* **Underflow:** consumidores retiram dados quando o buffer está vazio;
* **Consumo duplicado:** duas threads podem registrar o consumo do mesmo item;
* **Perda de dados:** itens podem ser sobrescritos ou permanecer sem consumo;
* **Inconsistência dos contadores:** o número registrado de itens pode divergir do conteúdo real do buffer.

As operações são divididas em etapas com pequenas pausas para tornar as intercalações mais visíveis. A ausência de sincronização é intencional nesta versão didática e pode causar comportamento indefinido em C.

Como a versão não utiliza o protocolo de encerramento das versões sincronizadas, cada produtor possui uma quantidade fixa de itens, e cada consumidor realiza uma quantidade fixa de retiradas. Ao final, o programa apresenta uma contabilidade dos itens produzidos e consumidos, incluindo sobrescritas, leituras de posições vazias e consumos em duplicidade.

---

# 🧵 Threads, atrasos e registros de execução

As threads são criadas e gerenciadas com a biblioteca POSIX Threads (`pthread`). As operações de sincronização utilizam semáforos POSIX e mutexes, conforme a versão.

Os atrasos artificiais são definidos por constantes nos arquivos principais, permitindo observar diferentes intercalações entre as threads.

| Implementação                  | Atraso do leitor/consumidor | Atraso do escritor/produtor |
| ------------------------------ | --------------------------: | --------------------------: |
| Leitores × Escritores — V1     |                         3 s |                         3 s |
| Leitores × Escritores — V2     |                         3 s |                         1 s |
| Leitores × Escritores — V3     |                         3 s |                         3 s |
| Produtores × Consumidores — V1 |             Consumidor: 2 s |               Produtor: 1 s |
| Produtores × Consumidores — V2 |             Consumidor: 2 s |               Produtor: 1 s |
| Produtores × Consumidores — V3 |             Consumidor: 2 s |               Produtor: 1 s |

O módulo `common/log.c`, declarado em `common/log.h`, padroniza a apresentação dos eventos. Os registros incluem a criação e finalização de threads, a entrada e saída das regiões críticas, os bloqueios, as operações realizadas e o estado do buffer.

Os atrasos facilitam a observação dos problemas, mas as garantias de sincronização das versões controladas decorrem dos semáforos e mutexes, e não dos tempos de espera.

---

# 🎲 Geração de entradas e sementes

A geração de entradas está implementada em `common/gerador.c` e declarada em `common/gerador.h`.

Os parâmetros são sorteados antes da criação das threads:

| Problema                  | Parâmetros gerados         | Intervalos definidos no código |
| ------------------------- | -------------------------- | ------------------------------ |
| Leitores × Escritores     | Quantidade de leitores     | 3 a 6                          |
| Leitores × Escritores     | Quantidade de escritores   | 2 a 5                          |
| Leitores × Escritores     | Saldo inicial              | 2000 a 5000                    |
| Leitores × Escritores     | Depósitos                  | Valores positivos até 500      |
| Leitores × Escritores     | Saques                     | Valores negativos até −300     |
| Produtores × Consumidores | Quantidade de produtores   | 2 a 4                          |
| Produtores × Consumidores | Itens por produtor         | 3 a 5                          |
| Produtores × Consumidores | Quantidade de consumidores | 2 a 3                          |

Os valores monetários são sorteados em múltiplos de 10 para facilitar a leitura dos registros. As faixas definidas no gerador também consideram os limites dos depósitos e saques para evitar saldos negativos nas configurações previstas.

Ao iniciar, cada programa imprime a semente utilizada. Caso nenhuma semente seja fornecida, o gerador utiliza uma baseada no horário e no identificador do processo.

É possível informar uma semente como argumento de linha de comando:

```bash
./le_v1 123
```

No PowerShell, por exemplo:

```powershell
.\pc_v2.exe 7
```

A mesma semente reproduz a entrada pseudoaleatória no mesmo ambiente e com a mesma biblioteca C. Entretanto, **não garante a repetição exata da ordem de execução das threads**, pois o escalonamento depende do sistema operacional.

A sequência de números de `rand()` também pode variar entre implementações da biblioteca C.

---

# 📂 Organização do projeto

```text
.
├── leitores-escritores/
│   ├── conta.c
│   ├── conta.h
│   ├── versao-1-sem-preferencia/
│   │   ├── main.c
│   │   └── README.md
│   ├── versao-2-preferencia-escritores/
│   │   ├── main.c
│   │   └── README.md
│   └── versao-3-sem-concorrencia/
│       ├── main.c
│       └── README.md
│
├── produtores-consumidores/
│   ├── buffer.c
│   ├── buffer.h
│   ├── versao-1-varios-produtores-1-consumidor/
│   │   ├── main.c
│   │   └── README.md
│   ├── versao-2-varios-produtores-varios-consumidores/
│   │   ├── main.c
│   │   └── README.md
│   └── versao-3-sem-concorrencia/
│       ├── main.c
│       └── README.md
│
├── common/
│   ├── gerador.c
│   ├── gerador.h
│   ├── log.c
│   └── log.h
│
├── exemplos/
│   ├── saida_le_v1.txt
│   ├── saida_le_v2.txt
│   ├── saida_le_v3.txt
│   ├── saida_pc_v1.txt
│   ├── saida_pc_v2.txt
│   └── saida_pc_v3.txt
│
├── fontes/
│   └── fontes.md
│
├── Relatorio/
│   └── Relatório.pdf
│
├── README.md
└── .gitignore
```

Os executáveis compilados localmente não precisam ser mantidos no repositório, pois podem ser gerados a partir dos arquivos-fonte.

---

# ⚙️ Requisitos

Para compilar e executar o projeto, são necessários:

* GCC compatível com C e com a opção `-pthread`;
* Biblioteca POSIX Threads;
* Suporte às interfaces de semáforos utilizadas pelas implementações;
* Windows com um ambiente GCC compatível para as versões testadas nesse sistema, ou Linux/WSL com Ubuntu.

As versões 1 e 2 de Leitores × Escritores utilizam semáforos POSIX nomeados por meio de `sem_open`. No ambiente utilizado pela equipe, essas versões são compiladas e executadas no WSL com Ubuntu.

As versões de Produtores × Consumidores utilizam `sem_init` e mutexes POSIX. As versões de Leitores × Escritores com semáforos nomeados também dependem do suporte do ambiente a essas interfaces.

Para verificar a instalação do compilador:

```bash
gcc --version
```

No Windows, a disponibilidade de `pthread` e das interfaces POSIX depende da distribuição do GCC instalada. Os comandos a seguir correspondem aos ambientes utilizados nos testes do projeto.

---

# 🚀 Compilação e execução

Execute os comandos a partir da raiz do repositório. Cada versão possui seu próprio `main.c`, mas reutiliza os módulos compartilhados `common/log.c`, `common/gerador.c` e, conforme o problema, `conta.c` ou `buffer.c`.

## 1. Windows — PowerShell

### Leitores × Escritores — Versão 3

```powershell
gcc -Wall -pthread leitores-escritores/versao-3-sem-concorrencia/main.c leitores-escritores/conta.c common/log.c common/gerador.c -o le_v3
```

### Produtores × Consumidores — Versão 1

```powershell
gcc -Wall -pthread produtores-consumidores/versao-1-varios-produtores-1-consumidor/main.c produtores-consumidores/buffer.c common/log.c common/gerador.c -o pc_v1
```

### Produtores × Consumidores — Versão 2

```powershell
gcc -Wall -pthread produtores-consumidores/versao-2-varios-produtores-varios-consumidores/main.c produtores-consumidores/buffer.c common/log.c common/gerador.c -o pc_v2
```

### Produtores × Consumidores — Versão 3

```powershell
gcc -Wall -pthread produtores-consumidores/versao-3-sem-concorrencia/main.c produtores-consumidores/buffer.c common/log.c common/gerador.c -o pc_v3
```

### Executar no Windows

```powershell
.\le_v3.exe
.\pc_v1.exe
.\pc_v2.exe
.\pc_v3.exe
```

Para repetir uma entrada usando uma semente específica, informe o número como argumento. Por exemplo:

```powershell
.\pc_v2.exe 7
```

## 2. Linux/WSL — Ubuntu

Entre no WSL pelo PowerShell:

```powershell
wsl -d Ubuntu
```

Depois, navegue até a raiz do repositório dentro do ambiente Linux. Se o projeto estiver no disco C:, ele poderá estar acessível por um caminho semelhante a `/mnt/c/Users/<usuario>/tp1-sistemas-operacionais`, conforme a localização real da pasta.

### Leitores × Escritores — Versão 1

```bash
gcc -Wall -pthread leitores-escritores/versao-1-sem-preferencia/main.c leitores-escritores/conta.c common/log.c common/gerador.c -o le_v1
```

### Leitores × Escritores — Versão 2

```bash
gcc -Wall -pthread leitores-escritores/versao-2-preferencia-escritores/main.c leitores-escritores/conta.c common/log.c common/gerador.c -o le_v2
```

### Executar no WSL

```bash
./le_v1
./le_v2
```

Para repetir uma entrada com a semente `123`, por exemplo:

```bash
./le_v1 123
```

**Importante:** executáveis compilados no Windows e no WSL pertencem a ambientes diferentes. Compile e execute cada programa no ambiente correspondente.

---

# 📊 Exemplos de execução

Os arquivos de saída disponíveis no repositório registram execuções das seis versões:

| Arquivo                    | Conteúdo                                   |
| -------------------------- | ------------------------------------------ |
| `exemplos/saida_le_v1.txt` | Execução de Leitores × Escritores — V1     |
| `exemplos/saida_le_v2.txt` | Execução de Leitores × Escritores — V2     |
| `exemplos/saida_le_v3.txt` | Execução de Leitores × Escritores — V3     |
| `exemplos/saida_pc_v1.txt` | Execução de Produtores × Consumidores — V1 |
| `exemplos/saida_pc_v2.txt` | Execução de Produtores × Consumidores — V2 |
| `exemplos/saida_pc_v3.txt` | Execução de Produtores × Consumidores — V3 |

Os registros permitem observar eventos como:

* Criação e finalização das threads;
* Entrada e saída das regiões críticas;
* Espera por outro escritor;
* Espera por uma vaga ou por um item no buffer;
* Leitura e atualização do saldo;
* Inserção e remoção de itens;
* Alterações no estado do buffer;
* Problemas detectados nas versões sem sincronização.

A ordem dos eventos e os resultados das versões sem controle podem variar entre execuções.

---

# 🧪 Comparação das versões

| Problema                  | Versão | Estratégia                                                 | Comportamento observado                                                       |
| ------------------------- | ------ | ---------------------------------------------------------- | ----------------------------------------------------------------------------- |
| Leitores × Escritores     | 1      | Semáforo protege apenas os escritores entre si             | Permite leitura suja                                                          |
| Leitores × Escritores     | 2      | Quatro semáforos e preferência para escritores             | Evita leitura simultânea durante escrita; pode causar starvation de leitores  |
| Leitores × Escritores     | 3      | Sem sincronização                                          | Pode apresentar atualizações perdidas e divergência no saldo                  |
| Produtores × Consumidores | 1      | Vários produtores, um consumidor, semáforos e mutex        | Demonstra bloqueio de produtores quando o buffer está cheio                   |
| Produtores × Consumidores | 2      | Vários produtores e vários consumidores, semáforos e mutex | Demonstra bloqueios por buffer cheio ou vazio                                 |
| Produtores × Consumidores | 3      | Sem sincronização                                          | Pode apresentar sobrescritas, leituras de posições vazias e consumo duplicado |

As versões sincronizadas mostram como a coordenação das threads preserva a consistência dos dados. As versões sem controle são experimentos didáticos para observar falhas que podem surgir quando operações concorrentes acessam e modificam os mesmos dados sem proteção.

---

# 📝 Relatório

O relatório apresenta a implementação e a análise das seis versões, incluindo:

* Organização dos TADs `Conta` e `Buffer`;
* Estratégias de sincronização;
* Funcionamento dos semáforos e mutexes;
* Análise dos bloqueios e da preferência para escritores;
* Discussão sobre starvation;
* Geração de entradas e reprodução por semente;
* Exemplos de execução;
* Atualizações perdidas e leituras sujas;
* Overflow, underflow, sobrescritas e consumos duplicados;
* Comparação dos resultados e limitações das implementações.

O relatório em PDF está disponível em:

`Relatorio/Relatório.pdf`

As fontes consultadas durante o desenvolvimento estão registradas em:

`fontes/fontes.md`

---

# 📚 Referências e fontes utilizadas

As fontes efetivamente utilizadas pela equipe estão documentadas em `fontes/fontes.md`. Entre elas estão:

* Enunciado do Trabalho Prático 1 da disciplina de Sistemas Operacionais;
* Slides da disciplina sobre gerência de processos, threads e concorrência;
* Documentação de POSIX Threads e semáforos POSIX;
* Documentação da linguagem C;
* Bibliografia clássica sobre semáforos, leitores–escritores e sistemas operacionais;
* Assistente de inteligência artificial Claude (Anthropic), utilizado como apoio no desenvolvimento e na revisão técnica, conforme registrado pela equipe.

O relatório também cita as seguintes referências bibliográficas:

1. Dijkstra, E. W. *Co-operating sequential processes*. 1968.
2. Courtois, P. J., Heymans, F. e Parnas, D. L. *Concurrent control with “readers” and “writers”*. Communications of the ACM, 1971.
3. Downey, A. B. *The Little Book of Semaphores*. 2. ed., 2009.
4. Silberschatz, A., Galvin, P. B. e Gagne, G. *Operating System Concepts*. 10. ed., 2018.

---

# 👥 Equipe

| Nome                          | Matrícula |
| ----------------------------- | --------- |
| Jonathas Monteiro da Trindade | 22551921  |
| Beatriz Quaresma Athaide      | 22552431  |
| Ruthelene Rodrigues Farias    | 22552011  |

---

# 📅 Informações da atividade

* **Disciplina:** Sistemas Operacionais
* **Instituição:** Universidade Federal do Amazonas — UFAM
* **Instituto:** Instituto de Computação — IComp
* **Professor:** João Marcos Bastos Cavalcanti
* **Trabalho:** Trabalho Prático 1
* **Tema:** Programação Concorrente
* **Data de entrega informada:** 09/10/2026

---

## 📌 Status do projeto

* [x] Leitores × Escritores — Versão 1
* [x] Leitores × Escritores — Versão 2
* [x] Leitores × Escritores — Versão 3
* [x] Produtores × Consumidores — Versão 1
* [x] Produtores × Consumidores — Versão 2
* [x] Produtores × Consumidores — Versão 3
* [x] Testes de compilação e execução
* [x] Exemplos de saída
* [x] Relatório
* [x] Revisão da documentação
