BITS 16

mov bx, HELLO
; add bx, 0x7c00

main:
    mov al, [bx]
    cmp al, 0
    je done
    mov ah, 0x0e ; tty mode
    int 0x10
    inc bx
    
    jmp main

; Boot Sector
; times 510-($-$$) db 0
; dw 0xaa55

HELLO:
    db 'Hello, World!', 0
done:
     