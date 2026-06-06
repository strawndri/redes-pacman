#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "servidor.h"
#include "jogo.h"

void servidor_envia_arquivo(int socket, char *caminho, enum tipo_msg_t tipo, unsigned char *seq)
{
    FILE *file = fopen(caminho, "rb");
    if (!file)
        return;

    // extrai a pasta e deixa apenas o nome do arquivo
    char *nome_arquivo = strrchr(caminho, '/');
    nome_arquivo = nome_arquivo ? nome_arquivo + 1 : caminho;

    printf("enviando arquivo: %s\n", caminho);

    // envia nome do arquivo
    struct mensagem_t *msg_nome = mensagem_cria(strlen(caminho) + 1, tipo, (unsigned char *)nome_arquivo, *seq);
    mensagem_envia_sw(socket, msg_nome, seq);
    free(msg_nome);

    // lendo do arquivo e adicionando ao buffer
    unsigned char buf[MAX_DADOS];
    int total_lido;
    while ((total_lido = fread(buf, 1, MAX_DADOS, file)) > 0)
    {
        struct mensagem_t *msg_arquivo = msg_arquivo = mensagem_cria(total_lido, MSG_DADOS, buf, *seq);
        mensagem_envia_sw(socket, msg_arquivo, seq);
        free(msg_arquivo);
    }

    // indica que o arquivo foi enviado por completo
    struct mensagem_t *fim = mensagem_cria(0, MSG_FIM, NULL, *seq);
    mensagem_envia_sw(socket, fim, seq);
    free(fim);

    fclose(file);
}

void servidor_envia_mapa(int socket, struct jogo_t *jogo, unsigned char *seq)
{
    unsigned char buf[MAX_DADOS];
    int pos = 0;
    int primeira = 1;

    printf("enviando visualizacao do mapa\n");

    for (int i = 0; i < TAM_MAPA; i++)
    {
        for (int j = 0; j < TAM_MAPA; j++)
        {
            buf[pos++] = jogo->mapa[i][j];

            if (pos == MAX_DADOS)
            {
                struct mensagem_t *msg = mensagem_cria(MAX_DADOS, primeira ? MSG_VISUAL : MSG_DADOS, buf, *seq);
                mensagem_envia_sw(socket, msg, seq);
                free(msg);
                pos = 0;
                primeira = 0;
            }
        }

        buf[pos++] = '\n';

        if (pos == MAX_DADOS)
        {
            struct mensagem_t *msg = mensagem_cria(MAX_DADOS, primeira ? MSG_VISUAL : MSG_DADOS, buf, *seq);
            mensagem_envia_sw(socket, msg, seq);
            free(msg);
            pos = 0;
            primeira = 0;
        }
    }

    if (pos > 0)
    {
        struct mensagem_t *msg = mensagem_cria(pos, primeira ? MSG_VISUAL : MSG_DADOS, buf, *seq);
        mensagem_envia_sw(socket, msg, seq);
        free(msg);
    }

    struct mensagem_t *fim = mensagem_cria(0, MSG_FIM, NULL, *seq);
    mensagem_envia_sw(socket, fim, seq);
    free(fim);
}

void servidor_executa(int socket, char *caminho_mapa)
{
    printf("executando em modo servidor\n");

    // inicializando jogo
    struct jogo_t jogo;
    jogo_carrega_mapa(caminho_mapa, &jogo);

    unsigned char seq_s = 0;
    unsigned char seq_c_esperada = 0;
    struct mensagem_t msg_get;

    char pastilha;

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

            if (msg_get.tipo == MSG_INICIO || msg_get.tipo == MSG_MOV_BAIXO ||
                msg_get.tipo == MSG_MOV_CIMA || msg_get.tipo == MSG_MOV_DIR ||
                msg_get.tipo == MSG_MOV_ESQ || msg_get.tipo == MSG_ERRO)
            {
                // manda ack - com o mesmo número de sequencia da mensagem
                struct mensagem_t *ack = mensagem_cria(0, MSG_ACK, NULL, msg_get.sequencia);
                mensagem_envia(socket, ack);
                free(ack);

                if (msg_get.sequencia == seq_c_esperada)
                {
                    pastilha = jogo_move_pacman(&jogo, msg_get.tipo);
                    jogo_move_fantasmas(&jogo);

                    seq_c_esperada = (seq_c_esperada + 1) % 64;

                    servidor_envia_mapa(socket, &jogo, &seq_s);

                    if (pastilha >= '1' && pastilha <= '6')
                    {
                        char caminho[32];
                        if (pastilha == '1' || pastilha == '2')
                        {
                            sprintf(caminho, "assets/%c.txt", pastilha);
                            servidor_envia_arquivo(socket, caminho, MSG_TXT, &seq_s);
                        }
                        else if (pastilha == '3' || pastilha == '4')
                        {
                            sprintf(caminho, "assets/%c.jpg", pastilha);
                            servidor_envia_arquivo(socket, caminho, MSG_JPG, &seq_s);
                        }
                        else if (pastilha == '5' || pastilha == '6')
                        {
                            sprintf(caminho, "assets/%c.mp4", pastilha);
                            servidor_envia_arquivo(socket, caminho, MSG_MP4, &seq_s);
                        }
                    }
                }
            }
        }
    }
}
