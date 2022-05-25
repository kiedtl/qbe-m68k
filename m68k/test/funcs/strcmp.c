//: 	.align 2
//: S0: .string "Vermin Supreme 2024!"
//: 	.align 2
//: S1: .string "Vermil Supreme 2024!"
//: 	.align 2
//: S2: .string "Vermiz Supreme 2024!"
//:
//: .macro M_STRCMP a b
//: 	move.l  #\b, -(a7)
//: 	move.l  #\a, -(a7)
//: 	bsr     strcmp
//: 	add.w   #8,a7
//: 	move.l  #0x1, 0x200000
//: .endm
//:
//: 	.align 2
//: 	.globl main
//: main:
//: 	M_STRCMP S0 S0
//: 	M_STRCMP S0 S1
//: 	M_STRCMP S0 S2
//: 	M_STRCMP S1 S2
//: 	M_STRCMP S2 S2
//:
//: 	rts

//$ expect_output=""
//$ expect_D0="0 4294967294 12 14 0"

/* Stolen from musl's implementation. */
int
strcmp(char *a, char *b)
{
	for (; *a && *a == *b; ++a, ++b);
	return *(unsigned char *)a - *(unsigned char *)b;
}
