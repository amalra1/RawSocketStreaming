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
    struct sockaddr_ll server_addr;
    socklen_t addr_len = sizeof(struct sockaddr_ll);
    int sockfd;
    char buffer[BUFFER_LENGTH];

    fprintf(stdout, "Iniciando Client ...\n");

    // Criação do Raw Socket para comunicação
    sockfd = cria_raw_socket("lo", &server_addr);
    if (sockfd == -1) 
    {
        perror("Error on client socket creation:");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Socket do Client criado com descritor: %d\n", sockfd);

    // LOOP PRINCIPAL DO CLIENT
    while (1) 
    {
        // Zera o buffer que será usado para mensagem
        memset(buffer, 0x0, BUFFER_LENGTH);

        // Captura da mensagem a ser enviada
        fprintf(stdout, "Diga algo para o Server: ");
        fgets(buffer, BUFFER_LENGTH, stdin);

        // Manda a mensagem pro Server
        if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, addr_len) < 0) 
        {
            perror("Send error:");
            return EXIT_FAILURE;
        }

        // Recebe resposta do Server
        if (recvfrom(sockfd, buffer, BUFFER_LENGTH, 0, (struct sockaddr*)&server_addr, &addr_len) < 0) 
        {
            perror("Receive error:");
            return EXIT_FAILURE;
        }
        fprintf(stdout, "Server answer: %s\n", buffer);

        // Se digita 'bye' acaba com a conexão
        if (strncmp(buffer, "bye", 3) == 0)
            break;
    }

    close(sockfd);

    fprintf(stdout, "\nConexão fechada\n\n");

    return EXIT_SUCCESS;
}
