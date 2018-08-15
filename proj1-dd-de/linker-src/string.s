# CS 61C Spring 2015 Project 1-2 
# string.s

#==============================================================================
#                              Project 1-2 Part 1
#                               String README
#==============================================================================
# In this file you will be implementing some utilities for manipulating strings.
# The functions you need to implement are:
#  - strlen()
#  - strncpy()
#  - copy_of_str()
# Test cases are in linker-tests/test_string.s
#==============================================================================

.data
newline:	.asciiz "\n"
tab:	.asciiz "\t"

.text
#------------------------------------------------------------------------------
# function strlen()
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = string input
#
# Returns: the length of the string
#------------------------------------------------------------------------------
strlen:
	addi $sp, $sp, -8
	sw $s0, 0($sp)
	sw $ra, 4($sp)
	li $s0, 0
loop:
	lb $t0, 0($a0)
	beq $t0, 0, finish
	addi $s0, $s0, 1
	addi $a0, $a0, 1
	j loop
finish:
	add $v0, $s0, $0
	lw $s0, 0($sp)
	lw $ra, 4($sp)
	addi $sp, $sp, 8
	jr $ra

#------------------------------------------------------------------------------
# function strncpy()
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = pointer to destination array
#  $a1 = source string
#  $a2 = number of characters to copy
#
# Returns: the destination array
#------------------------------------------------------------------------------
strncpy:
	addi $sp, $sp, -4
	sw $s0, 0($sp) #saved temp variable
	add $s0, $zero, $zero #start incrementer at 0
cpyLoop:
	add $t1, $s0, $a1 #get addr of value in $a1
	lbu $t2, 0($t1) #store it in $t2
	add $t3, $s0, $a0 #get addr of $a0
	sb $t2, 0($t3) # copy value over to $a0
	beq $s0, $a2, cpyEnd #if equal number of characters to copy, exit loop
	addi $s0, $s0, 1 #increment $s0
	j cpyLoop
cpyEnd:
	lw $s0, 0($sp)
	addi $sp, $sp, 4
	add $v0, $a0, $0
	jr $ra



#------------------------------------------------------------------------------
# function copy_of_str()
#------------------------------------------------------------------------------
# Creates a copy of a string. You will need to use sbrk (syscall 9) to allocate
# space for the string. strlen() and strncpy() will be helpful for this function.
# In MARS, to malloc memory use the sbrk syscall (syscall 9). See help for details.
#
# Arguments:
#   $a0 = string to copy
#
# Returns: pointer to the copy of the string
#------------------------------------------------------------------------------
copy_of_str:
	# YOUR CODE HERE
	add $s4, $ra, $0
	add $t2, $a0, $0
	addi $sp, $sp, -4
	sw $t2, 0($sp)
	jal strlen
	add $a0, $v0, $0
	lw $t2, 0($sp)
	addi $sp, $sp, 4
	li $v0, 9
	syscall
	add $a2, $a0, $0
	add $a0, $v0, $0
	add $a1, $t2, $0
	jal strncpy
	add $ra, $s4, $0	
	jr $ra

###############################################################################
#                 DO NOT MODIFY ANYTHING BELOW THIS POINT                       
###############################################################################

#------------------------------------------------------------------------------
# function streq() - DO NOT MODIFY THIS FUNCTION
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = string 1
#  $a1 = string 2
#
# Returns: 0 if string 1 and string 2 are equal, -1 if they are not equal
#------------------------------------------------------------------------------
streq:
	beq $a0, $0, streq_false	# Begin streq()
	beq $a1, $0, streq_false
streq_loop:
	lb $t0, 0($a0)
	lb $t1, 0($a1)
	addiu $a0, $a0, 1
	addiu $a1, $a1, 1
	bne $t0, $t1, streq_false
	beq $t0, $0, streq_true
	j streq_loop
streq_true:
	li $v0, 0
	jr $ra
streq_false:
	li $v0, -1
	jr $ra			# End streq()

#------------------------------------------------------------------------------
# function dec_to_str() - DO NOT MODIFY THIS FUNCTION
#------------------------------------------------------------------------------
# Convert a number to its unsigned decimal integer string representation, eg.
# 35 => "35", 1024 => "1024". 
#
# Arguments:
#  $a0 = int to write
#  $a1 = character buffer to write into
#
# Returns: the number of digits written
#------------------------------------------------------------------------------
dec_to_str:
	li $t0, 10			# Begin dec_to_str()
	li $v0, 0
dec_to_str_largest_divisor:
	div $a0, $t0
	mflo $t1		# Quotient
	beq $t1, $0, dec_to_str_next
	mul $t0, $t0, 10
	j dec_to_str_largest_divisor
dec_to_str_next:
	mfhi $t2		# Remainder
dec_to_str_write:
	div $t0, $t0, 10	# Largest divisible amount
	div $t2, $t0
	mflo $t3		# extract digit to write
	addiu $t3, $t3, 48	# convert num -> ASCII
	sb $t3, 0($a1)
	addiu $a1, $a1, 1
	addiu $v0, $v0, 1
	mfhi $t2		# setup for next round
	bne $t2, $0, dec_to_str_write
	jr $ra			# End dec_to_str()
