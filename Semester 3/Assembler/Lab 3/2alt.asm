.model small          
.stack 500h  
.data  
s db 200 dup (0) ; string[200] ($)  
j db 0
size dw 0
ind dw 0
flag db 0

;UI
cin db "Write string of symbols:", 0Dh, 0Ah, '$'
endl db 0Dh, 0Ah, '$'   
buffer_size equ 200 
  
.code  
main:  
    mov ax,@data
    mov ds, ax ;include data&stack 
      
    ;UI cin  
    mov ah, 9  
    lea dx, cin
    int 21h   
    mov bx, 0 ; index of string
    mov ah, 1 ; echo cin char          

    mov si, 0   
    
    
input: 
    cmp bx, buffer_size
    je process  

    int 21h
    cmp al, 13 ; cmp char with enter char
    je process
    
    inc size ; size++
    mov s[bx], al ; s[bx] = char                   
    inc bx
    jmp input 


process:   
    mov si, 0  
      
    mov ah, 9
    lea dx, endl
    int 21h   
    
mov bx, 0
cmp bx, size
je EOP

cmp s[bx], 32
jne for   
 
for:
mov cl, j  
mov ch, 0      
mov bx, cx; min
mov si, cx; j
mov ind, bx
lp:
call comp      
mov bx, ind 
call next_word 

mov ax, si
mov j, al ; begin of next word 
cmp si, size
jl lp
     
mov si,0  
 
call upd_s  
mov ah, 9
lea dx, endl
int 21h 
call check
cmp flag, 0
je EOP 

mov j,0 
cmp s[0], 32
jne for 
call next_word
mov ax, si
mov j, al ; begin of next word
jmp for
check:
mov bx, 0
mov flag, 0
check_loop: 
cmp s[bx], 0
je return
cmp s[bx], 32
jne set_flag
inc bx
jmp check_loop
ret

 
EOP:
    mov ax, 4C00h
    int 21h   
    
        
comp: ;res save in bx
cmp s[si], 32
je umx
cmp s[bx], 32
je return  
cmp s[si], 0
je umx
cmp s[bx], 0
je return

mov al, s[bx] 
mov ah, s[si] 

cmp ah, al
jl umx
jg return
         
inc si
inc bx
jmp comp

        
umx:
mov bl, j
mov bh, 0  
mov ind, bx
ret

   
next_word:
cmp s[si], 32
je next_alpha
cmp s[si], 0
je return
inc si
jmp next_word 

next_alpha:      
cmp s[si], 32
jne return
inc si
jmp next_alpha 
        

upd_s:  ;ax, si - free | i - aim pos, bx - need pos
  
cmp s[bx], 32
je return     
cmp s[bx], 0
je return 
mov dl, s[bx]
mov ah, 2 
int 21h  
mov s[bx], 32
inc bx 
jmp upd_s

return:
ret    

set_flag:
mov flag, 1
ret
  