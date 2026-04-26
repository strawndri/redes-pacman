#include <stdio.h>
#include <stdlib.h>

#include "cliente.h"
#include "../lib/mensagem.h"

void cliente_executa(int socket)
{
    printf("executando em modo cliente\n");
    
    unsigned char dados[] = "testando";
    struct mensagem *msg = mensagem_cria(sizeof(dados), MSG_INICIO, dados);
    
    if (mensagem_envia(socket, msg) == 0)
        printf("mensagem enviada.\n");
    else
        printf("mensagem não enviada.\n");
    
    free(msg);
}
