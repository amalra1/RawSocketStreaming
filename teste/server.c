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
#include <linux/if_ether.h>
#include "libServer.h"

#define BUFFER_LENGTH 4096

int main ()
{
    char buffer[BUFFER_LENGTH];

    // Create the raw socket using loopback interface
    int soquete = cria_raw_socket("lo");

    // Prepare the message to send
    const char *message = "Hello, loopback!";
    strcpy(buffer, message);

    // Prepare the destination address
    struct sockaddr_ll dest_addr = {0};
    dest_addr.sll_family = AF_PACKET;
    dest_addr.sll_protocol = htons(ETH_P_ALL);
    dest_addr.sll_ifindex = if_nametoindex("lo");
    dest_addr.sll_halen = ETH_ALEN;
    memset(dest_addr.sll_addr, 0xff, ETH_ALEN);  // Broadcast MAC address for testing

    // Send the message
    if (sendto(soquete, buffer, strlen(buffer), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) == -1) {
        perror("Error sending data");
        close(soquete);
        return EXIT_FAILURE;
    }

    printf("Message sent: %s\n", message);

    // Close the raw socket
    close(soquete);

    return EXIT_SUCCESS;
}