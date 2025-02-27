.model small
.stack 100h
 .data 
; Procedure vars
procedure_name db 255 dup (0) 
kol dw 0   
shift dw 0
EPB dw 0000 
 dw offset commandline,0 
 dw 005Ch,0,006Ch,0 
commandline db 125 
            db " /?"    
; Number of line vars
c db 0
numb dw 0
; Filename vars
filename db 255 dup (0)   
; Cmd line vars
line db 255 dup (0) 
line_size dw 0  
; File vars
file_id dw 0
char db 0
EOF db 0 
need_line db 0
; UI
endl_str db 0Dh, 0Ah, '$'    
BadFileNameMsg db "Bad filename", 0Dh, 0Ah, '$' 
OverflowMsg db "Overflow", 0Dh, 0Ah, '$'  
BadFileMsg db "Bad file", 0Dh, 0Ah, '$' 
WrongNumberMsg db "Wrong number", 0Dh, 0Ah, '$'                                               
CLPMsg db "Command line parameters: $"  
PNMsg db "Process name: $"  
BadProcedureNameMsg db "Bad procedure name", 0Dh, 0Ah, '$'                                               
BadProcess db "Bad process", 0Dh, 0Ah, '$'
; length
dlen dw $-main
plen dw 800  
.code
main:  
mov ax, @data
mov ds, ax     
call GetParam
call ShowString
call GetFileName
call GetNumber
call GetKol 
call FileOpen
call GetProcessName
call ShowProcessName
call FileClose
call RunProcessLoop      
EOP:         
mov ax, 4C00h
int 21h    
;Process procedures
RunProcessLoop proc
    mov cx, kol  
    
    lrpl:
    cmp kol, 0
    je EOP
   
    mov sp, plen + 100h + 200h            
    mov ah, 4ah   
    mov bx, (plen/16)+256/16+(dlen/16)+20
    int 21h           
    
    mov ax,cs
    mov word ptr EPB+02h, ax   
    mov word ptr EPB+06h, ax   
    mov word ptr EPB+0Ah, ax
    
    mov ax,4B00h
    mov dx,offset procedure_name
    mov bx,offset EPB
    int 21h   
   
    cmp al, 0
    je iter  
    mov ah, 02h 
    add al, '0'
    mov dl, al
    int 21h
    call ShowBadProcess
    
    iter:
    dec kol
    jmp lrpl
    ret
ENDP
ShowProcessName proc uses si ax dl
    mov si, 0 
    call pn
    lpspn:
    mov ax, 0
    mov ah, 02h
    mov dl, procedure_name[si]
    cmp dl, 0
    je retspn
    inc si
    int 21h
    jmp lpspn
    retspn: 
    call endl
    
    ret
ENDP

GetProcessName proc uses cx
    mov cx, numb
    dec cx
    lp:
    cmp cx, 0
    je last_read
    mov numb, cx
    call ReadLine
    mov cx, numb
    dec cx
    jmp lp       
    
    last_read:
    mov need_line, 1
    call ReadLine
    ret
ENDP

;File procedures  
ReadLine proc uses si ax cx bx dx
    mov si, 0
    lrl:
    
    mov ax, 0
    mov ah, 3fh
    mov bx, file_id
    mov cx, 1  
    lea dx, char
    int 21h
    
    cmp ax, 1
    je good
    mov EOF, 1
    jmp rlr
    good:  
    cmp char, 13
    je rlr
    
    cmp need_line, 0
    je lrl
    cmp si, 254
    je BadProcedureName   
    mov al, char
    mov procedure_name[si], al
    inc si
    jmp lrl 
    rlr:  
    mov ax, 0
    mov ah, 3fh
    mov bx, file_id
    mov cx, 1  
    lea dx, char
    int 21h
    ret
ENDP     

BadProcedureName:
call ShowBadProcedureName
jmp EOP

FileClose proc
    mov al, 0
    mov ah, 3eh
    mov bx, file_id
    ret     
ENDP  

