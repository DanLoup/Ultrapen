pen_x:.db 0,0,0
pen_y:.db 0,0
pentmp:.db 0,0,0,0,0,0

penphys:.db 0,0,$60, 0,$70, 0,0, 0,0, 24,24,0
	;physic object structure (X posH 0|fix 1 |posL 2), (Y fix 3|pos 4), vx (fix 5|pos 6),vy (fix 7|pos 8), sizx 9,sizy 10 , status return 11
pwalkloop: .db 4,5,6,5
cadi: .db 1 ;Cam dir pre thing
pframes: .db 0,0
inframe: .db 0,0
peninvis: .db 0
pdir: .db 0,0,0,0
penfrcool: .db 0
pwalkc: .db 0,0
penjumping: .db 0,0
penmode: .db 0,0 ;0 free range, 1 stairs, 2 knockback ,3 Slide , 4 dead, 5 celebrating
penstairedge: .db 0,0
penshooting: .db 0,0
penlimit: .db 0
penknock: .db 0
penblink: .db 0
penlife: .db 28
pendie: .db 0
penalt: .db 0
penaltx: .db 0
penalty: .db 0
penslide: .db 0,0
pencamlock: .db 0
penboot: .db 0

penlives: .db 5
pencoins: .db 0
penweapon: .db 0

penmenu: .db 0

penwenergy: .ds 12,28

HandlePen:
	ld a,$80
	cp b
	jp nz,@allocquestion
		ld b,0
		call CountInstances
		ld a,0
		cp c
		jp nz,@noextrapens
			ld a,$ff ; Set to fe to ask the system to reserve the next program as extra memory
		@noextrapens:
		ret ;Yep, i do want to join the mailing list, thanks
	@allocquestion:
	ld a,(penboot)
	cp 0
	jp nz,pen_is_booted

		ld a,1
		ld (penboot),a
		ld hl,$00
		ld de,$0919
		ld bc,$00
		call CreateSprite ;Create ultrapen's Object sprite
		ld c,0
		call SetAsVramSprite ;And tell the engine it's a vram sprite so the alternate copier will do it's thing
		ld a,(ix + 2)
		ld (penphys+2),a
		ld a,(ix + 3)
		ld (penphys),a
 		ld a,(ix + 4)
		ld (penphys+4),a
		
		ret 
	pen_is_booted:


;Initializer
	ld hl,0
	ld (penphys+5),hl
	ld a,1
	ld (cadi),a
	ld a,0
	ld (penalt),a
;**Free Range mode**


	ld a,(penmode)
	cp 0
	jp nz,nofreerange	



;Controls (Free range mode)
	ld a,(keys) 
	and 8
	jr z,noleft
		ld hl,-$140 ;-1 high -40 low
		ld (penphys+5),hl
		ld a,0
		ld (cadi),a
		ld a,1
		ld (pdir),a
		ld a,8
		ld (pdir+2),a
	noleft:
	ld a,(keys)
	and 2
	jr z,noright
		ld hl,$140 ;1 high 64 low
		ld (penphys+5),hl
		ld a,2
		ld (cadi),a
		ld a,0
		ld (pdir),a
		ld (pdir+2),a
	noright:
	call HandlePenLimit

	ld a,(keys)
	and 5
	jp z,noud
		ld a,(penstairedge)
		cp 0
		jp nz,noudorupdate ;Stop penguin from constantly getting in the stairs
		ld hl,(pen_x)
		ld a,(pdir)
		cp 0
		jr nz,rightenterst
			ld de,10
		jr leftenterst
		rightenterst:
			ld de,14
		leftenterst:
		
		add hl,de
		ld a,(pen_y) ;High tests
		ld d,a
		ld a,(keys)
		and 5
		cp 1
		jr nz,notgoupstairs;Only go up if pressing up
		call GetTileType
		cp 2
		jr z,itsstairs
		cp 3
		jr z,itsstairs
		notgoupstairs:
		ld a,(keys) ;You can only get in this one going down
		and 5
		cp 4
		jr nz,notstairs
		ld a,d ;Floor tests
		add a,24
		ld d,a
		call GetTileType
		cp 3
		jr nz,noenterfromup
		ld a,(pen_y)
		add a,8
		ld (pen_y),a
		jr itsstairs
		noenterfromup:
		
		jp notstairs
		itsstairs:
			ld c,1
			call ChangePenMode
			ld hl,(pen_x) ;Adjust penguin to stairs
			ld de,12
			add hl,de
			ld a,l
			and $f0
			ld l,a
			ex de,hl
			ld hl,(pen_x)
			sbc hl,de
			ld a,l
			ld (pdir+3),a
			ld (pdir+2),a
			ld a,(pen_y) ;And give a bit of a lift because the stairs sprite is taller
			sub 8
			ld (pen_y),a
			ld de,0
			ld (penphys+7),de ;Reseting physics to stop penguin from falling at maddening speeds if let go of stairs
			ld a,(pdir)
			ld (pentmp+3),a ;Store original direction
			ld a,0
			ld (pdir),a
			ld a,1
			ld (penjumping+1),a ;Set penguin to "jumping" when he gets on stairs to stop you from jumping from the stairs
			jp nofreerange ;Exit without touching physics
		notstairs:
	noud:
		ld a,0
		ld (penstairedge),a
	noudorupdate:
	
		
	ld a,(keys)
	and 32
	jr z,nojumpo ;not pressing button
		gotojump:
		ld a,(penphys+11)
		and 1
		ld a,1
		jr z,nojumpo ;Not on the floor
		ld a,(penjumping+1)
		cp 0
		jr nz,nojumpo ;already jumping

		
		ld a,(keys)
		and 4
		jr z,noslide
			ld c,3
			call ChangePenMode
			ld a,30
			ld (penslide),a
			ld a,1
			ld (penjumping+1),a
			ld hl,$1020
			ld (penphys+9),hl
			jr nojumpo
	noslide:
		ld hl,-$500 ;4 high 128 low
		ld (penphys+7),hl
		ld a,15
		ld (penjumping),a
		ld a,1
		ld (penjumping+1),a
nojumpo:
		ld (penstairedge+1),a

	;Jump cancel
	ld a,(penjumping)
	cp 0
	jr z,notjumping
	dec a
	ld (penjumping),a
	ld a,(keys)
	and 32
	jr nz,notjumping ;Still holding jump
	ld hl,-$100
	ld (penphys+7),hl;Whoops, player released button
	ld a,0
	ld (penjumping),a ;Better not hold penguin midair just because the jump got released
	notjumping:
	ld a,(keys)
	and 32
	jr nz,pressingjump;Reset the jump latch
	ld a,(penphys+11)
	and 1
	jr z,pressingjump;but only if released on the floor
	ld a,0
	ld (penjumping+1),a ;As we're here, why not reload the trigger?
	pressingjump:

;Shot handler
	ld a,(keys)
	and 16
	jr z,noshot
		ld a,(penshooting+1)
		cp 1
		jr z,yesshot
		ld b,1
		call penshot
		jr yesshot
noshot:
	ld a,0
	ld (penshooting+1),a ;Edge trigger
yesshot:


;Free range Animation handler	
	ld b,0 
	ld a,(cadi)
	cp 1
	jr z,standingp
		ld hl,pwalkloop
		ld a,(pwalkc)
		ld c,a
		ld b,0
		add hl,bc
		ld b,(hl)
		ld a,(pwalkc+1)
		inc a
		ld (pwalkc+1),a
		cp 7
		jr nz,standingp
		ld a,(pwalkc)
		inc a
		and $3
		ld (pwalkc),a
		ld a,0
		ld (pwalkc+1),a
	standingp:
	ld a,(penphys+11)
	and 1
	cp 1
	ld a,b
	jr z,anifloor
	ld a,10
	anifloor:
	ld (pframes),a

