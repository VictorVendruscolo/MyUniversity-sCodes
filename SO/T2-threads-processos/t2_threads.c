/* Trabalho 2 - Sistemas Operacionais - Victor Rech Vendruscolo
 *
 * Arquitetura : Monoprocesso multithread (4 threads: principal + 3 adicionais).
 *
 * Sincronizacao: apenas variaveis contadoras compartilhadas.
 *                Cada contador possui UM UNICO ESCRITOR e um unico leitor,
 *                como o par in/out do produtor-consumidor do livro. Por isso
 *                nao ha leitura-modificacao-escrita concorrente sobre nenhuma
 *                variavel e a sincronizacao funciona apenas com leituras e
 *                escritas comuns de inteiros alinhados (espera ocupada).
 *                NAO ha semaforos, mutexes, condicoes nem instrucoes atomicas.
 *
 * Memoria     : alocador proprio dentro de um buffer proprio, gerenciado por
 *                mapa de bits. Os nos da lista L nao guardam enderecos reais e
 *                sim o deslocamento (indice) em relacao ao inicio do buffer,
 *                mantendo o codigo identico ao da versao multiprocesso.
 *
 * Paralelismo : as 4 threads progridem simultaneamente sobre a mesma lista L
 *                (encadeamento em pipeline), nunca uma etapa por vez.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_NOS      1000050u   /* capacidade do buffer proprio de nos       */
#define FIM_DA_LISTA 0xFFFFFFFFu /* substitui o NULL no encadeamento         */
#define SENTINELA    0xFFFFFFFFu /* valor do ultimo no: marca fim da entrada */
#define NENHUM       0xFFFFFFFFu /* "nenhum no" para as variaveis locais     */

/* No da lista L: 'prox' e um deslocamento dentro do buffer, nao um endereco */
typedef struct {
    unsigned int valor;
    unsigned int prox;
} No;

/* ---------------------- regiao de dados compartilhada ---------------------- */

/* Buffer proprio de nos e o mapa de bits que controla sua ocupacao.
 * O mapa usa uma posicao por no (0 = livre, 1 = ocupado) de proposito:
 * com bits empacotados, liberar um no exigiria ler-alterar-gravar um byte
 * compartilhado com outros 7 nos, o que so seria seguro com instrucao atomica.
 * Com uma posicao por no, cada posicao tem um unico escritor de cada vez:
 * a thread principal grava 1 ao alocar e apenas a thread que remove o no grava 0. */
static volatile No            buffer_nos[MAX_NOS];
static volatile unsigned char mapa_bits[MAX_NOS];

/* Contadoras compartilhadas: cada uma e escrita por UMA UNICA thread.
 * Elas contam quantos nos ja foram liberados para a etapa seguinte, impedindo
 * que uma thread ultrapasse a anterior e leia L em estado inconsistente. */
static volatile unsigned int cont_lidos    = 0; /* escrita so pela principal */
static volatile unsigned int cont_sem_par  = 0; /* escrita so pela thread 1  */
static volatile unsigned int cont_primos   = 0; /* escrita so pela thread 2  */

static unsigned int cabeca; /* deslocamento do no cabeca de L */

/* ------------------------------ alocador ---------------------------------- */

/* Somente a thread principal chama esta funcao (unica responsavel por alocar). */
static unsigned int aloca_no(void)
{
    static unsigned int ultimo = 0;

    for (;;) {
        for (unsigned int i = 0; i < MAX_NOS; i++) {
            unsigned int k = (ultimo + i) % MAX_NOS;
            if (mapa_bits[k] == 0) {
                mapa_bits[k] = 1;
                ultimo = (k + 1) % MAX_NOS;
                return k;
            }
        }
        /* buffer cheio: espera ocupada ate alguma thread desalocar um no */
    }
}