FileOpen proc
    mov al, 0
    mov ah, 3dh
    lea dx, filename
    int 21h
    jc BadFile
    mov file_id, ax
    ret     
ENDP  

BadFile:
call ShowBadFile
jmp EOP
;Line process procedures   
GetKol proc uses bx
    mov bx, 0
    lgk:         
    mov al, line[si]
    
    cmp al, 0
    je EGK  
    
    sub al, '0'
    
    cmp al, 0
    jl WrongNumber
    cmp al, 9
    jg WrongNumber
    cmp si, line_size
    jge WrongNumber
    
    mov c, al  
    mov ax, 10
    mul bx
    jc Overflow
    mov bx, ax 
    mov ax, 0
    mov al, c
    add bx, ax
    jc Overflow
    
    inc si
    jmp lgk
    EGK:
    mov kol, bx
     
    ret
ENDP


GetNumber proc uses bx
    mov bx, 0
    lgn:         
    mov al, line[si]
    
    cmp al, 0
    je WrongNumber  
                  
    cmp al, 32
    je EGN
    sub al, '0'
    
    cmp al, 0
    jl WrongNumber
    cmp al, 9
    jg WrongNumber
    cmp si, line_size
    jge WrongNumber
    
    mov c, al  
    mov ax, 10
    mul bx
    jc Overflow
    mov bx, ax 
    mov ax, 0
    mov al, c
    add bx, ax
    jc Overflow
    
    inc si
    jmp lgn
    EGN:
    mov numb, bx
    inc si
    ret
ENDP


WrongNumber:
call ShowWrongNumber
jmp EOP

Overflow:
call ShowOverflow
jmp EOP

GetFileName proc
    mov si, 0
    lgfn:         
    mov al, line[si]
    
    cmp al, ' '
    je EGFN
    cmp al, 0
    je BadFileName  
    cmp si, line_size
    jge BadFileName
    
    mov filename[si], al
    inc si
    jmp lgfn
    EGFN:    
    inc si
    ret
ENDP

BadFileName:
call ShowBadFileName
jmp EOP
;CMD procedures               
GetParam proc uses si cx ax
    ; get size of filename 
    mov si, 0
    mov cx, 0
    mov cl, ES:[80h]
    dec cl
    mov line_size,cx
    ;get line
    lgp:    
    mov ax, ES:[si+81h] 
    cmp si, line_size
    jge EGP   
    cmp ax, 0
    je EGP
    mov line[si], ah
    dec cx
    inc si
    jmp lgp
    
    EGP:
    mov line_size,si
    ret
ENDP        

ShowString proc uses si ax dl
    mov si, 0 
    call clp
    lpss:
    mov ax, 0
    mov ah, 02h
    mov dl, line[si]
    cmp si, line_size
    jge retss
    inc si
    int 21h
    jmp lpss
    retss: 
    call endl
    ret
ENDP   
; UI

clp proc
mov ah, 9  
lea dx, CLPMsg
int 21h       
ret      
ENDP 

endl proc
mov ah, 9  
lea dx, endl_str
int 21h 
ret      
ENDP           

ShowBadFileName proc
mov ah, 9  
lea dx, BadFileNameMsg
int 21h 
ret      
ENDP

ShowOverflow proc
mov ah, 9  
lea dx, OverflowMsg
int 21h 
ret      
ENDP

ShowWrongNumber proc
mov ah, 9  
lea dx, WrongNumberMsg
int 21h 
ret      
ENDP 

ShowBadFile proc
mov ah, 9  
lea dx, BadFileMsg
int 21h 
ret      
ENDP 

ShowBadProcedureName proc
mov ah, 9  
lea dx, BadProcedureNameMsg
int 21h 
ret      
ENDP     

pn proc
mov ah, 9  
lea dx, PNMsg
int 21h       
ret      
ENDP

ShowBadProcess proc
mov ah, 9  
lea dx, BadProcess
int 21h       
ret      
ENDP  
mov plen, $-main
end main      
                                           