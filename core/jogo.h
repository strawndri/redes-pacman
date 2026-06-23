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

// estrutura que define o pacman
struct pacman_t
{
    int x;          // coordenada x
    int y;          // coordenada y
    int pastilhas;  // quantidade de pastilhas coletadas
};

// estrutura que define um fantasma
struct fantasmas_t
{
    int x;          // coordenada x
    int y;          // coordenada y
    char tipo;      // tipo do fantasma (R, G, B, Y)
    int dir_atual;  // direção atuação do movimento
    char piso;      // caractere do chão sob o fantasma
    int decisao;    // usado pelo fantasma verde para alternar direção
};

// estrutura que define o jogo
struct jogo_t
{
    char mapa[TAM_MAPA][TAM_MAPA];                     // grade do mapa
    struct pacman_t pacman;                            // estado do pacman
    struct fantasmas_t fantasmas[QUANTIDADE_FANTASMA]; // estado dos fantasmas
    int raio_visao;                                    // raio de visibilidade do pacman
    int movimento;                                     // contador de movimentos realizados
    int vitoria;                                       // 1 se o jogador venceu
    int colisao;                                       // 1 se houve colisão com fantasma
};

// carrega o mapa a partir de um arquivo CSV
// RETORNO: void
void jogo_carrega_mapa(char *caminho, struct jogo_t *jogo);

// move o pacman na direção indicada
// RETORNO: caractere da casa destino (pastilha, vazio ou parede)
char jogo_move_pacman(struct jogo_t *jogo, int direcao);

// verifica se o pacm está na mesma posição que algum fantasma
// RETORNO: 1 se houve colicao, 0 caso contrário
int jogo_verifica_colisao(struct jogo_t *jogo);

// verifica se o jogador coletou todas as pastilhas necessárias
// RETORNO: 1 se venceu, 0 caso contrário
int jogo_verifica_vitoria(struct jogo_t *jogo);

// move todos os fantamas de acordo com as regras estabelecidas
// RETORNO: void
void jogo_move_fantasmas(struct jogo_t *jogo);

// tenta mover um fantasma na direção indicada, validando colisões
// RETORNO: 1 se o movimento foi realizado, 0 caso contrário
int valida_movimento_fantasma(struct jogo_t *jogo, struct fantasmas_t *fantasma, int direcao);

#endif