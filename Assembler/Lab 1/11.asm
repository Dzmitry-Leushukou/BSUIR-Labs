 ldx #5
 ldy #6
	
; 1 way (use 3 because 2 - offset byte)
 stx $1
 sty $3 
 ldx $3
 ldy $1

; 2 way
 xgdx ; (swap x and d)
 xgdy ; (swap d(x) and y) => d=y y=x 
 xgdx

; 3 way
 pshx
 pshy
 pulx ; x=y
 puly ; y=x
 

