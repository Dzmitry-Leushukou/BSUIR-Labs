.model small
.stack 100h 

.data
a dw 30 dup (0)
i db 0  
j db 0
n db 48
numb db 5 dup (0)  
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

mov si, 0 ; si - size

input:  

cmp correct, 1
je call wrong
    
cmp i, 30
jge EOP
mov j, 0 
mov ah, 9  
lea dx, cin
int 21h 
mov ah, 1
mov sign, 0
mov correct, 0
int 21h
cmp al, 45 
jne read_ch  
dec j
call set_sign


read_ch:

call clr_cor
mov ah, 1
int 21h
call check 
cmp correct, 1
jmp input 
inc j
cmp j, 5
jl read_ch

call endl
call endl
inc i
jmp input

EOP:
mov ax, 4C00h
int 21h  

set_sign:
mov sign, 1
ret    

clr_sign:
mov sign, 0 
mov numb[0], al
inc j
ret

return: 
ret

check: 
mov n, 48     
call clr_cor
check_loop:
cmp al, n
je call set_cor
inc n
jmp check_loop  
ret

set_cor:
mov correct, 1
ret
clr_cor:
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