 .model small
.stack 100h
.data 
    prog_name db 'opera.exe', 0
.code
mov ah, 4Bh
lea dx, prog_name
int 21h 