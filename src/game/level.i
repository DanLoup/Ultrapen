;Level format (2.0)
; 32 bytes (pointers to all the data)
; 8400 Graphical data pointer
; 8402 Block map pointer
; 8404 hall pointer
; 8406 palette pointer
; 8408 sprite data
; 840A Collision data
; 2048 bytes 1bit tiles 8420 
; 2048 bytes color tables 8C20
; 512 bytes block map 9420
; 512 bytes big blockmap (5121 bytes until now) 9620
; 1 byte number of halls 9820
; per hall: (starts at 9821) 
; 1 byte number of screens
; 64 bytes screen map (big block)

;Video map
;0000-1800 chars (800 per bank)
;1800-1AFF tile ids
;2000-37ff charcolors
;(this video map is woefully innacuate)
;I bet on the REAL video banks being at 4000-4800

;I will unpack the blocks to $5000-$5800 as well MSX has quite a lot of memory if compared to an NES and will speed up code
;At $5800-$5A00 i will store the block config
; i think i will put the dual screen tilemap at $6000
; $4f00 = jump list to each tile

;The basic working of the level drawing/scrolling engine is that it keeps an 64x32 tile map on the main memory (2 screens)
;and then copy a section of this tile map into the VRAM, this way being able to smoothly scroll the screen in tile fine jumps
;This also allow the engine to not have to redraw the whole tile map while scrolling, only a 4x32 tile column on the direction the camera is panning
;And to keep everything smooth, the engine only draw a 4x4 tile block to the dual screen map per frame, and some lines of tile from the ram to vram
;And finally, the VRAM itself have two tile pages, so the line drawing is hidden from the player

;Level Shared vars
CamX: .ds 2,0 ; Global camera control
OCamX: .ds 2,0 ;Old Camera X(Actually used for the calculations)
CamDir: .db 1 ;Indicates the direction the camera is panning to (0 left 1 stopped 2 right)
CamODir: .db 1 ;Indicates the direction the camera is panning to (0 left 1 stopped 2 right)
CurHal: .db $03,$98 ;Hall start adress
CurHalnb: .db $0 ;Used by the reset routine to determine the new screen
Hallsiz: .db 0 ;Number of screens (can thankfully use that on scroll high pointer as each screen has 256 pixels)
Halladdr: .db 0,0 ;Memory adress for the current hall
;Internals
curscaddr: .dw 0 ;Current writing address
dpkcts: .ds 2,0 ;Depacker 32x32 block counter and smallblock counter
bigaddr: .ds 2,0 ;Depacker main pointer
CamDX: .db 0 ;Camera position, but in blocks
CamDbl: .db 0 ;Indicates on which vram page to draw and which to draw to screen
CamOfftile: .db 0 ;tile scroll offset
CamBuffPos: .db 0 ;offset from 0-63 for the dual screen
CamObuffpos: .db 50 ; Old offset (used to reset the frame copy)
CamTlPos: .ds 2,0 ;Offset in the 32 block range
Offsetval: .db 0 ;Save the per tile scroll position to help keep the correct scroll when the frame takes too long
Leftcfg: .db 0,12,34 ; First byte, 0-3 on how many blocks are needed to update, second and third = match with Tlpos to say that is not needed to update anymore
Rightcfg: .db 0,12,34 ; Same thing, but to the right side
BuffLcount: .db 0 ;Counter of the number of lines transferred to the buffer
Predictval: .db 0 ;Kludge to fix an intermitent bug by just checking if the correct position is in the tile buffer during the switch
holdframe: .db 0 ;Used to avoid writing the buffer for a frame when performing a quite heavy operation to save up on some CPU time
vpage: .db 0 ;Actually points out which page is being written to and which one is being displayed
eholdframe: .db 0 ; Same thing as holdframe, but won't be resetted by the own code
flipcorridor: .db 0 ;Indicates that the current corridor starts on the middle of the dual screen, rather than on beginning
pause: .db 0

