#include "libServer.h"

// Função auxiliar pra remover o a extensão do arquivo do nome dos filmes
void remove_ext(char *nome) 
{
    char *ponto = strrchr(nome, '.');
    if (ponto != NULL)
        *ponto = '\0';
}

int main(int argc, char *argv[]) 
{
    int sockfd, entrada;
    unsigned char dados[TAM_DADOS];
    struct sockaddr_ll sndr_addr;
    socklen_t addr_len = sizeof(struct sockaddr_ll);

    int ifindex = if_nametoindex("lo");
    struct sockaddr_ll endereco = {0};
    endereco.sll_family = AF_PACKET;
    endereco.sll_protocol = htons(ETH_P_ALL);
    endereco.sll_ifindex = ifindex;

    frame_t frame;
    frame_t frame_resp;
    inicializa_frame(&frame);
    inicializa_frame(&frame_resp);

    fprintf(stdout, "Iniciando Client ...\n");

    // Criação do Raw Socket para comunicação
    sockfd = cria_raw_socket("lo");
    if (sockfd == -1) 
    {
        perror("Error on client socket creation:");
        return EXIT_FAILURE;
    }

    printf("Client iniciado com sucesso, selecione alguma das opções abaixo.\n\n");

    printf("[1]. Listar todos os filmes\n[2]. Baixar algum filme\n[3]. Mostra na tela(?)\n[4]. Descritor arquivo(?)\n[5]. Dados de algum filme\n[6]. Fechar o Client\n");

    // LOOP PRINCIPAL DO CLIENT
    while (1) 
    {
        printf("\nEscolha uma opção: ");
        scanf("%d", &entrada);

        // Caso entrada inválida
        while (entrada < 1 || entrada > 6)
        {
            printf("\nEntrada inválida, escolha entre [1] e [6]\n");
            printf("[1]. Listar todos os filmes\n[2]. Baixar algum filme\n[3]. Mostra na tela(?)\n[4]. Descritor arquivo(?)\n[5]. Dados de algum filme\n[6]. Fechar o Client\n");

            printf("\nEscolha uma opção: ");
            scanf("%d", &entrada);
        }

        printf("Sua opção foi: %d\n", entrada);

        // Lista todos os filmes
        if (entrada == 1)  // Escolheu o tipo = Lista
        {
            memset(dados, 0, TAM_DADOS);
            frame = monta_mensagem(0x00, 0x00, 0x0A, dados, 0x00); // Mensagem sem dados - CRC 0x00

            printf("\n---------FRAME QUE SERÁ ENVIADO------------\n");
            print_frame(&frame);
            printf("-----------------------------------\n");

            // Envia o frame
            if (sendto(sockfd, &frame, sizeof(frame), 0, (struct sockaddr *)&endereco, sizeof(endereco)) < 0) 
            {
                perror("Erro no send: ");
                return EXIT_FAILURE;
            }

            // Fica recebendo até receber o ACK (Ou NACK, futuramente)
            while (!eh_ack(&frame_resp))
            {
                if (recvfrom(sockfd, &frame_resp, sizeof(frame_resp), 0, (struct sockaddr *)&sndr_addr, &addr_len) < 0) 
                {
                    perror("Erro de recebimento:");
                    return EXIT_FAILURE;
                }
                
                printf("---------FRAME RECEBIDO------------\n");
                print_frame(&frame_resp);
                printf("-----------------------------------\n");

                if (eh_ack(&frame_resp))
                    break;

                printf("Mensagem que não é um ACK recebida, recebendo outra...\n");
                memset(&frame_resp, 0, sizeof(frame_t));
            }

            // Chegou aqui, é porque detectou um ACK
            printf("---ACK recebido.\n");

            printf("Catálogo de filmes disponíveis:\n");

            // Começa a receber os dados referentes ao comando "lista"
            while (!eh_fimtx(&frame_resp)) // Até receber um frame do tipo == "fim_tx"
            {
                // !!!!!!!!!!!! CRC precisa ser calculado nas mensagens que chegam !!!!!!!!!!!!!!!!!

                recvfrom(sockfd, &frame_resp, sizeof(frame_resp), 0, (struct sockaddr *)&sndr_addr, &addr_len);

                // printf("---------FRAME RECEBIDO DPS DO ACK (LOOP FORA)------------\n");
                // print_frame(&frame_resp);
                // printf("-----------------------------------\n");

                if (eh_fimtx(&frame_resp))
                {
                    // Envia ACK e termina operação lista

                    printf("---Frame indicando final de transmissão recebido do server.\n");
 
                    memset(dados, 0, TAM_DADOS);
                    frame = monta_mensagem(0x00, 0x00, 0x00, dados, 0x00);

                    // printf("\n---------FRAME QUE SERÁ ENVIADO (ACK)------------\n");
                    // print_frame(&frame);
                    // printf("-----------------------------------\n");

                    // Enviando ACK
                    if (sendto(sockfd, &frame, sizeof(frame), 0, (struct sockaddr *)&sndr_addr, addr_len) < 0)
                    {
                        perror("Erro ao enviar o ACK:");
                        return EXIT_FAILURE;
                    }

                    printf("---ACK enviado para o server.\n");

                    break;
                }
                     
                while (!eh_dados(&frame_resp)) // Até receber um frame do tipo == "dados"
                {
                    recvfrom(sockfd, &frame_resp, sizeof(frame_resp), 0, (struct sockaddr *)&sndr_addr, &addr_len);

                    // printf("---------FRAME RECEBIDO DPS DO ACK (LOOP DENTRO)------------\n");
                    // print_frame(&frame_resp);
                    // printf("-----------------------------------\n");

                    if (eh_fimtx(&frame_resp))
                    {
                        // Envia ACK e termina operação lista

                        printf("---Frame indicando final de transmissão recebido do server.\n");
    
                        memset(dados, 0, TAM_DADOS);
                        frame = monta_mensagem(0x00, 0x00, 0x00, dados, 0x00);

                        // printf("\n---------FRAME QUE SERÁ ENVIADO (ACK)------------\n");
                        // print_frame(&frame);
                        // printf("-----------------------------------\n");

                        // Enviando ACK
                        if (sendto(sockfd, &frame, sizeof(frame), 0, (struct sockaddr *)&sndr_addr, addr_len) < 0)
                        {
                            perror("Erro ao enviar o ACK:");
                            return EXIT_FAILURE;
                        }

                        printf("---ACK enviado para o server.\n");

                        break;
                    }
                }

                if (eh_dados(&frame_resp))
                { 
                    // Envia ACK e realiza operação com o nome do filme

                    printf("---Frame com nome de filme recebido do server.\n");
 
                    memset(dados, 0, TAM_DADOS);
                    frame = monta_mensagem(0x00, 0x00, 0x00, dados, 0x00);

                    // printf("\n---------FRAME QUE SERÁ ENVIADO (ACK)------------\n");
                    // print_frame(&frame);
                    // printf("-----------------------------------\n");

                    // Enviando ACK
                    if (sendto(sockfd, &frame, sizeof(frame), 0, (struct sockaddr *)&sndr_addr, addr_len) < 0)
                    {
                        perror("Erro ao enviar o ACK:");
                        return EXIT_FAILURE;
                    }

                    printf("---ACK enviado para o server.\n");

                    remove_ext((char*)frame_resp.dados);
                    printf("%s\n", frame_resp.dados);
                }
            }

            printf("\nFim da lista.\n");
        }

        if (entrada == 6)
            break;
    }

    // Encerra o socket
    close(sockfd);

    fprintf(stdout, "\nConexão fechada\n\n");

    return EXIT_SUCCESS;
}
