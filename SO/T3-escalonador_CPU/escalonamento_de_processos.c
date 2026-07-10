/*
 * Simulador de escalonamento de processos
 * Disciplina: Sistemas Operacionais
 * Base teorica: Fundamentos de Sistemas Operacionais (Silberschatz) - Capitulo 7
 *
 * Algoritmos simulados: FCFS, SJF, SRTF, Prioridade Preemptivo e Round-Robin.
 *
 * Uso: ./escalonador <arquivo_entrada> <quantum_ms> [-seq]
 *
 * Observacao sobre prioridade: neste trabalho o valor da prioridade e
 * invertido em relacao ao livro: quanto MAIOR o numero, MAIOR a prioridade
 * (1 = menor prioridade, 10 = maior prioridade).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_PROCESSOS 100
#define MAX_RAJADAS   64
#define MAX_LINHA     1024
#define MAX_SEGMENTOS 100000

/* Algoritmos de escalonamento suportados */
typedef enum {
    ALG_FCFS,
    ALG_SJF,
    ALG_SRTF,
    ALG_PRIORIDADE,
    ALG_ROUND_ROBIN,
    QTD_ALGORITMOS
} Algoritmo;

/* Estrutura de um processo, lida do arquivo de entrada mais o estado
 * necessario para a simulacao do escalonamento (reiniciado a cada algoritmo) */
typedef struct {
    int id;                       /* identificador do processo (1..n) */
    int prioridade;               /* 1 (menor) a 10 (maior) */
    int chegada;                  /* instante de submissao, em ms */
    int rajadas[MAX_RAJADAS];     /* rajadas de CPU e E/S alternadas, iniciando em CPU */
    int n_rajadas;                /* quantidade de rajadas informadas */

    /* estado da simulacao */
    int indice_rajada;            /* proxima rajada de CPU a ser executada */
    int tempo_restante;           /* tempo restante da rajada de CPU atual */
    int duracao_es_pendente;      /* duracao da E/S a ser atendida (modo sequencial) */
    int tempo_fim_es;             /* instante em que a E/S em andamento termina */
    int tempo_espera;             /* tempo total acumulado na fila de prontos */
    int tempo_termino;            /* instante de termino do processo */
    bool chegou;                  /* ja chegou ao sistema? */
    bool em_es;                   /* esta em servico de E/S? */
    bool finalizado;              /* ja terminou sua execucao? */
} Processo;

/* Um segmento do diagrama de Gantt: processo (-1 = ociosa) e instante final */
typedef struct {
    int processo_id;
    int tempo_fim;
} SegmentoGantt;

/* Fila generica de indices de processos (usada como fila de prontos e fila de E/S) */
typedef struct {
    int itens[MAX_PROCESSOS];
    int tamanho;
} Fila;

typedef struct {
    SegmentoGantt segmentos[MAX_SEGMENTOS];
    int n_segmentos;
    double utilizacao_cpu;
    double throughput;
    int tempo_espera[MAX_PROCESSOS];
    int tempo_turnaround[MAX_PROCESSOS];
    double espera_media;
    double turnaround_media;
} ResultadoSimulacao;

/* ---------------------- operacoes de fila ---------------------- */

static void fila_inicializar(Fila *f) {
    f->tamanho = 0;
}

static bool fila_vazia(Fila *f) {
    return f->tamanho == 0;
}

static void fila_adicionar(Fila *f, int idx) {
    f->itens[f->tamanho++] = idx;
}

/* Remove o elemento na posicao "pos" e retorna o indice de processo removido */
static int fila_remover(Fila *f, int pos) {
    int valor = f->itens[pos];
    for (int i = pos; i < f->tamanho - 1; i++) {
        f->itens[i] = f->itens[i + 1];
    }
    f->tamanho--;
    return valor;
}

/* Posicao, dentro da fila, do processo com menor tempo restante de CPU.
 * Em caso de empate, prevalece o que estiver mais proximo do inicio da fila. */
static int posicao_menor_tempo_restante(Processo *proc, Fila *f) {
    int melhor_pos = 0;
    for (int i = 1; i < f->tamanho; i++) {
        if (proc[f->itens[i]].tempo_restante < proc[f->itens[melhor_pos]].tempo_restante) {
            melhor_pos = i;
        }
    }
    return melhor_pos;
}

/* Posicao, dentro da fila, do processo com maior prioridade (maior numero).
 * Em caso de empate, prevalece o que estiver mais proximo do inicio da fila. */
static int posicao_maior_prioridade(Processo *proc, Fila *f) {
    int melhor_pos = 0;
    for (int i = 1; i < f->tamanho; i++) {
        if (proc[f->itens[i]].prioridade > proc[f->itens[melhor_pos]].prioridade) {
            melhor_pos = i;
        }
    }
    return melhor_pos;
}