;Routine to mostly fill the VRAM with the tile data and unpack the 32x32 blocks for faster tile placing (horray for 64KB of memory)
LoadLevel:
	ld c,$99
	ld de,$008e ;First first thing, set page to zero
	out (c),d
	out (c),e
	;Second thing, add the data offset to the pointers
	ld hl,$8401
	ld c,$10
	pointeradder:
		ld a,(hl)
		add a,$84
		ld (hl),a
		inc hl
		inc hl
		dec c
	jp nz,pointeradder

	ld c,$99 ;Load the 1 bit bitmap data to the 3 tile maps 
	ld hl,$4000 
	ld e,4
	LLoop1:
		push hl
		out (c),l
		out (c),h ;the bit of $40 tells the VRAM it's a write
		ld hl,($8400)
		ld d,8
	l1:
			ld bc, $0098
			otir ;This Z80 instruction just rocks, b for number of loops, c for register to output, HL as where the data is read from
			dec d
		jp nz, l1
		pop hl
		ld bc,$800 
		add hl,bc ;The VDP9918 have 4 separate tile maps for each quarter of the screen, but i just repeat em because it would be quite messy to perform vertical scroll otherwise
		dec e
	jp nz,LLoop1
	
	ld c,$99 ;Same thing, but for the Color map thing
	ld hl,$6000
	ld e,4
	
	LLoop2:
		push hl
		out (c),l
		out (c),h
		push de
		ld hl,($8400)
		ld de,$800
		add hl,de
		pop de

		ld d,8
	l2:
			ld bc, $0098
			otir
			dec d
		jp nz, l2
		pop hl
		ld bc,$800
		add hl,bc
		dec e
	jp nz,LLoop2


	ld bc,$0
	ld (dpkcts),bc
	ld bc,$6800
	tiledec: ;Depacker of the 32x32 tiles, that writes to $6800
		ld a,(dpkcts)
		rlca
		rlca 
		and $fc
		ld e,a
		ld a,(dpkcts)
		rlca
		rlca
		and $3
		ld d,a 
		ld hl,($8402) ;Big block pointer
		add hl,de ; Calculates (with a 4 multiplication) the address of the current 32x32 block
		ld de,$200
		add hl,de ;But its the big blocks, not small
		ld (bigaddr),hl
		xor a
		ld (dpkcts+1),a ;Zeroes the block counter (and clears the carry for good measure)
		stiledec:
			ld a,(dpkcts+1)
			and $08 ;Get bigger block position y by getting the Y coordinate
			rra
			rra ;Divide it by 2 so it is on big block space (0,1,2,3)
			ld e,a
			ld a,(dpkcts+1) ;This one gets the X coordinate in big block space
			and $2
			rra 
			add a,e ;And with this, we have the (0,1,2,3) correctly assembled
			
			ld hl,(bigaddr) 
			add a,l 
			ld l,a 
			
			ld a,(hl) ;This gives the correct 16x16 tile address and this gets the correct tile data
			rlca
			rlca 
			ld d,a ;Save original data
			and $fc ;Bits 1 and 2 are the wrap around
			ld e,a
			ld a,d
			and $03
			ld d,a ;See? they're here
			ld hl,($8402) ;Small block pointer
			add hl,de ;And with the power of the 16bit add, we get to the actual correct adress
			ld a,(dpkcts+1) 
			and $04
			rra
			ld e,a
			ld a,(dpkcts+1)
			and $01
			add a,e ;All this 8bit pointer math probably will bite me in the ass later on, so better recheck here if more tile shit happens
			add a,l
			ld l,a
			ld a,(hl) ; And after all this pointer math, we have the blocks at 5000-5800 (or broken stuff, as it is at the moment)
			ld (bc),a
			inc bc
		ld a,(dpkcts+1)
		inc a
		ld (dpkcts+1),a
			cp $10
		jp nz,stiledec
	ld a,(dpkcts)
	inc a
	ld (dpkcts),a
	cp $80 ;128 blocks
	jp nz,tiledec


	ld c,$99 ;Load the 1 bit bitmap data to the 3 tile maps 
	ld hl,$1082 ;Puts map just outside the 16kb range
	out (c),h
	out (c),l
	ld hl,$018e ;Changes the VDP access page to page 1 (or $4000)
	out (c),h
	out (c),l
	ld a,0
	ld b,7
	ld hl,($8408)
	ld c,(hl) ;Number of sprites
	ld a,0
	cp c
	jp z,noobjects
	inc hl ;Jump tables to the sprites
	call AddSpritePointers
	noobjects:
	ld hl,($840A)
	ld b,0
	ld a,(hl) ;Number of programs
	add a,a ;Multiply by 2 because the function is a bit dumb
	ld c,a
	ld a,0
	cp c
	inc hl
	jp z,noprograms
	call AddPrgPointers ; This looks eerily similar to the sprite jump table, probably because it's the same idea and Z80 instruction
	inc hl

	noprograms:
	ld a,0
	ld (CurHalnb),a
	ld b,1
	call ResetScreen
	;call 5
	ld d,0
	ld ixl,0 
	call SpawnScreenPrgs
	ld a,0
	call SetLevelPal
ret 

;This function draw a block to the dual screen tilemap
;BC X,Y, D Block ID
DrawBlock:
	push de
	push bc
	push hl
	push de
	ld (coldata),bc ;Saving state of the input
	ld (coldata+2),de ;More stealing is made here
	
	ld hl,drmultiplier ;Read the palette created above
	ld a,c
	rlca 
	rlca ;It's actually a multiply by 64, so
	ld (hl),a
	ld a,0
	rld ;The magical multiply 16bits multiply by 16 (what i need here)
	ld d,a
	ld a,(hl)
	add a,b
	ld e,a ;Finally add X to it
	ld hl,$6000
	add hl,de

	ld (drawrite),hl ;Output address calculated
	pop de

	ld hl,drmultiplier
	ld a,d
	ld (hl),a
	ld a,0
	rld ;The blocks are on offset of 16, so multiply by 16
	ld d,a
	ld e,(hl)
	ld hl,$6800
	add hl,de
	ld de,(drawrite) ;As the name suggests, DE contains the write adress, HL has the read adress 
	ldi ;Optimization ahoy
	ldi
	ldi
	ldi
	ld bc,60 ;This pushes the write head to the next line, 4 tiles to the left
	ex de,hl
	add hl,bc
	ex de,hl
	ldi
	ldi
	ldi
	ldi
	ld bc,60 ;Why looping when we can unroll to get a prety darn quick 886 cycle per block thing?
	ex de,hl
	add hl,bc
	ex de,hl
	ldi
	ldi
	ldi
	ldi
	ld bc,60
	ex de,hl
	add hl,bc
	ex de,hl
	ldi
	ldi
	ldi
	ldi


	ld a,(coldata+3); hello block type
	add 0
	rlca
	rlca ;Multiplied by 4 now
	ld hl,($8402) ; A big block map appears!
	ld de,$200
	add hl,de ;I saved it as small - big for some reason
	add l
	ld l,a ;And now we're pointing at the correct big block, that is also collision stuff
	ld (drmultiplier),hl

	
	ld de,(coldata) ;Okay, now it's the raw data again
	rr e ;Half height	
	ld a,e ;Height first
	rrca
	rrca
	rrca ;Wraps A around 
	ld h,a
	and $e0
	ld l,a 
	ld a,h
	and $1f
	ld h,a ;HL is now y * 32
	ld e,d
	rr e ;Divide X by 2
	ld d,0
	add hl,de ;And with this, we have X/2 + y * 32
	ld de,$7700 ;That's a pretty good place to add a collision table i think?
	add hl,de ;At the table instead of corrupting the program

	push hl
	pop bc ;This will be slow as hell heh

	ld hl,(drmultiplier)
	ld a,(hl)
	exx
	ld e,a
	ld d,4
	ld hl,($8402) 
	add hl,de 
	ld a,(hl) ;fetch collision address
	exx
	ld (bc),a ;fill it in
	inc c 

	inc hl
	ld a,(hl)
	exx
	ld e,a
	ld d,4
	ld hl,($8402) 
	add hl,de 
	ld a,(hl) ;fetch collision address
	exx
	ld (bc),a ;fill it in

	ld a,31
	add a,c
	ld c,a

	inc hl
	ld a,(hl)
	exx
	ld e,a
	ld d,4
	ld hl,($8402) 
	add hl,de 
	ld a,(hl) ;fetch collision address
	exx
	ld (bc),a ;fill it in
	inc c

	inc hl
	ld a,(hl)
	exx
	ld e,a
	ld d,4
	ld hl,($8402) 
	add hl,de 
	ld a,(hl) ;fetch collision address
	exx
	ld (bc),a ;fill it in

	pop hl
	pop bc
	pop de
