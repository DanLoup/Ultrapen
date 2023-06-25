SetupMemory:
	ld de,tblcpy
	ld hl,$F341
	ld bc,8
	ldir
	ld b,0
	call GetPPage
	ld a,c
	ld (basicprisec),a
	call GetSPage
	ld a,c
	ld (basicprisec+1),a
	
ret
;Sets carry if we're dealing with a drive

Addmask: .db $FC,$F3,$CF,$3F
;Sets the primary page B with C
SetPPage:
	push bc
	push de
	push hl
	ld a,b
	ld hl,Addmask
	add a,l
	ld l,a
	ld d,(hl)
	in a,($a8)
	and d
	ld d,a ;d masks out the bytes to alter
	ld a,c ;value to multiply
	inc b ;thing to make zero != 1
	Msetp:
		dec b
		jp z,Endsetp
		rlca
		rlca ;Pushes the bits up until it fits
		jp Msetp
	Endsetp:
	or d
	out ($a8),a
	pop hl
	pop de
	pop bc
ret 

;Sets secondary page B with C
SetSPage:
	push bc
	push de
	push hl
	ld a,b
	ld hl,Addmask
	add a,l
	ld l,a
	ld d,(hl)
	ld a,($ffff)
	xor $ff
	and d
	ld d,a ;d masks out the bytes to alter
	ld a,c ;value to multiply
	inc b ;thing to make zero != 1
	Msets:
		dec b
		jp z,Endsets
		rlca
		rlca ;Pushes the bits up until it fits
		jp Msets
	Endsets:
	or d
	ld ($ffff),a
	pop hl
	pop de
	pop bc
ret 

;Gets the primary page at B and sets to C
GetPPage:
	push bc
	in a,($a8)
	inc b ;thing to make zero != 1
	Mgetp:
		dec b
		jp z,Endgetp
		rra
		rra ;Pushing the bits down
		jp Mgetp
	Endgetp:
	and $03
	pop bc
	ld c,a ;and as promissed, C returns the page number

ret 

;Gets the secondary page at B to C
GetSPage:
	push bc
	ld a,($ffff)
	xor $ff
	inc b ;thing to make zero != 1
	Mgets:
		dec b
		jp z,Endgets
		rra
		rra ;Pushing the bits down
		jp Mgets
	Endgets:
	and $03
	pop bc
	ld c,a ;and as promissed, C returns the page number
ret 

;Sets primary and secondary page B with C
SetPS:
	push de
	push bc
	push hl
	ld d,b
	ld e,c
	ld a,c
	and $03
	ld c,a
	ld b,3
	call SetPPage
	ld b,d
	call SetPPage
	ld a,e
	rra
	rra
	and $03
	ld c,a
	call SetSPage
	pop hl
	pop bc
	pop de
ret

SetMemmode:
	ld a,(tblcpy)
	ld c,a
	ld b,0
	call SetPS
	ld a,(tblcpy+1)
	ld c,a
	ld b,1
	call SetPS
	ld a,(tblcpy+3)
	ld c,a
	ld b,3
	call SetPS
ret

SetDiskmode;
	;ld a,(tblcpy)
	ld a,0
	ld a,(basicprisec)
	ld c,a
	ld b,0
	
	call SetPS
	ld a,(tblcpy+7)
	ld c,a
	ld b,1
	call SetPS
	ld a,(tblcpy+3)
	ld c,a
	ld b,3
	call SetPS
ret

