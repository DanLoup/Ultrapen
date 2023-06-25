;Video mem
;4800-4A00 sprite line colors
;4A00-4B00 Sprite parameters (i could just change those to 5800 etc and theorically have no conflict with the sprite graphics)
;5000-5800 sprite bitmaps


;7000-70E0 Sprite table: 0 state,1 x,2 y,3 count,4 width,5 inisprite,6 frame,7 jump table number(multi by 4 so can direct paste)
;State bits: (0 free,1 used,2 flip,4 update frame,8 update color table,16 flip mirror, 32 multicolor,64 alt graphictable)
;16 turns into 8 as soon sprite ends
;70E0-70FF Sprite allocation table
;7100 Sprite jump table and graphics offset
;7200 Sprite flip table

;Structure of the sprite in the file: 0 type, 1 subtype,2 wid,3 hei,4 nbframes,5 singlecolor pointer (probably useless),6 Max number of tiles(mostly used by the editor)


genspr: .ds 32,0
spriteptstack .dw $7100
cursptable: .db 0,$40,$97 ;40 page 0, 50 page 1
cotablemode: .db 0

InitSpriteSystem:
	ld hl,$7000
	ld bc,$0300
	ld a,0
	initmemsp:
			ld (hl),a
			inc hl
			dec c
		jp nz,initmemsp
		dec b
	jp nz,initmemsp

	ld hl,$7200
	revtablegen:
		ld b,l
		ld d,8
		ld a,0
	revtablegenm:
			rl b ;Bit gets pushed out by the left
			rr a ;Bit get pulled in by the left, reversing the order
			dec d 
		jp nz,revtablegenm
		ld (hl),a
		inc l
	jp nz,revtablegen
ret

RunSpriteSystem:
	ld c,$99
	ld hl,$018e ;Sprite page
	out (c),h
	out (c),l
	ld a,(cursptable+2) ;97 for page 0, b7 for page 1
	out (c),a
	ld a,$85
	out (c),a

	ld hl,$7000
	ld (genspr),hl
	seekspriteRSS:
		ld a,(hl)
		cp 0
		jp z,freespriteRSS
		ld (genspr+4),a ;A good copy of the sprite flag, for use on anything
		ld de,$5
		add hl,de
		ld a,(cursptable)
		cp 0
		ld a,(hl) ;initial sprite
		ld (genspr+9),a ;Saving the inisprite to use down there to calculate the sprite tile pointer thing
		jp z,page0RSS
			ld e,a ;On page 1, the sprite list is pretty much the same, but the order of the inisprites is reversed
			ld a,$20 
			sub e 
			dec l
			dec l
			ld e,(hl) ;Jezz, you really need to optimize this mess
			sub e
			inc l
			inc l
		page0RSS:
		rlca 
		rlca ;Multiply by 4
		out ($99),a ;Address (good freaking thing the table is like 128 bytes big)
		ld a,(cursptable+1) ;This one will have to switch around in the future
		add a,$a ; Heh
		out ($99),a ;Sprite table, writing
		ld a,(genspr+4)
		and 64 ;It's a page 2 sprite
		rrca ;Which means it's 32 bytes ahead
		ld l,a
		ld a,(genspr+9)
		add a,l
		rlca
		rlca ;It's 4 in 4
		ld (genspr+9),a ;And now we finally have the sprite graphics pointer
		
		ld hl,(genspr)
		inc hl
		ld e,(hl)
		inc hl
		ld d,(hl)
		ld a,(sc_offs)
		add a,e
		ld e,a
		ld a,(sc_offs+1)
		add a,d
		ld d,a ;Precalculate position and offset
		ld (genspr+2),de
		inc hl
		ld c,(hl) ;tile count
		inc hl
		ld b,(hl) ;width
		ld a,16
		ld (genspr+10),a ;Direction the sprite width go
		ld a,(genspr+4)
		and 2
		jp z,mainloopRSS ;Flipped sprite
		dec b
		jp z,ZeroaddsRSS
		ld a,e
		multiflipRSS:
			add a,16
			dec b
		jp nz,multiflipRSS
		ld e,a
		ld (genspr+2),de
		ld b,(hl)
		ZeroaddsRSS:
		ld a,240
		ld (genspr+10),a ;And if its flipped, we walk to the left
	mainloopRSS:
			ld a,d
			out ($98),a ;y
			ld a,e
			out ($98),a ;x
			ld a,(genspr+9)
			out ($98),a ;Sprite ID
			add a,4
			ld (genspr+9),a
			in a,($98) ;Skipping tile config
			ld a,(genspr+4)
			and 32
			jp z,notmultiRSS
				ld a,d
				out ($98),a
				ld a,e
				out ($98),a
				ld a,(genspr+9)
				out ($98),a ;Sprite ID+1
				add a,4
				ld (genspr+9),a ;repeat almost everything
				in a,($98) ;So the multicolor scheme can work
				dec c; total tiles is doubled so
		notmultiRSS:
			ld a,(genspr+10) 
			add a,e 
			ld e,a ;add 16 to X to multiblock
			dec b
			jp nz,nohoriRSS
				ld b,(hl) ;Reset the width counter
				ld a,$10
				add a,d
				ld d,a ;Add 16
				ld a,(genspr+2)
				ld e,a ;And reset X
		nohoriRSS:
			dec c ;Total tile counter
		jp nz,mainloopRSS
	freespriteRSS:
		ld hl,(genspr)
		ld de,8
		add hl,de ;next sprite
		ld (genspr),hl
		ld a,$E0 ;28 sprites total because i crammed the Sprite Allocation Table on the end
		cp l
	jp nz,seekspriteRSS
	
	ld a,(cursptable)
	xor 1
	ld (cursptable),a
	cp 0
	jp nz,npg0RSS
		ld a,$40
		ld (cursptable+1),a
		ld a,$97
		ld (cursptable+2),a
	jp exixpg0RSS
	npg0RSS:
		ld a,$50
		ld (cursptable+1),a
		ld a,$b7
		ld (cursptable+2),a
	exixpg0RSS:

	
	