ret
drmultiplier: .ds 2,0 ;Generic variable used by all the rld operations
drawrite: .ds 2,0 ;Variable used to store calculated adresses used by drawblock
fbladdr: .ds 2,0 ;Store the current screen position
fboverd: .db 0 ;if fb>32 then this tells the wrap to be simulated
coldata: .ds 4,0

;Routine that checks for the direction the camera is panning and place the 4x4 tiles accordingly
;And also the routine that upload the lines of tile to the VRAM

UpdateHall:
	ld hl,$018e ;Sets the current page of the video write
	out (c),h
	out (c),l
	ld a,(CamX)
	and $7
	ld (sc_offs),a
	
	ld hl,(OCamX)
	;CamX is in pixels, so i need to divide it by 8 and get just the first 6 bits
	ld a,l
	and $7
	ld (CamOfftile),a ;tile scroll offset
	xor a
	ld a,l
	rra
	rra
	rra ;Divide by 8 (because its on a tile base)
	and 31 ;Filters out the carry stuff
	ld d,a ;saves partial
	ld a,(flipcorridor)
	ld e,a
	ld a,h
	and $1
	xor e ;Add corridor flip
	cp $1
	ld a,0
	jp nz,nothighbit
	ld a,32
	nothighbit:
	add a,d

	ld (CamBuffPos),a ;Position on the dual screen
	ld hl,(OCamX)
		xor a
		rr h
		rr l
		xor a
		rr h
		rr l ;Divide it only by 4
		ld a,l
		and $f8 ;And remove the first 3 bits, so i get the value * 8, which is the ideal for the tile address below
		ld l,a
	ld (CamTlPos),hl ;Tile adress

	ld a,(CamObuffpos)
	ld b,a
	ld a,(CamBuffPos)

	cp b
	jp z,noupdatescreen ;Compare the current position with the last, to determine if its time to update screen
	ld b,a
	ld a,(Predictval) 
	cp b
	jp z,correctprediction ;Sometimes we got the wrong predicted frame on the next fb, so need to correct it
	ld a,(CamBuffPos) 
	ld b,a 
	ld c,2
	ld de,0
	call SetCurrentLine
	ld a,0
	ld (BuffLcount),a ;As in the other cases reset the frame buffer copier
	correctprediction:
	ld a,(BuffLcount)
	cp $4
	jp z,allinesdone
	;Contrary to the regular operation, this should be drawn on the page already on screen to avoid delaying the flip, causing a possible tearing
	ld b,a
	call FlipVpage
	ld a,(CamDbl)
	xor 1
	ld (CamDbl),a
	
	domissinglines: ;Sometimes we might not have the whole frame on the buffer, or a wrong predict so this fixes it
		ld d,6
		call DrawLines
		ld a,(BuffLcount)
		inc a
		ld (BuffLcount),a
		cp $4
	jp nz,domissinglines
	ld a,1
	ld (holdframe),a ;tell the frame buffer copier to not act in this frame, just to smooth out a bit the load
	ld (eholdframe),a
	allinesdone:
	ld a,(CamDir)
	sub 1
	ld b,a
	ld a,(CamBuffPos)
	add a,b
	and 63 ; This math calculates the NEXT position instead of the current, otherwise buffer would have the last frame on it on the flip
	ld (Predictval),a 
	ld b,a
	ld c,2
	ld de,0 
	call FlipVpage
	call SetCurrentLine ;bc mainram x,y de vram x,y
	ld a,0
	ld (BuffLcount),a
	noupdatescreen:
	
	ld a,(CamDir) ;Subroutine to reset the screen copying in case the player changes direction (nullfying the buffer)
	ld b,a
	ld a,(CamODir)
	cp b
	jp z,noredoline
		ld a,(CamDir)
		sub 1
		ld b,a
		ld a,(CamBuffPos)
		add a,b
		and 63 ;As with above, tries to aim for the next position
		ld (Predictval),a
		ld b,a
		ld c,2
		ld de,0 
		call SetCurrentLine ;bc mainram x,y de vram x,y
		ld a,0
		ld (BuffLcount),a
	noredoline:
	
	
	ld a,0
	ld a,(holdframe)
	cp $0
	jp nz,screenisdrawn 
	ld a,(BuffLcount)
	cp $4
	jp z,screenisdrawn
		inc a
		ld (BuffLcount),a
		ld d,6
		call DrawLines ;Regular copy
	screenisdrawn:
	
	
	ld a,0
	ld (holdframe),a ;Clears the holder so the frame buffer copier can resume on the next frame
	ld a,(CamBuffPos)
	ld (CamObuffpos),a
	ld a,(CamDir)
	ld (CamODir),a ;Old variable updaters

	ld a,(CamDir)
	cp $2 ;If going to right direction
	jp nz,noresetright
	ld hl,(Rightcfg+1)
	ld de,(CamTlPos) 
	xor a
	sbc hl,de ;16 bit value compare
	jp z,noresetright ;To determine if we're on the next block row
		ld (Rightcfg+1),de
		ld a,0
		ld (Rightcfg),a ;Resets the block updating counter (in tiles)
	noresetright:
	ld a,(Rightcfg)
	cp 28
	jp nc,norightf
	Call rollright ;Call the routine that draws the 32x32 blocks on the memory bitmap thing
	norightf:

	ld a,(CamDir) ;Same routine above, but to the left scroll
	cp $0
	jp nz,noresetleft
	ld hl,(Leftcfg+1)
	ld de,(CamTlPos) 
	xor a
	sbc hl,de
	jp z,noresetleft
		ld (Leftcfg+1),de
		ld a,0
		ld (Leftcfg),a
	noresetleft:
	ld a,(Leftcfg)
	cp 28
	jp nc,noleftf
	Call rollleft
	noleftf:
	ld hl,$008e ;Sets back to page 1
	out (c),h
	out (c),l
	ld hl,(CamX)
	ld de,(OCamX)
	sbc hl,de
	ld (globalvars+2),hl
	ld hl,(CamX)
	ld (OCamX),hl
