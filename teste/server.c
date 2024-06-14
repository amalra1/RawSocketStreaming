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

        recvfromRET = recvfrom(sockfd, buffer, BUFFER_LENGTH - 1, 0, (struct sockaddr*)&client_addr, &addr_len);

        // Recebe mensagens do Client
        if (recvfromRET < 0) 
        {
            perror("Receive error:");
            return EXIT_FAILURE;
        }
        else if (recvfromRET > 0)
        { 
            // Ensure the buffer is null-terminated
            buffer[recvfromRET] = '\0'; // Add null terminator at the end of received data

            printf("RECVFROMRET: %d\n", recvfromRET);
            printf("Client disse: %s\n", buffer);

            // Da mesma forma que o Client, 'bye' fecha a conexão
            if (strncmp(buffer, "bye", 3) == 0) 
            {
                sendto(sockfd, "bye", 3, 0, (struct sockaddr*)&client_addr, addr_len);
                break;
            } 
            else if (strncmp(buffer, "BBBBBBBBBBBBBBBBBBBBBBBBBBB", 27) == 0)
                // Manda resposta para o Client
                sendto(sockfd, "yepBBBBBBBBBBBBBBBBBBBBBBBBBBBB", 31, 0, (struct sockaddr*)&client_addr, addr_len);

            // // // Optional: Periodically send a message to the client
            // strcpy(buffer, "Periodic message from server test aaaaaaaaaa");
            // if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&client_addr, addr_len) < 0) 
            // {
            //     perror("Send error:");
            // }
        }
    }

    // Close socket
    close(sockfd);

    printf("Conexão fechada\n\n");

    return EXIT_SUCCESS;
}
