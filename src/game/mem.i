vdpmem: .db 0
vdpreg: .db 0
s_fotir:
	push bc
	ld a,b
	and $f
	ld b,a
	otir
	pop bc
	ld a,b
	and $f0
	ret z
	ld b,a

fotir:
ld a,0
fotloop:
outi
outi
outi
outi
outi
outi
outi
outi
outi
outi
outi
outi
outi
outi
outi
outi
cp b
jr c,fotloop
ret

fldir:
ld a,0
fldloop:
ldi
ldi
ldi
ldi
ldi
ldi
ldi
ldi
ldi
ldi
ldi
ldi
ldi
ldi
ldi
ldi
cp c
jp c,fldloop
ret

revspr:
ld a,0
ld de,16 ;thanks Z80
revlp:
outi
outi
outi
outi
outi
outi
outi
outi
xor a
sbc hl,de
outi
outi
outi
outi
outi
outi
outi
outi
xor a
sbc hl,de
cp b
jp c, revlp
ret