ret



;Function to setup where the lines of tile will be copied from and to (also abused to do the pan and big enemy effects)
;bc mainram x,y de vram x,y
SetCurrentLine: 
	push bc
	push de
	push hl
	push de ;Tricky thing to save a config value for later
	ld a,b
	sub 32
	jp nc,SCLOver
	ld a,0
	SCLOver:
	ld (fboverd),a ;If the screen scroll is over 31, this means we need to wrap around to the first page
	ld hl,drmultiplier 
	ld a,c
	rlca 
	rlca ;It's actually a multiply by 64, so
	ld (hl),a
	ld a,0
	rld ;The magical multiply 16bits multiply by 16 (what i need here)
	ld d,a
	ld a,(hl)
	add a,b
	ld e,a ;Finally add X to it
	ld hl,$6000
	add hl,de ;This is how we calculate a fb position, so just copypasted from above
	ld (fbladdr),hl
	
	pop bc ; see?
	ld hl,drmultiplier
	ld a,c
	rlca ;Multiply by 32 this time as screen position
	ld (hl),a
	ld a,0
	rld ;gotta love this decimal thing
	ld d,a
	ld a,(hl)
	add a,b
	ld e,a ;And again, x is added
	ld hl,$4000
	ld a,(CamDbl)
	cp 0
	jp z,sclbuff1
	ld hl,$4400
	sclbuff1:
	add hl,de ;This is how we calculate a fb position, so just copypasted from above
	ld c,$99
	out (c),l
	out (c),h
	ld (curscaddr),hl
	pop hl
	pop de
	pop bc
ret

; This is the function that actually copy the lines from ram to VRAM
; It supports wrap around so the dual screen scheme can work
; (i should put an wrap around disabler for the big boss mode)

; d number of lines,e disable mirror draw
disline: .db 0 ;Ugly but no better idea for now 
DrawLines:
	push bc
	push de
	push hl
	ld a,d
	cp 0
	jp z,nodrawlines
	ld c,$99
	ld hl,(curscaddr)
	out (c),l
	out (c),h
	ld hl,(fbladdr) ;Load current memory bitmap adress
	ld a,(fboverd) ;Overflow counter (for wrapping around purposes)
	cp $0
	jp z,notawrap
	wraploopdl:
	ld a,(fboverd)
	ld b,a
	ld a,$20
	sub b
	ld b,a
	ld c,$98 ;C is the port,B is the number of bytes and HL is the source
	otir ;Original 
	ld a,(disline)
	cp 1
	jp z,diswrap
	xor a
	ld bc,$0040
	sbc hl,bc
	ld a,(fboverd)
	ld b,a
	ld c,$98
	otir ;Wrap
	diswrap:
	ld bc,$60
	add hl,bc

	exx
	ld hl,(curscaddr)
	ld de,$20
	add hl,de
	ld (curscaddr),hl
	exx

	dec d
	jp nz,wraploopdl
	jp endblld
	notawrap:
	ld bc,$2098
	call fotir
	ld bc,$20
	add hl,bc

	push hl
	ld hl,(curscaddr)
	add hl,bc
	ld (curscaddr),hl
	pop hl

	dec d
	jp nz,notawrap
	
	endblld:
	ld (fbladdr),hl

	nodrawlines:
	pop hl
	pop de
	pop bc
ret

;Those functions are called by UpdateHall to do the actual job of drawing the 4x4 tiles on left/right of where the screen is
rollright:
		ld hl,(CurHal)
		ld de,(CamTlPos)
		add hl,de
		ld de,72
		add hl,de ;Offset
		ld a,(CamBuffPos)
		and $fc
		add a,36
		and 63
		ld b,a ;this b will have to be added to make the right stuff
		xor a
		ld a,(Rightcfg)
		ld c,a
		rra
		rra ;the Rightcfg runs on tile scale, so divide by 4 to get hl scale
		ld d,0
		ld e,a
		add hl,de
		ld a,(hl)
		ld d,a ;Read the first block
		call DrawBlock ;BC X,Y, D Block ID
		inc hl
		ld a,(hl)
		ld d,a
		xor a
		ld a,c
		add a,4
		ld c,a
		call DrawBlock ;BC X,Y, D Block ID
		xor a
		ld a,(Rightcfg)
		add a,8
		ld (Rightcfg),a
ret

rollleft:
		ld hl,(CurHal)
		ld de,(CamTlPos)
		add hl,de
		ld de,8
		xor a
		sbc hl,de ;Offset
		xor a
		ld a,(CamBuffPos)
		and $fc
		sbc a,4 ;it's the tile behind
		and 63
		ld b,a ;this b will have to be added to make the right stuff
		xor a
		ld a,(Leftcfg)
		ld c,a
		rra
		rra ;the Leftcfg runs on tile scale, so divide by 4 to get hl scale
		ld d,0
		ld e,a
		add hl,de
		ld a,(hl)
		ld d,a ;Read the first block
		call DrawBlock ;BC X,Y, D Block ID
		inc hl
		ld a,(hl)
		ld d,a
		xor a
		ld a,c
		add a,4
		ld c,a
		call DrawBlock ;BC X,Y, D Block ID
		xor a
		ld a,(Leftcfg)
		add a,8
		ld (Leftcfg),a
ret

dtbsc: .db 0,0