ret
;This will be the "update all" function
CheckForCF:
	ld c,$99
	ld hl,$018e ;Sprite page
	out (c),h
	out (c),l
	
	ld hl,$7000
FindUpdateCF:
	ld (genspr),hl
	ld a,(hl)
	and 8
	ld (cotablemode),a

	ld a,(hl)
	and 12
	call nz,UpdateFrame
	ld hl,(genspr)
	ld a,(hl)
	and 8
	jp z,remove8CCF
		ld a,(hl)
		xor 8
		ld (hl),a
	remove8CCF:
	ld a,(hl)
	and 4
	jp z,flipto8CCF
		ld a,(hl)
		xor 4
		or 8
		ld (hl),a
	flipto8CCF:
	ld de,8
	add hl,de
	ld a,l
	cp $e0
	jp nz,FindUpdateCF
ret

UpdateFrame:

	ld a,(hl)
	and 32
	call z,5 ;Debug


	ld a,(hl)
	ld b,a
	and 64 ;Is you an alt frame?
	xor 64 ;Flips result
	ld (genspr+2),a 
	ld a,b
	ld (genspr+9),a ;Multitile
	ld de,5
	add hl,de
	ld a,(hl) ;Fetching inisprite
	rrca
	rrca
	rrca ;Wraps A around 
	ld b,a
	and $e0
	ld c,a ;gets the higher 3 bits and puts on low, this way effectively shifting stuff around 5 times (mul by 32 low)
	ld a,b
	and $1f
	ld b,a ;And with this, i get the higher 5 bits and place on the high byte, thus multiplying A by 32
	ld (genspr+12),bc ;Copying Inisprite for later
	ld hl,$5000
	add hl,bc ;Bingo, sprite bitmaps
	ld a,(genspr+2) ;but
	cp 0
	jp z,NotAltUF
		ld bc,$400
		add hl,bc ;It's an alternate sprite
	NotAltUF:
	ld a,(vdpreg) ;Gotta respect the MSX standard
	ld c,a
	out (c),l
	nop
	out (c),h ;sets VRAM address to sprite graphics
	ld hl,(genspr)
	ld de,3 ;Number of sprites
	add hl,de
	ld a,(hl)
	ld (genspr+2),a ;Total number of blocks to copy
	ld (genspr+11),a ;Shadow for the color copy
	ld b,a
	ld a,(genspr+9)
	and 32
	jp z,AnotherMUltitestUF
		xor a
		rrc b
	AnotherMUltitestUF:
	add hl,de
	ld c,(hl) ;Frame selected
	ld a,0
	MultiFUF:
		add a,c
		dec b
	jp nz,MultiFUF
	ld (genspr+4),a ;Number of tiles to skip
	inc hl
	ld c,(hl) 
	ld b,$71 ;Assembles adress
	ld a,(bc)
	ld l,a
	inc bc
	ld a,(bc)
	ld h,a ;And with this, we fetch the data pointer
	ld de,7
	add hl,de ;Adress to the first frame
	ld a,(genspr+4)
	ld e,a
	add hl,de ;Adress to the correct frame
	ld (genspr+5),hl
	ld (genspr+14),hl ;Pointer to the actual data (start)
	ld hl,(genspr)
	ld de,7; Jump table
	add hl,de
	ld c,(hl)
	ld b,$71
	inc c
	inc c
	ld a,(bc)
	ld l,a
	inc c
	ld a,(bc)
	ld h,a ;Graphical pointer
	ld (genspr+7),hl

	ld a,(cotablemode)
	cp 0
	jp nz,ColorCopyMode
	
	ld a,(genspr+9)
	and 16
	jp nz,UploadFrameFlipped
	UploadFrameUF:
		ld hl,(genspr+5) ;Read frame pointer
		ld b,(hl) ;Fetch frame number
		inc hl
		ld (genspr+5),hl
		ld a,(genspr+9)
		and 32
		jp z,notmultiaddrUF
			rlc b ;Frame need to be doubled in multicolor
		notmultiaddrUF:
		ld a,b
		call MulA48
		ld hl,(genspr+7);Graphical data address
		add hl,de
		ld a,(vdpmem) ;Gotta respect the MSX standard
		ld c,a
		ld b,$20
		otir
		ld a,(genspr+9)
		and 32
		jp z,notmultitileUF
			ld a,(vdpmem)
			ld c,a
			ld b,$20
			otir ;Copy the following block because it's a multicolor sprite
			ld a,(genspr+2)
			dec a
			ld (genspr+2),a
		notmultitileUF:
		ld a,(genspr+2)
		dec a
		ld (genspr+2),a
	jp nz,UploadFrameUF

	jp UploadFrameExit1
	UploadFrameFlipped:
		ld hl,(genspr+5)
		ld b,(hl)
		inc hl
		ld (genspr+5),hl
		ld a,(genspr+9)
		and 32
		jp z,notmultiaddrUFF
			rlc b
		notmultiaddrUFF:
		ld a,b
		call MulA48
		ld hl,(genspr+7)
		add hl,de
		ld a,(vdpmem)
		ld c,a
		ld b,$2

		ld de,$10 ;Better explain this thing right
		add hl,de ;16x16 sprites are actually 2 columns of 8x16 tiles
		ld d,$72
		call Flippedsubr ;So we copy the right column on the left side
		ld de,$20
		sbc hl,de
		ld b,$2
		ld d,$72
		call Flippedsubr ;and the left column on the right side

		ld a,(genspr+9)
		and 32
		jp z,notmultitileUFF
		ld de,$20
		add hl,de
		ld b,$2
		ld d,$72
		call Flippedsubr ;And here on the multitile, we skip ahead to get the fourth column to the third
		ld de,$20
		sbc hl,de
		ld b,$2
		ld d,$72
		call Flippedsubr ;and finally third column to the fourth
			ld a,(genspr+2)
			dec a
			ld (genspr+2),a
		notmultitileUFF:
		
		ld a,(genspr+2)
		dec a
		ld (genspr+2),a
	jp nz,UploadFrameFlipped
	
	UploadFrameExit1:

	ld hl,(genspr)
	ld a,(hl)
	and $ff-2 ;Frame updated (also remove flip in case of flip)
	ld (hl),a
	and 16
	ld a,(hl)
	jp z,NoflipaddCF
		and $ef
		or 2
	NoflipaddCF:
	xor 64
	ld (hl),a


