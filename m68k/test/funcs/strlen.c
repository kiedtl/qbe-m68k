//: 	.align 2
//: S1: .string "Hello, world!"
//: 	.align 2
//: S2: .string ""
//: 	.align 2
//: S3: .string "Vermin Supreme 2024!"
//: 	.align 2
//: S4: .string "\0\0\0"
//:
//: .macro M_STRLEN str
//: 	move.l  #\str, -(a7)
//: 	bsr     strlen
//: 	add.w   #4,a7
//: 	move.l  #0x1, 0x200000
//: .endm
//:
//: 	.globl main
//: main:
//: 	M_STRLEN S1
//: 	M_STRLEN S2
//: 	M_STRLEN S3
//: 	M_STRLEN S4
//:
//: 	rts

//$ expect_output=""
//$ expect_D0="13 0 20 0"

#include "libqbe.h"

u32
strlen(char *s)
{
	unsigned r = 0;
	while (*s) ++s, ++r;
	return r;
}