/* ---------------------- leitura do arquivo de entrada ---------------------- */

/* Le o arquivo de entrada e preenche o vetor de processos. Retorna a
 * quantidade de processos lidos. */
static int ler_processos(const char *caminho, Processo processos[MAX_PROCESSOS]) {
    FILE *arquivo = fopen(caminho, "r");
    if (!arquivo) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo de entrada '%s'\n", caminho);
        exit(1);
    }

    char linha[MAX_LINHA];
    int n = 0;

    while (fgets(linha, sizeof(linha), arquivo)) {
        /* ignora linhas em branco */
        bool tem_conteudo = false;
        for (char *c = linha; *c; c++) {
            if (*c != ' ' && *c != '\t' && *c != '\n' && *c != '\r') {
                tem_conteudo = true;
                break;
            }
        }
        if (!tem_conteudo) continue;

        int valores[MAX_RAJADAS + 2];
        int qtd_valores = 0;
        char *ptr = linha;
        char *fim;
        while (qtd_valores < MAX_RAJADAS + 2) {
            long v = strtol(ptr, &fim, 10);
            if (fim == ptr) break; /* nenhum numero encontrado, fim da linha */
            valores[qtd_valores++] = (int) v;
            ptr = fim;
        }

        if (qtd_valores < 3) continue; /* linha invalida: precisa prioridade, chegada e ao menos 1 rajada */

        Processo *p = &processos[n];
        p->id = n + 1;
        p->prioridade = valores[0];
        p->chegada = valores[1];
        p->n_rajadas = qtd_valores - 2;
        for (int i = 0; i < p->n_rajadas; i++) {
            p->rajadas[i] = valores[2 + i];
        }
        n++;
    }

    fclose(arquivo);
    return n;
}

/* Reinicia o estado de simulacao de todos os processos para uma nova execucao
 * de algoritmo, preservando os dados originais (prioridade, chegada, rajadas). */
static void reiniciar_estado(Processo processos[MAX_PROCESSOS], int n) {
    for (int i = 0; i < n; i++) {
        processos[i].indice_rajada = 0;
        processos[i].tempo_restante = 0;
        processos[i].duracao_es_pendente = 0;
        processos[i].tempo_fim_es = 0;
        processos[i].tempo_espera = 0;
        processos[i].tempo_termino = 0;
        processos[i].chegou = false;
        processos[i].em_es = false;
        processos[i].finalizado = false;
    }
}

/* ---------------------- diagrama de Gantt ---------------------- */

/* Registra a execucao de "processo_id" (-1 para CPU ociosa) no instante t,
 * estendendo o ultimo segmento se for o mesmo processo, ou criando um novo. */
static void registrar_gantt(ResultadoSimulacao *r, int processo_id, int tempo_fim) {
    if (r->n_segmentos > 0 && r->segmentos[r->n_segmentos - 1].processo_id == processo_id) {
        r->segmentos[r->n_segmentos - 1].tempo_fim = tempo_fim;
    } else {
        r->segmentos[r->n_segmentos].processo_id = processo_id;
        r->segmentos[r->n_segmentos].tempo_fim = tempo_fim;
        r->n_segmentos++;
    }
}

/* ---------------------- simulacao ---------------------- */

/* Simula o escalonamento dos processos de acordo com o algoritmo informado,
 * preenchendo o diagrama de Gantt e as estatisticas em "r". */
