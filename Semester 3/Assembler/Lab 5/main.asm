    .model small
.stack 100h
.data
Filename db 256 dup(0)   
fsize dw 0 
;flags
WNF db 0
BadFile db 0   
;need size
n dw 0
;for fileread   
char db 0    
len dw 0
need_len dw 0
;UI
EnterFilename db "Enter path to your file (max_size 256):",0Dh, 0Ah, '$'
endl_str db 0Dh, 0Ah, '$'                                               
BadFileMsg db "Bad file",0Dh, 0Ah, '$'  
Wrong_number db "Wrong number format",0Dh, 0Ah, '$'  
.code    
;Include data & stack
mov ax, @data
mov ds, ax

main:
mov BadFile, 0
mov WNF, 0
clc ; set 0 in flag C 
call Filename_read; fsize stored in si, Filename was filling
mov fsize, si ; all registers are free

call Filename_check  
cmp BadFile, 1 ; Bad file 
je main       

;Open file mode activated
call FileRead ;AX - file descriptor     
call FileClose
cmp Wrong_number, 1
jmp main

EOP:
mov ax, 4C00h
int 21h  

Filename_read proc 
call cin_filename
mov si, 0
cin_fn_loop:
cmp si, 255; size >= 255 (Last char always dollar)
jae return
;cin char
mov al, 0
mov ah, 1 
int 21h

cmp al, 13 ;cmp enter char and cined char
je return

mov Filename[si], al ; s[si]=read char
inc si; si++
jmp cin_fn_loop   
ret 
ENDP
     
Filename_check proc 
; open file  
mov al, 0
mov ah, 3Dh
lea dx, Filename
int 21h    

jc bf ;doesn`t exist
ret 
  
bf: ;error bad file
mov BadFile, 1
call BadFileError
ret
ENDP
     
     
endl proc
mov ah, 9  
lea dx, endl_str
int 21h 
ret      
ENDP

cin_filename proc
mov ah, 9  
lea dx, EnterFilename
int 21h 
ret
ENDP

BadFileError proc 
call endl
mov ah, 9  
lea dx, BadFilemsg
int 21h 
ret
ENDP
  
FileRead proc   
mov bx, ax ; copy descriptor from check proc
mov cx, 1 ; read 1 byte
lea dx, char ; link to char for cin 
mov si, 0 ; local ind
call ReadNumber ; first word - number
read: 
mov ah, 3fh; read operation from open file
mov al, 0
int 21h ; read
cmp ax, 0;EOF
je return      

jmp read
ret
ENDP
  
ReadNumber proc
mov ah, 3fh; read operation from open file
mov al, 0 
mov cx, 1
int 21h ; read
cmp ax, 0;EOF
je return 
cmp char, 13 ; enter
jmp return

sub char, '0'
cmp char, 0
jl wrong_number_format
cmp char, 10
jge wrong_number_format
                
; zaebis` format
mov cx, ax
mul ax, 10
mov n, cx
jmp ReadNumber

wrong_number_format:
call wner; message 
mov WNF, 0
ret
ENDP  

wner proc
call endl
mov ah, 9  
lea dx, Wrong_number
int 21h 
ret
ENDP  
  
FileClose proc
mov ax, 003Eh
int 21h
ret
ENDP
  
  
  
return:
ret