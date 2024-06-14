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

int cria_raw_socket(char* nome_interface_rede, struct sockaddr_ll *endereco) 
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
    if (ifindex == 0) {
        perror("Ifindex error:");
        exit(-1);
    }

    memset(endereco, 0, sizeof(struct sockaddr_ll));
    endereco->sll_family = AF_PACKET;
    endereco->sll_protocol = htons(ETH_P_ALL);
    endereco->sll_ifindex = ifindex;

    // Bind the socket to the network interface
    if (bind(soquete, (struct sockaddr*) endereco, sizeof(struct sockaddr_ll)) == -1) 
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
