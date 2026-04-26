#ifndef UTILS_H__
#define UTILS_H__

#define MODO_CLIENTE 0
#define MODO_SERVIDOR 1

#define MAX_INTERFACES 32
#define MAX_INTERFACE_NOME 32
#define MAC_SIZE 6

// imprime o nome de todas as interfaces de rede disponíveis
// RETORNO: quantidade de interfaces disponíveis
int interface_imprime(char interfaces[MAX_INTERFACES][MAX_INTERFACE_NOME]);

// lê do teclado a interface escolhida
// RETORNO: índice da interface
int interface_escolhe(char interfaces[MAX_INTERFACES][MAX_INTERFACE_NOME], int quantidade);

void interface_mac(char *interface, unsigned char mac[MAC_SIZE]);

#endif