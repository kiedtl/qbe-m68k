/* From qbe/minic/test/collatz.c */

//: 	.include "arch/m68k/mul.asm"
//:
//:	.globl main
//: main:
//: 	bsr     collatz
//: 	move.l  #0x1, 0x200000
//: 	rts

//$ expect_output=""
//$ expect_D0="178"

#include "rt/rt.h"

unsigned
collatz(void)
{
	//unsigned *buf = (unsigned *)HERE;
	unsigned buf[1000];

	unsigned cmax = 0;
	for (unsigned nv = 1; nv < 1000; nv++) {
		unsigned n = nv;
		unsigned c = 0;

		while (n != 1) {
			if (n < nv) {
				c = c + buf[n];
				break;
			}
			if (n & 1) {
				n = 3 * n + 1;
			} else {
				n = n / 2;
			}
			c++;
		}
		buf[nv] = c;
		if (c > cmax)
			cmax = c;
	}

	return cmax;
}
