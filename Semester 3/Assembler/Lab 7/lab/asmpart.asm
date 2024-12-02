.686
.MODEL FLAT, C
.STACK
.DATA
;-----------Local data------------------------------
n dw 160;const for S
two dd 2
s dd 0; 4byte
f dw 0
f_old dw 0
x dw 0
a dw 0
b dw 0
c1 dw 0
d dw 0
min dw 65535
;segment
l dw 0
r dw 0

;flags
connect_direction dw 0
printflag dw 1
;UI
OFMSG BYTE "Overflow", 10, 13, 0
FMSG BYTE " %hu", 10, 13, 0
UMSG BYTE "S = %u", 10, 13,0
XMSG BYTE "[%hu]", 0
.CODE
;-----------External usage--------------------------
EXTRN  printf : proc;// we'll use printf |printf(ebx,eax)|
EXTRN  put : proc
EXTRN  set_min : proc
EXTRN  sqrtx : proc
;-----------Function definitions--------------------

;UI PRC
showfvalue proc
lea ebx, FMSG; ebx = &OFMSG
mov eax, 0
mov ax, f
push eax
push ebx
call printf
add esp, 8;pop x2
ret
showfvalue endp

showintvalue proc ;value stored in eax
lea ebx, UMSG; ebx = &OFMSG
push eax
push ebx
call printf
add esp, 8;pop x2
ret
showintvalue endp

showX proc
lea ebx, XMSG; ebx = &OFMSG
mov eax, 0
mov ax, x
push eax
push ebx
call printf
add esp, 8;pop x2
ret
showX endp


showOFmsg proc
lea ebx, OFMSG; ebx = &OFMSG
push eax
push ebx
call printf
add esp, 8;pop x2
ret
showOFmsg endp


;READ PRC
ReadType1 PROC
push ebp
mov ebp,esp

mov ax, [ebp + 8+4]
mov a, ax
mov ax, [ebp + 12+4]
mov b, ax
mov ax, [ebp + 16+4]
mov c1, ax
mov ax, [ebp + 20+4]
mov d, ax
mov ax, [ebp + 24+4]
mov l, ax
mov ax, [ebp + 28+4]
mov r, ax

mov esp, ebp
pop ebp

RETN
ReadType1 ENDP


ReadType2 PROC
push ebp
mov ebp,esp

mov ax, [ebp + 8+4]
mov a, ax
mov ax, [ebp + 12+4]
mov b, ax
mov ax, [ebp + 16+4]
mov l, ax
mov ax, [ebp + 20+4]
mov r, ax

mov esp, ebp
pop ebp

RETN
ReadType2 ENDP


;GRAPH PRC
buildGraphType1 PROC
mov min, 65535
mov printflag, 0
call ReadType1
mov ax,l
mov x,ax
call FindMin1
mov printflag, 1
mov ax, l
bgtloop1:
cmp ax, r
ja bgtend1

mov x, ax
call find1

mov ax, f
mov f_old, ax
mov ax, x
inc ax

jmp bgtloop1

bgtend1:	
retn

retn
buildGraphType1 ENDP


buildGraphType2 PROC ;ab^X a,b,l,r
mov printflag, 0
mov min, 65535
call ReadType2
mov ax,l
mov x,ax
call FindMin2
mov printflag, 1
mov ax, l
mov x, ax
bgtloop2:
cmp ax, r
ja bgtend2

mov x, ax
call find2

mov ax, f
mov f_old, ax
mov ax, x
inc ax

jmp bgtloop2

bgtend2:	
retn
buildGraphType2 ENDP

BuildSqrt proc
push ebp
mov ebp,esp

mov ax, [ebp + 8]
mov l, ax
mov ax, [ebp + 12]
mov r, ax
mov esp, ebp
pop ebp

mov printflag, 0
mov min, 65535
mov ax,l
mov x,ax
call FindMin3
mov printflag, 1
mov ax, l
mov x, ax
bgtloop3:
mov ax, x
cmp ax, r
ja bgtend3

call find3

mov ax, f
mov f_old, ax
inc x

jmp bgtloop3

bgtend3:	
retn
buildSqrt ENDP
;SQUARE PRC
findSquare PROC
mov s, 0
mov si, l
inc si
inc si

