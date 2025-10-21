; boot.asm — 16-bit boot sector: LBA load → Protected Mode
bits 16
org 0x7C00

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov [BootDrive], dl

    ; Enable A20
    in   al, 0x92
    or   al, 0000_0010b
    out  0x92, al

%include "kernel_sectors.inc"

    ; ---------- Load kernel (INT 13h extensions, LBA) to 0x0010_0000 ----------
    mov dl, [BootDrive]
    mov ax, 0x1000
    mov es, ax
    xor bx, bx

    mov ax, KERNEL_SECTORS
    mov [DAP+2], ax
    mov word [DAP+4], 0x0000
    mov word [DAP+6], 0x1000
    mov dword [DAP+8], 1
    mov dword [DAP+12], 0

    mov si, DAP
    mov ah, 0x42
    int 0x13
    jc  disk_error

    ; ---------- Enter Protected Mode ----------
    lgdt [gdt_desc]
    cli
    mov eax, cr0
    or  eax, 1
    mov cr0, eax
    jmp dword 0x08:0x0010000        ; IMPORTANT: 32-bit far jump

disk_error:
    mov si, msgDE
    call print
.hang:
    hlt
    jmp .hang

print:
    pusha
    mov bx, 0xB800
    mov es, bx
    mov di, 0
.next:
    lodsb
    or al, al
    jz .done
    stosb
    mov al, 0x4F
    stosb
    jmp .next
.done:
    popa
    ret

msgDE db 'DE',0

; ---------- Data & GDT ----------
align 4
BootDrive db 0

DAP:
    db 16
    db 0
    dw 0
    dw 0
    dw 0
    dd 0
    dd 0

align 8
gdt_start:
    dq 0x0000000000000000
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510-($-$$) db 0
dw 0xAA55
