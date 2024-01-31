N = 31
M = N * 2
DATA = 0xbbcc

.data  
	.space 20
A:	.space lo(A), 0xBC
	.byte 1
	.even
label1:
	.word 2, 3, 4, 5
	.byte 2,2,3,-3,0xFF
l2:	.byte 4,33,0xAA
	.even

.text
	MOVI R0, lo(DATA)
	MOVHI R0, hi(DATA)
	CMPEQ R1, R4, R5
	AND R3, R1, R2
	MOVI R3, lo (A) ; this is a comment
B:	MOVHI R3, hi(A)
	ADDI R0, R0, N
	ST N(R4), R3
	JALR R6, R6
	STB N(R4), R3
	OUT 0, R5
	BZ R4, B
.end



