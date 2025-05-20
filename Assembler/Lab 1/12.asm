;save A and B accumulator values
 psha
 pshb

;fill memory cell
 ldaa #255
 staa $1
 ldab #255
 stab $2

;do mul operation
 mul
 std $3

;repair A and B values
 pulb
 pula
