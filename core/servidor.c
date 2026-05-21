#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "servidor.h"
#include "../lib/mensagem.h"

void servidor_envia_arquivo(int socket, char *caminho,
    enum tipo_msg_t tipo, unsigned char *seq)
{
    FILE *file = fopen(caminho, "rb");
    if (!file)
        return;

    // extrai a pasta e deixa apenas o nome do arquivo
    char *nome_arquivo = strrchr(caminho, '/');
    nome_arquivo = nome_arquivo ? nome_arquivo + 1 : caminho;

    unsigned char buf[MAX_DADOS];

    int total_lido;

    printf("enviando arquivo: %s\n", caminho);

    // envia nome do arquivo
    struct mensagem_t *msg_nome = mensagem_cria(strlen(caminho) + 1, tipo, nome_arquivo, *seq);

    int ack_get = 0;
    struct mensagem_t resp;

    while (!ack_get)
    {
        mensagem_envia(socket, msg_nome);

        if (mensagem_recebe(socket, &resp, TIME_OUT_SEND) > 0)
        {
            if (resp.tipo == MSG_NACK)
                continue;
            if (resp.tipo == MSG_ACK && resp.sequencia == *seq)
                ack_get = 1;
        }
    }

    free(msg_nome);
    *seq = (*seq + 1) % 64;
    
    // lendo do arquivo e adicionando ao buffer
    while ((total_lido = fread(buf, 1, MAX_DADOS, file)) > 0)
    {   
        struct mensagem_t *msg_arquivo = msg_arquivo = mensagem_cria(total_lido, MSG_DADOS, buf, *seq);    

        ack_get = 0;

        while (!ack_get)
        {
            mensagem_envia(socket, msg_arquivo);

            if (mensagem_recebe(socket, &resp, TIME_OUT_SEND) > 0)
            {
                if (resp.tipo == MSG_NACK)
                    continue;
                if (resp.tipo == MSG_ACK && resp.sequencia == *seq)
                    ack_get = 1;
            }
        }

        free(msg_arquivo);
        *seq = (*seq + 1) % 64;
    }

    // indica que o arquivo foi enviado por completo
    struct mensagem_t *fim = mensagem_cria(0, MSG_FIM, NULL, *seq);
    ack_get = 0;

    while (!ack_get)
    {
        mensagem_envia(socket, fim);
        if (mensagem_recebe(socket, &resp, TIME_OUT_SEND) > 0)
            if (resp.tipo == MSG_ACK && resp.sequencia == *seq)
                ack_get = 1;
    }

    free(fim);
    *seq = (*seq + 1) % 64;

    fclose(file);
}


void servidor_executa(int socket)
{
    printf("executando em modo servidor\n");

    // validação com base na sequência
    unsigned char seq_s = 0;
    unsigned char seq_c_esperada = 0;

    char mapa_teste[3][3] = {
        {'0', '4', '0'},
        {'1', 'P', '3'},
        {'0', '2', '0'}};

    int pac_x = 1;     // coluna
    int pac_y = 1;     // linha
    char mapa_str[16]; // ira enviar o mapa
    char pastilha;

    struct mensagem_t msg_get;

    while (1)
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

            if (msg_get.tipo == MSG_INICIO ||
                msg_get.tipo == MSG_MOV_BAIXO ||
                msg_get.tipo == MSG_MOV_CIMA ||
                msg_get.tipo == MSG_MOV_DIR ||
                msg_get.tipo == MSG_MOV_ESQ ||
                msg_get.tipo == MSG_ERRO)
            {
                // manda ack - com o mesmo número de sequencia da mensagem
                struct mensagem_t *ack = mensagem_cria(0, MSG_ACK, NULL, msg_get.sequencia);
                mensagem_envia(socket, ack);
                free(ack);

                if (msg_get.sequencia == seq_c_esperada)
                {
                    int moveu = 0;
                    int x = pac_x;
                    int y = pac_y;
                    pastilha = 0;
                    
                    switch (msg_get.tipo)
                    {
                    case MSG_INICIO:
                        printf("iniciando jogo - mapa\n");
                        moveu = 1;
                        break;

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

                    case MSG_ERRO:
                        printf("tecla inválida\n");
                        break;

                    default:
                        break;
                    }

                    if (moveu)
                    {
                        if (x >= 0 && x <= 2 && y >= 0 && y <= 2)
                        {
                            // pacman pegou a pastila
                            pastilha = mapa_teste[y][x];
        
                            mapa_teste[pac_y][pac_x] = '0';
                            pac_x = x;
                            pac_y = y;
                            mapa_teste[pac_y][pac_x] = 'P';
                        }
    
                        seq_c_esperada = (seq_c_esperada + 1) % 64;
                    }

                }

                sprintf(mapa_str, "%c%c%c\n%c%c%c\n%c%c%c",     
                        mapa_teste[0][0], mapa_teste[0][1], mapa_teste[0][2],
                        mapa_teste[1][0], mapa_teste[1][1], mapa_teste[1][2],
                        mapa_teste[2][0], mapa_teste[2][1], mapa_teste[2][2]);


                struct mensagem_t *msg_send = mensagem_cria(strlen(mapa_str), MSG_VISUAL, (unsigned char *)mapa_str, seq_s);
                int ack_get = 0;
                struct mensagem_t msg_get;

                while (!ack_get)
                {
                    mensagem_envia(socket, msg_send);

                    if (mensagem_recebe(socket, &msg_get, TIME_OUT_SEND) > 0)
                    {
                        if (msg_get.tipo == MSG_NACK)
                            mensagem_envia(socket, msg_send);
                        if (msg_get.tipo == MSG_ACK && msg_get.sequencia == seq_s)
                            ack_get = 1;
                    }
                }

                free(msg_send);
                seq_s = (seq_s + 1) % 64;

                if (pastilha == '1' || pastilha == '2')
                {
                    char caminho[32];
                    sprintf(caminho, "assets/%c.txt", pastilha);
                    servidor_envia_arquivo(socket, caminho, MSG_TXT, &seq_s);
                }
                else if (pastilha == '3' || pastilha == '4')
                {
                    char caminho[32];
                    sprintf(caminho, "assets/%c.jpg", pastilha);
                    servidor_envia_arquivo(socket, caminho, MSG_JPG, &seq_s);
                }
                else if (pastilha == '5' || pastilha == '6')
                {
                    char caminho[32];
                    sprintf(caminho, "assets/%c.mp4", pastilha);
                    servidor_envia_arquivo(socket, caminho, MSG_MP4, &seq_s);
                }
            }
        }
    }
}
