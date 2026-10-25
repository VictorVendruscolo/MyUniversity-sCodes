// Trabalho 6 - Sistemas Operacionais - Victor Rech Vendruscolo
// Conta os erros de pagina dos algoritmos de substituicao FIFO, OPT e LRU.

#include <stdio.h>
#include <stdlib.h>

// Le todos os enderecos do arquivo; o vetor inteiro e necessario porque o OPT olha o futuro
int *ler_enderecos(char *nome, int *n)
{
    FILE *f = fopen(nome, "r");
    if (f == NULL) {
        printf("erro ao abrir %s\n", nome);
        exit(1);
    }

    // primeira leitura conta os enderecos, a segunda guarda no vetor
    int x;
    *n = 0;
    while (fscanf(f, "%d", &x) == 1) (*n)++;
    rewind(f);

    int *end = malloc(*n * sizeof(int));
    int i = 0;
    while (fscanf(f, "%d", &x) == 1) end[i++] = x;
    fclose(f);
    return end;
}

// Devolve o indice do quadro que contem a pagina, ou -1
int buscar(int *quadros, int ocupados, int pagina)
{
    for (int k = 0; k < ocupados; k++) {
        if (quadros[k] == pagina) return k;
    }
    return -1;
}

// Fila FIFO (vetor circular): substitui a pagina da cabeca da fila e a nova entra na cauda
int fifo(int *end, int n, int tam_pag, int q, FILE *saida)
{
    int *quadros = malloc(q * sizeof(int));
    int ocupados = 0, cabeca = 0, erros = 0;

    for (int i = 0; i < n; i++) {
        int pagina = end[i] / tam_pag;
        if (buscar(quadros, ocupados, pagina) != -1) continue;

        erros++;
        fprintf(saida, "Erro de pagina no endereco %d, pagina %d\n", end[i], pagina);
        if (ocupados < q) { // ha quadro livre: usa ele
            quadros[ocupados++] = pagina;
        } else {
            quadros[cabeca] = pagina;
            cabeca = (cabeca + 1) % q;
        }
    }
    free(quadros);
    return erros;
}

// Substitui a pagina que nao sera usada pelo periodo de tempo mais longo
int opt(int *end, int n, int tam_pag, int q, FILE *saida)
{
    int *quadros = malloc(q * sizeof(int));
    int ocupados = 0, erros = 0;

    for (int i = 0; i < n; i++) {
        int pagina = end[i] / tam_pag;
        if (buscar(quadros, ocupados, pagina) != -1) continue;

        erros++;
        fprintf(saida, "Erro de pagina no endereco %d, pagina %d\n", end[i], pagina);
        if (ocupados < q) { // ha quadro livre: usa ele
            quadros[ocupados++] = pagina;
            continue;
        }

        int vitima = 0, mais_longe = -1;
        for (int k = 0; k < q; k++) {
            int j = i + 1;
            while (j < n && end[j] / tam_pag != quadros[k]) j++;
            if (j > mais_longe) { // empate: fica o quadro de menor indice
                mais_longe = j;
                vitima = k;
            }
        }
        quadros[vitima] = pagina;
    }
    free(quadros);
    return erros;
}

// LRU com contadores: a cada referencia, o relogio (i) e copiado para o campo hora-de-uso
// do quadro da pagina; substitui a pagina com o menor valor de hora
int lru(int *end, int n, int tam_pag, int q, FILE *saida)
{
    int *quadros = malloc(q * sizeof(int));
    int *hora_uso = malloc(q * sizeof(int));
    int ocupados = 0, erros = 0;

    for (int i = 0; i < n; i++) {
        int pagina = end[i] / tam_pag;
        int k = buscar(quadros, ocupados, pagina);

        if (k == -1) {
            erros++;
            fprintf(saida, "Erro de pagina no endereco %d, pagina %d\n", end[i], pagina);
            if (ocupados < q) { // ha quadro livre: usa ele
                k = ocupados++;
            } else {
                k = 0;
                for (int m = 1; m < q; m++) {
                    if (hora_uso[m] < hora_uso[k]) k = m;
                }
            }
            quadros[k] = pagina;
        }
        hora_uso[k] = i;
    }
    free(quadros);
    free(hora_uso);
    return erros;
}

// Percentual truncado em 2 casas (12,195 -> 12,19), como no exemplo do enunciado
void imprimir(char *nome, int erros, int n)
{
    int p = erros * 10000 / n;
    printf("%s: %d erros de pagina, taxa de %d,%02d%%\n", nome, erros, p / 100, p % 100);
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("uso: %s tam_pagina tam_memoria arquivo\n", argv[0]);
        return 1;
    }

    int tam_pag = atoi(argv[1]);
    int tam_mem = atoi(argv[2]);
    int q = tam_mem / tam_pag; // quadros inteiros; o que sobra da memoria nao forma quadro
    if (q < 1) {
        printf("memoria menor que uma pagina\n");
        return 1;
    }

    int n;
    int *end = ler_enderecos(argv[3], &n);
    if (n == 0) {
        printf("arquivo sem enderecos\n");
        return 1;
    }

    FILE *saida = fopen("erros.out", "w");
    if (saida == NULL) {
        printf("erro ao criar erros.out\n");
        return 1;
    }

    fprintf(saida, "FIFO:\n");
    int e_fifo = fifo(end, n, tam_pag, q, saida);
    fprintf(saida, "\nOPT:\n");
    int e_opt = opt(end, n, tam_pag, q, saida);
    fprintf(saida, "\nLRU:\n");
    int e_lru = lru(end, n, tam_pag, q, saida);
    fclose(saida);

    imprimir("FIFO", e_fifo, n);
    imprimir("OPT", e_opt, n);
    imprimir("LRU", e_lru, n);

    free(end);
    return 0;
}
