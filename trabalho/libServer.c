#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "libServer.h"

int cria_raw_socket(char* nome_interface_rede) 
{ 
    // Cria o Raw Socket
    int soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (soquete == -1) 
    { 
        fprintf(stderr, "Erro ao criar socket: Verifique se você é root!\n");
        exit(-1); 
    }

    printf("Soquete criado\n");

    int ifindex = if_nametoindex(nome_interface_rede);
    struct sockaddr_ll endereco = {0};

    endereco.sll_family = AF_PACKET;
    endereco.sll_protocol = htons(ETH_P_ALL);
    endereco.sll_ifindex = ifindex;

    // Bind the socket to the network interface
    if (bind(soquete, (struct sockaddr *)&endereco, sizeof(endereco)) == -1)
    {
        perror("Erro ao fazer bind no socket");
        exit(-1);
    }

    struct packet_mreq mr = {0};
    mr.mr_ifindex = ifindex;
    mr.mr_type = PACKET_MR_PROMISC;

    // Coloca o socket no modo promíscuo
    if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) 
    {
        fprintf(stderr, "Erro ao fazer setsockopt: "
            "Verifique se a interface de rede foi especificada corretamente.\n");
        exit(-1);
    }

    return soquete;
}

void inicializa_frame(frame_t *frame) 
{
    memset(frame, 0, sizeof(frame_t));
    memset(frame->dados, 0, TAM_DADOS);
}

frame_t monta_mensagem(int inicio, unsigned char tam, unsigned char sequencia, unsigned char tipo, char* dados, unsigned char crc) 
{
    frame_t frame;
    
    if (inicio)
        frame.marcadorInicio = 0x7E; // (0111 1110)
    else
        frame.marcadorInicio = 0x00; // (0000 0000)

    frame.tamanho = tam;

    frame.sequencia = sequencia;

    frame.tipo = tipo;

    memset(frame.dados, 0, TAM_DADOS);
    if (dados != NULL) 
        strncpy(frame.dados, dados, TAM_DADOS - 1); // Ensure null-terminated string

    frame.crc8 = crc;
    return frame;
}

// Função interna para imprimir os bits de cada byte
void print_bits(unsigned char byte, int num_bits) 
{
    for (int i = num_bits - 1; i >= 0; --i)
        printf("%d", (byte >> i) & 1);
}

void print_frame(frame_t *frame) 
{
    printf("\nMarcador de inicio: ");
    print_bits(frame->marcadorInicio, 8);
    printf("\n");

    printf("Tamanho: ");
    print_bits(frame->tamanho, 6);
    printf("\n");

    printf("Sequência: ");
    print_bits(frame->sequencia, 5);
    printf("\n");

    printf("Tipo: ");
    print_bits(frame->tipo, 5);
    printf("\n");

    printf("Dados: ");
    for (int i = 0; i < TAM_DADOS; ++i) {
        print_bits(frame->dados[i], 8);
        printf(" ");
    }
    printf("\n");

    printf("Crc-8: ");
    print_bits(frame->crc8, 8);
    printf("\n\n");
}

int calcula_crc(frame_t *frame)
{
    // Primeiramente importante saber que o CRC é calculado no client.c e adicionado no campo designado
    // para o mesmo na struct do frame, quando ele chegar ao server.c, o server faz o mesmo processo só que com 
    // o frame que está com o CRC calculado adicionado no final, provindo do client;

    // Existem vários poliômios usados para o CRC-8, o 0x07 [00000111] é um deles;

    // Para a "Divisão", é acrescentado um 1 na esquerda do polinômio = [{1}00000111];

    // O processo do CRC é feito através de "divisões" sucessivas de polinômios, a "divisão" está entre
    // aspas porque na verdade é uma sequência de XOR's realizados entre [Dados] e [Polinômio];

    // =========================================================================================================
    // Seguindo os passos abaixo será possível fazer o CRC, usando como exemplo:
        // [Dados] = [11111001100]
        // [Polinômio] = [100000111]

    // 1) Adicionar (N-1) 0's ao final dos [Dados], sendo N o grau do [Polinômio]
        // [Novos Dados] = [11111001100]{00000000}

    // 2) Começar a sequência de XOR's entre [Novos Dados] e [Polinômio]
        // 1111100110000000000 | 100000111

    // 3) Enviar o frame para o server

    // 4) Realizar o mesmo cálculo do CRC no server:
        // Resto == 0 --> Sem erros
        // Resto != 0 --> Erro

    // 5) 11111001100{Resto da divisão anterior} | 100000111

    return 0;
}




