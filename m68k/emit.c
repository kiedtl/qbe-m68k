#include "all.h"

enum {
	Ki = -1, /* matches Kw and Kl */
	Ka = -2, /* matches all classes */
};

/* Instruction format strings:
 *
 * if the format string starts with *, the instruction
 * is assumed to be 1-address and is put in 2-address
 * mode using an extra mov if necessary
 *
 * if the format string starts with -, the instruction
 * is assumed to be 3-address and is put in 2-address
 * mode using an extra mov if necessary
 *
 * if the format string starts with +, the same as the
 * above applies, but commutativity is also assumed
 *
 * %k  is used to set the class of the instruction,
 *     expanding to `w` or `l` depending on the class.
 * %0  is the first argument
 * %1  is the second argument
 * %=  is the result
 *
 */
static struct {
	short op;
	short cls;
	char *asm;
} omap[] = {
	{ Oadd,    Ki, "+add.%k  %1, %=" },
	/* FIXME: Osub: figure out why convins fails when it's marked as
	 * non-commutative (as it should be); ie why the dest is sometimes %0.
	 */
	{ Osub,    Ki, "+sub.%k  %1, %=" },
	{ Oneg,    Ki, "*neg.%k  %="     },
	{ Odiv,    Ki, "-divs.%k %1, %=" },
	{ Oudiv,   Ki, "-divu.%k %1, %=" },
	{ Omul,    Ki, "+muls.%k %1, %=" },
	{ Oand,    Ki, "-and.%k  %1, %=" },
	{ Oor,     Ki, "-or.%k   %1, %=" },
	{ Oxor,    Ki, "-eor.%k  %1, %=" },
	{ Osar,    Ki, "-asr.%k  %1, %=" },
	{ Oshr,    Ki, "-lsr.%k  %1, %=" },
	{ Oshl,    Ki, "-lsl.%k  %1, %=" },
	{ Oxcmp,   Ki, "cmp.%k  %0, %1" },

	/*
	 * Note, scc/scs is used instead of shs/slo because some
	 * asms (e.g. vasm) don't support those alternative menmonics.
	 */
	{ Oceqw,   Ki, "cmp.%k  %0, %1\n\tseq.b  %="     },
	{ Ocnew,   Ki, "cmp.%k  %0, %1\n\tsne.b  %="     },
	{ Ocsgew,  Ki, "cmp.%k  %0, %1\n\tsge.b  %="     },
	{ Ocsgtw,  Ki, "cmp.%k  %0, %1\n\tsgt.b  %="     },
	{ Ocslew,  Ki, "cmp.%k  %1, %0\n\tsle.b  %="     },
	{ Ocsltw,  Ki, "cmp.%k  %1, %0\n\tslt.b  %="     },
	{ Ocugew,  Ki, "cmp.%k  %1, %0\n\tscc.b  %="     },
	{ Ocugtw,  Ki, "cmp.%k  %1, %0\n\tshi.b  %="     },
	{ Oculew,  Ki, "cmp.%k  %1, %0\n\tsls.b  %="     },
	{ Ocultw,  Ki, "cmp.%k  %1, %0\n\tscs.b  %="     },

	{ Ostoreb, Kw, "move.l %1, a0\n\tmove.b %0, (a0)" },
	{ Ostoreh, Kw, "move.l %1, a0\n\tmove.w %0, (a0)" },
	{ Ostorew, Kw, "move.l %1, a0\n\tmove.l %0, (a0)" },

	/* { Ostoreb, Kw, "move.w %0, %M1" }, */
	/* { Ostoreh, Kw, "move.w %0, %M1" }, */
	/* { Ostorew, Kw, "move.w %0, %M1" }, */
	//{ Ostorel, Ki, "move.l %1, a0\n\tmove.l %0,  (a0)" },

	/* { Oloadsb, Ki, "move.b %M0, %=" }, */
	/* { Oloadub, Ki, "move.b %M0, %=" }, */
	/* { Oloadsh, Ki, "move.w %M0, %=" }, */
	/* { Oloaduh, Ki, "move.w %M0, %=" }, */
	/* { Oloadsw, Ki, "move.l %M0, %=" }, */
	/* { Oloaduw, Ki, "move.l %M0, %=" }, */
	/* { Oload,   Kw, "move.l %M0, %=" }, */

	/*
	 * Load instructions expect a0 to have been loaded with %M0
	 * (See below.)
	 */
	{ Oloadsb, Ki, "move.b (a0), %=\n\text.w  %=" },
	{ Oloadub, Ki, "move.b (a0), %=\n\tandi.w #0xFF, %=" },
	{ Oloadsh, Ki, "move.b (a0), %=\n\text.w  %=" },
	{ Oloaduh, Ki, "move.b (a0), %=\n\tandi.w #0xFF, %=" },
	{ Oloadsw, Ki, "move.l (a0), %=" },
	{ Oloaduw, Ki, "move.l (a0), %=" },
	{ Oload,   Kw, "move.l (a0), %=" },

	{ Oload,   Kl, "[BUG] move.ll %M0, %=" },
	{ Oextsb,  Ki, "*ext.w  %=" },
	{ Oextub,  Ki, "*andi.w #0xFF, %=" },
	{ Oextsh,  Ki, "*ext.w  %=" },
	{ Oextuh,  Ki, "*andi.w #0xFFFF, %=" }, /* TODO: use swap/clr/swap pattern */
	{ Oextsw,  Kl, "[BUG] ext.w  %=" },
	{ Oextuw,  Kl, "[BUG] ext.w  %=" },
	{ Ocopy,   Ki, "move.%k  %0, %=" },
	{ Oswap,   Kw, "exg.l    %0" },
	{ Ocall,   Kw, "bsr      %0" },
	{ Opush,   Ki, "move.%k  %0, -(sp)" },
	{ Oaddr,   Ki, "+add     %0, %=" },
	{ NOp, 0, 0 }
};

