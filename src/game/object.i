;Physics
tmpstru:.db 0,0,0, 0,0, 0,0, 0,0, 0,0,0
coltable: .db 0,1,0,2,1,1,1
oposx: .db 0,0,0
	;HL pointer to physics structure
CalcPhysics:
	push de
	push bc
	push hl
	ld (temppeng),hl ;Save the thing for later
	ld de,tmpstru
	ld bc,12
	ldir
	;How to vector math
	;I need to sum VX and VY to X and y, which is summing to pos and fix, but the catch is that if we get a carry, posH must be increased OR decreased depending on if its a negative number
	;Given the nature of the subs, it will NOT be a carry if we got an overflow, so i might have to just test the most significant bit of vx/vy to determine if inc/dec is a thing

	;physic object structure (X posH 0|fix 1 |posL 2), (Y fix 3|pos 4), vx (fix 5|pos 6),vy (fix 7|pos 8), sizx 9,sizy 10, status return 11 
		;Gravity
		ld hl,(tmpstru+7)
		ld de,$0040
		add hl,de
		ld a,h
		cp 7
		jr c,notopsp ; Speed limit to stop certain mistakes to happen
		cp $7f
		jr nc,notopsp ; But you know, negative numbers are fine
		ld hl,$700
		notopsp:
		ld (tmpstru+7),hl


	;MAAAAAAAATH
	;Vector math
	ld hl,(tmpstru+1) ;PosX lowfix
	ld bc,(tmpstru+5) ;PosX vector
	ld (oposx),hl

	add hl,bc ;This sums/subtracts low/fix of the vector with the X coordinate. carry will be 1 if needs a inc or zero if needs a dec on highbyte (a screen transition)
	ld a,0
	adc a,0 ;Setting carry to A 
	ld c,a ;c turns into carry (lol)
	ld a,b
	and $80 ;if the high byte of the vector is over 128, then it's a subtraction, otherwise add
	cp $80
	jr nz,noffcph
	ld a,$ff
	noffcph:
	add a,c ;This add the carry to value, which make $01 if carry is true but neg is false or $ff if carry is false but neg is true
	ld (tmpstru+1),hl ;Record the adds
	ld c,a ;Save the result of the math above
	ld a,(tmpstru)
	ld (oposx+2),a
	add a,c
	ld (tmpstru),a ;And increase/decrease high byte accordingly finishing this weirdass 16bit signed+24bit unsigned addition/subtraction thing
	ld hl,(tmpstru+3) ;Now its time for the Y
	ld bc,(tmpstru+7)
	add hl,bc
	ld (tmpstru+3),hl ;Jeez, that's small now
	;MAAAAAAAATH


		;physic object structure (X posH 0|fix 1 |posL 2), (Y fix 3|pos 4), vx (fix 5|pos 6),vy (fix 7|pos 8), sizx 9,sizy 10
		ld a,0
		ld (tmpstru+11),a ;zeroes the status register
		;Collision
		ld a,(tmpstru) 
		ld (temppeng+3),a
		ld a,(tmpstru+2) 
		ld (temppeng+2),a ;first we fetch high and lowpos
		ld a,(tmpstru+4) 
		ld (temppeng+4),a ;Just one for Y

		ld a,(tmpstru+9) 
		ld (temppeng+5),a ;Size X
		ld a,(tmpstru+10) 
		ld (temppeng+6),a ;Size Y
		ld bc,4
		xor a
		sbc hl,bc
		ld a,(tmpstru+8) ;vec y
		and $80 ;Is the object falling or raising?
		jr z,fallingph
		ld a,8
		ld (temppeng+7),a ;Just storing Raising
		ld hl,(temppeng+2) ;Object is going up
		ld a,(temppeng+4)
		ld d,a
		ld bc,2
		add hl,bc
		call GetTileType ;Ceiling left
		call GetTableVal
		and 1 
		cp $0
		jp nz,onfloorph
		ld b,0
		ld a,(temppeng+5)
		sub 4
		ld c,a
		add hl,bc
		call GetTileType ;Ceiling right
		call GetTableVal
		and 1 

		cp $0
		jp z,sidecalcph
		jp onfloorph
		fallingph:
		ld a,0
		ld (temppeng+7),a ;And falling
		ld hl,(temppeng+2)
		ld a,(temppeng+4)
		ld d,a
		ld a,(temppeng+6)
		add a,d
		ld d,a ;floor left
		ld bc,2
		add hl,bc

		call GetTileType ;floor left
		call GetTableVal
		cp $0
		jr nz,onfloorph
		ld b,0
		ld a,(temppeng+5)
		sub 4
		ld c,a
		add hl,bc
		call GetTileType ;floor right
		call GetTableVal
		cp $0
		jp z,sidecalcph

		onfloorph:
		cp $2
		jr nz,nottopstairs
			ld a,(temppeng+4)
			ld b,a
			ld a,(temppeng+6)
			add a,b
			and 12
			jp nz,sidecalcph
	nottopstairs:

		ld a,(temppeng+7)
		ld b,a
		ld a,(tmpstru+4)
		add a,b ;Head offset thing
		and $f8
		ld (tmpstru+4),a ;Align to block
		ld a,0
		ld (tmpstru+3),a ;No subpixel
		ld hl,0
		ld (tmpstru+7),hl ;No vector
		ld a,8
		cp b
		jr nz,goingdown
			ld a,(tmpstru+11)
			or 2
			ld (tmpstru+11),a
			jp sidecalcph
		goingdown:
		ld a,(tmpstru+11)
		or 1
		ld (tmpstru+11),a
		;physic object structure (X posH 0|fix 1 |posL 2), (Y fix 3|pos 4), vx (fix 5|pos 6),vy (fix 7|pos 8), sizx 9,sizy 10
		sidecalcph:
		ld a,(tmpstru+6)
		ld d,a
		ld a,(tmpstru+5)
		add a,d ;we have subpixel movement, so the collision should still happen
		cp $0
		jp z,endcols
		ld a,(tmpstru+6)
		and $80
		jr z,rightmove
		ld hl,(temppeng+2)
		ld a,(temppeng+4)
		ld d,a
		ld a,(temppeng+6)
		sub 3
		add a,d
		ld d,a ;Wall left down
		call GetTileType ;HL x,D Y, Return A
		call GetTableVal
		cp $0
		jr nz,hittingwallph
		ld a,(temppeng+4)
		add a,3
		ld d,a ;wall left up
		call GetTileType ;HL x,D Y, Return A
		call GetTableVal
		cp $0
		jr nz,hittingwallph
		jr endcols
		rightmove:
		ld hl,(temppeng+2)
		ld b,0
		ld a,(temppeng+5)
		ld c,a
		add hl,bc
		ld a,(temppeng+4)
		ld d,a
		ld a,(temppeng+6)
		sub 3
		add a,d
		ld d,a ;Wall right down
		call GetTileType ;HL x,D Y, Return A
		call GetTableVal
		cp $0
		jr nz,hittingwallph
		ld a,(temppeng+4)
		add a,3
		ld d,a ;wall right up
		call GetTileType ;HL x,D Y, Return A
		call GetTableVal

		cp $0
		jr z,endcols
		hittingwallph:
		ld hl,(tmpstru+1)
		ld bc,(tmpstru+5)
		ld a,h
		sbc hl,bc ; Pushes ultrapen back the same vector in X
		ld (tmpstru+1),hl
		ld a,(oposx + 2)
		ld (tmpstru),a
		ld a,0
		ld (tmpstru+1),a

		ld bc,(tmpstru+5)
		ld hl,$0
		;sbc hl,bc
		ld (tmpstru+5),hl
		
		ld a,(tmpstru+11)
		or 8
		ld (tmpstru+11),a ;Let's just say "hitting the wall" and get with it
	endcols:



	;Now when everything fails...
	ld hl,(temppeng+2)
	ld a,(temppeng+5)
	sbc a,2 ; two pixels in
	ld b,0
	ld c,a
	add hl,bc
	ld a,(temppeng+4)
	ld d,a
	ld a,(temppeng+6)
	sub 7
	add a,d
	ld d,a ; Really deep in
	call GetTileType ;HL x,D Y, Return A
	call GetTableVal
	and 1
	cp 0
	jr z,notinsidewallright
		ld a,(tmpstru+2)
		sub 2
		ld (tmpstru+2),a
		ld a,(oposx+2)	
		ld (tmpstru),a
	notinsidewallright:
	ld hl,(temppeng+2)
	inc hl;Two pixels in from the left
	inc hl
	ld a,(temppeng+4)
	ld d,a
	ld a,(temppeng+6)
	sub 7
	add a,d
	ld d,a ; Really deep in
	call GetTileType ;HL x,D Y, Return A
	call GetTableVal
	and 1
	cp 0
	jr z,notinsidewallleft
		ld hl,tmpstru+2
		inc (hl)
		inc (hl)
		ld a,(oposx+2)	
		ld (tmpstru),a
	notinsidewallleft:


	;Math was here

	ld hl,tmpstru
	ld de,(temppeng) 
	ld bc,12
	ldir

	pop hl
	pop bc
	pop de