static void simular(Processo processos[MAX_PROCESSOS], int n, Algoritmo algo,
                     int quantum, bool modo_seq, ResultadoSimulacao *r) {
    reiniciar_estado(processos, n);
    r->n_segmentos = 0;

    /* instante inicial: primeira submissao (nao ha nada a simular antes disso) */
    int t = processos[0].chegada;
    for (int i = 1; i < n; i++) {
        if (processos[i].chegada < t) t = processos[i].chegada;
    }
    int tempo_inicio = t;

    Fila prontos, fila_es;
    fila_inicializar(&prontos);
    fila_inicializar(&fila_es);

    int em_servico_es = -1; /* processo atendido pelo unico dispositivo de E/S (modo -seq) */

    int rodando = -1;      /* indice do processo em execucao na CPU, -1 = ociosa */
    int quantum_usado = 0; /* usado somente pelo Round-Robin */

    int concluidos = 0;
    int ticks_ociosos = 0;

    while (concluidos < n) {
        /* ---- fase A: chegadas de novos processos ---- */
        for (int i = 0; i < n; i++) {
            if (!processos[i].chegou && processos[i].chegada == t) {
                processos[i].chegou = true;
                processos[i].tempo_restante = processos[i].rajadas[processos[i].indice_rajada];
                fila_adicionar(&prontos, i);
            }
        }

        /* ---- fase A: conclusao de operacoes de E/S ---- */
        if (!modo_seq) {
            /* cada processo possui seu proprio dispositivo de E/S */
            for (int i = 0; i < n; i++) {
                if (processos[i].em_es && processos[i].tempo_fim_es == t) {
                    processos[i].em_es = false;
                    processos[i].tempo_restante = processos[i].rajadas[processos[i].indice_rajada];
                    fila_adicionar(&prontos, i);
                }
            }
        } else {
            /* dispositivo unico e compartilhado, atendimento em fila FIFO */
            if (em_servico_es != -1 && processos[em_servico_es].tempo_fim_es == t) {
                processos[em_servico_es].em_es = false;
                processos[em_servico_es].tempo_restante = processos[em_servico_es].rajadas[processos[em_servico_es].indice_rajada];
                fila_adicionar(&prontos, em_servico_es);
                em_servico_es = -1;
            }
            if (em_servico_es == -1 && !fila_vazia(&fila_es)) {
                int prox = fila_remover(&fila_es, 0);
                em_servico_es = prox;
                processos[prox].em_es = true;
                processos[prox].tempo_fim_es = t + processos[prox].duracao_es_pendente;
            }
        }

        /* ---- fase B: escolha do processo a executar (dependente do algoritmo) ---- */
        switch (algo) {
            case ALG_FCFS:
                if (rodando == -1 && !fila_vazia(&prontos)) {
                    rodando = fila_remover(&prontos, 0);
                }
                break;

            case ALG_SJF:
                if (rodando == -1 && !fila_vazia(&prontos)) {
                    int pos = posicao_menor_tempo_restante(processos, &prontos);
                    rodando = fila_remover(&prontos, pos);
                }
                break;

            case ALG_SRTF:
                if (!fila_vazia(&prontos)) {
                    int pos = posicao_menor_tempo_restante(processos, &prontos);
                    int candidato_tempo = processos[prontos.itens[pos]].tempo_restante;
                    if (rodando == -1 || candidato_tempo < processos[rodando].tempo_restante) {
                        int novo = fila_remover(&prontos, pos);
                        if (rodando != -1) fila_adicionar(&prontos, rodando);
                        rodando = novo;
                    }
                }
                break;

            case ALG_PRIORIDADE:
                if (!fila_vazia(&prontos)) {
                    int pos = posicao_maior_prioridade(processos, &prontos);
                    int candidato_prio = processos[prontos.itens[pos]].prioridade;
                    if (rodando == -1 || candidato_prio > processos[rodando].prioridade) {
                        int novo = fila_remover(&prontos, pos);
                        if (rodando != -1) fila_adicionar(&prontos, rodando);
                        rodando = novo;
                    }
                }
                break;

            case ALG_ROUND_ROBIN:
                if (rodando == -1) {
                    if (!fila_vazia(&prontos)) {
                        rodando = fila_remover(&prontos, 0);
                        quantum_usado = 0;
                    }
                } else if (quantum_usado == quantum) {
                    if (!fila_vazia(&prontos)) {
                        fila_adicionar(&prontos, rodando);
                        rodando = fila_remover(&prontos, 0);
                    }
                    quantum_usado = 0;
                }
                break;

            default:
                break;
        }

        /* ---- fase C: execucao de 1 ms e contabilizacao de espera ---- */
        if (rodando == -1) {
            ticks_ociosos++;
            registrar_gantt(r, -1, t + 1);
        } else {
            processos[rodando].tempo_restante--;
            if (algo == ALG_ROUND_ROBIN) quantum_usado++;
            registrar_gantt(r, processos[rodando].id, t + 1);
        }
        for (int i = 0; i < prontos.tamanho; i++) {
            processos[prontos.itens[i]].tempo_espera++;
        }

        /* ---- fase D: verifica termino da rajada de CPU corrente ---- */
        if (rodando != -1 && processos[rodando].tempo_restante == 0) {
            int idx = rodando;
            processos[idx].indice_rajada++; /* aponta para a rajada de E/S, se houver */

            if (processos[idx].indice_rajada < processos[idx].n_rajadas) {
                int duracao = processos[idx].rajadas[processos[idx].indice_rajada];
                processos[idx].indice_rajada++; /* aponta para a proxima rajada de CPU */
                processos[idx].duracao_es_pendente = duracao;
                if (!modo_seq) {
                    processos[idx].em_es = true;
                    processos[idx].tempo_fim_es = (t + 1) + duracao;
                } else {
                    fila_adicionar(&fila_es, idx);
                }
            } else {
                processos[idx].finalizado = true;
                processos[idx].tempo_termino = t + 1;
                concluidos++;
            }
            rodando = -1;
            quantum_usado = 0;
        }

        t++;
    }

    /* ---- estatisticas finais ---- */
    int tempo_fim_geral = tempo_inicio;
    for (int i = 0; i < n; i++) {
        if (processos[i].tempo_termino > tempo_fim_geral) tempo_fim_geral = processos[i].tempo_termino;
    }
    int duracao_total = tempo_fim_geral - tempo_inicio;

    r->utilizacao_cpu = duracao_total > 0
        ? 100.0 * (duracao_total - ticks_ociosos) / duracao_total
        : 0.0;
    r->throughput = duracao_total > 0
        ? n / (duracao_total / 1000.0)
        : 0.0;

    int soma_espera = 0, soma_turnaround = 0;
    for (int i = 0; i < n; i++) {
        int turnaround = processos[i].tempo_termino - processos[i].chegada;
        r->tempo_espera[i] = processos[i].tempo_espera;
        r->tempo_turnaround[i] = turnaround;
        soma_espera += processos[i].tempo_espera;
        soma_turnaround += turnaround;
    }
    r->espera_media = (double) soma_espera / n;
    r->turnaround_media = (double) soma_turnaround / n;
}

