#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mensagem.h"

struct mensagem_t *mensagem_cria(unsigned char tamanho,
                                 enum tipo_msg_t tipo, unsigned char *dados)
{
    struct mensagem_t *msg;
    msg = malloc(sizeof(struct mensagem_t));

    msg->marcador_inicio = MSG_MARCADOR_INICIO;
    msg->tamanho = tamanho;
    msg->sequencia = 0; // TODO: calcular depois
    msg->tipo = tipo;
    msg->crc = 0; // TODO: calcular depois
    strcpy(msg->dados, dados);

    return msg;
}

void mensagem_imprime(struct mensagem_t *msg)
{
    if (!msg)
        return;

    printf("mensagem: %d %d %d %d %d %d %s", msg->marcador_inicio, msg->tamanho,
           msg->sequencia, msg->tipo, msg->crc, msg->dados);
}
