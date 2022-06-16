BITS 16

section .text
mov di, 0xdead ; tty mode

; Boot Sector 
; times 510 - ($-$$) db 0
; dw 0xaa55 

; section .data
; Success: 
;     db 'Hello, world', 0 ; Allocating space for our array of bytes (chars) and adding a null byte at the end