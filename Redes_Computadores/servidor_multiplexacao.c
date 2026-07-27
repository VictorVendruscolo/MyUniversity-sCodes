// Servidor que atende MÚLTIPLOS clientes usando MULTIPLEXAÇÃO (select()).
// Compilar: gcc servidor_multiplexacao.c -o servidor_multiplexacao
//
// Diferente do fork e do multithread, aqui existe UM ÚNICO processo e UMA
// ÚNICA thread. Em vez de criar um "ajudante" para cada cliente, o servidor
// usa select() para vigiar vários sockets ao mesmo tempo e só trata aquele
// que deu sinal (chegou conexão nova ou chegou mensagem).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAX_CLIENTES 30

int main()
{
    int meu_socket, novo_socket, valor;
    struct sockaddr_in servidor;
    int clientes[MAX_CLIENTES]; // descritores dos clientes (0 = posição livre)
    char buffer[25];

    fd_set conjunto; // conjunto de sockets que o select() vai vigiar
    int maior_fd;    // maior descritor: select precisa saber até onde olhar

    // Zera a lista de clientes.
    for (int i = 0; i < MAX_CLIENTES; i++)
        clientes[i] = 0;

    // 1) socket
    meu_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (meu_socket == -1) { perror("Erro: socket"); return 1; }

    // SO_REUSEADDR: permite reusar a porta logo após reiniciar o servidor.
    valor = 1;
    setsockopt(meu_socket, SOL_SOCKET, SO_REUSEADDR, &valor, sizeof(valor));

    servidor.sin_family = AF_INET;
    servidor.sin_addr.s_addr = INADDR_ANY;
    servidor.sin_port = htons(1972);

    // 2) bind
    if (bind(meu_socket, (struct sockaddr *)&servidor, sizeof(servidor)) < 0) {
        perror("Erro: bind"); return 1;
    }
    // 3) listen
    listen(meu_socket, 5);
    puts("Servidor (multiplexacao) aguardando conexoes na porta 1972...");

    while (1)
    {
        // Monta o conjunto de sockets a vigiar A CADA volta do laço:
        // o socket de escuta + todos os clientes ativos.
        FD_ZERO(&conjunto);
        FD_SET(meu_socket, &conjunto);
        maior_fd = meu_socket;

        for (int i = 0; i < MAX_CLIENTES; i++) {
            int sd = clientes[i];
            if (sd > 0) FD_SET(sd, &conjunto);
            if (sd > maior_fd) maior_fd = sd;
        }

        // 4) select() BLOQUEIA até algum socket do conjunto ter atividade e
        // marca no conjunto quais estão prontos.
        if (select(maior_fd + 1, &conjunto, NULL, NULL, NULL) < 0) {
            perror("Erro: select");
            continue;
        }

        // Caso 1: atividade no socket de escuta = chegou um cliente NOVO.
        if (FD_ISSET(meu_socket, &conjunto)) {
            novo_socket = accept(meu_socket, NULL, NULL);
            if (novo_socket >= 0) {
                printf("Novo cliente conectado (socket %d).\n", novo_socket);
                // Guarda o novo cliente na primeira posição livre.
                for (int i = 0; i < MAX_CLIENTES; i++) {
                    if (clientes[i] == 0) { clientes[i] = novo_socket; break; }
                }
            }
        }

        // Caso 2: atividade em algum cliente = chegou mensagem (ou ele saiu).
        for (int i = 0; i < MAX_CLIENTES; i++) {
            int sd = clientes[i];
            if (sd > 0 && FD_ISSET(sd, &conjunto)) {
                int n = recv(sd, buffer, sizeof(buffer) - 1, 0);
                if (n <= 0) {
                    // 0 = cliente desconectou; <0 = erro.
                    printf("Cliente do socket %d desconectado.\n", sd);
                    close(sd);
                    clientes[i] = 0;
                } else {
                    buffer[n] = '\0';
                    printf("Socket %d enviou: %s\n", sd, buffer);
                    send(sd, buffer, n, 0); // eco: devolve a mensagem
                }
            }
        }
    }

    return 0;
}
