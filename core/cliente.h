#ifndef CLIENTE_H__
#define CLIENTE_H__

#include "../lib/mensagem.h"

// inicialização
void cliente_inicializacao(int socket);

// envio de movimento
void cliente_envia_mov(int socket, enum tipo_msg_t mov);

void cliente_executa(int socket);

#endif