ColorCopyMode:
	ld bc,(genspr+12) ;Fetch inisprite
	xor a
	rr b
	rr c ;divide by 2
	ld a,(cursptable)
	cp 0
	jp z,reversetableUF ;When on page 1, the sprite order is reversed so we can have sprite blinking
	
	ld hl,$200
	sbc hl,bc ;Subtracts the calculated value from 0x200 to revert the address

	ld a,(genspr+11) ;Number of tiles to copy
	rlca
	rlca
	rlca
	rlca ;* 16
	ld c,a
	ld b,0
	sbc hl,bc ;Subtracts the calculated value from 0x200 to revert the address
	push hl
	pop bc
	reversetableUF:


	ld l,$00 ;4800 page 0, 5800 page 1
	ld a,(cursptable+1)
	add a,8
	ld h,a
	add hl,bc ;Just dividing by half because 16 bytes instead of 32
	ld a,(vdpreg) ;Gotta respect the MSX standard again
	ld c,a

	out (c),l
	nop
	out (c),h ;sets VRAM address to sprite color pointer

	ld hl,32
	ld a,(genspr+9)
	and 32
	jp z,notmultiaccolUF
	ld hl,64	
	notmultiaccolUF:
	ld (genspr+5),hl

	UploadColorUF:
	ld hl,(genspr+14)
	ld c,(hl) ;Getting frame
	inc hl
	ld (genspr+14),hl
	ld b,16
	ld a,(genspr+9)
	and 32
	jp z,notmultiacolUF
		rlc c
		ld b,32
	notmultiacolUF:
	ld a,c
	call MulA48
	ld hl,(genspr+7)
	add hl,de
	ld de,(genspr+5)
	add hl,de
	ld a,(vdpmem)
	ld c,a

	otir ;Copy the following block because it's a multicolor sprite
	ld a,(genspr+9)
	and 32
	jp z,doubledrUCT
		ld a,(genspr+11)
		dec a
		ld (genspr+11),a
	doubledrUCT:
		ld a,(genspr+11)
		dec a
		ld (genspr+11),a

	jp nz,UploadColorUF