static char *clsstr[] = {
	[Kw] = "l",
	[Kl] = "ll",
	[Ks] = "BUG",
	[Kd] = "BUG",
};

static char *rname[] = {
	[D0] = "d0", "d1", "d2", "d3", "d4", "d5", "d6",
	[A0] = "a0", "a1", "a2", "a3", "a4", "a5",
	[A6] = "a6",
	[A7] = "a7",
	[D7] = "d7",
	[SR] = "sr",
};

static void emitins(Ins *i, Fn *fn, FILE *f);

static int64_t
slot(int s, Fn *fn)
{
	s = ((int32_t)s << 3) >> 3;
	assert(s <= fn->slot);
	if (s < 0)
		return 8 + (-s * 4);
	else
		return 8 + (-4 * (fn->slot - s));
}

static void
emitaddr(Con *c, FILE *f)
{
	char off[32], *p;

	if (c->bits.i)
		sprintf(off, "+%"PRIi64, c->bits.i);
	else
		off[0] = 0;
	p = c->local ? ".L" : "";
	fprintf(f, "%s%s%s", p, str(c->label), off);
}

static void
emitcopy(Ref dest, Ref src, int k, Fn *fn, FILE *f)
{
	Ins icp = {0};

	icp.op = Ocopy;
	icp.arg[0] = src;
	icp.to = dest;
	icp.cls = k;

	emitins(&icp, fn, f);
}

/*
 * Convert an A-address instruction to B-address, where A is {2,3} and B is
 * {1,2}.
 *
 * Convmode is one of '*', '+', or '-'. See the comment at the beginning of the
 * opcode table for more info.
 *
 */
static void
convins(Fn *fn, Ins *i, char convmode, FILE *f)
{
	(void)fn;

	switch (convmode) {
	case '*': /* 2-address -> 1-address */
		assert((req(i->arg[1], R)) &&
			"2/3-address instruction was supposed to be 1-address");
		if (!req(i->arg[0], i->to)) {
			emitcopy(i->to, i->arg[0], i->cls, fn, f);
		}
		break;
	case '+': /* 3-address -> 2-address, commutative */
		/* Instruction is commutative, allow i->to and i->arg[1]
		 * to be merged by swapping with i->arg[0] */
		if (req(i->arg[1], i->to)) {
			Ref tmp = i->arg[0];
			i->arg[0] = i->arg[1];
			i->arg[1] = tmp;
		}
		/* fallthrough */
	case '-': /* 3-address -> 2-address */
		if (req(i->arg[1], i->to) && !req(i->arg[0], i->to)) {
			err("ins=%d, char=%c\n", i->op, convmode);
		}

		assert((!req(i->arg[1], i->to) || req(i->arg[0], i->to)) &&
			"cannot convert instruction to 2-address!");
		emitcopy(i->to, i->arg[0], i->cls, fn, f);
		break;
	}
}

