#TRABALHO DE LM 2026 - TORRE DE HANOI
#RGM: 51520 / (RGM da dupla)
#
#COMO RODAR NO MARS
#  1. Tools > Bitmap Display: Unit Width = 8 e Unit Height = 8
#     (o resto fica no padrao: 512 x 256, base 0x10010000) e "Connect to MIPS"
#  2. Assemble e Run; na aba Run I/O digitar n e apertar ESPACO a cada passo
#  Sem pseudoinstrucoes: roda com Settings > "Permit extended (pseudo)
#  instructions and formats" desmarcado.
#
#MEMORIA (.data)
#  0x10010000 tela[64*32] -> tela[y*64 + x]
#  0x10012000 torre[3][9] -> torre[t][i] na posicao t*9 + i
#  0x1001206C topo[3]     -> quantos discos tem em cada torre
#  0x10012078 textos
#
#CONSTANTES
#  9       maximo de discos
#  64      largura da tela; 2048 = 64*32 quadradinhos
#  10, 21  centro da 1a torre e distancia entre as torres
#  31      linha da base; 11 a 30 = haste
#  29      linha do 1o disco; cada disco sobe 2 linhas
#
#CORES (0x00RRGGBB)
#  base 0x00FFFFFF, haste 0x00808080
#  disco 0x00FF0000 + tamanho*0x1C00 (degrade do vermelho ao amarelo)

.data
	.space 8192 #int tela[2048]  // 0x10010000
	.space 108  #int torre[3][9] // 0x10012000
	.space 12   #int topo[3]     // 0x1001206C
	.asciiz "Quantos discos (1 a 9)? "        # 0x10012078
	.asciiz "Aperte ESPACO para cada passo\n" # 0x10012091
	.asciiz "\nFim!\n"                        # 0x100120B0

.text
	#int n, i; //n->$s0, i->$s1

	#do{ imprime "Quantos discos (1 a 9)? "; leia n; } while(n<1 || n>9);
LEIA_N:
	addi $v0,$zero,4
	lui $a0,0x1001
	ori $a0,$a0,0x2078
	syscall
	addi $v0,$zero,5
	syscall
	add $s0,$zero,$v0
	slti $t0,$s0,1
	bne $t0,$zero,LEIA_N
	slti $t0,$s0,10
	beq $t0,$zero,LEIA_N

	#imprime "Aperte ESPACO para cada passo"
	addi $v0,$zero,4
	lui $a0,0x1001
	ori $a0,$a0,0x2091
	syscall

	#for(i=0; i<n; ++i) torre[0][i] = n - i;
	add $s1,$zero,$zero
FOR_INICIA:
	slt $t0,$s1,$s0
	beq $t0,$zero,FIM_FOR_INICIA
		sub $t2,$s0,$s1
		addi $t0,$zero,4
		mul $t0,$s1,$t0
		lui $t1,0x1001
		ori $t1,$t1,0x2000
		add $t0,$t0,$t1
		sw $t2,0($t0)
		addi $s1,$s1,1
		j FOR_INICIA
FIM_FOR_INICIA:

	#topo[0] = n; topo[1] = 0; topo[2] = 0;
	lui $t1,0x1001
	ori $t1,$t1,0x206C
	sw $s0,0($t1)
	sw $zero,4($t1)
	sw $zero,8($t1)

	#desenha();
	jal PROC_DESENHA

	#hanoi(n, 0, 2, 1);
	add $a0,$zero,$s0
	addi $a1,$zero,0
	addi $a2,$zero,2
	addi $a3,$zero,1
	jal PROC_HANOI

	#imprime "Fim!"
	addi $v0,$zero,4
	lui $a0,0x1001
	ori $a0,$a0,0x20B0
	syscall

	#return 0;
	addi $v0,$zero,10
	syscall


PROC_HANOI:
#void hanoi(int n, int orig, int dest, int aux){ //n->$a0, orig->$a1, dest->$a2, aux->$a3
	#if(n==0) return;
	bne $a0,$zero,PROC_HANOI_FIM_IF
	jr $ra
PROC_HANOI_FIM_IF:
	addi $sp,$sp,-20
	sw $ra,4($sp)
	sw $a0,8($sp)
	sw $a1,12($sp)
	sw $a2,16($sp)
	sw $a3,20($sp)

	#hanoi(n-1, orig, aux, dest);
	addi $a0,$a0,-1
	lw $a2,20($sp)
	lw $a3,16($sp)
	jal PROC_HANOI

	#mover(orig, dest);
	lw $a0,12($sp)
	lw $a1,16($sp)
	jal PROC_MOVER

	#hanoi(n-1, aux, dest, orig);
	lw $a0,8($sp)
	addi $a0,$a0,-1
	lw $a1,20($sp)
	lw $a2,16($sp)
	lw $a3,12($sp)
	jal PROC_HANOI

	lw $ra,4($sp)
	addi $sp,$sp,20
	jr $ra
#}


PROC_MOVER:
#void mover(int orig, int dest){ //orig->$a0, dest->$a1
	addi $sp,$sp,-4
	sw $ra,4($sp)

	#do{ c = leia char; } while(c != ' ');