ret
GetTableVal:
push hl
push de
		ld hl,coltable
		ld e,a
		ld d,0
		add hl,de
		ld a,(hl)
		pop de
		pop hl
ret

;The plan for objects is to replace the "dropping" by using a unified program list that gets called from here every cycle and then the program themselves get removed
;7400-7500 will be the memory space used by the potential programs, with 32 bytes given to each
;While most bytes will be "free",the two first will also be the pointer to the program entry point, so the function below can just loop on the addresses and jump around
;the second byte (not the first) will define if that slot has a program or not
;so the initialization is lower byte,high byte, status and type ,X low, X high,Y, data byte 1-4
;Status is just an edge triggered write that tell the object it just entered the screen again, so i can create those megaman ass things that respawn
;I should also take 7600 for the actual executable list

;I PROBABLY should get the program format too

;The format in the level file is 1 byte type, x low, x high, y, param 1-4

;Thinking a bit, 32bytes for stuff like shots is a bit too much, i think i will work with 16 bytes instead
;a hard cap of 16 objects may prove to be a bit terrible, specially given small script stuff
;The boot format is actually lower byte,high byte, type (and status on bit 7) ,X low, X high,Y, data byte 1-4
;i will use the 15 on the structure to tell the program it died, by shoving a ff there

protable: .ds 8,0 ;Generic table to help loading stuff into the add program

