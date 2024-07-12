#include "libServer.h"

int main(int argc, char *argv[]) 
{
    int sockfd;
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

    unsigned char tam;
    unsigned char seq;
    unsigned char tipo;
    unsigned char dados[TAM_DADOS];

    int entrada;

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
            tam = 0x00;
            seq = 0x00;
            tipo = 0x0A; 
            memset(dados, 0, TAM_DADOS);

            frame = monta_mensagem(tam, seq, tipo, dados);

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
            while (frame_resp.tipo != 0x00 || frame_resp.marcadorInicio != 0x7E)
            {
                if (recvfrom(sockfd, &frame_resp, sizeof(frame_resp), 0, (struct sockaddr *)&sndr_addr, &addr_len) < 0) 
                {
                    perror("Erro de recebimento:");
                    return EXIT_FAILURE;
                }
                
                printf("---------FRAME RECEBIDO------------\n");
                print_frame(&frame_resp);
                printf("-----------------------------------\n");

                if (frame_resp.tipo == 0x00 && frame_resp.marcadorInicio == 0x7E)
                    break;

                printf("Mensagem que não é um ACK recebida, recebendo outra...\n");
                memset(&frame_resp, 0, sizeof(frame_t));
            }

            // Chegou aqui, é porque detectou um ACK
            printf("ACK recebido.\n");

            // Começa a receber os dados referentes ao comando "lista"
            while (frame_resp.tipo != 0x1E) // Até receber um frame do tipo == "fim_tx"
            {
                // !!!!!!!!!!!! CRC precisa ser calculado nas mensagens que chegam !!!!!!!!!!!!!!!!!

                if (recvfrom(sockfd, &frame_resp, sizeof(frame_resp), 0, (struct sockaddr *)&sndr_addr, &addr_len))
                {
                    // printf("---------FRAME RECEBIDO DPS DO ACK (LOOP FORA)------------\n");
                    // print_frame(&frame_resp);
                    // printf("-----------------------------------\n");
                }

                if (frame_resp.tipo == 0x1E) // envia ACK aqui?
                    break; 

                while (frame_resp.tipo != 0x12) // Até receber um frame do tipo == "dados"
                {
                    if (recvfrom(sockfd, &frame_resp, sizeof(frame_resp), 0, (struct sockaddr *)&sndr_addr, &addr_len))
                    {
                        // printf("---------FRAME RECEBIDO DPS DO ACK (LOOP DENTRO)------------\n");
                        // print_frame(&frame_resp);
                        // printf("-----------------------------------\n");
                    }

                    if (frame_resp.tipo == 0x1E) // envia ACK aqui?
                        break;  
                }

                if (frame_resp.tipo == 0x12) // envia ACK aqui?
                    printf("%s\n", frame_resp.dados);
            }

            printf("\nAcabou lista\n");
        }

        if (entrada == 6)
            break;
    }

    // Encerra o socket
    close(sockfd);

    fprintf(stdout, "\nConexão fechada\n\n");

    return EXIT_SUCCESS;
}
