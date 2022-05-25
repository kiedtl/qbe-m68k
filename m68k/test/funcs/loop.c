//: .macro CALL func arg1
//: 	move.l  #\arg1, -(a7)
//: 	bsr     \func
//: 	add.w   #4,a7
//: 	move.l  #0x1, 0x200000
//: .endm
//:
//: 	.globl main
//: main:
//: 	CALL loop1 21
//: 	CALL loop1 0
//: 	CALL loop2 84
//:
//: 	rts

//$ expect_output=""
//$ expect_D0="21 0 0"

unsigned
loop1(unsigned i)
{
	unsigned x = 0;
	for (; x < i; ++x);
	return x;
}

unsigned
loop2(unsigned i)
{
	unsigned x = i;
	for (; x && x == i; --x, --i);
	return x;
}
