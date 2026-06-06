#ifndef MENSAGEM_H__
#define MENSAGEM_H__

#define MSG_MARCADOR_INICIO 0b01111110
#define MAX_DADOS 31

#define TIME_OUT_SEND 100
#define TIME_OUT_GET 100

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
    unsigned char marcador_inicio;  // 01111110
    unsigned short tamanho : 5;     // tamanho da região de dados (0-31)
    unsigned short sequencia : 6;   // número da sequência (0-63)
    unsigned short tipo : 5;        // tipo da mensagem (definidos num ENUM)
    unsigned char dados[MAX_DADOS]; // área de dados
    unsigned char crc;              // método de detecção de erros
};

// função auxiliar
long long timestamp();

struct mensagem_t *mensagem_cria(unsigned char tamanho, enum tipo_msg_t tipo, unsigned char *dados, unsigned char seq);

int mensagem_envia(int socket, struct mensagem_t *msg);

int mensagem_recebe(int socket, struct mensagem_t *msg, int timeoutMillis);

unsigned char crc8_gera(unsigned char *dados, unsigned char tamanho);

unsigned char *mensagem_serializa(struct mensagem_t *msg);

void mensagem_imprime(struct mensagem_t *msg);

void mensagem_envia_sw(int socket, struct mensagem_t *msg, unsigned char *seq);

#endif
