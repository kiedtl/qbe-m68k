//: 	.align 2
//: S1: .string "Hello, \0\0\0\0\0\0\0\0\0\0"
//: 	.align 2
//: S2: .string "world!\n\0"
//:
//: 	.align 2
//: 	.globl main
//: main:
//: 	move.l  #S2, -(a7)
//: 	move.l  #S1, -(a7)
//: 	bsr     strcat
//: 	add.w   #8,a7
//: 	move.l  #S1, -(a7)
//: 	bsr     _rt_puts
//: 	add.w   #4,a7
//:
//: 	rts

//$ expect_output="Hello, world!"
//$ expect_D0=""

void
strcat(char *dest, char *src)
{
	while (*dest) dest++;
	while (*dest++ = *src++);
	//for (; *src; ++src) *dest++ = *src;
}