ret

UpdateColorTable:

ret
;A is the frame id, DE returns the multiplied by 48
MulA48:
	push hl
	rrca
	rrca
	rrca
	ld h,a
	and $e0
	ld l,a
	ld a,h
	and $1f
	ld h,a ;Multiply A by 32
	ld d,h
	ld e,l
	xor a
	rr d
	rr e
	add hl,de ;Final result, A multiplied by 48 on HL
	ex de,hl
	pop hl
ret

;HL address of the pointers, C number of pointers
AddSpritePointers:
	ld (genspr),hl
	ld a,c
	ld (genspr+8),a 
	asploop:
		ld c,(hl) 
		inc hl
		ld b,(hl) ;Fetch pointer from the table
		inc hl
		push hl ;Saves position on the table
		ld hl,(genspr)
		add hl,bc ;Add the offset of the table itself to the offsets
		ex de,hl
		ld hl,(spriteptstack)
		ld (hl),e
		inc hl
		ld (hl),d ;And writes it to the table
		inc hl
		ld (spriteptstack),hl ;but for speed stuff up, i'm also building a pointer to the graphics
		ex de,hl
		inc hl
		inc hl
		ld a,(hl) ;Size X
		inc hl
		ld c,(hl) ;Size Y
		inc hl
		ld d,(hl) ;Numbr of frames
		ld b,0
		push hl
		ld hl,0
		mulasp1:
			add hl,bc
			dec a
		jp nz,mulasp1 ;Multiply width and height
		ld a,d
		ex de,hl
		pop hl
		mulasp2:
			add hl,de
			dec a
		jp nz,mulasp2 ;Multiply total tile count by the number of frames
		ld de,3 ;just skipping the rest of the header
		add hl,de
		ex de,hl
		ld hl,(spriteptstack)
		ld (hl),e
		inc hl
		ld (hl),d
		inc hl
		ld (spriteptstack),hl ;All this to generate the pointer to the graphical data
		pop hl
		ld a,(genspr+8)
		dec a
		ld (genspr+8),a
	jp nz,asploop
