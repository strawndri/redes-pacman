#include <stdio.h>

#include "servidor.h"
#include "../lib/mensagem.h"

void servidor_executa(int socket)
{
    printf("executando em modo servidor\n");

    struct mensagem_t msg;

    while (1)
    {
        printf("aguardando msg...\n");

        int status = mensagem_recebe(socket, &msg);

        if (status > 0)
        {
            // verificação crc
            unsigned char crc = crc8_gera(msg.dados, msg.tamanho);
            if (crc != msg.crc)
            {
                printf("Erro CRC\n"); // por hora apenas ignoramos (teste)
                continue;
            }
            switch (msg.tipo)
            {
            // movimentação:
            case MSG_MOV_CIMA:
            case MSG_MOV_BAIXO:
            case MSG_MOV_DIR:
            case MSG_MOV_ESQ:
                mensagem_imprime(&msg);
                break;

            // fim:
            case MSG_FIM:
                mensagem_imprime(&msg);
                return;

            // erro:
            default:
                printf("erro");
                break;
            }
        }
    }
}
