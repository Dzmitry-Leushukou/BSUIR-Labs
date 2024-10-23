.model small
.stack 100h 

.data
a dw 30 dup (0)
i dw 0  
j db 0
n db 48
h dw 0  
l dw 0
sign db 0
correct db 0  
;UI
cin db "Write number (-32768, 32767):", 0Dh, 0Ah, '$'
wr db "Wrong input", 0Dh, 0Ah, '$' 
per db 0Dh, 0Ah, '$'  
.code
             
; include data & stack
mov ax,@data
mov ds, ax

; cin
call input    


EOP:
mov ax, 4C00h
int 21h  

set_sign:
mov sign, 1
ret    

clr_sign:
mov sign, 0 
ret

return: 
ret

check: 
mov n, 48     
check_loop: 
cmp al, n
je return 
inc n 
cmp n,58
jge incorrect
jmp check_loop  
ret

incorrect:
mov correct, 0
ret
  
wrong:  
call endl
mov ah, 9  
lea dx, wr
int 21h   
call endl
ret 

endl:
mov ah, 9  
lea dx, per
int 21h 
ret 

prepare_to_iterate:
mov j,0 ; begin of new number
mov sign, 0 ; eq to plus
mov correct, 1;  all correct  
mov bx, 0 
mov h, 0
mov l, 0
ret   

check_negative:
mov correct, 0 ;incor
cmp dx, 0
jne return
cmp bx, 32768
jl return
mov correct, 1;cor
ret

check_positive:
mov correct, 0 ;incor
cmp dx, 0
jne return
cmp bx, 32767
ja return
mov correct, 1;cor
ret
 
input proc  
cmp i, 30 ; i?30
jge EOP  

mov ah, 9  
lea dx, cin
int 21h 

mov ah, 1  

call prepare_to_iterate

int 21h; read


 
call set_sign; set sign
cmp al, 45; if minus 
je read_ch; start read number         


; else first char of number read yet => go to check
call clr_sign
jmp check_ch

read_ch:

mov ah, 1
int 21h
cmp al, 13
je comp

check_ch:
call check 

cmp correct, 0 ; incor
jne cor_check 
wrong_check:
call wrong
jmp input
cor_check:
;else all correct
sub al,'0'
mov ah, 0
mov h, ax

mov ax, 10 
mul bx
mov bx, ax
add bx, h 

to_j:
inc j; j++
cmp j, 5 ; if the end of word
jl read_ch ; read new char

comp:
cmp sign, 0
jne negative
call check_positive 
  
cmp correct, 0 ; incor
jne call iter
call wrong  
jmp input


negative:
;sign - minus     
call to_additional_code
call check_negative 
cmp correct, 0 ; incor
jne call iter
call wrong  
jmp input

iter: 
call endl
mov a[si], bx
inc si
inc si
inc i ; all good i++     
jmp input

ret
ENDP    
   
to_additional_code:
not bx
add bx, 1
ret