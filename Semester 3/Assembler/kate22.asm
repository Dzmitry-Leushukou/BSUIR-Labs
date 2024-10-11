 ldx #2
 ldab #0
 ldaa #%11110100
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
 stab 0,x
 inx
 inx

start:
 staa 0,x
 ldab #0

 rol 0,x 
 rol 0,x
 rol 0,x
 rol 0,x

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

 rol 0,x ; мусор убираем

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
 stab 0,x
 inx
 inx
 cpx #66
 bne start
 swi