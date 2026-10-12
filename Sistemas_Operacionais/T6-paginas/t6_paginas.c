// Trabalho 6 - Sistemas Operacionais - Victor Rech Vendruscolo
// Conta os erros de pagina dos algoritmos de substituicao FIFO, OPT e LRU.

#include <stdio.h>
#include <stdlib.h>

// Le todos os enderecos do arquivo; o vetor inteiro e necessario porque o OPT olha o futuro
static int *ler_enderecos(char *nome, int *n)
{
    FILE *f = fopen(nome, "r");
    if (f == NULL) {
        perror(nome);
        exit(1);
    }

    int cap = 64, x;
    int *end = malloc(cap * sizeof(int));
    *n = 0;
    while (fscanf(f, "%d", &x) == 1) {
        if (*n == cap) {
            cap *= 2;
            end = realloc(end, cap * sizeof(int));
        }
        end[(*n)++] = x;
    }
    fclose(f);
    return end;
}

// Devolve o indice do quadro que contem a pagina, ou -1
static int buscar(int *quadros, int ocupados, int pagina)
{
    for (int k = 0; k < ocupados; k++) {
        if (quadros[k] == pagina) return k;
    }
    return -1;
}

// 'prox' anda em circulo: aponta sempre para a pagina que entrou ha mais tempo
static int fifo(int *end, int n, int tam_pag, int q, FILE *saida)
{
    int *quadros = malloc(q * sizeof(int));
    int ocupados = 0, prox = 0, erros = 0;

    for (int i = 0; i < n; i++) {
        int pagina = end[i] / tam_pag;
        if (buscar(quadros, ocupados, pagina) != -1) continue;

        erros++;
        fprintf(saida, "Erro de pagina no endereco %d, pagina %d\n", end[i], pagina);
        if (ocupados < q) {
            quadros[ocupados++] = pagina;
        } else {
            quadros[prox] = pagina;
            prox = (prox + 1) % q;
        }
    }
    free(quadros);
    return erros;
}

// Sai a pagina que demora mais para ser usada de novo (ou que nunca mais e usada)
static int opt(int *end, int n, int tam_pag, int q, FILE *saida)
{
    int *quadros = malloc(q * sizeof(int));
    int ocupados = 0, erros = 0;

    for (int i = 0; i < n; i++) {
        int pagina = end[i] / tam_pag;
        if (buscar(quadros, ocupados, pagina) != -1) continue;

        erros++;
        fprintf(saida, "Erro de pagina no endereco %d, pagina %d\n", end[i], pagina);
        if (ocupados < q) {
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

// Sai a pagina com o menor 'ultimo_uso', atualizado em todo acesso, com ou sem erro
static int lru(int *end, int n, int tam_pag, int q, FILE *saida)
{
    int *quadros = malloc(q * sizeof(int));
    int *ultimo_uso = malloc(q * sizeof(int));
    int ocupados = 0, erros = 0;

    for (int i = 0; i < n; i++) {
        int pagina = end[i] / tam_pag;
        int k = buscar(quadros, ocupados, pagina);

        if (k == -1) {
            erros++;
            fprintf(saida, "Erro de pagina no endereco %d, pagina %d\n", end[i], pagina);
            if (ocupados < q) {
                k = ocupados++;
            } else {
                k = 0;
                for (int m = 1; m < q; m++) {
                    if (ultimo_uso[m] < ultimo_uso[k]) k = m;
                }
            }
            quadros[k] = pagina;
        }
        ultimo_uso[k] = i;
    }
    free(quadros);
    free(ultimo_uso);
    return erros;
}

// Percentual truncado em 2 casas (12,195 -> 12,19), como no exemplo do enunciado
static void imprimir(char *nome, int erros, int n)
{
    int p = erros * 10000 / n;
    printf("%s: %d erros de pagina (%d,%02d%%)\n", nome, erros, p / 100, p % 100);
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <tam_pagina> <tam_memoria> <arquivo>\n", argv[0]);
        return 1;
    }

    int tam_pag = atoi(argv[1]);
    int tam_mem = atoi(argv[2]);
    int q = tam_mem / tam_pag; // quadros inteiros; o que sobra da memoria nao forma quadro
    if (q < 1) {
        fprintf(stderr, "A memoria precisa caber pelo menos uma pagina\n");
        return 1;
    }

    int n;
    int *end = ler_enderecos(argv[3], &n);

    FILE *saida = fopen("erros.out", "w");
    if (saida == NULL) {
        perror("erros.out");
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
