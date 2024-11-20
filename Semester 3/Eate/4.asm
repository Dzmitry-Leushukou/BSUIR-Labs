.model small
.stack 100h
.data
number1 db 256 dup ('$')

message1 db "Enter first number: $"
message2 db "Enter second number: $" 
endl db 0Dh, 0Ah, '$'
messageWarning db "You wrote not a number! $"
number dw 0 
over db "Overflow in number$"
.code

mov ax, @data
mov ds, ax

lea dx, message1
mov ah, 09h
int 21h

mov dx,  offset number1
mov ah, 0Ah
int 21h 

lea dx, endl
mov ah, 09h
int 21h

translate:
mov si, 2
 
walking:
mov ah, 0
mov al, number1[si] 
inc si
cmp ax, 0
je Done

cmp ax, 47
jbe Ending

cmp ax, 58                                           ;number = number*16 + (ch -'0')
jb processing





processing:
sub ax, 48 
push ax
mov ax, 16
mul number     
jc Overflow
mov number, ax
pop ax 
;mov ah, 0
add number, ax
jc Overflow
jmp walking



Overflow:
lea dx, over
mov ah, 09h
int 21h

Ending:
 lea dx, messageWarning
 mov ah, 09h
 int 21h

Done:
mov ax, 4C00h
int 21h