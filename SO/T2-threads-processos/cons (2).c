#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>

int main()
{
	/* o tamanho (em bytes) do objeto de memória compartilhada */
	const int SIZE = 4096;

	/* nome do objeto de memória compartilhada */
	const char *name = "OS";

	/* descritor de arquivo da memória compartilhada */
	int shm_fd;

	/* ponteiro para o objeto de memória compartilhada */
	void *ptr;

	/* abre o objeto de memória compartilhada */
	shm_fd = shm_open(name, O_RDONLY, 0600);

	/* mapeia o objeto de memória compartilhada para a memória */
	ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);

	/* imprime endereço armazenado em ptr */
	printf("cons ptr: %p\n", ptr);

	/* lê a partir do objeto de memória compartilhada */
	printf("%s",(char *)ptr);

	/* remove o objeto de memória compartilhada */
	shm_unlink(name);
	
	return 0;
}
