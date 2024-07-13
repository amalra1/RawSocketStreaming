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

    unsigned char dados[TAM_DADOS];

    // LOOP PRINCIPAL DO SERVER
    while (1) 
    {
        if (recvfrom(sockfd, &frame, sizeof(frame), 0, (struct sockaddr *)&sndr_addr, &addr_len))
        {
            // Se a mensagem recebida for válida, e não lixo
            if (eh_valida(&frame))
            {
                printf("---------FRAME RECEBIDO------------\n");
                print_frame(&frame);
                printf("-----------------------------------\n");

                // Prepara a mensagem de volta (ACK)
                memset(dados, 0, TAM_DADOS);
                frame_resp = monta_mensagem(0x00, 0x00, 0x00, dados, 0x00);

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

                // // recebeu um "ACK"
                // if (frame.tipo == 0x00){}  
                // // recebeu um "NACK"
                // if (frame.tipo == 0x01){} 

                // Esses dois acima vão acontecer alguma hora? Acho que client só envia ACK/NACK dps 
                // de ter pedido alguma coisa (ex: Lista/Baixar)

                // recebeu um "LISTA"
                if (eh_lista(&frame)) 
                {
                    printf("Cliente solicitou: LISTA\n");
                    
                    // Vasculha na pasta dos filmes pelos nomes
                    struct dirent *entry;

                    DIR *dir = opendir(diretorio);
                    if (dir == NULL)
                    {
                        perror("opendir");
                        return 1;
                    }

                    while ((entry = readdir(dir)))
                    {
                        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
                        {
                            // Preparar e enviar o frame com nome do filme

                            printf("Nome do filme que será enviado: %s\n", entry->d_name);
                            
                            strncpy((char*)dados, entry->d_name, TAM_DADOS-1);
                            frame_resp = monta_mensagem((unsigned char)strlen(entry->d_name), 0x00, 0x12, dados, calcula_crc(&frame_resp));

                            printf("\n---------FRAME QUE SERÁ ENVIADO (NOME FILME)------------\n");
                            print_frame(&frame_resp);
                            printf("-----------------------------------\n");

                            // Envia nome do filme
                            sendto(sockfd, &frame_resp, sizeof(frame_resp), 0, (struct sockaddr *)&sndr_addr, addr_len);

                            printf("Filme %s enviado para o client\n", entry->d_name);

                            // Aguarda pelo recebimento do ACK do client

                            printf("Aguardando recebimento do ACK do client...\n");

                            if (recvfrom(sockfd, &frame, sizeof(frame), 0, (struct sockaddr *)&sndr_addr, &addr_len) < 0) 
                            {
                                perror("Erro de recebimento:");
                                return EXIT_FAILURE;
                            }

                            printf("---------FRAME RECEBIDO------------\n");
                            print_frame(&frame);
                            printf("-----------------------------------\n");

                            // Fica recebendo até ler um ACK
                            while (!eh_ack(&frame))
                            {
                                if (recvfrom(sockfd, &frame, sizeof(frame), 0, (struct sockaddr *)&sndr_addr, &addr_len) < 0) 
                                {
                                    perror("Erro de recebimento:");
                                    return EXIT_FAILURE;
                                }

                                printf("---------FRAME RECEBIDO------------\n");
                                print_frame(&frame);
                                printf("-----------------------------------\n");

                                if (eh_ack(&frame))
                                    break;

                                printf("Mensagem que não é um ACK recebida, recebendo outra...\n");
                                memset(&frame, 0, sizeof(frame_t));
                            }

                            printf("ACK recebido, prosseguindo para enviar próximo filme\n");
                        }
                    }
                    printf("\n");

                    closedir(dir);

                    // Prepara FIM_TX
                    printf("\nFim de transmissão, todos os filmes enviados\n");

                    memset(dados, 0, TAM_DADOS);
                    frame_resp = monta_mensagem(0x00, 0x00, 0x1E, dados, 0x00);

                    printf("\n---------FRAME QUE SERÁ ENVIADO (FIM TX)------------\n");
                    print_frame(&frame_resp);
                    printf("-----------------------------------\n");

                    // Envia FIM_TX
                    sendto(sockfd, &frame_resp, sizeof(frame_resp), 0, (struct sockaddr *)&sndr_addr, addr_len);

                    printf("\nFrame de fim de transmissão enviado ao client\n");
                    printf("Aguardando recebimento do ACK do client...\n");

                    if (recvfrom(sockfd, &frame, sizeof(frame), 0, (struct sockaddr *)&sndr_addr, &addr_len) < 0) 
                    {
                        perror("Erro de recebimento:");
                        return EXIT_FAILURE;
                    }

                    printf("---------FRAME RECEBIDO------------\n");
                    print_frame(&frame);
                    printf("-----------------------------------\n");

                    // Fica recebendo até ler um ACK
                    while (!eh_ack(&frame))
                    {
                        if (recvfrom(sockfd, &frame, sizeof(frame), 0, (struct sockaddr *)&sndr_addr, &addr_len) < 0) 
                        {
                            perror("Erro de recebimento:");
                            return EXIT_FAILURE;
                        }

                        printf("---------FRAME RECEBIDO------------\n");
                        print_frame(&frame);
                        printf("-----------------------------------\n");

                        if (eh_ack(&frame))
                            break;

                        printf("Mensagem que não é um ACK recebida, recebendo outra...\n");
                    }

                    printf("ACK recebido, transmissão finalizada\n");
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