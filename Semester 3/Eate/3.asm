.model small
.stack 100h
ends

.data 
    message1 db  13, "Write a line: $", 0
    firstString db 255, ?, 255 dup('$')
    endl db 0Dh, 0Ah, '$'
    substring db 255, ?, 255 dup('$'), 0
    message2 db 10, "Write a substring: $", 0
    no_msg db 0Dh, 0Ah, 10, "No such substring $", 0
    message3 db 10, "Write new substring: $", 0
    newSubstring db 255, ?, 255 dup('$'), 0

.code
main:
 mov ax, @data
 mov ds, ax
 mov es, ax
 
 mov dx, offset message1    ;outputs a string message1
 mov ah, 09h
 int 21h 
 
 lea dx, firstString
 mov ah, 0Ah
 int 21h              ; entering your own string in firstString  
 
 lea dx, endl
 mov ah, 09h
 int 21h
 
 lea dx, firstString + 2  
 mov ah, 09h
 int 21h   
 
 mov dx, offset message2    ;outputs a string message2
 mov ah, 09h
 int 21h  
 
 lea dx, substring
 mov ah, 0Ah
 int 21h
 
 check_substring:
    lea si, firstString + 2      ;Set si, di
    lea di, substring + 2
    mov cl, [substring + 1]
    mov al, [firstString + 1]
    cmp al, cl              ;Check sizes
    jb not_found
    mov ch, 0
    
 looking_for_found: 
    push si
    push di
    push cx
    
    repe cmpsb
    jz found0 
    
    pop cx
    pop di
    pop si
    
    inc si
    dec al
    cmp al, cl
    jb not_found
    jmp looking_for_found
    
found0: 
    mov dx, offset message3    ;outputs a string message3
    mov ah, 09h
    int 21h  
 
    lea dx, newSubstring
    mov ah, 0Ah
    int 21h
    
    
    mov al, [newSubstring + 1]
    
    pop cx
    pop di
    pop si  
    lea di, newSubstring + 2
found:
    mov bl, [di]
mov byte ptr [si], bl

    
    inc si
    inc di
    mov bl, [di]
   
    cmp bl, 0Dh
    je Answer
    
    ;bla-bla
    jmp found
    
    
    ;size string > size substring
    ;size string < size substring => >256=> end
    
    
    
 Answer: 
 
    lea dx, endl
    mov ah, 09h
    int 21h  
    
    lea dx, firstString + 2  
 mov ah, 09h
 int 21h
    
    jmp Done   
     
    
 not_found:
    lea dx, no_msg
    mov ah, 09h
    int 21h
    jmp Done   
 
Done: 
 mov ax, 4C00h
 int 21h
end main