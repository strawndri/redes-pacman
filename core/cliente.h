#ifndef CLIENTE_H__
#define CLIENTE_H__

#include "../lib/mensagem.h"

void cliente_stop_and_wait(int socket, struct mensagem_t *msg_send, unsigned char *seq_c, unsigned char *seq_s_esperada);

void cliente_executa(int socket);

#endif