objpt: .dw 0000
objbpt: .dw 0000 ;For the backward scroll
objptini: .dw 0000
objptmin: .dw 0000
objptct: .db 00

objct: .db 00
objttct: .db 0
InitPrograms:
ld hl,$7400
ld a,255
ld c,255
ld b,2
@cleloop:
	ld (hl),a
	inc hl
	dec c
jp nz,@cleloop
dec b
ld c,255
jp nz,@cleloop
ret


;HL pointer to pointers, BC count
Prgpointerstack: .db 0
AddPrgPointers:
	ld d,$76
	ld a,(Prgpointerstack)
	ld e,a
	ldir
	ld a,e
	ld (Prgpointerstack),a
ret

;d hall number ;ixl > 0 means end of the corridor
SpawnScreenPrgs:
	xor a; Whoops, gotta zero it
	ld (objct),a ;and reset objct as well
	ld a,d
	rla
	rla
	rla ;multiply by 8
	ld hl,($8406)
	add a,4 ;Skip over addresses to get to the number of objects in the hall
	ld e,a
	ld a,0
	adc a,0
	ld d,a ;In case we have a roll over
	add hl,de 
	
	ld a,(hl) ;Number of objects (total)
	ld (objttct),a
	inc hl
	ld e,(hl) 
	inc hl
	ld d,(hl) ;And pointer to objects of course, probably would be faster to just this into a routine that runs once per hall change
	ld hl,$8400
	add hl,de ;Don't forget about the offset
	ld (objpt),hl ;Well, it's set
	ld (objbpt),hl ;Same as we didn't did spawning stuff yet
	ld (objptini),hl ;min stuff
	ld (objptmin),hl ;min stuff

	ld a,ixl
	cp 0
	jp z,@notendcorridor
		ld de,3
		add hl,de ;push the pointer forward
		ld de,8
		;ld b,(Hallsiz)
		dec a
		ld b,a ;last screen
		ld a,(objttct)
		ld c,a ;number of objects in the hall
		@findlast:
			dec c
			jp z,nobjectstospawn ;no objects on the last screen it seems
			ld a,(hl)
			add hl,de
			cp b ; are we on the last screen yet?
		jp nz,@findlast ;nope
		ld de,3
		sbc hl,de
		ld (objpt),hl ;now it's actually set

	@notendcorridor:
	ld ix,(objpt)
	;ld (objttct),a

spascloop:
	ld b,a ;Fetching program type
	ld a,(ix+2) ;Stops a metric load of incs and decs
	cp 0
	jp nz,endspawn ;You can stop spawning as the objects will now be sorted
	push ix
	pop hl ;kludge to make HL have the pointer
	ld a,h
	xor l
	ld e,a ;E is supposedly "the program number", but it only need to be an unique number i can compare to later
	;so by xoring low and high address, i get an unique number.. unless we get flipped stuff like 8040/4080 but hopefully weren't getting an object table THIS long
	call SpawnProgram
	ld de,8
	add ix,de
	;call 5
	ld a,(objct)
	inc a
	ld (objct),a
	ld b,a
	ld a,(objttct)
	cp b
	jp z,endspawn ;If we run out of programs to spawn, such as a single screen case, we stop here
	jp spascloop
	endspawn:
	ld (objpt),ix
ret
	nobjectstospawn:
	ret

;alt idea 1: detect when the camera change the page, then calculate the minimum object that could be wrapped around by it
;objpt is now a "minimum object to check", rather than a hard counter

klcounter: .db 0 ;No idea how to make this one clean, probably will be a generic mem thing in the future

nRunPrgSpawner:
	ld a,(CamDir)
	cp 1
	ret z ;Camera is stopped so no spawning 
	ld a,(OCamX + 1)
	ld b,a
	ld a,(CamX + 1)
	cp b
	jp z,@nominupdate
		ld de,8
		ld c,0 
		ld iy,(objptini)
		@toosmall:
			ld b,(iy + 2) ;Fetch page of the object
			cp b
			jp c,@endsearch ;Is it in a future page?
			add iy,de
			inc c
			ld a,(objttct)
			cp c
			ret z ;No more objects to load in this level whatsoever, just bail everything
		jp @toosmall
		@endsearch:
		ld (objptmin),iy ;Here we go, this is the minimum object that can possible be spawned
		ld a,c
		ld (objptct),a ;pretty useful for the bailing out
	@nominupdate:

	ld iy,(objptmin)
	ld a,(objptct)
	ld (klcounter),a
	@seekloop:
		ld l,(iy + 1)
		ld h,(iy + 2)
		ld de,(CamX)
		xor a
		push hl ;save it for previous
		sbc hl,de
		ld c,h ;Page
		ld de,(OCamX)
		pop hl
		sbc hl,de
		ld d,h ;Previous frame

		cp c ;If the object is now onscreen
		jp nz,@notzero
		cp d ;But wasn't on the previous frame..
		jp nz,@justloaded
		
		@notzero:
		ld de,8
		add iy,de ;Next!
		ld b,(iy + 2)
		ld a,(CamX + 1)
		add 2
		sub b
		ret c ; Not even worth the trouble
		ld a,(objttct)
		ld d,a
		ld a,(klcounter)
		inc a
		ld (klcounter),a
		cp d
		ret z ;we're out of objects to test

		;ld a,(klcounter)

	jp @seekloop

	@justloaded:
		;call 5

		push iy
		pop hl
		ld a,h
		xor l
		ld e,a ;More xor trick here
		;call 5
		call isSpawned
		cp 1
		call nz,SpawnProgram
		
		ld de,8
		add iy,de
		ld (objpt),iy
		ld a,(objct)
		inc a
		ld (objct),a ;and increase this friend as well, that is only used to determine if we used all the programs
		ret