;This function "instantly" fills a whole screen of the dual screen buffer for purposes such as booting the game or ending a pan
;b screen, c page (set to $20 for page 2) , ixh is the number of lines
;d hall id
DrawToBuffer:
	push hl
	push bc
	push de
	ld a,ixh
	ld (dtbsc + 1),a

	ld a,c
	ld (dtbsc),a
	xor a
	ld a,d
	rla
	rla
	rla ;multiply by 8
	
	ld hl,($8406)
	ld d,0
	ld e,a
	add hl,de 
	inc hl ;Adress to the correct hall of the index table
	ld a,(hl) ;Just making a clamper on the screen number
	inc hl
	ld e,(hl)
	inc hl
	ld d,(hl)
	dec a
	cp b
	jp nc,noclamp
	ld b,a ;Clamp the value in case of too high
	noclamp:
	ld hl,$8400
	add hl,de ;Adress to the actual screens of the selected hall

	ld a,b
	cp 0
	jp z,Dtbzero
	ld de,64
	Dtbloop:
		add hl,de
		dec b
		jp nz,Dtbloop
	Dtbzero:
	;Now i have the HL at the start of the screen
	ld bc,0
	ld a,(dtbsc)
	ld b,a
	ld a,(dtbsc + 1)
	add b
	ld e,a ;Fills the counter, and use e as a comparator for choosing screen 0 or 1
	dloop1:
		ld a,(hl)
		inc hl
		ld d,a
		call DrawBlock ;BC X,Y, D Block ID
		ld a,c
		add a,4
		ld c,a
		cp $1f
		jp c,dloop1

		ld c,0
		ld a,b
		add a,4
		ld b,a
		cp e
	jp c,dloop1
	eend:
	pop de
	pop bc
	pop hl
ret

;This dual function thing is what changes the VRAM page the line copier will copy to and the actual MSX tile data adress
;Reload only make it rewrite the MSX video address
FlipVpage:
	ld a,(CamDbl)
	xor 1
	ld (CamDbl),a ;Flips page
	xor 1
	add a,$10
	ld (vpage),a
ReloadVpage:
	ld a,(vpage)
	out ($99),a
	ld a,$82
	out ($99),a ;And flips the memory address, thus giving me a nice double buffer
	ld a,$03
	out ($99),a
	ld a,$84
	out ($99),a ;Set back
ret

;This function only keeps the camera in boundaries to stop hell from happening
HandleLevel:
	ld hl,(CamX) ;Stops camX from going below 0
	ld a,$ff
	cp h
	jp nz,nozerocam
		ld hl,0
		ld (CamX),hl
	nozerocam:
	ld a,(Hallsiz)
	dec a
	cp h
	jp nz,noresetlow
		ld l,$0
	noresetlow:
	ld (CamX),hl

	;Handle dark color
	ld hl,(palette+9*2)
	ld a,(fadect)
	add 3
	cp 7
	jp c,@noccap
		ld a,7
	@noccap:
	ld b,a
	call ShadeColor
	ld (pendarkcol),hl


ret

Pandir: .db 0
Panscroll: .db 0 ;Current counter
Panscrolltl: .db 0 ;Scroll converted in tile
Panscrolloff: .db 0 ;screen pan of the scroll
Panscrolltlold: .db 0 ;Used to make the code only update the tilemap if the coarse scroll changed
Pancount: .db 0
PanDpage: .db 0

HandlePan:
ret

resetstage: .db 0
resetsavest: .ds 4,0
resetsc: .db 0
resetrew: .db 0
;A points out to which screen we're going from the current hall
;if B is 1, tells to rewrite the current screen
ResetScreen:
	ld (resetsc),a
	ld a,0
	ld (resetstage),a
	ld a,2
	ld (gamemode),a
	ld a,b
	ld (resetrew),a
	;call clearbars
ret 

HandleResetScreen:
	ld hl,(sc_offs)
	call UpdateScroll
	call RunSpriteSystem
	ld a,(resetstage)
	inc a
	ld (resetstage),a
	dec a
	cp 0
	jp z,eres_st1
	cp 1
	jp z,eres_st2
	cp 2
	jp z,eres_st3
	cp 3
	jp z,eres_st4

ret
eres_st1:
	;call DestroyObjects
	ld a,0
	ld (sc_offs+1),a
	ld (sc_offs),a
	ld (CamX),a

	ld a,(CurHalnb)
	rla
	rla 
	rla ;multiply by 8 because new table
	ld hl,($8406)
	ld d,0
	ld e,a
	add hl,de 
	inc hl ;Jump over offset of number of halls
	ld a,(hl) ;Saving the number of screens
	ld (Hallsiz),a
	inc hl ;Points at screen adress
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl
	push hl
	ld hl,$8400
	add hl,de
	ld (CurHal),hl
	pop hl
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl
	ld hl,$8400
	add hl,de
	ld (Halladdr),hl

	ld hl,pencamlock
	inc (hl) ;Set pencamlock to 1
	ld a,(resetsc)
	ld b,a
	ld a,0
	cp b
	jp nz,notbegi 
		ld b,2 ;sets second page as 1
		ld a,(pencamlock)
		inc (hl) ;Set pencamlock to 2
	notbegi:
	dec b ;if the penguin is coming from the left, the pages should be setup as 0 and 1, otherwise max screen/max screen-1
	ld a,(Hallsiz)
	cp 0
	jp nz,notoneroom
		ld b,0
	notoneroom:
	ld c,$0
	ld a,b
	and 1
	jp z,evenpagepan ;The game *ALWAYS* organize the dual screen setup as odd/even/odd/even.. so when writing to there...
		ld c,$20
	evenpagepan:
	ld a,(CurHalnb) ;Writing reverse page
	ld d,a
	ld ixh,$20
	call DrawToBuffer	;b screen, c page (set to $20 for page 2),d hall id
	ld (resetsavest),bc
	ld (resetsavest+2),de
ret
eres_st2:
	ld bc,(resetsavest)
	ld de,(resetsavest+2)
	ld a,c
	xor $20
	ld c,a
	ld a,(resetsc)
	ld b,a
	ld ixh,$20
	call DrawToBuffer	;Writing current page
ret 