PROC_MOVER_ESPERA:
	addi $v0,$zero,12
	syscall
	addi $t0,$zero,32
	bne $v0,$t0,PROC_MOVER_ESPERA

	#topo[orig] = topo[orig] - 1;
	lui $t1,0x1001
	ori $t1,$t1,0x206C
	addi $t0,$zero,4
	mul $t2,$a0,$t0
	add $t2,$t2,$t1
	lw $t3,0($t2)
	addi $t3,$t3,-1
	sw $t3,0($t2)

	#disco = torre[orig][topo[orig]]; //disco->$t6
	addi $t0,$zero,9
	mul $t4,$a0,$t0
	add $t4,$t4,$t3
	addi $t0,$zero,4
	mul $t4,$t4,$t0
	lui $t5,0x1001
	ori $t5,$t5,0x2000
	add $t4,$t4,$t5
	lw $t6,0($t4)

	#torre[dest][topo[dest]] = disco;
	addi $t0,$zero,4
	mul $t2,$a1,$t0
	add $t2,$t2,$t1
	lw $t3,0($t2)
	addi $t0,$zero,9
	mul $t4,$a1,$t0
	add $t4,$t4,$t3
	addi $t0,$zero,4
	mul $t4,$t4,$t0
	add $t4,$t4,$t5
	sw $t6,0($t4)

	#topo[dest] = topo[dest] + 1;
	addi $t3,$t3,1
	sw $t3,0($t2)

	#desenha();
	jal PROC_DESENHA

	lw $ra,4($sp)
	addi $sp,$sp,4
	jr $ra
#}


PROC_DESENHA:
#void desenha(){ //t->$s0, i->$s1, centro->$s2
	addi $sp,$sp,-16
	sw $ra,4($sp)
	sw $s0,8($sp)
	sw $s1,12($sp)
	sw $s2,16($sp)

	#for(i=0; i<2048; ++i) tela[i] = PRETO;
	lui $t1,0x1001
	add $t2,$zero,$zero
	addi $t3,$zero,2048
PROC_DESENHA_FOR_LIMPA:
	slt $t0,$t2,$t3
	beq $t0,$zero,PROC_DESENHA_FIM_FOR_LIMPA
		sw $zero,0($t1)
		addi $t1,$t1,4
		addi $t2,$t2,1
		j PROC_DESENHA_FOR_LIMPA
PROC_DESENHA_FIM_FOR_LIMPA:

	#for(t=0; t<3; ++t){
	add $s0,$zero,$zero
PROC_DESENHA_FOR_T:
	slti $t0,$s0,3
	beq $t0,$zero,PROC_DESENHA_FIM_FOR_T

		#centro = 10 + 21*t;
		addi $t0,$zero,21
		mul $s2,$s0,$t0
		addi $s2,$s2,10

		#linha(centro-9, centro+9, 31, BRANCO); //base
		addi $a0,$s2,-9
		addi $a1,$s2,9
		addi $a2,$zero,31
		lui $a3,0x00FF
		ori $a3,$a3,0xFFFF
		jal PROC_LINHA

		#for(i=11; i<31; ++i) linha(centro, centro, i, CINZA); //haste
		addi $s1,$zero,11
PROC_DESENHA_FOR_HASTE:
		slti $t0,$s1,31
		beq $t0,$zero,PROC_DESENHA_FIM_FOR_HASTE
			add $a0,$zero,$s2
			add $a1,$zero,$s2
			add $a2,$zero,$s1
			lui $a3,0x0080
			ori $a3,$a3,0x8080
			jal PROC_LINHA
			addi $s1,$s1,1
			j PROC_DESENHA_FOR_HASTE
PROC_DESENHA_FIM_FOR_HASTE:

		#for(i=0; i<topo[t]; ++i){
		add $s1,$zero,$zero
PROC_DESENHA_FOR_I:
		lui $t1,0x1001
		ori $t1,$t1,0x206C
		addi $t0,$zero,4
		mul $t0,$s0,$t0
		add $t0,$t0,$t1
		lw $t2,0($t0)
		slt $t0,$s1,$t2
		beq $t0,$zero,PROC_DESENHA_FIM_FOR_I

			#disco = torre[t][i]; //disco->$t4
			addi $t0,$zero,9
			mul $t3,$s0,$t0
			add $t3,$t3,$s1
			addi $t0,$zero,4
			mul $t3,$t3,$t0
			lui $t1,0x1001
			ori $t1,$t1,0x2000
			add $t3,$t3,$t1
			lw $t4,0($t3)

			#linha(centro-disco, centro+disco, 29-2*i, VERMELHO + disco*0x1C00);
			sub $a0,$s2,$t4
			add $a1,$s2,$t4
			add $t5,$s1,$s1
			addi $t6,$zero,29
			sub $a2,$t6,$t5
			addi $t0,$zero,0x1C00
			mul $t0,$t4,$t0
			lui $a3,0x00FF
			add $a3,$a3,$t0
			jal PROC_LINHA

			addi $s1,$s1,1
			j PROC_DESENHA_FOR_I
PROC_DESENHA_FIM_FOR_I:
		#}

		addi $s0,$s0,1
		j PROC_DESENHA_FOR_T
PROC_DESENHA_FIM_FOR_T:
	#}

	lw $s2,16($sp)
	lw $s1,12($sp)
	lw $s0,8($sp)
	lw $ra,4($sp)
	addi $sp,$sp,16
	jr $ra
#}


PROC_LINHA:
#void linha(int x0, int x1, int y, int cor){ //x0->$a0, x1->$a1, y->$a2, cor->$a3
	#ptr = &tela[y*64 + x0]; //ptr->$t1
	addi $t0,$zero,64
	mul $t1,$a2,$t0
	add $t1,$t1,$a0
	addi $t0,$zero,4
	mul $t1,$t1,$t0
	lui $t0,0x1001
	add $t1,$t1,$t0

	#for(x=x0; x<=x1; ++x, ++ptr) *ptr = cor; //x->$t2
	add $t2,$zero,$a0
PROC_LINHA_FOR_X:
	slt $t0,$a1,$t2
	bne $t0,$zero,PROC_LINHA_FIM_FOR_X
		sw $a3,0($t1)
		addi $t1,$t1,4
		addi $t2,$t2,1
		j PROC_LINHA_FOR_X
PROC_LINHA_FIM_FOR_X:
	jr $ra
#}