call runpenphys	


;Floor Nasty handler (4 spikes, 5 left roll, 6 right roll)
	ld hl,(pen_x) ;foot test
	ld de,8
	add hl,de
	ld a,(pen_y)
	add a,25
	ld d,a
	call GetTileType
	cp 1
	jp z,touchingfloor
	ld (pentmp+5),a


	cp 3
	jr nc,norighttest
		ld hl,(pen_x) ;foot test
		ld bc,16
		add hl,bc
		call GetTileType
		ld (pentmp+5),a
		cp 1
		jp z,touchingfloor

	norighttest:
	
	
	ld a,(pentmp+5)
	cp 4
	jr nz,nokillpenspike
		ld a,0
		ld (penphys+7),a
		ld (penphys+8),a ;Handling those falling corridors
		ld c,29
		call DamageUPen
	nokillpenspike:
	
	ld hl,(pen_x) ;foot test
	ld de,0
	add hl,de
	ld a,(pen_y)
	add a,25
	ld d,a
	call GetTileType
	ld (pentmp+5),a
	
	
	cp 3
	jr nc,norighttest2
		ld hl,(pen_x) ;foot test
		ld bc,32
		add hl,bc
		call GetTileType
		ld (pentmp+5),a
	norighttest2:
		
	ld (pentmp+5),a
	
	
	cp 5
	jr nz,penpushleft
		ld a,(penphys+2)
		ld l,a
		ld a,(penphys)
		ld h,a
		ld de,1
		sbc hl,de
		ld a,l
		ld (penphys+2),a
		ld a,h
		ld (penphys),a
		ld a,(cadi)
		cp 1
		jp nz,penpushleft
			ld a,0
			ld (cadi),a
	penpushleft:
	cp 6
	jr nz,penpushright
		ld a,(penphys+2)
		ld l,a
		ld a,(penphys)
		ld h,a
		ld de,1
		add hl,de
		ld a,l
		ld (penphys+2),a
		ld a,h
		ld (penphys),a

		ld a,(cadi)
		cp 1
		jp nz,penpushright
			ld a,2
			ld (cadi),a

	penpushright:
	
	touchingfloor:
nofreerange:
	call HandlePenLimit ;Still check, but for the transitions


;**Stairs Handler**
	ld a,(penmode)
	cp 1
	jp nz,nostairs
		
		ld a,(penshooting)
		cp 0
		jp nz,nosdown ;No up and down while shooting
		ld a,(keys)
		and 1
		jp z,nosup
			ld a,(pen_y)
			sub 1
			ld (pen_y),a
			ld a,(pwalkc+1)
			inc a
			ld (pwalkc+1),a
		nosup:
		ld a,(keys)
		and 4
		jp z,nosdown
			ld a,(pen_y)
			add a,1
			ld (pen_y),a
			ld a,(pwalkc+1)
			inc a
			ld (pwalkc+1),a
		nosdown:
	
	ld a,(keys) 
	and 8
	jp z,nosetleftst ;Decides which side ultrapen will be facing while shooting and exiting stairs
		ld a,1
		ld (pentmp+3),a
	nosetleftst:

	ld a,(keys) 
	and 2
	jp z,nosetrightst
		ld a,0
		ld (pentmp+3),a
	nosetrightst:
	
	ld a,(keys)
	and 16
	jp z,noshotst
		ld a,(penshooting+1)
		cp 1
		jp z,yesshotst
		ld a,(pentmp+3)
		ld (pdir),a
		ld b,1
		call penshot
		jp yesshotst
	noshotst:
		ld a,0
		ld (penshooting+1),a ;Edge trigger
	yesshotst:
	
	ld a,(penshooting)
	cp 0
	jp z,noforceshootst ;Massive kludge to save quite a few bytes (see below) while making the sprite face correctly
		ld a,(pentmp+3)
		xor 1
		ld (pdir),a
		jp forcedirst
	
	noforceshootst:
	
		ld a,(pwalkc+1)
		cp 10
		jp c,noclimbflip
	forcedirst:
			ld a,0
			ld (pwalkc+1),a
			ld a,(pdir+3) ;Old flip to flip
			ld b,a
			ld a,(pdir)
			xor 1
			ld (pdir),a
			rlca
			rlca
			rlca
			rlca
			add a,b ;Convert Pdir (direction) into sprite offset so climbing works
			ld (pdir+2),a
			
		noclimbflip:
			ld a,0
			ld (pentmp+2),a

		ld a,14
		ld (pframes),a
		ld hl,(pen_x) ;Head test
		ld de,12
		add hl,de
		ld a,(pen_y)
		add a,16
		ld d,a
		call GetTileType
		cp 0
		jp nz,nopenbottom
		ld a,d
		add a,8
		ld d,a
		call GetTileType
		cp 0
		jp z,nopenbottom
		ld a,(penshooting)
		cp 0
		jp nz,nopenbottom ;Cancel bottom mode while shooting to make more sense
			ld a,16
			ld (pframes),a
			ld a,(pen_y)
			and $f0
			add a,20
			ld (penalty),a
			ld a,2
			ld (penalt),a
			ld a,0
			ld (pdir),a
			ld a,(pdir+3) ;Old flip to flip
			ld (pdir+2),a
		nopenbottom:


			ld hl,(pen_x) 
			ld bc,2
			xor a
			sbc hl,bc
			ld a,(pen_y)
			add a,16
			ld d,a
			call GetTileType ;Test the penguin sides to determine if there walls, so you can offset the bird to the right side 
			cp 1
			jp nz,nostairleftadjust
				ld a,1
				ld (pentmp+2),a
			nostairleftadjust:
			ld bc,26
			add hl,bc
			call GetTileType
			cp 1
			jp nz,nostairrightadjust
				ld a,(pentmp+2)
				or 2
				ld (pentmp+2),a
			nostairrightadjust:
			ld a,(pen_y)
			cp 220
			jp nc,skipstairtests ;Do not test if the penguin can exit stairs when too high or low

			ld hl,(pen_x) ;Head test
			ld de,12
			add hl,de
			ld a,(pen_y)
			ld d,a
			call GetTileType

			ld (pentmp),a
			ld a,(pen_y)
			add a,24
			ld d,a
			call GetTileType
			cp 1
			jp z,Exitstairs2 ;Stepping on the floor

			ld e,a
			ld a,(pentmp)
			or e
			cp 0
			jp z,Exitstairs2 ;Midair penguin
			skipstairtests:
			ld a,(pentmp+2)
			cp 3
			jp z,nostairs ;Penguin is squeezed between two walls, so disable the jump exit to avoid the 24x24 penguin on 16xX hole scenario
			ld a,(keys)
			and 32
			cp 32
			jp z,trytoexitstairs
			ld a,0
			ld (penstairedge+1),a ;no jump no edge
			jp nostairs
			trytoexitstairs:
			ld a,(penstairedge+1) ;Stops you from falling from the stairs if you're holding jump as you land in em
			cp 1
			jp z,nostairs
			jp Exitstairs2
		
		Exitstairs:
		ld a,(pen_y)
		sub 16
		ld (pen_y),a
		Exitstairs2:
		ld a,(pentmp+2)
		ld hl,(pen_x)

		ld de,8
		cp 2
		jp nz,noffright
			xor a
			sbc hl,de
		noffright:
		;ld (pen_x),hl ;Penguin exit
		;call 5
		ld c,0
		call ChangePenMode
	nostairs:

	ld a,(penknock)
	cp 0
	jp z,notkickback
	and 4
	ld (peninvis),a

	ld a,(penmode) ;Pen knock back mode
	cp 2
	jp nz,notkickback
		ld a,(penknock)
		cp 30
		jp nz,nooutofmode
			ld a,0
			ld (penmode),a

		nooutofmode:

		ld a,(pdir)
		cp 0
		jp z,knockleft
			ld a,2
			ld (cadi),a	
			ld hl,$70 
			ld (penphys+5),hl
			jp noknockright
		knockleft:
			ld a,0
			ld (cadi),a
			ld hl,-$70
			ld (penphys+5),hl
		noknockright:
		call HandlePenLimit
		call runpenphys
	notkickback:


