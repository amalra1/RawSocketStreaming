#include "libServer.h"

int main(int argc, char *argv[]) 
{
    int sockfd;

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

    // LOOP PRINCIPAL DO SERVER
    while (1) 
    {
        if (recvfrom(sockfd, &frame, sizeof(frame), 0, (struct sockaddr *)&sndr_addr, &addr_len))
        {
            // Se a mensagem recebida for válida, e não lixo
            if (frame.marcadorInicio == 0x7E)
            {
                printf("---------FRAME RECEBIDO------------\n");
                print_frame(&frame);
                printf("-----------------------------------\n");

                // Prepara a mensagem de volta (ACK)
                tam = 0x00;
                seq = 0x00;
                tipo = 0x00;
                memset(dados, 0, TAM_DADOS);

                frame_resp = monta_mensagem(tam, seq, tipo, dados);
                printf("\n---------FRAME QUE SERÁ ENVIADO (ACK)------------\n");
                print_frame(&frame_resp);
                printf("-----------------------------------\n");

                // Enviando ACK
                if (sendto(sockfd, &frame_resp, sizeof(frame_resp), 0, (struct sockaddr *)&sndr_addr, addr_len) < 0)
                {
                    perror("Erro ao enviar o ACK:");
                    return EXIT_FAILURE;
                }

                printf("ACK enviado\n");

                // recebeu um "ACK"
                if (frame.tipo == 0x00){}
                // recebeu um "NACK"
                if (frame.tipo == 0x01){} 

                // recebeu um "LISTA"
                if (frame.tipo == 0x0A) 
                {
                    frame_resp.marcadorInicio = 0x7E;
                    frame_resp.tamanho = 0x2C; // Os 44 bytes da mensagem
                    frame_resp.sequencia = 0x00;
                    frame_resp.tipo = 0x12; // Tipo de dados - 10010
                    strncpy((char*)frame_resp.dados, "Possuímos as seguintes opções de vídeo:\n\n", TAM_DADOS-1);
                    frame_resp.crc8 = calcula_crc(&frame_resp);

                    printf("\n---------FRAME QUE SERÁ ENVIADO (TEXTO)------------\n");
                    print_frame(&frame_resp);
                    printf("-----------------------------------\n");

                    // Manda frame com texto inicial
                    sendto(sockfd, &frame_resp, sizeof(frame_resp), 0, (struct sockaddr *)&sndr_addr, addr_len);
                    
                    // Vasculha na pasta dos filmes pelos nomes
                    struct dirent *entry;

                    DIR *dir = opendir(diretorio);
                    if (dir == NULL)
                    {
                        perror("opendir");
                        return 1;
                    }

                    // Para teste
                    while ((entry = readdir(dir)))
                    {
                        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
                        {
                            // Preparar e enviar o frame com nome do filme

                            printf("Filme: %s\n", entry->d_name);
                            
                            frame_resp.marcadorInicio = 0x7E;
                            frame_resp.tamanho = (unsigned char)strlen(entry->d_name);
                            frame_resp.sequencia = 0x00;
                            frame_resp.tipo = 0x12; // Tipo de dados - 10010
                            strncpy((char*)frame_resp.dados, entry->d_name, TAM_DADOS-1);
                            frame_resp.crc8 = calcula_crc(&frame_resp);

                            printf("\n---------FRAME QUE SERÁ ENVIADO (NOME FILME)------------\n");
                            print_frame(&frame_resp);
                            printf("-----------------------------------\n");

                            // Envia nome do filme
                            sendto(sockfd, &frame_resp, sizeof(frame_resp), 0, (struct sockaddr *)&sndr_addr, addr_len);

                        }
                    }
                    printf("\n");

                    closedir(dir);

                    // Envia fim_tx

                    printf("\nFim de transmissão\n");

                    frame_resp.marcadorInicio = 0x7E;
                    frame_resp.tamanho = 0x00;
                    frame_resp.sequencia = 0x00;
                    frame_resp.tipo = 0x1E; // Tipo de dados - 11110
                    memset(frame_resp.dados, 0, TAM_DADOS);
                    frame_resp.crc8 = 0x00;;

                    printf("\n---------FRAME QUE SERÁ ENVIADO (FIM TX)------------\n");
                    print_frame(&frame_resp);
                    printf("-----------------------------------\n");

                    // Envia nome do filme
                    sendto(sockfd, &frame_resp, sizeof(frame_resp), 0, (struct sockaddr *)&sndr_addr, addr_len);
                }

                // recebeu um "BAIXAR"
                if (frame.tipo == 0x0B){} 
                // recebeu um "MOSTRA NA TELA"
                if (frame.tipo == 0x10){} 
                // recebeu um "DESCRITOR ARQUIVO" -- server recebe?
                if (frame.tipo == 0x11){} 
                // recebeu um "DADOS" -- server recebe?
                if (frame.tipo == 0x12){} 
                // recebeu um "FIM TX" -- server recebe?
                if (frame.tipo == 0x1E){} 
                // recebeu um "ERRO"
                if (frame.tipo == 0x1F){} 
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