ret
;e is the xored value, a returns 1 if its spawned
isSpawned:
	push hl
	push bc
	ld hl,$7400
	ld bc,15
	;call 5
	@seek_boot:
		ld a,(hl)
		cp 255
		jp z,@next
		inc hl
		ld a,(hl)
		cp e
		jp z,@found_obj
		@next:
		add hl,bc
		ld a,$76
		cp h
	jp nz,@seek_boot
	ld a,0
	jp @notfound_obj 
	@found_obj:
	ld a,1
	@notfound_obj:
	pop bc
	pop hl
ret


maxpr: .db 0, 0,0,0,0
saveptr: .db 0,0,0

;HL table pointer, HL returns pointer to created object table
;E is the program position on the program table
SpawnProgram:
	push ix ;Need to preserve this one because disk programs use this
	push hl ;Save stuff for ldiring
	ld a,e
	ld (saveptr+2),a
	ld a,(hl)
	push hl
	pop ix ;This way the program being queued can access it's boot data
	ld b,$80
	call runprg ;Ask the program if it even wants to be summoned at all
	cp $fd
	jp c,failuretoprogram ;The program does not want to have a slot and keep running
	ld (maxpr+3),a

	;push de ;Read the pointer to program execution, push to stack
	ld hl,$7400 - 16 ;Initial adress minus some to make the sum below get the first correct value
	ld de,16
	ld c,0
	@findslot:
		inc c
		add hl,de
		ld a,h
		cp $76
		jp z,failuretoprogram ;Failed to find a program slot (conveniently returns 76 as A)
		ld a,(hl)
		cp 255 
	jp nz,@findslot
	ld a,c
	ld (maxpr + 4),a ;storing the position of this program
	ld (saveptr),hl
	pop de ; hello pointer to init data
	ex de,hl ; ldir expect the opposite


	ld bc,7
	ldi ;First write
	ld a,(saveptr+2)
	ld (de),a ;Add the friend to the list
	inc de
	ldir ;Finishes the setup by copying the rest of the initalization data
	ld a,0
	ld c,8
	@fillzero:
		ld (de),a
		inc de
		dec c
	jp nz, @fillzero
	ld a,(maxpr+3)
	cp $ff
	jp z,@noextra ;Hey, i want more memory
		ld a,$fe
		ld (de),a
	@noextra:
	ld a,(maxpr)
	ld c,a	
	ld a,(maxpr + 4) ;Was 2, but i discovered this runs INSIDE the program loop when a program summon a program, it's quite wicked
	cp c
	jp c,@notbigger
		ld (maxpr),a ;It's bigger so, we store here
	@notbigger:
	;ld (maxpr),a ;Reset the max program to the maximum, so the GC in it does the job of shrinking
	;This actually is a pretty bad idea, because the gc takes a massive chunk of the CPU everytime a new program is spawned
	ld hl,(saveptr) ;Returns the pointer to the newly created program so add shot can change the parameters?
	pop ix
ret
	failuretoprogram:
		;pop hl
		pop hl ;Needed to put the stack pointer back in place instead of just freaking crashing the thing with no survivors
		pop ix
	ret

RunPrograms:

	ld a,32
	ld (maxpr+2),a ;Find the highest active program

	ld a,(maxpr)
	ld c,a ;Reset counter
	ld hl,$7400
	
		progloop:
		push de
		ld a,c
		and $1
		rla
		rla
		ld e,a
		ld d,$05
		call dbgcolor
		pop de

		push bc
		ld a,(hl)
		inc hl
		cp 253 ;Programs declared as fe will not be executed, so a smart routine can reserve a bunch of program slots for extended memory
		jr nc,nextprg ;If the program is 255, then it's not a valid program and we should go to the next
		push hl ;Saving it for the next program
		ex de,hl
		ld hl,maxpr+2;not using it anymore anyway
		ld (hl),c ;Update the maximum program found

		dec de
		ld ixh,d
		ld ixl,e
		ld b,0 ;b will tell the program if it's a regular run or a "ask"

		call runprg

		pop hl ;Load the state so we can try the next program
		nextprg:

		ld de,$000f
		add hl,de ;next page

		pop bc
		dec c
	jr nz,progloop
		ld a,(maxpr+2)
		ld b,a
		ld a,(maxpr)
		sub a,b
		inc a ;need to be sure
		ld (maxpr),a ;Garbage collection

	push de
	ld de,$0000
	call dbgcolor
	pop de

ret

sap: .db 99,0,0
	runprg:
		add a ;Multiply the number for address
		ld l,a
		ld a,(sap)
		cp l
		jp nz,@difprogram
			ld hl,(sap+1)
			jp (hl)
		@difprogram:
		ld a,l
		ld (sap),a
		ld h,$76 ;and with this i get the address
		ld e,(hl)
		inc hl
		ld d,(hl) ;fetching actual program run address
		ex de,hl 
		ld (sap + 1),hl
		jp (hl)
	ret ;Never taken