;**Slide Handler**
	ld a,(penmode)
	cp 3
	jp nz,nopenslide
		ld a,2
		ld (penalt),a
		ld a,(pen_y)
		sub 16
		ld (penalty),a
	ld a,0
	ld (penslide+1),a

	ld hl,(pen_x)
	ld de,2
	adc hl,de
	ld a,(pen_y)
	dec a
	ld d,a

	call GetTileType
	cp 1
	jp z,notinceilingslide
		ld bc,27
		add hl,bc
		call GetTileType
		cp 1
	jp z,notinceilingslide
		ld a,1
		ld (penslide+1),a
	notinceilingslide:
		ld hl,penslide
		dec (hl)
		jp nz,noendslide
			ld a,(hl)
			add a,4
			ld (hl),a
			ld a,(penslide+1)
			cp 0
			jp z,noendslide
			ld c,0
			call ChangePenMode
		noendslide:
		ld a,(pdir)
		cp 1
		jp nz,rightslide
			ld a,0
			ld (cadi),a
			ld hl,-$0280 
			ld (penphys+5),hl
			jp leftslide
		rightslide:
			ld a,2
			ld (cadi),a
			ld hl,$0280 
			ld (penphys+5),hl
		leftslide:
		ld a,(penslide+1)
		cp 0
		jp z,noslidecanceljp ;Still stuck under a ceiling
		ld a,(penjumping+1)
		cp 0 ;released jump
		jp z,noslidecanceljp
		ld a,(keys)
		and 36 
		cp 32 ;And pressed jump but with no slide
		jp nz,noslidecanceljp
			ld hl,0
			ld (penslide),hl
			ld a,0
			ld (penjumping+1),a

			ld c,0
			call ChangePenMode ;So we cancel the slide

		noslidecanceljp:
		call HandlePenLimit

		call runpenphys

		ld a,(penslide)
		cp 27
		jp nz,nopenslide ;Only smoke on frame 27
		;ld hl,penslide_eff
		ld a,0
		ld (penfrcool),a
		ld bc,(pen_x)
		ld a,(pen_y)
		add a,7
		ld d,a
		ld e,0
		ld a,(pdir)
		cp 0
		jp nz,noflipeffslide
		ld a,c
		sub 11
		ld c,a
		jp yesflipslide
		noflipeffslide:
		ld a,c
		add a,11
		ld c,a
		ld e,1
		yesflipslide:

;		ld ix,protable		
;		ld a,9 ;Slide effect, with bootstrap
;		ld (ix+0),a
;		ld a,2 ;slide 
;		ld (ix+1),a
;		ld (ix+2),b
;		ld (ix+3),c
;		ld (ix+4),d
;;		ld e,0 ;Side right
;		ld (ix+5),e ;Effect side
;		ld hl,protable
;		call SpawnProgram ;Boot structure: 0 it's a effect (set to 9), 1 effect ID, 2 screen Xh, 3 screen Xl, 4 screen Y, 5 side
		ld a,2
		call AddEffect ;A effect ID , bc X pos (16bit), D Y pos, E flip

		;call AddEffect ;AddEffect BC,X (ex),D y, HL pointer to effect type, E is flip (add 128 here)

nopenslide:


	ld a,(penknock)
	cp 0
	jp z,pennotimm
	dec a
	ld (penknock),a
	pennotimm:
	ld a,(penmode)
	cp 5
	jp nz,notcelebrating
		ld a,(pen_y)
		sub 4
		ld (penalty),a
		ld a,2
		ld (penalt),a
notcelebrating:

;Scroll handler
	ld hl,(pen_x) 
	ld de,116
	xor a
	ld a,l
	ld (pentmp),a
	sbc hl,de
	jp c,forceini ;If pen is under 116, lock camera
	ld a,(Hallsiz)
	cp 1
	jp z,forceini ;If a single room, no point at scrolling at all
	dec a
	ld d,a
	ld e,2
	xor a
	sbc hl,de
	jp nc,forceend ;Also lock the camera if on the last room

;	ld a,(pencamlock)
;	cp 0
;	jp nz,pennotstuck ;And finally, if the lock is set, don't pan
		ld hl,(pen_x)
		ld de,116
		xor a
		sbc hl,de
		ld a,(cadi)
		ld (CamDir),a
		ld (CamX),hl
		ld a,116
		ld (pentmp),a
	pencamstuck:

	jp noforce
	forceini:
	ld hl,0
	ld (CamX),hl
	jp noforce

	forceend:
	ld a,(Hallsiz)
	inc a
	ld h,a
	ld l,0
	ld (CamX),hl
	noforce:
	
;Camlock handler (mostly used to handle certain very specific level things)
;Also i should just implement this with a program thing instead of this "catch all solution" that just screw with the rest of the game
	ld a,(pencamlock)
	cp 1
	jp z,nounllockneeded
		ld a,(pen_x)
		cp 140
		jp nc,nounlockneeded
			ld a,0
			ld (pencamlock),a
	nounllockneeded:
	cp 2
	jp z,nounlockneeded
		ld a,(pen_x)
		cp 140
		jp c,nounlockneeded
			ld a,0
			ld (pencamlock),a
	nounlockneeded:
;Sprite position calculator
	ld a,(pentmp) 
	ld d,a
	ld a,(pdir+2)
	ld e,a
	ld a,d
	sub e ;Flip offset
	ld d,a
	ld a,(pen_y)
	ld e,a
	ld a,(penalt)
	and 1
	jp z,noaltx
		ld a,(penaltx)
		ld d,a
	noaltx:
	ld a,(penalt)
	and 2
	jp z,noalty
		ld a,(penalty)
		ld e,a
	noalty:
	ld a,(peninvis)
	cp 0
	jp z,notpeninvis
		ld e,224
	notpeninvis:
	ld c,0
	ld (globalvars),de ;Stealing the ultrapen local position
	call UpdateSprite



;Penguin blinker
	ld a,(pframes)
	cp 0
	jp nz,nopenblink
	ld a,(penblink)
	inc a
	and 127
	ld (penblink),a
	cp 124
	jp c,nopenblink
		ld a,1
		ld (pframes),a
	nopenblink:
	ld a,(penblink)
	cp 1
	jp nz,@penopeneye
		ld a,0
		call SetPenFace
	@penopeneye:
	ld a,(penblink)
	cp 125
	jp nz,@penocloseeyes
		ld a,1
		call SetPenFace
	@penocloseeyes:


;Penguin shot animation masker
	ld a,(penshooting)
	cp 0
	ld a,(pframes)
	jr z,noshotpenfr
	ld hl,shotmask
	ld e,a
	ld d,0
	add hl,de
	ld a,(hl)
noshotpenfr:
	ld (inframe),a

; shot countdown
	ld a,(penshooting)
	cp 0
	jr z,noshcountdown
	dec a
	ld (penshooting),a
	noshcountdown:
;Penguin hit animation masker
	ld a,(penmode)
	cp 2
	jr nz,nohitpenani
		ld a,(penknock)
		and 8
		ld a,17
		jp z,penhurtfr
		ld a,12
		penhurtfr:
		ld (inframe),a
	nohitpenani:

	ld a,(penmode)
	cp 3
	jr nz,noslideani
		ld a,13
		ld (inframe),a
	noslideani:
	
	ld a,(penmode)
	cp 5
	jr nz,noscelebanim
		ld a,18
		ld (inframe),a

	noscelebanim:


	
