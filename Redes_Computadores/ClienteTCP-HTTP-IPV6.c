#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int meu_socket;
    struct sockaddr_in6 servidor;
    char *ipv6_addr = "2800:3f0:4001:80b::200e"; // Exemplo de IP do Google
    char *mensagem, resposta_servidor[2000];

    // 1. Criar o socket IPv6
    meu_socket = socket(AF_INET6, SOCK_STREAM, 0);
    if (meu_socket == -1) {
        perror("Erro ao criar o socket IPv6");
        exit(1);
    }

    // 2. Configurar endereço IPv6 e porta HTTP (80)
    memset(&servidor, 0, sizeof(servidor));
    servidor.sin6_family = AF_INET6;
    servidor.sin6_port = htons(80);

    // Convertendo IP de string para formato binário
    if (inet_pton(AF_INET6, ipv6_addr, &servidor.sin6_addr) <= 0) {
        perror("Endereço IPv6 inválido");
        exit(1);
    }

    // 3. Conectar ao servidor remoto
    if (connect(meu_socket, (struct sockaddr *)&servidor, sizeof(servidor)) == -1) {
        perror("Erro ao conectar ao servidor");
        exit(1);
    }

    // 4. Enviar requisição HTTP GET
    mensagem = "GET / HTTP/1.1\r\nHost: [2800:3f0:4001:80b::200e]\r\nConnection: close\r\n\r\n";
    if (send(meu_socket, mensagem, strlen(mensagem), 0) == -1) {
        perror("Erro ao enviar mensagem");
        exit(1);
    }

    // 5. Receber e imprimir a resposta
    int bytes_recebidos = recv(meu_socket, resposta_servidor, sizeof(resposta_servidor) - 1, 0);
    if (bytes_recebidos < 0) {
        perror("Erro ao receber resposta");
        exit(1);
    }
    resposta_servidor[bytes_recebidos] = '\0'; // Garante o fim da string C

    printf("Resposta do servidor:\n%s\n", resposta_servidor);

    // 6. Fechar a conexão
    close(meu_socket);
    return 0;
}