#ifndef JOGO_H__
#define JOGO_H__

// constantes do mapa
#define TAM_MAPA 40
#define TOTAL_ITENS 11
#define QUANTIDADE_FANTASMA 4
#define PASTILHAS_NECESSARIAS 6

// constantes de direção
#define CIMA 0
#define DIREITA 1
#define BAIXO 2
#define ESQUERDA 3

// caracteres do mapa
#define PACMAN 'P'
#define PAREDE 'X'
#define VAZIO '0'
#define PASTILHA_MIN '1'
#define PASTILHA_MAX '6'

// tipos de fantasma
#define FANTASMA_VERMELHO 'R'
#define FANTASMA_AZUL 'B'
#define FANTASMA_VERDE 'G'
#define FANTASMA_AMARELO 'Y'

struct pacman_t
{
    int x;
    int y;
    int pastilhas;
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
    int raio_visao;
    int movimento;
    int vitoria;
    int colisao;
};

void jogo_carrega_mapa(char *caminho, struct jogo_t *jogo);
char jogo_move_pacman(struct jogo_t *jogo, int direcao);

int jogo_verifica_colisao(struct jogo_t *jogo);
int jogo_verifica_vitoria(struct jogo_t *jogo);

void jogo_move_fantasmas(struct jogo_t *jogo);
int valida_movimento_fantasma(struct jogo_t *jogo, struct fantasmas_t *fantasma, int direcao);

#endif