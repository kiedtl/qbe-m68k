//: 	.include "arch/m68k/mul.asm"
//:
//: .macro M_PUTUL x
//: 	move.l  #\x, -(a7)
//: 	bsr     putul
//: 	add.w   #4,a7
//: .endm
//:
//: 	.globl main
//: main:
//: 	M_PUTUL 244
//: 	M_PUTUL 0
//: 	M_PUTUL 833123
//: 	M_PUTUL 256
//:
//: 	rts

//$ expect_output="244 0 833123 256 "

#include "libqbe.h"
#include "rt/rt.h"

#define LIBQBE_NO_BUILTIN_MAGICS
#define LIBQBE_NO_DIVU32_MAGIC
#include "div.c"

void
putul(unsigned u)
{
	unsigned x = 10;
	for (; x < u;) x *= 10;
	x /= 10;

	unsigned y = 0;
	for (; x > 0; x /= 10)
		_rt_putc('0' + (u / x) % 10);

	_rt_putc(' ');
}
