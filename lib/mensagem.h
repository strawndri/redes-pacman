#ifndef MENSAGEM_H__
#define MENSAGEM_H__

#define MSG_MARCADOR_INICIO 0b01111110
#define MAX_DADOS 31

#define TIME_OUT_SEND 100
#define TIME_OUT_GET 100

// ENUM para definir o tipo da mensagem a ser enviada
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
    MSG_DERROTA = 8,
    MSG_VITORIA = 9,
    MSG_MOV_DIR = 10,
    MSG_MOV_ESQ = 11,
    MSG_MOV_CIMA = 12,
    MSG_MOV_BAIXO = 13,
    MSG_FIM_RODADA = 14,
    MSG_ERRO = 15,
    MSG_FIM = 16,
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

// função auxiliar usada para medir o timeout
// RETORNO: tempo atual em milissegundos
long long timestamp();

// cria uma nova mensagem
// RETORNO: ponteiro para struct mensagem_t ou NULL se erro
struct mensagem_t *mensagem_cria(unsigned char tamanho, enum tipo_msg_t tipo, 
                                 unsigned char *dados, unsigned char seq);

// serializa e envia uma mensagem pelo socket
// RETORNO: número de bytes enviados ou -1 em caso de erro
int mensagem_envia(int socket, struct mensagem_t *msg);

// aguarda e recebe uma mensagem pelo socket respeitando o timeout
// RETORNO: número de bytes recebidos ou -1 em caso de timeout ou erro
int mensagem_recebe(int socket, struct mensagem_t *msg, int timeoutMillis);

// gera o CRC-8 de um bloco de dados para detecção de erros
// RETORNO: byte CRC calculado
unsigned char crc8_gera(unsigned char *dados, unsigned char tamanho);

// copia uma mensagem para um buffer para envio pelo socket
// RETORNO: ponteiro para o buffer serializado ou NULL em caso de erro
unsigned char *mensagem_serializa(struct mensagem_t *msg);

// envia uma mensagem com para-e-espera, retransmitindo até receber ACK
// incrementa o número de sequência após a confirmação
// RETORNO: void
void mensagem_envia_sw(int socket, struct mensagem_t *msg, unsigned char *seq);

// preenche os dados de uma mensagem aplicando escape de VLAN
// para quando os dados não cabem em um único pacote e precisam ser divididos
// RETORNO: quantos bytes de dados foram consumidos
int mensagem_preenche_dados(struct mensagem_t *msg, unsigned char *dados, int tamanho);

#endif