static void
emitf(char *s, Ins *i, Fn *fn, FILE *f)
{
	Ref r;
	int c;
	Con *pc;
	int64_t offset;

	switch (*s) {
	break; case '*': case '+': case '-':
		convins(fn, i, *s, f);
		s++;
		break;
	}

	fputc('\t', f);
	for (;;) {
		while ((c = *s++) != '%')
			if (!c) {
				fputc('\n', f);
				return;
			} else
				fputc(c, f);
		switch ((c = *s++)) {
		default:
			die("invalid escape");
			break;
		case 'k':
			fputs(clsstr[i->cls], f);
			break;
		case '=':
		case '0':
		case '1':
			if (c == '=') {
				r = i->to;
			} else {
				assert(c == '0' || c == '1');
				r = i->arg[c - '0'];
			}
			switch (rtype(r)) {
			default:
				die("invalid argument (arg %c)", c);
			case RTmp:
				if (c == '=')
					assert(isreg(r));
				fputs(rname[r.val], f);
				break;
			case RCon:
				pc = &fn->con[r.val];
				assert(pc->type == CBits || pc->type == CAddr);
				if (pc->type == CBits) {
					fprintf(f, "#%ld", pc->bits.i);
				} else if (pc->type == CAddr) {
					char *l = str(pc->label);
					char *p = pc->local ? gasloc : l[0] == '"' ? "" : gassym;
					fprintf(f, "%s%s%s", gaslit, p, l);
					if (pc->bits.i) {
						fprintf(f, "%+"PRId64, pc->bits.i);
					}
				}
				break;
			}
			break;
		case 'M':
			c = *s++;
			assert(c == '0' || c == '1');
			r = i->arg[c - '0'];
			switch (rtype(r)) {
			default:
				die("invalid address argument");
			case RTmp:
				fprintf(f, "0(%s)", rname[r.val]);
				break;
			case RCon:
				pc = &fn->con[r.val];
				assert(pc->type == CAddr);
				emitaddr(pc, f);
				break;
			case RSlot:
				offset = slot(r.val, fn);
				fprintf(f, "%d(fp)", (int)offset);
				break;
			}
			break;
		}
	}
}

static void
loadcon(Con *c, int r, int k, FILE *f)
{
	char *rn;
	int64_t n;
	int w;

	w = KWIDE(k);
	rn = rname[r];
	switch (c->type) {
	case CAddr:
		fprintf(f, "\tmove.l ");
		emitaddr(c, f);
		fprintf(f, ", %s\n", rn);
		break;
	case CBits:
		n = c->bits.i;
		if (!w)
			n = (int32_t)n;
		fprintf(f, "\tmove.%s #%"PRIu64", %s\n", clsstr[k], n, rn);
		break;
	default:
		die("invalid constant");
	}
}

static void
emitins(Ins *i, Fn *fn, FILE *f)
{
	int o;
	Con *con;

	switch (i->op) {
	default:
		/* fallthrough */
	Table:
		/* most instructions are just pulled out of
		 * the table omap[], some special cases are
		 * detailed below */
		for (o=0;; o++) {
			/* this linear search should really be a binary
			 * search */
			if (omap[o].op == NOp)
				die("no match for %s(%c)",
					optab[i->op].name, "wlsd"[i->cls]);
			if (omap[o].op == i->op)
			if (omap[o].cls == i->cls || omap[o].cls == Ka
			|| (omap[o].cls == Ki && KBASE(i->cls) == 0))
				break;
		}
		emitf(omap[o].asm, i, fn, f);
		break;
	case Ocopy:
		if (req(i->to, i->arg[0]))
			break;
		if (rtype(i->to) == RSlot) {
			switch (rtype(i->arg[0])) {
			case RSlot:
			case RCon:
				die("unimplemented");
				break;
			default:
				assert(isreg(i->arg[0]));
				i->arg[1] = i->to;
				i->to = R;
				switch (i->cls) {
				case Kw: i->op = Ostorew; break;
				//case Kl: i->op = Ostorel; break;
				case Kl: die("64-bit stuff unimplemented"); break;
				case Ks: i->op = Ostores; break;
				case Kd: i->op = Ostored; break;
				}
				goto Table;
			}
			break;
		}
		assert(isreg(i->to));
		switch (rtype(i->arg[0])) {
		case RCon:
			loadcon(&fn->con[i->arg[0].val], i->to.val, i->cls, f);
			break;
		case RSlot:
			i->op = Oload;
			goto Table;
		default:
			assert(isreg(i->arg[0]));
			goto Table;
		}
		break;
	case Onop:
		break;
	case Ocall:
		switch (rtype(i->arg[0])) {
		case RCon:
			con = &fn->con[i->arg[0].val];
			if (con->type != CAddr || con->bits.i)
				goto invalid;
			fprintf(f, "\tbsr %s\n", str(con->label));
			break;
		case RTmp:
			emitf("jalr %0", i, fn, f);
			break;
		default:
		invalid:
			die("invalid call argument");
		}
		break;
	case Oloadsb: case Oloadub:
	case Oloadsh: case Oloaduh:
	case Oloadsw: case Oloaduw:
	case Oload:
		switch (rtype(i->arg[0])) {
		default:
			die("invalid address argument");
		case RTmp:
			fprintf(f, "\tmove.l %s, a0\n", rname[i->arg[0].val]);
			goto Table;
			break;
		case RCon:
			;
			Con *pc = &fn->con[i->arg[0].val];
			assert(pc->type == CAddr);
			fprintf(f, "\tmove.l ");
			emitaddr(pc, f);
			fprintf(f, ", a0\n");
			break;
		case RSlot:
			emitf("move.l %M0, %=", i, fn, f);
			break;
		}
		break;
	case Odiv: case Oudiv:
	case Orem: case Ourem:
		die("DIV/UDIV/REM/UREM not implemented");
		break;
	}
}

