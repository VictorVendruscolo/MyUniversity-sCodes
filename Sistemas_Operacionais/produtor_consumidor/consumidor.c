#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define BUFFER_SIZE 10

typedef struct {
    int buffer[BUFFER_SIZE];
    int in;
    int out;
} shared_data;

int main() {
    /* o tamanho (em bytes) do objeto de memória compartilhada */
    const int SIZE = sizeof(shared_data);
    /* nome do objeto de memória compartilhada */
    const char *name = "/OS";
    /* descritor de arquivo da memória compartilhada */
    int shm_fd;
    /* ponteiro para o objeto de memória compartilhada */
    void *ptr;
    shared_data *shm_data;

    /* abre o objeto de memória compartilhada (ajustado para RDWR para permitir atualizar o 'out') */
    shm_fd = shm_open(name, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Erro shm_open");
        exit(-1);
    }

    /* mapeia o objeto de memória compartilhada para a memória */
    ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Erro mmap");
        exit(-1);
    }

    shm_data = (shared_data *) ptr;

    /* lógica de consumo original */
    for(;;) {
        while(shm_data->in == shm_data->out)
            ; // Espera ocupada


        usleep(100000); // sleep adicionado para ver a concorrência

        
        int next_consumed = shm_data->buffer[shm_data->out];
        shm_data->out = (shm_data->out + 1) % BUFFER_SIZE;

        printf("%i ", next_consumed);
        fflush(stdout);

        if(next_consumed == 0)
            break;
    }
    printf("\n");

    /* remove o objeto de memória compartilhada */
    shm_unlink(name);

    return 0;
}