;de starting program
UnloadPrograms:
	ld hl,$7400
	add hl,de
	push hl
	@uloop1:
		ld de,15
		add hl,de	
		ld a,255
		ld (hl),a
		inc hl
		ld a,h
		cp $76
	jp nz,@uloop1 
	call RunPrograms ;Telling every program that they're about to die so they can unload their sprites etc 
	pop hl
	@uloop2: ;They were warned,now they all die
		inc hl
		ld a,0
		ld (hl),a
		ld de,14
		add hl,de	
		ld (hl),a ;Reset the kill flag, or next time the program boot, it just kills itself again instantly
		inc hl
		ld a,h
		cp $76
	jp nz,@uloop2 


ret

TestProgram:
	ld bc,$1234
	ld de,$5678
	ld (ix + 1),b
	ld (ix + 2),c
 	ld (ix + 3),d
	ld (ix + 4),e
	ld a,255
	ld (ix + 0),a ;Program ended
 ret

;B object type, C returns count
CountInstances:
	ld c,0
	ld hl,$7400
	ld de,$10
	@cloop:
	ld a,(hl)
	cp b
	jp nz,@differentobj
		inc c
	@differentobj:
	add hl,de
	ld a,h
	cp $76
	jp nz,@cloop
ret


;Boot structure: 1 it's a shot, 2 Shot Type, 3 screen X, 4 screen Y, 5 side
;7 sprite number
shpointers: 
.dw shot_normal,shot_coin,shot_hiroshi,shot_hiroshi
.dw shot_normal,shot_coin,shot_hiroshi,shot_hiroshi
.dw shot_normal,shot_coin,shot_hiroshi,shot_hiroshi
.dw shot_normal,shot_coin,shot_hiroshi,shot_ding
.dw shot_generic
shlimits: .ds 16,0 ;Table to allow the shots to limit themselves from spawning

shuse: 
.db 0,2,2,2,2
.db 2,2,2,2,2
.db 2,2,2,2,2

sshot: .db 69,69,69 ;Same shot test
RunNewShot:

	ld a,$80
	cp b
	jp nz,@allocquestion
	ld a,$ff ;Yep, fine, allocate me please

	ret 
	@allocquestion:
	ld e,(ix + 2)
	ld a,(sshot)
	cp e
	jr nz,@newshot
		ld hl,(sshot + 1)
		jp (hl)
	@newshot:
	ld a,e
	ld (sshot),a
	xor a
	rl e
	ld d,0
	ld hl,shpointers
	add hl,de
	ld e,(hl)
	inc hl
	ld d,(hl)
	ex de,hl
	ld (sshot + 1),hl
	jp (hl) ;select which shot code to run

ret

shot_ding:
	ld a,(ix + 14)
	cp 127
	jp nz,@nosecboot
		ld a,64
		ld (ix + 14),a

		ld hl,$ffff
		ld e,(ix + 7)
		call UpdateDot ;Just destroy the shot dot
	@nosecboot:
	ld a,(ix + 5)
	cp 0
	ld a, (ix + 3)
	jp nz,@rightshot
		sub 8
	@rightshot:
	add 4
	ld (ix + 3),a
	ld d,a

	ld a,(ix + 4)
	sub 4
	ld (ix + 4),a

	ld e,(ix + 4)
	ld c,(ix + 6)
	call UpdateSprite

	ld a,(ix + 4)
	cp 248
	jp c, @noend
		@endit:
		ld a,255
		ld (ix + 0),a ;Program ended
		ld c,(ix + 6)
		call RemoveSprite ;Shot done, remove sprite
	@noend:
ret



shot_normal:
	ld a,(ix + 14) ;Type and status
	cp 0
	jr nz,@noshotboot ;Boot

		ld a,127
		ld (ix + 14),a ;And it's no longer boot time
		
		ld a,(shlimits)
		inc a
		ld (shlimits),a
		cp 4
		jp nc,@terminate
		ld a,0
		ld c,0
		ld d,$ff ;No loop
		ld hl,sfx_shoot
		call set_channel_sound


		ld hl,$01
		ld d,(ix + 3) ;X
		ld e,(ix + 4) ;Y
		ld bc,$0000
		;L Type,DE X,Y,B Flip X,Y ,C Frame
		call CreateSprite ;Create shot
		ld c,a ;Returns the sprite in A, shove in C
		ld (ix + 6), c ;Store the sprite number on 6
		call SetAsVramSprite ;And tell the engine it's a vram sprite so the alternate copier will do it's thing
		ld c,$10
		call CreateDot
		ld (ix + 7),a
	@noshotboot:
	ld a,(ix + 5)
	cp 0
	ld a, (ix + 3)
	jp z,@rightshot
		sub 8
	@rightshot:
	add 4
	ld (ix + 3),a
	ld d,a
	ld e,(ix + 4)
	push de
	ld c,(ix + 6)
	call UpdateSprite
	pop hl
	ld e,(ix + 7)
	push de
	call UpdateDot
	pop de
	call GetDotType
	cp 255
	jp z,@endit
	cp 254
	jp nz,@dingit
		ld a,(shlimits)
		dec a
		ld (shlimits),a ;Dinging also counts as "removing a shot"
		ld a,15
		ld (ix + 2),a
		ret	
	@dingit:


	ld a,(ix + 3)
	cp 248
	jp c, @noend
		@endit:
		ld a,255
		ld (ix + 0),a ;Program ended
		ld c,(ix + 6)
		call RemoveSprite ;Shot done, remove sprite
		ld hl,$ffff
		ld e,(ix + 7)
		call UpdateDot
		@terminate:
		ld a,(shlimits)
		dec a
		ld (shlimits),a
		ld a,255
		ld (ix + 0),a ;Program ended

	@noend:


