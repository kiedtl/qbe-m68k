//: 	.globl main
//: main:
//: 	move.l  #21, -(a7)
//: 	bsr     loop
//: 	addq.w  #4,a7
//: 	move.l  #0x1, 0x200000
//:
//: 	move.l  #0, -(a7)
//: 	bsr     loop
//: 	addq.w  #4,a7
//: 	move.l  #0x1, 0x200000
//:
//: 	rts

//$ expect_output=""
//$ expect_D0="21 0"

unsigned
loop(unsigned i)
{
	unsigned x = 0;
	for (; x < i; ++x);
	return x;
}
