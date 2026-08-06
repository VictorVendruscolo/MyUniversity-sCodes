# Trabalho 4 - Sistemas Operacionais

Victor Rech Vendruscolo

Programa multithread que le numeros naturais de `in.txt` e encadeia tres
listas:

- a **thread principal** cria as 3 threads adicionais e depois le `in.txt`,
  inserindo cada numero no final de `L1`;
- a **thread 1** le `L1` e cria `L2`, com os mesmos elementos exceto os pares
  maiores que 2;
- a **thread 2** le `L2` e cria `L3`, com os mesmos elementos exceto os nao
  primos;
- a **thread 3** le `L3` e imprime os primos (arquivo `out.txt`).

Nenhuma lista e modificada por quem a le: cada thread cria nos novos na sua
propria lista de saida, e `L1` permanece completa ate o fim.

## Compilacao e execucao

```sh
gcc -Wall -Wextra -O2 -o t4_semaforos t4_semaforos.c -pthread

./t4_semaforos    # le in.txt, escreve out.txt
```

## Sincronizacao: 3 semaforos de contagem nomeados

Numero pequeno e fixo, um por lista, independente do tamanho da entrada:

| semaforo | `sem_post` | `sem_wait` | significa |
| -------- | ---------- | ---------- | --------- |
| `/t4_l1` | principal  | thread 1   | ha mais um no pronto em `L1` |
| `/t4_l2` | thread 1   | thread 2   | ha mais um no pronto em `L2` |
| `/t4_l3` | thread 2   | thread 3   | ha mais um no pronto em `L3` |

Os tres sao abertos com `sem_open(nome, O_CREAT, 0644, 0)`, valor inicial 0, e
contam itens disponiveis - o valor chega a centenas de milhares durante a
execucao. Nenhum e usado como semaforo binario ou trava de exclusao mutua.

Como semaforos nomeados persistem no sistema depois que o processo termina
(`/dev/shm/sem.t4_l1` e afins), o programa faz `sem_unlink` antes de abrir e
`sem_close` + `sem_unlink` ao terminar. Sem isso, uma segunda execucao herdaria
o contador da primeira.

### Por que nao e preciso mais nada

Cada lista tem **um unico escritor e um unico leitor**, e nenhuma variavel
compartilhada e escrita por duas threads:

| dado | escreve | le | garantia |
| ---- | ------- | -- | -------- |
| `prox` do ultimo no | so a produtora daquela lista | so a consumidora | a produtora liga o no **antes** do `sem_post`; a consumidora so le `prox` **depois** do `sem_wait` |
| `valor`/`prox` do no novo | produtora, antes do post | consumidora, depois do wait | idem |
| `ultimo` (produtora) e `atual` (consumidora) | - | - | variaveis locais de cada thread |
| nos cabeca das 3 listas | principal, antes do `pthread_create` | as 3 threads | publicados na criacao das threads |

Nao ha regiao critica, portanto nao ha mutex, variavel de condicao, contadora
compartilhada, espera ocupada nem `volatile`. `sem_post`/`sem_wait` tambem
funcionam como barreira de memoria, o que dispensa qualquer outra garantia de
visibilidade.

O `helgrind` do valgrind nao acusa nenhuma corrida de dados.

### Fim da entrada

A principal fecha `L1` com um no de valor especial (`max unsigned`) e cada
thread repassa esse no para a sua lista de saida antes de encerrar. Assim o
termino viaja como dado dentro das listas: sem ele, a consumidora ficaria
bloqueada para sempre no ultimo `sem_wait`, e a alternativa seria uma variavel
de controle compartilhada - ou seja, sincronizacao fora dos semaforos.

### Progresso simultaneo

Cada `sem_post` acontece na insercao de cada no, nao ao fim da lista, de modo
que as quatro threads trabalham ao mesmo tempo: a principal ainda esta lendo
`in.txt` enquanto a thread 3 ja imprime primos.

## Memoria

Nos alocados com `malloc`; as tres listas sao destruidas pela thread principal
depois dos `pthread_join`. Com a entrada de 1000000 numeros sao cerca de 25 MB
(`L1` completa, `L2` com os impares e o 2, `L3` com os 78498 primos). O
valgrind acusa 0 bytes em uso na saida.

## Testes

`100.txt`/`100.out.txt` e `in.txt` (1000000 numeros)/`1000000.out.txt`. As
saidas conferem exatamente com os gabaritos; a entrada de 1000000 elementos
roda em cerca de 3 segundos.
