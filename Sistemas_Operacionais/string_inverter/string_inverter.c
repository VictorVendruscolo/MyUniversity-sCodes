#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char str[1024]; /* string compartilhada entre threads */

typedef struct {
    int thread_id;
    int start;
    int end;
} thread_args;

void *inverter(void *param)
{
    thread_args *args = (thread_args *)param;
    int thread_id = args->thread_id;
    int section_size = args->start;  /* n/8 */
    int len = strlen(str);
    
    /* Cada thread faz n/8 trocas, sem conflito */
    for (int i = 0; i < section_size; i++) {
        int start = thread_id * section_size + i;
        int end = len - 1 - thread_id * section_size - i;
        
        if (start < end) {
            char temp = str[start];
            str[start] = str[end];
            str[end] = temp;
        }
    }
    
    free(args);
    pthread_exit(0);
}

int main()
{
    FILE *in, *out;
    pthread_t tid[4];
    int len, section_size;
    
    /* Lê string do arquivo */
    in = fopen("../in.txt", "r");
    if (in == NULL) {
        fprintf(stderr, "erro ao abrir ../in.txt\n");
        return -1;
    }
    
    fgets(str, sizeof(str), in);
    fclose(in);
    
    len = strlen(str);
    if (str[len - 1] == '\n') {
        str[len - 1] = '\0';
        len--;
    }
    
    section_size = (len + 7) / 8; /* cada thread pega n/8 do início e n/8 do final (ceil) */
    
    /* Cria 4 threads */
    for (int i = 0; i < 4; i++) {
        thread_args *args = malloc(sizeof(thread_args));
        args->thread_id = i;
        args->start = section_size;  /* tamanho de cada seção (n/8) */
        args->end = section_size;    /* tamanho de cada seção (n/8) */
        
        pthread_create(&tid[i], NULL, inverter, args);
    }
    
    /* Aguarda todas as threads */
    for (int i = 0; i < 4; i++) {
        pthread_join(tid[i], NULL);
    }
    
   
    out = fopen("../out.txt", "w");
    if (out == NULL) {
        fprintf(stderr, "erro ao abrir ../out.txt\n");
        return -1;
    }
    
    fprintf(out, "%s\n", str);
    fclose(out);
    
    return 0;
}