;Sprite graphics handler	
	ld a,(penfrcool)
	cp 0
	jp nz,norunframe

	ld a,(inframe+1) 
	ld b,a
	ld a,(inframe)
	cp b
	jp nz,newframe
	ld a,(pdir+1)
	ld b,a
	ld a,(pdir)
	cp b
	jp nz,newframe
	jp nonewframe
	newframe:
	ld c,0
	ld a,(inframe)
	ld d,a
	ld a,(pdir)
	ld e,a
	call UpdateSpriteFrame
	ld a,1
	ld (penfrcool),a

nonewframe:

;Old frame handler
	ld a,(inframe) 
	ld (inframe+1),a
	ld a,(pdir)
	ld (pdir+1),a
	jp nodec
norunframe:
	dec a
	ld (penfrcool),a
nodec:
	call CheckForPan
	call UpenCheckshots
	call UpenCheckPowerUps

;Ultrapen kill handler
	ld a,(pendie)
	cp 0
	jp z,nopendie
		inc a
		ld (pendie),a
		cp 120
		jp nz,nofadeout
			ld a,2
			ld (fadedir),a
		nofadeout:
		ld a,(pendie)
		cp 150
		call z,ResetLevel
nopendie:

ld a,(CurHalnb) ;Hard coded waypoints
cp 5
jp nz,nowp1
	ld (waypoint),a
nowp1:


ld a,(CurHalnb) ;Hard coded waypoints
cp 7
jp nz,nowp2
	ld (waypoint),a
nowp2:

ld a,(CurHalnb) ;Hard coded waypoints
cp 11
jp nz,nowp3
	ld (waypoint),a
nowp3:


	ld a,(keys) ;Handle weapon menu
	and 64
	jp z,nopausemenu
		ld a,(penmenu)
		cp 1
		jp z,nopausemenu
			call SetMenuMode
	nopausemenu:


ret
fun: .db 0,0,0,0

runpenphys:
;Physics transferer
	ld a,1 
	ld (CamDir),a
	ld a,(penphys)
	ld (pen_x+1),a
	ld a,(penphys+2)
	ld (pen_x),a
	ld a,(penphys+4)
	ld (pen_y),a

;Physics handler

	ld hl,penphys
	call CalcPhysics
ret

;B adjusts the shot height
penshot:
		ld a,(penweapon)
		ld hl,shuse
		ld e,a
		ld d,0
		add hl,de
		ld a,(hl)
		ld hl,penwenergy
		add hl,de
		ld d,a
		ld a,(hl)
		cp 0
		ret z;Weapon is empty so no dice
		sub d
		cp 30
		jp c,@zeroedweapon
			ld a,0
		@zeroedweapon:
		ld (hl),a ;decreasing the weapon
		call updateweaponbar	

		ld (pentmp+4),bc 
		ld c,1
		;call CountShot
		ld a,2 ;Do not shoot more than 3
		cp b
		ret c ;Shot overflow
		ld a,10
		ld (penshooting),a
		ld a,1
		ld (penshooting+1),a
		ld hl,(pen_x)
		ld de,(CamX)
		sbc hl,de
		ld bc,(pentmp+4)

		ld a,(pdir)
		cp 1
		jp z,pshotleft
		ld a,l ;Right shot
		add a,24
		ld d,a
		ld a,(pen_y)
		add a,9
		add a,b
		ld e,a
		ld b,0
		ld a, (penweapon)
		cp 0
		jp z,@nocoin1l
			ld b,1
		@nocoin1l:

		ld ix,protable		
		ld a,8 ;Shot, with bootstrap
		ld (ix+0),a
		ld (ix+1),b
		ld (ix+2),d
		ld (ix+3),e
		ld a,0 ;Side right
		ld (ix+4),a
		ld hl,protable
		call SpawnProgram;Boot structure: Shot Type, screen X,screen Y, side

		ld de,0
		ld bc,$0400
		;call SetShotVector	;A shotID, BC VX, DE VY
		jp endshot
		pshotleft:
		ld a,l ;Left shot
		cp $10
		jp c,noshot
		sub 8
		ld d,a
		ld a,(pen_y)
		add a,9
		add a,b
		ld e,a
		ld b,0
		ld a, (penweapon)
		cp 0
		jp z,@nocoin1r
			ld b,1
		@nocoin1r:

		ld ix,protable		
		ld a,8 ;Shot with bootstrap as well
		ld (ix+0),a
		ld (ix+1),b
		ld (ix+2),d
		ld (ix+3),e
		ld a,1 ;Side left
		ld (ix+4),a
		ld hl,protable
		;call 5
		call SpawnProgram

		;call AddShot ; b shottype+target ,de X/Y (high)
		ld de,0
		ld bc,$FAFF
		;call SetShotVector	;A shotID, BC VX, DE VY
		endshot:
ret

;C target mode
ChangePenMode:
	push bc ;Mode exit
	ld a,(penmode)
	cp 1
	jp nz,noexitstairs
		;ld a,0
		;ld (penmode),a
		ld a,(pen_x+1)
		ld (penphys),a
		ld a,(pen_x)
		ld (penphys+2),a
		ld a,(pen_y)
		ld (penphys+4),a
		ld a,0
		ld (pwalkc+1),a
		ld a,1
		ld (penstairedge),a
		ld a,(pentmp+3) ;restore original direction
		ld (pdir),a
		ld b,0
		cp 1
		jp nz,flipsetstairs
		ld b,8
		flipsetstairs:
		ld a,b
		ld (pdir+2),a
noexitstairs:

	ld a,(penmode)
	cp 3
	jp nz,noexitslide
	ld hl,$1818
	ld (penphys+9),hl
	ld a,(penphys+4)
	sub 8
	ld (penphys+4),a
	ld a,(pen_y)
	sub 8
	ld (pen_y),a
	ld a,0
	ld (penalt),a
	ld (penfrcool),a

	noexitslide:
	
	pop bc
	ld a,c
	ld (penmode),a
ret



temppeng: .db 0,0,0,0,0,0,0,0
shotmask: .db 2,2,2,2,7,8,9,7,8,9,11,11,12,13,15,15,16

