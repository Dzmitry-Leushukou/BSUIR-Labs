 ldaa #%11011010
 ldab #%10101010
 staa $1
 stab $2
 
;Make a and b
 ANDA $2
;Now in a there are only bits that are included in a and b at the same time
 EORA $1