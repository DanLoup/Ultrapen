.include "header.i"

.MACRO watch
	push af
		ld a,(\1)
		ld (\2),a
		ld a,(\1+1)
		ld (\2+1),a
	pop af
.ENDM
;the label does nothing, the program loading routine just jump to $100 and the first command here just happens to be it
globalvars: .ds 32,0

boot:
	di
	;ld hl,filefont
	;call $800a
	;call uploadfont
	ld a,0
	ld b,0
	call SetPal
	call InitSpriteSystem

	ld hl,filebase
	call $800a
	call LoadNewBase

	ld hl,prgpointers
	ld bc,32
	call AddPrgPointers ;HL pointer to pointers, BC count

	ld hl,filename
	call $800a ;Load the level, and it better keep it loaded, as countless routines just use the loaded data in the memory

	ld a,($f0)
	ld (vdpmem),a
	inc a
	ld (vdpreg),a
	ld sp,$efff ;Moving it to some place safer
	call InitPrograms	
	call initbar

	;ld hl,$4000 ;Load the standard sprites into the sprite table
	;ld c,(hl)
	;inc hl
	;call AddSpritePointers

	call LoadLevel


	ld a,(vdpreg)
	ld c,a
	jp +
	initvtb: .db $14,$80,$10,$82,$b7,$85,$0a,$86,$01,$8f,$00,$8b,$b5,$93 
	+ 
	ld b, 14
	ld hl,initvtb
	otir ;Initialize the video mode

	in a,(c) ;acknowledge hblank and detect it's an MSX 2+ / turbo R
	and 4
	jp z,notmsxplus
		ld a,1
		ld (msxplus),a
	notmsxplus:

	ld a,(vdpreg)
	ld c,a

	ld hl,$e281
	out (c),h
	out (c),l
	ld a,0
	ld (pause),a
	ei
	ld a,0
	ld (fadedir),a
jp maincode

testtable: .db 01,0,0,0,0,0,0,0,0,0,0



gamemode: .db 0
slowstuff: .db 7
framenew: .db 0
logicnew: .db 0
missedframe: .db 0
msxplus: .db 0
skipvbl: .db 0
vblfailed: .db 0
bar_fill: .db 0
bar_id: .db 0,0

waypoint: .db 0

dbgcolor:
		ret
		
		di
		push hl
		push bc
		ld c,$99
		ld hl,$0090
		out (c),h
		out (c),l
		ld c,$9a
		out (c),d
		out (c),e
		ld c,$99
		ld hl,$1090
		out (c),h
		out (c),l
		pop bc
		pop hl
		ei
ret

