#include "libServer.h"

int cria_raw_socket(char* nome_interface_rede) 
{ 
    // Cria o Raw Socket
    int soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (soquete == -1) 
    { 
        fprintf(stderr, "Erro ao criar socket: Verifique se você é root!\n");
        exit(-1); 
    }
    printf("Soquete criado\n");

    int ifindex = if_nametoindex(nome_interface_rede);

    struct sockaddr_ll endereco = {0};
    endereco.sll_family = AF_PACKET;
    endereco.sll_protocol = htons(ETH_P_ALL);
    endereco.sll_ifindex = ifindex;

    // Bind the socket to the network interface
    if (bind(soquete, (struct sockaddr *)&endereco, sizeof(endereco)) == -1)
    {
        perror("Erro ao fazer bind no socket\n");
        exit(-1);
    }

    struct packet_mreq mr = {0};
    mr.mr_ifindex = ifindex;
    mr.mr_type = PACKET_MR_PROMISC;

    // Coloca o socket no modo promíscuo (Não joga fora o que identifica como lixo)
    if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) 
    {
        fprintf(stderr, "Erro ao fazer setsockopt: "
            "Verifique se a interface de rede foi especificada corretamente.\n");
        exit(-1);
    }

    return soquete;
}

void inicializa_frame(frame_t *frame) 
{
    memset(frame, 0, sizeof(frame_t));
    memset(frame->dados, 0, TAM_DADOS);
}

void listar_videos(const char *diretorio)
{
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

frame_t monta_mensagem(unsigned char tam, unsigned char sequencia, unsigned char tipo, unsigned char* dados, int crc_flag)
{
    frame_t frame;
    
    frame.marcadorInicio = 0x7E; // (0111 1110)
    frame.tamanho = tam;
    frame.sequencia = sequencia;
    frame.tipo = tipo;

    memset(frame.dados, 0, TAM_DADOS);
    if (dados != NULL)
        strncpy((char*)frame.dados, (char*)dados, TAM_DADOS - 1);

    // caso seja necessário, calcula o crc
    if (crc_flag){
        frame.crc8 = calcula_crc(&frame);
    } else {
        frame.crc8 = 0x00;
    }

    return frame;
}

void send_ack()
{
}

// Função interna para imprimir os bits de cada byte
void print_bits(unsigned char byte, int num_bits) 
{
    for (int i = num_bits - 1; i >= 0; --i)
        printf("%d", (byte >> i) & 1);
}

void print_frame(frame_t *frame) 
{
    printf("\nMarcador de inicio: ");
    print_bits(frame->marcadorInicio, 8);
    printf("\n");

    printf("Tamanho: ");
    print_bits(frame->tamanho, 6);
    printf("\n");

    printf("Sequência: ");
    print_bits(frame->sequencia, 5);
    printf("\n");

    printf("Tipo: ");
    print_bits(frame->tipo, 5);
    printf("\n");

    printf("Dados: ");
    for (int i = 0; i < TAM_DADOS; ++i) {
        print_bits(frame->dados[i], 8);
        printf(" ");
    }
    printf("\n");

    printf("Crc-8: ");
    print_bits(frame->crc8, 8);
    printf("\n\n");
}

unsigned char calcula_crc(frame_t *frame)
{
    unsigned char gerador = 0x07; // 8 bits, porque o bit mais significativo é implicitamente 1
    unsigned char crc = 0x00; // inicialmente é 0;

    for (unsigned int i = 0; i < frame->tamanho; i++)
    {
        crc ^= frame->dados[i];

        for (unsigned int j = 0; j < 8; j++)
        {
            if (crc & 0x80){
                crc = (crc << 1) ^ gerador;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

int verifica_crc(frame_t *frame)
{
    if (frame->crc8 == calcula_crc(frame))
        return 1;
    
    return 0;
}

int eh_valida(frame_t *frame)
{
    if (frame->marcadorInicio != 0x7E)
        return 0;
    return 1;
}

int eh_ack(frame_t *frame)
{
    if (frame->marcadorInicio != 0x7E || frame->tipo != 0x00)
        return 0;
    return 1;
}

int eh_nack(frame_t *frame)
{
    if (frame->marcadorInicio != 0x7E || frame->tipo != 0x01)
        return 0;
    return 1;
}

int eh_fimtx(frame_t *frame)
{
    if (frame->marcadorInicio != 0x7E || frame->tipo != 0x1E)
        return 0;
    return 1;
}

int eh_lista(frame_t *frame)
{
    if (frame->tipo != 0x0A)
        return 0;
    return 1;
}

int eh_baixar(frame_t *frame)
{
    if (frame->tipo != 0x0B)
        return 0;
    return 1;
}

int eh_dados(frame_t *frame)
{
    if (frame->tipo != 0x12)
        return 0;
    return 1;
}
