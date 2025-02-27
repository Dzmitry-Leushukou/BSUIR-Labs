 ldx #$8200
 ldaa #255
 staa $8201
 ldaa #255	
 staa $8233
 ldaa #255
 staa $8202
 ldaa #0
 ldy #0
 jmp main
main:
 ldab 0,x
 cmpb #0
 bgt check ; if>0
 cpx #$82ff ;End?
 beq return
 inx
 jmp main
check:
 ldab 0,x
 inca ; a++
 aby ;y+=b
 cpx #$82ff ;End?
 beq return ;end
 inx 
 bne main ;not end 
 
 
return:
 bra *
 