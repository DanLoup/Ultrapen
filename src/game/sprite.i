;Video mem
;4800-4A00 sprite line colors
;4A00-4B00 Sprite parameters (i could just change those to 5800 etc and theorically have no conflict with the sprite graphics)
;5000-5800 sprite bitmaps


;7000-70E0 Sprite table: 0 state,1 x,2 y,3 count,4 width,5 inisprite,6 frame,7 jump table number(multi by 4 so can direct paste)
;State bits: (0 free,1 used,2 flip,4 update frame,8 update color table,16 flip mirror, 32 multicolor,64 alt graphictable,128 VRAM sprite)
;An idea would be to combine 4 and 8 into an actual 2 bit state value
;12 first copy, 8/4 (color copy is at page x), 0 done.. seems reasonable
;16 turns into 8 as soon sprite ends
;70E0-70FF Sprite allocation table
;7100 Sprite jump table and graphics offset
;7200 Sprite flip table
;7300-7380 Sprite table buffer (to allow the engine to ALWAYS send sprites to table during Vsync) 
;Structure of the sprite in the file: 0 type, 1 subtype,2 size,3 nothing,4 nbframes,5 singlecolor pointer (probably useless),6 Max number of tiles(mostly used by the editor)


genspr: .ds 32,0
spriteptstack .dw $7100
cursptable: .db 0,$40,$97,$50 ;40 page 0, 50 page 1 (adding one more just for testing a thing)
cotablemode: .db 0
tuc: .db 0 ;Tile Upload Counter, gives a rough approximation of how heavy is the heaviest function in the game
maxsprite:.db $0 ;Number that will self adjust to the maximum sprite number allocated to save (actually a lot) of CPU time
maxspritetb: .db $40 ;number of bytes to copy per frame, multiple of 16, minimun of 16

sizetable: .db 1,4,2,8 ;Number of tiles required per size
SPpostable: .db 0,0,  0,0,0,16,16,0,16,16, 0,0,0,0,  0,0,0,0,0,16,0,16,16,0,16,0,16,16,16,16
SPrpostable: .db 0,0,  0,16,0,0,16,16,16,0, 0,0,0,0, 0,16,0,16,0,0,0,0,16,16,16,16,16,0,16,0
SPjtable: .db 0,2,10,14


tmpxspriteoff: .db 0
gc: .db 0,0
lockflicker: .db 0
dsprites: .db 0
bakect: .db 0
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


	ld hl,$73ff
	ld a,$e0
	initmemsp2:
		ld (hl),a
		dec l
	jp nz,initmemsp2


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
	ld c,$99
	ld de,$038e ;Set page 3 because its where clipped sprites go
	out (c),d
	out (c),e
	ld de,$6000 ;E000 is the zone to clear
	out (c),e
	out (c),d
	ld hl,$1fff
	ld a,0
	clearclip:
	out ($98),a
	dec l
	jp nz,clearclip
	dec h
	jp nz,clearclip
	
	ld de,$008e ;And reset the page like a good boy
	out (c),d
	out (c),e

ret
bt1: .ds 32,0



