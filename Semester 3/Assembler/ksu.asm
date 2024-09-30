 org $8000
 ldx #$ffff
 ldy #$ffff
 stx $fff0
 sty $fff5
 clr $fff2
 
 ldaa $fff0
 ldab $fff6
 mul
 std $fff4

 ldaa $fff4
 ldab $fff6
 mul
 addb $fff4
 std $fff3

 ldaa $fff1
 ldab $fff5
 mul
 addb $fff4
 adca $fff3
 std $fff3
 ldaa $fff2
 adca $fff2
 staa $fff2

 ldaa $fff0
 ldab $fff0
 mul
 addb $fff3
 adca $fff2
 std $fff2
 ldy $fff4
 ldx $fff2