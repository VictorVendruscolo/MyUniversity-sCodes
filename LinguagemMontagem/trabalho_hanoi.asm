#TRABALHO DE LM 2026 - TORRE DE HANOI
#RGM: 51520 / (RGM da dupla)
#
#ANTES DE RODAR, NO MARS:
#  Settings -> desmarcar "Permit extended (pseudo) instructions and formats"
#  Tools -> Bitmap Display:
#    Unit Width in Pixels = 8       Unit Height in Pixels = 8
#    Display Width in Pixels = 512  Display Height in Pixels = 256
#    Base address for display = 0x10008000 ($gp)
#    clicar em "Connect to MIPS"
#  Para avancar cada passo: clicar na aba Run I/O e apertar ESPACO
#
#MEMORIA:
#  tela  -> 0x10008000 // 64 colunas x 32 linhas, 1 word por quadradinho
#           tela[y*64 + x] fica em 0x10008000 + (y*64 + x)*4
#           endereco: lui $reg,0x1000 / ori $reg,$reg,0x8000
#  torre -> 0x10010000 // torre[3][9]: a torre t comeca na posicao t*9
#           torre[t][i] fica em 0x10010000 + (t*9 + i)*4
#           endereco: lui $reg,0x1001
#  topo  -> 0x1001006C // topo[3]: quantos discos tem em cada torre
#           topo[t] fica em 0x1001006C + t*4
#           endereco: lui $reg,0x1001 / ori $reg,$reg,0x006C
#
#CORES (0x00RRGGBB):
#  PRETO    = 0x00000000 -> $zero
#  BRANCO   = 0x00FFFFFF -> lui $reg,0x00FF / ori $reg,$reg,0xFFFF
#  VERMELHO = 0x00FF0000 -> lui $reg,0x00FF
#
#DESENHO:
#  a torre t tem centro na coluna 10 + 21*t (10, 31, 52)
#  base de cada torre: linha 31, da coluna centro-9 ate centro+9
#  disco de tamanho d na altura i: linha 29 - 2*i, da coluna centro-d ate centro+d

.data
.space 108 #int torre[3][9] // 0x10010000 (3*9*4 = 108 bytes)
.space 12  #int topo[3]     // 0x1001006C (108 = 0x6C)

.text
#int n, i; //n->$s0, i->$s1

#leia n
LEIA_N:
#COMPLETAR (syscall 5)

#if(n<1 || n>9) leia de novo //9 e o maximo que cabe na largura da tela
#COMPLETAR (slti + bne/beq para LEIA_N)

#for(i=0; i<n; ++i)
#COMPLETAR: i = 0
FOR_INICIA:
#COMPLETAR: if(!(i<n)) sai do for (slt + beq para FIM_FOR_INICIA)
#	torre[0][i] = n - i; //maior disco embaixo
#COMPLETAR
#COMPLETAR: ++i e volta (j FOR_INICIA)
FIM_FOR_INICIA:

#topo[0] = n; topo[1] = 0; topo[2] = 0;
#COMPLETAR

#desenha(); //mostra o estado inicial
#COMPLETAR

#hanoi(n, 0, 2, 1); //tudo da torre 0 para a torre 2, usando a 1 como auxiliar
#COMPLETAR

#return 0;
addi $v0,$zero,10
syscall


PROC_HANOI:
#void hanoi(int n, int orig, int dest, int aux){ //n->$a0, orig->$a1, dest->$a2, aux->$a3
#if(n==0)
#COMPLETAR (bne para PROC_HANOI_FIM_IF)
# return;
jr $ra
PROC_HANOI_FIM_IF:
#pilha: 4($sp)=$ra, 8($sp)=n, 12($sp)=orig, 16($sp)=dest, 20($sp)=aux
#COMPLETAR: reservar 20 bytes e salvar $ra, $a0, $a1, $a2, $a3

#hanoi(n-1, orig, aux, dest);
#COMPLETAR

#mover(orig, dest); //a chamada acima mudou os $a: recarregar da pilha
#COMPLETAR

#hanoi(n-1, aux, dest, orig);
#COMPLETAR

#COMPLETAR: recuperar $ra e liberar os 20 bytes
jr $ra
#}


PROC_MOVER:
#void mover(int orig, int dest){ //orig->$a0, dest->$a1
#pilha: 4($sp)=$ra //mover chama desenha, entao precisa guardar o $ra
#COMPLETAR: reservar 4 bytes e salvar $ra

#do{ c = leia char; } while(c != ' '); //espera o ESPACO (' ' = 32)
PROC_MOVER_ESPERA:
#COMPLETAR (syscall 12 + bne para PROC_MOVER_ESPERA)

#topo[orig] = topo[orig] - 1;
#COMPLETAR

#disco = torre[orig][topo[orig]];
#COMPLETAR

#torre[dest][topo[dest]] = disco;
#COMPLETAR

#topo[dest] = topo[dest] + 1;
#COMPLETAR

#desenha();
#COMPLETAR

#COMPLETAR: recuperar $ra e liberar os 4 bytes
jr $ra
#}


PROC_DESENHA:
#void desenha(){
#int t, i, centro; //t->$s0, i->$s1, centro->$s2
#pilha: 4($sp)=$ra, 8($sp)=$s0, 12($sp)=$s1, 16($sp)=$s2 //o main tambem usa $s0 e $s1
#COMPLETAR: reservar 16 bytes e salvar

#for(i=0; i<2048; ++i) tela[i] = PRETO; //limpa a tela (64*32 = 2048)
#COMPLETAR: i = 0 e endereco da tela
PROC_DESENHA_FOR_LIMPA:
#COMPLETAR
PROC_DESENHA_FIM_FOR_LIMPA:

#for(t=0; t<3; ++t){
#COMPLETAR: t = 0
PROC_DESENHA_FOR_T:
#COMPLETAR: if(!(t<3)) sai (slti + beq para PROC_DESENHA_FIM_FOR_T)
#	centro = 10 + 21*t;
#COMPLETAR
#	linha(centro-9, centro+9, 31, BRANCO); //base da torre
#COMPLETAR
#	for(i=0; i<topo[t]; ++i){
#COMPLETAR: i = 0
PROC_DESENHA_FOR_I:
#COMPLETAR: if(!(i<topo[t])) sai (lw do topo[t], slt + beq para PROC_DESENHA_FIM_FOR_I)
#		disco = torre[t][i];
#COMPLETAR
#		linha(centro-disco, centro+disco, 29-2*i, VERMELHO);
#COMPLETAR
#COMPLETAR: ++i e volta (j PROC_DESENHA_FOR_I)
PROC_DESENHA_FIM_FOR_I:
#	}
#COMPLETAR: ++t e volta (j PROC_DESENHA_FOR_T)
PROC_DESENHA_FIM_FOR_T:
#}
#COMPLETAR: recuperar $s2, $s1, $s0, $ra e liberar os 16 bytes
jr $ra
#}


PROC_LINHA:
#void linha(int x0, int x1, int y, int cor){ //x0->$a0, x1->$a1, y->$a2, cor->$a3
#procedimento folha: so usa $t, nao precisa de pilha
#for(x=x0; x<=x1; ++x) //x->$t2
#COMPLETAR: x = x0
PROC_LINHA_FOR_X:
#COMPLETAR: x<=x1 e o mesmo que !(x1<x) (slt + bne para PROC_LINHA_FIM_FOR_X)
#	tela[y*64 + x] = cor;
#COMPLETAR
#COMPLETAR: ++x e volta (j PROC_LINHA_FOR_X)
PROC_LINHA_FIM_FOR_X:
jr $ra
#}
