 ldx #%11000000
 ldaa #%00000100
 staa 0,x
 brclr 0,x,#%00000011,ceh
 ldaa #3
ceh:
 ldaa #2