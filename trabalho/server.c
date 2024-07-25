#include "libServer.h"

int main(int argc, char *argv[]) 
{
    int sockfd, bytesEscritos;
    unsigned char dados[TAM_DADOS], sequencia = 0;
    char videoBuffer[TAM_DADOS], caminhoCompleto[TAM_DADOS + 9];
    char *diretorio = "./videos/";

    FILE *arq, *arquivoTesteServer, *arquivoLocal;

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

    // Começa a receber os dados
    if (recv(sockfd, &frame, sizeof(frame), 0) < 0)
    {
        perror("Erro no recebimento:");
        return EXIT_FAILURE;
    }

    // LOOP PRINCIPAL DO SERVER
    while (1)
    {
        // recebeu um "LISTA"
        if (eh_lista(&frame))
        {
            if (!send_ack(sockfd, sequencia))
                return EXIT_FAILURE;

            sequencia = (sequencia + 1) % 32;
            
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
                    strncpy((char*)frame_resp.dados, entry->d_name, TAM_DADOS-1);

                    frame_resp.marcadorInicio = 0x7E;
                    frame_resp.tamanho = (unsigned char)strlen(entry->d_name);
                    frame_resp.sequencia = sequencia;
                    frame_resp.tipo = 0x12;
                    frame_resp.crc8 = calcula_crc(&frame_resp);

                    sequencia = (sequencia + 1) % 32;

                    printf("%s\n", frame_resp.dados);

                    if (send(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                    {
                        perror("Erro no envio:");
                        return EXIT_FAILURE;
                    }

                    if (!wait_ack(sockfd, &frame_resp, 1000))
                        return EXIT_FAILURE;
                }
            }

            closedir(dir);

            // Prepara FIM_TX
            memset(dados, 0, TAM_DADOS);
            frame_resp = monta_mensagem(0x00, sequencia, 0x1E, dados, 0);
            sequencia = (sequencia + 1) % 32;

            // Envia FIM_TX
            if (send(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
            {
                perror("Erro no envio:");
                return EXIT_FAILURE;
            }

            if (!wait_ack(sockfd, &frame_resp, 1000))
                return EXIT_FAILURE;
        }
        
        // recebeu um "BAIXAR"
        else if (eh_baixar(&frame))
        {
            if (verifica_crc(&frame))
            {
                if (!send_ack(sockfd, sequencia))
                    return EXIT_FAILURE;

                sequencia = (sequencia + 1) % 32;

                strncpy(videoBuffer, (char*)frame.dados, TAM_DADOS-1);
                snprintf(caminhoCompleto, TAM_DADOS + 9, "%s%s", diretorio, videoBuffer);

                printf("%s\n", caminhoCompleto);

                arq = fopen(caminhoCompleto, "rb");
                if (!arq)
                {
                    perror("Erro ao abrir o arquivo");
                    return EXIT_FAILURE;
                }

                arquivoTesteServer = fopen("ArquivoTesteServer", "wb");
                if (!arquivoTesteServer)
                {
                    perror("Erro ao abrir/criar o arquivo");
                    return EXIT_FAILURE;
                }

                arquivoLocal = fopen("ArquivoLocal", "wb");
                if (!arquivoLocal)
                {
                    perror("Erro ao abrir/criar o arquivo");
                    return EXIT_FAILURE;
                }

                while ((bytesEscritos = fread(dados, sizeof(unsigned char), TAM_DADOS-1, arq)) > 0)
                {
                    fwrite(dados, sizeof(unsigned char), (unsigned char)bytesEscritos, arquivoLocal);

                    // Prepara e envia o frame com os dados do vídeo
                    frame_resp = monta_mensagem((unsigned char)bytesEscritos, sequencia, 0x12, dados, 1);
                    sequencia = (sequencia + 1) % 32;

                    print_frame(&frame_resp, arquivoTesteServer);

                    if (send(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                    {
                        perror("Erro no envio:");
                        return EXIT_FAILURE;
                    }

                    if (!wait_ack(sockfd, &frame_resp, 1000))
                        return EXIT_FAILURE;
                }

                fclose(arq);
                fclose(arquivoTesteServer);
                fclose(arquivoLocal);

                // Prepara FIM_TX
                memset(dados, 0, TAM_DADOS);
                frame_resp = monta_mensagem(0x00, sequencia, 0x1E, dados, 0);
                sequencia = (sequencia + 1) % 32;

                // Envia FIM_TX
                if (send(sockfd, &frame_resp, sizeof(frame_resp), 0) < 0)
                {
                    perror("Erro no envio:");
                    return EXIT_FAILURE;
                }

                if (!wait_ack(sockfd, &frame_resp, 1000))
                    return EXIT_FAILURE;
            }
            else
            {
                if (!send_nack(sockfd, sequencia))
                    return EXIT_FAILURE;
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

        if (recv(sockfd, &frame, sizeof(frame), 0) < 0)
        {
            perror("Erro no recebimento:");
            return EXIT_FAILURE;
        }
    }

    // Close socket
    close(sockfd);

    printf("Conexão fechada\n\n");

    return EXIT_SUCCESS;
}
