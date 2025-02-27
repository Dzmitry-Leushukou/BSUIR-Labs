 .model small
 .stack 100h ; stack 256 bytes 
 
 .data  
msg db "String",0Dh,0Ah,'$' 

 .code 
  mov ax,@data ; include .data & .stack
  mov ds, ax   ; data segment = adress of data
  mov ah,9     ; cout
  mov dx, offset msg
  int 21h      ; interrupt
  mov ax, 4C00h ; 4C - EOP, 00H - return code
  int 21h
  
  