RunSpriteSystem:
	ld a,(msxplus)
	cp 1
	jp nz,mxp3
		ld a,7
		ld (tmpxspriteoff),a
		jp nomxp3
	mxp3:
		ld a,(sc_offs)
		ld (tmpxspriteoff),a
	nomxp3:
	ld hl,$7000
	ld (genspr),hl
	ld a,64
	ld (maxspritetb),a	

	seekspriteRSS:

		ld a,(hl)
		cp 0
		jp z,freespriteRSS
		ld (genspr+9),a ;A good copy of the sprite flag, for use on anything
		ld de,$3 ;First, we need the sprite size
		add hl,de
		ld d,(hl) ;Size!
		inc l
		inc l ;Roll to inisprite
		;this is probably too small to be an effective table

		;position on the sprite table calculator
		ld a,(cursptable) ;controls the JP Z down there for sprite inversion
		cp 0
		ld a,(hl) ;initial sprite on the sprite memory

		ld (genspr+4),a ;Saving the inisprite to use down there to calculate the sprite tile pointer thing
		jp nz,page0RSS
			ld e,a ;On page 1, the sprite list is pretty much the same, but the order of the inisprites is reversed
			ld a,$20 
    		sub e ;32-inisprite 
			sub d ;-Size
			add $20 ;for some reason, this was doing math down there
		page0RSS:
		rlca 
		rlca ;Multiply by 4
		;Theorically speaking, i need 79(A) here as DE
		ld (genspr+12),a ;Just storing the address for now

		ld a,(hl) ;initial sprite on the sprite memory
		ld (genspr+4),a ;Saving the inisprite to use down there to calculate the sprite tile pointer thing

		ld a,(bakect)
		cp 0
		ld a,(genspr)
		call nz,BakeSpriteData

		; sprite position math (the only one that will be left)
		ld hl,(genspr)
		inc l
		ld d,(hl)
		inc l
		ld e,(hl)
		ld a,(tmpxspriteoff)
		add a,d ;Add X scroll
		ld d,a
		ld a,(sc_offs+1)
		add a,e
		sub $10 ;Screen got rolled up so this serve to counteract it
		ld e,a ;Precalculate position and offset
		ld (genspr+2),de
		

		inc l
		ld c,(hl) ;tile count

		;Setup the multicolor position
		ld a,1
		cp c
		jp z,flipsetRSS ;don't do this math for single tile sprites
		inc hl
		ld d,0
		ld e,(hl) ;Sprite type
		ld hl,SPjtable
		add hl,de
		ld e,(hl) ;Sprite table offset
		ld d,0
		ld a,(genspr+9)
		and 2
		jp nz,noflipRSS ;Flipped sprite
			ld hl,SPpostable
			add hl,de
			jp flipsetRSS
		noflipRSS:
			ld hl,SPrpostable
			add hl,de
		flipsetRSS:
		;so, burn 2 bytes or do that math above?
		;I don't think this routine in particular is very heavy, specially given the massive skip it suffers
		;For most objects

		;Rest of the landing address math
		ld a,(genspr+12)
		ld e,a
		ld d,$73
		ld a,1
		cp c
		jp nz,mainloopRSSS ;If it's a single sprite, better off just skipping all the math
			ld hl,genspr+2
			ldi			
			ldi			
		jp freespriteRSS
		mainloopRSSS:

		ld ix,(genspr+2) ;The hell, a case where IX is actually faster
		mainloopRSS:
			ld a,ixl
			ld b,(hl)
			add a,b
			ld (de),a
			inc e
			inc hl
			ld a,ixh
			ld b,(hl)
			add a,b
			ld (de),a
			inc hl
			inc e
			inc e
			inc e
			dec c
			ld a,ixl
			ld b,(hl)
			add a,b
			ld (de),a
			inc e
			inc hl
			ld a,ixh
			ld b,(hl)
			add a,b
			ld (de),a
			inc hl
			inc e
			inc e
			inc e
			dec c ;It's more than one, so a basic loop unroll help a bit
		jp nz,mainloopRSS


	freespriteRSS:
	ld a,(genspr+9)
	and 2
	jp nz,@nohs 
	ld a,e
	cp 63
	jp c,@nohs
		ld a,128
		ld (maxspritetb),a
	@nohs:
		ld hl,(genspr)
		ld de,8
		add hl,de ;next sprite
		ld (genspr),hl
		ld a,(maxsprite) ;28 sprite objects total because i crammed the Sprite Allocation Table on the end
		cp l
	jp nz,seekspriteRSS

	ld a,(gc)
	cp 1
	jp nz,nogc
	ld hl,$70d0
	ld de,$0008
	findmaxlp:
		ld a,(hl)
		and 1
		jp nz,foundmax
		sbc hl,de
	jp nz,findmaxlp
	ld a,0
	ld (maxsprite),a
	ld (gc),a
	ret
	foundmax:
	ld a,l
	add a,$8
	ld (maxsprite),a
	ld a,0
	ld (gc),a
	nogc:

	ld a,(bakect) ;controls the JP Z down there for sprite inversion
	cp 0
	jp z,bkcl1
		ld a,(bakect)
		dec a
		ld (bakect),a
	bkcl1:
	ld a,(clearspritetable)
	cp 0
	jp z,@itsfine
		ld a,128
		ld (maxspritetb),a

	@itsfine:
