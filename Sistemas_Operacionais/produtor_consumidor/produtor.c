#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define BUFFER_SIZE 10
#define SHM_NAME "/OS" // Nome do objeto de memória, "/" pro linux

// O "Registro" (Struct) para gerenciar a alocação de memória dentro do buffer
typedef struct {
    int buffer[BUFFER_SIZE];
    int in;
    int out;
} shared_data;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr,"Uso: %s <valor inteiro>\n", argv[0]);
        return -1;
    }
    
    unsigned n_itens = atoi(argv[1]);
    if (n_itens <= 0) {
        fprintf(stderr,"%d deve ser > 0\n", n_itens);
        return -1;
    }

    /* o tamanho (em bytes) do objeto de memória compartilhada */
    const int SIZE = sizeof(shared_data);
    /* nome do objeto de memória compartilhada */
    const char *name = SHM_NAME;
    /* descritor de arquivo da memória compartilhada */
    int shm_fd;
    /* ponteiro para o objeto de memória compartilhada (Ponteiro inicial) */
    void *ptr;
    /* Usar outro ponteiro com registro para manipular os dados */
    shared_data *shm_data;

    /* cria o objeto de memória compartilhada */
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Falha no shm_open");
        exit(-1);
    }

    /* configura o tamanho do objeto de memória compartilhada */
    ftruncate(shm_fd, SIZE);

    /* mapeia o objeto de memória compartilhada para a memória */
    ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Falha no mmap");
        exit(-1);
    }

    /* Converte o ponteiro genérico para o ponteiro com registro */
    shm_data = (shared_data *) ptr;

    /* Inicializa os índices no primeiro uso */
    shm_data->in = 0;
    shm_data->out = 0;

    printf("Produtor iniciado. Produzindo %u itens...\n", n_itens);

    /* Lógica de produção original adaptada para o ponteiro de registro */
    for(int i = n_itens - 1; i >= 0; i--) {
        usleep(100000);
        int next_produced = i;

        /* Espera ocupada (busy waiting) */
        while(((shm_data->in + 1) % BUFFER_SIZE ) == shm_data->out)
            ; // Aguarda espaço

        shm_data->buffer[shm_data->in] = next_produced;
        shm_data->in = (shm_data->in + 1) % BUFFER_SIZE;
    }

    printf("Produtor finalizou!\n");
    return 0;
}