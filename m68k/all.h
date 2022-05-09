#include "../all.h"

typedef struct M68kOp M68kOp;

#define FP A6
#define SP A7

enum M68kReg {
	D0 = RXX + 1, D1, D2, D3, D4, D5, D6,
	A0, A1, A2, A3, A4, A5,

	A6, A7,
	D7,

	NFPR = 0, /* No floating-point registers */
	NGPR = D7 - D0 + 1,
	NGPS = (D1 - D0 + 1) + (A1 - A0 + 1),
	NFPS = 0,
	NCLR = (A5 - A2 + 1) + (D6 - D2 + 1),
};
MAKESURE(reg_not_tmp, D7 < (int)Tmp0);

struct M68kOp {
	char imm;
};

/* targ.c */
extern int m68k_rsave[];
extern int m68k_rclob[];
extern M68kOp m68k_op[];

/* lower.c */
void m68k_lower(Fn *);

/* abi.c */
bits m68k_retregs(Ref, int[2]);
bits m68k_argregs(Ref, int[2]);
void m68k_abi(Fn *);

/* isel.c */
void m68k_isel(Fn *);

/* emit.c */
void m68k_emitfn(Fn *, FILE *);
