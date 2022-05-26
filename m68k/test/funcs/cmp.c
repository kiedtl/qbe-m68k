//: .macro M_OP a func b
//: 	move.l  #\b, -(a7)
//: 	move.l  #\a, -(a7)
//: 	bsr     \func
//: 	add.w   #8,a7
//: 	move.l  #0x1, 0x200000
//: .endm
//:
//: 	.globl main
//: main:
//: 	M_OP 0    eq 0     ; true     ; 1
//: 	M_OP 1    eq 0     ; false    ; 2
//: 	M_OP 1    ne 1     ; false    ; 3
//: 	M_OP 1    ne 0     ; true     ; 4
//:
//: 	M_OP 342 gtu 0     ; true     ; 5
//: 	M_OP 0   gtu 342   ; false    ; 6
//: 	M_OP 342 gtu 342   ; false    ; 7
//:
//: 	M_OP 342 ltu 0     ; false    ; 8
//: 	M_OP 1   ltu 1000  ; true     ; 9
//: 	M_OP 342 ltu 342   ; false    ; 10
//:
//: 	M_OP  -1 lts 0     ; true     ; 11
//: 	M_OP   0 lts -1    ; false    ; 12
//: 	M_OP  -1 lts -1    ; false    ; 13
//:
//: 	M_OP  -1 gts 0     ; false    ; 14
//: 	M_OP   0 gts -1    ; true     ; 15
//: 	M_OP  -1 gts -1    ; false    ; 16
//:
//: 	M_OP   0 les -1    ; false    ; 17
//: 	M_OP  -1 les 0     ; true     ; 18
//: 	M_OP  -1 les -1    ; true     ; 19
//:
//: 	M_OP   0 ges -1    ; true     ; 20
//: 	M_OP  -1 ges 0     ; false    ; 21
//: 	M_OP  -1 ges -1    ; true     ; 22
//:
//: 	M_OP   0 geu 342   ; false    ; 23
//: 	M_OP 342 geu 0     ; true     ; 24
//: 	M_OP 342 geu 342   ; true     ; 25
//:
//: 	M_OP   0 leu 342   ; true     ; 26
//: 	M_OP 342 leu 0     ; false    ; 27
//: 	M_OP 342 leu 342   ; true     ; 28
//:
//: 	rts

//$ expect_output=""
//                               1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2
//  number     1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8
//             -------------------------------------------------------
//$ expect_D0="1 0 0 1 1 0 0 0 1 0 1 0 0 0 1 0 0 1 1 1 0 1 0 1 1 1 0 1"
//             1 0 0 1 1 0 0 0 1 0 1 0 0 0 1 0 0 1 1 1 0 1 0 1 1 1 0 1

/* inb4 someone posts a screenshot of this terrible formatting
 * onto r/programmerhumor or something: I'll format my test
 * suit harnesses as I so damn please, okay? */

     int  eq(int      a, int      b) { return a == b; }
     int  ne(int      a, int      b) { return a != b; }
unsigned ltu(unsigned a, unsigned b) { return a  < b; }
unsigned gtu(unsigned a, unsigned b) { return a  > b; }
unsigned leu(unsigned a, unsigned b) { return a <= b; }
unsigned geu(unsigned a, unsigned b) { return a >= b; }
     int lts(int      a, int      b) { return a  < b; }
     int gts(int      a, int      b) { return a  > b; }
     int les(int      a, int      b) { return a <= b; }
     int ges(int      a, int      b) { return a >= b; }