/* Cada thread adicional so desaloca os nos que ela mesma removeu de L. */
static void desaloca_no(unsigned int k)
{
    mapa_bits[k] = 0;
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

/* --------------------------- threads adicionais ---------------------------- */

/* Thread 1: percorre L e remove os numeros pares maiores que 2.
 *
 * Um no mantido em L so e liberado para a thread 2 (cont_sem_par++) depois que
 * esta thread ja avancou para o proximo no mantido. Nesse instante o campo
 * 'prox' do no liberado e definitivo, pois esta thread nunca mais o altera.
 * E essa espera de um no que dispensa qualquer trava entre as etapas. */
static void *remove_pares(void *arg __attribute__((unused)))
{
    unsigned int ant      = cabeca; /* ultimo no mantido por esta thread */
    unsigned int a_liberar = NENHUM; /* no mantido aguardando liberacao  */
    unsigned int consumidos = 0;     /* contadora local de nos ja vistos */

    for (;;) {
        while (consumidos == cont_lidos)
            ; /* espera ocupada: a principal ainda nao liberou outro no */
        consumidos++;

        unsigned int atual = buffer_nos[ant].prox;
        unsigned int valor = buffer_nos[atual].valor;

        if (valor != SENTINELA && valor > 2 && valor % 2 == 0) {
            buffer_nos[ant].prox = buffer_nos[atual].prox; /* remove de L    */
            desaloca_no(atual);                            /* devolve ao mapa*/
        } else {
            ant = atual;
            if (a_liberar != NENHUM)
                cont_sem_par++;
            a_liberar = atual;

            if (valor == SENTINELA) {
                cont_sem_par++; /* libera a propria sentinela e encerra */
                break;
            }
        }
    }

    pthread_exit(NULL);
}

/* Thread 2: percorre L e remove os numeros que nao sao primos. */
static void *remove_nao_primos(void *arg __attribute__((unused)))
{
    unsigned int ant       = cabeca;
    unsigned int a_liberar = NENHUM;
    unsigned int consumidos = 0;

    for (;;) {
        while (consumidos == cont_sem_par)
            ; /* espera ocupada: a thread 1 ainda nao liberou outro no */
        consumidos++;

        unsigned int atual = buffer_nos[ant].prox;
        unsigned int valor = buffer_nos[atual].valor;

        if (valor != SENTINELA && !eh_primo(valor)) {
            buffer_nos[ant].prox = buffer_nos[atual].prox;
            desaloca_no(atual);
        } else {
            ant = atual;
            if (a_liberar != NENHUM)
                cont_primos++;
            a_liberar = atual;

            if (valor == SENTINELA) {
                cont_primos++;
                break;
            }
        }
    }

    pthread_exit(NULL);
}

/* Thread 3: imprime os numeros primos armazenados em L. */
static void *imprime_primos(void *arg __attribute__((unused)))
{
    unsigned int ant        = cabeca;
    unsigned int consumidos = 0;

    FILE *saida = fopen("out.txt", "w");
    if (saida == NULL) {
        perror("out.txt");
        pthread_exit(NULL);
    }

    for (;;) {
        while (consumidos == cont_primos)
            ; /* espera ocupada: a thread 2 ainda nao liberou outro no */
        consumidos++;

        unsigned int atual = buffer_nos[ant].prox;
        unsigned int valor = buffer_nos[atual].valor;

        if (valor == SENTINELA)
            break;

        fprintf(saida, "%u ", valor);
        ant = atual;
    }

    fclose(saida);
    pthread_exit(NULL);
}

/* ----------------------------- thread principal ---------------------------- */

int main(void)
{
    pthread_t t1, t2, t3;

    /* Mapa de bits criado e inicializado pela thread principal. */
    for (unsigned int i = 0; i < MAX_NOS; i++)
        mapa_bits[i] = 0;

    /* Lista L com no cabeca: simplifica a remocao do primeiro elemento. */
    cabeca = aloca_no();
    buffer_nos[cabeca].valor = 0;
    buffer_nos[cabeca].prox  = FIM_DA_LISTA;

    /* Primeiro sao criadas as 3 threads adicionais... */
    pthread_create(&t1, NULL, remove_pares, NULL);
    pthread_create(&t2, NULL, remove_nao_primos, NULL);
    pthread_create(&t3, NULL, imprime_primos, NULL);

    /* ...e so depois a principal le in.txt e insere no final de L. */
    FILE *entrada = fopen("in.txt", "r");
    if (entrada == NULL) {
        perror("in.txt");
        return 1;
    }

    unsigned int fim       = cabeca;  /* ultimo no de L */
    unsigned int a_liberar = NENHUM;  /* no inserido aguardando liberacao */
    unsigned int valor;

    while (fscanf(entrada, "%u", &valor) == 1) {
        unsigned int novo = aloca_no();
        buffer_nos[novo].valor = valor;
        buffer_nos[novo].prox  = FIM_DA_LISTA;

        buffer_nos[fim].prox = novo;
        fim = novo;

        /* o no anterior ja tem 'prox' definitivo: pode ser lido pela thread 1 */
        if (a_liberar != NENHUM)
            cont_lidos++;
        a_liberar = novo;
    }
    fclose(entrada);

    /* Ultimo no com valor especial (max unsigned): avisa as demais threads
     * que a entrada terminou, dispensando qualquer outra variavel de controle. */
    unsigned int ultimo = aloca_no();
    buffer_nos[ultimo].valor = SENTINELA;
    buffer_nos[ultimo].prox  = FIM_DA_LISTA;
    buffer_nos[fim].prox = ultimo;
    fim = ultimo;

    if (a_liberar != NENHUM)
        cont_lidos++;
    cont_lidos++; /* libera a sentinela */

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    /* A thread principal destroi a lista que sobrou (primos + cabeca + sentinela). */
    unsigned int p = cabeca;
    while (p != FIM_DA_LISTA) {
        unsigned int seguinte = buffer_nos[p].prox;
        desaloca_no(p);
        p = seguinte;
    }

    return 0;
}