ret

BakeSpriteData:
	ld h,$70
	ld l,a
	ld a,(hl)
	ld (genspr+9),a
	ld a,3
	add l
	ld l,a
	ld d,(hl) ;size
	;Sprite tile id
	ld a,(genspr+9)
	and 64 ;It's a page 2 sprite
	rrca ;Which means it's 32 bytes ahead
	ld l,a
	ld a,(genspr+4)
	add a,l
	rlca
	rlca ;It's 4 in 4
	ld c,a
	ld e,0
	drawer:
	ld a,(genspr+12)
	add 2
	add e
	ld l,a
	ld h,$73
	ld (hl),c

	ld a,c
	add 4
	ld c,a
	ld a,e
	add 4
	ld e,a
	dec d
	jp nz,drawer
ret 

clearspritetable: .db 0
;Measures the highest sprite on the list and sets maxsprite accordingly
UpdateSpriteBank:
	;Load the table at 7300 and upload to the memory adress
	ld c,$99
	ld hl,$018e ;Sprite page
	out (c),h
	out (c),l
	ld a,(cursptable+2) ;97 for page 0, b7 for page 1
	out (c),a
	ld a,$85
	out (c),a
	ld a,0
	out ($99),a
	ld a,(cursptable+1) ;Sprite page for the sprite flicker
	add a,$a ; Heh
	out ($99),a ;Sprite table, writing
	;Now you actually copy the table
	ld a,(vdpmem) ;Gotta respect the MSX standard
	ld c,a
	ld a,(maxspritetb)
	ld b,a
	ld a,(cursptable)
	cp 0
	ld hl,$7300
	jp nz,revcopy
		ld a,128
		sub b
		ld e,a
		add $80
		ld l,a
		ld a,e
		out ($99),a
		ld a,(cursptable+1) ;Sprite page for the sprite flicker
		add a,$a ; Heh
		out ($99),a ;Sprite table, writing
	revcopy:
	call fotir

ret

flick_sprites:
	ld a,(clearspritetable)
	cp 0
	jp z,notthistimecleanlp
		ld a,$e0
		ld hl,$73e0
		cleanlp:
			ld (hl),a
			dec l
			ld (hl),a
			dec l
			ld (hl),a
			dec l
			ld (hl),a
			dec l
			jr nz,cleanlp
			;ld a,0
			;ld (clearspritetable),a

	ld a,(clearspritetable)
	dec a
	ld (clearspritetable),a


		notthistimecleanlp:		
	
	ld a,(lockflicker)
	cp 0
	jp nz,exixpg0RSS

	ld a,(cursptable)
	xor 1
	ld (cursptable),a ;Sprite bank flipper?
	cp 0
	jp z,npg0RSS
		ld a,$40 ;$40
		ld (cursptable+1),a
		ld a,$97 ;$97
		ld (cursptable+2),a

		ld a,$40 ;$50 ;Flipped one just for stuff
		ld (cursptable+3),a

	jp exixpg0RSS
	npg0RSS:
		ld a,$50 ;$50
		ld (cursptable+1),a
		ld a,$b7 ;$b7
		ld (cursptable+2),a

		ld a,$50 ;$40
		ld (cursptable+3),a

	exixpg0RSS:
ret 



;This will be the "update all" function
CheckForCF:
	ld a,0
	ld (tuc),a ;reset the tile upload counter
	ld c,$99
	ld hl,$018e ;Sprite page
	out (c),h
	nop
	out (c),l
	
	ld hl,$7000
