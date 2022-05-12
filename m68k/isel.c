#include "all.h"

static void
fixarg(Ref *r, int k, Ins *i, Fn *fn)
{
	(void)r;
	(void)k;
	(void)i;
	(void)fn;
}

static void
negate(Ref *pr, Fn *fn)
{
	Ref r;

	r = newtmp("isel", Kw, fn);
	emit(Oxor, Kw, *pr, r, getcon(1, fn));
	*pr = r;
}

/*
 * Convert cmp operations into some CCR-bit fiddling.
 *
 * TODO: we should instead be emitting a conditional branch to blocks that set
 * the dest as appropriate, like vbcc does.
 */
static void
selcmp(Ins i, int k, int op, Fn *fn)
{
	(void)k;

	Ref tmp;

	switch (op) {
	break; case Cieq:
		/*
		 * NE = Z
		 *
		 * move   ccr, dest
		 * and    0x4, dest
		 */
		emit(Oand, Kl, i.to, i.to, getcon(0x0004, fn));
		emit(Ocopy, Kl, i.to, TMP(CCR), i.to);
	break; case Cine:
		/*
		 * NE = ~Z
		 *
		 * move   ccr, dest
		 * and    0x4, dest
		 * not    dest
		 */
		emit(Oxor, Kl, i.to, i.to, getcon(-1, fn));
		emit(Oand, Kl, i.to, i.to, getcon(0x0004, fn));
		emit(Ocopy, Kl, i.to, TMP(CCR), i.to);
	break; case Cisge:
		tmp = newtmp("isel", Kl, fn);
		/*
		 * SGE = N.V + ~N.~V
		 *
		 * move   ccr, dest
		 * and    0x4, dest
		 * move   dest, tmp
		 * not    tmp
		 * add    tmp, dest
		 */
		emit(Oadd, Kl, i.to, tmp, i.to);
		emit(Oxor, Kl, tmp, i.to, getcon(-1, fn));
		emit(Ocopy, Kl, i.to, tmp, i.to);
		emit(Oand, Kl, i.to, i.to, getcon(0x000a, fn));
		emit(Ocopy, Kl, i.to, TMP(CCR), i.to);
	break; case Cisgt:
		err("Cisgt unimplemented");
	break; case Cisle:
		err("Cisle unimplemented");
	break; case Cislt:
		err("Cislt unimplemented");
	break; case Ciuge:
		/*
		 * UGE = ~C
		 *
		 * move   ccr, dest
		 * and    0x1, dest
		 * not    dest
		 */
		emit(Oxor, Kl, i.to, i.to, getcon(-1, fn));
		emit(Oand, Kl, i.to, i.to, getcon(0x0001, fn));
		emit(Ocopy, Kl, i.to, TMP(CCR), i.to);
	break; case Ciugt:
		/*
		 * UGT = ~C.~Z
		 *
		 * move   ccr, dest
		 * and    0x5, dest
		 * not    dest
		 */
		emit(Oxor, Kl, i.to, i.to, getcon(-1, fn));
		emit(Oand, Kl, i.to, i.to, getcon(0x0005, fn));
		emit(Ocopy, Kl, i.to, i.to, TMP(CCR));
	break; case Ciule:
		err("Ciule unimplemented");
	break; case Ciult:
		/*
		 * ULT = C
		 *
		 * move   ccr, dest
		 * and    0x1, dest
		 */
		emit(Oand, Kl, i.to, i.to, getcon(0x0001, fn));
		emit(Ocopy, Kl, i.to, i.to, TMP(CCR));
	break;
	}

	emit(Oxcmp, i.cls, R, i.arg[0], i.arg[1]);
}

static void
sel(Ins i, Fn *fn)
{
	if (INRANGE(i.op, Oalloc, Oalloc1)) {
		Ins *i0 = curi - 1;
		salloc(i.to, i.arg[0], fn);
		fixarg(&i0->arg[0], Kl, i0, fn);
		return;
	}

	int ck, cc;
	if (iscmp(i.op, &ck, &cc)) {
		selcmp(i, ck, cc, fn);
		return;
	}

	switch (i.op) {
	break; case Onop:
		/* do nothing */
	break; default:
		emiti(i);
		Ins *i0 = curi; /* fixarg() can change curi */
		fixarg(&i0->arg[0], argcls(&i, 0), i0, fn);
		fixarg(&i0->arg[1], argcls(&i, 1), i0, fn);
	break;
	}
}

static void
seljmp(Blk *b, Fn *fn)
{
	/* TODO: replace cmp+jnz with beq/bne/blt[u]/bge[u] */
	if (b->jmp.type == Jjnz)
		fixarg(&b->jmp.arg, Kw, 0, fn);
}

void
m68k_isel(Fn *fn)
{
	Blk *b, **sb;
	Ins *i;
	Phi *p;
	uint n;
	int al;
	int64_t sz;

	/* assign slots to fast allocs */
	b = fn->start;
	/* specific to NAlign == 3 */ /* or change n=4 and sz /= 4 below */
	for (al=Oalloc, n=4; al<=Oalloc1; al++, n*=2)
		for (i=b->ins; i<&b->ins[b->nins]; i++)
			if (i->op == al) {
				if (rtype(i->arg[0]) != RCon)
					break;
				sz = fn->con[i->arg[0].val].bits.i;
				if (sz < 0 || sz >= INT_MAX-15)
					err("invalid alloc size %"PRId64, sz);
				sz = (sz + n-1) & -n;
				sz /= 4;
				if (sz > INT_MAX - fn->slot)
					die("alloc too large");
				fn->tmp[i->to.val].slot = fn->slot;
				fn->slot += sz;
				*i = (Ins){.op = Onop};
			}

	for (b=fn->start; b; b=b->link) {
		curi = &insb[NIns];
		for (sb=(Blk*[3]){b->s1, b->s2, 0}; *sb; sb++)
			for (p=(*sb)->phi; p; p=p->link) {
				for (n=0; p->blk[n] != b; n++)
					assert(n+1 < p->narg);
				fixarg(&p->arg[n], p->cls, 0, fn);
			}
		seljmp(b, fn);
		for (i=&b->ins[b->nins]; i!=b->ins;)
			sel(*--i, fn);
		b->nins = &insb[NIns] - curi;
		idup(&b->ins, curi, b->nins);
	}

	if (debug['I']) {
		fprintf(stderr, "\n> After instruction selection:\n");
		printfn(fn, stderr);
	}
}
