#ifndef JOGO_H__
#define JOGO_H__

#define TAM_MAPA 40
#define TOTAL_ITENS 11
#define QUANTIDADE_FANTASMA 4
#define CIMA 0
#define DIREITA 1
#define BAIXO 2
#define ESQUERDA 3

struct pacman_t
{
    int x;
    int y;
    int vida;
    int pastilhas;
    int direcao;
};

struct fantasmas_t
{
    int x;
    int y;
    char tipo;
    int dir_atual;
    char piso;
    int decisao;
};

struct jogo_t
{
    char mapa[TAM_MAPA][TAM_MAPA];
    struct pacman_t pacman;
    struct fantasmas_t fantasmas[QUANTIDADE_FANTASMA];
    int raio_visão;
    int movimento;
};

void jogo_carrega_mapa(char *caminho, struct jogo_t *jogo);
char jogo_move_pacman(struct jogo_t *jogo, int direcao);

void jogo_move_fantasmas(struct jogo_t *jogo);
int valida_movimento(struct jogo_t *jogo, struct fantasmas_t *fantasma, int direcao);

#endif