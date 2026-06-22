#ifndef CLIENTE_H__
#define CLIENTE_H__

#include <termios.h>

#define MAP_LINHAS 40
#define MAP_COLUNAS 40

extern struct termios tecla_original;
void desliga_modo_jogo();
void liga_modo_jogo();

void cliente_recebe_mapa(int socket, unsigned char *seq_s_esperada);
int cliente_recebe_arquivo(int socket, unsigned char *seq_s_esperada);
void cliente_executa(int socket);

#endif