FindUpdateCF:
	ld (genspr),hl
	ld a,(hl)
	and 8
	rrca
	ld b,a
	ld a,(hl)
	and 4
	xor b
	ld (cotablemode),a ;if 4 or 8 (but not both) is color table time

	ld a,(hl)
	and 12
	call nz,UpdateFrame
	ld hl,(genspr)
	ld a,(hl)
	and 12
	cp 12

	jp nz,norfirstuf
		ld a,(cursptable)
		cp 0
		jr nz,@secpage
			ld a,(hl)
			and 255-4
			ld (hl),a 
			jp totucuf
		@secpage:
			ld a,(hl)
			and 255-8
			ld (hl),a 
		jp totucuf
	norfirstuf:
	ld a,(hl)
	and 255-12
	ld (hl),a ;Remove all the remains as it's the second copy
	
	totucuf:

	ld a,(tuc)
	cp 16
	ret nc

	ld de,8
	add hl,de
	ld a,(maxsprite)
	ld b,a
	ld a,l
	cp b
	jp nz,FindUpdateCF
ret

outsprite: .db 0,0

UpdateFrame:
	ld a,4
	ld (bakect),a

	ld a,(hl)
	ld b,a
	and 64 ;Is you an alt frame?
	xor 64 ;Flips result

	ld (genspr+2),a ;Store the alt frame bit for future use
	ld a,b
	ld (genspr+9),a ;saving a copy of the sprite header for Multitile test
	ld de,5
	;Calculate (partially) the address of the sprite in memory
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
	ld (genspr+12),bc ;This is actually for the VRAM address of the sprite


	ld hl,(genspr)
	ld de,3 ;Number of sprites
	add hl,de
	ld a,(hl) ;This is fetched from the sprite header itself, not data in the vram
	ld (genspr+11),a ;Shadow for the color copy
	ld b,a

	ld a,(tuc)
	add a,b
	ld (tuc),a ;add the total number of blocks to tuc, so i can know how big it is

	ld a,(genspr+9)
	and 32
	jr z,AnotherMUltitestUF
		xor a
		rrc b ;Multi color, so need to halve to take it in account during the multi
	AnotherMUltitestUF: 
	add hl,de ;Quite obscure, it is jumping more 3 bytes
	ld c,(hl) ;Frame selected
	ld a,0 
	;This thing is potentially heavy as it's a literal multiply loop doing number of pieces * current frame
	;Given the now limited number of sizes, i probably can replace all this with some jps

	MultiFUF:
		add a,c
		dec b
	jr nz,MultiFUF
	;This thing is potentially heavy as it's a literal multiply loop
	;It also calculates the total number of tiles (size * frames)

	ld (genspr+4),a ;Number of tiles to skip
	inc hl
	ld c,(hl)  ;this combined with below
	ld b,$71 ;Assembles adress to the jump table, from sprite data
	ld a,(bc)
	ld l,a
	inc c
	ld a,(bc) 
	ld h,a ;And with this, we fetch the data pointer to the sprite data
	ld de,4 ;Skip the header..
	add hl,de ;for the address Adress to the first frame 
	ld a,(genspr+4)
	ld e,a
	add hl,de ;Adress to the correct frame

	ld (genspr+5),hl
	ld (genspr+14),hl ;Pointer to the actual frame data (start)
	;Now this one is harder to optimize

	ld hl,(genspr)
	ld de,7; Jump table
	xor a
	add hl,de
	ld c,(hl)
	ld b,$71 ;redoing the math to get the header...
	inc c
	inc c
	ld a,(bc)
	ld l,a
	inc c
	ld a,(bc) ;but with the twist we get the address from the next entry that has the color pointer
	ld h,a ;Graphical pointer from the table
	ld (genspr+7),hl

	inc c
	ld a,(bc)
	ld l,a
	inc c
	ld a,(bc)
	ld h,a
	ld (genspr + 24),hl ;Saving the color pointer as well for the vram sprites (and futurely normal sprites?)
	;i probably should integrate those pointers to the header itself
	;i bet i can turn em into 8bit offset registers 
	;Yep, i think it's trash, 4 pointers (and one being empty) is very bad	



	ld a,(cotablemode)
	cp 0
	jp nz,ColorCopyMode
	
	ld bc,(genspr+12) ;Loading address

	ld hl,$5000
	add hl,bc ;Bingo, sprite bitmaps
	ld a,(genspr+2) ;but
	cp 0
	jp z,NotAltUF
		ld bc,$400
		add hl,bc ;It's an alternate sprite
	NotAltUF:
	ld a,(genspr+11) 
	ld (genspr+2),a ;kludge to fix some reordering bug with the tile count
	
	ld a,(genspr+9)
	and 128
	jp nz, VramSpriteMode ;Hijack the execution just in time in case of vram sprites


	ld a,(vdpreg) ;Gotta respect the MSX standard
	ld c,a
	out (c),l
	nop
	out (c),h ;sets VRAM address to sprite graphics
	
	ld a,(genspr+9)
	and 2
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
		call MulA48 ;Another one that is very heavy as you run this thing 8 times
		ld hl,(genspr+7);Graphical data address
		add hl,de
		ld a,(vdpmem) ;Gotta respect the MSX standard
		ld c,a
		ld b,$20
		call fotir

		ld a,(genspr+9)
		and 32
		jp z,notmultitileUF
			ld b,$20
			call fotir ;Copy the following block because it's a multicolor sprite
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
	xor 64
	ld (hl),a

	;The lack of ret here is purposeful
	;When you upload the sprite graphics, you need to upload the first color table as well, so this function is designed to "leak"
	;into the next function that does this job


