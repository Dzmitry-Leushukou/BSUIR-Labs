.model tiny
.code
org 100h  

 mov ah,9; 9 - number of function (cout<<string)
 mov dx, offset msg ; dx - register for data transmision, offset give adress
 int 21h ; interruption for cin/cout/work with memory
 ret ;return
   
msg db "String",0Dh,0Ah,'$' ; db - define byte, ODh - beginnig of current line, OAh - new line, $ - EOL