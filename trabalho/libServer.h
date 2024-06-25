#ifndef _libServer_
#define _libServer_

#define TAM_DADOS 64 // 1byte = 8bits, 64bytes = 512bits
#define TAM_JANELA_PEE 1  // Tamanho da janela do Para-E-Espera
#define TAM_JANELA_JD 5  // Tamanho da janela da Janela Deslizante
#define MAX_TENTATIVAS 8  // Número máximo de tentativas após falha do CRC-8
#define TIMEOUT_SEC 60 

typedef struct
{
    unsigned char marcadorInicio: 8;
    unsigned char tamanho: 6;
    unsigned char sequencia: 5;
    unsigned char tipo: 5;
    char dados[TAM_DADOS];
    unsigned char crc8: 8;

} frame_t;

// Meio de comunicar entre cliente e servidor
int cria_raw_socket(char* nome_interface_rede); 

// Preenche todo o frame com 0's
void inicializa_frame(frame_t *frame); 

// Monta a mensagem baseada na estrutura do enunciado e nos parâmetros passados
frame_t monta_mensagem(int inicio, unsigned char tam, unsigned char sequencia, unsigned char tipo, char* dados, unsigned char crc);

// Imprime o frame na tela
void print_frame(frame_t *frame);

// Calcula o CRC-8 da mensagem, retorna 1 se OK e 0 se não
int calcula_crc(frame_t *frame);

#endif