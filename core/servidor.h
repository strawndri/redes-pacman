#ifndef SERVIDOR_H__
#define SERVIDOR_H__
#include "../lib/mensagem.h"
#include "jogo.h"

void servidor_envia_mapa(int socket, struct jogo_t *jogo, unsigned char *seq);
void servidor_envia_arquivo(int socket, char *caminho, enum tipo_msg_t tipo, unsigned char *seq);
void servidor_executa(int socket, char *caminho_mapa);

#endif