BITS 16

section .text
main:
    mov ah, 0x0e ; tty mode
    je main

; Boot Sector 
; times 510 - ($-$$) db 0
; dw 0xaa55 

; section .data
; Success: 
;     db 'Hello, world', 0 ; Allocating space for our array of bytes (chars) and adding a null byte at the end
; Test:
;     dw 0xdead
;     db 0x0
