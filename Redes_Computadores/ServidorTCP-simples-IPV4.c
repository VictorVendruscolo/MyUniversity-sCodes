#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int meu_socket, novo_socket, c;
    struct sockaddr_in servidor, cliente;
    char *mensagem;

    // 1. Criar o socket IPv4
    meu_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (meu_socket == -1) {
        perror("Erro ao criar o socket");
        return 1;
    }

    // 2. Configurar o endereço e a porta do servidor
    servidor.sin_family = AF_INET;
    servidor.sin_addr.s_addr = INADDR_ANY;
    servidor.sin_port = htons(8888);

    // 3. Vincular o socket à porta (Bind)
    if (bind(meu_socket, (struct sockaddr *)&servidor, sizeof(servidor)) < 0) {
        perror("Erro no bind");
        return 1;
    }
    puts("Bind executado com sucesso!");

    // 4. Escutar conexões recebidas (Fila máxima de 3)
    listen(meu_socket, 3);
    puts("Aguardando conexões na porta 8888...");

    // 5. Aceitar uma conexão do cliente
    c = sizeof(struct sockaddr_in);
    novo_socket = accept(meu_socket, (struct sockaddr *)&cliente, (socklen_t *)&c);
    if (novo_socket < 0) {
        perror("Erro ao aceitar conexão");
        return 1;
    }
    puts("Conexão aceita!");

    // 6. Responder ao cliente e fechar
    mensagem = "E aí meu chapa. Acabei de receber a sua conexão. Falows!\n";
    write(novo_socket, mensagem, strlen(mensagem));

    close(novo_socket);
    close(meu_socket);
    return 0;
}