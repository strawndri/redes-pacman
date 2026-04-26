#ifndef MENSAGEM_H__
#define MENSAGEM_H__

#define MSG_MARCADOR_INICIO 0b01111110
#define MAX_DADOS 32

enum tipo_msg_t
{
    MSG_ACK = 0,
    MSG_NACK = 1,
    MSG_VISUAL = 2,
    MSG_INICIO = 3,
    MSG_DADOS = 4,
    MSG_TXT = 5,
    MSG_JPG = 6,
    MSG_MP4 = 7,
    MSG_MOV_DIR = 10,
    MSG_MOV_ESQ = 11,
    MSG_MOV_CIMA = 12,
    MSG_MOV_BAIXO = 13,
    MSG_ERRO = 15,
    MSG_FIM = 16
};

// estrutura que define uma mensagem
struct mensagem_t
{
    unsigned char marcador_inicio;    // 01111110
    unsigned char tamanho;            // tamanho da região de dados (0-31)
    unsigned char sequencia;          // número da sequência (0-63)
    enum tipo_msg_t tipo;             // tipo da mensagem
    unsigned char dados[MAX_DADOS];   // área de dados
    unsigned char crc;                // método de detecção de erros
};

struct mensagem_t *mensagem_cria(unsigned char tamanho,
                                 enum tipo_msg_t tipo, unsigned char *dados);

int mensagem_envia(int socket, struct mensagem_t *msg);

int mensagem_recebe(int socket, struct mensagem_t *msg);

void mensagem_imprime(struct mensagem_t *msg);

#endif
