#1ª aula segundo semestre: processos, $ra, jal, jr, modularização, pilha

#int a, b, c, d;
#a=1 // $s0
#b=2 // $s1
#c=3 // $s2
#d=5 // $s3

#a = leaf_exemple(a,b,c,d)

#imprime a

#return 0

#int leaf_exemple (int g, int h, int j, int i) {
#	int f;
#	f = (g+h) - (i+j);
#	return f;
#}

addi $s0, $zero, 1
addi $s1, $zero, 2
addi $s2, $zero, 3
addi $s3, $zero, 5

add $a0, $zero, $s0
add $a1, $zero, $s1
add $a2, $zero, $s2
add $a3, $zero, $s3
jal PROC_LEAF_EXAMPLE
add $s0, $zero, $v0

addi $v0, $zero, 1
add $a0, $zero, $s0
syscall

addi $v0, $zero, 10 #encerra antes de executar o procesimento novamente
syscall

PROC_LEAF_EXAMPLE:



	add $t0, $a0, $a1
	add $t1, $a2, $a3
	sub $v0, $t0, $t1
	
	jr $ra

