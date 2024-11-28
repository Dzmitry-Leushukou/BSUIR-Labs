.686
.MODEL FLAT, C
.STACK
.DATA
;-----------Local data------------------------------
helloFormat BYTE "Hello, %s!", 10, 13, 0
.CODE
;-----------External usage--------------------------
EXTRN  printf : proc;// we'll use printf
EXTRN  readName : proc;//and void* readName()
;-----------Function definitions--------------------
sayHello PROC


call readName; eax = readName()
lea ebx, helloFormat; ebx = &helloFormat

;printf(ebx,eax)
push eax
push ebx
call printf
add esp, 8;pop x2

retn

sayHello ENDP

END