ret


shot_coin:
	ld a,(ix + 14) ;Type and status
	cp 0
	jr nz,@noshotboot ;Boot
		ld a,127
		ld (ix + 14),a ;And it's no longer boot time

		ld a,(shlimits)
		inc a
		ld (shlimits),a
		cp 5
		jp nc,@terminate


		ld hl,$01
		ld d,(ix + 3) ;X
		ld e,(ix + 4) ;Y
		;ld c,(ix + 2) ;Every shot will have a frame
		ld bc,$0001
		;L Type,DE X,Y,B Flip X,Y ,C Frame
		call CreateSprite ;Create shot
		ld c,a ;Returns the sprite in A, shove in C
		ld (ix + 6), c ;Store the sprite number on 6
		call SetAsVramSprite ;And tell the engine it's a vram sprite so the alternate copier will do it's thing
		ld c,$10
		call CreateDot
		ld (ix + 7),a
	@noshotboot:
	ld a,(ix + 5)
	cp 0
	ld a, (ix + 3)
	jp z,@rightshot
		sub 6
	@rightshot:
	add 3
	ld (ix + 3),a
	ld d,a
	ld e,(ix + 4)
	ld c,(ix + 6)
	
	call UpdateSprite

	ld h,(ix + 3)
	ld l,(ix + 4)
	ld e,(ix + 7)
	call UpdateDot

	ld e,(ix + 7)
	call GetDotType
	cp 255
	jp z,@endit
	cp 254
	jp nz,@dingit
		ld a,(shlimits)
		dec a
		ld (shlimits),a ;Dinging also counts as "removing a shot"
		ld a,15
		ld (ix + 2),a
		ret	
	@dingit:

	ld a,(ix + 3)
	cp 248
	jp c, @noend
		@endit:
		ld c,(ix + 6)
		call RemoveSprite ;Shot done, remove sprite
		ld hl,$ffff
		ld e,(ix + 7)
		call UpdateDot
		@terminate:
		ld a,(shlimits)
		dec a
		ld (shlimits),a
		ld a,255
		ld (ix + 0),a ;Program ended

	@noend:
ret

shot_hiroshi:
	ld a,255
	ld (ix + 0),a ;Program ended
ret

;This shot will not obey the rules, as its used bt the enemy
;ix + 5 will the the sprite graphics
shot_generic:

	ld a,(ix + 14) ;Type and status
	cp 0
	jr nz,@noshotboot ;Boot
		ld a,0
		ld (ix + 13),a ; Reseting y pos
		ld a,127
		ld (ix + 14),a ;And it's no longer boot time
		ld h,$00
		ld l,(ix + 5)
		ld d,(ix + 3) ;X
		ld e,(ix + 4) ;Y
		ld bc,$0000
		;L Type,DE X,Y,B Flip X,Y ,C Frame
		call CreateSprite ;Create shot
		ld c,a ;Returns the sprite in A, shove in C
		ld (ix + 6), c ;Store the sprite number on 6
		;call SetAsVramSprite ;And tell the engine it's a vram sprite so the alternate copier will do it's thing
		ld c,$2f
		call CreateDot
		ld (ix + 7),a
	@noshotboot:

	ld a,(globalvars+2)
	ld c,a
	ld b,(ix + 10) ;Speed X (easy)
	ld a, (ix + 3)
	add b
	sub c
	ld (ix + 3),a

	push ix
	pop hl
	ld a,11
	add l
	ld l,a
	ld b,(hl)
	inc hl
	ld c,(hl)
	inc hl
	ld e,(hl)
	ld d,(ix + 4)
	ex de,hl
	add hl,bc
	ex de,hl
	ld (ix + 4),d ;16bit height to get smooth up and down
	ld (hl),e


	ld d,(ix + 3)
	ld e,(ix + 4)
	push de
	ld c,(ix + 6)
	call UpdateSprite
	pop hl
	ld e,(ix + 7)
	push de
	call UpdateDot
	pop de
	call GetDotType
	cp 255
	jp z,@endit
	ld a,(ix + 3)
	cp 248
	jp c, @noend
		@endit:
		ld a,255
		ld (ix + 0),a ;Program ended
		ld c,(ix + 6)
		call RemoveSprite ;Shot done, remove sprite
		ld hl,$ffff
		ld e,(ix + 7)
		call UpdateDot
	@noend:

ret


VEfftable: ;Sprite type, frames per new frame, total frames, initial frame
.db 2,2,3,0 ;enemy pop
.db 3,4,3,0 ;Penhit
.db 4,4,3,0 ;Penslide
.db 5,4,3,0 ;Pendie1
.db 6,4,3,0 ;Pendie2


