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

int main(int argc, char *argv[]) {
    struct sockaddr_ll client_addr;
    int sockfd;
    socklen_t addr_len;
    char buffer[BUFFER_LENGTH];

    fprintf(stdout, "Starting server\n");

    // Create raw socket
    sockfd = cria_raw_socket("lo", &client_addr);
    if (sockfd == -1) {
        perror("Error on server socket creation:");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Server socket created with fd: %d\n", sockfd);

    addr_len = sizeof(struct sockaddr_ll);

    // Send a welcome message to the client
    strcpy(buffer, "Hello, client!\n");
    if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&client_addr, addr_len) < 0) {
        perror("Send error:");
        return EXIT_FAILURE;
    }

    // Loop to receive messages from the client
    while (1) {
        // Zeroing the buffer
        memset(buffer, 0x0, BUFFER_LENGTH);

        // Receive message from client
        if (recvfrom(sockfd, buffer, BUFFER_LENGTH, 0, (struct sockaddr*)&client_addr, &addr_len) < 0) {
            perror("Receive error:");
            return EXIT_FAILURE;
        }
        printf("Client says: %s\n", buffer);

        // 'bye' message finishes the connection
        if (strncmp(buffer, "bye", 3) == 0) {
            sendto(sockfd, "bye", 3, 0, (struct sockaddr*)&client_addr, addr_len);
            break;
        } else {
            // Send response to client
            sendto(sockfd, "yep\n", 4, 0, (struct sockaddr*)&client_addr, addr_len);
        }
    }

    // Close the socket
    close(sockfd);

    printf("Connection closed\n\n");

    return EXIT_SUCCESS;
}
