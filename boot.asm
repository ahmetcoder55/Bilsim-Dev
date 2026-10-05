;BILSIM OS & KURUCUK OS
[bits 16]
[org 0x7c00]

KERNEL_OFFSET equ 0x1000

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00          
    mov bp, sp

    ; Video Modu: 80x25 Renkli Metin
    mov ax, 0x0003
    int 0x10

    ; Ekrana "KU" bas
    mov ah, 0x0e
    mov al, 'K'
    int 0x10
    mov al, 'U'
    int 0x10

    ; Diski Resetle
    xor ax, ax
    xor dx, dx              ; dl = 0x00 (Floppy / fda)
    int 0x13

    ; Kernel'ı 0x0000:0x1000 adresine oku
    mov ax, 0x0000
    mov es, ax
    mov bx, KERNEL_OFFSET
    
    mov ah, 0x02            
    mov al, 40              ; 40 sektör (Yaklaşık 20KB) Real Mode için yeterli ve güvenli
    mov ch, 0x00            
    mov dh, 0x00            
    mov cl, 0x02            
    xor dx, dx              ; dl = 0x00
    int 0x13
    
    jnc .launch             
    
    ; Disk hatası varsa kırmızı 'E' bas
    mov ax, 0x0e45
    int 0x10
    jmp $

.launch:
    ; Segmentleri sıfırla ve kesin uzak sıçrama yap
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    jmp 0x0000:KERNEL_OFFSET

times 510-($-$$) db 0
dw 0xAA55
