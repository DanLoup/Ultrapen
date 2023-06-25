;I'm probably not calling dibs on any area of memory in this one in particular
bb_coords: .db $7f,0,$40,0 ;XH-L and Y of the effect in pixels (it's 16bit because as the character is HUGE, you need the extra precision to be able to shove it offscreen in all four sides
bb_clamp: .db 1,21 ;Hard clamping of the effect
bb_size: .db 16,20 ;Size in tiles
bb_offs: .db 10,3 ;Offset on the second page, It determines the "UV" of the boss in the "second page texture"
bb_offscan: .db 25,192-20
bb_ceilingless: .db 1
bb_running: .db 0
bb_bossid: .db 0
bb_busy: .db 0
bb_oldoff: .db 0,0,0,0

bb_coordstack: .ds 24,0
bb_ocoords: .db 0,0,0,0
bb_countdown: .db 0
bb_wrapmode: .db 0
bb_gem: .db 0
bb_gem_invul: .db 0
bb_gem_pos: .db 0,0
boss_bar_en: .db 0
boss_bar_life: .db 0
bb_skip: .db 0
bb_storething: .db 0,0
;Useful routines
;DrawBlock: ;BC X,Y, D Block ID -This function draw a block to the dual screen tilemap


;SetCurrentLine: ;bc mainram x,y de vram x,y  ;Function to setup where the lines of tile will be copied from and to (also abused to do the pan and big enemy effects)

;Okay, found how this can done for easy
;The trick is that you make the map with two screens, one with the actual level data, annnd one with the boss
;As soon the boss load, the script artificially limit the screen back to one screen (yeah, that seems to work as long i setup the exit correctly)
;Then it use only SetCurrentLine to draw and move the boss around
;No messy drawblock, no messy anything
BootBB:
	ld a,1
	ld (bb_running),a
	ld (Hallsiz),a
	ld hl,(tmpstru) ;Loads the pointer to the object system because the data we need is there
	inc hl
	ld a,(hl) ;Roll to the subtype
	ld (bb_bossid),a

	ld hl,$7f
	ld (bb_coords),hl
	ld hl,$40
	ld (bb_coords+2),hl
	ld c,42
	ld hl,bb_busy
	ld a,0
	cleloopo:
		ld (hl),a
		inc hl
		dec c
	jp nz,cleloopo
	ld c,14
	ld hl,bb_tmp
	cleloopobbt:
		ld (hl),a
		inc hl
		dec c
	jp nz,cleloopobbt
	
	
	ld a,0
	ld (bb_postinit),a

	ld c,$99
	ld hl,$018e ;Sets the current page of the video write
	out (c),h
	out (c),l

	ld bc,$0002
	ld de,$0000
	call SetCurrentLine
	ld d,27
	call DrawLines
	call FlipVpage

	ld bc,$0002
	ld de,$0000
	call SetCurrentLine
	ld d,27
	call DrawLines


	ld bc,$0120
	ld a,(CurHalnb) ;Writing reverse page
	ld d,a
	call DrawToBuffer

	call setupboss
	
	ld hl,$010A
	ld de,$F8c0
	ld bc,$0000
	call CreateSprite	;HL Type/Subtype,DE X,Y,B Flip X,Y ,C Frame  Returns OBJ ID on A
	ld (bb_gem),a
	ld hl,-$400
	ld (bb_curboing),hl
	
	ld a,0
	ld (bb_unfinished),a ;reset unfinished test

	
ret

bb_postinit: .db 0

RunBBVBL:

	ld a,(bb_running)
	cp 1
	jp nz,nobb

	call InitBoing

	call removescroll
	ld c,$99
	ld a,(bb_offscan)
	out (c),a
	ld a,$93
	out (c),a

	ld a,2
	ld (hbl_redirct),a ;Hblank splits, 2


	ld a,(bb_ceilingless) ; Ceiling less mode
	cp 1
	jp nz,bb_hasceiling
	ld a,0
	ld (bb_clamp),a
	ld hl,(sc_offs)
	call UpdateScroll
	
	ld c,$99	
	ld a,(bb_offscan+1)
	ld b,a
	ld a,(sc_offs+1) 
	add a,b
	out (c),a
	ld a,$93
	out (c),a ;Here it just setup the scroll as we're at line 0
	ld a,1
	ld (hbl_redirct),a ;Set coordinates and hblank to just execute the floor, as well, ceilingless

	bb_hasceiling:


	ld a,(vblfailed) 
	cp 0
	jp nz,bb_nocomplete

	ld a,(bb_countdown)
	cp 1
	jp nz,bb_nocomplete
	ld a,0
	ld (bb_countdown),a
	call FlipVpage 
	bb_nocomplete:


	ld a,(vblfailed) 
	cp 0
	jp z,no_readjust
		ld hl,(bb_storething)
		call UpdateScroll
		ld c,$99
		ld a,(bb_offscan+1)
		ld b,a
		ld a,(bb_storething+1) 
		add a,b
		out (c),a
		ld a,$93
		out (c),a ;Here it just setup the scroll as we're at line 0
	no_readjust:

	ld hl,(sc_offs)
	ld (bb_storething),hl


	;Setup Scroll position (and save it)
	ld a,(vblfailed) 
	cp 0
	jp nz,noscoff
	
	ld a,(bb_ocoords)
	and $7
	ld b,a
	ld a,$7
	sub b
	ld (sc_offs),a
	ld a,(bb_ocoords+2)
	and $7
	ld b,a
	ld a,$7
	sub b
	add a,$f4
	ld (sc_offs+1),a ;Setup the scroll registers for the function that runs later on
noscoff:

	
	ld hl,(sc_offs)
	ld (bb_oldoff+2),hl ;And save the old scroll position for the ceiling mode
	;Routine that makes it flip later so it syncs with the prediction

	ld a,0
	ld (bb_unfinished),a ;reset unfinished test
	
nobb:


ret
;BC X/Y
bb_unfinished: .db 0
RunBBI:
	ld a,(bb_running)
	cp 0
	ret z
	ld a,1
	ld (bb_unfinished),a ;Set unfinished 


	ld a,(bb_postinit)
	cp 7
	;call nc,RunBoing


	ld a,(tuc)
	cp 16
	jp nc,bb_vblworked ;Avoid copying data if the sprite upload system used too much time already, so we avoid the slowdowns

	ld a,(bb_coordstack+4  )
	and $f8
	ld b,a
	ld a,(bb_coordstack  )
	and $f8 ;its a per tile test
	cp b
	jp nz,bb_coordchange
	ld a,(bb_coordstack+7 )
	ld c,a
	ld a,(bb_coordstack+6 )
	and $f8
	add a,c
	ld b,a
	ld a,(bb_coordstack+3 )
	ld c,a
	ld a,(bb_coordstack+2 )
	and $f8 ;its a per tile test
	add a,c ;add it to represent up/down coords
	cp b
	jp z,bb_nocoordchange
	bb_coordchange:
	call setupboss
	ld a,20
	call drawboss
	ld a,1	
	ld (bb_countdown),a
	bb_nocoordchange:
	
	ld c,21 ;Stack 
	ld hl,bb_coordstack+20
	ld de,bb_coordstack+24
	bb_stackdown:
		ld a,(hl)
		ld (de),a
		dec hl
		dec de
		dec c
	jp nz,bb_stackdown
	
	ld hl,(bb_coords)
	ld (bb_coordstack),hl
	ld hl,(bb_coords+2)
	ld (bb_coordstack+2),hl
	
	ld b,0
	call cback

	
	bb_vblworked:


	ld a,0
	ld (bb_unfinished),a ;reset unfinished test again


ret 


bb_tmp: .ds 14,0


setupboss:
	;Input
	;40 XL|1 XH|2 YL|3 YH|4 CLX|5 CLY|6 SZX|7 SZY|8 OFFX|9 OFFY
	;Output
	;50 STarX|1 LenX|2 Clip Y|3 StartY|4 LenY

	
	;Things to fill
	;X always start with 0+offset on source 
	;Start X (affect dest coordinates)
	;Len X (number of bytes to copy)
	;start Clip Y (affects source coordinates+offset)
	;Start Y (affect dest coordinate)
	;Len Y (number of lines to copy)
	ld a,(bb_coordstack+3 )
	cp 0
	ld a,(bb_coordstack+2 )
	jp nz,bb_testup
		cp $80
		ret c
	jp bb_testdown
	bb_testup:
		cp $90
		ret nc
	bb_testdown:
	xor a
	ld a,(bb_coordstack )
	and $f8 ;Just remove the bits that would wrap over
	rrca
	rrca
	rrca
	ld (bb_tmp),a ;Start X
	ld b,a
	ld a,(bb_size)
	ld c,a
	add a,b ;Size + offset
	sub 32
	jp nc,bb_nosizeclip ;If A is smaller than 32 (the screen witdh), no clipping is needed
		ld a,0
	bb_nosizeclip:
	xor $ff
	add a,c ;But if it is not, we add the negative of the extra size from the screen and it works out
	inc a

	ld (bb_tmp+1),a ;Finally Len X
	ld a,(bb_clamp)
	ld b,a
	ld a,(bb_coordstack+2 )
	and $f8 ;Ditto
	rrca
	rrca
	rrca
	add a,b ;Add clamp up so well.. clamp works
	ld (bb_tmp+3),a ;Start Y
	ld a,0
	ld (bb_tmp+2),a ;Start clip Y is zero...
	ld (bb_wrapmode),a ;No need for wrapping this time
	ld a,(bb_coordstack+3 ) ;This clips if negative, but i can solve this thing but just adding the clamp offset to the actual coordinates instead of doing this annoying thing
	cp 0
	jp nz,bb_notnegy
		ld a,(bb_tmp+3)
		xor $1f 
		inc a
;		inc a
;		dec a
		ld (bb_tmp+2),a ;Unless we're pushing it up, and this gives the clip y
		ld a,(bb_clamp) 
		ld (bb_tmp+3),a ;Start Y starts at clamp up
		ld a,2
		ld (bb_wrapmode),a ;The idea here is to write to the last line of tiles so it wraps around
	bb_notnegy:


	ld a,(bb_tmp+2)
	ld b,a
	ld a,(bb_size+1)
	sub b ;As we're clipping up, less lines should be copied
	ld (bb_tmp+4),a ;And with that, size Y is *ALMOST FILLED*
	ld b,a
	ld a,(bb_clamp+1)
	ld c,a
	ld a,(bb_tmp+3)
	add a,b ;And with this i know how low we are
	
	sub c ;
	
	jp c,bb_notlowclip
		ld b,a
		ld a,(bb_tmp+4)
		sub b
		ld (bb_tmp+4),a ;As the box clipped down, what's rest of the operation is how much we have to clip
	bb_notlowclip:
	ld a,(bb_tmp+4)
	ld b,a
	ld a,(bb_size+1)
	cp b
	ret c ;Lines bigger than the actual size? time to nope out of this as routines above are probably pushing the thing too low
	
	
	;ld de,0000 ;Here i need to calculate the X/Y coordinates in the video memory
	ld a,(bb_tmp+3) ;Let's start this hell with y
	ld b,a
	ld a,(bb_clamp)
	add a,b
	call mula32

	ld a,(bb_tmp)
	ld h,0
	ld l,a
	add hl,de ;And add X as well
	ex de,hl
	
	ld hl,$4000
	ld a,(CamDbl)
	cp 0
	jp z,bb_pg2
	ld hl,$4400
	bb_pg2:
	add hl,de ;This is how we calculate a fb position, so just copypasted from above
	ld (bb_tmp+8),hl
	
	
	;Calculating Y (more complex
	ld a,(bb_offs+1)
	ld b,a
	ld a,(bb_tmp+2)
	add a,b
	
	call mula32

	ld a,(bb_offs)
	ld c,a
	ld b,0
	ld hl,$6020 ;Tilemap (+ offset to get to the right tilemap)
	add hl,de ;Add Y coordinate
	add hl,de ;But we add it again, because in this case is *64, not *32
	add hl,bc ;And X coordinate
	ld (bb_tmp+10),hl
	ld a,(bb_wrapmode)
	cp 0
	jp z,bb_nowrapsetup
		ld de,$40
		ld hl,(bb_tmp+8)
		sbc hl,de
		ld (bb_tmp+8),hl
		
		ld hl,(bb_tmp+10)
		sbc hl,de
		sbc hl,de
		ld (bb_tmp+10),hl

	bb_nowrapsetup:
	ld a,1
	ld (bb_busy),a
ret 

drawboss:
	ld (bb_tmp+12),a
	ld a,(bb_tmp+4)
	cp 0
	ret z
	
	bb_fillloop:
		ld hl,(bb_tmp+8)
		ld c,$99
		ld a,(bb_wrapmode)
		cp 0
		jp z,bb_notwrap
			;ld de,$3C0
			ld de,$400

			add hl,de
			ld a,(bb_wrapmode)
			dec a
			ld (bb_wrapmode),a
			bb_notwrap:
		out (c),l
		out (c),h 

		ld hl,(bb_tmp+10) ;Input adress
		ld a,(bb_tmp+1)
		ld b,a ;Count
		ld c,$98 ;Output port
		otir
		ld de,$0020
		ld hl,(bb_tmp+8)
		add hl,de
		ld (bb_tmp+8),hl
		ld de,$0040
		ld hl,(bb_tmp+10)
		add hl,de
		ld (bb_tmp+10),hl

		
		ld hl,bb_tmp+4  ;This variable is definitively broken
		dec (hl)
		jp z,bb_endofcopy
		ld hl,bb_tmp+12
		dec (hl)
		jp nz,bb_fillloop 
	ret
	bb_endofcopy:
	ld a,0
	ld (bb_busy),a
	ret
	
mula32:
	rrca ;Lifting the code directly from up there
	rrca
	rrca ;Multiply by 32 (trust me)
	ld b,a
	and $e0
	ld e,a ;Low byte
	ld a,b
	and $1f
	ld d,a ;High byte
ret

cback:
	xor a
	ld a,b
	rlca
	rlca
	ld l,a
	ld h,0
	ld de,bb_coordstack
	add hl,de
	ld de,bb_ocoords
	ld c,4
	bb_copyback:
		ld a,(hl)
		ld (de),a
		inc hl
		inc de
		dec c
	jp nz,bb_copyback
	
	ld hl,(bb_oldoff+2)
	ld (bb_oldoff),hl ;mini stack thing for the ceiling mode (will have to fix it)

		
ret


bbredir:
	ld a,(hbl_redirct)
	cp 1
	jp nz,bb_nohblst1

		ld hl,(bb_oldoff)
	
		call UpdateScroll

		ld c,$99
		ld a,(bb_offscan+1)
		ld b,a
		ld a,(bb_oldoff+1) 
		add a,b
		out (c),a
		ld a,$93
		out (c),a

		
	jp bb_nohblst2
	bb_nohblst1:
		call removescroll
		ld c,$99
		ld hl,$1c86
		out (c),h 
		out (c),l ;Disable sprites without wrecking with the video ram
	
		ld c,$99
		ld de,$c093 ;Set floor positiion
		out (c),d
		out (c),e
bb_nohblst2:
	
	ld a,0; Just to avoid running the other redirs
ret

bb_boing: .db 0,$84,0,$20,$0,0,0,0
bb_curboing: .db 0,0
bb_boingcount: .db 0
bb_boingdir: .dw $100

bb_mini: .ds 8,0 ;Minime

InitBoing:
	ld a,(bb_postinit)
	cp 255
	jp z,bounce_init
	inc a
	ld (bb_postinit),a
	cp 5
	jp nz,bounce_init
		ld a,1
		ld (boss_bar_en),a
		ld a,28*2
		ld (bar_fill),a
		ld hl,boss_bar_life
		ld (bar_id),hl
		ld a,3
		ld (gamemode),a
		ld a,255
		ld (bb_postinit),a
		ld c,20
		ld a,0
		ld hl,bb_boing
		clebb:
			ld (hl),a
			inc hl
			dec c
		jp nz,clebb
		ld a,$84
		ld (bb_boing+1),a
		ld a,$20
		ld (bb_boing+3),a
		ld hl,$100
		ld (bb_boingdir),hl
		call 5
	bounce_init:
ret

