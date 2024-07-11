#include "libServer.h"

#define BUFFER_LENGTH 4096

int main(int argc, char *argv[]) 
{
    int sockfd;
    int ack_enviado = 0;
    int ack_recebido = 0;
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

    const char *diretorio = "./videos";

    unsigned char tam;
    unsigned char seq;
    unsigned char tipo;
    unsigned char dados[TAM_DADOS];
    unsigned char crc;

    int sequencia = 0;

    // LOOP PRINCIPAL DO SERVER
    while (1) 
    {
        // Clear frame_resp before receiving
        memset(&frame, 0, sizeof(frame_t));

        if (recvfrom(sockfd, &frame, sizeof(frame), 0, (struct sockaddr *)&sndr_addr, &addr_len))
        {
            // Se a mensagem recebida for válida, e não lixo
            if (frame.marcadorInicio == 0x7E)
            {
                // printf("---------FRAME RECEBIDO------------\n");
                // print_frame(&frame);
                // printf("-----------------------------------\n");

                if (frame.tipo == 0x00){} // recebeu um "ack"
                if (frame.tipo == 0x01){} // recebeu um "nack"

                if (frame.tipo == 0x0A) // recebeu um "lista"
                {
                    frame_resp.marcadorInicio = 0x7E;
                    frame_resp.tamanho = 0x2C; // os 44 bytes da mensagem
                    frame_resp.sequencia = 0x00;
                    frame_resp.tipo = 0x12; // tipo de dados - 10010
                    strncpy(frame_resp.dados, "Possuímos as seguintes opções de vídeo:\n\n", TAM_DADOS-1);
                    frame_resp.crc8 = calcula_crc(&frame_resp);

                    send(sockfd, &frame_resp, sizeof(frame_resp), 0);

                    while (!ack_recebido /*&& !timeout()*/)
                    {
                        // Clear frame before receiving
                        memset(&frame, 0, sizeof(frame_t));

                        recv(sockfd, &frame, sizeof(frame), 0);

                        if (frame.marcadorInicio == 0x7E)
                        {
                            if (frame.tipo == 0x00) // recebeu um "ack"
                            {
                                ack_recebido = 1;
                                // cancela o timeout
                            }

                            if (frame.tipo == 0x01) // recebeu um "nack"
                            {
                                send(sockfd, &frame_resp, sizeof(frame_resp), 0);
                                // reseta o timeout
                            }
                        }
                    }
                    
                    struct dirent *entry;

                    DIR *dir = opendir(diretorio);
                    if (dir == NULL)
                    {
                        perror("opendir");
                        return;
                    }

                    while ((entry = readdir(dir)))
                        printf("%s\n", entry->d_name);
                    printf("\n");

                     closedir(dir);
                }

                if (frame.tipo == 0x0B){} // recebeu um "baixar"
                if (frame.tipo == 0x10){} // recebeu um "mostra na tela"
                if (frame.tipo == 0x11){} // recebeu um "descritor arquivo"
                if (frame.tipo == 0x12){} // recebeu um "dados"
                if (frame.tipo == 0x1E){} // recebeu um "fim tx"
                if (frame.tipo == 0x1F){} // recebeu um "erro"

                // // Prepara a mensagem de volta (ACK)
                // inicio = 0x7E; // (0111 1110)
                // tam = 0x00;
                // seq = 0x00;
                // tipo = 0x00; // tipo do ACK - 00000
                // memset(dados, 0, TAM_DADOS);
                // crc = 0x00;

                // frame_resp = monta_mensagem(inicio, tam, seq, tipo, dados, crc);
                // printf("\n---------FRAME QUE SERÁ ENVIADO------------\n");
                // print_frame(&frame_resp);
                // printf("-----------------------------------\n");
                // send_ack(&frame_resp);

                // if (sendto(sockfd, &frame_resp, sizeof(frame_resp), 0, (struct sockaddr *)&sndr_addr, addr_len) < 0)
                // {
                //     perror("Erro ao enviar o ACK:");
                //     return EXIT_FAILURE;
                // }
                // printf("ACK enviado\n");
                // break;

                // ack_enviado = 1;
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