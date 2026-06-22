#define _GNU_SOURCE
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "utils.h"

int interface_imprime(char interfaces[MAX_INTERFACES][MAX_INTERFACE_NOME])
{
    struct ifaddrs *ifaddr;
    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    printf("interfaces disponíveis:\n");

    int num = 0;

    // percorre todas as interfaces disponíveis
    for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        // mostra apenas interfaces de rede
        if (ifa->ifa_addr->sa_family != AF_PACKET)
            continue;
        if (strcmp(ifa->ifa_name, "lo") == 0)
            continue;

        printf("  %d: %s\n", num, ifa->ifa_name);
        strcpy(interfaces[num], ifa->ifa_name);
        num++;

        if (num >= MAX_INTERFACES)
            break;
    }

    freeifaddrs(ifaddr);

    return num;
}

int interface_escolhe(int quantidade)
{
    if (quantidade <= 0)
    {
        fprintf(stderr, "nenhuma interface encontrada.\n");
        exit(EXIT_FAILURE);
    }

    if (quantidade == 1)
        return 0;

    int escolha = -1;
    while (escolha < 0 || escolha >= quantidade)
    {
        printf("escolha uma interface: ");
        scanf("%d", &escolha);
    }

    return escolha;
}

FILE *log_cria()
{
    FILE *fd = fopen("log.txt", "w");

    if (!fd)
        return NULL;

    return fd;
}

void log_mensagem(enum action_t acao, struct mensagem_t *msg, char *txt, int tipo)
{
    FILE *fd = fopen("log.txt", "a");
    if (!fd)
        return;

    if ((!msg && tipo == LOG_MSG) || (!txt && tipo == LOG_TXT))
        return;

    const char *nome_acao;

    switch (acao)
    {
        case ENVIOU_MSG:
            nome_acao = "ENVIOU";
            break;
        case RECEBEU_MSG:
            nome_acao = "RECEBEU";
            break;
        case ARQUIVO:
            nome_acao = "ARQUIVO";
            break;
        case ERRO:
            nome_acao = "ERRO";
            break;
        default:
            nome_acao = "";
            break;
    }

    if (tipo == LOG_MSG)
    {
        if (msg->tipo == MSG_ACK)
            fprintf(fd, " -> ACK\n");
        else if (msg->tipo == MSG_NACK)
            fprintf(fd, " -> NACK\n");
        else
            fprintf(fd, "%7s: tipo=%2d seq=%2d tam=%2d crc=%02x",
                    nome_acao, msg->tipo, msg->sequencia, msg->tamanho, msg->crc);
    }
    else if (tipo == LOG_TXT)
        fprintf(fd, "%7s: %s\n", nome_acao, txt);
    
    fclose(fd);
}