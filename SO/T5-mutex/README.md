# Trabalho 5 - Sistemas Operacionais

Victor Rech Vendruscolo

Programa multithread que le numeros naturais de `in.txt`, insere no final de
uma unica lista simplesmente encadeada `L` e filtra essa MESMA lista em tres
etapas, tal como o T2, mas com a sincronizacao inteira feita por **um mutex
por no**, sem semaforo e sem variavel contadora:

- a **thread principal** cria as 3 threads adicionais e depois le `in.txt`,
  inserindo cada numero no final de `L`;
- a **thread 1** remove de `L` os pares maiores que 2;
- a **thread 2** remove de `L` os nao primos;
- a **thread 3** imprime os primos armazenados em `L` (arquivo `out.txt`).

Nenhum no e removido duas vezes nem lido apos ser liberado: a seguranca vem
inteiramente do mutex de cada no.

## Compilacao e execucao

```sh
gcc -Wall -Wextra -O2 -o t5_mutex t5_mutex.c -pthread

./t5_mutex    # le in.txt, escreve out.txt
```

## Sincronizacao: um mutex por no, nada alem disso

Cada `No` carrega o proprio `pthread_mutex_t`. Nao existe mutex global, nem
semaforo, nem contadora compartilhada, nem espera ocupada, nem `volatile` - a
unica ferramenta e `pthread_mutex_lock`/`pthread_mutex_unlock`.

### Lock coupling (mao sobre mao)

Toda thread - a principal inclusive - percorre `L` mantendo sempre o no
anterior (`ant`) travado enquanto trava o no seguinte (`atual`), e so libera
`ant` depois de garantir `atual` travado. Isso impede que uma thread pule
para um no que outra esteja no meio de destruir: para alcancar qualquer no,
e preciso primeiro travar o seu antecessor - e enquanto alguem segura esse
antecessor, mais ninguem consegue "entrar" por ali.

### O mutex tambem sinaliza "ainda nao chegou"

Um no nasce **travado por quem o cria** (`novo_no` trava o proprio mutex
antes de devolver o ponteiro). Ele so e destravado quando o seu campo `prox`
se torna definitivo:

- quando a principal liga um no ao proximo (`fim->prox = novo`), o no
  **antigo** (`fim`) acaba de ganhar seu `prox` definitivo, entao a principal
  o destrava nesse instante;
- o **novo** no continua travado pela principal, porque seu proprio `prox`
  ainda nao se sabe.

Uma thread adicional so consegue travar um no depois que a principal (ou,
mais adiante na lista, a propria thread adicional que acabou de decidir manter
um no) o destravou. Antes disso, `pthread_mutex_lock` simplesmente bloqueia -
sem busy waiting, sem checar variavel nenhuma: a thread dorme ate o mutex
ficar livre. E exatamente esse bloqueio, feito pelo primitivo mais basico de
exclusao mutua (`acquire()`/`release()`), que substitui as variaveis
contadoras do T2 e os semaforos do T4.

### Por que isso e seguro mesmo com remocao concorrente

Para apagar um no `atual`, a thread precisa manter `ant` E `atual` travados
ao mesmo tempo (`ant->prox = atual->prox`, so entao destrava e libera
`atual`). Nenhuma outra thread pode estar "a caminho" de `atual` nesse
instante, porque para chegar a `atual` ela precisaria antes ter travado
`ant` - e `ant` esta com a thread que esta apagando. Assim, quando alguem
finalmente consegue travar `ant` de novo, o `prox` que ela le ja aponta para
alem do no apagado; o no removido nunca e alcancado por ninguem depois de
liberado.

Como toda thread (principal e as tres adicionais) so avanca na mesma ordem da
lista - nunca trava um no "fora de ordem" nem solta um no para travar outro
mais atras - nao ha espera circular possivel, logo nao ha deadlock.

### Por que os filtros nao precisam de ordem entre si

Um par maior que 2 nunca e primo, entao ele seria removido pela thread 1 OU
pela thread 2, tanto faz quem chegue primeiro - o estado final de `L` e o
mesmo. E a thread 3 testa a primalidade do proprio valor ao imprimir, sem
depender de as threads 1 e 2 ja terem passado por aquele no - primos nunca
sao removidos por nenhuma das duas, entao a thread 3 sempre imprime
corretamente, mesmo que esteja adiantada em relacao as outras. Essa
propriedade e o que permite as quatro threads progredirem de fato em
paralelo: cada uma trava, no maximo, os dois nos que esta examinando naquele
instante, e so ha espera quando duas threads miram exatamente o mesmo no ao
mesmo tempo.

### Fim da entrada

Igual ao T2 e ao T4: a principal fecha `L` com um no de valor especial
(`max unsigned`). Esse no e criado, ligado e imediatamente destravado pela
principal (ele nunca ganhara sucessor), entao qualquer thread adicional que o
alcance sabe, so pelo valor, que a lista terminou.

## Memoria

Nos alocados com `malloc`, cada um com seu `pthread_mutex_t` inicializado e
destruido junto (`pthread_mutex_destroy` antes de `free`). A thread principal
destroi o que sobrou de `L` (cabeca + primos + sentinela) depois dos tres
`pthread_join`.

## Testes

`100.txt`/`100.out.txt` e `in.txt` (1000000 numeros)/`1000000.out.txt`. As
saidas conferem exatamente com os gabaritos - 30 execucoes do caso de 100 e 5
do caso de 1000000, sem divergencia (~0,5 s para 1M).

`valgrind --leak-check=full`: 0 bytes em uso na saida, 0 erros.
`valgrind --tool=helgrind` e `valgrind --tool=drd`: 0 erros nos dois - nenhuma
corrida de dados, nenhuma violacao de ordem de lock.
