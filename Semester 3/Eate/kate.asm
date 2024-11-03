 ldx #1
 ldaa #0
 staa $0
start:
 ldab #0
 ldaa #%11110101
 staa 0,x
 rol 0,x
 rolb
 clc
 rol 0,x
 rolb
 clc
 rol 0,x
 rolb
 clc
 rol 0,x
 rolb
 clc
 dex
 ORAB 0,x
 stab 0,x
 inx
 inx
 cpx #65
 bne start
 swi