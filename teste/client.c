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
    char bufferMensagem[BUFFER_LENGTH];
    int entrada;
    int tentativas = 0;

    fprintf(stdout, "Iniciando Client ...\n");

    // Criação do Raw Socket para comunicação
    sockfd = cria_raw_socket("lo", &server_addr);
    if (sockfd == -1) 
    {
        perror("Error on client socket creation:");
        return EXIT_FAILURE;
    }
    //fprintf(stdout, "Socket do Client criado com descritor: %d\n", sockfd);
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

        // Aqui cria o frame para envio da mensagem e variáveis para preenchê-lo 
        // de acordo com a opt escolhida acima
        frame_t frame;

        inicializa_frame(&frame);

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

            // Zera o buffer que será usado para mensagem
            memset(bufferMensagem, 0x0, BUFFER_LENGTH); 

            // Copia o frame para o buffer de envio
            memcpy(bufferMensagem, &frame, sizeof(frame_t));

            // Retry loop for sending the frame
            while (tentativas < MAX_TENTATIVAS) 
            {
                if (sendto(sockfd, bufferMensagem, sizeof(frame_t), 0, (struct sockaddr *)&server_addr, addr_len) < 0) 
                {
                    perror("Send error:");
                    return EXIT_FAILURE;
                }

                frame_t frame_resp;

                if (recvfrom(sockfd, &frame_resp, sizeof(frame_t), 0, (struct sockaddr *)&server_addr, &addr_len) < 0) 
                {
                    perror("Receive ACK error:");
                    return EXIT_FAILURE;
                }

                print_frame(&frame_resp);

                // Validação da resposta do Server

                if (calcula_crc(&frame_resp))
                {
                    printf("CRC-8 OK, mensagem recebida com sucesso.\n");
                    break;  // Sai do loop das tentativas
                }
                else
                {
                    printf("CRC-8 falhou, (tentativa %d/%d).\n", tentativas + 1, MAX_TENTATIVAS);
                    tentativas++;
                }

                // unsigned char received_crc = calculate_crc8(&frame_resp);

                // if (received_crc == frame_resp.crc8 && frame_resp.marcadorInicio == 0x7E) 
                // {
                //     printf("Valid ACK received.\n");
                //     break; // Se válido, sai desse loop interno de tentativas
                // } 
                // else 
                // {
                //     printf("Invalid ACK received (attempt %d/%d).\n", tentativas + 1, MAX_TENTATIVAS);
                //     tentativas++;
                // }
            }

            if (tentativas == MAX_TENTATIVAS) 
            {
                printf("Número máximo de tentativas alcançadas. O processo de validação da resposta falhou.\n");
                break;
            }

            tentativas = 0;

        if (entrada == 6)
            break;

        }
    }

            // ------------------------ PRÓXIMOS PASSOS

            // Enviar a mensagem 
                // -- Dividir em pedaços 5 bits por 5 bits? -> Janela Deslizante de tamanho 5 do enunciado
                // -- Para mensagens que não envolvem transferência de arquivo, é Para-e-espera
                // Aparentemente é frame | frame | frame e não bit | bit | bit de cada frame

            // A cada pedaço enviado, receber resposta do server
                // Ack / Nack

            // Depois de ter recebido Ack de todas as partes, aí vem a parte do Server interpretar a msg lá dentro
                // Na verdade antes de interpretar, vem o server(?) realizar o CRC-8 com a msg completa

            // Depois do CRC-8, server tem que mandar o que foi pedido para o Client
                // Mesmo processo de montar e mandar msg, mas agora do Server pro Client

            // Depois de recebida, usar o conteúdo da msg para fazer algo na tela
                // No caso 1, usar os títulos dos filmes que virão nos Dados, para imprimir eles na tela

        // -------------------PARTE ANTIGA

        // // Zera o buffer que será usado para mensagem
        // memset(bufferMensagem, 0x0, BUFFER_LENGTH);

        // // Captura da mensagem a ser enviada
        // fprintf(stdout, "Diga algo para o Server: ");
        // fgets(buffer, BUFFER_LENGTH, stdin);

        // // Manda a mensagem pro Server
        // if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, addr_len) < 0) 
        // {
        //     perror("Send error:");
        //     return EXIT_FAILURE;
        // }

        // // Recebe resposta do Server
        // if (recvfrom(sockfd, buffer, BUFFER_LENGTH, 0, (struct sockaddr*)&server_addr, &addr_len) < 0) 
        // {
        //     perror("Receive error:");
        //     return EXIT_FAILURE;
        // }
        // fprintf(stdout, "Server answer: %s\n", buffer);

        // // Se digita 'bye' acaba com a conexão
        // if (strncmp(buffer, "bye", 3) == 0)
        //     break;

    close(sockfd);

    fprintf(stdout, "\nConexão fechada\n\n");

    return EXIT_SUCCESS;
}
