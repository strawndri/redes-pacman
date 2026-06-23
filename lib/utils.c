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

// garante que apenas o servidor tenha a capacidade
// de escrever no arquivo de log
static int log_ativo = 0;

int interface_imprime(char interfaces[MAX_INTERFACES][MAX_INTERFACE_NOME])
{
    struct ifaddrs *ifaddr;
    int num;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    printf("interfaces disponíveis:\n");

    num = 0;
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

    int escolha;

    if (quantidade == 1)
        return 0;

    escolha = -1;
    while (escolha < 0 || escolha >= quantidade)
    {
        printf("escolha uma interface: ");
        scanf("%d", &escolha);
    }

    return escolha;
}

FILE *log_cria()
{   
    // log habilitado para o servidor
    log_ativo = 1;
    FILE *file;
    file = fopen("log.txt", "w");

    if (!file)
        return NULL;

    return file;
}

void log_mensagem(enum action_t acao, struct mensagem_t *msg, char *txt, int tipo)
{   
    // log não está habilitado
    if (!log_ativo)
        return;
    
    FILE *file;
    const char *nome_acao;
    
    file = fopen("log.txt", "a");
    if (!file)
        return;

    if ((!msg && tipo == LOG_MSG) || (!txt && tipo == LOG_TXT))
        return;


    // cria uma string conforme o tipo de ação disponível
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
        default:
            nome_acao = "";
            break;
    }

    if (tipo == LOG_MSG)
    {
        if (msg->tipo == MSG_ACK)
            fprintf(file, " -> ACK");
        else if (msg->tipo == MSG_NACK)
            fprintf(file, " -> NACK");
        else
        {
            fprintf(file, "\n");
            fprintf(file, "%7s: tipo=%2d seq=%2d tam=%2d crc=%02x",
                    nome_acao, msg->tipo, msg->sequencia, msg->tamanho, msg->crc);
        }
    }
    else if (tipo == LOG_TXT)
    {
        fprintf(file, "\n");
        fprintf(file, "%7s: %s\n", nome_acao, txt);
    }
    
    fclose(file);
}

void arquivo_abre(char *arquivo)
{   
    char command[512];
    const char *sudo_user;
    
    sudo_user = getenv("SUDO_USER");
    if (sudo_user == NULL) {
        sudo_user = getenv("USER");
    }

    // usa o comando xdg-open para abrir uma janela com o arquivo
    // faz isso com sudo + variáveis de ambiente
    snprintf(command, sizeof(command),
             "sudo -u %s PULSE_SERVER=unix:/run/user/$(id -u %s)/pulse/native "
             "xdg-open %s > /dev/null 2>&1 &",
             sudo_user, sudo_user, arquivo);
    system(command);
}