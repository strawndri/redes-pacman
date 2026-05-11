#include <stdio.h>
#include <stdlib.h>

#include "cliente.h"
#include "../lib/mensagem.h"

void cliente_executa(int socket)
{
    printf("executando em modo cliente\n");
    char tecla;

    while (1)
    {
        printf("Digite 'w/a/s/d' para se movimentar ou 'q' para encerrar: ");
        scanf(" %c", &tecla);

        enum tipo_msg_t tipo;
        int fim = 0;

        switch (tecla)
        {
        case 'w':
            tipo = MSG_MOV_CIMA;
            break;
        case 's':
            tipo = MSG_MOV_BAIXO;
            break;
        case 'a':
            tipo = MSG_MOV_ESQ;
            break;
        case 'd':
            tipo = MSG_MOV_DIR;
            break;
        case 'q':
            tipo = MSG_FIM;
            fim = 1;
            break;
        default:
            printf("Tecla inválida!\n");
            continue;
        }

        unsigned char dados[] = "move";
        struct mensagem_t *msg = mensagem_cria(sizeof(dados), tipo, dados);

        if (mensagem_envia(socket, msg) >= 0)
            printf("comando %d enviado\n", tipo);
        else
            printf("mensagem não enviada.\n");

        free(msg);

        if (fim)
        {
            break;
        }
    }
}
