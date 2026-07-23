#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <stdint.h>

void *thread_proc(void *arg);

int main(int argc, char *argv[]) {
    struct sockaddr_in sAddr;
    int listensock, novo_socket, resultado, valor = 1;
    pthread_t thread_id;

    listensock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    // SO_REUSEADDR permite reutilizar a porta imediatamente após fechar
    setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &valor, sizeof(valor));

    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(1972);
    sAddr.sin_addr.s_addr = INADDR_ANY;

    bind(listensock, (struct sockaddr *)&sAddr, sizeof(sAddr));
    
    if (listen(listensock, 5) < 0) {
        perror("Erro no listen");
        return 1;
    }

    puts("Servidor Multithread aguardando conexões na porta 1972...");

    while (1) {
        // Correção no código original: passamos a variável 'listensock' e não o nome da função 'listen'
        novo_socket = accept(listensock, NULL, NULL);
        if (novo_socket < 0) continue;

        // Dispara uma nova thread para atender o cliente
        resultado = pthread_create(&thread_id, NULL, thread_proc, (void *)(intptr_t)novo_socket);
        if (resultado != 0) {
            printf("Erro ao criar a thread!\n");
            close(novo_socket);
            continue;
        }

        // Evita a criação de threads "zumbis" na memória
        pthread_detach(thread_id);
        sched_yield(); // Cede tempo de CPU para a thread criada iniciar
    }

    close(listensock);
    return 0;
}

void *thread_proc(void *arg) {
    int sock = (int)(intptr_t)arg;
    char buffer[25];
    int n_lidos;

    printf("Thread filha %lu (PID %d) criada.\n", (unsigned long)pthread_self(), getpid());
    
    n_lidos = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (n_lidos > 0) {
        buffer[n_lidos] = '\0';
        printf("Recebido: %s\n", buffer);
        send(sock, buffer, n_lidos, 0); // Echo de volta
    }

    close(sock);
    printf("Thread filha %lu finalizada.\n", (unsigned long)pthread_self());
    return NULL;
}