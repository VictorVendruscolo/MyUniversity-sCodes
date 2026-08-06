# Trabalho 2 - Sistemas Operacionais

Victor Rech Vendruscolo

Dois programas que filtram uma lista simplesmente encadeada `L` construida a
partir de `in.txt`:

- `t2_threads.c` - monoprocesso multithread (thread principal + 3 threads);
- `t2_processos.c` - multiprocesso monothread (processo pai + 3 filhos), com
  toda a IPC feita por memoria compartilhada (`shm_open` + `mmap`).

Em ambos, a principal cria os 3 processos/threads adicionais, depois le os
numeros naturais de `in.txt` inserindo-os no final de `L`; o adicional 1 remove
de `L` os pares maiores que 2, o adicional 2 remove os nao primos e o adicional
3 imprime os primos que restaram em `L` (arquivo `out.txt`).

## Compilacao e execucao

```sh
gcc -Wall -Wextra -O2 -o t2_threads   t2_threads.c   -pthread
gcc -Wall -Wextra -O2 -o t2_processos t2_processos.c -lrt

./t2_threads      # le in.txt, escreve out.txt
./t2_processos    # le in.txt, escreve out.txt
```

## Sincronizacao: so contadoras, sem semaforo e sem instrucao atomica

A sincronizacao usa apenas variaveis contadoras compartilhadas, no mesmo
esquema do par `in`/`out` do produtor-consumidor do livro:

| contadora      | quem escreve  | quem le       |
| -------------- | ------------- | ------------- |
| `cont_lidos`   | principal     | adicional 1   |
| `cont_sem_par` | adicional 1   | adicional 2   |
| `cont_primos`  | adicional 2   | adicional 3   |

Cada contadora e monotonica e tem **um unico escritor**. Quem le mantem uma
contadora local privada e espera de forma ocupada enquanto
`local == compartilhada`. Como nenhuma variavel compartilhada sofre
leitura-modificacao-escrita por dois fluxos ao mesmo tempo, a sincronizacao
funciona com leitura e escrita comuns de inteiros alinhados - nao ha semaforo,
mutex, variavel de condicao nem instrucao atomica. Os binarios gerados nao
contem nenhuma instrucao com prefixo `lock`.

### Por que ninguem le `L` em estado inconsistente

Cada etapa so libera um no para a etapa seguinte **depois** de ja ter avancado
para o proximo no que manteve na lista. Nesse instante o campo `prox` do no
liberado e definitivo, porque a etapa que o liberou nunca mais escreve nele.
Assim, o no visto pela etapa seguinte ja esta estavel, e a etapa anterior nunca
e ultrapassada: os quatro fluxos avancam simultaneamente sobre `L`, em
pipeline, e nao um de cada vez.

O fim da entrada e sinalizado por um ultimo no com valor especial
(`max unsigned`), que atravessa o pipeline e encerra cada etapa - sem precisar
de nenhuma outra variavel de controle.

## Memoria

Os nos ficam num buffer proprio gerenciado por um mapa de bits criado e
inicializado pela principal, unica responsavel por alocar. Cada adicional
desaloca apenas os nos que ele mesmo removeu; a principal destroi no fim a
lista que sobrou. Os encadeamentos guardam o **deslocamento** do no dentro do
buffer, nunca um endereco real - necessario porque cada processo mapeia a
regiao compartilhada num endereco virtual diferente.

O mapa usa uma posicao por no (0 = livre, 1 = ocupado) de proposito: com bits
empacotados, liberar um no exigiria ler-alterar-gravar um byte compartilhado
com outros 7 nos, o que so seria seguro com instrucao atomica.

## Testes

`100.txt`/`100.out.txt` e `in.txt` (1000000 numeros) / `1000000.out.txt`.
As duas versoes reproduzem exatamente as saidas esperadas, sem nos vazados no
mapa ao final.
