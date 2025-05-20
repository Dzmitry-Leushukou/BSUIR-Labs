.model small
.stack 100h
.data
;vars
char db 0 
need_size dw 0 
size dw 0 
ans dw 0                     
;file vars
filename db 256 dup(0) 
fsize dw 0
;filename db 'D:\test.txt', 0 
file_id dw 0
;flags
BadFileFlag db 0   
;UI
endl_str db 0Dh, 0Ah, '$'                                               
BadFileMsg db "Bad file",0Dh, 0Ah, '$' 
AnsMsg db "Lines with size less than given: ", '$'  
Wrong_number db "Wrong number format",0Dh, 0Ah, '$'
OpenFileMsg db  "File opened successfully",0Dh, 0Ah, '$'  
CloseFileMsg db "File closed",0Dh, 0Ah, '$'  
.code    
;Include data & stack
mov ax, @data
mov ds, ax

main:
call GetFileName
call OpenFile 
call FileOpenMessage
call ReadNumber 
call ReadFile
call CloseFile
call FileCloseMessage  
call ShowAns
EOP:
mov ax, 4C00h
int 21h  
;CMD LINE PROC
GetFileName proc uses ax cx    
    ; get size of filename
    mov cx, 0
    mov cl, ES:[80h]
    mov fsize, cx
    
    cld
    mov di, 81h
    mov al, ' '
    rep scasb ;find ' ' in cmd to define the end of path file
    dec di ;show to char after ' '
    lea si, filename

copy:
    mov al, ES:[DI] ;ES:[DI] - cmd
    cmp al, 0Dh 
    je endCopy
    cmp al, 20h 
    je endCopy
    cmp al, 9h 
    je endCopy
    
    mov DS:[SI], al
    inc di
    inc si
    jmp copy
    
endCopy:
    ret
ENDP
;READ PROCEDURES
ReadNumber proc
    
    rn_loop:
    ; based settings
    call ReadChar
    cmp char, 13
    je return    
    
    mov bx, need_size
    mov ax, 10
    mul bx
    jc WrongNumber
    
    mov need_size, ax
    mov ax, 0
    mov al, char
    sub ax, '0'
     
    cmp ax, 0
    jl WrongNumber  
    cmp ax, 10
    jge WrongNumber  
    
    add need_size, ax
    jc WrongNumber  
    jmp rn_loop
    ret
endp 

ReadFile proc
mov si, need_size

    rf_loop:
    mov size, 0
    inc ans
    line_loop:
    call ReadChar
    
    cmp ax, 0
    je return
    
    cmp char, 9
    je line_loop
    
    cmp char, 10
    je line_loop
    
    cmp char, 13
    je rf_loop
    
    inc size
    
    cmp size, si
    jge good_line
       
    jmp line_loop
    
    good_line: 
    dec ans
    gl_loop:
    call ReadChar
    cmp char, 13
    je rf_loop
    
    cmp ax, 0
    je return
    jmp gl_loop
       
    ret
endp

ReadChar proc
    mov ah, 3fh
    mov bx, file_id
    mov cx, 1
    lea dx, char
    int 21h
    ret
endp    

;FILE PROCEDURES
OpenFile proc
mov ah, 3dh
mov al, 0
lea dx, filename
int 21h         
jc WrongNumber
mov file_id, ax
ret
ENDP 
 
CloseFile proc
mov ah, 3eh
mov bx, file_id   
int 21h
ret 
ENDP 
;ERROR
WrongNumber:
    call WrongNumberError
    jmp EOP

;MESSAGES   
endl proc
mov ah, 9  
lea dx, endl_str
int 21h 
ret      
ENDP

FileOpenMessage proc
mov ah, 9  
lea dx, OpenFileMsg
int 21h 
ret      
ENDP
     
FileCloseMessage proc
mov ah, 9  
lea dx, CloseFileMsg
int 21h 
ret      
ENDP
     
BadFileError proc 
mov ah, 9  
lea dx, BadFilemsg
int 21h 
ret
ENDP  

WrongNumberError proc
mov ah, 9  
lea dx, Wrong_number
int 21h 
ret
ENDP 

ShowAns proc
mov ah, 9  
lea dx, AnsMsg
int 21h 
  
mov dx, 0
mov ax, ans
mov bx, 10000
div bx
mov cx,dx

add ax, '0'
mov dl, al
mov ax, 0
mov ah, 02h
int 21h


mov ax, cx   
mov dx, 0
mov bx, 1000
div bx
mov cx,dx

add ax, '0'
mov dl, al
mov ax, 0
mov ah, 02h
int 21h

mov ax, cx
mov dx, 0
mov bx, 100
div bx
mov cx,dx
add ax, '0'
mov dl, al
mov ax, 0
mov ah, 02h
int 21h

mov ax, cx
mov dx, 0
mov bx, 10
div bx
mov cx,dx
add ax, '0'
mov dl, al
mov ax, 0
mov ah, 02h
int 21h

mov ax, cx
add ax, '0'
mov dl, al
mov ax, 0
mov ah, 02h
int 21h

ret 
ENDP 


;RETURN
return:
ret