#ifndef CLIENTE_H__
#define CLIENTE_H__

#include "../lib/mensagem.h"

unsigned char cliente_stop_and_wait(int socket, struct mensagem_t *msg_send, int seq_c);

void cliente_executa(int socket);

#endif