eres_st3:
	ld bc,0002
	ld a,(resetsc)
	and 1
	jp z,moreoddstuffep ;Remember, odd/even/odd/even, so when redrawing the main screen (needed because the pan sometimes leave things in a messy state)
		ld b,32 
	moreoddstuffep:
	ld de,0
	call SetCurrentLine ;bc mainram x,y de vram x,y
	ld d,27
	call DrawLines ;Redraw the whole screen to fix pan messes
	ld bc,$0002
	ld de,$0000
	call SetCurrentLine ;and reset the scan line rasterer
	call FlipVpage
ret
eres_st4:
	ld a,(resetrew)
	cp 1
	jp nz,nofillscreenreset
		call FlipVpage
	nofillscreenreset:
	ld a,(resetsc)
	ld (CamX+1),a
	ld (OCamX+1),a ;Sets camera to the page the penguin is landing on
	ld hl,(Halladdr)
	xor a ;Skipping exits
	ld a,(hl)
	rlca
	rlca ;Every exit have 4 bytes
	inc a ;And there's the exit count
	ld e,a
	ld d,0
	add hl,de ;And with this, i have a pointer to the object list
	ld a,0
	ld (gamemode),a
	;call SetupObjectList
	
	;ld d,0
	;call SpawnScreenPrgs

	;call SpawnScreen
ret


;HL X (yep,16bit), D y, Return A
GetTileType:
	push hl
	push bc
	push de

	ld a,(flipcorridor)
	xor h
	ld h,a

	ld a,l
	and $f0 ;Remove lower bits
	rr h
	rra
	rrca
	rrca
	rrca ;Divide by 16
	ld c,a ;And c is now the X coordinate, neat!
	ld b,0 ;For the fun sum later
	ld a,d
	and $f0 ; Remove the lower bits, as it's multiple of 16
	rla ;Multiply by 2, so it turns into a y * 32
	ld e,a
	ld a,0
	adc 0 ;stealing the carry generated by the roll
	ld d,a ;and now we have X/Y
	ld hl, $7700 ;Adress to the collision map
	add hl,de ;plus Y
	add hl,bc ;Plus X
	ld a, (hl) ; It's just this?
	
	pop de
	pop bc
	pop hl
ret 

;HL X (yep,16bit), D y, Return A
lvtmp: .db 0,0

;I'm pretty sure this have to happen in two frames
ResetLevel:
	ld a,(waypoint)
	ld (CurHalnb),a
	ld b,0
	ld a,0
	call ResetScreen ;A=next corridor B redraw screen
	ld a,0
	ld (fadedir),a
	call InitPen
ret
palette: .ds 32,0

;A, set the level palette
SetLevelPal:
	;Copying initial palette
	rrca
	rrca
	rrca ;Wraps around to multiply by 32
	ld d,a
	and $E0 ;Filter out the useless bits
	ld e,a
	ld a,d
	and $1
	ld d,a ;Just getting a basic 16 palette support in
	ld hl,($8404) 
	add hl,de
	ld de,palette
	ld c,32
	ldir
ret 

;A chooses the palette
;B choose the brightness intensity
SetPal:
	di
	ld hl,palette
	ld c,$99
	ld de,$0090
	out (c),d
	out (c),e
	xor a
	ld a,b
	rlca
	rlca
	rlca
	rlca ;Push contrast color to high bits
	ld c,a ;Not using C anymore so..
	ld e,$20
	palfill:
		ld a,(hl)
		and $0f
		sub b
		jp nc,zr1_pal
			ld a,0
		zr1_pal:
		ld d,a
		ld a,(hl)
		and $f0
		sub c
		jp nc,zr2_pal
			ld a,0
		zr2_pal:
		add a,d
	
		out ($9a),a
		inc hl
		dec e
	jp nz,palfill
	ei
ret
;B is brightness
;hl is color input and output
ShadeColor:

	xor a
	ld a,b
	rlca
	rlca
	rlca
	rlca ;Push contrast color to high bits
	ld c,a ;Not using C anymore so..
	ld ixl,2
	@looponce:
	ld a,h
	and $0f
	sub b
	jp nc,@zr1_pal
		ld a,0
	@zr1_pal:
	ld d,a
	ld a,h
	and $f0
	sub c
	jp nc,@zr2_pal
		ld a,0
	@zr2_pal:
	add a,d
	push af
	ld h,l
	dec ixl
	jp nz,@looponce
	pop af
	ld l,a
	pop af
	ld h,a
ret


fadect: .db 7
fadecct: .db 0
fadedir: .db 1
HandleFade:
	ld a,(fadedir)
	cp 1 
	jp z,fadeup

	ld a,(fadecct)
	inc a
	ld (fadecct),a
	cp 3
	jp nz,nosetpal
	ld a,(fadedir)
	cp 0
	jp nz,fadedown
		ld a,(fadect)
		dec a
		ld (fadect),a
		cp 0
		jp nz,noresetfaded
			ld a,1
			ld (fadedir),a
		noresetfaded:
		jp fadeup
	fadedown:
		ld a,(fadect)
		inc a
		ld (fadect),a
		cp 7
		jp nz,noresetfadeu
			ld a,1
			ld (fadedir),a
		noresetfadeu:
		jp fadeup

	fadeup:
	ld a,(fadect)
	ld b,a
	ld a,0
	call SetPal

	ld a,0
	ld (fadecct),a
nosetpal:
ret

ChangePaletteobj:
ret

ProgramPan:
	;call 5
	ld a,0
	ld c,16
	ld hl,PanParams
	@cleanparams:
	ld (hl),a
	inc hl
	dec c
	jp nz,@cleanparams ;Just reset everything to make the thing predictable

	ld a,$80
	cp b
	jp nz,@allocquestion
	ld a,$ff
	ret ;Please, do allocate me in all occasions, and don't run it without the parameters
	@allocquestion:
	
	ld a,(ix + 3)
	ld (PanParams),a ;Exit screen

	ld a,(ix + 5)
	ld (PanParams +3), a ;exit side
	ld a,(ix + 6)
	ld (PanParams +1), a ;next corridor
	ld a,(ix + 7)
	ld (PanParams +2), a ;next screen


	ld a,255
	ld (ix + 0),a ;Program ended
	;call 5
