/* Trabalho 4 - Sistemas Operacionais - Victor Rech Vendruscolo
 * Multithread com sincronizacao por semaforos de contagem nomeados. */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#define SENTINELA 0xFFFFFFFFu /* valor do ultimo no: marca o fim da entrada */

#define SEM_L1 "/t4_l1"
#define SEM_L2 "/t4_l2"
#define SEM_L3 "/t4_l3"

typedef struct No {
    unsigned int valor;
    struct No   *prox;
} No;

static No *L1, *L2, *L3; /* nos cabeca */

static sem_t *sem_l1, *sem_l2, *sem_l3;

static No *novo_no(unsigned int valor)
{
    No *n = malloc(sizeof(No));
    if (n == NULL) {
        perror("malloc");
        exit(1);
    }
    n->valor = valor;
    n->prox  = NULL;
    return n;
}

static void destroi(No *cabeca)
{
    while (cabeca != NULL) {
        No *seguinte = cabeca->prox;
        free(cabeca);
        cabeca = seguinte;
    }
}

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

/* Le L1 e cria L2 sem os pares maiores que 2. */
static void *cria_l2(void *arg __attribute__((unused)))
{
    No *atual  = L1;
    No *ultimo = L2;

    for (;;) {
        sem_wait(sem_l1);
        atual = atual->prox;
        unsigned int valor = atual->valor;

        if (valor == SENTINELA || !(valor > 2 && valor % 2 == 0)) {
            ultimo->prox = novo_no(valor); /* liga o no antes de anunciar */
            ultimo = ultimo->prox;
            sem_post(sem_l2);
        }

        if (valor == SENTINELA)
            break;
    }

    pthread_exit(NULL);
}

/* Le L2 e cria L3 sem os nao primos. */
static void *cria_l3(void *arg __attribute__((unused)))
{
    No *atual  = L2;
    No *ultimo = L3;

    for (;;) {
        sem_wait(sem_l2);
        atual = atual->prox;
        unsigned int valor = atual->valor;

        if (valor == SENTINELA || eh_primo(valor)) {
            ultimo->prox = novo_no(valor);
            ultimo = ultimo->prox;
            sem_post(sem_l3);
        }

        if (valor == SENTINELA)
            break;
    }

    pthread_exit(NULL);
}

/* Le L3 e imprime os primos. */
static void *imprime_l3(void *arg __attribute__((unused)))
{
    No   *atual = L3;
    FILE *saida = fopen("out.txt", "w");

    if (saida == NULL) {
        perror("out.txt");
        pthread_exit(NULL);
    }

    for (;;) {
        sem_wait(sem_l3);
        atual = atual->prox;

        if (atual->valor == SENTINELA)
            break;

        fprintf(saida, "%u ", atual->valor);
    }

    fclose(saida);
    pthread_exit(NULL);
}

int main(void)
{
    pthread_t t1, t2, t3;

    L1 = novo_no(0);
    L2 = novo_no(0);
    L3 = novo_no(0);

    /* semaforos nomeados persistem apos a execucao: remove antes de criar */
    sem_unlink(SEM_L1);
    sem_unlink(SEM_L2);
    sem_unlink(SEM_L3);

    sem_l1 = sem_open(SEM_L1, O_CREAT, 0644, 0);
    sem_l2 = sem_open(SEM_L2, O_CREAT, 0644, 0);
    sem_l3 = sem_open(SEM_L3, O_CREAT, 0644, 0);
    if (sem_l1 == SEM_FAILED || sem_l2 == SEM_FAILED || sem_l3 == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    pthread_create(&t1, NULL, cria_l2, NULL);
    pthread_create(&t2, NULL, cria_l3, NULL);
    pthread_create(&t3, NULL, imprime_l3, NULL);

    FILE *entrada = fopen("in.txt", "r");
    if (entrada == NULL) {
        perror("in.txt");
        return 1;
    }

    No          *ultimo = L1;
    unsigned int valor;

    while (fscanf(entrada, "%u", &valor) == 1) {
        ultimo->prox = novo_no(valor);
        ultimo = ultimo->prox;
        sem_post(sem_l1);
    }
    fclose(entrada);

    ultimo->prox = novo_no(SENTINELA);
    sem_post(sem_l1);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    destroi(L1);
    destroi(L2);
    destroi(L3);

    sem_close(sem_l1);
    sem_close(sem_l2);
    sem_close(sem_l3);
    sem_unlink(SEM_L1);
    sem_unlink(SEM_L2);
    sem_unlink(SEM_L3);

    return 0;
}
