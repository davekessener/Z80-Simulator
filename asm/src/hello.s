#define	PORT_SCREEN	16
#define PORT_SCREEN_CX 17
#define PORT_SCREEN_CY 18
#define PORT_SCREEN_STATUS 19

main:
	call ClearScreen
    ld hl,msg
    call PrintString
    halt
    
msg:
.db	"Hello, World!",0
    
ClearScreen:
	ld a,$80
    out (PORT_SCREEN_STATUS),a
    ret
    
PrintString:
	ld a,(hl)
    or a
    ret z
    inc hl
    out (PORT_SCREEN),a
    jr PrintString


