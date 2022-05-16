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

static void
selcmp(Ins i, int k, int op, Fn *fn)
{
	(void)k;
	(void)op;

	/* Sxx (SCC & friends) set the <Ea> to 0xFFFF, which might possibly
	 * cause problems. So we convert it to the more usual boolean values.
	 *
	 * TODO: check if this is really necessary.
	 */
	//emit(Oand, i.cls, i.to, i.to, getcon(1, fn));

	emit(i.op, Kw, i.to, R, R);

	/* cmp doesn't like immediates as the second operand */
	if (rtype(i.arg[1]) == RCon)
	if (fn->con[i.arg[1].val].type == CBits) {
		Ref tmp = i.arg[0];
		i.arg[0] = i.arg[1];
		i.arg[1] = tmp;
	}

	emit(Oxcmp, i.cls, R, i.arg[0], i.arg[1]);
}

static void
selload(Ins i, Fn *fn)
{
	(void)fn;
	emiti(i);
}

static void
selstore(Ins i, Fn *fn)
{
	(void)fn;

	//emit(Ocopy, i.cls, i.to, TMP(A0), R);
	//emit(i.op, i.cls, TMP(A0), i.arg[0], i.arg[1]);
	emiti(i);
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
	break; case Ostoreb: case Ostoreh: case Ostorel:
		selstore(i, fn);
	break; case Oloadsb: case Oloadub: case Oloadsh:
	       case Oloaduh: case Oloadsw: case Oloaduw:
		selload(i, fn);
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
