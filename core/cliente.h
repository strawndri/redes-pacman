#ifndef CLIENTE_H__
#define CLIENTE_H__

#include <termios.h>

extern struct termios tecla_original;
void desliga_modo_jogo();
void liga_modo_jogo();

void cliente_imprime_mapa(char *mapa);
void cliente_recebe_mapa(int socket, unsigned char *seq_s_esperada);
void cliente_recebe_arquivo(int socket, unsigned char *seq_s_esperada);
void cliente_executa(int socket);

#endif