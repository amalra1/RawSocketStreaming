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
    int sockfd;
    int ack_enviado = 0;
    int frame_recebido = 0;

    frame_t frame;
    frame_t frame_resp;

    inicializa_frame(&frame);
    inicializa_frame(&frame_resp);

    struct sockaddr_ll sndr_addr;
    socklen_t addr_len = sizeof(struct sockaddr_ll);

    fprintf(stdout, "Iniciando Server ....\n");

    // Cria o Raw Socket
    sockfd = cria_raw_socket("lo");
    if (sockfd == -1) 
    {
        perror("Error on server socket creation:");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Server socket created with fd: %d\n", sockfd);

    // LOOP PRINCIPAL DO SERVER
    while (1) 
    {
        if (recvfrom(sockfd, &frame, sizeof(frame), 0, (struct sockaddr *)&sndr_addr, &addr_len))        
        {
            // Se a mensagem recebida for válida, e não lixo
            if (frame.marcadorInicio == 0x7E)
            {
                unsigned char inicio;
                unsigned char tam;
                unsigned char seq;
                unsigned char tipo;
                char dados[TAM_DADOS];
                unsigned char crc;

                printf("---------FRAME RECEBIDO------------\n");
                print_frame(&frame);
                printf("-----------------------------------\n");

                // Prepara a mensagem de volta (ACK)
                inicio = 0x7E;
                tam = 0x00;
                seq = 0x00;
                tipo = 0x00; // tipo do ACK - 00000
                memset(dados, 0, TAM_DADOS);
                crc = 0x00;

                frame_resp = monta_mensagem(inicio, tam, seq, tipo, dados, crc);

                printf("\n---------FRAME QUE SERÁ ENVIADO------------\n");
                print_frame(&frame_resp);
                printf("-----------------------------------\n");

                if (sendto(sockfd, &frame_resp, sizeof(frame_resp), 0, (struct sockaddr *)&sndr_addr, addr_len) < 0)
                {
                    perror("Erro ao enviar o ACK:");
                    return EXIT_FAILURE;
                }
                printf("ACK enviado\n");

                ack_enviado = 1;
            }
        }
        else
        {
            perror("Erro de recebimento:");
            return EXIT_FAILURE;
        }
    }

    // Close socket
    close(sockfd);

    printf("Conexão fechada\n\n");

    return EXIT_SUCCESS;
}