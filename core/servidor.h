#ifndef SERVIDOR_H__
#define SERVIDOR_H__
#include "../lib/mensagem.h"

struct jogo_t
{
    char mapa[TAM_MAPA][TAM_MAPA];
    int pac_x;
    int pac_y;
    // int raio_visão;
    int movimento;
};

void servidor_executa(int socket);

void servidor_carrega_csv(char *caminho, struct jogo_t *jogo);

void servidor_visualizacao(char *buffer, struct jogo_t *jogo);

void servidor_envia_mapa(int socket, char *mapa_str, unsigned char *seq);
void servidor_envia_arquivo(int socket, char *caminho, enum tipo_msg_t tipo, unsigned char *seq);

#endif