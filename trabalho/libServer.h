#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <dirent.h>

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
    unsigned char dados[TAM_DADOS];
    unsigned char crc8: 8;

} frame_t;

// Meio de comunicar entre cliente e servidor
int cria_raw_socket(char* nome_interface_rede); 

// Preenche todo o frame com 0's
void inicializa_frame(frame_t *frame);

// Lista os vídeos presentes no diretório vídeos
void listar_videos(const char *diretorio);

// Monta a mensagem baseada na estrutura do enunciado e nos parâmetros passados
frame_t monta_mensagem(unsigned char tam, unsigned char sequencia, unsigned char tipo, unsigned char* dados, int crc_flag);

// Imprime o frame na tela
void print_frame(frame_t *frame);

// Calcula o CRC-8 da mensagem e o retorna
unsigned char calcula_crc(frame_t *frame);

// Detecta erros nos dados a partir do crc
int verifica_crc(frame_t *frame);

// Analisa o campo MarcadorInicio
int eh_valida(frame_t *frame);

// Analisa dois campos do frame para ver se é um ACK
int eh_ack(frame_t *frame);

// Analisa dois campos do frame para ver se é um NACK
int eh_nack(frame_t *frame);

// Analisa dois campos do frame para ver se é um FIM_TX
int eh_fimtx(frame_t *frame);

// Analisa o frame para ver se é um LISTA
int eh_lista(frame_t *frame);

// Analisa o frame para ver se é um BAIXAR
int eh_baixar(frame_t *frame);

// Analisa o frame para ver se é um DADOS
int eh_dados(frame_t *frame);

void send_ack();

#endif