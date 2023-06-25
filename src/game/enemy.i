;I guess 7500 will be the enemy table then
;0 enemy type (0 for no enemy),the other 15 bytes vary with the enemy type (but we boot it with X,X, Y so the enemy knows where it was spawned)
enetmp: .db 0,0,0,0,0,0,0
enepar: .ds 16,0
maxenemy: .db $40

enetimer: .db 0

InitEnemy:
ld hl,$7500
ld a,0
ld c,255
cleloop:
	ld (hl),a
	inc hl
	dec c
jp nz,cleloop
ret

RunEnemy:
	ld hl,$7500
	reloop:
	ld (enetmp),hl
	ld a,(hl)
	cp 0
	jp z,notenemy
		ld de,enepar
		ld bc,$10
		ldir ;Writes the current parameters of the enemy in enepar to easy handling
		;ld de,enejmp ;pointer to pointers of enemies
		dec a ;type of enemy doubles as "there is an enemy on this slot?" so 1
		rlca ;Pointers have two bytes
		ld l,a
		ld h,0
		add hl,de
		ex de,hl
		ld a,(de)
		ld l,a
		inc de
		ld a,(de)
		ld h,a
		jp (hl) ;And finally jump to the pointer fetched from the list above
	retene:
		ld hl,enepar
		ld bc,$10
		ld de,(enetmp)
		ldir ;Copies it back
	notenemy:
	ld hl,(enetmp)
	ld de,16
	add hl,de
	ld a,(maxenemy)
	ld b,a
	ld a,l
	cp b ;Maximum enemy numbers
	jp nz,reloop
	ld hl,enetimer
	inc (hl)
ret

;Summoned by the object system
AddEnemy:
	ld hl,$74F0
	ld de,$10
	FindFreeEnemy:
		add hl,de
		ld a,(hl)
		cp 0
		jp nz,FindFreeEnemy
	ex de,hl
	ld hl,(tmpstru) ;Loads the pointer to the object system because the data we need is there
	inc hl
	ld a,(hl) ;Roll to the subtype
	inc a
	ld (de),a ;Copy subtype (increased by 1) to the enemy structure
	inc de
	inc hl ;Jump to coordinates
	ld a,(tmpstru+6)
	ld (de),a ;Object ID
	inc de
	ld bc,6 ;unlike outi, ldir need the full BC
	ldir ;Just copy the data that was already on the object
	ld a,$ff
	ld (de),a; A signal that it's a boot
	
ret

GetLocal: ;Very specific function that gets the coordinate on the init table and turns it into local DE
	ld de,(enepar+2)
	ex de,hl
	ld bc,(CamX)
	sbc hl,bc
	ld d,l
	ld a,(enepar+4)
	ld e,a
ret

;Returns on A if the penguin is at the left/right and up/down [bit 0 left right, bit 1 up down]
GetUpenSide:
	push hl
	push de
	ld hl,(enepar+4)
	ld a,(pen_y)
	sbc a,l
	ld a,0
	adc a,a
	rlca ;add carry,turn into 2 (completing Y quite quickly)
	ld hl,(enepar+2)
	ld de,(pen_x)
	sbc hl,de
	adc a,a; Add carry (completing X)
	pop de
	pop hl
	
ret
;C Object ID, DE Screen/X B, Y
;UpdateSpriteGlobal

;Return 1 on A if offscreen
GetOffscreen:
		ld c,0 ;Not dead
		ld hl,(enepar+2)
		ld de,(CamX)
		sbc hl,de
		ld a,h
		cp 0
		jp z,notdead ;Still alive
		ld a,l
		xor h ;This kinda inverts but not really the situation in case of negative page
		cp 16
		jp nc,notdead
			ld c,1
		notdead:
		ld a,c
ret
;Enemy boots with : [0] subtype, [1] Object ID,[2,3] XSC-X, [4] Y, [5,6,7] custom data coming from editor,[8] start as $ff to indicate its a boot


