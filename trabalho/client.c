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
    int entrada;

    struct sockaddr_ll sndr_addr;
    socklen_t addr_len = sizeof(struct sockaddr);

    int ifindex = if_nametoindex("lo");
    struct sockaddr_ll endereco = {0};
    endereco.sll_family = AF_PACKET;
    endereco.sll_protocol = htons(ETH_P_ALL);
    endereco.sll_ifindex = ifindex;

    frame_t frame;
    frame_t frame_resp;
    inicializa_frame(&frame);
    inicializa_frame(&frame_resp);

    fprintf(stdout, "Iniciando Client ...\n");

    // Criação do Raw Socket para comunicação
    sockfd = cria_raw_socket("lo");
    if (sockfd == -1) 
    {
        perror("Error on client socket creation:");
        return EXIT_FAILURE;
    }
    printf("Client iniciado com sucesso, selecione alguma das opções abaixo.\n\n");

    printf("[1]. Listar todos os filmes\n[2]. Baixar algum filme\n[3]. Mostra na tela(?)\n[4]. Descritor arquivo(?)\n[5]. Dados de algum filme\n[6]. Fechar o Client\n");

    // LOOP PRINCIPAL DO CLIENT
    while (1) 
    {
        printf("Escolha uma opção: ");
        scanf("%d", &entrada);

        // Caso entrada inválida
        while (entrada < 1 || entrada > 6)
        {
            printf("\nEntrada inválida, escolha entre [1] e [6]\n");
            printf("[1]. Listar todos os filmes\n[2]. Baixar algum filme\n[3]. Mostra na tela(?)\n[4]. Descritor arquivo(?)\n[5]. Dados de algum filme\n[6]. Fechar o Client\n");

            printf("Escolha uma opção: ");
            scanf("%d", &entrada);
        }

        printf("Sua opção foi: %d\n", entrada);

        // Aqui pega o frame para envio da mensagem e variáveis para preenchê-lo 
        // de acordo com a opt escolhida acima

        unsigned char inicio;
        unsigned char tam;
        unsigned char seq;
        unsigned char tipo;
        char dados[TAM_DADOS];
        unsigned char crc;

        // Lista todos os filmes
        if (entrada == 1)
        {
            // Para listar, só precisará enviar uma mensagem, o que importa 
            // mesmo é o campo "tipo" ser entendido pelo Server

            inicio = 1;
            tam = 0x00;
            seq = 0x00;  
            tipo = 0x0A; // 01010
            memset(dados, 0, TAM_DADOS);
            strncpy(dados, "test", TAM_DADOS);
            crc = 0x00;

            frame = monta_mensagem(inicio, tam, seq, tipo, dados, crc);

            print_frame(&frame);

            // Envia o frame
            if (sendto(sockfd, &frame, sizeof(frame), 0, (struct sockaddr *)&endereco, sizeof(endereco)) < 0) 
            {
                perror("Send error:");
                return EXIT_FAILURE;
            }

            // Clear frame_resp before receiving
            memset(&frame_resp, 0, sizeof(frame_t));

            // Keep receiving until ACK is received
            while (1)
            {
                if (recvfrom(sockfd, &frame_resp, sizeof(frame_resp), 0, (struct sockaddr *)&sndr_addr, &addr_len) < 0) 
                {
                    perror("Receive error:");
                    return EXIT_FAILURE;
                }

                print_frame(&frame_resp);

                // Check if the received frame is an ACK
                if (frame_resp.tipo == 0x00 && frame_resp.marcadorInicio == 0x7E)
                {
                    printf("ACK received.\n");
                    break; // Exit the receiving loop
                }
                else
                {
                    printf("Received non-ACK message, continue receiving...\n");
                    memset(&frame_resp, 0, sizeof(frame_t));
                }
            }
        }

        if (entrada == 6)
            break;
    }

    // Encerra o socket
    close(sockfd);

    fprintf(stdout, "\nConexão fechada\n\n");

    return EXIT_SUCCESS;
}
