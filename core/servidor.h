#ifndef SERVIDOR_H__
#define SERVIDOR_H__
#include "../lib/mensagem.h"
#include "jogo.h"

// serializa e envia o mapa atual do jogo ao cliente,
// respeitando o raio de visão do pacman
// RETORNO: void
void servidor_envia_mapa(int socket, struct jogo_t *jogo, unsigned char *seq);

// lê um arquivo do disco e o envia ao cliente em partes via para-e-espera
// RETORNO: void
void servidor_envia_arquivo(int socket, char *caminho, enum tipo_msg_t tipo, unsigned char *seq);

// loop principal do servidor
// processa movimentos do cliente e atualizao estado do jogo
// RETORNO: void
void servidor_executa(int socket, char *caminho_mapa);

#endif