ColorCopyMode:

	ld bc,(genspr+12) ;Fetch inisprite
	xor a
	rr b
	rr c ;divide by 2
	ld a,(genspr+9)
	and 12
	cp 12
	jp z,firstruncol ;First copy is always not reversed for some probably good reason
	cp 8
	jp z,reversetableUF
	jp secrunnorev

	firstruncol:
	ld a,(cursptable)
	cp 0
	jp nz,reversetableUF ;When on page 1, the sprite order is reversed so we can have sprite blinking
	secrunnorev:
	xor a ;Whoops, carry doing carry stuff
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

	ld a,(genspr+9)
	and 12
	cp 12
	jp nz,not_first_col_p2

	ld a,(cursptable+1)
	add a,8
	ld h,a
	add hl,bc ;Just dividing by half because 16 bytes instead of 32
	jp exit_sel_col_p2
	not_first_col_p2:

		ld a,(genspr+9)
		and 8
		cp 8
		jp nz,not_8_sel_col_p2
			ld h,$48
			add hl,bc
			jp exit_sel_col_p2
		not_8_sel_col_p2:
			ld h,$58
			add hl,bc
	exit_sel_col_p2: ;quite a few kludges here

	ld a,(genspr+9)
	and 128
	jp nz, VramColorTileMode ;Nice math!, do you mind if i just hijack the hell out of it if it's vram?

	;call 5

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
	call MulA48 ;and here it is again, the white devil
	ld hl,(genspr+7)
	add hl,de
	ld de,(genspr+5)
	add hl,de
	ld a,(vdpmem)
	ld c,a

	;otir ;Copy the following block because it's a multicolor sprite
	call fotir

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

	ld de,$0000
	;call dbgcolor ;line


ret

UpdateColorTable:

ret

somewhere: .db 0,0
VramSpriteMode:
	; genspr+5 Adress to the frame data,  genspr+7 Pointer to the data input 
	; genspr+2 number of tiles to copy, ;genspr+9 contains the sprite flag, with 32 pointing it's a multi color copy (and the frame data need to be doubled if it's the case)
	; cotablemode tell the system that it's a color copy instead
	; hl is pipping hot with the calculated vram target address

	;ld bc,$4000
	;;sbc hl,bc ;But it's a good idea to remove the write bit as this address will be used differently
	ld (somewhere),hl ;Gotta store it somewhere

	;notmultiaddrUFVRAM:
	ld d,32
	ld a,(genspr+9)
	and 32
	add d
	ld d,a
	ld a,(genspr+9)
	and 2
	ld a,0
	jp z, VramNoFlip
		ld a,d
	VramNoFlip:
