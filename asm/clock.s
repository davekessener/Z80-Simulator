#define	PORT_SCREEN	16
#define PORT_SCREEN_CX 17
#define PORT_SCREEN_CY 18
#define PORT_SCREEN_STATUS 19

; # ---------------------------------------------------------------------------------------------

	jp Init
    
.db                $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
.db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
.db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
.db $00, $00, $00, $00, $00, $00, $00, $00

; # ---------------------------------------------------------------------------------------------

ISR:
	di
    inc bc
    ret


; # ---------------------------------------------------------------------------------------------


clock_sec:
.db $00

clock_min:
.db $00

; # ---------------------------------------------------------------------------------------------


Init:
    di
    im 1
	ld sp,$0000
	ld a,$5F
    out (PORT_SCREEN_STATUS),a
    ld a,$82
    out (PORT_SCREEN_STATUS),a
    call main
End:
    halt
    jr End

; # ---------------------------------------------------------------------------------------------

main:
	call ClearScreen
	xor a
    ld (clock_sec),a
    ld (clock_min),a
    ld c,0
loop:
	xor a
    out (PORT_SCREEN_CX),a
    call Update
    call Display
    ei
    halt
    jr loop

; # ---------------------------------------------------------------------------------------------

Update:
	ld a,c
    cp 60
    ret c
    sub 60
    ld c,a
    ld hl,clock_sec
    ld a,(hl)
    inc a
    daa
    ld (hl),a
    cp $60
    ret c
    sub $60
    ld (hl),a
    ld hl,clock_min
    ld a,(hl)
    inc a
    daa
    ld (hl),a
    cp $60
    ret c
    sub $60
    ld (hl), a
    ret

; # ---------------------------------------------------------------------------------------------

Display:
	ld a,(clock_min)
    add a,0
    daa
    call PrintBC_digit
    ld a,':'
    out (PORT_SCREEN),a
    ld a,(clock_sec)
    add a,0
    daa
    call PrintBC_digit
    ret

; # ---------------------------------------------------------------------------------------------

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

PrintBC:
	ld a,b
    call PrintBC_digit
    ld a,c
    jp PrintBC_digit
PrintBC_digit:
	ld d,a
    rrca
    rrca
    rrca
    rrca
    and $0F
    call PrintBC_nibble
    ld a,d
    and $0F
    jp PrintBC_nibble
PrintBC_nibble:
	cp $0A
    jr c,PrintBC_nibble_09
    add a,'A'-10-'0'
PrintBC_nibble_09:
	add a,'0'
    out (PORT_SCREEN),a
    ret

