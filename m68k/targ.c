#include "all.h"

M68kOp m68k_op[NOp] = {
#define O(op, t, x) [O##op] =
#define V(imm) { imm },
#include "../ops.h"
};

int m68k_rsave[] = {
	D0, D1,
	A0, A1,
	-1
};
int m68k_rclob[] = {
	D2, D3, D4, D5, D6,
	A2, A3, A4, A5,
	-1
};

/* D7 is used as a swap register */
#define RGLOB  (BIT(D7) | BIT(FP) | BIT(SP))
#define NRGLOB 3

static int
m68k_memargs(int op)
{
	(void)op;
	return 0;
}

Target T_m68k = {
	.name = "m68k",
	.gpr0 = D0,
	.ngpr = NGPR,
	.fpr0 = 1,
	.nfpr = NFPR,
	.rglob = RGLOB,
	.nrglob = NRGLOB,
	.rsave = m68k_rsave,
	.nrsave = {NGPS, NFPS},
	.retregs = m68k_retregs,
	.argregs = m68k_argregs,
	.memargs = m68k_memargs,
	.abi = m68k_abi,
	.isel = m68k_isel,
	.emitfn = m68k_emitfn,
};


MAKESURE(rsave_size_ok, sizeof m68k_rsave == (NGPS+NFPS+1) * sizeof(int));
MAKESURE(rclob_size_ok, sizeof m68k_rclob == (NCLR+1) * sizeof(int));