;	ld (genspr + 20),a ;Math to precalculate the sprite flip

	ld hl,(genspr+7);Graphical data address 
	ld d,$10
	ld e,a
	add hl,de
	ld (genspr+20),hl;Graphical data address 


	VramTileLoop:

	;-Frame setup-
	ld hl,(genspr+5) ;Copypasting the Regular as this does what i need it to do (fetch the frame)
	ld b,(hl) ;Fetch frame number
	inc hl
	ld (genspr+5),hl ;next frame
	ld a,(genspr+9)
	;call 5
	and 32
	jp z,notmultiaddrUFVRAM2
		rl b
	notmultiaddrUFVRAM2:
	;end frame setup
	;-Frame pointer math-
	xor a
	ld a,b
	rrca
	rrca
	ld d,a
	and $c0
	ld e,a
	ld a,d
	and $3f
	ld d,a ;Multiply the fetched frame number by 64 to get the right pointer
	ld hl,(genspr+20);Modified graphical pointer address
	add hl,de ;And with this, we have the source
	;-Completed calculating the frame pointer-
	ld de,(somewhere)
	ld a,(genspr+9)
	and 32
	jp z,@multifuf
		ld a,64
		;HL source, DE destination, A count (this solution is just not very optimal at all lol)
		;iyl precalculated copy weight (again)
		ld iyl,4 ;(every scanline copy around 16 ish bytes, so 64 bytes)
		call AddBlitterAddressCopy			
		ld hl,(somewhere)
		ld de,64
		add hl,de
		ld (somewhere),hl
		ld hl,genspr + 2
		dec (hl)
		dec (hl)
		jp @toloop
	@multifuf:
	ld iyl,2 ;(And here it's just 32, so 32 it is)
	ld a,32
	call AddBlitterAddressCopy
	ld de,32
	add hl,de
	ld (somewhere),hl
	ld hl,genspr + 2
	dec (hl)
	@toloop:
	jp nz,VramTileLoop
jp UploadFrameExit1

VramColorTileMode:
	; genspr+5 Adress to nothe frame data,  genspr+7 Pointer to the data input 
	; genspr+2 number of tiles to copy, ;genspr+9 contains the sprite flag, with 32 pointing it's a multi color copy (and the frame data need to be doubled if it's the case)
	; cotablemode tell the system that it's a color copy instead
	; hl is pipping hot with the calculated vram target address
	ld (somewhere),hl ;Gotta store it somewhere as well

	vramUploadColorUF:
	;-Setup-
	ld hl,(genspr+14)
	ld c,(hl) ;Getting frame
	inc hl
	ld (genspr+14),hl

	ld a,(genspr+9)
	and 32
	jp z,vramnotmultiacolUF
		rl c
	vramnotmultiacolUF:
	;-End Setup-
	;Frame pointer math
	xor a
	ld a,c
	rrca
	rrca
	rrca
	rrca ;This is actually pretty fast, only 16 cycles
	ld e,a
	and $0f
	ld d,a
	ld a,e
	and $f0
	ld e,a ;Multiply by 16
	ld hl,(genspr + 24) ;Start of the sprite COLOR data
	add hl,de 
	;End ponter math

	ld de,(somewhere)	
	
	;ld a,(genspr + 20)
	;ld c,a
	;HL source, DE destination, A count (this solution is just not very optimal at all lol)
	;iyl precalculated copy weight (again)
	call 5
	ld a,(genspr+9)
	and 32
	jp z,@vramcol1
		ld iyl,2 
		ld a,32
		call AddBlitterAddressCopy
		ld hl,(somewhere)
		ld de,32
		add hl,de
		ld (somewhere),hl
		ld hl,genspr+11
		dec (hl)
		dec (hl)
		jp @loopend	
	@vramcol1:
		ld iyl,1 
		ld a,16
		call AddBlitterAddressCopy
		ld hl,(somewhere)
		ld de,16
		add hl,de
		ld (somewhere),hl
		ld hl,genspr+11
		dec (hl)
	@loopend:
	jp nz,vramUploadColorUF
	;End counter hell


	ld a,(cotablemode)
	cp 0
	ret nz ;if it's not a color table mode, this means it the first color copy from "function leak"	
	ld hl,(genspr)
	;call addSpritePageFlip

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