ret
;exit screen, next corridor, next screen, exit side

PanParams: .ds 16,0

RunPan:
ld a,(PanParams + 5)
cp 0
jp nz,@RunningPan
ld a,(l_setpan)
cp 0
ret z ; the penguin is not at any corners
ld b,a
ld a,(PanParams + 3) ;1 Right, 2 Left, 3 Down, 4 Up (after the inc)
inc a
cp b
ret nz ;The penguin is not at the right corner
ld a,(PanParams + 0)
ld b,a
ld a,(pen_x + 1)
cp b
ret nz ;The penguin its not on the right screen

ld a,1
ld (PanParams + 5),a ;the penguin did everything as expected, so it's time to run the pan
ld (pause),a ;Initiate a pause as well, with a 1 so you can't unpause the game when i support it

@RunningPan:

cp 1
jr nz,@nostage1
	;The pan was set
	;call 5
	ld b,0
	ld a,(pen_x + 1)
	inc a ; first page is page 1
	and 1
	ld b,a
	ld a,(flipcorridor)
	xor b ;is the last page odd?
	ld (flipcorridor),a ;If so, the next page is clearly flipped

	ld a,(PanParams + 2)
	ld b,a
	ld a,(PanParams + 1)
	ld d,a

	ld c,0
	ld a,$20
	ld (PanParams + 11),a
	ld a,(flipcorridor)
	cp 0
	jr z,@pagefla
		ld c,$20
		ld a,0
		ld (PanParams + 11),a
	@pagefla:
	ld ixh,$20
	call DrawToBuffer ;b screen, c page (set to $20 for page 2),d hall id

	ld a,2
	ld (PanParams + 5),a
	;call FlipVpage

	;ld a,1 ; TEST ADJUST 
	;ld (PanParams),a
	ld a,(PanParams + 3)
	ld (Pandir),a
	inc a
	ld (PanParams),a
	ld a,(PanParams + 2)
	ld (PanDpage),a

	ld de,16
	call UnloadPrograms
	ld a,0
	;ld (CamX),a
	;ld (CamX + 1),a
	;ld (sc_offs),a
	;ld (sc_offs+1),a
	
	ld a,32
	ld (PanParams + 13),a
	ld a,(PanParams)
	cp 2
	jr nz,@leftpanpopremove
		ld a,6
		ld (PanParams + 9),a
		ld a,33
		ld (PanParams + 13),a ; yep, we need the left pan to have an extra tile

	@leftpanpopremove:
	ret
@nostage1:
cp 2
jp nz,@nostage2
	ld a,(PanParams + 0)
	cp 3
	jp nc,@vertpan
	ld a,(PanParams + 6)
	cp 0
	jr nz,@notpanmath
		ld a,(PanParams + 7)
		cp 0
		call nz,FlipVpage
		ld a,(PanParams + 13)
		ld b,a
		ld a,(PanParams + 7)
		inc a
		cp b
		jp nz,@panend
			ld a,3
			ld (PanParams + 5),a
			ret
		@panend:
		ld (PanParams + 7),a
		ld b,a
		ld a,(flipcorridor) ;The pages are reversed, so better take that into account
		cp 0
		jp nz,@notrevpagehpan
			ld a,$20
			add b
			ld b,a
		@notrevpagehpan:
		ld a,(PanParams + 0)
		cp 2
		jr nz,@noleftpan
			ld a,64
			sub b
			ld b,a
		@noleftpan:
		ld c,2
		ld de,0
		call SetCurrentLine ;bc mainram x,y de vram x,y

	@notpanmath:

	ld a,(PanParams + 6)
	ld h,0
	ld l,a
	ld a,(PanParams)
	cp 2
	jr nz,@noleftpansub
		ld a,(PanParams + 7)
		cp 1
		jp z,@skipalot
		ld a,(PanParams + 9)
		ld l,a
	@noleftpansub:
	call UpdateScroll

	ld a,(PanParams + 9)
	sub 3
	ld (PanParams + 9),a
	@skipalot:
	ld a,(PanParams + 6)
	add 3
	cp 8
	jr c,@resetcth
		ld a,7
		ld (PanParams + 9),a
		ld a,0
	@resetcth:
	ld (PanParams + 6),a ;Counts from 0 to 3
	ld (sc_offs),a
	ld b,a
	ld a,(PanParams)
	cp 2
	jr nz,@noleftpansub2
		ld a,(PanParams + 9)
		ld (sc_offs),a
	@noleftpansub2:
	call HandlePenPan
	ld d,9
	call DrawLines
	ret

	@vertpan:
	;6 per pixel count, 7 scroll position in tile, 10 second page
	ld a,(PanParams + 6)
	cp 0
	jr nz,@notpanmathv
		ld a,(PanParams + 7) 
		cp 0
		call nz,FlipVpage
		ld a,(PanParams + 7) 
		ld c,a
		ld b,a
		ld a,(PanParams)
		cp 4
		jp nz,@anohofx
			dec b
		@anohofx:
		ld a,b
		cp 26
		jp nz,@panendv
			ld a,3
			ld (PanParams + 5),a
			ret
		@panendv:
		ld a,(PanParams + 0)
		cp 4
		jr nz,@nouppan
			ld a,28
			sub a,c
			ld c,a 
			ld a,(PanParams + 11)
			xor 32
			ld (PanParams + 10),a
			ld b,a
			ld de,0
			call SetCurrentLine ;bc mainram x,y de vram x,y
			ld a,255
			ld (PanParams + 9),a
			jp @nodownpan
		@nouppan:
			ld a,(PanParams + 11)
			ld (PanParams + 10),a
			ld b,a
			ld de,0
			inc c
			inc c
			inc c
			call SetCurrentLine ;bc mainram x,y de vram x,y
			ld a,255
			ld (PanParams + 9),a
		@nodownpan:
		ld a,(PanParams + 7) 
		inc a
		ld (PanParams + 7),a



	@notpanmathv:

	ld a,0
	ld (PanParams + 12),a ;Kludge to fix sprite offset thing

	ld a,(PanParams + 6)

	ld h,a
	ld a,(PanParams + 0)
	cp 4
	jr nz,@nouppansub
		ld a,7
		sub h
		ld h,a
		ld a,(PanParams + 7)
		cp 1
		jp nz,@noinifix
			ld a,h
			sub 8
			ld h,a

			ld a,(PanParams + 6)
			cp 6
			jp nc,@nouppansub

			ld a,8
			ld (PanParams + 12),a ;More kludging 
			jp @nouppansub
		@noinifix:
		ld a,(PanParams + 7)
		cp 18
		jp c,@nouppansub
			sub a,18
			ld b,a
			ld a,h
			sub b
			ld h,a					
	@nouppansub:
	ld c,$99
	ld a,h
	ld b,a
	ld a,$bd-8
	add a,b
	out (c),a ;Setup the real position of the HUD
	ld a,$93
	out (c),a

	ld l,0
	call UpdateScroll
	ld a,(PanParams + 6)
	add 3
	cp 8
	jr c,@resetcthv
		ld a,0
	@resetcthv:

	ld (PanParams + 6),a ;Counts from 0 to 7
	ld (sc_offs + 1),a
	ld b,a
	ld a,0
	ld (sc_offs),a
	ld a,(PanParams + 0)
	cp 4
	jr nz,@nouppan2
		ld a,(PanParams + 12)
		ld c,a
		ld a,7
		sub a,b
		ld b,a
		sub a,c
		ld (sc_offs + 1),a
	@nouppan2:
	call HandlePenPan
	ld a,9
	ld (PanParams + 8),a
	@laup: ;Loop that copy the data to the vram line by line, so we can caught when we have the change
		ld a,(PanParams + 7) ;Counter of scroll position
		ld b,a
		dec b
		dec b

		ld c,0
		ld a,(PanParams + 0)
		cp 4
		jp z,@notup3
			ld a,28
			sub a,b
			sub a,5
			ld b,a ;Reverts the trigger point for the screen change
			ld c,2
		@notup3:
		ld a,(PanParams + 9)
		
		cp b
		jp nz,@nopf
			ld a,(PanParams + 10)
			xor 32
			ld (PanParams + 10),a
			ld b,a ;Yep, X, not Y, we're using the horizontal dual screen as a makeshift vertical dual screen mode
