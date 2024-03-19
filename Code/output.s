.data
_prompt: .asciiz "Enter an integer:"
_ret: .asciiz "\n"
.globl main
.text

read:
	li $v0, 4
	la $a0, _prompt
	syscall
	li $v0, 5
	syscall
	jr $ra

write:
	li $v0, 1
	syscall
	li $v0, 4
	la $a0, _ret
	syscall
	move $v0, $0
	jr $ra


main:
	li $t0, 2
	move $t1, $t0
	li $t2, 5
	move $t0, $t2
	li $t3, 10
	move $t4, $t3
	li $t5, 3
	li $t6, 4
	mul $t7, $t5, $t6
	li $s0, 5
	mul $s1, $t7, $s0
	li $s2, 10
	sub $s3, $s1, $s2
	mul $s4, $t1, $t0
	li $s5, 3
	mul $s6, $s4, $s5
	sub $s7, $0, $s6
	sub $t8, $s3, $s7
	mul $t9, $t1, $t0
  sw $t0, -28($sp)
	sub $t0, $t8, $t9
  sw $t1, -32($sp)
  sw $t2, -36($sp)
  sw $t3, -40($sp)
	mul $t1, $t2, $t3
  sw $t4, -44($sp)
  lw $t0, -28($sp)
  lw $t1, -32($sp)
	sub $t4, $t0, $t1
  sw $t5, -48($sp)
	li $t5, 3
  sw $t6, -52($sp)
	add $t6, $t4, $t5
  sw $t7, -56($sp)
	li $t7, 2
  sw $s0, -60($sp)
	add $s0, $t6, $t7
  sw $s1, -64($sp)
	li $s1, 1
  sw $s2, -68($sp)
	add $s2, $s0, $s1
  sw $s3, -72($sp)
	move $s3, $s2
	move $a0, $s3
	addi $sp, $sp, -4
	sw $ra, 0($sp)
	jal write
	lw $ra, 0($sp)
	addi $sp, $sp, 4
  sw $s4, -76($sp)
	li $s4, 0
	move $v0, $s4
	jr $ra
