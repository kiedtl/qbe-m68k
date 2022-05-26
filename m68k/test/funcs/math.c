//: 	.include "arch/m68k/mul.asm"
//:
//: .macro CALL func a b
//: 	move.l  #\b, -(a7)
//: 	move.l  #\a, -(a7)
//: 	bsr     \func
//: 	add.w   #8,a7
//: 	move.l  #0x1, 0x200000
//: .endm
//:
//: 	.globl main
//: main:
//:
//: 	CALL mul      7      3        ; normal small mul
//: 	CALL mul     14     10        ; normal small mul
//: 	CALL mul  42342  32791        ; larger mul, still 16-bit
//: 	CALL mul 281938 401825        ; large mul, 32-bit
//: 	CALL mul     21      0        ; x * 0
//: 	CALL mul    321      1        ; x * 1
//:
//: 	CALL mod    100     10        ;
//: 	CALL mod    124     10        ;
//:
//: 	rts

//$ expect_output=""
//$ expect_D0="21 140 1388436522 1620587154 0 321 0 4"

#include "libqbe.h"
#include "rt/rt.h"

#define LIBQBE_NO_BUILTIN_MAGICS
#define LIBQBE_NO_DIVU32_MAGIC
#include "div.c"

unsigned
mul(unsigned a, unsigned b)
{
	return a * b;
}

unsigned
mod(unsigned a, unsigned b)
{
	return a % b;
}
