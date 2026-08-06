/* Trabalho 2 - Sistemas Operacionais - Victor Rech Vendruscolo
 *
 * Arquitetura : Multiprocesso monothread (processo pai + 3 processos filhos).
 *
 * IPC         : exclusivamente memoria compartilhada (shm_open + mmap), nos
 *                moldes dos exemplos prod/cons do livro. Cada processo faz o
 *                seu proprio mmap, portanto o ponteiro inicial 'ptr' guarda
 *                enderecos diferentes em cada processo ainda que aponte para
 *                a mesma regiao. Por isso os "ponteiros" da lista encadeada L
 *                sao deslocamentos (indices) em relacao ao inicio da regiao,
 *                e nunca enderecos reais.
 *
 * Sincronizacao: apenas variaveis contadoras compartilhadas. Cada contadora
 *                tem UM UNICO ESCRITOR e um unico leitor, como o par in/out do
 *                produtor-consumidor do livro. Nao existe leitura-modificacao-
 *                escrita concorrente sobre nenhuma variavel, de modo que
 *                leituras e escritas comuns de inteiros alinhados bastam
 *                (espera ocupada). NAO ha semaforos, mutexes nem instrucoes
 *                atomicas.
 *
 * Memoria     : alocador proprio dentro da regiao compartilhada, gerenciado
 *                por mapa de bits. O mapa e criado e inicializado pelo processo
 *                pai, unico responsavel pela alocacao de nos; cada processo
 *                filho desaloca apenas os nos que ele mesmo remove de L.
 *
 * Paralelismo : os 4 processos progridem simultaneamente sobre a mesma lista L
 *                (encadeamento em pipeline), nunca uma etapa por vez.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define MAX_NOS      1000050u
#define FIM_DA_LISTA 0xFFFFFFFFu /* substitui o NULL no encadeamento         */
#define SENTINELA    0xFFFFFFFFu /* valor do ultimo no: marca fim da entrada */
#define NENHUM       0xFFFFFFFFu /* "nenhum no" para as variaveis locais     */
#define SHM_NOME     "/trabalho2_so_shm"

/* No da lista L: 'prox' e um deslocamento dentro da regiao compartilhada. */
typedef struct {
    unsigned int valor;
    unsigned int prox;
} No;

/* Registro que descreve todo o conteudo da regiao de memoria compartilhada. */
typedef struct {
    No            buffer_nos[MAX_NOS]; /* buffer proprio de nos             */
    unsigned char mapa_bits[MAX_NOS];  /* 0 = livre, 1 = ocupado            */

    /* Contadoras compartilhadas, cada uma com um unico processo escritor.
     * Contam quantos nos ja foram liberados para a etapa seguinte, impedindo
     * que um processo ultrapasse o anterior e leia L em estado inconsistente. */
    unsigned int cont_lidos;   /* escrita so pelo pai      */
    unsigned int cont_sem_par; /* escrita so pelo filho 1  */
    unsigned int cont_primos;  /* escrita so pelo filho 2  */

    unsigned int cabeca;       /* deslocamento do no cabeca de L */
} MemCompartilhada;

/* O mapa usa uma posicao por no (e nao bits empacotados) de proposito: com
 * bits empacotados, liberar um no exigiria ler-alterar-gravar um byte
 * compartilhado com outros 7 nos, o que so seria seguro com instrucao atomica.
 * Com uma posicao por no, cada posicao tem um unico escritor de cada vez: o
 * pai grava 1 ao alocar e apenas o filho que remove o no grava 0. */

/* ------------------------- acesso a memoria compartilhada ------------------ */

/* Mapeia a regiao compartilhada no espaco de enderecamento do processo e
 * devolve o ponteiro inicial (void *) da regiao. Cada processo guarda esse
 * ponteiro inicial em 'ptr' e trabalha com um segundo ponteiro, 'mem', que
 * sobrepoe o registro a mesma regiao. */
