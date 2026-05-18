#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cliente.h"
#include "../lib/mensagem.h"

// atualmente apenas para o mapa
void cliente_stop_and_wait(int socket, struct mensagem_t *msg_send, unsigned char *seq_c, unsigned char *seq_s_esperada)
{
    // envia mensagem
    int ack_get = 0;
    struct mensagem_t msg_get;

    while (!ack_get)
    {
        mensagem_envia(socket, msg_send);

        if (mensagem_recebe(socket, &msg_get, TIME_OUT_SEND) > 0)
        {
            if (msg_get.tipo == MSG_NACK)
                mensagem_envia(socket, msg_send);
            else if (msg_get.tipo == MSG_ACK && msg_get.sequencia == *seq_c)
                ack_get = 1;
        }
    }

    free(msg_send);

    // aguarda recebimento do mapa
    int mapa = 0;

    while (!mapa)
    {
        if (mensagem_recebe(socket, &msg_get, TIME_OUT_GET) > 0)
        {
            if (crc8_gera(msg_get.dados, msg_get.tamanho) != msg_get.crc)
            {
                struct mensagem_t *nack = mensagem_cria(0, MSG_NACK, NULL, msg_get.sequencia);
                mensagem_envia(socket, nack);
                free(nack);
                continue;
            }

            if (msg_get.tipo == MSG_VISUAL)
            {
                // manda ack - com o mesmo número de sequencia da mensagem
                struct mensagem_t *ack = mensagem_cria(0, MSG_ACK, NULL, msg_get.sequencia);
                mensagem_envia(socket, ack);
                free(ack);

                if (msg_get.sequencia == *seq_s_esperada)
                {
                    // imprime mapa
                    msg_get.dados[msg_get.tamanho] = '\0';
                    printf("\n\n\n%s\n", msg_get.dados);
                    *seq_s_esperada = (*seq_s_esperada + 1) % 64;
                }

                mapa = 1;
            }
        }
    }

    *seq_c = (*seq_c + 1) % 64;
}

void cliente_executa(int socket)
{
    printf("executando em modo cliente\n");

    // validação com base na sequência
    unsigned char seq_c = 0;
    unsigned char seq_s_esperado = 0;

    // inicializando o jogo
    struct mensagem_t *msg_ini = mensagem_cria(0, MSG_INICIO, NULL, seq_c);
    cliente_stop_and_wait(socket, msg_ini, &seq_c, &seq_s_esperado);

    while (1)
    {
        char tecla;
        scanf(" %c", &tecla);

        enum tipo_msg_t tipo_mov;

        switch (tecla)
        {
        case 'w':
            tipo_mov = MSG_MOV_CIMA;
            break;
        case 's':
            tipo_mov = MSG_MOV_BAIXO;
            break;
        case 'a':
            tipo_mov = MSG_MOV_ESQ;
            break;
        case 'd':
            tipo_mov = MSG_MOV_DIR;
            break;
        default:
            tipo_mov = MSG_ERRO;
            break;
        }

        // enviando movimentação
        struct mensagem_t *msg_mov = mensagem_cria(0, tipo_mov, NULL, seq_c);
        cliente_stop_and_wait(socket, msg_mov, &seq_c, &seq_s_esperado);
    }

    printf("jogo finalizado\n");
}
