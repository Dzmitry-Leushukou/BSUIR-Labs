[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

main_loop:
    ; Вывод в реальном режиме
    mov si, msg_real
    call print_string_16

    ; Ожидание клавиши
    mov ah, 0x00
    int 0x16

    ; Проверка на 'q' для выхода
    cmp al, 'q'
    je graceful_exit
    cmp al, 'Q'
    je graceful_exit

    ; Переход в защищенный режим
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG_32:protected_mode

graceful_exit:
    ; Корректное завершение программы
    mov si, msg_exit
    call print_string_16
    
    ; Завершение через BIOS
    mov ax, 0x4C00
    int 0x21
    ; Или просто вечный цикл
    jmp $

print_string_16:
    mov ah, 0x0E
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    ret

[BITS 32]
protected_mode:
    ; Инициализация сегментов
    mov ax, DATA_SEG_32
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000

    ; Очистка экрана и вывод
    mov edi, 0xB8000
    mov ecx, 80*25
    mov ax, 0x0720
    rep stosw

    mov edi, 0xB8000
    mov al, 'P'
    mov ah, 0x1F
    mov [edi], ax

    ; Задержка
    mov ecx, 0xFFFFFF
.delay:
    nop
    loop .delay

    ; Возврат в реальный режим
    mov ax, DATA_SEG_16
    mov ds, ax
    mov es, ax
    mov ss, ax
    jmp CODE_SEG_16:back_to_real

[BITS 16]
back_to_real:
    mov eax, cr0
    and eax, 0x7FFFFFFE
    mov cr0, eax
    jmp 0x0000:real_again

real_again:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Возврат в главный цикл вместо завершения
    jmp main_loop

; Данные
msg_real db "Real Mode. Press any key for Protected Mode, 'Q' to quit.", 0x0D, 0x0A, 0
msg_exit db "Program terminated. You can safely close QEMU.", 0x0D, 0x0A, 0

; GDT (остается без изменений)
gdt_start:
    dq 0
gdt_code_32:
    dw 0xFFFF, 0x0000
    db 0x00, 0x9A, 0xCF, 0x00
gdt_data_32:
    dw 0xFFFF, 0x0000
    db 0x00, 0x92, 0xCF, 0x00
gdt_code_16:
    dw 0xFFFF, 0x0000
    db 0x00, 0x9A, 0x0F, 0x00
gdt_data_16:
    dw 0xFFFF, 0x0000
    db 0x00, 0x92, 0x0F, 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG_32 equ gdt_code_32 - gdt_start
DATA_SEG_32 equ gdt_data_32 - gdt_start
CODE_SEG_16 equ gdt_code_16 - gdt_start
DATA_SEG_16 equ gdt_data_16 - gdt_start

times 510 - ($ - $$) db 0
dw 0xAA55