vblank:
		push af
		push bc
		push de
		push hl
		push ix
		push iy
		di
		ld c,$99
		ld de,$018f
		out (c),d
		out (c),e
		in a,(c) ;acknowledge hblank in case
		and $1
		jp z,nohblank

		ld c,$99
		ld de,$018f
		out (c),d
		out (c),e
		in a,($99) ;Acknowledge hblank
		call hblank
		jp hblankend
		nohblank:

		ld c,$99
		ld de,$008f ;Roll back to status register zero..
		out (c),d
		out (c),e
		in a,($99) ;acknowledge vblank


		ld hl,$b593 ;Set hblank for the status bar
		out (c),h
		out (c),l

	;ld de,$2000
	;call dbgcolor

	call UpdateSpriteBank ;This should happen before anything, because you can seriously glitch up the sprites otherwise
	;ld de,$0000
	
	;call dbgcolor

	;ld b,1 ;(let's steal around 8 up to lines here)
	;call RunBlitterCopy
	ld c,$99 ; Update sprite bank wrecks with the C



	;call RunNextPage

;Original position
		ld a,(msxplus)
		cp 1
		jp nz,mxp1
			ld a,(Offsetval)
			out (c),a
			ld a,$9b
			out (c),a ;Keep the scroll true
			ld hl,$0299
			out (c),h ;Masking the 8 pixels
			out (c),l
			jp nomxp1
		mxp1:
			ld a,(Offsetval)
			out (c),a
			ld a,$92
			out (c),a ;Keep the scroll true
		nomxp1:
		ld a,0 ;In case of missed frame, it is a zero
		out (c),a
		ld a,$97
		out (c),a

		

		call ReloadVpage
		ld de,$008A ;Pulls the color table back
		out (c),d
		out (c),e
		ld de,$ff83 ;Set color table to C000 
		out (c),d
		out (c),e
		ld hl,$0a86
		out (c),h
		out (c),l ;Obj Tiles
		ld de,$0000
		;call dbgcolor

		ld hl, $1480 ;Back to glorious screen 4
		out (c),h
		out (c),l
		ld hl, $0888 ;With sprites (and hopefully not screwing the VRAM)
		out (c),h
		out (c),l
		
		ld a,(dsprites)
		cp 0
		jp z,dissprites
			ld hl, $0A88 ;No sprites (and hopefully not screwing the VRAM)
			out (c),h
			out (c),l
		dissprites:



		ld a,(logicnew)
		cp 1
		jp nz,nologic
		;call UpdateSpriteBank
		ei
		call HandleFade
		ld a,0
		ld (logicnew),a

		ld a,(gamemode)
		cp 0
		call z,ingame_vbl
		ld a,(gamemode)
		cp 1
		call z,pan_vbl
		ld a,(gamemode)
		cp 2
		call z,HandleResetScreen
;		ld a,(gamemode)
;		cp 3
;		call z,FillLifeHBL
		jp exitvbl
		nologic:
		
		;ld de,$2000
		;call dbgcolor ;Frame missed
		ld a,1
		ld (vblfailed),a
		exitvbl:

		VBLnotpanmode:
;			call RunBBVBL ; HERESY
		ld a,0
		ld (vblfailed),a
		;Music driver spot
		call music


		ld a,1
		ld (framenew),a
		
		hblankend:
		pop iy
		pop ix
		pop hl
		pop de
		pop bc
		pop af
		ei
;		watch sc_offs,2
reti

maincode:

	vblwait:
		ld a,(framenew)
		cp 1
		jp nz,vblwait ;wait for new frame
		ld a,0
		ld (logicnew),a

		;watch 1,tuc

	ld a,(gamemode)
	cp 0
	call z,ingame_mode
	ld a,(gamemode)
	cp 1
	call z,pan_mode

	ld a,(gamemode)
	cp 3
	call z,FillLifeHBL
	ld a,(gamemode)

	ld a,0
	ld (framenew),a
	ld a,1
	ld (logicnew),a


	ld de,$0200
	;call dbgcolor ;Free time...

	;ld de,$0000
	;call dbgcolor ;line
	
	ld a,0
	ld (eholdframe),a ;Clear the external holdframe flag because we don't need it on this frame
jp maincode

ingame_mode:

	;ld de,$edff
	;call dbgcolor 
	call flick_sprites


	call CheckForCF ;This one is heavy when data is being copied

	ld de,$7777
	call dbgcolor ;Free time...

	call RunSpriteSystem ;This get fatter when data is being copied as well

	ld de,$0000
	call dbgcolor ;Free time...

	;;call RunBBI

	;ld de,$2222
	;call dbgcolor 

	call GetKeys ;This one is purposefully slow to not screw with the keyboard
	ld a,(Hallsiz)
	cp 1
	call nz,HandleLevel ;this one is cheap as hell

	ld a,(penmenu)
	cp 1
	call z,RunMenuMode

	ld a,(pause)
	cp 0
	ret nz ;If the game is paused, there's no need to run those

	;ld de,$0600
	;call dbgcolor ;Sprite system execution time end
	call RunPrograms

	;ld de,$0000
	;call dbgcolor ;Sprite system execution time end

	call nRunPrgSpawner

;	call slow
;	call slow
;	call slow
;	call slow

	
ret

ingame_vbl:

	call RunPan ;Run panning when needed

	ld a,(pause)
	cp 0
	ret nz ; Pause/pan

	ld a,(Hallsiz)
	cp 1
	ret z


	ld hl,(sc_offs)
	call UpdateScroll

	;ld de,$2222
	;call dbgcolor 

	call UpdateHall
	call ReloadVpage
	
	;ld de,$0
	;call dbgcolor 




ret

pan_mode:
	call CheckForCF 
	;call RunSpriteSystem
ret
	
pan_vbl:
	call CheckForCF 
	call HandlePan
	call HandlePenPan
	call ReloadVpage
	ld c, $99
	ld a,(sc_offs+1) 
	ld b,a
	ld a,$b5
	add a,b
	out (c),a ;Setup the real position of the scroll bar
	ld a,$93
	out (c),a
ret

FillLifeHBL:
	call CheckForCF 
	call RunSpriteSystem
	;call ReloadVpage
	;call handlelifebars
	ld a,(bar_fill)
	dec a
	ld (bar_fill),a
	cp 0
	jp nz,not_full_yet
		ld a,0
		ld (gamemode),a
	not_full_yet:
	ld a,(fillspeed)
	xor 1
	ld (fillspeed),a
	ld b,a
	ld a,(bar_fill)
	ld hl,(bar_id)
	ld a,(hl)
	add a,b
	ld (hl),a
	
	cp 28
	jp nz,no_excess_either
		ld a,0
		ld (gamemode),a
	no_excess_either:
ret

fillspeed: .db 0

big_boss_mode:

ret

big_boss_vbl:

ret	
	
	
UpdateScroll:
		;Handling scroll X
		ld a,(msxplus)
		cp 1
		jp nz,mxp2
			ld c,$99
			ld a,l
			and $7
			ld b,a
			ld a,7
			sub b
			and $7
			ld (Offsetval),a
			out (c),a
			ld a,$9b
			out (c),a
		jp nomxp2
		mxp2:
			ld c,$99
			ld a,l
			ld (Offsetval),a
			out (c),a
			ld a,$92
			out (c),a
		nomxp2:
		ld a,h ;Handling scroll y
		out (c),a
		ld a,$97
		out (c),a

ret

def:
ret


blinkct: .db 0
fchange: .db 0,0
	filename: .db "LEVEL   LVL",0
	filefont: .db "FONT    BIN",0
	filebase: .db "BASE    BIN",0

prgpointers: .dw HandlePen,HandlePowerUp,ProgramPan,ChangePaletteobj,def,def,def,def ;Objects that can be placed
.dw RunNewShot,VramEffect,PenguinDeath,def,def,def,def,def

keys: .db 0
keymap .db 8,$20,8,$80,8,$40,8,$10,2,$40,5,$1,7,$80 ;Up,right,down,left,a,s,enter
sc_offs: .db 0,0

GetKeys:
	push bc
	push de
	push hl
	ld a,0
	ld (keys),a
	ld b,1
	ld d,7
	ld hl,keymap
	keyfetchloop:
		ld a,(hl)
		inc hl
		out ($aa),a
		xor a
		xor a
		xor a
		xor a ;Waiting a bit to stabilize the keyboard just in case
		ld c,(hl)
		inc hl
		in a,($a9)
		xor 255
		and c ;Filter out to get the bit
		cp $0
		jp z,keynot
		ld a,(keys)
		add a,b
		ld (keys),a ; Key pressed, adding it to the mask
		keynot:
		xor a
		rlc b ;multiply stuff
		dec d
		jp nz,keyfetchloop
	pop hl
	pop de
	pop bc
ret

	;call $5 //This jumps to my generic debug thing

slow:
	ld hl,$0201
slolo:
	dec l
	jp nz,slolo
	dec h
	jp nz,slolo
ret

hblpos: .db 0,0,0,0

hbl_redir: .db 0
hbl_redirct: .db 0

hblank:
	ld a,(hbl_redirct)
	cp 0
	jp z,noredir
		dec a
		ld (hbl_redirct),a
		ld a,(hbl_redir)
		cp 0
		;call z,bbredir

		ret ;Return without actually running the menu code
	noredir:
	call bitmapbar
	;call textmode
ret

textmode:
	ld c,$99
	ld hl,$1c86
	out (c),h 
	out (c),l ;Disable sprites without wrecking with the video ram
	ld hl,$2882
	out (c),h ;Push tile map to $A000
	out (c),l 
	ld hl, $1384 ;Push BGtiles to $8000
	out (c),h
	out (c),l ;Set back
	ld a,(msxplus)
	cp 1
	jp nz,nopl2
		ld hl,$99
		out (c),h ;not masking the 8 pixels
		out (c),l
		ld hl,$9b
		out (c),h ;Also no scroll please
		out (c),l
	nopl2:
	ld hl, $92 ;Make TV scroll fixed
	out (c),h 
	out (c),l
	ld hl,$0497 ;Make the y scroll not be a thing
	out (c),h
	out (c),l
	ld de,$038A 
	out (c),d
	out (c),e
	ld de,$7f83 ;Set color table to C000 
	out (c),d
	out (c),e
ret

removescroll:
	ld c,$99

	ld hl,$9b
	out (c),h ;Also no scroll please
	out (c),l
	ld hl, $92 ;Make TV scroll fixed
	out (c),h 
	out (c),l
ret

pendarkcol: .dw $0002 

bitmapbar:
;di
	ld a,(vdpreg)
	ld c,a

	ld hl,$97 ;Make the y scroll not be a thing
	out (c),h
	out (c),l

	ld hl,$0680 ;Mode 5
	out (c),h
	out (c),l

	ld hl, $1f82 ;In theory ,should set the bank to $8000
	out (c),h
	out (c),l

	ld hl, $0A88 ;No sprites (and hopefully not screwing the VRAM)
	out (c),h
	out (c),l

	ld hl, $0b90
	out (c),h
	out (c),l

	call removescroll

	ld c,$9a
	ld hl,(pendarkcol)
	out (c),l
	out (c),h ; Just setting a blue color

	ld de,$000f
	;call dbgor ;Free time end

	ld b,24; (menu is 24 lines tall so, 24)
	call RunBlitterCopy

	ld de,$0000
	;call dbgcolor ;Free time end
	;ei
ret

vertbarpt: .db $ff,$00,$99,$99,$99,$99,$99,$99,$99,$99,$99,$99,$99,$99,$99,$99,$99,$99,$99,$99,$00,$ff,$00

initbar:
	;call 5
	di
	ld c,$99
	ld de,$018e ;Set page 1 (the end of the first 32KB, where the tile engine just don't touch)
	out (c),d
	out (c),e

	ld hl, $5c00 ;1c00 + 40 to tell MSX we're writing to the vram	

	out (c),l
	out (c),h
	ld de, $1880
	ld c,$98
	ld a,$77
	ld hl,vertbarpt
	fillblue:
			out (c),a
			dec e
		jp nz,fillblue
		ld a,(hl)
		inc hl
		ld e,$80
		dec d
	jp nz,fillblue
	
		ld hl,$5c84
		ld ix, $00ff
		ld iy, $ff00
		
		ld a,22
	borderdraw:
		ld c,$99
		out (c),l
		out (c),h
		push ix
		pop de
		ld c,$98
		out (c),d
		out (c),e
		ld bc,51
		add hl,bc
		ld c,$99
		out (c),l
		out (c),h
		ld c,$98
		push iy
		pop de
		out (c),d
		out (c),e
		push ix
		pop de
		out (c),d
		out (c),e
		ld bc,14
		add hl,bc
		ld c,$99
		out (c),l
		out (c),h
		ld c,$98
		push iy
		pop de
		out (c),d
		out (c),e
		push ix
		pop de
		out (c),d
		out (c),e
		ld bc,57
		add hl,bc
		ld c,$99
		out (c),l
		out (c),h
		ld c,$98
		push iy
		pop de
		out (c),d
		out (c),e

		ld bc,6
		add hl,bc
		ld c,$99
		out (c),l
		out (c),h
		ld ix, $0ff0
		ld iy, $0ff0
		cp 2
		jp nz,nospecialbd
			ld ix, $00ff
			ld iy, $ff00
		nospecialbd:
		dec a
		jp nz,borderdraw

	;HL Source, DE destination, BC copy retangle,iyl precalculated copy time
 	ld hl,$0040
	ld de,$76bb
	ld bc,$1513
	ld iyl, $1
	call AddBlitterCopy

 	ld hl,$8040
	ld de,$00b9
	ld bc,$0816
	ld iyl, $1
	call AddBlitterCopy

 	ld hl,$184a
	ld de,$50bc
	ld bc,$0806
	ld iyl, $1
	call AddBlitterCopy

 	ld hl,$144a
	ld de,$52c4
	ld bc,$0407
	ld iyl, $1
	call AddBlitterCopy

 	ld hl,$2840
	ld de,$58be
	ld bc,$0404
	ld iyl, $1
	push hl
	push bc
	call AddBlitterCopy

	pop bc
	pop hl
	ld de,$58c6
	ld iyl, $1
	call AddBlitterCopy

	ld h,2
	ld l,4
	call UpdateStatusBar

	ld h,3
	ld l,0
	call UpdateStatusBar

	ld h,0
	ld l,28
	call UpdateStatusBar

	ld h,1
	ld l,255
	call UpdateStatusBar

	ld h,4
	ld l,255
	call UpdateStatusBar

	ld a,4
	call SetPenFace

	ei
ret

lnbct: .db 0
LoadNewBase:
	;call 5
	ld ix,$8400
	ld a,(ix+0)
	ld (lnbct),a
	inc ix
	copbl:
	ld a,(ix+0)
	ld l,(ix+1)
	ld h,(ix+2)
	ld c,(ix+3)
	ld b,(ix+4)
	ld e,(ix+5)
	ld d,(ix+6)
	cp 1
	jp z,cpvram
	cp 2
	jp z,cpaddsp
	;copy to ram
	ldir

	jp lbnendtrn
	cpvram: ;Copy vram
	ld a,d
	out ($99),a
	ld a,$8e
	out ($99),a
	ld a,0
	out ($99),a
	ld a,e
	add $40
	out ($99),a ; Page, position
	inc b
	ld a,b
	ld c,$98
	cpvramloop:
	ld b,$ff
	otir
	dec a
	jp nz,cpvramloop

	jp lbnendtrn
	cpaddsp: ;Add to sprite table
	;Just fill spriteptstack
	call addSpritesLocal
	ld de,$7180 ;Just being sure sprite pointer it's at the right place
	ld (spriteptstack),de

	lbnendtrn:
	ld de,$0007
	add ix,de
	ld a,(lnbct)
	dec a
	ld (lnbct),a
	jp nz,copbl
ret

music:
ret

copybase:
	ld hl,$8400
	ld de,$4000
	ld bc,$2000
	ldir
ret

blittable: .ds 255,0 ;blitter Stack
nblittable: .ds 255,0 ;next blitter Stack

blitpos: .db 0,0,0,0
blitcpos: .db 0
intable: .ds 8,0

;HL Source, DE destination, BC copy retangle
;iyl precalculated copy weight
AddBlitterCopy:
	di
	push de
	ld ix,blittable
	ld a,(blitpos)
	ld d,0
	ld e,a
	add ix,de
	pop de

	ld (ix+0),h
	ld (ix+1),l
	ld (ix+2),d
	ld (ix+3),e
	ld (ix+4),b
	ld (ix+5),c
	ld a,iyl
	ld (ix+6),a

	ld a,(blitpos)
	add a,7
	cp 128
	jp m,no_resetbp
	ld a,0
	no_resetbp:
	ld (blitpos),a
	ei
ret
;BC source, DE destination, A count (this solution is just not very optimal at all lol)
;iyl precalculated copy weight (again)
AddBlitterAddressCopy:
;	;call 5
	di
	add hl,hl
	push hl
	ld (blitpos + 2),a
	ld hl,blittable
	ld b,0
	ld a,(blitpos)
	ld c,a
	add hl,bc
	ld a,(blitpos + 2)

	pop bc
	ld (hl),c
	inc hl
	ld (hl),b
	inc hl

	ex de,hl
	add hl,hl
	ex de,hl

	ld (hl),e
	inc hl
	ld (hl),d
	inc hl

	add a,a
	ld (hl),a
	inc hl
	ld a,1
	ld (hl),a
	inc hl
	ld a,iyl
	ld (hl),a

	ld a,(blitpos)
	add a,7
	cp 128
	jp m,no_resetbpa
	ld a,0
	no_resetbpa:
	ld (blitpos),a
	ei
ret 


;HL source, DE destination, C count (this solution is just not very optimal at all lol)
;iyl precalculated copy weight (again)
AddBlitterAddressCopyNext:
	di
	ld ix,nblittable
	ld a,(blitpos+2)
	add a,ixl
	ld ixl,a
	ld a,0
	adc a,ixh
	ld ixh,a
	xor a
	rl l
	rl h
	xor a
	rl e
	rl d
	ld (ix+0),l
	ld (ix+1),h
	ld (ix+2),e
	ld (ix+3),d
	xor a
	rl c
	ld (ix+4),c
	ld b,1
	ld (ix+5),b
	ld a,iyl
	ld (ix+6),a

	ld a,(blitpos+2)
	add a,7
		cp 128
		jp m,no_resetbpc
		ld a,0
		no_resetbpc:
	ld (blitpos+2),a
	ei
ret 

RunNextPage:
	ld a,(blitpos+2)
	cp 0
	ret z

	ld hl,blittable
	ld a,(blitpos)
	add a,l
	ld l,a
	ld a,0
	adc a,h
	ld h,a
	ex de,hl

	ld hl,nblittable

	@copystuffrnp:
		ld bc,7
		ldir
		
		ld a,(blitpos)
		add a,7
		cp 128
		jp m,@no_resetbpd
			ld a,0
		@no_resetbpd:
		ld (blitpos),a


		ld a,(blitpos+2)
		sub a,7
		ld (blitpos+2),a
	jp nz,@copystuffrnp
	ld a,0
	ld (blitpos +2),a
ret

;HL sprite pointer
addSpritePageFlip:
	di
	ld ix,blittable
	ld a,(blitpos)
	add a,ixl
	ld ixl,a
	ld a,0
	adc a,ixh
	ld ixh,a
	ld a,255
	ld (ix+0),a
	ld (ix+1),a ;FFFF adress means it's a sprite pointer
	ld (ix+2),l
	ld (ix+3),h ;actual adress of the sprite
	ld a,0
	ld (ix+4),a
	ld (ix+5),a
	ld (ix+6),a ;Zeroing stuff so setting sprite "takes no time"

	ld a,(blitpos)
	add a,7
	cp 128
	jp m,no_resetbpb
	ld a,0
	no_resetbpb:
	ld (blitpos),a
	;ld a,1
	;ld (lockflicker),a
	ei
ret

;B,  max number of precalculated units to copy
RunBlitterCopy:
	ld c,$99
	ld hl,$0680 ;Just certifying we're ready for that blitter copy
	out (c),h
	out (c),l

	ld a,b
	ld (blitpos + 1),a ;Load the "maximum number of copies"

	Blittercopyloop: ;Keeps looping to here as long there's time

	ld a,(blitpos)
	ld b,a
	ld a,(blitcpos)
	cp b
	ret z ;No task to perform


	ld c,$99
	ld de,$2091 ;As we're filling a truckload of video registers, will use the 9b thing for faster/smaller code
	out (c),d
	out (c),e

	ld hl, blittable
	ld a,(blitcpos)
	ld e,a
	ld d,0
	add hl,de
	
	ld a,(hl)
	ld d,a
	inc hl
	ld a,(hl)
	and d
	cp 255
	jp z,@spriteflip
	dec hl

	call waitforblitterend 

	ld c,$9b
	ld de,$0001
	outi	  ;Register 20h: Source X Low byte (0-FF)
	out (c),d ;Register 21h: Source X High byte (0-1)
	outi	  ;Register 22h: Source Y Low byte (0-FF)
	out (c),e ;Register 23h: Source Y High byte (0-3)
	outi	  ;Register 24h: Destination X Low byte (0-FF)
	out (c),d ;Register 25h: Destination X High byte (0-1)
	outi	  ;Register 26h: Destination Y Low byte (0-FF)
	out (c),d ;Register 27h: Destination Y High byte (0-3)
	outi	  ;Register 28h: Number of X dots low byte (0-FF)
	out (c),d ;Register 29h: Number of X dots high byte (0-3)
	outi 	  ;Register 2Ah: Number of Y dots low byte (0-FF)
	out (c),d ;Register 2Bh: Number of Y dots high byte (0-3)
	
	ld c,$99
	ld de,$d0ae ;Tell out friend blitter it's time to blit
	out (c),d
	out (c),e

	jp @nospriteflip
	@spriteflip:
		inc hl ;It's time to fetch that juicy address
		ld e,(hl)
		inc hl
		ld d,(hl)
		inc hl ;Pushing it to a zero
		ld a,(de)
		xor 64
		ld (de),a ;Sprite status, flipped
	@nospriteflip:

	ld a,(blitcpos)
	add a,7
	cp 128
	jp m,no_resetsd
	ld a,0
	no_resetsd:
	ld (blitcpos),a
	xor a
	ld a,(blitpos+1)
	ld b,(hl)
	sbc b
	ld (blitpos+1),a
	jp nc,Blittercopyloop ;There's still time, so let's check if there's more tasks to do
ret

waitforblitterend:
	ld c,$99
	ld de,$028f ;Set the status read to the register 2...
	out (c),d
	out (c),e
	in a,($99) ;As the name implies, we're waiting until the blitter finished doing the job
	and 1 
	jp nz,waitforblitterend
ret

.include "sprite.i"
.include "level.i"
.include "object.i"
.include "pen.i"
.include "mem.i"
;.include "shot.i"
;.include "enemy.i"
;.include "effects.i"
;.include "bigboss.i"

fontdt: .incbin "font.bin"

;return [expr {[set [get_active_cpu]_freq]} * [machine_info time]]