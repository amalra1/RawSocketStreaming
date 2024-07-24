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

    limpa_tela();

    // INICIA CLIENT E MOSTRA OS FILMES DISPONÍVEIS

    memset(dados, 0, TAM_DADOS);
    frame = monta_mensagem(0x00, 0x00, 0x0A, dados, 0);

    // Envia o frame com tipo == LISTA
    if (send(sockfd, &frame, sizeof(frame), 0) < 0)
    {
        perror("Erro no envio:");
        return EXIT_FAILURE;
    }

    if (!wait_ack(sockfd, &frame, 1000))
        return EXIT_FAILURE;

    // Começa a receber os dados referentes ao comando "lista"
    if (recv(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
    {
        perror("Erro no recebimento:");
        return EXIT_FAILURE;
    }

    // Até receber um frame válido do tipo == "FIM_TX"
    while (!eh_fimtx(&frame_resp)) 
    {
        if (eh_dados(&frame_resp))
        {
            if (verifica_crc(&frame_resp))
            {
                // Mesmo que o server não tenha recebido o ACK da iteração anterior,
                // e esteja reenviando o nome do vídeo, essa verificação garante que
                // o mesmo nome não seja considerado mais de uma vez...
                if (!esta_na_pilha(&pilhaFilmes, (char*)frame_resp.dados))
                    push(&pilhaFilmes, (char*)frame_resp.dados);
                
                if (!send_ack(sockfd))
                    return EXIT_FAILURE;
            }
            else
            {
                if (!send_nack(sockfd))
                    return EXIT_FAILURE;
            }
        }

        if (recv(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
        {
            perror("Erro no recebimento:");
            return EXIT_FAILURE;
        }
    }

    // Frame indicando final de transmissão recebido do server.

    if (!send_ack(sockfd))
        return EXIT_FAILURE;

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

            if (!wait_ack(sockfd, &frame, 1000))
                return EXIT_FAILURE;

            // Cria o arquivo que será o vídeo
            arq = fopen(pilhaFilmes.items[entradaVideo-1], "wb");
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
                if (recv(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                {
                    perror("Erro no recebimento:");
                    return EXIT_FAILURE;
                }
            }

            while (!eh_fimtx(&frame_resp)) // Até receber um frame válido do tipo == "fim_tx"
            {
                if (eh_dados(&frame_resp))
                {
                    // Pode ocorrer de o server não receber o ACK pela mensagem anterior.
                    // Nesse sentido, ele ficaria reenviando até receber um ACK:
                    if (sequencia == 0)
                    {
                        if (frame_resp.sequencia == 31)
                        {
                            if (!send_ack(sockfd))
                                return EXIT_FAILURE;
                        }
                    }
                    else if (frame_resp.sequencia == sequencia - 1)
                    {
                        if (!send_ack(sockfd))
                            return EXIT_FAILURE;
                    }

                    if (frame_resp.sequencia == sequencia)
                    {
                        if (verifica_crc(&frame_resp))
                        {
                            fwrite(frame_resp.dados, sizeof(unsigned char), frame_resp.tamanho, arq);
                            sequencia = (sequencia + 1) % 32;

                            if (!send_ack(sockfd))
                                return EXIT_FAILURE;
                        }
                        else
                        {
                            if (!send_nack(sockfd))
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

            if (!send_ack(sockfd))
                return EXIT_FAILURE;

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
