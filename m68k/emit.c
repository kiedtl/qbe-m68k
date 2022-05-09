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
	{ Oadd,    Ki, "+add.%k  %=, %1" },
	{ Osub,    Ki, "-sub.%k  %=, %1" },
	{ Oneg,    Ki, "*neg.%k  %=, %0" },
	{ Odiv,    Ki, "-divs.%k %=, %1" },
	{ Oudiv,   Ki, "-divu.%k %=, %1" },
	{ Omul,    Ki, "+muls.%k %=, %0, %1" },
	{ Oand,    Ki, "and %=, %0, %1" },
	{ Oor,     Ki, "or %=, %0, %1" },
	{ Oxor,    Ki, "xor %=, %0, %1" },
	{ Osar,    Ki, "sra%k %=, %0, %1" },
	{ Oshr,    Ki, "srl%k %=, %0, %1" },
	{ Oshl,    Ki, "sll%k %=, %0, %1" },
	{ Ocsltl,  Ki, "slt %=, %0, %1" },
	{ Ocultl,  Ki, "sltu %=, %0, %1" },
	{ Oceqs,   Ki, "feq.s %=, %0, %1" },
	{ Ocges,   Ki, "fge.s %=, %0, %1" },
	{ Ocgts,   Ki, "fgt.s %=, %0, %1" },
	{ Ocles,   Ki, "fle.s %=, %0, %1" },
	{ Oclts,   Ki, "flt.s %=, %0, %1" },
	{ Oceqd,   Ki, "feq.d %=, %0, %1" },
	{ Ocged,   Ki, "fge.d %=, %0, %1" },
	{ Ocgtd,   Ki, "fgt.d %=, %0, %1" },
	{ Ocled,   Ki, "fle.d %=, %0, %1" },
	{ Ocltd,   Ki, "flt.d %=, %0, %1" },
	{ Ostoreb, Kw, "sb %0, %M1" },
	{ Ostoreh, Kw, "sh %0, %M1" },
	{ Ostorew, Kw, "sw %0, %M1" },
	{ Ostorel, Ki, "sd %0, %M1" },
	{ Ostores, Kw, "fsw %0, %M1" },
	{ Ostored, Kw, "fsd %0, %M1" },
	{ Oloadsb, Ki, "lb %=, %M0" },
	{ Oloadub, Ki, "lbu %=, %M0" },
	{ Oloadsh, Ki, "lh %=, %M0" },
	{ Oloaduh, Ki, "lhu %=, %M0" },
	{ Oloadsw, Ki, "move.w %M0, %=" },
	/* riscv64 always sign-extends 32-bit
	 * values stored in 64-bit registers
	 */
	{ Oloaduw, Ki, "move.w %M0, %=" },
	{ Oload,   Kw, "move.w %M0, %=" },
	{ Oload,   Kl, "move.l %M0, %=" },
	{ Oextsb,  Ki, "sext.b %=, %0" },
	{ Oextub,  Ki, "zext.b %=, %0" },
	{ Oextsh,  Ki, "sext.h %=, %0" },
	{ Oextuh,  Ki, "zext.h %=, %0" },
	{ Oextsw,  Kl, "sext.w %=, %0" },
	{ Oextuw,  Kl, "zext.w %=, %0" },
	{ Otruncd, Ks, "fcvt.s.d %=, %0" },
	{ Oexts,   Kd, "fcvt.d.s %=, %0" },
	{ Ostosi,  Kw, "fcvt.w.s %=, %0, rtz" },
	{ Ostosi,  Kl, "fcvt.l.s %=, %0, rtz" },
	{ Ostoui,  Kw, "fcvt.wu.s %=, %0, rtz" },
	{ Ostoui,  Kl, "fcvt.lu.s %=, %0, rtz" },
	{ Odtosi,  Kw, "fcvt.w.d %=, %0, rtz" },
	{ Odtosi,  Kl, "fcvt.l.d %=, %0, rtz" },
	{ Odtoui,  Kw, "fcvt.wu.d %=, %0, rtz" },
	{ Odtoui,  Kl, "fcvt.lu.d %=, %0, rtz" },
	{ Oswtof,  Ka, "fcvt.%k.w %=, %0" },
	{ Ouwtof,  Ka, "fcvt.%k.wu %=, %0" },
	{ Osltof,  Ka, "fcvt.%k.l %=, %0" },
	{ Oultof,  Ka, "fcvt.%k.lu %=, %0" },
	{ Ocast,   Kw, "fmv.x.w %=, %0" },
	{ Ocast,   Kl, "fmv.x.d %=, %0" },
	{ Ocast,   Ks, "fmv.w.x %=, %0" },
	{ Ocast,   Kd, "fmv.d.x %=, %0" },
	{ Ocopy,   Ki, "move.%k %=, %0" },
	{ Oswap,   Kl, "exg.l %0, %1" }, /* TODO: is Oswap EXG or SWAP? */
	//{ Oswap,   Ki, "mv %?, %0\n\tmv %0, %1\n\tmv %1, %?" },
	{ Oreqz,   Ki, "seqz %=, %0" },
	{ Ornez,   Ki, "snez %=, %0" },
	{ Ocall,   Kw, "jalr %0" },
	{ Opush,   Ki, "move.%k %0, -(a7)" },
	{ Oaddr,   Ki, "add %0, %=" }, /* TODO: assert that %1 == %= */
	{ NOp, 0, 0 }
};

