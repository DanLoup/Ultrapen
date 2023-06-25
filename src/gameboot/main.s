.include "header.i"
.include "setup.i"

boot:
di
;;ld sp,$bf00
ld sp,$8400
ld a,($7) ;MSX Video port
ld (videoport),a
call SetupMemory
call setupVideoMode

ld hl,filename
call LoadFile
	
	ld hl,$8400
	ld de,$0000
	ld bc,$5000
	ldir
	ld a,(videoport)
	ld ($f0),a
jp $100
filename: .db "UPEN    BIN"
graphicmode: .db $04,$80,$02,$81,$00,$87,$08,$88,$80,$89,$06,$82,$ff,$83,$03,$84
setupVideoMode:
	ld a,(videoport)
	inc a
	ld c,a
	ld b,$12
	ld hl,graphicmode
	otir
	ld a,($ffe7)
	or 2
	out (c),a
	ld a,$88 ;Attempts to setup the video memory to have sprites enabled 
	ld hl,$4000
	out (c),l
	out (c),h
	ld hl,$ffff
	dec c
	ld a,0
	vidclean:
	out (c),a
	dec l
	jp nz,vidclean
	dec h
	jp nz,vidclean
ret

.include "file.i"
.include "mem.i"


