//: 	.globl main
//: main:
//: 	bsr     hello
//: 	rts

//$ expect_output="Hello, world!"

#include "rt/rt.h"

void
hello(void)
{
	_rt_puts("Hello, world!\n");
}
