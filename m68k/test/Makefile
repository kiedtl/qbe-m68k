DEF = -D_M68K_ -DSTANDALONE
QBEFLAGS = -tm68k -Gv
VLINKFLAGS = -brawbin1 -evector_table
VASMFLAGS  = -quiet

all: hello.bin putul.bin

.PHONY: clean
clean:
	rm -f rt/rt.o hello.bin putul.bin

rt/rt.o: rt/rt.asm
	vasmm68k_std $(VASMFLAGS) $< -Fvobj -o $@

hello.bin: hello.c rt/rt.o
	cpp $(DEF) $< | cproc-qbe > hello.ssa
	qbe $(QBEFLAGS) hello.ssa >| hello.asm
	vasmm68k_std $(VASMFLAGS) hello.asm -Fvobj -o hello.o
	vlink rt/rt.o hello.o $(VLINKFLAGS) -o $@

putul.bin: putul.c rt/rt.o
	cpp $(DEF) $< | cproc-qbe > putul.ssa
	qbe $(QBEFLAGS) putul.ssa >| putul.asm
	vasmm68k_std $(VASMFLAGS) putul.asm -Fvobj -o putul.o
	vlink rt/rt.o putul.o $(VLINKFLAGS) -o $@
