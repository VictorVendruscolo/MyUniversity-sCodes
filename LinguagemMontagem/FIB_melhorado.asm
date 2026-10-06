#int n, res; //n->$s0, res->$s1
#leia n
addi $v0,$zero,5
syscall
add $s0,$zero,$v0
#res = fib(n)
add $a0,$zero,$s0
jal PROC_FIB
add $s1,$zero,$v0
#imprime res
addi $v0,$zero,1
add $a0,$zero,$s1
syscall
#return 0;
addi $v0,$zero,10
syscall

PROC_FIB:
#int fib(int n){
#if(n<3)
slti $t0,$a0,3
beq $t0,$zero,PROC_FIB_FIM_IF
# return 1
addi $v0,$zero,1
jr $ra
PROC_FIB_FIM_IF:
#return fib_rec(1,1,n-3);
addi $sp,$sp,-4
sw $ra,4($sp)
addi $a2,$a0,-3
addi $a1,$zero,1
addi $a0,$zero,1
jal FIB_REC
#add $v0,$zero,$v0 //não precisa dessa inst.
lw $ra,4($sp)
addi $sp,$sp,4
jr $ra
#}
FIB_REC:
#int fib_rec(int fibNMenos1,int fibNMenos2, int faltaParaN){
#if(faltaParaN==0)
bne $a2,$zero,FIB_REC_FIM_IF
# return fibNMenos1+fibNMenos2
add $v0,$a0,$a1
jr $ra
FIB_REC_FIM_IF:
#return fib_rec(fibNMenos2,fibNMenos1+fibNMenos2,faltaParaN-1);
addi $sp,$sp,-4
sw $ra,4($sp)
add $t0,$zero,$a0
add $a0,$zero,$a1
add $a1,$t0,$a1
addi $a2,$a2,-1
jal FIB_REC
#add $v0,$zero,$v0 //não precisa dessa inst.
lw $ra,4($sp)
addi $sp,$sp,4
jr $ra
#}
