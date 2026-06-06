#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jogo.h"
#include "../lib/mensagem.h"

void jogo_carrega_mapa(char *caminho, struct jogo_t *jogo)
{
    int mapa_padrao = 0;
    FILE *file = NULL;

    // inicialização geral
    jogo->pacman.vida = 1;
    jogo->pacman.pastilhas = 0;
    jogo->movimento = 0;
    jogo->raio_visão = 1;

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
                    if (linha[k] == 'P')
                    {
                        jogo->pacman.x = j;
                        jogo->pacman.y = i;
                    }
                    else if (linha[k] == 'R' || linha[k] == 'B' || linha[k] == 'G' || linha[k] == 'Y')
                    {
                        jogo->fantasmas[index_ghost].tipo = linha[k];
                        jogo->fantasmas[index_ghost].x = j;
                        jogo->fantasmas[index_ghost].y = i;
                        jogo->fantasmas[index_ghost].dir_atual = 0;
                        jogo->fantasmas[index_ghost].piso = '0';
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
        char itens[TOTAL_ITENS] = {'P', 'R', 'B', 'G', 'Y', '1', '2', '3', '4', '5', '6'};

        for (int i = 0; i < TOTAL_ITENS; i++)
        {
            int colocado = 0;

            while (!colocado)
            {
                int x = rand() % TAM_MAPA;
                int y = rand() % TAM_MAPA;

                if (jogo->mapa[y][x] == '0')
                {
                    jogo->mapa[y][x] = itens[i];
                    colocado = 1;

                    if (itens[i] == 'P')
                    {
                        jogo->pacman.x = x;
                        jogo->pacman.y = y;
                    }
                    else if (itens[i] == 'R' || itens[i] == 'B' || itens[i] == 'G' || itens[i] == 'Y')
                    {
                        jogo->fantasmas[index_ghost].tipo = itens[i];
                        jogo->fantasmas[index_ghost].x = x;
                        jogo->fantasmas[index_ghost].y = y;
                        jogo->fantasmas[index_ghost].dir_atual = 0;
                        jogo->fantasmas[index_ghost].piso = '0';
                        jogo->fantasmas[index_ghost].decisao = 0;
                        index_ghost++;
                    }
                }
            }
        }
    }
}

char jogo_move_pacman(struct jogo_t *jogo, int direcao)
{
    int x = jogo->pacman.x;
    int y = jogo->pacman.y;
    int moveu = 1;

    if (direcao == MSG_MOV_CIMA)
        y--;
    else if (direcao == MSG_MOV_BAIXO)
        y++;
    else if (direcao == MSG_MOV_ESQ)
        x--;
    else if (direcao == MSG_MOV_DIR)
        x++;
    else
        moveu = 0;

    if (moveu)
    {
        if (x >= 0 && x <= TAM_MAPA && y >= 0 && y <= TAM_MAPA)
        {
            char casa_destino = jogo->mapa[y][x];
            if (casa_destino != 'X')
            {
                jogo->mapa[jogo->pacman.y][jogo->pacman.x] = '0';
                jogo->pacman.x = x;
                jogo->pacman.y = y;
                jogo->mapa[jogo->pacman.y][jogo->pacman.x] = 'P';

                jogo->movimento++;
                if (jogo->movimento % 5 == 0)
                    jogo->raio_visão++;

                if (casa_destino >= '1' && casa_destino <= '6')
                    jogo->pacman.pastilhas++;
                if (casa_destino == 'R' || casa_destino == 'B' || casa_destino == 'G' || casa_destino == 'Y')
                    jogo->pacman.vida = 0;
            }

            return casa_destino;
        }
    }

    return '0';
}

int valida_movimento(struct jogo_t *jogo, struct fantasmas_t *fantasma, int direcao)
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

    // validação: tamanho do mapa
    if (x < 0 || x > TAM_MAPA || y < 0 || y > TAM_MAPA)
        return 0;

    char casa_destino = jogo->mapa[y][x];

    // validação: colisão com outros fantasmas/paredes
    if (casa_destino == 'X' || casa_destino == 'R' || casa_destino == 'B' || casa_destino == 'G' || casa_destino == 'Y')
        return 0;

    jogo->mapa[fantasma->y][fantasma->x] = fantasma->piso;

    if (casa_destino != 'P')
        fantasma->piso = casa_destino;

    fantasma->x = x;
    fantasma->y = y;
    fantasma->dir_atual = direcao;
    jogo->mapa[y][x] = fantasma->tipo;

    if (casa_destino == 'P')
        jogo->pacman.vida = 0;

    return 1;
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

        // lógica de direcionamento dos fantasmas
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

        // regra da mão esquerda
        if (fantasma->tipo == 'R')
        {
            if (valida_movimento(jogo, fantasma, frente))
                continue;
            if (valida_movimento(jogo, fantasma, esquerda))
                continue;
            if (valida_movimento(jogo, fantasma, direita))
                continue;
            valida_movimento(jogo, fantasma, esquerda);
        }

        // regra da mão direita
        if (fantasma->tipo == 'B')
        {
            if (valida_movimento(jogo, fantasma, frente))
                continue;
            if (valida_movimento(jogo, fantasma, direita))
                continue;
            if (valida_movimento(jogo, fantasma, esquerda))
                continue;
            valida_movimento(jogo, fantasma, esquerda);
        }

        // alterna direita esquerda a cada decisão
        if (fantasma->tipo == 'G')
        {
            if (valida_movimento(jogo, fantasma, frente))
                continue;

            // lógica de decisão
            int primeiro_lado;
            int segundo_lado;

            if (fantasma->decisao)
            {
                primeiro_lado = esquerda;
                segundo_lado = direita;
            }
            else
            {
                primeiro_lado = direita;
                segundo_lado = esquerda;
            }

            if (valida_movimento(jogo, fantasma, primeiro_lado))
            {
                fantasma->decisao = !fantasma->decisao;
                continue;
            }
            if (valida_movimento(jogo, fantasma, segundo_lado))
            {
                fantasma->decisao = !fantasma->decisao;
                continue;
            }

            valida_movimento(jogo, fantasma, tras);
        }

        // movimentos aleatórios
        if (fantasma->tipo == 'Y')
        {
            int direcoes[4] = {CIMA, BAIXO, DIREITA, ESQUERDA};

            // tenta até achar um válido
            for (int i = 0; i < 10; i++)
            {
                int index = rand() % 4;
                if (valida_movimento(jogo, fantasma, direcoes[index]))
                    break;
            }
        }
    }
}
