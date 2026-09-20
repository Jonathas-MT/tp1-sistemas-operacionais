#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include "log.h"

/* Mutex so para o log: evita que duas threads imprimam ao mesmo tempo
   e a saida saia embaralhada no terminal. Isso NAO tem relacao com a
   sincronizacao do problema leitores/escritores - e so higiene de
   impressao. */
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

static void imprime_timestamp(void) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char buf[9];
    strftime(buf, sizeof(buf), "%H:%M:%S", tm_info);
    printf("[%s] ", buf);
}

void log_evento(int id, const char *tipo, const char *evento) {
    pthread_mutex_lock(&log_mutex);
    imprime_timestamp();
    printf("%s %d - %s\n", tipo, id, evento);
    fflush(stdout);
    pthread_mutex_unlock(&log_mutex);
}

void log_evento_valor(int id, const char *tipo, const char *evento, int valor) {
    pthread_mutex_lock(&log_mutex);
    imprime_timestamp();
    printf("%s %d - %s (saldo = %d)\n", tipo, id, evento, valor);
    fflush(stdout);
    pthread_mutex_unlock(&log_mutex);
}
