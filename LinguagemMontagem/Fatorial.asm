#int n, res; //n->$s0, res->$s1
#leia n
addi $v0,$zero,5
syscall
add $s0,$zero,$v0
#res = fatorial(n)
add $a0,$zero,$s0
jal PROC_FATORIAL
add $s1,$zero,$v0
#imprime res
addi $v0,$zero,1
add $a0,$zero,$s1
syscall
#return 0;
addi $v0,$zero,10
syscall

PROC_FATORIAL:
#int fatorial(int n){
# int fat_n_menos_1; //$s0
# if(n<2)
slti $t0,$a0,2
beq $t0,$zero,FIM_DO_IF
# return 1;
addi $v0,$zero,1
jr $ra
FIM_DO_IF:
# fat_n_menos_1 = fat(n-1);
addi $sp,$sp,-12 #reserva espaço na pilha
sw $ra,4($sp) #salvar na pilha
sw $a0,8($sp)
sw $s0,12($sp)

addi $a0,$a0,-1
jal PROC_FATORIAL
add $s0,$zero,$v0

# return fat_n_menos_1 * n;
lw $a0,8($sp) #recupera argumento
mul $v0,$a0,$s0
lw $s0,12($sp) #recupera S's
lw $ra,4($sp) #recupera $ra
addi $sp,$sp,12
jr $ra