static void *mapeia_memoria(int criar)
{
    int   shm_fd;
    void *ptr;

    shm_fd = shm_open(SHM_NOME, criar ? (O_CREAT | O_RDWR) : O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    if (criar && ftruncate(shm_fd, sizeof(MemCompartilhada)) == -1) {
        perror("ftruncate");
        exit(1);
    }

    ptr = mmap(0, sizeof(MemCompartilhada), PROT_READ | PROT_WRITE,
               MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    close(shm_fd);

    return ptr;
}

/* ------------------------------ alocador ---------------------------------- */

/* Somente o processo pai chama esta funcao (unico responsavel por alocar). */
static unsigned int aloca_no(volatile MemCompartilhada *mem)
{
    static unsigned int ultimo = 0;

    for (;;) {
        for (unsigned int i = 0; i < MAX_NOS; i++) {
            unsigned int k = (ultimo + i) % MAX_NOS;
            if (mem->mapa_bits[k] == 0) {
                mem->mapa_bits[k] = 1;
                ultimo = (k + 1) % MAX_NOS;
                return k;
            }
        }
        /* buffer cheio: espera ocupada ate algum filho desalocar um no */
    }
}

/* Cada processo filho so desaloca os nos que ele mesmo removeu de L. */
static void desaloca_no(volatile MemCompartilhada *mem, unsigned int k)
{
    mem->mapa_bits[k] = 0;
}

/* ------------------------------ auxiliares -------------------------------- */

/* Testa divisores apenas ate a raiz quadrada de n (i <= n/i evita estouro). */
static int eh_primo(unsigned int n)
{
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    for (unsigned int i = 5; i <= n / i; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return 0;
    }
    return 1;
}

/* --------------------------- processos adicionais -------------------------- */

/* Filho 1: percorre L e remove os numeros pares maiores que 2.
 *
 * Um no mantido em L so e liberado para o filho 2 (cont_sem_par++) depois que
 * este processo ja avancou para o proximo no mantido. Nesse instante o campo
 * 'prox' do no liberado e definitivo, pois este processo nunca mais o altera.
 * E essa espera de um no que dispensa qualquer trava entre as etapas. */
static void processo_remove_pares(void)
{
    void *ptr = mapeia_memoria(0);                 /* ponteiro inicial da regiao */
    volatile MemCompartilhada *mem = ptr;          /* ponteiro de trabalho       */

    unsigned int ant        = mem->cabeca; /* ultimo no mantido por este processo */
    unsigned int a_liberar  = NENHUM;      /* no mantido aguardando liberacao     */
    unsigned int consumidos = 0;           /* contadora local de nos ja vistos    */

    for (;;) {
        while (consumidos == mem->cont_lidos)
            ; /* espera ocupada: o pai ainda nao liberou outro no */
        consumidos++;

        unsigned int atual = mem->buffer_nos[ant].prox;
        unsigned int valor = mem->buffer_nos[atual].valor;

        if (valor != SENTINELA && valor > 2 && valor % 2 == 0) {
            mem->buffer_nos[ant].prox = mem->buffer_nos[atual].prox; /* remove */
            desaloca_no(mem, atual);                                 /* libera */
        } else {
            ant = atual;
            if (a_liberar != NENHUM)
                mem->cont_sem_par++;
            a_liberar = atual;

            if (valor == SENTINELA) {
                mem->cont_sem_par++; /* libera a propria sentinela e encerra */
                break;
            }
        }
    }

    munmap(ptr, sizeof(MemCompartilhada));
    exit(0);
}

/* Filho 2: percorre L e remove os numeros que nao sao primos. */
static void processo_remove_nao_primos(void)
{
    void *ptr = mapeia_memoria(0);                 /* ponteiro inicial da regiao */
    volatile MemCompartilhada *mem = ptr;          /* ponteiro de trabalho       */

    unsigned int ant        = mem->cabeca;
    unsigned int a_liberar  = NENHUM;
    unsigned int consumidos = 0;

    for (;;) {
        while (consumidos == mem->cont_sem_par)
            ; /* espera ocupada: o filho 1 ainda nao liberou outro no */
        consumidos++;

        unsigned int atual = mem->buffer_nos[ant].prox;
        unsigned int valor = mem->buffer_nos[atual].valor;

        if (valor != SENTINELA && !eh_primo(valor)) {
            mem->buffer_nos[ant].prox = mem->buffer_nos[atual].prox;
            desaloca_no(mem, atual);
        } else {
            ant = atual;
            if (a_liberar != NENHUM)
                mem->cont_primos++;
            a_liberar = atual;

            if (valor == SENTINELA) {
                mem->cont_primos++;
                break;
            }
        }
    }

    munmap(ptr, sizeof(MemCompartilhada));
    exit(0);
}

/* Filho 3: imprime os numeros primos armazenados em L. */
static void processo_imprime_primos(void)
{
    void *ptr = mapeia_memoria(0);                 /* ponteiro inicial da regiao */
    volatile MemCompartilhada *mem = ptr;          /* ponteiro de trabalho       */

    unsigned int ant        = mem->cabeca;
    unsigned int consumidos = 0;

    FILE *saida = fopen("out.txt", "w");
    if (saida == NULL) {
        perror("out.txt");
        exit(1);
    }

    for (;;) {
        while (consumidos == mem->cont_primos)
            ; /* espera ocupada: o filho 2 ainda nao liberou outro no */
        consumidos++;

        unsigned int atual = mem->buffer_nos[ant].prox;
        unsigned int valor = mem->buffer_nos[atual].valor;

        if (valor == SENTINELA)
            break;

        fprintf(saida, "%u ", valor);
        ant = atual;
    }

    fclose(saida);
    munmap(ptr, sizeof(MemCompartilhada));
    exit(0);
}

/* ------------------------------ processo pai ------------------------------- */

int main(void)
{
    shm_unlink(SHM_NOME); /* limpeza preventiva de execucoes anteriores */

    void *ptr = mapeia_memoria(1);                 /* ponteiro inicial da regiao */
    volatile MemCompartilhada *mem = ptr;          /* ponteiro de trabalho       */

    /* Mapa de bits e contadoras criados e inicializados pelo processo pai. */
    memset(ptr, 0, sizeof(MemCompartilhada));

    /* Lista L com no cabeca: simplifica a remocao do primeiro elemento. */
    mem->cabeca = aloca_no(mem);
    mem->buffer_nos[mem->cabeca].valor = 0;
    mem->buffer_nos[mem->cabeca].prox  = FIM_DA_LISTA;

    /* Primeiro sao criados os 3 processos adicionais... */
    pid_t p1 = fork();
    if (p1 == 0) processo_remove_pares();

    pid_t p2 = fork();
    if (p2 == 0) processo_remove_nao_primos();

    pid_t p3 = fork();
    if (p3 == 0) processo_imprime_primos();

    /* ...e so depois o pai le in.txt e insere no final de L. */
    FILE *entrada = fopen("in.txt", "r");
    if (entrada == NULL) {
        perror("in.txt");
        return 1;
    }

    unsigned int fim       = mem->cabeca; /* ultimo no de L */
    unsigned int a_liberar = NENHUM;      /* no inserido aguardando liberacao */
    unsigned int valor;

    while (fscanf(entrada, "%u", &valor) == 1) {
        unsigned int novo = aloca_no(mem);
        mem->buffer_nos[novo].valor = valor;
        mem->buffer_nos[novo].prox  = FIM_DA_LISTA;

        mem->buffer_nos[fim].prox = novo;
        fim = novo;

        /* o no anterior ja tem 'prox' definitivo: pode ser lido pelo filho 1 */
        if (a_liberar != NENHUM)
            mem->cont_lidos++;
        a_liberar = novo;
    }
    fclose(entrada);

    /* Ultimo no com valor especial (max unsigned): avisa os demais processos
     * que a entrada terminou, dispensando qualquer outra variavel de controle. */
    unsigned int ultimo = aloca_no(mem);
    mem->buffer_nos[ultimo].valor = SENTINELA;
    mem->buffer_nos[ultimo].prox  = FIM_DA_LISTA;
    mem->buffer_nos[fim].prox = ultimo;
    fim = ultimo;

    if (a_liberar != NENHUM)
        mem->cont_lidos++;
    mem->cont_lidos++; /* libera a sentinela */

    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);
    waitpid(p3, NULL, 0);

    /* O processo pai destroi a lista que sobrou (primos + cabeca + sentinela). */
    unsigned int p = mem->cabeca;
    while (p != FIM_DA_LISTA) {
        unsigned int seguinte = mem->buffer_nos[p].prox;
        desaloca_no(mem, p);
        p = seguinte;
    }

    munmap(ptr, sizeof(MemCompartilhada));
    shm_unlink(SHM_NOME);

    return 0;
}
