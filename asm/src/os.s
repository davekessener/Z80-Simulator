#define PORT_STATUS				0
#define PORT_STATUS_INT			PORT_STATUS+1

#define PORT_SCREEN				16
#define PORT_SCREEN_CX			PORT_SCREEN+1
#define PORT_SCREEN_CY 			PORT_SCREEN+2
#define PORT_SCREEN_STATUS		PORT_SCREEN+3

#define PORT_KEYBOARD_STATUS	32
#define PORT_KEYBOARD			PORT_KEYBOARD_STATUS+1
#define PORT_KEYBOARD_MOD		PORT_KEYBOARD_STATUS+2

#define SCREEN_F_CURSOR_ENABLE	$01
#define SCREEN_F_ADVANCE_X		$02
#define SCREEN_F_WRAP_X			$04
#define SCREEN_F_ADVANCE_Y		$08
#define SCREEN_F_SCROLL_ENABLE	$10
#define SCREEN_F_INVERT			$20
#define SCREEN_F_SCREEN_ENABLE	$40
#define SCREEN_F_COMMAND		$80

#define SCREEN_CMD_CLEAR		$00
#define SCREEN_CMD_SCROLL		$01
#define SCREEN_CMD_EN_60HZ_INT	$02
#define SCREEN_CMD_DE_60HZ_INT	$03

#define SCREEN_DEFAULT_OPTIONS	SCREEN_F_CURSOR_ENABLE|SCREEN_F_ADVANCE_X|SCREEN_F_WRAP_X|SCREEN_F_ADVANCE_Y|SCREEN_F_SCROLL_ENABLE|SCREEN_F_SCREEN_ENABLE

#define KEYBOARD_MODE_POLL		$00
#define KEYBOARD_MODE_RAW		$01
#define KEYBOARD_MODE_TEXT		$02

#define ISR_SCREEN_ID			$01
#define ISR_KEYBOARD_ID			$02

#define KEY_BACKSPACE			$08
#define KEY_RETURN				$0A

; # -------------------------------------------------------------------------------------------------------------------

Init:
	di
	ld sp,$0000
    im 1
    call Screen_Init
    call Keyboard_Init
    ei
    call main
Init_loop:
	halt
    jr Init_loop

; # -------------------------------------------------------------------------------------------------------------------

ISR_Table:
.db                $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
.db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
.db $00, $00, $00, $00, $00, $00, $00, $00


ISR:
	di
    push af
    push hl
    push de
    push bc
    ld hl,ISR_Table
    in a,(PORT_STATUS_INT)
    add a,a
    add a,l
	ld l,a
    ld e,(hl)
    inc hl
    ld d,(hl)
    ex de,hl
    ld a,l
    or h
    call nz,ISR_valid
    pop bc
    pop de
    pop hl
    pop af
    ei
    ret
ISR_valid:
	jp (hl)
    
    
ISR_Register:
	push de
	ld de,ISR_Table
    ex de,hl
    add a,a
    add a,l
    ld l,a
    ld (hl),e
    inc hl
    ld (hl),d
    pop de
    ret

; # -------------------------------------------------------------------------------------------------------------------

main:
	call Screen_Clear
	ld hl,prompt
    call Screen_PrintString
    call Keyboard_ReadLine
    call Screen_PrintString
    ret
    
prompt:
.db "$> ",0
  
; # -------------------------------------------------------------------------------------------------------------------


Screen_Init:
    ld a,SCREEN_DEFAULT_OPTIONS
    jp Screen_SetOptions

Screen_Clear:
	ld a,SCREEN_CMD_CLEAR
    jp Screen_Command

Screen_SetOptions:
	and $7F
    out (PORT_SCREEN_STATUS),a
    ret

Screen_Command:
	or $80
    out (PORT_SCREEN_STATUS),a
    ret

Screen_SetCursor:
	ld a,b
    out (PORT_SCREEN_CX),a
    ld a,c
    out (PORT_SCREEN_CY),a
    ret

Screen_GetCursor:
	in a,(PORT_SCREEN_CX)
    ld b,a
    in a,(PORT_SCREEN_CY)
    ld c,a
    ret

Screen_PrintChar:
	out (PORT_SCREEN),a
    ret
    
Screen_PrintString:
	push hl
Screen_PrintString_loop:
    ld a,(hl)
    or a
    jr z,Screen_PrintString_end
    out (PORT_SCREEN),a
    inc hl
    jr Screen_PrintString_loop
Screen_PrintString_end:
	pop hl
    ret
    
; # -------------------------------------------------------------------------------------------------------------------

Keyboard_Init:
    ld a,KEYBOARD_MODE_POLL
    out (PORT_KEYBOARD_STATUS),a
    ld de,KB_Buffer
    ld (KB_Idx),de
	ld a,ISR_KEYBOARD_ID
	ld hl,ISR_Keyboard
    call ISR_Register
    ret
    
Keyboard_ReadLine:
	ld hl,KB_Mode
    ld (hl),1
    ld a,KEYBOARD_MODE_TEXT
    out (PORT_KEYBOARD_STATUS),a
Keyboard_ReadLine_wait:
	halt
    ld a,(hl)
    cp 2
    jr nz,Keyboard_ReadLine_wait
    ld (hl),0
    ld hl,KB_Buffer
    ret

ISR_Keyboard:
	ld a,(KB_Mode)
    or a
    ret z
    dec a
    ret nz
    ld de,(KB_Idx)
	in a,(PORT_KEYBOARD_STATUS)
    ld b,a
    inc b
ISR_Keyboard_loop:
	dec b
    ret z
    in a,(PORT_KEYBOARD)
    ld c,a
    call KB_IsValidChar
    ld a,c
    jr z,ISR_Keyboard_print
    cp KEY_BACKSPACE
    jr z,ISR_Keyboard_backspace
    cp KEY_RETURN
    jr z,ISR_Keyboard_nl
    jr ISR_Keyboard_loop
ISR_Keyboard_backspace:
	ld a,e
    cp KB_Buffer
    jr nz,ISR_Keyboard_bs_do
    ld a,d
    cp KB_Buffer/$100
    jr z,ISR_Keyboard_loop
ISR_Keyboard_bs_do:
    ld hl,KB_Del
    call Screen_PrintString
    dec de
    ld (KB_Idx),de
    jr ISR_Keyboard_loop
ISR_Keyboard_print:
	out (PORT_SCREEN),a
    ld (de),a
    inc de
    ld (KB_Idx),de
    jr ISR_Keyboard_loop
ISR_Keyboard_nl:
	out (PORT_SCREEN),a
	xor a
	ld (de),a
    ld hl,KB_Mode
    inc (hl)
    ld a,KEYBOARD_MODE_POLL
    out (PORT_KEYBOARD_STATUS),a
    ret
    

KB_IsValidChar:
	cp ' '
    jr c,KB_IsValidChar_no
    cp $80
    jr nc,KB_IsValidChar_no
    xor a
    ret
KB_IsValidChar_no:
	xor a
    inc a
    ret

KB_Mode:
.db $00

KB_Del:
.db KEY_BACKSPACE, ' ', KEY_BACKSPACE, 0

KB_Idx:
.db $00, $00

KB_Buffer:
.db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
.db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
.db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
.db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