static char *clsstr[] = {
	[Kw] = "l",
	[Kl] = "BUG",
	[Ks] = "BUG",
	[Kd] = "BUG",
};

static char *rname[] = {
	[D0] = "d0", "d1", "d2", "d3", "d4", "d5", "d6",
	[A0] = "a0", "a1", "a2", "a3", "a4", "a5",
	[A6] = "a6",
	[A7] = "a7",
	[D7] = "d7",
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
	int k, c;
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
		k = i->cls;
		while ((c = *s++) != '%')
			if (!c) {
				fputc('\n', f);
				return;
			} else
				fputc(c, f);
		switch ((c = *s++)) {
		default:
			die("invalid escape");
		case '?':
			if (KBASE(k) == 0)
				fputs("t6", f);
			else
				abort();
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
				die("invalid second argument");
			case RTmp:
				if (c == '=')
					assert(isreg(r));
				fputs(rname[r.val], f);
				break;
			case RCon:
				pc = &fn->con[r.val];
				assert(pc->type == CBits || pc->type == CAddr);
				if (pc->type == CBits) {
					fprintf(f, "#%d", (int)pc->bits.i);
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
		fprintf(f, "\tmove.l %s, ", rn);
		emitaddr(c, f);
		fputc('\n', f);
		break;
	case CBits:
		n = c->bits.i;
		if (!w)
			n = (int32_t)n;
		fprintf(f, "\tmove %s, %"PRIu64"\n", rn, n);
		break;
	default:
		die("invalid constant");
	}
}

static void
fixslot(Ref *pr, Fn *fn, FILE *f)
{
	Ref r;
	int64_t s;

	r = *pr;
	if (rtype(r) == RSlot) {
		s = slot(r.val, fn);
		if (s < -2048 || s > 2047) {
			fprintf(f, "\tli d7, %"PRId64"\n", s);
			fprintf(f, "\tadd d7, fp, d7\n");
			*pr = TMP(D7);
		}
	}
}

static void
emitins(Ins *i, Fn *fn, FILE *f)
{
	int o;
	Con *con;

	switch (i->op) {
	default:
		if (isload(i->op))
			fixslot(&i->arg[0], fn, f);
		else if (isstore(i->op))
			fixslot(&i->arg[1], fn, f);
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
				case Kl: i->op = Ostorel; break;
				case Ks: i->op = Ostores; break;
				case Kd: i->op = Ostored; break;
				}
				fixslot(&i->arg[1], fn, f);
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
			fixslot(&i->arg[0], fn, f);
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
	case Orem: case Ourem:
		die("REM/UREM not implemented");
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
	int lbl, neg, off, *pr, r;
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

	int frame = (16 + 4 * fn->slot + 15) & ~15;
	for (pr=m68k_rclob; *pr>=0; pr++) {
		if (fn->reg & BIT(*pr))
			frame += 8;
	}
	frame = (frame + 15) & ~15;

	for (pr=m68k_rclob, off=0; *pr>=0; pr++) {
		if (fn->reg & BIT(*pr)) {
			fprintf(f,
				"\tmove.l %d(sp), %s,\n",
				off, rname[*pr]
			);
			off += 8;
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
				if (frame - 16 <= 2048)
					fprintf(f,
						"\tadd sp, fp, -%d\n",
						frame - 16
					);
				else
					fprintf(f,
						"\tli t6, %d\n"
						"\tsub sp, fp, t6\n",
						frame - 16
					);
			}
			for (pr=m68k_rclob, off=0; *pr>=0; pr++) {
				if (fn->reg & BIT(*pr)) {
					fprintf(f,
						"\tmove.l %s, %d(sp)\n",
						rname[*pr], off
					);
					off += 8;
				}
			}
			fprintf(f, "\trts\n");
			break;
		case Jjmp:
		Jmp:
			if (b->s1 != b->link)
				fprintf(f, "\tj .L%d\n", id0+b->s1->id);
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
			assert(isreg(b->jmp.arg));
			fprintf(f, "\ttst %s\n", rname[b->jmp.arg.val]);
			fprintf(f, "\tb%s .L%d\n", neg ? "ne" : "eq", id0+b->s2->id);
			goto Jmp;
		}
	}
	id0 += fn->nblk;
}