;A is the frame id, DE returns the multiplied by 80
MulA80:
	push hl
	add 0 ; just to clear carry flag
	rrca
	rrca
	ld h,a
	and $c0
	ld l,a
	ld a,h
	and $3f
	ld h,a ;Multiply A by 64

	ld d,h
	ld e,l
	xor a
	rr d
	rr e
	rr d
	rr e ; Divide by 80
	add hl,de ;Final result, A multiplied by 80 on HL
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
		add hl,bc ;Add the offset of the table itself to the offsets (that's pretty darn smart)
		ex de,hl
		ld hl,(spriteptstack)
		ld (hl),e
		inc hl
		ld (hl),d ;And writes it to the table
		inc hl
		ld (spriteptstack),hl ;but for speed stuff up, i'm also building a pointer to the graphics
		ex de,hl
		inc hl
		;inc hl ;New format don't have subtype
		ld a,(hl) ;Size
		inc hl
		;inc hl ;Skip the old Y
		ld d,(hl) ;Numbr of frames
		push hl
		ld hl,sizetable ;Unlike the last engine, only need to get the sizes from the table
		and 1 ;Remove the multicolor flag, as this is about the animation frame table
		add a,l
		ld l,a
		ld a,h
		adc a,0
		ld h,a ;Kludge to solve the table offset problem
		ld a,d ;Save number of frames in A as required
		ld e,(hl) 
		ld d,0 ;And DE have to have the number of tiles of the sprite
		pop hl
		mulasp2:
			add hl,de
			dec a
		jp nz,mulasp2 ;Multiply total tile count by the number of frames
		;ld de,3 ;just skipping the rest of the header
		;add hl,de
		inc hl ;
		inc hl ;New format use only 4 bytes, so a couple of incs will do

		ex de,hl
		ld hl,(spriteptstack)
		ld (hl),e
		inc hl
		ld (hl),d
		inc hl
		
		ld de,4
		add hl,de ;Skipping stuff for now

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
	jp nz,Flippedsubr
ret


;E number of sprites to be subtracted
RemoveSpritePointers:
	rlc E
	ld d,0
	ld hl,(spriteptstack)
	sbc hl,de
	ld (spriteptstack),hl
ret


;L Type,DE X,Y,B Flip X,Y ,C Frame;
;A returns The Object ID
CreateSprite:
	xor a
	ld (genspr),hl
	ld (genspr+2),bc
	ld (genspr+4),de
	rl l
	rl l
	rl l ;I'm adding a pointer to the colors as well
	ld h,$71
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl
	ex de,hl ;This SHOULD make the code below compatible
	foundTypeCS:
	dec de
	dec de
	ld (genspr),de ;Saving the address of the pointer so i can put it on the last thingamabob later
	;dec hl
	ld de,genspr+8
	ld bc,4
	ldir ;Copying the sprite data because easier to deal
	;0 type, 1 size + multicolor, 2 frame count, 3 maximum tiles
	;push hl
	ld a,(genspr+8+1) ;Sprite Configuration
	ld d,a 
	ld hl,sizetable
	;ld a,(genspr+8+1) ;Getting actual size
	add a,l
	ld l,a
	ld a,h
	adc a,0
	ld h,a
	ld e,(hl) ; of tableSize
	;pop hl ;Probably completely useless as HL just gets wasted by next instruction

	ld hl,$70E0;Time to find a free slot on the actual sprite table
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
		ld a,e ;note how total number of sprites is used here
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
			ld a,l
			ld (genspr+30),a ;saving the sprite data address
			ld a,c
			ld (genspr+15),a
			;ld b,5 ; used+update sprite (no subcolor)
			ld b,13 ; used + initial copy stage 
		
			;Now this HL contains the sprite address
			ld a,(genspr+8+1) ;Fetching singlecolor
			and 2
			cp 0
			jp z,NotMulticolorCS
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
			ld a,l
			ld (genspr+16),a ;Save the sprite ini position for later on
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
			ld (hl),d ;Sprite configuration
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
			
			ld a,(genspr+16)
			add a,$8
			ld b,a
			ld a,(maxsprite)
			cp b
			jp nc,notbiggerCS
			ld a,b
			ld (maxsprite),a
			notbiggerCS:
			xor a
			ld a,(genspr+15)
			rlca
			rlca
			rlca

ret
NoSlotsCS:
ld a,255
ret

;C Object ID, DE X,Y
UpdateSprite:
	push hl
	ld a,e
	cp $d8+16 ;Stop sprite from ever getting to 208, where it disable all the sprites
	jp nz,noclipdue208
	ld de,$e0e0
	noclipdue208:
;	xor a
	ld a,c
	ld h,$70
	inc a
	ld l,a
	ld (hl),d
	inc l
	ld (hl),e
	pop hl
ret

;C Object ID, DE Screen/X B, Y
UpdateSpriteGlobal:
	push hl
	ex de,hl
	ld de,(CamX)
	sbc hl,de; position - camera = 
	ld a,l
	cp $f8
	jp c,noclipsprite
		ld hl,$e0e0
	noclipsprite:
	
	ld d,l ;X transformed
	ld e,b ;Y pass
	ld a,h
	cp 0
	jp z,onscreen
	ld de,$e0e0
	onscreen:
	call UpdateSprite
	pop hl
ret


;C Object ID, DE X,Y
GetSpritePosition:
	push hl ;Just preserving HL 
	xor a ;Clears carry flag because multiplier down there will include it
	ld a,c ;Load object id to A
;	rlca
;	rlca
;	rlca ;Multiply by 8 because table is 8 bytes long
	ld h,$70 ;Table is at $7000, so set the high byte to $70
	inc a ;Second byte of the table
	ld l,a
	ld d,(hl) ;Load X to D
	inc hl ;next memory address
	ld e,(hl) ;Load Y to E
	pop hl ; HL is now back to the old value
ret

;C Object ID,DE frame,flip
UpdateSpriteFrame:
	push hl
	ld a,c
	ld (genspr + 30),a
	ld h,$70
	ld l,a
	ld a,(hl)
	and $ff-12 ;Just to avoid some terrible mistakes above
	or 12 ;New system
	and $ef ;Remove flip in case of not flip
	ld (hl),a
	ld a,(hl)
	and $ff-2 
	ld (hl),a
	ld a,0
	cp e
	jp z,notflipUSF
		ld a,(hl)
		or 2
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
	ld a,4
	ld (bakect),a
	push hl
	push de
	ld a,c
;	rlca
;	rlca
;	rlca ;Multiply by 8
	ld h,$70
	ld l,a ;Calculate the adress to the sprite
	ld (genspr+16),a
	ld a,(hl)
	cp 0
	jp z,noworkrs ;The sprite is already removed
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
	
	ld a,(genspr+16)
	ld b,a
	ld a,(maxsprite)
	sub $8
	cp b
	jp nz,noworkrs
	ld (maxsprite),a
	ld a,1
	ld (gc),a
	noworkrs:
	ld a,2 ;Sprite table size got shrunk
	ld (clearspritetable),a ;so better tell the sprite system to purge the table, as it's no longer automatic
	pop de
	pop hl
ret

addSpritesLocal:
	ld de,(spriteptstack)
	ldir
	ld (spriteptstack),de

ret

;C Object ID
SetAsVramSprite:
	push hl
	ld a,c
;	rlca
;	rlca
;	rlca ;Multiply by 8
	ld h,$70
	ld l,a ;Get current sprite
	ld a,(hl)
	or 128
	ld (hl),a
	pop hl
ret
