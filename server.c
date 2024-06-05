#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_LENGTH 4096

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_LENGTH];

    // Create raw socket
    sockfd = socket(AF_INET, SOCK_RAW, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to port
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for message...\n");

    // Receive message
    client_len = sizeof(client_addr);
    if (recvfrom(sockfd, buffer, BUFFER_LENGTH, 0, (struct sockaddr *)&client_addr, &client_len) < 0) {
        perror("recvfrom");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Received message from client: %s\n", buffer);

    close(sockfd);
    return 0;
}