ret



Flippedsubr:
		ld e,(hl) ;Unrolled flipped copy comand to be faster
		ld a,(de)
		out (c),a
		inc hl
		ld e,(hl)
		ld a,(de)
		out (c),a
		inc hl
		ld e,(hl)
		ld a,(de)
		out (c),a
		inc hl
		ld e,(hl)
		ld a,(de)
		out (c),a
		inc hl
		ld e,(hl)
		ld a,(de)
		out (c),a
		inc hl
		ld e,(hl)
		ld a,(de)
		out (c),a
		inc hl
		ld e,(hl)
		ld a,(de)
		out (c),a
		inc hl
		ld e,(hl)
		ld a,(de)
		out (c),a
		inc hl
		dec b
	jr nz,Flippedsubr
ret


;E number of sprites to be subtracted
RemoveSpritePointers:
	rlc E
	ld d,0
	ld hl,(spriteptstack)
	sbc hl,de
	ld (spriteptstack),hl
ret


;HL Type/Subtype,DE X,Y,B Flip X,Y ,C Frame
;A returns The Object ID
CreateSprite:
	ld (genspr),hl
	ld (genspr+2),bc
	ld (genspr+4),de
	ld hl,$7100
	FindTypesubCS:
		ld e,(hl)
		inc hl
		ld d,(hl)
		inc hl ;Read the pointers from the sprite data pointer list
		ex de,hl ;And now HL is a pointer to the data
		ld a,(genspr+1)
		cp (hl)
		jp nz,NotThisCS
		inc hl
		ld a,(genspr) ;And here we compare the type/subtype of the data
		cp (hl)
		jp nz,NotThisCS
		jp foundTypeCS
	NotThisCS:
	ex de,hl ;Back to the pointer being for the 
	inc hl
	inc hl ;Pointer list also contains a direct jump to the graphics
	ld a,l
	cp $00
	jp nz,FindTypesubCS
	ld a,255 ;Failed seek
	ret ;Not found
	foundTypeCS:
	dec de
	dec de
	ld (genspr),de ;Saving the address of the pointer so i can put it on the last thingamabob later
	dec hl
	ld de,genspr+8
	ld bc,8
	ldir ;Copying the sprite data because easier to deal
	;0 type, 1 subtype,2 wid,3 hei,4 nbframes,5 singlecolor pointer (probably useless),6 Max number of tiles(mostly used by the editor)
	ld de,(genspr+8+2) ;As with the table above, we're getting wid hei
	ld c,d
	MultiCS:
		add a,e
		dec c
		jp nz,MultiCS
	ld e,a ;Now DE contains Width and count, and as we have count...
	ld a,(genspr+8+5) ;Fetching singlecolor
	cp 0
	jp nz,NotMcCS
		rlc e ;Stop right there, criminal scumm, need to multiply by 2 if it's a multicolor to allocate it correctly
	NotMcCS:
	ld hl,$70E0;Time to find a free slot
	ld c,0
	FindSlotCS:
		inc c
		ld a,(hl)
		inc hl
		cp 0
		jp z,keepgoingCS
			ld c,0
		keepgoingCS:
		ld a,l
		cp 0
		jp z,NoSlotsCS
		ld a,e
		cp c
	jp nz,FindSlotCS
	;HL now contains the last address of a valid block
	;DE have width and count
	;BC can be used for fun stuff
	push hl
	push de
	ld hl,$7000
	ld de,8
	ld c,0
	FindSpriteCS:
		ld a,(hl)
		cp 0
		jp z,FoundSpriteCS
		add hl,de
		inc c
		ld a,l
		cp $e0
	jp nz,FindSpriteCS
	pop hl
	pop de ;Just reverting above
	ld a,255
	ret ;No sprite slot available
	FoundSpriteCS:
			ld a,c
			ld (genspr+15),a
			ld b,5 ; used+update sprite (no subcolor)
			;Now this HL contains the sprite address
			ld a,(genspr+8+5) ;Fetching singlecolor
			cp 0
			jp nz,NotMulticolorCS
				ld a,32
				add a,b
				ld b,a
			NotMulticolorCS:
			ld a,(genspr+3) ;Fetches flip
			cp 0
			jp z,NotFlippedCS
				ld a,2
				add a,b
				ld b,a
			NotFlippedCS:
			ld a,b
			ld (hl),a ;And the sprite is finally configured
			inc hl
			ld a,(genspr+5) ;X
	 		ld (hl),a
			inc hl
			ld a,(genspr+4) ;Y
			ld (hl),a
			inc hl
			pop de
			ld (hl),e ;Count
			inc hl
			ld (hl),d ;Width
			inc hl
			pop bc ;Pull the sprite table adress
			push hl ;And push the current write head
			ld a,l
			srl a 
			srl a
			srl a
			inc a ;Calculates the Sprite table ID thing
		WriteAllocCS:
				dec bc
				ld (bc),a
				dec e ;Allocating the sprite table
			jp nz,WriteAllocCS
			ld a,c
			sub $e0 ;Oh gosh! it's exactly what i need for the inisprite
			pop hl
			ld (hl),a ;Inisprite
			inc hl
			ld a,(genspr+2)
			ld (hl),a ;Frame
			inc hl
			ld a,(genspr) ;Low byte of the sprite pointer
			ld (hl),a ;Compressed pointer to the sprite table
			ld a,(genspr+15)
