# RawSocketStreaming

Este projeto visa criar um protocolo de comunicação entre duas máquinas para implementar um sistema de Streaming operando no modo Cliente-Servidor.

## Informações úteis

- A interface **loopback** no Linux é conhecida como `lo` com o endereço IP `127.0.0.1`. Qualquer dado enviado para este endereço é recebido de volta.

### Cliente

Um cliente normalmente executa os seguintes passos para estabelecer comunicação com um servidor:

1. Cria um socket usando a chamada de sistema `socket`.
2. Conecta ao servidor através dos valores das variáveis da estrutura na criação do socket.
3. Envia e recebe dados através das chamadas de sistema `sendto` e `recvfrom`.
4. Encerra a comunicação fechando o socket com a chamada `close`.

### Servidor

Um servidor normalmente executa os seguintes passos para oferecer serviço aos clientes:

1. Cria um socket usando a chamada de sistema `socket`, utilizando a mesma função de criação de socket do cliente.
2. Associa um endereço ao socket usando a chamada de sistema `bind`.
3. Aceita a conexão do cliente através das variáveis dentro da função.
4. Envia e recebe dados através das chamadas de sistema `sendto` e `recvfrom`.
5. Encerra a comunicação com o cliente fechando o descritor da conexão (chamada `close`).

### Passos a seguir

1. ~~Estabelecer conexão entre `server` e `client`.~~
2. ~~Criar estrutura do `frame` que será enviado.~~
3. ~~Enviar `frame` do `client` para o `server`.~~
4. ~~Enviar resposta do `server` para o `client`. (ACK por exemplo).~~
5. ~~Implementar CRC-8 no `client` e no `server`.~~
6. ~~Implementar lógica do envio do NACK se o CRC-8 falhar.~~
7. ~~Fazer lógica do para-e-espera para envio da lista de filmes. (1 nome de filme por frame)~~
8. ~~Implementar o item `dados`, mandando o tipo dele e no campo dados o nome do filme, retornando uma descrição do filme (Não sei se é isso).~~ (não era)  
9. ~~Entender o que é o `mostra na tela`.~~
10. ~~Entender o que é o `descritor arquivo`.~~
11. ~~Fazer a transferência de arquivo do tipo `baixar`.~~
12. ~~Implementar a lógica do `timeout`.~~
13. [NOVO] Fazer o server enviar o `descritor arquivo`.
14. Implementar a janela deslizante.
15. Implementar e testar os erros possíveis.  
16. Testar em dois computadores.

## Atualizações e erros

* Primeiramente, focamos na criação do arquivo `server.c` e na implementação básica do Raw Socket.
* Em seguida, criamos o `client.c` com o objetivo de estabelecer a comunicação entre servidor e cliente, o que foi alcançado, apesar de alguns problemas observados:
    - [X] O cliente está recebendo dados do servidor de forma inconsistente.
        - Foi resolvido, o server tinha uma flag ativada que travava a função `sendto`, client tava recebendo lixo.   
    - [X] O server está copiando e enviando de volta as mensagens do client.
        - Após resolver o problema acima, o filtro das mensagens resolve esse problema.
    - [X] O servidor só aceita mensagens com mais de ~18 caracteres, caso contrário, ocorre um erro no cliente.
        - Este problema foi resolvido porque o frame já bate o tamanho mínimo.
    - [X] Às vezes, o cliente não consegue se conectar ao servidor sem reiniciar a conexão.
        - Este problema foi resolvido utilizando as funções corretas nos arquivos `server.c` e `client.c`.
    - [X] O servidor continua recebendo pacotes do cliente repetidamente, mesmo sem entrada de dados visível.
        - Este problema foi resolvido implementando uma verificação no servidor para analisar o "Marcador de Início" antes de processar os dados.
    - [X] A resposta do servidor para o cliente ainda está inconsistente. O cliente às vezes recebe um ACK do servidor, mas algumas mensagens enviadas são ecoadas de volta para ele. Às vezes, também recebe frames com lixo em todos os campos, resolvendo isso, será caminho livre para implementar as funções do trabalho.
        - Resolvido destravando a flag que estava no server, agora o server fica mandando a última mensagem enviada infinitamente.