lofs:
cmp si, r
jae gfsr
mov x,si
call find1
mov eax, 0
mov ax, f
add s, eax
jc overflow
inc si
inc si

jmp lofs

overflow:
call showOFmsg
jmp fsr
gfsr:
mov eax, s
mul two
jc overflow
mov s, eax
mov eax, 0
mov ax, r
mov x, ax
call find1
mov eax, 0
mov ax, f

add s, eax
jc overflow
mov eax, 0
mov ax, l
mov x, ax
call find1
mov eax, 0
mov ax, f

add s, eax
jc overflow

mov eax,s
call showintvalue
fsr:
retn
findSquare ENDP


;FIND F(X) VALUE

find3 PROC
mov eax, 0
mov ax, x
push eax
call sqrtx ;eax - ans
mov f,ax
pop eax

cmp printflag, 0
je retfind3
call showX
call showfvalue
call trysetpixel

retfind3:
retn
find3 ENDP

find1 PROC
mov bx, d
mov f, bx


cmp c1, 0
je skc1
mov ax, x
mul c1
jc inbuild_overflow1
add f, ax
jc inbuild_overflow1

skc1:
cmp b, 0
je skb
mov ax, x
mul x
jc inbuild_overflow1
mul b
jc inbuild_overflow1
add f, ax
jc inbuild_overflow1

skb:
cmp a, 0
je ska
mov ax, x
mul x
jc inbuild_overflow1
mul x
jc inbuild_overflow1
mul a
jc inbuild_overflow1
add f, ax
jc inbuild_overflow1

ska:
jmp good_value1
inbuild_overflow1:
cmp printflag, 0
je retfind1
call showX
call showOFmsg

jmp retfind1

good_value1:

cmp printflag, 0
je retfind1
call showX
call showfvalue
call trysetpixel


retfind1:
retn
find1 ENDP


find2 PROC
mov si, x
mov bx, a
mov f, bx

loopf2:
cmp si, 0 ; End of loop
je good_value

mov ax, b
mul f 

jc inbuild_overflow

mov f, ax
dec si
jmp loopf2

inbuild_overflow:
cmp printflag, 0
je retfind2
call showX
call showOFmsg

jmp retfind2

good_value:
cmp printflag, 0
je retfind2
call showX
call showfvalue
call trysetpixel

retfind2:
retn
find2 ENDP


;Find min prc
FindMin1 PROC

fml1:
mov si, x
cmp si,r
jae fmr1
mov ax, si
call find1
inc x
mov ax, f
cmp ax, min
jae fml1
mov min,ax
jmp fml1
fmr1:
call return_main
RETN
FindMin1 ENDP


FindMin2 PROC

fml2:
mov si, x
cmp si,r
jae fmr2

mov ax, si
call find2
inc x
mov ax, f
cmp ax, min
jae fml2
mov min,ax
jmp fml2
fmr2:
call return_main
RETN
FindMin2 ENDP


FindMin3 PROC

fml3:
mov si, x
cmp si,r
jae fmr3

mov ax, si
call find3
inc x
mov ax, f
cmp ax, min
jae fml3
mov min,ax
jmp fml3
fmr3:
call return_main
RETN
FindMin3 ENDP


return_main proc
mov eax, 0
mov ax, min
push eax
call set_min
pop eax
retn
return_main endp

;PIXEL PRC
trysetpixel PROC
push ax
mov ax, x
cmp ax, l ; not first value of x => need connect two vert
je skip_connect
call connect
skip_connect:
call setpixel
pop ax
retn
trysetpixel endp

setpixel PROC
push eax
push ebx
mov eax, 0
mov ebx, 0
mov ax, f
mov bx, x
push eax
push ebx
call put
add esp, 8;pop x2
pop ebx
pop eax
retn
setpixel endp


connect PROC
push bx
push ax
mov bx,x
dec bx
mov ax, f_old
mov connect_direction, 1 ; +
cmp ax, f
jb connect_loop
mov connect_direction, -1 ; -

connect_loop:
cmp ax, f
je ecl

push eax
push ebx
call put
pop ebx
pop eax
cmp connect_direction, 1
jne dc
inc ax
jmp connect_loop
dc:
dec ax
jmp connect_loop
ecl:
pop ax
pop bx
retn
connect endp
END