l_setpan: .db 0,0,0,0
HandlePenLimit:
;pen_x:.db 0,0,0 (i think it's low,high, page)
;pen_y:.db 0,0 (low ,high?)
	;Thanks to Ultrapen being a chunky boy of 24x24, i (probably) will not need to check for advanced negative stuff
	;For example, from 244 to 255 means it is actually trying to go the left, while 231 to 243 means it is trying to go to the right
	;for Y, same thing for going up, but 168 to 192 for going down

	;	Check for left boundary

	ld a,0 
	ld (l_setpan),a
	ld a,(penphys)
	cp 0
	jp nz,NoXleft
	ld a,(penphys+2)
	cp 9
	jp nc,NoXleft
		ld hl,$0030
		ld (penphys+5),hl
		ld a,1+1
		ld (l_setpan),a
	NoXleft:
	;Check for right boundary
	ld a,(Hallsiz)
	ld b,a 
	dec b
	ld a,(penphys)
	cp b
	jp nz,NoXright
	ld a,(penphys+2)
	cp 230
	jp c,NoXright
		ld hl,$FF50
		ld (penphys+5),hl
		ld a,0+1
		ld (l_setpan),a
	NoXright:
	;Check for Down boundary
	ld a,(pen_y)
	cp 190+24
	jp c,NoXdown
		ld hl,$0000
		ld (penphys+7),hl
		ld a,2+1
		ld (l_setpan),a
	NoXdown:
	;Check for Up boundary
	ld a,(pen_y)
	cp 237

	jp c,NoXup
		ld hl,$0050
		ld (penphys+7),hl
		ld a,3+1
		ld (l_setpan),a
	NoXup:
ret



pendiebyhole: .db 0
CheckForPan:
		ld a,(l_setpan)
		cp 0
		jp z,nopan
		cp 3
		jp nz,nodiehole
			ld a,1
			;ld (pendiebyhole),a

		nodiehole:

;		ld hl,(Halladdr)
;		ld c,(hl)
;		ld a,0
;		cp c
;		jp z,nopan
;		inc hl
;		exitsl:
;			ld a,(l_setpan)
			dec a
;			ld b,(hl)
;			inc hl
;			cp b
;			jp nz,notthisexit ;Wrong direction
;			ld b,(hl)
;			ld a,(penphys)
;			cp b
;			jp nz,notthisexit ;Wrong Screen
;				;Actual structure
;				ld a,0
;				ld (pendiebyhole),a
;				ld a,(l_setpan)
;				dec a
;				ld c,a ;Direction to get there
;				inc hl
;				ld e,(hl) ;Next course
;				inc hl
;				ld d,(hl) ;Next screen
;				call SetPanMode
;
;			jp nopan ;There is a pan but you know
;			notthisexit:
;		inc hl
;		inc hl
;		inc hl
;		dec c
;		jp nz,exitsl
nopan:
	ld a,(pendiebyhole)
	cp 1
	ret nz
	ld c,29
	call DamageUPen
	ld a,0
	ld (pendiebyhole),a
ret


penpanf: .db 0,0,0

HandlePenPan:	
	;Pancount
	;Pandir
	ld a,(penpanf+2)
	inc a
	cp 2
	jp nz,@resetadj
		ld a,0
	@resetadj:
	ld (penpanf+2),a

	ld a,0
	ld (penpanf+1),a
	ld a,(penpanf)
	inc a
	cp 3
	jp nz,resetor
	ld a,1
	ld (penpanf+1),a
	dec a
	resetor:
	ld (penpanf),a

	ld a,(Pandir)
	ld e,a
	ld a,0
	cp e
	jp nz,HPPnright
		;C Object ID, DE X,Y
		ld c,0
		call GetSpritePosition
		ld a,(penpanf+1)
		add a,d
		sub 3
		ld d,a
		ld a,(penpanf+2)
		cp 0
		jr nz,@noadjustright
			inc d
		@noadjustright:
		call UpdateSprite

		ld a,(Pancount)
		cp 1
		jp nz,HPPnright
		ld bc,$1000
		ld (penphys+1),bc 
	HPPnright:

	ld a,1
	cp e
	jp nz,HPPnleft
		ld c,0
		call GetSpritePosition
		ld a,(penpanf+1)
		xor 1
		add a,d
		add a,2
		ld d,a
		ld a,(penpanf + 2)
		cp 0
		jr nz,@noadjustleft
			dec d
		@noadjustleft:

		call UpdateSprite

		ld a,(Pancount)
		cp 1
		jp nz,HPPnleft
		ld bc,$E000
		ld (penphys+1),bc
	HPPnleft:

	ld a,2
	cp e
	jp nz,HPPndown
		ld bc,$2000
		ld (penphys+3),bc
		ld a,$20
		ld (pen_y),a
		ld hl,$0350
		ld (penphys+7),hl
		call GetSpritePosition
		ld a,(penpanf+1)
		add a,e
		sub 3
		ld e,a
		call UpdateSprite
	HPPndown:
	
	ld a,3
	cp e
	jp nz,HPPnup
		ld bc,$b000
		ld (penphys+3),bc
		ld a,$b0
		ld (pen_y),a
		ld c,0
		call GetSpritePosition
		ld a,(penpanf+1)
		xor 1
		add a,e
		add a,2
		ld e,a
		call UpdateSprite
	HPPnup:
	ld a,(PanDpage)
	ld (penphys),a
	ld (pen_x+1),a ;When you leave stairs, you go to this magical place known as "the last screen"
ret



	;A320 seems to be where it is
	;D800 is where the color tables of the bar are
UpenCheckshots:
	ld c,0
	call GetSpritePosition
	ld hl,$1818
	ex de,hl
	ld c,$20
	call QueryDots ; HL position, DE size,C dot type (only high nibble),A to dot ID if found
	cp 255
	jp z,noshotupen
	ld e,a
	call GetDotType
	and $f
	ld c,a
	ld b,$ff
	call ChangeDot ;Kill the dot

	ld a,0
	cp c
	jp nz,@noinsta
	ld c,28
	@noinsta:
	;ld c,2
	
	call DamageUPen
	noshotupen:
ret

UpenCheckPowerUps:

	ld c,0
	call GetSpritePosition
	ld hl,$1818
	ex de,hl
	ld c,$30
	call QueryDots ; HL position, DE size,C dot type (only high nibble),A to dot ID if found
	cp 255
	jp z,nopowerupen
	ld e,a
	call GetDotType
	and $f
	ld c,a
	ld b,$ff
	call ChangeDot ;Use the power up
	ld d,$ff
	ld a,0
	cp c ;Small life up
	jp nz,@nosmalll
	ld a,(penlife)
	add 2
	ld (penlife),a
	ld l,a
	ld h,0
	jp @end
	@nosmalll:
	inc a
	cp c ;Big life up
	jp nz,@nobigl
	ld a,(penlife)
	add 16
	ld (penlife),a
	ld l,a
	ld h,0
	jp @end
	@nobigl:
	inc a
	cp c ;Small energy up
	jp nz,@nosmalle
	ld hl,penwenergy
	ld d,0
	ld a,(penweapon)
	cp 0
	jp z,@noupdate
	ld e,a
	add hl,de
	ld a,(hl)
	add 2
	cp 29
	jp c,@noclp1
		ld a,28
	@noclp1:
	ld (hl),a
	ld l,a
	ld h,1

	jp @end
	@nosmalle:
	inc a
	cp c ;Big energy up
	jp nz,@nobige
	ld hl,penwenergy
	ld d,0
	ld a,(penweapon)
	cp 0
	jp z,@noupdate
	ld e,a
	add hl,de
	ld a,(hl)
	add 16
	cp 29
	jp c,@noclp2
		ld a,28
	@noclp2:
	ld (hl),a
	ld l,a
	ld h,1
	jp @end
	@nobige:
	inc a
	cp c ;One coin
	jp nz,@noocoin
	ld a,(pencoins)
	inc a
	ld (pencoins),a
	ld l,a
	ld h,3
	jp @end
	@noocoin:
	inc a
	cp c ;Five coins
	jp nz,@noficoin
	ld a,(pencoins)
	add 5
	ld (pencoins),a
	ld l,a
	ld h,3
	jp @end
	@noficoin:
	inc a
	cp c ;fishcan
	jp nz,@nofican
	jp @end
	@nofican:
	inc a
	cp c ;squidcan
	jp nz,@nosquican
	jp @end
	@nosquican:
	inc a
	cp c ;life can
	jp nz,@nolifecan
	jp @end
	@nolifecan:
	@end:
	ld a,(penlife)
	cp 29
	jp c,@nofixpl
	ld a,28
	ld l,28 ;Because 
	ld (penlife),a
	@nofixpl:
	ld a,(pencoins)
	cp 100
	jp c,@nofixcoin
	ld a,99
	ld l,99 ;Because 
	ld (pencoins),a
	@nofixcoin:


	ld a,255
	cp h
	jp z,@noupdate
	call UpdateStatusBar

	
	@noupdate:
	nopowerupen:

ret




penreset: .db 0,0,$60,0,$70
; C damage number
DamageUPen:
	ld a,(pendie)
	cp 0
	ret nz ; You can't damage the dead
	ld a,(penknock)
	cp 0
	ret nz ;no hitting an immortal penguin 
	ld a,60
	ld (penknock),a ;Immortal time
	push bc
	ld c,2
	call ChangePenMode

	;ld hl,penhit_eff
	ld bc,(pen_x)
	ld a,(pen_y)
	sub 16
	ld d,a
	ld e,0
	ld a,1
	call AddEffect ;AddEffect BC,X (ex),D y, E is flip (add 128 here)

	pop bc
	xor a
	ld a,(penlife)
	sbc a,c
	ld (penlife),a
	ld a,(pen_y) ;Mostly for being hit on stairs, but inconsequent otherwise
	ld (penphys+4),a

	jp z,zerokillstoo
	jp nc,zeroupendamage
		zerokillstoo:
		ld a,1
		ld (pendie),a
		ld a,0
		ld (penlife),a
		ld a,4
		ld (penmode),a
		;ld a,1
		;ld (peninvis),a
	
		;ld hl,enepop_eff
		ld bc,(pen_x)
		ld a,(pen_y)
		ld d,a
		ld e,0
		ld a,0
		call AddEffect ;AddEffect BC,X (ex),D y, E is flip (add 128 here), A effect id

		ld hl,(pen_x)
		ld a,(pen_y)
		ld c,a
		;call Explode ; hl/X,c/Y 
		ld ix,protable		
		ld a,10 ;Penguin death
		ld (ix+0),a
		ld (ix+2),b
		ld (ix+3),c
		ld (ix+4),d
		ld hl,protable
		call SpawnProgram	

		ld a,224
		ld (pen_y),a
	zeroupendamage:
	ld h,0
	ld a,(penlife)
	ld l,a
	call UpdateStatusBar
ret

hittmp: .ds 6,0
;DE X/Y,HL x/y Size
;A is 1 if bounding box hits upen
IsHittingUPen:
	ld a,0
	ld (hittmp),de
	ld (hittmp+2),hl
	ld c,0
	call GetSpritePosition ;Sets DE with local ultrapen graphics
	ld a,(hittmp+1) ;test X
	sbc a,d ;X is now on upen
	ld d,a
	ld a,(hittmp+3);And now it's not
	add a,d
	ld d,a
	ld a,(pdir)
	cp 1
	jp nz,nopenflip ;When penguin is flipped, there's an offset on the sprite coordinates
		ld a,d
		sub 8
		ld d,a
	nopenflip:
	ld a,(hittmp+3) ;Size X of bb
	add a,24; Add size of Upen
	cp d ;If adjusted X is inside
	ld a,0
	ret c
	ld a,(hittmp) ;test Y
	sbc a,e ;Y is now on upen
	ld e,a
	ld a,(hittmp+2) ;Size Y of bb
	add a,e
	ld e,a
	ld a,(hittmp+2) ;Size Y of bb
	add a,24; Add size of Upen again
	cp e ;If adjusted Y is inside
	ld a,0
	ret c
	ld a,1
ret

;penphys:.db 0,0,$60, 0,$70, 0,0, 0,0, 24,24,0

InitPen:
		ld a,28
		ld (penlife),a
		ld hl,penreset
		ld de,penphys
		ld bc,5
		ldir
		ld hl,0
		ld a,0
		ld (pendie),a
		ld (peninvis),a
		ld hl,0
		ld (CamX),hl
		ld (penmode),hl
		ld (penknock),hl
		ld a,2
		ld (CamDir),a

		ld a,(ix + 2) ;Reseting the penguin to the actual position
		ld (penphys+2),a
		ld a,(ix + 3)
		ld (penphys),a
 		ld a,(ix + 4)
		ld (penphys+4),a

		ld h,0
		ld a,(penlife)
		ld l,a
		call UpdateStatusBar

;		ld c,8
;		ld a,0
;		d hl,penphys
	
;	Resetpenpos:
;		ld (hl),a
;		inc hl
;		dec c
;		jp nz,Resetpenpos
;		ld a,$60
;		ld (penphys+2),a
;		ld a,$70
;		ld (penphys+4),a
		
	ret

;H Mode (0 life, 1 weapon, 2 lives, 3 coins, 4 enemy bar), L value (255 on weapon erase the bar)
UpdateStatusBar:
	ld a,h
	and 2
	jp nz,@numberupdate
	;It's a bar graphics update
		ld de,$0ebe
		ld a,h
		and 1
		jp z,@notdown
			ld e,$c5
		@notdown:
		ld a,h
		and 4
		jp z,@notright
			ld d,$c2
		@notright:
		ld a,l
		cp $ff
		jp z,@noblank
			ld a,56
			sub l
			sub l
			add $14
			ld h,a		
			ld l,$52
			jp @noblankatall
		@noblank:
			ld hl,$434c
		@noblankatall:
		ld bc,$3806
		ld iyl, $1
		call AddBlitterCopy
ret
	@numberupdate:
	;It's a number update
	ld de,$5ebd
	ld a,h ;Before i wreck h
	and 1
	jp z,@lives
		ld e,$c5
	@lives:
	ld c,0 ;this friend will do the job
	ld a,l
	@tens:
		sub 10
		jp c,@etens ;Loop until it's a negative number, which could be before anything
		inc c ;high number gets added
		jp @tens
	@etens:
	add 10 ;Go back from negative
	push af ;storing d (and e incidentally) for later
	ld iyl,$1
	ld a,c
	cp 0
	jp nz,@skipnumber
		pop af
		push hl
		push de
		push af
		ld hl,$434c
		ld bc,$0405
		ld d,$5e+5
		call AddBlitterCopy
		pop af
		pop de
		pop hl
		ld iyl,$2
	@skipnumber:
	@repeateonce:
		ld bc,$0405
		;call 5
		ld l,$40
		cp 5
		jp c,@lower5
			ld l,$45
			sub 5
		@lower5:
		rla
		rla ;multiply by 4
		and 60
		add $14 ;and sum it to the left side of the letters
		ld h,a
		call AddBlitterCopy
		ld a,iyl
		cp $1
		ret nz
		ld d,$5e+5
		ld iyl,$2
		pop af
	jp @repeateonce
ret
;a face
SetPenFace:
	cp 0
	jp nz,@noneutral
		ld bc,$0c03
		ld hl,$0447
		ld de,$7bc2
		call AddBlitterCopy
	ret
	@noneutral:
	cp 1
	jp nz,@noblink
		ld bc,$0c03
		ld hl,$0053
		ld de,$7bc2
		call AddBlitterCopy
	ret
	@noblink:
ret

memem: .db 0,0,0,0,0,0,0,0,0,0,0,0

SetMenuMode:
	ld a,1
	ld (penmenu),a
	ld (pause),a
	ld a,0
	ld (memem + 1),a ;Stage
	ld (memem + 6),a ;Selected weapon
	ld (memem + 5),a
	ld (memem + 4),a

	ld a,2
	ld (fadedir),a ;setting the fade out
ret

RunMenuMode:
	ld a,(memem + 1)
	cp 0
	jp nz,@nost1 ;Fade out
		ld a,(memem + 5)
		inc a
		ld (memem + 5),a
		cp 20
		jp nz,@fadd1
			ld a,1
			ld (memem + 1),a
		@fadd1: ;Just a delay to wait for the fade out
		jp @endrm
	@nost1:
	ld a,(memem + 1)
	cp 1
	jp nz,@nost2 ;complete vram destruction
		ld a,1
		ld (dsprites),a ;Well, gotta remove sprites too
		ld a,2
		ld (memem + 1),a
		ld ix,fontdt
		ld iy,$ffff
		call writeTileData
		jp @endrm
	@nost2:
	ld a,(memem + 1)
	cp 2
	jp nz,@nost3 ;more VRAM destruction (and construction)
		di
		ld hl,0
		call UpdateScroll

		ld a,3
		ld (memem + 1),a
		ld de,0
		call locate
		ld c,$98 ;Time to draw the borders
		call drawub
		ld d,15
		@lp1:
			call drawmb
			dec d
		jp nz,@lp1
		call drawdb
		call drawub
		ld d,4
		@lp2:
			call drawmb
			dec d
		jp nz,@lp2
		call drawdb
		call drawmenu
		ei
		ld a,0
		ld (fadedir),a ;menu done drawing, fade in
		jp @endrm
	@nost3:
	ld a,(memem + 1)
	cp 3
	jp nz,@nost4
		ld a,(memem + 5)
		inc a
		and 31
		ld (memem + 5),a
		and 15
		cp 15
		jp nz,@meinter
			call drawmenu
			ld a,(memem + 5)
			and 16
			jp z,@meinter
				xor a 
				ld a,(memem + 7)
				add 2
				ld c,a
				ld a,(memem + 6)
				add 1
				rrca
				rrca
				ld d,a
				and 128+64
				add c ;X, will need to do the whole reload dance later
				ld e,a
				ld a,d
				and 63
				ld d,a ;Multiply the number of A by 64 with a back shift, 
				call locate
				ld hl,wbl
				call print ;Disgusting but easy
		@meinter:
		ld a,(keys)
		ld b,a
		ld a,(memem + 8)
		and b
		jp nz,@nokeys
		ld a,(keys)
		and 1
		jp z,@up
			ld a,(memem + 6)
			dec a
			cp 255
			jp nz,@noureset
				ld a,5
			@noureset:
			ld (memem + 6),a
			ld a,24
			ld (memem + 5),a
		@up:
		ld a,(keys)
		and 4
		jp z,@down
			ld a,(memem + 6)
			inc a
			cp 6
			jp nz,@nodreset
				ld a,0
			@nodreset:
			ld (memem + 6),a
			ld a,24
			ld (memem + 5),a
		@down:
		ld a,255
		ld (memem + 8),a
		
		ld a,(keys)
		and 2+8
		jp z,@nolr ;Both sides do the same thing, lol, but will have to think better when we get to etank code
			ld a,(memem + 7)
			xor 13
			ld (memem + 7),a
			ld a,24
			ld (memem + 5),a ;boosts the menu visual responsiviness
		@nolr:
		ld a,(keys)
		and 16
		jp z,@sele
			ld a,4
			ld (memem + 1),a
			ld a,2
			ld (fadedir),a ;Fading out to reset
			ld a,0
			ld (memem + 5),a;and giving it a timer
			ld a,(memem + 6)
			ld b,a
			ld a,(memem + 7)
			cp 13
			jp nz,@nosecondrow
				ld a,6
			@nosecondrow:
			add b
			ld (penweapon),a
		@sele:

		@nokeys:
		ld a,(keys)
		ld b,a
		ld a,(memem+8)
		and b
		jp nz,@noreset
			ld (memem+8),a ;8 is used as a edge trigger, so you can't select multiple objects at once
		@noreset:
		jp @endrm
	@nost4:

	ld a,(memem + 1)
	cp 4
	jp nz,@nost5 ;Fade out
		ld a,(memem + 5)
		inc a
		ld (memem + 5),a
		cp 20
		jp nz,@fadd2
			ld a,5
			ld (memem + 1),a
		@fadd2: ;Just a delay to wait for the fade out again
 		jp @endrm
	@nost5:

	ld a,(memem + 1)
	cp 5
	jp nz,@nost6
		ld a,0
		ld (penmenu),a
		ld (pause),a
		ld (dsprites),a
		ld ix,($8400)
		ld hl,($8400)
		ld de,$800
		add hl,de
		push hl
		pop iy
		call writeTileData
		ld a,(CamObuffpos)
		add 128
		ld (CamObuffpos),a ;hey engine, i'm pretty sure the player moved a lot, can you update it?
		ld a,0
		ld (fadedir),a ;All done, final fade in
		;Now let's set the penguin color for reasons
		ld a,(penweapon)
		add a;multiply by 2
		ld e,a
		ld d,0
		ld hl,wecols
		add hl,de
		ld de,palette + 9*2
		ld bc,2
		ldir
	
		ld hl,(sc_offs) ;Restore scroll
		call UpdateScroll
		call updateweaponbar
		jp @endrm
	@nost6:
	@endrm:
ret

;The it's nothing, green, red , blue lol, also values form 0 to 7
wecols: .dw $0007,$0002,$0450,$f444,$f024,$f050,$f231,$f111,$f262,$0462,$0260,$0260,$0260

wen: .db "b.shot",0, "ink blast",0, "gold coin",0,"s.salt",0,"g.spirit",0,"f.mask",0,"p.welder",0,"briefcase",0,"c.fan",0,"h.cannon",0,"h.shield",0,"h.rocket",0
wbl: .db "            ",0
drawmenu:
		ld de,2+(2*32) ;X and Y lol
		ld (memem+2),de
		ld a,12
		ld (memem+4),a
		ld hl,penwenergy
		ld a,(penlife)
		ld (hl),a ;As we're here, might update the pen weapon to display the life
		ld (memem+10),hl
		ld hl,wen
		@medraw:
			call locate
			call print
			push hl
			ld hl,32
			add hl,de
			ex de,hl
			call locate
			call drawbarn
			ld hl,32
			add hl,de
			ex de,hl
			pop hl

			ld a,(memem+4)
			dec a
			ld (memem+4),a
			cp 6
			jp nz,@noseline
				ld de,15+(2*32)
			@noseline:
			cp 0
		jp nz,@medraw
ret

drawub:
	ld a,7
	out (c),a
	ld b,29
	ld a,5
	@fi1:
		out (c),a
		dec b
	jp nz,@fi1
	ld a,8
	out (c),a
	ld a,0
	out (c),a
ret

drawdb:
	ld c,$98
	ld a,9
	out (c),a
	ld b,29
	ld a,5
	@fi1:
		out (c),a
		dec b
	jp nz,@fi1
	ld a,10
	out (c),a
	ld a,0
	out (c),a
ret

drawmb:
	ld c,$98
	ld a,6
	out (c),a
	ld b,29
	ld a,15
	@fi1:
		out (c),a
		dec b
	jp nz,@fi1
	ld a,6
	out (c),a
	ld a,0
	out (c),a
ret

locate:
	push hl
	ld hl,$4000
	ld a,(CamDbl)
	cp 0
	jp nz,@sc1
		ld hl,$4400
	@sc1:
	add hl,de
	ld c,$99
	out (c),l
	out (c),h ;Selects the current video screen (not the previous one)
	pop hl
ret

print:
	ld c,$98
	ld a,(hl)
	cp 0
	jp z,@eprint;Printing done
	cp 95
	jp c,@char
		sub 96-15 ;lower case letters
		jp @tx
	@char:

	cp 48
	jp c,@numb
		sub 48-(27+16) ;lower case letters
		jp @tx
	@numb:

	cp 32
	jp nz,@space
		ld a,15
	@space:
	cp 46
	jp nz,@dot
		ld a,16+26
	@dot:

	@tx:
	out (c),a
	inc hl
	jp print
	@eprint:
	inc hl ;Just to allow multiple entry lists
ret 

;d = total number
drawbarn:
	push hl
	push de
	push bc
	ld b,7
	ld hl,(memem+10)
	ld d,(hl)
	inc hl
	ld (memem+10),hl
	ld c,$98
	@lp:
		ld a,d
		cp 4
		jp c,@notop
			ld a,4
		@notop:
		out (c),a
		ld a,d
		sub 4
		cp 28
		jp c,@nozerobar
			ld a,0
		@nozerobar:
		ld d,a
		dec b
	jp nz,@lp
	pop bc
	pop de
	pop hl
ret

writeTileData:
	ld c,$99
	ld de,$008e ;First first thing, set page to zero
	out (c),d
	out (c),e

	ld c,$99 ;Load the 1 bit bitmap data to the 3 tile maps 
	ld hl,$4000 
	ld e,4
	@LLoop1:
		ld c,$99
		out (c),l
		out (c),h ;the bit of $40 tells the VRAM it's a write
		push hl
		push ix
		pop hl
		ld bc, $0098
		otir 
		otir ;Only 512 bytes will be wrecked
		pop hl
		ld bc,$800 
		add hl,bc 
		dec e
	jp nz,@LLoop1
	

	ld c,$99 ;Same thing, but for the Color map thing
	ld de,$008e ;Second second thing, set page to zero again
	out (c),d
	out (c),e
	ld hl,$6000
	ld e,4
	ld a,iyl
	cp $ff
	jp z,@fontset
	@LLoop2:
		ld c,$99
		out (c),l
		out (c),h
		push hl
		push iy
		pop hl
		ld bc, $0098
		otir
		otir
		ld bc,$800
		pop hl
		add hl,bc
		dec e
	jp nz,@LLoop2
	ret
	@fontset:
		ld c,$99
		out (c),l
		out (c),h
		push hl
		ld bc, $0598
		@f0:
			ld a,10*16
			out (c),a
			out (c),a
			ld a,15*16
			out (c),a
			ld a,10*16
			out (c),a
			out (c),a
			out (c),a
			out (c),a
			out (c),a
			dec b
		jp nz,@f0
		ld bc, $e098
		@fs:
			ld a,(15*16)+7
			out (c),a
			out (c),a
			dec b
		jp nz,@fs

		ld bc,$800
		pop hl
		add hl,bc
		dec e
	jp nz,@fontset
ret
updateweaponbar:
	push hl
	push de
	push bc
	ld a,(penweapon)
	cp 0
	jp z,@nobar
	ld e,a
	ld d,0
	ld hl,penwenergy
	add hl,de
	ld a,(hl)
	ld l,a
	ld h,1
	call UpdateStatusBar
	pop bc
	pop de
	pop hl
ret
	@nobar:
	ld h,1
	ld l,255
	call UpdateStatusBar
	pop bc
	pop de
	pop hl
ret

PenguinDeath:
	ld a,$80
	cp b
	jp nz,@allocquestion
	ld a,$ff
	ret ;Please, do allocate me in all occasions
	@allocquestion:
	ld a,(ix + 10)
	cp 0
	jp nz,@noboot
		ld a,123
		ld (ix + 10),a
		call Explode
		ret
	@noboot:
	ld a,(explomode)
	cp 0
	jp nz,@noendyet
		ld a,255
		ld (ix + 0),a ;Program ended
		ld iy,10
		;call 5
		ld ix,expdata
		@clearsp:
			ld c,(ix + 0)
			ld a,255
			cp 255
			call nz,RemoveSprite
			inc ix
			dec iy
		jp nz,@clearsp
		ret
	@noendyet:
	call runexplosion

ret


explomode: .db 0 ; 0 counter
expdata: .ds 128,0
tmpeff: .ds 32,0
expvt: .db $21,$53, $22,$65, $11,$74 ,$14,$52 ,$25,$64 ,$22,$73, $15,$50 ,$5,$43, $25,$70 , $9,$55  ;20 bytes

; hl/X,c/Y 
Explode:
	ld hl,(pen_x) ; temp stuff 1
	ld a,(pen_y) ;Temp stuff 2
	sub 60
	ld c,a

	ld a,120
	ld (explomode),a
	ld de,(CamX)
	sbc hl,de
	ld d,c
	ld e,l
	ld (tmpeff+4),de
;HL Type/Subtype,DE X,Y,B Flip X,Y ,C Frame
;A returns The Object ID
	ld IYL,10
	ld ix,expdata
	floop1explo:
		ld l,$05
		ld de,(tmpeff+4)
		ld bc,0
		ld a,IYL
		cp 5
		jp nc,nochangetofr2
			ld c,3
		nochangetofr2:
		call CreateSprite
		ld c,a ;Returns the sprite in A, shove in C for set as vram sprite
		ld (ix+0),a
		call SetAsVramSprite
		inc ix
		dec IYL
	jp nz,floop1explo
	ld IYL,10
	ld hl,expdata+16
	ld de,expvt

	setcoordexpl:
			ld c,0 
		ld a,IYL
		and 1
		jp nz,gorightexpl
			ld c,$ff ;X should sometimes go left
		gorightexpl:
		
		call sbexpsetvec
		ld a,0
		ld (hl),a ;X low
		inc hl
		ld a,(tmpeff+4)
		ld (hl),a ;X pos high
		inc hl
		ld c,$ff ;Y is supposed to go up
		call sbexpsetvec
		ld a,0
		ld (hl),a ;Y low
		inc hl
		ld a,(tmpeff+5)
		ld (hl),a ;Y pos high
		inc hl
		dec IYL
		jp nz,setcoordexpl
ret
sbexpsetvec:
		xor a
		ld a,(de)
		rrca
		rrca
		rrca
		rrca
		ld b,a
		and $f0
		xor c
		ld (hl),a
		inc hl
		ld a,b
		and $f
		xor c
		ld (hl),a ;Unpacks vector table
		inc hl
		inc de
ret

exploframe: .db 0

runexplosion:
	ld ix,expdata
	ld hl,expdata+16
	ld iy,10
	exploloop:
		call vaddexplo
		ld (tmpeff+6),de ;X
		ld bc,$0040
		ld e,(hl) ;Gravity
		inc hl
		ld d,(hl)
		ex de,hl
		add hl,bc
		ex de,hl
		ld (hl),d
		dec hl
		ld (hl),e
		
		call vaddexplo
		ld (tmpeff+8),de ;Y
		ld a,(tmpeff+9)
		cp 220 ;If the pieces fall offscreen, disable em
		jp c,nowreckthewreck
		cp 230 ;...Except if by "fall offscreen we mean go up offscreen"
		jp nc,nowreckthewreck
			ld c,(ix+0)
			call RemoveSprite
			ld a,$ff
			ld (ix+0),a
		nowreckthewreck:

		ld c,(ix+0)
		ld a,0
		cp c
		jp z,protpenexplo
		ld a,255
		cp c
		jp z,protpenexplo
		ld a,(tmpeff+7)
		ld d,a
		ld a,(tmpeff+9)
		ld e,a
		
		;Add actual sprite updates
		call UpdateSprite ;C Object ID, DE X,Y
		ld a,ixl
		and 7
		ld b,a
		ld a,(explomode)
		and 7
		cp b
		jp nz,noframeup
			ld a,ixl
			and 1
			ld e,a
			ld a,(exploframe)
			ld d,a
			ld a,iyl
			cp 5
			jp nc,noefra
				ld a,3
				add d
				ld d,a
			noefra:
			ld c,(ix+0)
			call UpdateSpriteFrame ;C Object ID,DE frame,flip
		noframeup:
		protpenexplo:
		inc IXL
		dec iyl	
	jp nz,exploloop
	ld a,(explomode)
	dec a
	ld (explomode),a
	and 3
	jp nz,noexploframeup
		ld hl,exploframe
		inc (hl)
		ld a,(hl)
		cp 3
		jp nz,noexploframeup
			ld a,0
			ld (hl),a ;reset animation
	noexploframeup:
	
ret

vaddexplo:
		ld c,(hl)
		inc hl
		ld b,(hl)
		inc hl
		ld e,(hl)
		inc hl
		ld d,(hl)
		ex de,hl
		add hl,bc
		ex de,hl
		dec hl
		ld (hl),e
		inc hl
		ld (hl),d
		inc hl
ret
