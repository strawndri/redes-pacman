#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

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
    memcpy(msg->dados, dados, tamanho);

    return msg;
}

unsigned char crc8_gera(unsigned char *dados, unsigned char tamanho)
{
    unsigned char crc = 0xff;
    unsigned char i, j;

    for (i = 0; i < tamanho; i++)
    {
        crc ^= dados[i];
        for (j = 0; j < 8; j++)
        {
            if ((crc & 0x80) != 0)
                crc = (unsigned char)((crc << 1) ^ 0x31);
            else
                crc <<= 1;
        }
    }

    return crc;
}

unsigned char *mensagem_serializa(struct mensagem_t *msg)
{
    if (!msg)
        return NULL;

    unsigned char *buffer;
    int offset = 0;

    buffer = malloc(sizeof(struct mensagem_t));

    memcpy(buffer + offset, &msg->marcador_inicio, 1);
    offset++;
    memcpy(buffer + offset, &msg->tamanho, 1);
    offset++;

    memcpy(buffer + offset, &msg->sequencia, 1);
    offset++;

    memcpy(buffer + offset, &msg->tipo, 1);
    offset++;

    memcpy(buffer + offset, msg->dados, MAX_DADOS);
    offset += MAX_DADOS;

    memcpy(buffer + offset, &msg->crc, 1);

    return buffer;
}

int mensagem_envia(int socket, struct mensagem_t *msg)
{
    if (!msg)
        return -1;

    // TODO: enviar o buffer pelo socket
    msg->crc = crc8_gera(msg->dados, msg->tamanho);
    
    // TODO: serializar a mensagem em um buffer
    unsigned char *buffer = mensagem_serializa(msg);
    printf("%s %ld\n", buffer, sizeof(buffer));

    // TODO: implementar para-e-espera
    int bytes_enviados = send(socket, buffer, sizeof(struct mensagem_t), 0);

    mensagem_imprime(msg);

    return bytes_enviados;
}

int mensagem_recebe(int socket, struct mensagem_t *msg)
{
    if (!msg)
        return -1;
    
    unsigned char buffer[sizeof(struct mensagem_t)];
    
    unsigned char bytes_lidos = recv(socket, buffer, sizeof(buffer), 0);
    
    if (bytes_lidos <= 0)
        return -1;
    
    int offset = 0;
    msg->marcador_inicio = buffer[offset++];
    msg->tamanho = buffer[offset++];
    msg->sequencia = buffer[offset++];
    msg->tipo = buffer[offset++];
    
    memcpy(msg->dados, buffer + offset, MAX_DADOS);
    offset += MAX_DADOS;
    
    msg->crc = buffer[offset];
    
    if (msg->marcador_inicio != MSG_MARCADOR_INICIO)
        return 0;
    
    return (int)bytes_lidos;
}

void mensagem_imprime(struct mensagem_t *msg)
{
    if (!msg)
        return;

    printf("mensagem: %d %d %d %d %d %s\n", msg->marcador_inicio, msg->tamanho,
           msg->sequencia, msg->tipo, msg->crc, msg->dados);
}