;Boot structure: 1 it's a effect (set to 4), 2 effect ID, 3 screen Xh, 4 screen xl, 5 screen Y, 6 side

;7 will be sprite id, 8 fpf copy, 9 totframe copy, 10 iniframe copy/counter, 11 fpf counter, 12 contains local X coordinate
VramEffect:
	ld a,$80
	cp b
	jp nz,@allocquestion
	ld a,$ff
	ret ;Please, do allocate me in all occasions
	@allocquestion:

	ld de,(CamX)
	ld h,(ix + 3)
	ld l,(ix + 4)
	sbc hl,de
	ld (ix + 12),l
	ld a,(ix + 14) ;Type and status
	cp 0
	jr nz,@noeffectboot ;Boot
		ld a,128
		ld (ix + 14),a ;And it's no longer boot time
		xor a
		ld a,(ix + 2)
		and 127
		rl a
		rl a ;Multi by 4
		ld iy,VEfftable
		ld e,a
		ld d,0
		add iy,de ;now HL has the correct table		

		ld a,(iy + 1) ;Some backups first
		ld (ix + 8),a
		ld a,(iy + 2)
		ld (ix + 9),a
		ld l,(iy + 0) ;Sprite id		
		ld c,(iy + 3)
		ld (ix + 10),c;And back up the last one
		ld d,(ix + 12) ;X
		ld e,(ix + 5) ;Y
		ld b,(ix + 6) ;Flip
		ld h,0
		;L Type,DE X,Y,B Flip X,Y ,C Frame
		call CreateSprite ;Create effect
		ld c,a ;Returns the sprite in A, shove in C
		ld (ix + 7), c ;Store the sprite number on 10
		call SetAsVramSprite ;And tell the engine it's a vram sprite so the alternate copier will do it's thing
		ld a,(ix + 8)
		ld (ix + 11),a ; Local counter
	@noeffectboot:
		;C Object ID, DE X,Y
		ld c,(ix+7)
		ld d,(ix + 12)
		ld e,(ix + 5)
		call UpdateSprite ;Update the sprite position
	dec (ix + 11)
	jr nz,@noframe
		;call 5
		ld a,(ix + 8)
		ld (ix + 11),a
		inc (ix + 10)
		dec (ix + 9)
		jp z,@endeffect
		ld c,(ix + 7)
		ld d,(ix + 10)
		ld e,(ix + 6)
		call UpdateSpriteFrame	;C Object ID,DE frame,flip
		jp @noframe
		@endeffect:
		ld a,255
		ld (ix + 0),a ;Program ended
		ld c,(ix + 7)
		call RemoveSprite ;Shot done, remove sprite
	@noframe:
ret

HandlePowerUp:
	ld a,$80
	cp b
	jp nz,@allocquestion
	ld b,1
	call CountInstances
	ld a,4
	cp c
	ld a,$ff
	ret nc ;Please, do allocate me if we have less than 4
	ld a,0
	ret ;Whoops 4 is too much
	@allocquestion:
	ld a,(ix + 10)
	cp 5
	jp nc,@runitem
		inc a
		ld (ix + 10),a
		ret  ;Cool down until spawn of item to dodge special effect errors etc
	@runitem:
	ld a,(ix + 8)
	cp 0
	jp nz,@staf
		ld l,6
		ld de,$00f0
		ld bc,3
		call CreateSprite
		ld (ix + 8),a
		ld c,a 
		call SetAsVramSprite ;And tell the engine it's a vram sprite so the alternate copier will do it's thing

		ld c,$33 ;Type
		call CreateDot
		ld (ix + 7),a


		ret 
	@staf:
	ld l,(ix + 2)
	ld h,(ix + 3)
	call GetScreenCoord
	ld d,a
	ld a,b
	cp 0
	jp nz,@end

	ld c,(ix + 8)
	ld e,(ix + 4)	
	push de
	call UpdateSprite

	pop hl
	ld e,(ix + 7)
	push de
	call UpdateDot
	pop de
	call GetDotType
	cp 255
	jp z,@end

ret
	@end:
	ld a,255
	ld (ix + 0),a ;Program ended
	ld c,(ix + 8)
	call RemoveSprite ;Item was collected or went offscreen
	ld hl,$ffff
	ld e,(ix + 7)
	call UpdateDot ;Kills the dot as well
ret




;A returns coordinates you can use for sprites
;B returns if it's seriously offscreen and should be despawned?
GetScreenCoord:
	ld b,0
	xor a
	ld de,(CamX)
	sbc hl,de
	ld a,h
	cp 0
	jp nz,@outofscreen
		ld a,l
		cp 248
		jp c,@stillout
		ld a,248
		@stillout:
		ret
	@outofscreen:
		ld a,h
		cp 255
		jp nz,@reallyoutnotleft
			ld a,l
			cp 248
			ret nc ;This allows the MSX 2 to have SOME entering from the left sprite effect
		@reallyoutnotleft:
		ld a,l
		cp 16
		jp c,@nodieyet
		cp 240
		jp nc,@nodieyet
		ld b,1 ;Lets try like this

		@nodieyet:
		ld a,248
		ret
