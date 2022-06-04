//: .macro M_DIVMODU32 dvd dvs
//: 	move.l  #0, -(a7)
//: 	move.l  #\dvs, -(a7)
//: 	move.l  #\dvd, -(a7)
//: 	bsr     divmod32
//: 	add.w   #12,a7
//: 	move.l  #0x1, 0x200000
//: .endm
//:
//: 	.globl main
//: main:
//:
//: 	M_DIVMODU32    21   7
//: 	M_DIVMODU32    10   2
//: 	M_DIVMODU32    21 342
//: 	M_DIVMODU32     1   1
//: 	M_DIVMODU32 0xFFF  24
//:
//: 	rts

//$ expect_output=""
//$ expect_D0="3 5 0 1 170"

#include "libqbe.h"

u32
divmod32(u32 dvd, u32 dvs, u32 *rem_dest)
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
