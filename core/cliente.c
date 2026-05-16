#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cliente.h"
#include "../lib/mensagem.h"

// TODO: atualmente apenas para o mapa
unsigned char cliente_stop_and_wait(int socket, struct mensagem_t *msg_send, int seq_c)
{
    // TODO: envia mensagem
    int ack_get = 0;
    struct mensagem_t msg_get;

    while (!ack_get)
    {
        mensagem_envia(socket, msg_send);

        if (mensagem_recebe(socket, &msg_get, TIME_OUT_SEND) > 0)
        {
            if (msg_get.tipo == MSG_ACK && msg_get.sequencia == seq_c)
            {
                printf("Retornou ACK para meu pedido\n");
                ack_get = 1;
            }
        }
    }

    free(msg_send);

    // TODO: aguarda recebimento do mapa
    int mapa = 0;

    while (!mapa)
    {
        if (mensagem_recebe(socket, &msg_get, TIME_OUT_GET) > 0)
        {
            // TODO: enviar nack
            if (crc8_gera(msg_get.dados, msg_get.tamanho) != msg_get.crc)
                continue;

            if (msg_get.tipo == MSG_VISUAL)
            {
                // TODO: manda ack - com o mesmo número de sequencia da mensagem
                struct mensagem_t *ack = mensagem_cria(0, MSG_ACK, NULL);
                ack->sequencia = msg_get.sequencia;
                mensagem_envia(socket, ack);
                free(ack);

                // TODO: imprime mapa
                msg_get.dados[msg_get.tamanho] = '\0';
                printf("\n\n\n%s\n", msg_get.dados);
                mapa = 1;
            }
        }
    }

    // TODO: fazer tratativa do loop - sequência
    return seq_c + 1;
}

void cliente_executa(int socket)
{
    printf("executando em modo cliente\n");

    // TODO: sequência das mensagens do cliente - Precisa implementar
    unsigned char seq_c = 0;

    // TODO: inicializando o jogo
    struct mensagem_t *msg_ini = mensagem_cria(0, MSG_INICIO, NULL);
    msg_ini->sequencia = seq_c;

    seq_c = cliente_stop_and_wait(socket, msg_ini, seq_c);

    while (1)
    {
        char tecla;
        scanf(" %c", &tecla);

        enum tipo_msg_t tipo_mov;
        int fim = 0;

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
            continue;
        }

        if (fim)
            break;

        struct mensagem_t *msg_mov = mensagem_cria(0, tipo_mov, NULL);
        msg_mov->sequencia = seq_c;

        seq_c = cliente_stop_and_wait(socket, msg_mov, seq_c);
    }

    printf("jogo finalizado\n");
}
