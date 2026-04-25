#include <linux/if.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "raw_socket.h"

int create_raw_socket(char *device)
{
    int socket;
    struct ifreq ir;
    struct sockaddr_ll socket_address;
    struct packet_mreq mr;

    // cria socket
    // captura todos os protocolos Ethernet
    socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (socket == -1)
    {
        printf("Erro ao criar socket.\n");
        exit(-1);
    }

    // obtém o índice da interface de rede (eth0)
    memset(&ir, 0, sizeof(struct ifreq));
    memcpy(ir.ifr_name, device, sizeof(device));
    if (ioctl(socket, SIOCGIFINDEX, &ir) == -1)
    {
        printf("Erro no ioctl.\n");
        exit(-1);
    }

    // configura o endereço do socket para associar à interface
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_protocol = htons(ETH_P_ALL);
    socket_address.sll_ifindex = ir.ifr_ifindex;

    // associa o socket à interface de rede
    if (bind(socket, (struct sockaddr *)&socket_address, sizeof(socket_address)) == -1)
    {
        printf("Erro no bind\n");
        exit(-1);
    }

    // modo promiscuo
    memset(&mr, 0, sizeof(mr));
    mr.mr_ifindex = ir.ifr_ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    if (setsockopt(socket, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)
    {
        printf("Erro ao fazer setsockopt\n");
        exit(-1);
    }

    return socket;
}
