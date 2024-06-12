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
    struct sockaddr_ll server_addr;
    int sockfd;
    socklen_t addr_len;
    char buffer[BUFFER_LENGTH];

    fprintf(stdout, "Starting Client ...\n");

    // Create raw socket
    sockfd = cria_raw_socket("lo", &server_addr);
    if (sockfd == -1) {
        perror("Error on client socket creation:");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Client socket created with fd: %d\n", sockfd);

    addr_len = sizeof(struct sockaddr_ll);

    // Receive message from server
    if (recvfrom(sockfd, buffer, BUFFER_LENGTH, 0, (struct sockaddr*)&server_addr, &addr_len) < 0) {
        perror("Receive error:");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Server says: %s\n", buffer);

    while (1) {
        // Zeroing the buffer
        memset(buffer, 0x0, BUFFER_LENGTH);

        fprintf(stdout, "Say something to the server: ");
        fgets(buffer, BUFFER_LENGTH, stdin);

        // Send message to server
        if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, addr_len) < 0) {
            perror("Send error:");
            return EXIT_FAILURE;
        }

        // Receive response from server
        if (recvfrom(sockfd, buffer, BUFFER_LENGTH, 0, (struct sockaddr*)&server_addr, &addr_len) < 0) {
            perror("Receive error:");
            return EXIT_FAILURE;
        }
        fprintf(stdout, "Server answer: %s\n", buffer);

        // 'bye' message finishes the connection
        if (strncmp(buffer, "bye", 3) == 0)
            break;
    }

    close(sockfd);

    fprintf(stdout, "\nConnection closed\n\n");

    return EXIT_SUCCESS;
}
