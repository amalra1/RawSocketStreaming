#include "libServer.h"

int main(int argc, char *argv[]) 
{
    int sockfd, bytesEscritos, sequencia = 0;
    unsigned char dados[TAM_DADOS];
    char videoBuffer[TAM_DADOS], caminhoCompleto[TAM_DADOS + 9];
    char *diretorio = "./videos/";

    FILE *arq;

    frame_t frame;
    frame_t frame_resp;
    inicializa_frame(&frame);
    inicializa_frame(&frame_resp);

    fprintf(stdout, "Iniciando Server ....\n");

    // Cria o Raw Socket
    sockfd = cria_raw_socket("lo");
    if (sockfd == -1) 
    {
        perror("Error on server socket creation:");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Server socket created with fd: %d\n", sockfd);

    // LOOP PRINCIPAL DO SERVER
    while (1)
    {
        if (recv(sockfd, &frame, sizeof(frame), 0) >= 0)
        {
            // Se a mensagem recebida for válida, e não lixo
            if (eh_valida(&frame))
            {
                // recebeu um "LISTA"
                if (eh_lista(&frame))
                {
                    // Prepara a mensagem de volta (ACK)
                    memset(dados, 0, TAM_DADOS);
                    frame_resp = monta_mensagem(0x00, 0x00, 0x00, dados, 0);

                    // Enviando ACK
                    if (send(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                    {
                        perror("Erro no envio:");
                        return EXIT_FAILURE;
                    }
                    
                    // Vasculha na pasta dos vídeos pelos nomes
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
                            // Prepara e envia o frame com o nome do vídeo
                            strncpy((char*)dados, entry->d_name, TAM_DADOS-1);
                            frame_resp = monta_mensagem((unsigned char)strlen(entry->d_name), 0x00, 0x12, dados, 1);

                            if (send(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                            {
                                perror("Erro no envio:");
                                return EXIT_FAILURE;
                            }

                            // Aguarda pelo recebimento do ACK do client
                            if (recv(sockfd, &frame, sizeof(frame), 0) < 0)
                            {
                                perror("Erro no recebimento:");
                                return EXIT_FAILURE;
                            }
                            while (!eh_ack(&frame))
                            {
                                if (eh_nack(&frame))
                                {
                                    if (send(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                                    {
                                        perror("Erro no envio:");
                                        return EXIT_FAILURE;
                                    }
                                }

                                if (recv(sockfd, &frame, sizeof(frame), 0) < 0)
                                {
                                    perror("Erro no recebimento:");
                                    return EXIT_FAILURE;
                                }
                            }
                        }
                    }
                    closedir(dir);

                    // Prepara FIM_TX
                    memset(dados, 0, TAM_DADOS);
                    frame_resp = monta_mensagem(0x00, 0x00, 0x1E, dados, 0);

                    // Envia FIM_TX
                    if (send(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                    {
                        perror("Erro no envio:");
                        return EXIT_FAILURE;
                    }

                    // Aguarda pelo recebimento do ACK do client
                    if (recv(sockfd, &frame, sizeof(frame), 0) < 0) 
                    {
                        perror("Erro no recebimento:");
                        return EXIT_FAILURE;
                    }
                    while (!eh_ack(&frame))
                    {
                        if (eh_nack(&frame))
                        {
                            if (send(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                            {
                                perror("Erro no envio:");
                                return EXIT_FAILURE;
                            }
                        }
                        
                        if (recv(sockfd, &frame, sizeof(frame), 0) < 0)
                        {
                            perror("Erro no recebimento:");
                            return EXIT_FAILURE;
                        }
                    }
                }
                
                // recebeu um "BAIXAR"
                else if (eh_baixar(&frame))
                {
                    printf("Opção selecionada: BAIXAR\n\n");

                    if (verifica_crc(&frame))
                    {
                        printf("CRC validado com sucesso\n\n");

                        strncpy(videoBuffer, (char*)frame.dados, TAM_DADOS-1);
                        snprintf(caminhoCompleto, TAM_DADOS + 9, "%s%s", diretorio, videoBuffer);
                        
                        // Prepara a mensagem de volta (ACK)
                        memset(dados, 0, TAM_DADOS);
                        frame_resp = monta_mensagem(0x00, 0x00, 0x00, dados, 0);

                        // Enviando ACK
                        if (send(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                        {
                            perror("Erro no envio:");
                            return EXIT_FAILURE;
                        }

                        printf("ACK enviado para o client\n\n");

                        //printf("%s\n", caminhoCompleto);

                        arq = fopen(caminhoCompleto, "r+");
                        if (!arq)
                        {
                            perror("Erro ao abrir o arquivo");
                            return EXIT_FAILURE;
                        }

                        while ((bytesEscritos = fread(dados, sizeof(unsigned char), TAM_DADOS-1, arq)) > 0)
                        {
                            printf("Montando frame com pedaços do arquivo\n\n");

                            // Prepara e envia o frame com os dados do vídeo
                            frame_resp = monta_mensagem((unsigned char)bytesEscritos, (unsigned char)sequencia, 0x12, dados, 1);
                            sequencia = (sequencia + 1) % 32;

                            printf("%s\n\n", (char*)dados);

                            if (send(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                            {
                                perror("Erro no envio:");
                                return EXIT_FAILURE;
                            }

                            printf("Dados acima enviados para o client\n\n");

                            // Aguarda pelo recebimento do ACK do client
                            if (recv(sockfd, &frame, sizeof(frame), 0) < 0)
                            {
                                perror("Erro no recebimento:");
                                return EXIT_FAILURE;
                            }

                            while (!eh_ack(&frame))
                            {
                                if (eh_nack(&frame))
                                {
                                    printf("NACK recebido\n\n");

                                    if (send(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                                    {
                                        perror("Erro no envio:");
                                        return EXIT_FAILURE;
                                    }

                                    printf("Dados enviados novamente para o client\n\n");
                                }

                                if (recv(sockfd, &frame, sizeof(frame), 0) < 0)
                                {
                                    perror("Erro no recebimento:");
                                    return EXIT_FAILURE;
                                }
                            }

                            printf("ACK recebido\n\n");
                        }

                        printf("Chegou no final do video\n\n");

                        fclose(arq);

                        // Prepara FIM_TX
                        memset(dados, 0, TAM_DADOS);
                        frame_resp = monta_mensagem(0x00, 0x00, 0x1E, dados, 0);

                        // Envia FIM_TX
                        if (send(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                        {
                            perror("Erro no envio:");
                            return EXIT_FAILURE;
                        }

                        printf("FIM_TX enviado para o client\n\n");

                        // Aguarda pelo recebimento do ACK do client
                        if (recv(sockfd, &frame, sizeof(frame), 0) < 0) 
                        {
                            perror("Erro no recebimento:");
                            return EXIT_FAILURE;
                        }

                        while (!eh_ack(&frame))
                        {
                            if (eh_nack(&frame))
                            {
                                printf("NACK recebido\n\n");

                                if (send(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                                {
                                    perror("Erro no envio:");
                                    return EXIT_FAILURE;
                                }

                                printf("FIM_TX enviado novamente para o client\n\n");
                            }
                            
                            if (recv(sockfd, &frame, sizeof(frame), 0) < 0)
                            {
                                perror("Erro no recebimento:");
                                return EXIT_FAILURE;
                            }
                        }

                        printf("ACK recebido\n\n");
                    }
                    else
                    {
                        printf("Verificação com CRC falhou\n\n");

                        // Prepara a mensagem de volta (NACK)
                        memset(dados, 0, TAM_DADOS);
                        frame_resp = monta_mensagem(0x00, 0x00, 0x01, dados, 0);

                        // Enviando NACK
                        if (send(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                        {
                            perror("Erro no envio:");
                            return EXIT_FAILURE;
                        }

                        printf("NACK enviado para o client\n\n");
                    }
                }

                // recebeu um "MOSTRA NA TELA"
                else if (frame.tipo == 0x10){} 
                // recebeu um "DESCRITOR ARQUIVO" -- server recebe? resp: sei nem oq faz man kkkkkkkk
                else if (frame.tipo == 0x11){} 
                // recebeu um "DADOS" -- server recebe?
                else if (frame.tipo == 0x12){} 
                // recebeu um "FIM TX" -- server recebe?
                else if (frame.tipo == 0x1E){} 
                // recebeu um "ERRO"
                else if (frame.tipo == 0x1F){}
                // caso não seja nenhuma das opções anteriores - campo tipo com erro
                else
                {
                    // Tipo não reconhecido...
                    // Prepara a mensagem de volta (NACK)
                    memset(dados, 0, TAM_DADOS);
                    frame_resp = monta_mensagem(0x00, 0x00, 0x01, dados, 0);

                    // Enviando NACK
                    if (send(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                    {
                        perror("Erro no envio:");
                        return EXIT_FAILURE;
                    }
                }
            }
        }
    }

    // Close socket
    close(sockfd);

    printf("Conexão fechada\n\n");

    return EXIT_SUCCESS;
}