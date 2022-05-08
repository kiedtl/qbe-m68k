#ifdef _M68K_
#define putc(c, f) _rt_putc(c)
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
		putc('0' + (u / x % 10), stderr);
}

#ifndef _M68K_
void
main(void)
{
	putul(256);
	putc('\n', stdout);
}
#endif
