#ifdef _M68K_
#include "rt/rt.h"
#define putchar(c) _rt_putc(c)
#else
#include <stdio.h>
#endif

void
putul(unsigned u)
{
	unsigned x = 10;
	for (; x < u;) x *= 10;
	x /= 10;

	for (; x > 0; x /= 10)
		putchar('0' + (u / x % 10));
}

#ifdef STANDALONE
void
main(void)
{
	putul(256);
	putchar('\n');
}
#endif
