#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cliente.h"
#include "../lib/mensagem.h"

void cliente_inicializacao(int socket)
{
    printf("solicitando mapa para o servidor\n");

    struct mensagem_t *msg_ini = mensagem_cria(0, MSG_INICIO, NULL);
    mensagem_envia(socket, msg_ini);
    free(msg_ini);

    struct mensagem_t resp_ini;
    while (1)
    {
        if (mensagem_recebe(socket, &resp_ini) > 0)
        {
            if (crc8_gera(resp_ini.dados, resp_ini.tamanho) != resp_ini.crc)
            {
                struct mensagem_t *nack = mensagem_cria(0, MSG_NACK, NULL);
                if (mensagem_envia(socket, nack) >= 0)
                    printf("nack enviado\n");
                else
                    printf("nack não enviada\n");

                free(nack);
                continue;
            }

            if (resp_ini.tipo == MSG_VISUAL)
            {
                printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
                printf("mapa incial\n");
                printf("%s\n", resp_ini.dados);
                break;
            }
        }
    }
}

void cliente_envia_mov(int socket, enum tipo_msg_t mov)
{
    struct mensagem_t *msg_mov = mensagem_cria(0, mov, NULL);
    if (mensagem_envia(socket, msg_mov) >= 0)
        printf("movimento enviado\n");
    else
        printf("mensagem não enviada\n");

    free(msg_mov);
}

void cliente_executa(int socket)
{
    printf("executando em modo cliente\n");

    // [0] inicializando jogo -> pedindo para o servidor
    cliente_inicializacao(socket);

    // [1] implementando método stop-and-wait
    while (1)
    {
        // [2] leitura do teclado (movimentação)
        char tecla;
        printf("Digite 'w/a/s/d' para se movimentar ou 'q' para encerrar: \n");
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
        case 'q':
            tipo_mov = MSG_FIM;
            fim = 1;
            break;
        default:
            printf("tecla inválida\n");
            continue;
        }

        // [3] envia mensagem de movimento para o servidor
        cliente_envia_mov(socket, tipo_mov);

        if (fim)
            break;

        // [4] aguardando a resposta
        int valido = 0;
        struct mensagem_t msg_resp;

        while (!valido)
        {
            if (mensagem_recebe(socket, &msg_resp) > 0)
            {
                if (crc8_gera(msg_resp.dados, msg_resp.tamanho) != msg_resp.crc)
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

                if (msg_resp.tipo == MSG_VISUAL)
                {
                    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
                    msg_resp.dados[msg_resp.tamanho] = '\0';
                    printf("%s\n", msg_resp.dados);
                    valido = 1;
                }
            }
        }
    }

    printf("jogo finalizado\n");
}
