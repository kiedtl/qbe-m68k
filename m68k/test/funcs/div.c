//: 	.globl main
//: main:
//: 	move.l  #0, -(a7)
//: 	move.l  #7, -(a7)
//: 	move.l  #21, -(a7)
//: 	bsr     __divmodu32
//: 	add.w   #12,a7
//: 	move.l  #0x1, 0x200000
//:
//: 	rts

//$ expect_output=""
//$ expect_D0="3"

#include "libqbe.h"

u32
__divmodu32(u32 dvd, u32 dvs, u32 *rem_dest)
{
	u32 bit = 1;
	while (dvs < dvd && bit && (dvs & (1 << 31)) == 0) {
		dvs <<= 1;
		bit <<= 1;
	}

	u32 rem = dvd;
	u32 res = 0;

	while (bit != 0) {
		if (rem >= dvs) {
			rem -= dvs;
			res |= bit;
		}

		bit >>= 1;
		dvs >>= 1;
	}

	if (rem_dest != NULL)
		*rem_dest = rem;
	return res;
}