/* ---------------------- geracao da saida ---------------------- */

static const char *nome_algoritmo(Algoritmo algo, int quantum, char *buf, size_t tam) {
    switch (algo) {
        case ALG_FCFS:         return "FCFS";
        case ALG_SJF:           return "SJF";
        case ALG_SRTF:          return "SRTF";
        case ALG_PRIORIDADE:    return "PRIORIDADE PREEMPTIVO";
        case ALG_ROUND_ROBIN:
            snprintf(buf, tam, "ROUND-ROBIN(q=%dms)", quantum);
            return buf;
        default:                return "?";
    }
}

/* Escreve, para um algoritmo, o diagrama de Gantt e as estatisticas exigidas */
static void escrever_resultado(FILE *saida, Algoritmo algo, int quantum, int n,
                                ResultadoSimulacao *r) {
    char buf_nome[32];
    const char *nome = nome_algoritmo(algo, quantum, buf_nome, sizeof(buf_nome));

    fprintf(saida, "%s: %d[", nome, n);
    for (int i = 0; i < r->n_segmentos; i++) {
        if (r->segmentos[i].processo_id == -1) {
            fprintf(saida, "*** %d", r->segmentos[i].tempo_fim);
        } else {
            fprintf(saida, "P%d %d", r->segmentos[i].processo_id, r->segmentos[i].tempo_fim);
        }
        if (i < r->n_segmentos - 1) fprintf(saida, "|");
    }
    fprintf(saida, "]\n");

    fprintf(saida, "Utilizacao da CPU: %.2f%%\n", r->utilizacao_cpu);
    fprintf(saida, "Throughput: %.4f processos/s\n", r->throughput);

    fprintf(saida, "Tempo de espera:");
    for (int i = 0; i < n; i++) {
        fprintf(saida, " P%d=%dms", i + 1, r->tempo_espera[i]);
    }
    fprintf(saida, " | Media=%.2fms\n", r->espera_media);

    fprintf(saida, "Tempo de turnaround:");
    for (int i = 0; i < n; i++) {
        fprintf(saida, " P%d=%dms", i + 1, r->tempo_turnaround[i]);
    }
    fprintf(saida, " | Media=%.2fms\n", r->turnaround_media);

    fprintf(saida, "\n");
}

/* ---------------------- programa principal ---------------------- */

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "Uso: %s <arquivo_entrada> <quantum_ms> [-seq]\n", argv[0]);
        return 1;
    }
    if (argc == 4 && strcmp(argv[3], "-seq") != 0) {
        fprintf(stderr, "Uso: %s <arquivo_entrada> <quantum_ms> [-seq]\n", argv[0]);
        return 1;
    }

    const char *caminho_entrada = argv[1];
    int quantum = atoi(argv[2]);
    bool modo_seq = (argc == 4);

    if (quantum <= 0) {
        fprintf(stderr, "Erro: o quantum deve ser um numero inteiro positivo\n");
        return 1;
    }

    Processo processos[MAX_PROCESSOS];
    int n = ler_processos(caminho_entrada, processos);
    if (n == 0) {
        fprintf(stderr, "Erro: nenhum processo valido encontrado em '%s'\n", caminho_entrada);
        return 1;
    }

    char caminho_saida[MAX_LINHA + 4];
    snprintf(caminho_saida, sizeof(caminho_saida), "%s.out", caminho_entrada);
    FILE *saida = fopen(caminho_saida, "w");
    if (!saida) {
        fprintf(stderr, "Erro: nao foi possivel criar o arquivo de saida '%s'\n", caminho_saida);
        return 1;
    }

    static ResultadoSimulacao resultado;
    for (Algoritmo algo = ALG_FCFS; algo < QTD_ALGORITMOS; algo++) {
        simular(processos, n, algo, quantum, modo_seq, &resultado);
        escrever_resultado(saida, algo, quantum, n, &resultado);
    }

    fclose(saida);
    return 0;
}
