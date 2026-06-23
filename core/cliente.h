#ifndef CLIENTE_H__
#define CLIENTE_H__

#include <termios.h>

#define MAP_LINHAS 40
#define MAP_COLUNAS 40

extern struct termios tecla_original;

// restaura o terminal para o modo original
// RETORNO: void
void desliga_modo_jogo();

// configura o terminal para leitura de tecla a tecla
// RETORNO: void
void liga_modo_jogo();

// recebe e renderiza o apa enviado pelo servidor
// RETORNO: void
void cliente_recebe_mapa(int socket, unsigned char *seq_s_esperada);

// recebe e salva o arquivo enviado pelo servidor, abrindo-o ao final
// RETORNO: 0 jogo em andamento, 1 vitória ou -1 derrota
int cliente_recebe_arquivo(int socket, unsigned char *seq_s_esperada);

// loop principal do cliente
// envia movimentos e atualiza o estado do jogo
// RETORNO: void
void cliente_executa(int socket);

#endif