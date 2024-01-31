	N = 8 ; Asumimos N par

	.data
	V:	.space 2*2*N

.text
	; primera mitad
	MOVI	R0, lo(V)
	MOVHI	R0, hi(V)
	MOVI	R1, lo(N)
	MOVHI	R1, hi(N)
	MOVI	R2, 0
	MOVI	R3, 0
L1:	ADD		R2, R2, R3
	ST		0(R0), R2
	ADDI	R0, R0, 2
	ADDI	R3, R3, 1
	CMPLTU	R4, R3, R1
	BNZ		R4, L1

	; segunda mitad
	MOVI	R2, 0
	MOVI	R3, 0
L2: ADD		R2, R2, R3
	ST		0(R0), R2
	ADDI	R3, R3, 1
	ADD		R2, R2, R3
	ST		2(R0), R2
	ADDI	R0, R0, 4
	ADDI	R3, R3, 1
	CMPLTU	R4, R3, R1
	BNZ		R4, L2
.end


