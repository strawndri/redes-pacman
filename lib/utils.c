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
