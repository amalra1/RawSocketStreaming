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
    int sockfd, entradaOpcao, entradaVideo;
    unsigned char dados[TAM_DADOS], sequencia = 0;

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

    //limpa_tela();

    // FASE 1 - INICIAR CLIENT E MOSTRAR OS FILMES DISPONÍVEIS

    memset(dados, 0, TAM_DADOS);
    frame = monta_mensagem(0x00, 0x00, 0x0A, dados, 0); // Mensagem sem dados - CRC 0x00

    // Envia o frame com tipo == LISTA
    if (send(sockfd, &frame, sizeof(frame), 0) < 0)
    {
        perror("Erro no envio:");
        return EXIT_FAILURE;
    }

    //printf("Frame com tipo LISTA enviado\n\n");

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
            //printf("NACK recebido\n\n");

            if (send(sockfd, &frame, sizeof(frame), 0) < 0)
            {
                perror("Erro no envio:");
                return EXIT_FAILURE;
            }

            //printf("Enviando frame com tipo LISTA de novo\n\n");
        }

        if (recv(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
        {
            perror("Erro no recebimento:");
            return EXIT_FAILURE;
        }
    }

    //printf("ACK recebido\n\n");

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
                    //printf("CRC validado com sucesso\n\n");

                    if (!esta_na_pilha(&pilhaFilmes, (char*)frame_resp.dados))
                        push(&pilhaFilmes, (char*)frame_resp.dados);

                    //printf("Filme adicionado na pilha\n\n");
                    
                    // Prepara a mensagem de volta (ACK)
                    memset(dados, 0, TAM_DADOS);
                    frame = monta_mensagem(0x00, 0x00, 0x00, dados, 0);

                    // Enviando ACK
                    if (send(sockfd, &frame, sizeof(frame), 0) < 0)
                    {
                        perror("Erro no envio:");
                        return EXIT_FAILURE;
                    }

                    //printf("ACK enviado\n\n");
                }
                else
                {
                    //printf("CRC falhou\n\n");

                    // Prepara a mensagem de volta (NACK)
                    memset(dados, 0, TAM_DADOS);
                    frame = monta_mensagem(0x00, 0x00, 0x01, dados, 0);

                    // Enviando NACK
                    if (send(sockfd, &frame, sizeof(frame), 0) < 0)
                    {
                        perror("Erro no envio:");
                        return EXIT_FAILURE;
                    }

                    //printf("NACK enviado\n\n");
                }
            }
            else
            {
                //printf("Não-dados recebido\n\n");

                // Prepara a mensagem de volta (NACK)
                // memset(dados, 0, TAM_DADOS);
                // frame = monta_mensagem(0x00, 0x00, 0x01, dados, 0);

                // // Enviando NACK
                // if (send(sockfd, &frame, sizeof(frame), 0) < 0)
                // {
                //     perror("Erro no envio:");
                //     return EXIT_FAILURE;
                // }

                //printf("NACK enviado\n\n");
            }
        }

        if (recv(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
        {
            perror("Erro no recebimento:");
            return EXIT_FAILURE;
        }
    }

    // Frame indicando final de transmissão recebido do server.
    //printf("Recebido frame indicando final de transmissão recebido do server\n\n");

    // Prepara a mensagem de volta (ACK)
    memset(dados, 0, TAM_DADOS);
    frame = monta_mensagem(0x00, 0x00, 0x00, dados, 0);

    // Enviando ACK
    if (send(sockfd, &frame, sizeof(frame), 0) < 0)
    {
        perror("Erro no envio:");
        return EXIT_FAILURE;
    }

    //printf("ACK enviado\n\n");

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
            sequencia = 0;

            //limpa_tela();
            printf("\nCatálogo de vídeos disponíveis:\n\n");

            for (int i = 0; i <= pilhaFilmes.topo; i++)
                printf("[%d] %s\n", i + 1, remove_ext(pilhaFilmes.items[i]));

            printf("\nEscolha um vídeo para baixar: ");
            scanf("%d", &entradaVideo);

            while (entradaVideo < 1 || entradaVideo > pilhaFilmes.topo + 1)
            {
                //limpa_tela();
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

            printf("Frame com titulo do filme enviado para o server\n\n");

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
                    printf("NACK recebido\n\n");

                    if (send(sockfd, &frame, sizeof(frame), 0) < 0)
                    {
                        perror("Erro no envio:");
                        return EXIT_FAILURE;
                    }

                    printf("Titulo do filme enviado novamente para o server\n\n");
                }

                if (recv(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                {
                    perror("Erro no recebimento:");
                    return EXIT_FAILURE;
                }
            }

            printf("ACK recebido pelo server\n\n");

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

            // Se pega um FIM_TX sem querer da placa de rede
            while (eh_fimtx(&frame_resp))
            {
                printf("Pegou FIM_TX sem querer\n");

                if (recv(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                {
                    perror("Erro no recebimento:");
                    return EXIT_FAILURE;
                }
            }

            printf("SEQUENCIA ANTES DE INICIAR: ");
            print_bits(sequencia, 5);
            printf("\n");
            
            while (!eh_fimtx(&frame_resp)) // Até receber um frame válido do tipo == "fim_tx"
            {
                if (eh_valida(&frame_resp))
                {
                    if (eh_dados(&frame_resp))
                    {
                        printf("--------------FRAME RECEBIDO (Dados)--------------\n");
                        print_frame(&frame_resp);
                        printf("--------------------------------------------------\n");

                        printf("Sequência recebida: ");
                        print_bits(frame_resp.sequencia, 5);
                        printf("\n");
                        printf("Sequência que está: ");
                        print_bits(sequencia, 5);
                        printf("\n");

                        if (frame_resp.sequencia == sequencia)
                        {
                            if (verifica_crc(&frame_resp))
                            {
                                printf("CRC Validado com sucesso\n\n");

                                fwrite(frame_resp.dados, sizeof(unsigned char), frame_resp.tamanho, arq);
                                sequencia = (sequencia + 1) % 32;

                                printf("Operação com os dados realizada\n\n");

                                // Prepara a mensagem de volta (ACK)
                                memset(dados, 0, TAM_DADOS);
                                frame = monta_mensagem(0x00, 0x00, 0x00, dados, 0);

                                // Enviando ACK
                                if (send(sockfd, &frame, sizeof(frame), 0) < 0)
                                {
                                    perror("Erro no envio:");
                                    return EXIT_FAILURE;
                                }

                                printf("ACK Enviado para o server\n\n");
                            }
                            else
                            {
                                printf("Verificação do CRC falhou\n\n");

                                // Prepara a mensagem de volta (NACK)
                                memset(dados, 0, TAM_DADOS);
                                frame = monta_mensagem(0x00, 0x00, 0x01, dados, 0);

                                // Enviando NACK
                                if (send(sockfd, &frame, sizeof(frame), 0) < 0)
                                {
                                    perror("Erro no envio:");
                                    return EXIT_FAILURE;
                                }

                                printf("NACK enviado para o server\n\n");
                            }
                        }
                    }
                    // else
                    // {
                    //     printf("Não-dados recebido\n\n");

                    //     // //Prepara a mensagem de volta (NACK)
                    //     // memset(dados, 0, TAM_DADOS);
                    //     // frame = monta_mensagem(0x00, 0x00, 0x01, dados, 0);

                    //     // // Enviando NACK
                    //     // if (send(sockfd, &frame, sizeof(frame), 0) < 0)
                    //     // {
                    //     //     perror("Erro no envio:");
                    //     //     return EXIT_FAILURE;
                    //     // }

                    //     // printf("NACK enviado para o server\n\n");
                    // }
                }

                if (recv(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                {
                    perror("Erro no recebimento:");
                    return EXIT_FAILURE;
                }
            }

            // Frame indicando final de transmissão recebido do server.
            if (eh_fimtx(&frame_resp))
            {
                printf("Frame indicando final de transmissão recebido do server\n\n");

                printf("--------------FRAME RECEBIDO (FIM TX)--------------\n");
                print_frame(&frame_resp);
                printf("--------------------------------------------------\n");
            }

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

            printf("ACK enviado para o server\n\n");

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
