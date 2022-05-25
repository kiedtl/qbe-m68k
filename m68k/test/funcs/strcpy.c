//: 	.align 2
//: B:  .string "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
//: 	.align 2
//: S:  .string "Hello, world!\n"
//:
//: 	.align 2
//: 	.globl main
//: main:
//: 	move.l  #S, -(a7)
//: 	move.l  #B, -(a7)
//: 	bsr     strcpy
//: 	add.w   #8,a7
//: 	move.l  #B, -(a7)
//: 	bsr     _rt_puts
//: 	add.w   #4,a7
//:
//: 	rts

//$ expect_output="Hello, world!"
//$ expect_D0=""

void
strcpy(char *dest, char *src)
{
	while (*dest++ = *src++);
}