ret
NoSlotsCS:
ld a,255
ret



;C Object ID, DE X,Y
UpdateSprite:
	push hl
	ld a,c
	rlca
	rlca
	rlca ;Multiply by 8
	ld h,$70
	inc a
	ld l,a
	ld (hl),d
	inc hl
	ld (hl),e
	pop hl
ret

;C Object ID,DE flip,frame
UpdateSpriteFrame:
	push hl
	ld a,c
	rlca
	rlca
	rlca ;Multiply by 8
	ld h,$70
	ld l,a
	ld a,(hl)
	and $ff-8 ;Just to avoid some terrible mistakes above
	or 4
	and $ef ;Remove flip in case of not flip
	ld (hl),a
	ld a,0
	cp e
	jp z,notflipUSF
		ld a,(hl)
		or 16
		ld (hl),a
	notflipUSF:
	ld a,6
	add a,l
	ld l,a
	ld (hl),d
	
	pop hl
ret

;C Object ID
RemoveSprite:
	push hl
	push de
	ld a,c
	rlca
	rlca
	rlca ;Multiply by 8
	ld h,$70
	ld l,a ;Calculate the adress to the sprite
	ld a,0 
	ld (hl),a ;Remove the sprite reference 
	ld de,3
	add hl,de 
	ld d,(hl) ;Count
	inc l
	inc l
	ld e,(hl) ;Initial S
	ld a,e
	add a,$e0
	ld l,a ;And with this, we're already on the table
	ld a,0
	ld (genspr),de
	RSloop:
		ld (hl),a
		inc l
		dec d
	jp nz,RSloop
	ld c,$99
	ld hl,$018e ;Sprite page
	out (c),h
	nop
	out (c),l ;Time to remove em from both sprite pages
	ld de,(genspr)
	ld a,$20
	sub e ; Flip stuff
	sub d ; to the start of the sprites
	rlca
	rlca ;MUltiply by 4 to get to the address
	ld h,$5A ;Page 2
	out (c),a
	out (c),h ;Setup some pages
	ld c,$98
	ld a,$ff
	RSloop2:
		out (c),a
		out (c),a
		out (c),a
		out (c),a ;Wreck the sprite page 1
		dec d
	jp nz,RSloop2
	ld de,(genspr)
	ld a,e
	rlca
	rlca 
	ld h,$4A
	ld c,$99
	out (c),a
	out (c),h ;Setup page 0
	ld c,$98
	ld a,$ff
	RSloop3:
		out (c),a
		out (c),a
		out (c),a
		out (c),a ;Wreck the sprite page 0 as well
		dec d
	jp nz,RSloop3
	pop de
	pop hl
ret
