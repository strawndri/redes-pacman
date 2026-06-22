#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jogo.h"
#include "../lib/mensagem.h"

void jogo_carrega_mapa(char *caminho, struct jogo_t *jogo)
{
    int mapa_padrao = 0;
    FILE *file = NULL;

    // inicialização do jogo
    jogo->pacman.pastilhas = 0;
    jogo->movimento = 0;
    jogo->raio_visao = 1;
    jogo->vitoria = 0;
    jogo->colisao = 0;

    if (caminho != NULL)
        file = fopen(caminho, "r");

    if (!file)
    {
        mapa_padrao = 1;
        file = fopen("assets/mapa_padrao.csv", "r");
        if (!file)
            exit(1);
    }

    char linha[256];     // limite de segurança
    int i = 0;           // linha
    int index_ghost = 0; // indice dos fantasmas

    while (fgets(linha, sizeof(linha), file) && i < TAM_MAPA)
    {
        int j = 0; // coluna
        int k = 0; // índice de leitura

        while (linha[k] != '\0' && linha[k] != '\n' && j < TAM_MAPA)
        {
            if (linha[k] != ';')
            {
                jogo->mapa[i][j] = linha[k];

                if (!mapa_padrao)
                {
                    if (linha[k] == PACMAN)
                    {
                        jogo->pacman.x = j;
                        jogo->pacman.y = i;
                    }
                    else if (linha[k] == FANTASMA_VERMELHO ||
                             linha[k] == FANTASMA_AZUL ||
                             linha[k] == FANTASMA_VERDE ||
                             linha[k] == FANTASMA_AMARELO)
                    {
                        jogo->fantasmas[index_ghost].tipo = linha[k];
                        jogo->fantasmas[index_ghost].x = j;
                        jogo->fantasmas[index_ghost].y = i;
                        jogo->fantasmas[index_ghost].dir_atual = 0;
                        jogo->fantasmas[index_ghost].piso = VAZIO;
                        jogo->fantasmas[index_ghost].decisao = 0;
                        index_ghost++;
                    }
                }
                j++;
            }
            k++;
        }
        i++;
    }
    fclose(file);

    if (mapa_padrao)
    {
        char itens[TOTAL_ITENS] = {PACMAN, FANTASMA_VERMELHO, FANTASMA_AZUL,
                                   FANTASMA_VERDE, FANTASMA_AMARELO,
                                   '1', '2', '3', '4', '5', '6'};

        for (int w = 0; w < TOTAL_ITENS; w++)
        {
            int colocado = 0;

            while (!colocado)
            {
                int x = rand() % TAM_MAPA;
                int y = rand() % TAM_MAPA;

                if (jogo->mapa[y][x] == VAZIO)
                {
                    jogo->mapa[y][x] = itens[w];
                    colocado = 1;

                    if (itens[w] == PACMAN)
                    {
                        jogo->pacman.x = x;
                        jogo->pacman.y = y;
                    }
                    else if (itens[w] == FANTASMA_VERMELHO ||
                             itens[w] == FANTASMA_AZUL ||
                             itens[w] == FANTASMA_VERDE ||
                             itens[w] == FANTASMA_AMARELO)
                    {
                        jogo->fantasmas[index_ghost].tipo = itens[w];
                        jogo->fantasmas[index_ghost].x = x;
                        jogo->fantasmas[index_ghost].y = y;
                        jogo->fantasmas[index_ghost].dir_atual = 0;
                        jogo->fantasmas[index_ghost].piso = VAZIO;
                        jogo->fantasmas[index_ghost].decisao = 0;
                        index_ghost++;
                    }
                }
            }
        }
    }
}

int jogo_verifica_colisao(struct jogo_t *jogo)
{
    for (int i = 0; i < QUANTIDADE_FANTASMA; i++)
    {
        if (jogo->pacman.x == jogo->fantasmas[i].x &&
            jogo->pacman.y == jogo->fantasmas[i].y)
        {
            jogo->colisao = 1;
            return 1;
        }
    }

    return 0;
}

int jogo_verifica_vitoria(struct jogo_t *jogo)
{
    if (jogo->pacman.pastilhas >= PASTILHAS_NECESSARIAS)
    {
        jogo->vitoria = 1;
        return 1;
    }

    return 0;
}

