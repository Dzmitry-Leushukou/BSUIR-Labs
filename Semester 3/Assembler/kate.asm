START:
    LDAA #55
    BRN SKIP 
    LDAA #$AA
    BSR SKIP 
    LDAA #$BB
SKIP:
    LDAA #1
    RTS
