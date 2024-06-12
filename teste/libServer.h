#ifndef _libServer_
#define _libServer_

#define TAM_DADOS 512 // 1byte = 8bits, 64bytes = 512bits

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
int cria_raw_socket(char* nome_interface_rede, struct sockaddr_ll *endereco); 

#endif