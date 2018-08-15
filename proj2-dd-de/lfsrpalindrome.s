main:			addiu $a1, $zero, 1
			add $a2, $a0, $zero
			lfsr $a2, $a2

LfsrPalindrome:		beq $a2, $a0, palNotFound
			bitpal $a3, $a2
			beq $a3, $a1, palFound
			lfsr $a2, $a2
			j LfsrPalindrome
			
palNotFound:		add $v0, $a0, $zero			
			jr $ra

palFound:		add $v0, $a2, $zero
			jr $ra
			