int valida_movimento_fantasma(struct jogo_t *jogo, struct fantasmas_t *fantasma, int direcao)
{
    int x = fantasma->x;
    int y = fantasma->y;

    if (direcao == CIMA)
        y--;
    if (direcao == BAIXO)
        y++;
    if (direcao == ESQUERDA)
        x--;
    if (direcao == DIREITA)
        x++;

    if (x < 0 || x >= TAM_MAPA || y < 0 || y >= TAM_MAPA)
        return 0;

    char casa_destino = jogo->mapa[y][x];

    if (casa_destino == PAREDE ||
        casa_destino == FANTASMA_VERMELHO ||
        casa_destino == FANTASMA_AZUL ||
        casa_destino == FANTASMA_VERDE ||
        casa_destino == FANTASMA_AMARELO)
        return 0;

    jogo->mapa[fantasma->y][fantasma->x] = fantasma->piso;

    if (casa_destino != PACMAN)
        fantasma->piso = casa_destino;

    fantasma->x = x;
    fantasma->y = y;
    fantasma->dir_atual = direcao;
    jogo->mapa[y][x] = fantasma->tipo;

    if (casa_destino == PACMAN)
        jogo->colisao = 1;

    return 1;
}

char jogo_move_pacman(struct jogo_t *jogo, int direcao)
{
    int x = jogo->pacman.x;
    int y = jogo->pacman.y;

    if (direcao == MSG_MOV_CIMA)
        y--;
    else if (direcao == MSG_MOV_BAIXO)
        y++;
    else if (direcao == MSG_MOV_ESQ)
        x--;
    else if (direcao == MSG_MOV_DIR)
        x++;
    else
        return VAZIO;

    if (x < 0 || x >= TAM_MAPA || y < 0 || y >= TAM_MAPA)
        return VAZIO;

    char casa_destino = jogo->mapa[y][x];

    if (casa_destino == PAREDE)
    {
        jogo->movimento++;
        if (jogo->movimento % 5 == 0)
            jogo->raio_visao++;
        return VAZIO;
    }

    if (casa_destino == FANTASMA_VERMELHO || casa_destino == FANTASMA_AZUL ||
        casa_destino == FANTASMA_VERDE || casa_destino == FANTASMA_AMARELO)
    {
        return VAZIO;
    }

    jogo->mapa[jogo->pacman.y][jogo->pacman.x] = VAZIO;

    jogo->pacman.x = x;
    jogo->pacman.y = y;

    if (casa_destino >= PASTILHA_MIN && casa_destino <= PASTILHA_MAX)
        jogo->pacman.pastilhas++;

    jogo->mapa[y][x] = PACMAN;

    jogo->movimento++;
    if (jogo->movimento % 5 == 0)
        jogo->raio_visao++;

    return casa_destino;
}

void jogo_move_fantasmas(struct jogo_t *jogo)
{
    for (int i = 0; i < QUANTIDADE_FANTASMA; i++)
    {
        struct fantasmas_t *fantasma = &jogo->fantasmas[i];

        int frente = fantasma->dir_atual;
        int tras;
        int direita;
        int esquerda;

        if (frente == CIMA)
        {
            tras = BAIXO;
            direita = DIREITA;
            esquerda = ESQUERDA;
        }
        if (frente == BAIXO)
        {
            tras = CIMA;
            direita = ESQUERDA;
            esquerda = DIREITA;
        }
        if (frente == DIREITA)
        {
            tras = ESQUERDA;
            direita = BAIXO;
            esquerda = CIMA;
        }
        if (frente == ESQUERDA)
        {
            tras = DIREITA;
            direita = CIMA;
            esquerda = BAIXO;
        }

        if (fantasma->tipo == FANTASMA_VERMELHO)
        {
            if (valida_movimento_fantasma(jogo, fantasma, frente))
                continue;
            if (valida_movimento_fantasma(jogo, fantasma, esquerda))
                continue;
            if (valida_movimento_fantasma(jogo, fantasma, direita))
                continue;
            valida_movimento_fantasma(jogo, fantasma, tras);
        }

        if (fantasma->tipo == FANTASMA_AZUL)
        {
            if (valida_movimento_fantasma(jogo, fantasma, frente))
                continue;
            if (valida_movimento_fantasma(jogo, fantasma, direita))
                continue;
            if (valida_movimento_fantasma(jogo, fantasma, esquerda))
                continue;
            valida_movimento_fantasma(jogo, fantasma, tras);
        }

        if (fantasma->tipo == FANTASMA_VERDE)
        {
            if (valida_movimento_fantasma(jogo, fantasma, frente))
                continue;

            int primeiro_lado = fantasma->decisao ? esquerda : direita;
            int segundo_lado = fantasma->decisao ? direita : esquerda;

            fantasma->decisao = !fantasma->decisao;

            if (valida_movimento_fantasma(jogo, fantasma, primeiro_lado))
                continue;

            if (valida_movimento_fantasma(jogo, fantasma, segundo_lado))
                continue;

            valida_movimento_fantasma(jogo, fantasma, tras);
        }

        if (fantasma->tipo == FANTASMA_AMARELO)
        {
            int direcoes[4] = {CIMA, BAIXO, DIREITA, ESQUERDA};

            for (int j = 0; j < 10; j++)
            {
                int index = rand() % 4;
                if (valida_movimento_fantasma(jogo, fantasma, direcoes[index]))
                    break;
            }
        }
    }
}
