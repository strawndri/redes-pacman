#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "mensagem.h"
#include "utils.h"

long long timestamp()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

struct mensagem_t *mensagem_cria(unsigned char tamanho,
                                 enum tipo_msg_t tipo, unsigned char *dados, unsigned char seq)
{
    struct mensagem_t *msg;
    msg = malloc(sizeof(struct mensagem_t));

    msg->marcador_inicio = MSG_MARCADOR_INICIO;
    msg->tamanho = tamanho;
    msg->sequencia = seq;
    msg->tipo = tipo;
    msg->crc = 0;

    // envio de mesagens que possuem dados
    if (tamanho > 0 && dados != NULL)
        mensagem_preenche_dados(msg, dados, tamanho);
    else
    {
        msg->tamanho = 0;
        memset(msg->dados, 0, MAX_DADOS);
    }

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

    unsigned char *buffer = malloc(sizeof(struct mensagem_t));
    memcpy(buffer, msg, sizeof(struct mensagem_t));

    return buffer;
}

int mensagem_envia(int socket, struct mensagem_t *msg)
{
    if (!msg)
        return -1;

    msg->crc = crc8_gera(msg->dados, msg->tamanho);
    unsigned char *buffer = mensagem_serializa(msg);

    int bytes_enviados = send(socket, buffer, sizeof(struct mensagem_t), 0);

    free(buffer);

    log_mensagem(ENVIOU_MSG, msg, NULL, LOG_MSG);

    return bytes_enviados;
}

int mensagem_recebe(int socket, struct mensagem_t *msg, int timeoutMillis)
{
    if (!msg)
        return -1;

    long long comeco = timestamp();
    struct timeval timeout = {.tv_sec = timeoutMillis / 1000, .tv_usec = (timeoutMillis % 1000) * 1000};
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

    unsigned char buffer[sizeof(struct mensagem_t)];
    
    do
    {
        int bytes_lidos = recv(socket, buffer, sizeof(buffer), 0);
        if (bytes_lidos > 0)
        {
            // validação de início
            if (buffer[0] == MSG_MARCADOR_INICIO)
            {
                // copia os dados do buffer diretamente para a struct
                memcpy(msg, buffer, sizeof(struct mensagem_t));
                log_mensagem(RECEBEU_MSG, msg, NULL, LOG_MSG);
                return (int)bytes_lidos;
            }

        }
    } while (timestamp() - comeco <= timeoutMillis);

    return -1;
}

void mensagem_envia_sw(int socket, struct mensagem_t *msg, unsigned char *seq)
{
    int ack_get = 0;
    struct mensagem_t msg_resp;

    while (!ack_get)
    {
        mensagem_envia(socket, msg);

        if (mensagem_recebe(socket, &msg_resp, TIME_OUT_SEND) > 0)
        {
            if (msg_resp.tipo == MSG_NACK)
                continue;

            if (msg_resp.tipo == MSG_ACK && msg_resp.sequencia == *seq)
                ack_get = 1;
        }
    }

    *seq = (*seq + 1) % 64;
}

int mensagem_preenche_dados(struct mensagem_t *msg, unsigned char *dados, int tamanho)
{
    int n = 0; // bytes escritos em msg->dados;
    int i = 0; // bytes lidos de fato

    while (i < tamanho && n < MAX_DADOS)
    {
        msg->dados[n++] = dados[i];

        if ((dados[i] == 0x88 || dados[i] == 0x81) && n < MAX_DADOS)
            msg->dados[n++] = 0xff;

        i++;
    }

    msg->tamanho = (unsigned char)n;
    if (n < MAX_DADOS)
        memset(msg->dados + n, 0, MAX_DADOS - n);

    return i;
}