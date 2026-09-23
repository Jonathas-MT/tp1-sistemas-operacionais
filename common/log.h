#ifndef LOG_H
#define LOG_H

/*
 * STUB PROVISORIO do TAD Log.
 *
 * O Jonathas ficou responsavel pelo TAD Log definitivo (log.h/log.c),
 * com o formato final combinado pela equipe (tempo, tipo, id, evento).
 * Versao simples so para conseguir compilar e testar a
 * minha parte (v1 e v2) desde ja. Quando o log oficial estiver pronto,
 * troco este arquivo pelo dele - a assinatura das funcoes deve
 * continuar igual (ou eu ajusto as chamadas no meu codigo).
 */

void log_evento(int id, const char *tipo, const char *evento);
void log_evento_valor(int id, const char *tipo, const char *evento, int valor);

#endif
