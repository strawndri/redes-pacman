#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../lib/utils.h"
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

    char txt[50];
    snprintf(txt, sizeof(txt), "enviando arquivo %s...", caminho);
    log_mensagem(ARQUIVO, NULL, txt, LOG_TXT);

    // envia nome do arquivo
    struct mensagem_t *msg_nome = mensagem_cria(strlen(nome_arquivo) + 1, tipo, (unsigned char *)nome_arquivo, *seq);
    mensagem_envia_sw(socket, msg_nome, seq);
    free(msg_nome);

    // lendo do arquivo e adicionando ao buffer
    unsigned char buf[MAX_DADOS];
    int total_lido;

    while ((total_lido = fread(buf, 1, sizeof(buf), file)) > 0)
    {   
        int consumido = 0;

        while (consumido < total_lido)
        {
            struct mensagem_t *msg_arquivo = mensagem_cria(0, MSG_DADOS, NULL, *seq);
            consumido += mensagem_preenche_dados(msg_arquivo, buf + consumido, total_lido);
            mensagem_envia_sw(socket, msg_arquivo, seq);
            free(msg_arquivo);
        }
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

    int pac_x = jogo->pacman.x;
    int pac_y = jogo->pacman.y;
    int raio = jogo->raio_visao;

    for (int i = 0; i < TAM_MAPA; i++)
    {
        for (int j = 0; j < TAM_MAPA; j++)
        {
            int distancia_x = j - pac_x;
            int distancia_y = i - pac_y;

            int dentro_raio = (distancia_x >= -raio && distancia_x <= raio) && (distancia_y >= -raio && distancia_y <= raio);

            char casa = dentro_raio ? jogo->mapa[i][j] : VAZIO;
            buf[pos++] = casa;

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

    struct jogo_t jogo;
    jogo_carrega_mapa(caminho_mapa, &jogo);

    unsigned char seq_s = 0;
    unsigned char seq_c_esperada = 0;
    struct mensagem_t msg_get;

    while (1)
    {
        if (mensagem_recebe(socket, &msg_get, TIME_OUT_GET) <= 0)
            continue;

        if (crc8_gera(msg_get.dados, msg_get.tamanho) != msg_get.crc)
        {
            struct mensagem_t *nack = mensagem_cria(0, MSG_NACK, NULL, msg_get.sequencia);
            mensagem_envia(socket, nack);
            free(nack);
            continue;
        }

        if (!(msg_get.tipo == MSG_INICIO || msg_get.tipo == MSG_MOV_BAIXO ||
              msg_get.tipo == MSG_MOV_CIMA || msg_get.tipo == MSG_MOV_DIR ||
              msg_get.tipo == MSG_MOV_ESQ || msg_get.tipo == MSG_ERRO))
            continue;

        struct mensagem_t *ack = mensagem_cria(0, MSG_ACK, NULL, msg_get.sequencia);
        mensagem_envia(socket, ack);
        free(ack);

        if (msg_get.sequencia != seq_c_esperada)
            continue;

        seq_c_esperada = (seq_c_esperada + 1) % 64;

        char pastilha = jogo_move_pacman(&jogo, msg_get.tipo);
        jogo_move_fantasmas(&jogo);
        servidor_envia_mapa(socket, &jogo, &seq_s);

        if (pastilha >= PASTILHA_MIN && pastilha <= PASTILHA_MAX)
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

        if (jogo_verifica_colisao(&jogo))
        {
            servidor_envia_arquivo(socket, "assets/game_over.jpg", MSG_JPG, &seq_s);

            struct mensagem_t *msg_derrota = mensagem_cria(0, MSG_DERROTA, NULL, seq_s);
            mensagem_envia_sw(socket, msg_derrota, &seq_s);
            free(msg_derrota);

            printf("derrota\n");
            break;
        }

        if (jogo_verifica_vitoria(&jogo))
        {
            servidor_envia_arquivo(socket, "assets/win.jpg", MSG_JPG, &seq_s);

            struct mensagem_t *msg_vitoria = mensagem_cria(0, MSG_VITORIA, NULL, seq_s);
            mensagem_envia_sw(socket, msg_vitoria, &seq_s);
            free(msg_vitoria);

            printf("vitória\n");
            break;
        }

        // sinaliza o fim de uma rodada
        struct mensagem_t *msg_fim_rodada = mensagem_cria(0, MSG_FIM_RODADA, NULL, seq_s);
        mensagem_envia_sw(socket, msg_fim_rodada, &seq_s);
        free(msg_fim_rodada);
    }

    close(socket);
    printf("encerrando jogo\n");
}
