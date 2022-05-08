DEF = -D_M68K_ -DSTANDALONE
QBEFLAGS = -tm68k -Gv
VLINKFLAGS = -brawbin1 -evector_table

all: hello.bin putul.bin

.PHONY: clean
clean:
	rm -f rt/rt.o hello.bin putul.bin

rt/rt.o: rt/rt.asm
	vasmm68k_std $< -Fvobj -o $@

hello.bin: hello.c rt/rt.o
	cpp $(DEF) $< | cproc-qbe > hello.ssa
	qbe $(QBEFLAGS) hello.ssa >| hello.asm
	vasmm68k_std hello.asm -Fvobj -o hello.o
	vlink rt/rt.o hello.o $(VLINKFLAGS) -o $@

putul.bin: putul.c rt/rt.o
	cpp $(DEF) $< | cproc-qbe > putul.ssa
	qbe $(QBEFLAGS) putul.ssa >| putul.asm
	vasmm68k_std putul.asm -Fvobj -o putul.o
	vlink rt/rt.o putul.o $(VLINKFLAGS) -o $@
