# Fontes utilizadas no desenvolvimento do trabalho

## Material da disciplina

- Enunciado do Trabalho Prático 1 (Sistemas Operacionais, prof. João Marcos
  Bastos Cavalcanti, UFAM).
- Slides da disciplina: "Gerência de Processos", "Threads" e "Concorrência
  entre Processos" (prof. João Marcos Bastos Cavalcanti).

## Documentação oficial

- Documentação POSIX Threads (`pthread.h`) e Semáforos POSIX
  (`semaphore.h`), consultadas para a assinatura e o comportamento das
  funções `pthread_create`, `pthread_join`, `pthread_mutex_t`, `sem_init`,
  `sem_wait`, `sem_post`, `sem_trywait` e `sem_destroy`.
- Documentação da linguagem C (biblioteca padrão: `stdio.h`, `stdlib.h`,
  `unistd.h`, `time.h`).

## Assistente de IA

- Foi utilizado o assistente de IA Claude (Anthropic) como apoio durante o
  desenvolvimento desta parte do trabalho (Produtores × Consumidores,
  versões 1 e 2, TAD `Buffer`), para:
  - explicação dos conceitos de threads, semáforos (genéricos e binários)
    e exclusão mútua;
  - revisão e depuração do código (identificação de um erro de memória —
    `free` em endereço de pilha — na versão 1);
  - apoio na organização e na redação técnica das seções correspondentes
    do relatório.
- Todo o código gerado com apoio da IA foi lido, testado (compilado e
  executado) e compreendido pela equipe antes de ser incorporado ao
  repositório.

## Observação

Cada integrante é responsável por registrar aqui as fontes específicas
utilizadas em sua parte do trabalho, caso tenha usado outras além das
listadas acima.