;			ld c,0
			ld a,(PanParams + 9)
			inc a
			ld d,0
			ld e,a
			call SetCurrentLine ;bc mainram x,y de vram x,y
		@nopf:

		ld a,(PanParams + 9)
		inc a
		ld (PanParams + 9),a ;Counter of lines drawn

		ld d,1
		call DrawLines
		ld a,(PanParams + 8)
		dec a
		ld (PanParams + 8),a
	jp nz, @laup
	ret


@nostage2:
cp 3
jp nz,@nostage3

	ld a,0
	ld (sc_offs + 1),a
	ld (pause),a
	ld (PanParams + 5),a ;Reseting some things

	ld a,(PanParams + 1)
	ld (CurHalnb),a
	ld a,(CurHalnb) 
	rla
	rla 
	rla ;multiply by 8 because new table
	ld hl,($8406) 

	ld d,0
	ld e,a
	add hl,de ;address to the current hall header
	inc hl ;Jump over offset of number of halls

	ld a,(hl) 
	ld (Hallsiz),a ;Saving the number of screens

	inc hl ;Points at screen adress
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl

	ld hl,$8400
	add hl,de
	ld (CurHal),hl ;pointer to the blockmap (will need to change this for right screen spawn)


	ld a,(PanParams + 2)
	and 1
	ld c,a
	ld a,(flipcorridor)
	xor c
	ld (flipcorridor),a ;And NOW we flip the corridor according to next screen


	;Now it's the complicated where the hell we offset the penguin to stuff
	ld a,0
	ld (CamX),a
	ld (OCamX),a 
	ld (CamBuffPos),a 
	
	ld a,(PanParams + 3) ; 0 left 1 right, 2 down, 3 up
	cp 2
	jp nc,@udpan
	;Horizontal pan
		ld c,20
		cp 1
		jp nz,@rightpan
			ld c,230
		@rightpan:

		ld a,c
		ld (penphys + 2),a
	jp @lrpan
	@udpan:
	;Vertical pan
		ld c,20
		cp 3
		jp nz,@uppan
		ld c,160
		@uppan:

		ld a,c
		ld (penphys + 4),a
	@lrpan:
	
	ld a,(PanParams + 2)
	cp 0
	jp nz,@rightland
	;Coming from the left
	;Zeroing stuff for now, but this will have to take in account the pan direction to avoid teleporty stuff
	ld a,0
	ld (CamX+1),a
	ld (OCamX+1),a ;start on page 0

	;Redrawing a strip from the next screen, because the bg tile updater does it's business offscreen
	ld b,1 
	ld a,(PanParams + 1)
	ld d,a ;Hall id
	ld c,0
	ld a,(flipcorridor)
	cp 0
	jr nz,@pagefla2
		ld c,$20
	@pagefla2:
	ld ixh,4
	call DrawToBuffer ;b screen, c page (set to $20 for page 2),d hall id

	ld a,(PanParams + 1)
	ld d,a ;Hall id
	ld ixl,0 ;inital screen spawn
	;call 5
	call SpawnScreenPrgs
	call UpdateHall

ret
	@rightland:

	;coming from the right

	ld a,(PanParams + 2) ;Nice guess, this is indeed the screen the penguin will land on
	ld (CamX+1),a
	ld (OCamX+1),a ;Sets camera to the page the penguin is landing on
	ld (penphys + 0),a ;And sets the penguin on the right page because why not
	ld (PanParams),a
	cp 2
	jp nz,@nofixtoleftoff
		ld a, 210
		ld (penphys + 2),a ;just setting the penguin position a bit better when the pan ends
		ld (pen_x),a
	@nofixtoleftoff:

	ld a,(PanParams + 1)
	ld d,a ;Hall id
	ld ixl,1 ;end screen spawn
	;call 5
	call SpawnScreenPrgs
	call UpdateHall

@nostage3:

ret

RunPanVBL:

ret