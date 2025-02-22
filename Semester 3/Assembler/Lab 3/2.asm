.model small          
.stack 100h  
.data  
s db 200 dup (0) ; string[200] ($)  
i db 0
j db 0
size dw 0
mi db 0
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
    
; check i 
mov bl, i
mov bh, 0  

cmp bx, size
jge EOP

cmp s[bx], 32
jne for   

 
find_i:      
inc bx
cmp s[bx], 32
je find_i

mov i,bl 

 
for:
mov cl, i
mov j, cl ; i=j   
mov mi, cl
mov ch, 0      
mov bx, cx; min
mov si, cx; j
; find min on [j;size]  

lp:
call comp
call next_word 
mov ax, si
mov j, al ; begin of next word 
cmp si, size
jl lp
     
mov si,0    
call upd_s
mov bl, i
call next_word
mov i, bl
cmp bx, size
jl for

print_loop: 

    mov dl, s[si]
    cmp dl, 0 
    je EOP
    mov ah, 2 
    int 21h 
    inc si 
    jmp print_loop
 
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
mov ax, 0
mov al, i
mov si, ax

mov al, s[bx]
ulp:  
cmp si, bx
je return
mov ah, s[si]
mov s[si], al
mov al, ah
inc si   
jmp ulp

return:
ret       