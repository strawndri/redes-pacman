#include <stdio.h>

#include "servidor.h"
#include "../lib/mensagem.h"

void servidor_executa(int socket)
{
    printf("executando em modo servidor\n");
    
    struct mensagem_t msg;
    printf("aguardando msg...\n");
    
    mensagem_recebe(socket, &msg);
    mensagem_imprime(&msg);
}