;IY start position (set to 7400 to begin), C type, Returns A to 1 if EOL, IY with last address
findObj:
	push hl
	push de
	push bc
	ld de,16
	ld a,(maxpr)
	ld b,a
	push iy
	pop hl
	add hl,de ;Skip the first, so this can be a continuous test
	@seekloop:
		ld a,(hl)
		cp c
		jp z,@found
		add hl,de
		dec b
	jp nz,@seekloop
	ld a,1
	jp @notfound	
	@found:
	ld a,0
	@notfound:
	push hl
	pop iy
	pop bc
	pop de
	pop hl
ret

atmp .db 0,0
;E points to the offset in IX structure with position, size and point, returns 1 to A if in box
BoxTest:
	push hl
	push de
	push bc
	ld (atmp),SP
	ld d,0
	push ix
	pop hl
	add hl,de
	ld SP,hl
	pop hl
	pop de
	pop bc
	ld a,b
	cp h
	jp c,@notfound ;Too left
	ld a,c
	cp l
	jp c,@notfound ;Too up
	add hl,de
	ld a,b
	cp h
	jp nc,@notfound ;Too right
	ld a,c
	cp l
	jp nc,@notfound ;too down
	ld a,1
	jp @found
	@notfound:
	ld a,0
	@found:
	ld SP,(atmp)
	pop bc
	pop de
	pop hl
ret

dots: .ds 45,0 ;type X,Y

CreateDot: ;HL position, C dot type,Returns in A
	ld b,15
	push hl
	ld hl,dots
	ld de,3
	@findfree:
		ld a,(hl)
		cp 0
		jp z,@found
		add hl,de
		dec b
	jp nz,@findfree
	pop hl
	ld a,255
	ret ;whoops, too many shots
	@found:
	ld a,(qd + 4)
	ld d,a
	ld a,15
	sub b; flip stuff around
	cp d
	jp c,@nonewmax
		ld (qd + 4),a ;maximum updated!
	@nonewmax:	
	ld (hl),c
	pop bc
	inc hl
	ld (hl),b
	inc hl
	ld (hl),c
	inc hl
	xor a
	sbc hl,de
	ld de,dots
	sbc hl,de
	ld a,l ;fetch the ID number
ret
UpdateDot: ;HL position, E ID, set to FFFF to kill it

	push hl
	ld d,0
	ld hl,dots
	add hl,de
	pop de
	ld a,255
	cp e
	jp z,@deletemode
		inc hl
		ld (hl),d
		inc hl
		ld (hl),e
		ret
	@deletemode:
	ld a,0
	ld (hl),a
	ld hl,dots+14*3
	ld b,15
	ld de,3
	@findmax:
		ld a,(hl)
		cp 0
		jp nz,@foundadot
		sbc hl,de
		dec b
	jp nz,@findmax
	@foundadot:
	ld a,b
	ld (qd + 4),a
ret

ChangeDot: ;B new dot type, E ID
	ld d,0
	ld hl,dots
	add hl,de
	ld (hl),b
ret 

GetDotType: ;E ID, A returns dot type
	ld d,0
	ld hl,dots
	add hl,de
	ld a,(hl)
ret 


qd: .ds 8,0

QueryDots: ; HL position, DE size,C dot type (only high nibble),A to dot ID if found
	;call 5

	ld (qd),hl
	ld (qd+2),de
	ld a,(qd+4)
	inc a
	ld b,a
	ld hl,dots
	ld de,3
	@seeker:
		ld a,(hl)
		and 240 ;filter low bits
		cp c
		jp nz,@nottype
			push de
			push hl
			inc hl
			ld a,(qd+1)
			ld d,a
			ld a,(qd+3)
			ld e,a
			ld a,(hl)
			sub d
			cp e
			jp nc,@notinbox
			inc hl
			ld a,(qd)
			ld d,a
			ld a,(qd+2)
			ld e,a
			ld a,(hl)
			inc hl
			sub d
			cp e
			jp nc,@notinbox
			;Found it
			ld de,dots
			sbc hl,de
			ld a,l
			sub 2
			pop hl
			pop hl ;Just getting the SP back to place
			ret
			@notinbox:
			pop hl
			pop de
		@nottype:
		add hl,de
		dec b
	jp nz,@seeker
	ld a,255
ret

AddEffect: ;A effect ID , bc X pos (16bit), D Y pos, E flip
		push ix
		ld ix,protable		
		ld (ix+1),a ;effect ID
		ld a,9 ;effect
		ld (ix+0),a
		ld (ix+2),b
		ld (ix+3),c
		ld (ix+4),d
		ld (ix+5),e ;Effect side
		ld hl,protable
		call SpawnProgram ;Boot structure: 0 it's a effect (set to 4), 1 effect ID, 2 screen Xh, 3 screen Xl, 4 screen Y, 5 side
		pop ix
ret

TestCol:

ret 

AddItem: ;bc X pos (16bit), D Y pos, e max item value
	push ix
	ld ix,protable		
	ld e,0
	ld (ix+1),e ;item ID
	ld a,1 ;Item
	ld (ix+0),a
	ld (ix+2),b
	ld (ix+1),c ;Different strokes i suppose
	ld (ix+3),d
	ld hl,protable
	call SpawnProgram ;Boot structure: 0 it's a effect (set to 4), 1 item ID, 2 screen Xh, 3 screen Xl, 4 screen Y
	pop ix
ret
.db 0,0,0,0
