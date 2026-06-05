#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "cliente.h"
#include "../lib/mensagem.h"

// aux - leitura de teclas no terminal
void desliga_modo_jogo()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &tecla_original);
}

void liga_modo_jogo()
{
    tcgetattr(STDIN_FILENO, &tecla_original);
    atexit(desliga_modo_jogo);
    struct termios tecla_jogo = tecla_original;
    tecla_jogo.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &tecla_jogo);
}

// funções principais

void cliente_recebe_mapa(int socket, unsigned char *seq_s_esperada)
{
    struct mensagem_t msg_get;
    int recebendo_mapa = 1;
    char *buf = NULL;
    int tam_atual = 0;

    while (recebendo_mapa)
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

            struct mensagem_t *ack = mensagem_cria(0, MSG_ACK, NULL, msg_get.sequencia);
            mensagem_envia(socket, ack);
            free(ack);

            if (msg_get.sequencia != *seq_s_esperada)
                continue;

            *seq_s_esperada = (*seq_s_esperada + 1) % 64;

            if (msg_get.tipo == MSG_VISUAL || msg_get.tipo == MSG_DADOS)
            {
                // aumenta memória para salvar o mapa
                char *temp = realloc(buf, tam_atual + msg_get.tamanho + 1);
                if (temp == NULL)
                {
                    free(buf);
                    exit(1);
                }
                buf = temp;

                memcpy(buf + tam_atual, msg_get.dados, msg_get.tamanho);
                tam_atual += msg_get.tamanho;
            }
            else if (msg_get.tipo == MSG_FIM)
            {
                if (buf)
                {
                    buf[tam_atual] = '\0'; // termina a string

                    printf("\x1b[H"); // reescreve por cima do terminal

                    // imprime mapa
                    for (int i = 0; i < tam_atual; i++)
                    {
                        switch (buf[i])
                        {
                        case 'X':
                            printf("▒▒");
                            break;
                        case '0':
                            printf("  ");
                            break;
                        case 'P':
                            printf("\x1b[1;33mᗧ \x1b[0m");
                            break;
                        case 'R':
                            printf("📕");
                            break;
                        case 'B':
                            printf("📘");
                            break;
                        case 'G':
                            printf("📗");
                            break;
                        case 'Y':
                            printf("📔");
                            break;
                        case '\n':
                            printf("\n");
                            break;
                        default:
                            if (buf[i] >= '1' && buf[i] <= '6')
                                printf("🪙");
                            else if (buf[i] != ' ')
                                printf("%c ", buf[i]);
                            break;
                        }
                    }
                    printf("\n");

                    free(buf);
                }
                recebendo_mapa = 0;
            }
        }
    }
}

void cliente_recebe_arquivo(int socket, unsigned char *seq_s_esperada)
{
    struct mensagem_t msg_get;
    FILE *arquivo = NULL;
    int recebendo_arquivo = 1;

    while (recebendo_arquivo)
    {
        // deu timeout -> nenhum arquivo foi enviado
        if (mensagem_recebe(socket, &msg_get, TIME_OUT_SEND) <= 0)
            break;

        if (crc8_gera(msg_get.dados, msg_get.tamanho) != msg_get.crc)
        {
            struct mensagem_t *nack = mensagem_cria(0, MSG_NACK, NULL, msg_get.sequencia);
            mensagem_envia(socket, nack);
            free(nack);
            continue;
        }

        struct mensagem_t *ack = mensagem_cria(0, MSG_ACK, NULL, msg_get.sequencia);
        mensagem_envia(socket, ack);
        free(ack);

        if (msg_get.sequencia != *seq_s_esperada)
            continue;

        *seq_s_esperada = (*seq_s_esperada + 1) % 64;

        // cria arquivo
        if ((msg_get.tipo == MSG_TXT || msg_get.tipo == MSG_JPG || msg_get.tipo == MSG_MP4) && !arquivo)
            arquivo = fopen((const char *)msg_get.dados, "wb");

        // escreve arquivo
        if ((msg_get.tipo == MSG_DADOS) && arquivo)
            fwrite(msg_get.dados, 1, msg_get.tamanho, arquivo);

        // acabou o arquivo
        if (msg_get.tipo == MSG_FIM)
        {
            if (arquivo)
            {
                fclose(arquivo);
                arquivo = NULL;
                printf("pastilha recebida\n");
            }
            recebendo_arquivo = 0;
        }
    }
}

void cliente_executa(int socket)
{
    printf("executando em modo cliente\n");

    liga_modo_jogo();

    unsigned char seq_c = 0;
    unsigned char seq_s_esperado = 0;

    // inicialização
    struct mensagem_t *msg_ini = mensagem_cria(0, MSG_INICIO, NULL, seq_c);
    mensagem_envia_sw(socket, msg_ini, &seq_c);
    free(msg_ini);

    system("clear");

    cliente_recebe_mapa(socket, &seq_s_esperado);
    cliente_recebe_arquivo(socket, &seq_s_esperado);

    while (1)
    {
        // garante que leia apenas a tecla selecionada no momento
        tcflush(STDIN_FILENO, TCIFLUSH);

        char tecla = getchar();

        enum tipo_msg_t tipo_mov;

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
            printf("cliente finalizou o jogo\n");
            desliga_modo_jogo();
            return;
        default:
            tipo_mov = MSG_ERRO;
            break;
        }

        if (tipo_mov == MSG_ERRO)
            continue;

        // envia movimentação
        struct mensagem_t *msg_mov = mensagem_cria(0, tipo_mov, NULL, seq_c);
        mensagem_envia_sw(socket, msg_mov, &seq_c);
        free(msg_mov);

        // mapa atualizado e pastilha
        cliente_recebe_mapa(socket, &seq_s_esperado);
        cliente_recebe_arquivo(socket, &seq_s_esperado);
    }

    printf("jogo finalizado\n");
}