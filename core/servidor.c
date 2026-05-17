#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "servidor.h"
#include "../lib/mensagem.h"

void servidor_executa(int socket)
{
    printf("executando em modo servidor\n");

    struct mensagem_t msg;
    char mapa_teste[3][3] = {
        {'0', '0', '0'},
        {'0', 'P', '0'},
        {'0', '0', '0'}};

    int pac_x = 1; // coluna
    int pac_y = 1; // linha

    char mapa_str[16]; // ira enviar o mapa

    // [0] implementando método stop-and-wait
    while (1)
    {
        printf("aguardando msg...\n");

        if (mensagem_recebe(socket, &msg) > 0)
        {
            if (crc8_gera(msg.dados, msg.tamanho) != msg.crc)
            {
                printf("erro CRC, enviando nack\n");

                struct mensagem_t *nack = mensagem_cria(0, MSG_NACK, NULL);
                if (mensagem_envia(socket, nack) >= 0)
                    printf("nack enviado\n");
                else
                    printf("nack não enviada\n");

                free(nack);
                continue;
            }

            // [1] processa mensagem
            struct mensagem_t *msg_resp = NULL;
            int moveu = 0;
            int x = pac_x;
            int y = pac_y;

            switch (msg.tipo)
            {
            // inicialização:
            case MSG_INICIO:
                printf("inciando jogo - mapa\n");
                moveu = 1;
                break;

            // movimentação: obs: precisa aplicar movimentação
            case MSG_MOV_CIMA:
                printf("movimento: cima\n");
                y--;
                moveu = 1;
                break;

            case MSG_MOV_BAIXO:
                printf("movimento: baixo\n");
                y++;
                moveu = 1;
                break;

            case MSG_MOV_ESQ:
                printf("movimento: esquerda\n");
                x--;
                moveu = 1;
                break;

            case MSG_MOV_DIR:
                printf("movimento: direita\n");
                x++;
                moveu = 1;
                break;

            // erro ao enviar - por hora apenas o mapa
            case MSG_NACK:
                moveu = 1;
                break;

            // fim:
            case MSG_FIM:
                printf("encerramento do jogo\n");
                return;

            // erro:
            default:
                break;
            }

            if (moveu)
            {
                if (x >= 0 && x <= 2 && y >= 0 && y <= 2)
                {
                    mapa_teste[pac_y][pac_x] = '0';
                    pac_x = x;
                    pac_y = y;
                    mapa_teste[pac_y][pac_x] = 'P';
                }

                sprintf(mapa_str, "%c%c%c\n%c%c%c\n%c%c%c",
                        mapa_teste[0][0], mapa_teste[0][1], mapa_teste[0][2],
                        mapa_teste[1][0], mapa_teste[1][1], mapa_teste[1][2],
                        mapa_teste[2][0], mapa_teste[2][1], mapa_teste[2][2]);

                msg_resp = mensagem_cria(strlen(mapa_str), MSG_VISUAL, (unsigned char *)mapa_str);
                mensagem_envia(socket, msg_resp);
                free(msg_resp);
            }
        }
    }
}
