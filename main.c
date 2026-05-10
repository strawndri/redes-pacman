#include <stdio.h>

#include "lib/utils.h"
#include "lib/raw_socket.h"
#include "core/cliente.h"
#include "core/servidor.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    
    if (argv[1][0] != '-')
        return 1;
    
    char modo = argv[1][1]; // servidor ou cliente
    int modo_opcao;
    
    if (modo == 's')
        modo_opcao = MODO_SERVIDOR;
    else if (modo == 'c')
        modo_opcao = MODO_CLIENTE;
    else
    {
        printf("opção inválida.\n");
        return 1;
    }
    
    char nome_interfaces[MAX_INTERFACES][MAX_INTERFACE_NOME];
    int qtd, i, socket;
    
    qtd = interface_imprime(nome_interfaces);
    i = interface_escolhe(qtd);
    
    printf("criando socket para a interface %s...\n", nome_interfaces[i]);
    
    socket = raw_socket_cria(nome_interfaces[i]);
    
    if (modo_opcao == MODO_SERVIDOR)
        servidor_executa(socket);
    else
        cliente_executa(socket);
    
    return 0;
}