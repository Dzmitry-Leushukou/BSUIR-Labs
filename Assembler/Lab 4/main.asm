.model small
.stack 100h 

.data
a dw 30 dup (0)
lb dw 0
rb dw 0
i dw 0  
j db 0
n db 48
h dw 0  
l dw 0
sign db 0
correct db 0
min dw 32767
max dw -32768 
size dw 0   
left_bound dw 6  
right_bound dw 8  
new_delta dw 0
old_delta dw 0   
error_delta db 0
base dw 0
ost dw 0 
os dw 0
need_size dw 0
size_read dw 0
value dw 0
;UI    
cin db "Write number (-32768, 32767):", 0Dh, 0Ah, '$'
wr db "Wrong input", 0Dh, 0Ah, '$' 
lbound db "Write first bound number (-32768, 32767):", 0Dh, 0Ah, '$' 
szcin db "Write size:(1,30)", 0Dh, 0Ah, '$' 
rbound db "Write second bound number (-32768, 32767):", 0Dh, 0Ah, '$' 
per db 0Dh, 0Ah, '$'      
dexc db "Delta`s exception",0Dh, 0Ah, '$' 
szexc db "Wrong size",0Dh, 0Ah, '$'
exc db "Overflow",0Dh, 0Ah, '$'
na db "Normalised array:",0Dh, 0Ah, '$'   
NULL db "0",0Dh, 0Ah, '$'
.code
             
; include data & stack
mov ax,@data
mov ds, ax

; cin
  
read_size:
call size_cin  
call prepare_to_iterate  
mov bx,0
read_chis:  
mov ah, 1 
int 21h
cmp al, 13
jne reading
cmp size_read, 1
je  size_prc
call bad_size
jmp read_size
reading: 
sub al, '0'
cmp al, 0
jge ok_cin1 
call bad_size
jmp read_size
ok_cin1:    
mov size_read,1
cmp al, 10
jl ok_cin2
call bad_size
jmp read_size
ok_cin2:
mov ah, 0
mov size, ax
mov ax, 10
mul bx
add ax, size
mov bx, ax
jmp read_chis

size_prc:
cmp bx, 30 
jg read_size
mov size, bx
mov need_size, bx
inc need_size
mov left_bound, bx
add left_bound,bx
mov right_bound, bx
inc right_bound 
add right_bound,bx
inc right_bound 
call endl
call input
; left and right bound process  
mov bx, left_bound 
mov ax, a[bx]
mov lb, ax     
mov bx, right_bound
mov ax, a[bx]
mov rb, ax  

call bound_process

;mov si,i    
dec si
dec si
dec si
dec si 
mov size, si 
call find_min
call find_max

mov si, 0     
mov bx, max
sub bx, min
mov old_delta, bx 
jo set_error_delta 
cmp old_delta, 0  ; divide by 0
je set_error_delta 

mov bx, rb
sub bx, lb
mov new_delta, bx
jo set_error_delta
  
mov ah, 9  
lea dx, na
int 21h  
  
normalize_for:
cmp si, size
jge EOP

cmp error_delta, 1
jne prc
call delta_exception
jmp it
prc: ;new_value=((value-old_min)/(old_max-old_min))*(new_max-new_min)+new_min
;value - old_min    

mov ax, a[si]
sub ax, min   
jo of
; lets divide this  
mov dx,0
idiv old_delta
mov ost, dx  
; we should do (base*new_delta)+(ost*new_delta)%old_delta+new_min
;(base*new_delta)  = value
mov bx, new_delta
imul bx     
;mov ul, ax
;mov uh, dx  
jo of
mov value, ax
;(ost*new_delta)%old_delta
mov ax, ost
mov bx, new_delta
imul bx
jo of     
mov cx, old_delta
idiv cx
mov base, ax
mov os, dx

mov dx, 0
mov ax, old_delta
mov cx, 2
idiv cx   
add ax, dx
cmp os, ax
jge inc_sum
jmp next_prc
inc_sum:
inc value
jo of
next_prc:
mov ax, base  
mov bx, value
add bx, ax
jc of    
add bx, lb
jc of

cmp bx,0
jne norm_cout
mov ah, 9  
lea dx, NULL
int 21h 
jmp it
norm_cout:
call cout_ch

jmp it
of:
mov ah, 9  
lea dx, exc
int 21h 

it:
inc si
inc si
jmp normalize_for

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
size_cin:
mov ah, 9  
lea dx, szcin
int 21h 
ret
bad_size: 
call endl
mov ah, 9  
lea dx, szexc
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
mov bx, i 
cmp bx, need_size ; i?31
jg return  

mov bx, i 
cmp bx, size  ; i?30
je left  
jg right
mov ah, 9  
lea dx, cin
int 21h 
jmp iteration_norm

left:
mov ah, 9  
lea dx, lbound
int 21h 

jmp iteration_norm

right:
mov ah, 9  
lea dx, rbound
int 21h 


iteration_norm:
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


find_min:
mov si, 0

fmin_loop:
cmp si, size
jge return  

mov ax, a[si]
cmp min, ax
jg mn
inc si
inc si   
jmp fmin_loop
mn: 

mov min, ax
jmp fmin_loop
ret

find_max:
mov si, 0

fmax_loop:
cmp si, size
jge return  

mov ax, a[si]
cmp max, ax
jl mx
inc si
inc si   
jmp fmax_loop
mx: 
mov max, ax
jmp fmax_loop
ret
    
bound_process:
mov ax, lb
mov bx, rb
cmp ax, bx
jle return
; swap
mov lb, bx
mov rb, ax
ret    
 
set_error_delta:
mov error_delta, 1
jmp normalize_for

delta_exception:
mov ah, 9  
lea dx, dexc
int 21h 
ret 
  
cout_ch: ; ch in bx 
js sign_prc
jmp op     
sign_prc:
mov dl, '-'
mov ax, 0
mov ah,2
int 21h    

mov ax, -1
imul bx
mov bx, ax
op:
mov cx, 10000
cmp bx, 10000
jge print   
mov cx, 1000
cmp bx, 1000
jge print   
mov cx, 100
cmp bx, 100
jge print 
mov cx, 10
cmp bx, 10
jge print 
cmp bx, 0
je end_pr
mov cx, 1
print: 

cmp cx, 0     
je end_pr
call divide

mov ax, cx
mov dx, 0
mov cx, 10
idiv cx
mov cx, ax

jmp print
end_pr:
call endl  
ret 


divide:
mov ax, bx 
mov dx, 0
idiv cx    
mov bx, dx
add ax, '0'
mov dl, al
mov ax, 0
mov ah,2
int 21h
ret