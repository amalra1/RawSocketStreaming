#include "libServer.h"
#include "pilha.h"

// Função auxiliar pra remover a extensão do arquivo do nome dos vídeos
char *remove_ext(char *nome) 
{
    char *string = nome;
    // char *ponto = strrchr(string, '.');
    // if (ponto != NULL)
    //     *ponto = '\0';

    return string;
}

void print_cabecalho_client() 
{
    printf(".__________________________.\n");
    printf("|                          |\n");
    printf("|  ____    ____   ____     |       Raw Socket Streaming\n");
    printf("| |  _ \\  / ___| / ___|    |       Desenvolvido por:\n");
    printf("| | |_) | \\___ \\ \\___ \\    |       Pedro Amaral Chapelin e Arthur Lima Gropp\n");
    printf("| |  _ <   ___) | ___) |   |\n");
    printf("| |_| \\_\\ |____/ |____/    |\n");
    printf("|                          |\n");
    printf("|__________________________|\n");
}

void limpa_tela()
{
    system("clear");
    print_cabecalho_client();
}

int main(int argc, char *argv[]) 
{
    int sockfd, entradaOpcao, entradaVideo, sequencia = 0;
    unsigned char dados[TAM_DADOS];

    FILE *arq;

    frame_t frame;
    frame_t frame_resp;
    inicializa_frame(&frame);
    inicializa_frame(&frame_resp);
   
    pilhaString pilhaFilmes;
    inicializa_pilha(&pilhaFilmes);

    printf("Iniciando Client ...\n");

    // Criação do Raw Socket para comunicação
    sockfd = cria_raw_socket("lo");
    if (sockfd == -1) 
    {
        perror("Error on client socket creation:");
        return EXIT_FAILURE;
    }

    limpa_tela();

    // FASE 1 - INICIAR CLIENT E MOSTRAR OS FILMES DISPONÍVEIS

    memset(dados, 0, TAM_DADOS);
    frame = monta_mensagem(0x00, 0x00, 0x0A, dados, 0); // Mensagem sem dados - CRC 0x00

    // Envia o frame com tipo == LISTA
    if (send(sockfd, &frame, sizeof(frame), 0) < 0)
    {
        perror("Erro no envio:");
        return EXIT_FAILURE;
    }

    // Aguarda pelo recebimento do ACK do server
    if (recv(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
    {
        perror("Erro no recebimento:");
        return EXIT_FAILURE;
    }
    while (!eh_ack(&frame_resp))
    {
        if (eh_nack(&frame_resp))
        {
            if (send(sockfd, &frame, sizeof(frame), 0) < 0)
            {
                perror("Erro no envio:");
                return EXIT_FAILURE;
            }
        }

        if (recv(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
        {
            perror("Erro no recebimento:");
            return EXIT_FAILURE;
        }
    }

    // Começa a receber os dados referentes ao comando "lista"
    if (recv(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
    {
        perror("Erro no recebimento:");
        return EXIT_FAILURE;
    }
    while (!eh_fimtx(&frame_resp)) // Até receber um frame válido do tipo == "fim_tx"
    {
        if (eh_valida(&frame_resp))
        {
            if (eh_dados(&frame_resp))
            {
                if (verifica_crc(&frame_resp))
                {
                    if (!esta_na_pilha(&pilhaFilmes, (char*)frame_resp.dados))
                        push(&pilhaFilmes, (char*)frame_resp.dados);
                    
                    // Prepara a mensagem de volta (ACK)
                    memset(dados, 0, TAM_DADOS);
                    frame = monta_mensagem(0x00, 0x00, 0x00, dados, 0);

                    // Enviando ACK
                    if (send(sockfd, &frame, sizeof(frame), 0) < 0)
                    {
                        perror("Erro no envio:");
                        return EXIT_FAILURE;
                    }
                }
                else
                {
                    // Prepara a mensagem de volta (NACK)
                    memset(dados, 0, TAM_DADOS);
                    frame = monta_mensagem(0x00, 0x00, 0x01, dados, 0);

                    // Enviando NACK
                    if (send(sockfd, &frame, sizeof(frame), 0) < 0)
                    {
                        perror("Erro no envio:");
                        return EXIT_FAILURE;
                    }
                }
            }
            else
            {
                // Prepara a mensagem de volta (NACK)
                memset(dados, 0, TAM_DADOS);
                frame = monta_mensagem(0x00, 0x00, 0x01, dados, 0);

                // Enviando NACK
                if (send(sockfd, &frame, sizeof(frame), 0) < 0)
                {
                    perror("Erro no envio:");
                    return EXIT_FAILURE;
                }
            }
        }

        if (recv(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
        {
            perror("Erro no recebimento:");
            return EXIT_FAILURE;
        }
    }

    // Frame indicando final de transmissão recebido do server.

    // Prepara a mensagem de volta (ACK)
    memset(dados, 0, TAM_DADOS);
    frame = monta_mensagem(0x00, 0x00, 0x00, dados, 0);

    // Enviando ACK
    if (send(sockfd, &frame, sizeof(frame), 0) < 0)
    {
        perror("Erro no envio:");
        return EXIT_FAILURE;
    }

    // LOOP PRINCIPAL DO CLIENT
    while (1) 
    {
        printf("\nCatálogo de vídeos disponíveis:\n\n");
        for (int i = 0; i <= pilhaFilmes.topo; i++)
            printf("[%d] %s\n", i + 1, remove_ext(pilhaFilmes.items[i]));
        printf("\nOpções:\n\n[1]. Baixar algum vídeo\n[2]. Fechar o Client\n");
        printf("\nEscolha uma opção: ");
        scanf("%d", &entradaOpcao);

        // Caso entrada inválida
        while (entradaOpcao < 1 || entradaOpcao > 2)
        {
            limpa_tela();
            printf("\nCatálogo de vídeos disponíveis:\n\n");
            for (int i = 0; i <= pilhaFilmes.topo; i++)
                printf("[%d] %s\n", i + 1, remove_ext(pilhaFilmes.items[i]));
            printf("\nEntrada inválida, escolha entre [1] e [2]\n");
            printf("\nOpções:\n\n[1]. Baixar algum filme\n[2]. Fechar o Client\n");

            printf("\nEscolha uma opção: ");
            scanf("%d", &entradaOpcao);
        }

        if (entradaOpcao == 1)
        {
            limpa_tela();
            printf("\nCatálogo de vídeos disponíveis:\n\n");
            for (int i = 0; i <= pilhaFilmes.topo; i++)
                printf("[%d] %s\n", i + 1, remove_ext(pilhaFilmes.items[i]));
            printf("\nEscolha um vídeo para baixar: ");
            scanf("%d", &entradaVideo);

            while (entradaVideo < 1 || entradaVideo > pilhaFilmes.topo + 1)
            {
                limpa_tela();
                printf("\nPor favor, escolha uma opção válida!\n");
                printf("\nCatálogo de vídeos disponíveis:\n\n");
                for (int i = 0; i <= pilhaFilmes.topo; i++)
                    printf("[%d] %s\n", i + 1, remove_ext(pilhaFilmes.items[i]));
                printf("\nEscolha uma opção: ");
                scanf("%d", &entradaVideo);
            }

            printf("\nIniciando Download\n");

            // Prepara e envia o frame com o nome do vídeo
            strncpy((char*)dados, pilhaFilmes.items[entradaVideo-1], TAM_DADOS-1);
            frame = monta_mensagem((unsigned char)strlen(pilhaFilmes.items[entradaVideo-1]), 0x00, 0x0B, dados, 1);

            if (send(sockfd, &frame, sizeof(frame), 0) < 0)
            {
                perror("Erro no envio:");
                return EXIT_FAILURE;
            }

            // Aguarda pelo recebimento do ACK do server
            if (recv(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
            {
                perror("Erro no recebimento:");
                return EXIT_FAILURE;
            }
            while (!eh_ack(&frame_resp))
            {
                if (eh_nack(&frame_resp))
                {
                    if (send(sockfd, &frame, sizeof(frame), 0) < 0)
                    {
                        perror("Erro no envio:");
                        return EXIT_FAILURE;
                    }
                }

                if (recv(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                {
                    perror("Erro no recebimento:");
                    return EXIT_FAILURE;
                }
            }

            // Cria o arquivo que será o vídeo
            arq = fopen(pilhaFilmes.items[entradaVideo-1], "w+");
            if (!arq)
            {
                perror("Erro ao abrir/criar o arquivo");
                return EXIT_FAILURE;
            }

            // Começa a receber os dados referentes ao vídeo pedido
            if (recv(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
            {
                perror("Erro no recebimento:");
                return EXIT_FAILURE;
            }
            while (!eh_fimtx(&frame_resp)) // Até receber um frame válido do tipo == "fim_tx"
            {
                if (eh_valida(&frame_resp))
                {
                    if (eh_dados(&frame_resp))
                    {
                        if (frame_resp.sequencia == sequencia)
                        {
                            if (verifica_crc(&frame_resp))
                            {
                                fwrite(frame_resp.dados, sizeof(char), frame_resp.tamanho, arq);

                                sequencia = (sequencia + 1) % 32;

                                // Prepara a mensagem de volta (ACK)
                                memset(dados, 0, TAM_DADOS);
                                frame = monta_mensagem(0x00, 0x00, 0x00, dados, 0);

                                // Enviando ACK
                                if (send(sockfd, &frame, sizeof(frame), 0) < 0)
                                {
                                    perror("Erro no envio:");
                                    return EXIT_FAILURE;
                                }
                            }
                            else
                            {
                                // Prepara a mensagem de volta (NACK)
                                memset(dados, 0, TAM_DADOS);
                                frame = monta_mensagem(0x00, 0x00, 0x01, dados, 0);

                                // Enviando NACK
                                if (send(sockfd, &frame, sizeof(frame), 0) < 0)
                                {
                                    perror("Erro no envio:");
                                    return EXIT_FAILURE;
                                }
                            }
                        }
                    }
                    else
                    {
                        // Prepara a mensagem de volta (NACK)
                        memset(dados, 0, TAM_DADOS);
                        frame = monta_mensagem(0x00, 0x00, 0x01, dados, 0);

                        // Enviando NACK
                        if (send(sockfd, &frame, sizeof(frame), 0) < 0)
                        {
                            perror("Erro no envio:");
                            return EXIT_FAILURE;
                        }
                    }
                }

                if (recv(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                {
                    perror("Erro no recebimento:");
                    return EXIT_FAILURE;
                }
            }

            // Frame indicando final de transmissão recebido do server.

            fclose(arq);

            // Prepara a mensagem de volta (ACK)
            memset(dados, 0, TAM_DADOS);
            frame = monta_mensagem(0x00, 0x00, 0x00, dados, 0);

            // Enviando ACK
            if (send(sockfd, &frame, sizeof(frame), 0) < 0)
            {
                perror("Erro no envio:");
                return EXIT_FAILURE;
            }

            printf("\nDownload realizado com sucesso\n");
        }

        if (entradaOpcao == 2)
            break;
    }

    // Encerra o socket
    close(sockfd);

    fprintf(stdout, "\nConexão fechada\n\n");

    return EXIT_SUCCESS;
}
