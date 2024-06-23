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
#include "libServer.h"

#define BUFFER_LENGTH 4096

int main(int argc, char *argv[]) 
{
    struct sockaddr_ll client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_ll);
    int sockfd;
    char buffer[BUFFER_LENGTH];
    int recvfromRET;
    int ack_enviado = 0;

    fprintf(stdout, "Iniciando Server ....\n");

    // Cria o Raw Socket
    sockfd = cria_raw_socket("lo", &client_addr);
    if (sockfd == -1) 
    {
        perror("Error on server socket creation:");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Server socket created with fd: %d\n", sockfd);

    // LOOP PRINCIPAL DO SERVER
    while (1) 
    {
        // Zera o buffer
        memset(buffer, 0x0, BUFFER_LENGTH);

        if (recvfrom(sockfd, buffer, sizeof(frame_t), 0, (struct sockaddr*)&client_addr, &addr_len))
        {
            // Interpreta o buffer recebido como um frame
            frame_t *frame = (frame_t*)buffer;

            // Cria frame de resposta
            frame_t frame_resp;
            inicializa_frame(&frame_resp);
            unsigned char inicio;
            unsigned char tam;
            unsigned char seq;
            unsigned char tipo;
            char dados[TAM_DADOS];
            unsigned char crc;

            // Checa pelo marcadorInicio para saber se é uma mensagem válida
            // (Pra não receber lixos que eventualmente vem para o server)
            if (frame->marcadorInicio == 0x7E && !ack_enviado) 
            {
                // Aqui dentro tem que:
                    // Enviar o ACK ou NACK
                    // Enviar o que foi pedido

                printf("Received valid frame:\n");
                print_frame(frame);

                 // Prepara a mensagem de volta (ACK)
                inicio = 0x7E;
                tam = 0x00;
                seq = 0x00;
                tipo = 0x00; // tipo do ACK - 00000
                memset(dados, 0, TAM_DADOS);
                crc = 0x00;

                frame_resp = monta_mensagem(inicio, tam, seq, tipo, dados, crc);

                printf("\n\n\n");
                print_frame(&frame_resp);

                if (sendto(sockfd, &frame_resp, sizeof(frame_t), 0, (struct sockaddr*)&client_addr, addr_len) < 0) 
                {
                    perror("Send ACK error:");
                    return EXIT_FAILURE;
                }
                printf("ACK Sent\n");

                ack_enviado = 1;
            } 
        }
        else
        {
            perror("Receive error:");
            return EXIT_FAILURE;
        }

        // TUDO ABAIXO DESSE COMENTÁRIO É CÓDIGO ANTIGO, não mexi no Server ainda

        // // Zera o buffer
        // memset(buffer, 0x0, BUFFER_LENGTH);

        // recvfromRET = recvfrom(sockfd, buffer, BUFFER_LENGTH - 1, 0, (struct sockaddr*)&client_addr, &addr_len);

        // // Recebe mensagens do Client
        // if (recvfromRET < 0) 
        // {
        //     perror("Receive error:");
        //     return EXIT_FAILURE;
        // }
        // else if (recvfromRET > 0)
        // { 
        //     // Ensure the buffer is null-terminated
        //     buffer[recvfromRET] = '\0'; // Add null terminator at the end of received data

        //     printf("RECVFROMRET: %d\n", recvfromRET);
        //     printf("Client disse: %s\n", buffer);

        //     // Da mesma forma que o Client, 'bye' fecha a conexão
        //     if (strncmp(buffer, "bye", 3) == 0) 
        //     {
        //         sendto(sockfd, "bye", 3, 0, (struct sockaddr*)&client_addr, addr_len);
        //         break;
        //     } 
        //     else if (strncmp(buffer, "BBBBBBBBBBBBBBBBBBBBBBBBBBB", 27) == 0)
        //         // Manda resposta para o Client
        //         sendto(sockfd, "yepBBBBBBBBBBBBBBBBBBBBBBBBBBBB", 31, 0, (struct sockaddr*)&client_addr, addr_len);
        // }
    }

    // Close socket
    close(sockfd);

    printf("Conexão fechada\n\n");

    return EXIT_SUCCESS;
}
