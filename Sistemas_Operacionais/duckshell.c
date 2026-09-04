#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#define MAX_INPUT 255 //Tamanho máximo do buffer de type-ahead do terminal
#define MAX_ARGS 7  //comando + 5 argumentos (max) + NULL 

void imprimir_arvore(int pid_processo, int profundidade) { // função recursiva para imprimir a árvore de processos
    char caminho[256];
    FILE *arquivo;
    int pid_atual, pid_pai;
    char nome[256], estado[4];

    snprintf(caminho, sizeof(caminho), "/proc/%d/stat", pid_processo);
    arquivo = fopen(caminho, "r");
    if (!arquivo) return;

    fscanf(arquivo, "%d %s %s %d", &pid_atual, nome, estado, &pid_pai);
    fclose(arquivo);

    for (int i = 0; i < profundidade; i++) printf("  ");
    printf("%d %s\n", pid_processo, nome);

    // olha os filhos
    DIR *diretorio = opendir("/proc");
    if (!diretorio) return;

    struct dirent *entrada;
    while ((entrada = readdir(diretorio)) != NULL) { // percorre os processos em /proc
        int pid_filho = atoi(entrada->d_name);
        if (pid_filho <= 0) continue;

        char caminho_filho[256];
        snprintf(caminho_filho, sizeof(caminho_filho), "/proc/%d/stat", pid_filho);
        FILE *arquivo_filho = fopen(caminho_filho, "r");
        if (!arquivo_filho) continue;

        int pid_proc, pid_pai_filho;
        char nome_filho[256], estado_filho[4];
        fscanf(arquivo_filho, "%d %s %s %d", &pid_proc, nome_filho, estado_filho, &pid_pai_filho);
        fclose(arquivo_filho);

        if (pid_pai_filho == pid_processo) {
            imprimir_arvore(pid_filho, profundidade + 1);
        }
    }
    closedir(diretorio);
}

int main(void) {
    char input[MAX_INPUT];

    while (1) {
        printf("duckshell> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) break;

        // tira o \n do final da string
        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0) continue;

        int background = 0;
        int len = strlen(input);
        if (input[len - 1] == '&') { // verifica se o último caractere é '&' 
            background = 1; // ativa execução em "background"
            input[len - 1] = '\0'; //apaga o & do final da string
            
            len = strlen(input);
            while (len > 0 && input[len - 1] == ' ') { //limpeza dos espaços em branco
                input[--len] = '\0';
            }
        }

        char *args[MAX_ARGS];
        int argc = 0;
        char *comando = strtok(input, " "); //divide os comandos
        while (comando != NULL && argc < MAX_ARGS - 1) { 
            args[argc++] = comando;
            comando = strtok(NULL, " ");
        }
        args[argc] = NULL;

        if (argc == 0) continue;

        if (strcmp(args[0], "exit") == 0) {
            printf("Encerrando duckshell...\n QUACK-QUACK (goodbye!)\n");
            break;
        }

        if (strcmp(args[0], "cd") == 0) {
            if (argc < 2) {
                fprintf(stderr, "cd: precisa de um argumento\n");
            } else {
                if (chdir(args[1]) != 0) {
                    perror("cd");
                }
            }
            continue;
        }

        if (strcmp(args[0], "tree") == 0) {
            if (argc < 2) {
                fprintf(stderr, "tree: precisa de um PID\n");
            } else {
                int pid = atoi(args[1]);
                imprimir_arvore(pid, 0);
            }
            continue;
        }

        pid_t pid = fork(); //cria um processo filho
        if (pid < 0) {
            perror("fork");
        } else if (pid == 0) {
            // suporte para até 5 argumentos
            execlp(args[0], args[0],
                   argc > 1 ? args[1] : NULL,
                   argc > 2 ? args[2] : NULL,
                   argc > 3 ? args[3] : NULL,
                   argc > 4 ? args[4] : NULL,
                   argc > 5 ? args[5] : NULL,
                   NULL);
            perror("execlp");
            exit(1);
        } else {
            // suporte para execução em segundo plano
            if (!background) {
                waitpid(pid, NULL, 0);
            }
        }
    }

    return 0;
}
