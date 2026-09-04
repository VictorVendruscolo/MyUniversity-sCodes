/* Trabalho 5 - Sistemas Operacionais - Victor Rech Vendruscolo
 * Multithread com sincronizacao por mutex, um por no da lista L. */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SENTINELA 0xFFFFFFFFu /* valor do ultimo no: marca o fim da entrada */

typedef struct No {
    unsigned int     valor;
    struct No       *prox;
    pthread_mutex_t  mutex;
} No;

static No *cabeca;

/* Todo no nasce travado por quem o cria; so e destravado quando seu 'prox'
 * fica definitivo (lock coupling: nunca se trava 'atual' sem ja segurar
 * 'ant', e nenhuma thread consegue travar um no antes disso acontecer). */
static No *novo_no(unsigned int valor)
{
    No *n = malloc(sizeof(No));
    if (n == NULL) {
        perror("malloc");
        exit(1);
    }
    n->valor = valor;
    n->prox  = NULL;
    pthread_mutex_init(&n->mutex, NULL);
    pthread_mutex_lock(&n->mutex);
    return n;
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

/* Remove de L os pares maiores que 2. */
static void *remove_pares(void *arg __attribute__((unused)))
{
    No *ant = cabeca;
    pthread_mutex_lock(&ant->mutex);

    for (;;) {
        No *atual = ant->prox;
        pthread_mutex_lock(&atual->mutex);
        unsigned int valor = atual->valor;

        if (valor == SENTINELA) {
            pthread_mutex_unlock(&ant->mutex);
            pthread_mutex_unlock(&atual->mutex);
            break;
        }

        if (valor > 2 && valor % 2 == 0) {
            ant->prox = atual->prox;
            pthread_mutex_unlock(&atual->mutex);
            pthread_mutex_destroy(&atual->mutex);
            free(atual);
        } else {
            pthread_mutex_unlock(&ant->mutex);
            ant = atual;
        }
    }

    pthread_exit(NULL);
}

/* Remove de L os nao primos. */
static void *remove_nao_primos(void *arg __attribute__((unused)))
{
    No *ant = cabeca;
    pthread_mutex_lock(&ant->mutex);

    for (;;) {
        No *atual = ant->prox;
        pthread_mutex_lock(&atual->mutex);
        unsigned int valor = atual->valor;

        if (valor == SENTINELA) {
            pthread_mutex_unlock(&ant->mutex);
            pthread_mutex_unlock(&atual->mutex);
            break;
        }

        if (!eh_primo(valor)) {
            ant->prox = atual->prox;
            pthread_mutex_unlock(&atual->mutex);
            pthread_mutex_destroy(&atual->mutex);
            free(atual);
        } else {
            pthread_mutex_unlock(&ant->mutex);
            ant = atual;
        }
    }

    pthread_exit(NULL);
}

/* Imprime os primos armazenados em L. */
static void *imprime_primos(void *arg __attribute__((unused)))
{
    No *ant = cabeca;
    pthread_mutex_lock(&ant->mutex);

    FILE *saida = fopen("out.txt", "w");
    if (saida == NULL) {
        perror("out.txt");
        pthread_mutex_unlock(&ant->mutex);
        pthread_exit(NULL);
    }

    for (;;) {
        No *atual = ant->prox;
        pthread_mutex_lock(&atual->mutex);
        unsigned int valor = atual->valor;

        if (valor == SENTINELA) {
            pthread_mutex_unlock(&ant->mutex);
            pthread_mutex_unlock(&atual->mutex);
            break;
        }

        if (eh_primo(valor))
            fprintf(saida, "%u ", valor);

        pthread_mutex_unlock(&ant->mutex);
        ant = atual;
    }

    fclose(saida);
    pthread_exit(NULL);
}

int main(void)
{
    pthread_t t1, t2, t3;

    cabeca = novo_no(0);

    pthread_create(&t1, NULL, remove_pares, NULL);
    pthread_create(&t2, NULL, remove_nao_primos, NULL);
    pthread_create(&t3, NULL, imprime_primos, NULL);

    FILE *entrada = fopen("in.txt", "r");
    if (entrada == NULL) {
        perror("in.txt");
        return 1;
    }

    No *fim = cabeca;
    unsigned int valor;

    while (fscanf(entrada, "%u", &valor) == 1) {
        No *novo = novo_no(valor);
        fim->prox = novo;
        pthread_mutex_unlock(&fim->mutex); /* fim->prox agora e definitivo */
        fim = novo;
    }
    fclose(entrada);

    No *sentinela = novo_no(SENTINELA);
    fim->prox = sentinela;
    pthread_mutex_unlock(&fim->mutex);
    pthread_mutex_unlock(&sentinela->mutex); /* sentinela nunca tera prox */

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    No *p = cabeca;
    while (p != NULL) {
        No *seguinte = p->prox;
        pthread_mutex_destroy(&p->mutex);
        free(p);
        p = seguinte;
    }

    return 0;
}
