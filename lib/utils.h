#ifndef UTILS_H__
#define UTILS_H__

#include <stdio.h>
#include "mensagem.h"

#define MODO_CLIENTE 0
#define MODO_SERVIDOR 1

#define MAX_INTERFACES 32
#define MAX_INTERFACE_NOME 32
#define MAC_SIZE 6

#define LOG_MSG 1
#define LOG_TXT 2

// ENUM para os tipos de ações a serem
// identificadas nas mensagens de log
enum action_t
{
    ENVIOU_MSG,
    RECEBEU_MSG,
    ARQUIVO,
};

// imprime o nome de todas as interfaces de rede disponíveis
// RETORNO: quantidade de interfaces disponíveis
int interface_imprime(char interfaces[MAX_INTERFACES][MAX_INTERFACE_NOME]);

// lê do teclado a interface escolhida
// RETORNO: índice da interface
int interface_escolhe(int quantidade);

// cria arquivo de log (log.txt)
// RETORNO: ponteiro para o arquivo de log; NULL se erro
FILE *log_cria();

// escreve em log.txt o que está sendo processado pelo servidor
// RETORNO: void
void log_mensagem(enum action_t acao, struct mensagem_t *msg, char *txt, int tipo);

// abre uma janela para as pastilhas e os arquivos de derrota/vitória
// RETORNO: void
void arquivo_abre(char *arquivo);

#endif