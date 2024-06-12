# RawSocketStreaming
Criação de um protocolo de comunicação entre duas máquinas visando implementar um sistema de Streaming que opere no modo Cliente-Servidor.

## Informações úteis
- A interface **loopback** no Linux é conhecida como lo com o endereço de IP 127.0.0.1. Quando você manda algo pra esse endereço, ele manda de volta pra você

 Um cliente executa normalmente os seguintes passos para estabelecer uma comunicação com um servidor:

    Cria um socket, usando a chamada de sistema socket
    Conecta seu socket ao endereço do servidor, usando a chamada de sistema connect
    Envia e recebe dados através do socket, usando as chamadas de sistema read e write
    Encerra a comunicação, fechando o socket através da chamada close.

Um servidor normalmente executa os seguintes passos para oferecer serviço a seus clientes:

    Cria um socket, usando a chamada de sistema socket
    Associa um endereço a seu socket, usando a chamada de sistema bind.
    Coloca o socket em modo de escuta, através da chamada de sistema listen.
    Aguarda um pedido de conexão, através da chamada accept (que gera um descritor específico para a conexão recebida).
    Envia e recebe dados através do socket, usando as chamadas de sistema read e write.
    Encerra a comunicação com aquele cliente, fecha o descritor da conexão (chamada close).
    Volta ao passo 4, ou encerra suas atividades fechando seu socket (chamada close).

## Atualizações
* Por primeiro, focamos em criar o arquivo 'server.c' e fazer a criação do Raw Socket funcionar.
* Em seguida, criamos o 'client.c' e o objetivo agora foi estabelecer a comunicação entre servidor e cliente, foi feito, porém alguns problemas notados.
- [ ] Server copia e responde as mensagens enviadas pelo Client no primeiro momento da conexão.
- [ ] Server só aceita mensagens que tenham mais de ~18 caracteres, senão não envia e da erro no client.
- [ ] Às vezes Client não se conecta com o Server, precisando reiniciar esse para que a conexão funcione.

  
