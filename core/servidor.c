#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "servidor.h"
#include "../lib/mensagem.h"

void servidor_carrega_csv(char *caminho, struct jogo_t *jogo)
{
    int mapa_padrao = 0;
    FILE *file = fopen(caminho, "r");
    if (!file)
    {
        mapa_padrao = 1;
        file = fopen("assets/mapa_padrao.csv", "r");
        if (!file)
            exit(1);
    }

    char linha[256]; // limite de segurança
    int i = 0;       // linha

    while (fgets(linha, sizeof(linha), file) && i < TAM_MAPA)
    {
        int j = 0; // coluna
        int k = 0; // índice de leitura

        while (linha[k] != '\0' && linha[k] != '\n' && j < TAM_MAPA)
        {
            if (linha[k] != ';')
            {
                jogo->mapa[i][j] = linha[k];

                if (!mapa_padrao && linha[k] == 'P')
                {
                    jogo->pac_x = j;
                    jogo->pac_y = i;
                }
                j++;
            }
            k++;
        }
        i++;
    }
    fclose(file);

    // se usar mapa padrão, implementar aleatoridade
}

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

void servidor_envia_mapa(int socket, char *mapa_str, unsigned char *seq)
{
    int tamanho = strlen(mapa_str);
    int lido = 0;

    printf("enviando visualizacao do mapa\n");

    unsigned char buf[MAX_DADOS];

    int total_lido = (tamanho - lido > MAX_DADOS) ? MAX_DADOS : (tamanho - lido);
    memcpy(buf, mapa_str + lido, total_lido);

    // transferência pelo tipo VISUAL
    struct mensagem_t *msg_visual = mensagem_cria(total_lido, MSG_VISUAL, buf, *seq);
    mensagem_envia_sw(socket, msg_visual, seq);
    free(msg_visual);

    lido += total_lido;

    while (lido < tamanho)
    {
        total_lido = (tamanho - lido > MAX_DADOS) ? MAX_DADOS : (tamanho - lido);
        memcpy(buf, mapa_str + lido, total_lido);

        // tranferência pelo tipo DADOS
        struct mensagem_t *msg_dados = mensagem_cria(total_lido, MSG_DADOS, buf, *seq);
        mensagem_envia_sw(socket, msg_dados, seq);
        free(msg_dados);

        lido += total_lido;
    }

    // indica que o arquivo foi enviado por completo
    struct mensagem_t *fim = mensagem_cria(0, MSG_FIM, NULL, *seq);
    mensagem_envia_sw(socket, fim, seq);
    free(fim);
}

void servidor_visualizacao(char *buffer, struct jogo_t *jogo)
{
    // implementar campo de visão

    int pos = 0;

    for (int i = 0; i < TAM_MAPA; i++)
    {
        for (int j = 0; j < TAM_MAPA; j++)
        {
            buffer[pos++] = jogo->mapa[i][j];
            buffer[pos++] = ' ';
        }
        buffer[pos++] = '\n';
    }
    buffer[pos++] = '\0';
}

void servidor_executa(int socket)
{
    printf("executando em modo servidor\n");

    // inicializando jogo
    struct jogo_t jogo;
    jogo.movimento = 0;

    servidor_carrega_csv("assets/mapa_padrao.csv", &jogo);

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
                    int moveu = 0;
                    int x = jogo.pac_x;
                    int y = jogo.pac_y;
                    pastilha = '0';

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
                        if (x >= 0 && x <= TAM_MAPA && y >= 0 && y <= TAM_MAPA)
                        {
                            char casa_destino = jogo.mapa[y][x];
                            if (casa_destino != 'X')
                            {
                                pastilha = casa_destino;

                                jogo.mapa[jogo.pac_y][jogo.pac_x] = '0';
                                jogo.pac_x = x;
                                jogo.pac_y = y;
                                jogo.mapa[jogo.pac_y][jogo.pac_x] = 'P';
                            }
                        }

                        seq_c_esperada = (seq_c_esperada + 1) % 64;
                    }

                    // tamanho do arquivo do mapa (linhas * colunas) + '\n' + '\0'
                    int tam_mapa = (TAM_MAPA * TAM_MAPA * 2) + TAM_MAPA + 1;
                    char *mapa_str = malloc(tam_mapa * sizeof(char));
                    if (!mapa_str)
                        exit(1);

                    servidor_visualizacao(mapa_str, &jogo);
                    servidor_envia_mapa(socket, mapa_str, &seq_s);

                    free(mapa_str);

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