/*

  Stack-frame layout:

  +=============+
  | varargs     |
  |  save area  |
  +-------------+
  |  saved ra   |
  |  saved fp   |
  +-------------+ <- fp
  |    ...      |
  | spill slots |
  |    ...      |
  +-------------+
  |    ...      |
  |   locals    |
  |    ...      |
  +-------------+
  |   padding   |
  +-------------+
  | callee-save |
  |  registers  |
  +=============+

*/

void
m68k_emitfn(Fn *fn, FILE *f)
{
	static int id0;
	int lbl, neg, *pr, r;
	Blk *b, *s;
	Ins *i;

	gasemitlnk(fn->name, &fn->lnk, ".text", f);

	if (fn->vararg) {
		/* TODO: only need space for registers
		 * unused by named arguments
		 */
		fprintf(f, "\tadd sp, sp, -64\n");
		for (r=A0; r<=A7; r++)
			fprintf(f,
				"\tsd %s, %d(sp)\n",
				rname[r], 8 * (r - A0)
			);
	}

	int frame = (4 * fn->slot + 2) & ~2;
	for (pr=m68k_rclob; *pr>=0; pr++) {
		if (fn->reg & BIT(*pr))
			frame += 4;
	}
	frame = (frame + 2) & ~2;

	fprintf(f, "\tlink   fp, #-%d\n", frame);

	int off;
	for (pr=m68k_rclob, off=0; *pr>=0; pr++) {
		if (fn->reg & BIT(*pr)) {
			fprintf(f,
				"\tmove.l %s, %d(sp)\n",
				rname[*pr], off
			);
			off += 4;
		}
	}

	for (lbl=0, b=fn->start; b; b=b->link) {
		if (lbl || b->npred > 1)
			fprintf(f, ".L%d:\n", id0+b->id);
		for (i=b->ins; i!=&b->ins[b->nins]; i++)
			emitins(i, fn, f);
		lbl = 1;
		switch (b->jmp.type) {
		case Jret0:
			if (fn->dynalloc) {
				/* TODO */
			}
			for (pr=m68k_rclob, off=0; *pr>=0; pr++) {
				if (fn->reg & BIT(*pr)) {
					fprintf(f,
						"\tmove.l %d(sp), %s\n",
						off, rname[*pr]
					);
					off += 4;
				}
			}
			fprintf(f, "\tunlk   fp\n");
			fprintf(f, "\trts\n");
			break;
		case Jjmp:
		Jmp:
			if (b->s1 != b->link)
				fprintf(f, "\tbra   .L%d\n", id0+b->s1->id);
			else
				lbl = 0;
			break;
		case Jjnz:
			neg = 0;
			if (b->link == b->s2) {
				s = b->s1;
				b->s1 = b->s2;
				b->s2 = s;
				neg = 1;
			}
			fprintf(f, "\ttst     %s\n", rname[b->jmp.arg.val]);
			fprintf(f, "\tb%s    .L%d\n", neg ? "ne" : "eq", id0+b->s2->id);
			goto Jmp;
		}
	}
	id0 += fn->nblk;
}
