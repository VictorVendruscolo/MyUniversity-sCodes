#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

// Manipulador de sinal para limpar processos filhos encerrados (evita zombies)
void sigchld_handler(int signo) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[]) {
    struct sockaddr_in sAddr;
    int meu_socket, novo_socket, leitor, pid, valor = 1;
    char buffer[25];

    meu_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    setsockopt(meu_socket, SOL_SOCKET, SO_REUSEADDR, &valor, sizeof(valor));

    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(1972);
    sAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(meu_socket, (struct sockaddr *)&sAddr, sizeof(sAddr)) < 0) {
        perror("Erro no bind");
        return 1;
    }

    if (listen(meu_socket, 5) < 0) {
        perror("Erro no listen");
        return 1;
    }

    // Registra o tratador de sinal para SIGCHLD
    signal(SIGCHLD, sigchld_handler);

    puts("Servidor Multiprocesso (fork) aguardando conexões na porta 1972...");

    while (1) {
        novo_socket = accept(meu_socket, NULL, NULL);
        if (novo_socket < 0) continue;

        pid = fork();
        
        if (pid == 0) { 
            // --- PROCESSO FILHO ---
            printf("Processo filho criado (PID %d).\n", getpid());
            close(meu_socket); // O filho não precisa do socket de escuta do pai

            leitor = recv(novo_socket, buffer, sizeof(buffer) - 1, 0);
            if (leitor > 0) {
                buffer[leitor] = '\0';
                printf("Filho %d leu: %s\n", getpid(), buffer);
                send(novo_socket, buffer, leitor, 0); // Echo
            }

            close(novo_socket);
            printf("Processo filho %d encerrado.\n", getpid());
            exit(0); // Encerra apenas o processo filho
        }

        // --- PROCESSO PAI ---
        close(novo_socket); // O pai fecha a sua referência da conexão do cliente
    }

    close(meu_socket);
    return 0;
}