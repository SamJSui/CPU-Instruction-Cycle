BITS 16

mov bx, TEST
; ; add bx, 0x7c00

; mov ah, 0x0e
; mov al, [bx]
; int 0x10

main:
    mov al, [bx]
    cmp al, 0
    je done
    mov ah, 0x0e ; tty mode
    int 0x10
    inc bx
    
    jmp main

; ; Boot Sector 

TEST:
    db 'B', 0
done:
     