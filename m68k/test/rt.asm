	.equ OUTPUT_ADDR, 0x400000

vector_table:
	.long STACK_AREA                     ;  0: SP
	.long _rt_begin                      ;  1: PC
	.long _rt_exception_unhandled        ;  2: bus error
	.long _rt_exception_unhandled        ;  3: address error
	.long _rt_exception_unhandled        ;  4: illegal instruction
	.long _rt_exception_unhandled        ;  5: zero divide
	.long _rt_exception_unhandled        ;  6: chk
	.long _rt_exception_unhandled        ;  7: trapv
	.long _rt_exception_unhandled        ;  8: privilege violation
	.long _rt_exception_unhandled        ;  9: trace
	.long _rt_exception_unhandled        ; 10: 1010
	.long _rt_exception_unhandled        ; 11: 1111
	.long _rt_exception_unhandled        ; 12: -
	.long _rt_exception_unhandled        ; 13: -
	.long _rt_exception_unhandled        ; 14: -
	.long _rt_exception_unhandled        ; 15: uninitialized interrupt
	.long _rt_exception_unhandled        ; 16: -
	.long _rt_exception_unhandled        ; 17: -
	.long _rt_exception_unhandled        ; 18: -
	.long _rt_exception_unhandled        ; 19: -
	.long _rt_exception_unhandled        ; 20: -
	.long _rt_exception_unhandled        ; 21: -
	.long _rt_exception_unhandled        ; 22: -
	.long _rt_exception_unhandled        ; 23: -
	.long _rt_exception_unhandled        ; 24: spurious interrupt
	.long _rt_output_ready               ; 25: l1 irq
	.long _rt_exception_ignore           ; 26: l2 irq
	.long _rt_exception_unhandled        ; 27: l3 irq
	.long _rt_exception_unhandled        ; 28: l4 irq
	.long _rt_exception_unhandled        ; 29: l5 irq
	.long _rt_exception_unhandled        ; 30: l6 irq
	.long _rt_nmi                        ; 31: l7 irq
	.long _rt_exception_unhandled        ; 32: trap 0
	.long _rt_exception_unhandled        ; 33: trap 1
	.long _rt_exception_unhandled        ; 34: trap 2
	.long _rt_exception_unhandled        ; 35: trap 3
	.long _rt_exception_unhandled        ; 36: trap 4
	.long _rt_exception_unhandled        ; 37: trap 5
	.long _rt_exception_unhandled        ; 38: trap 6
	.long _rt_exception_unhandled        ; 39: trap 7
	.long _rt_exception_unhandled        ; 40: trap 8
	.long _rt_exception_unhandled        ; 41: trap 9
	.long _rt_exception_unhandled        ; 42: trap 10
	.long _rt_exception_unhandled        ; 43: trap 11
	.long _rt_exception_unhandled        ; 44: trap 12
	.long _rt_exception_unhandled        ; 45: trap 13
	.long _rt_exception_unhandled        ; 46: trap 14
	.long _rt_exception_unhandled        ; 47: trap 15

; ------------------------------------------------------------------------------

	; Entry point
	;
	.globl _rt_begin
_rt_begin:
	move.b  #0, CAN_OUTPUT             ; Reset CAN_OUTPUT
	andi    #0xf8ff, sr                ; clear status register
	reset                              ; reset peripherals

	bsr main

	stop    #27000


	; Default entry point. Print hello world and exit.
	;
	.weak main
main:
	move.l  #STR,-(a7)
	bsr     _rt_puts
	addq.w  #4,a7
	rts


; ----- IO interfacing ---------------------------------------------------------

	; Print a single character. Blocking.
	;
	; NOTE: `puts' assumes that putc doesn't clobber any registers
	.globl _rt_putc
_rt_putc:
	tst.b   CAN_OUTPUT
	beq     _rt_putc
	move.l  (4,a7), OUTPUT_ADDR
	move.b  #0, CAN_OUTPUT
	rts

	; Print an unsigned long integer. Blocking.
	;
	.globl _rt_putul
_rt_putul:
	move.l  (4,a7),d0                  ; d0 is integer


	; Print a string. Blocking.
	;
	.globl _rt_puts
_rt_puts:
	move.l  (4,a7),a0                  ; a0 is the string ptr
.puts_loop:
	move.b  (a0),d2                    ; d0 is scratch
	ext.w   d2
	ext.l   d2

	move.l  d2,-(a7)
	bsr     _rt_putc
	addq.w  #4,a7

	addq.l  #1,a0                      ; ++ptr
	cmp.b   #0,(a0)                    ; have we reached the string end?
	bne     .puts_loop

	rts

; ----- Exception handling -----------------------------------------------------

_rt_exception_ignore:
	rte

_rt_exception_unhandled:
	stop    #2700                      ; wait for NMI
	bra     _rt_exception_unhandled    ; shouldn't get here

_rt_output_ready:
	move.l  d0, -(a7)
	move.b  #1, CAN_OUTPUT
	move.b  OUTPUT_ADDR, d0            ; acknowledge the interrupt
	move.l  (a7)+, d0
	rte

_rt_nmi:
	; perform a soft reset
	move    2700, SR                   ; set status register
	move.l  (vector_table,PC), a7      ; reset stack pointer
	reset                              ; reset peripherals
	jmp     (vector_table+4,PC)        ; reset program counter


; ----- Some allocated space ---------------------------------------------------

	; For the default main
STR:
	.string "Hello, World!\n"


	; Stack area, allocated 255 bytes for it. Must be aligned properly
	; if you don't want weird bus errors.
	;
	; (NOTE: stack grows downwards, hence the .zero before the label)
	.zero 0xff
	.align 4
STACK_AREA:

CAN_OUTPUT:
	.zero 2
