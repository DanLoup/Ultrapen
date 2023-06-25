;7400-7440 shot list
; 0 shot type/target,1 spriteID, 2-3 xl-xh, 4-5 yl-yh,6-7 vxl-vxh,8-9 vyl,vhh (10 bytes)
;Yeah, will redesign all this shit to be simpler AND more flexible
;I need the shot on character check to work (which means shot list)
;Maybe just tossing the program thing entirely and just offering vector/position you can change per frame?

;Found a good way to not wreck the CPU with it
;Do TWO lists, one with all the shots that will hit ultrapen (and hiroshi), and one with shots done by ultrapen
;As ultrapen is only one target, its only one check per enemy shot
;As ultrapen can only shot up to 3 shots, its 3*all enemies onscreen
;So if for example, you have 5 enemies shooting 3 shots each, and ultrapen is shooting 3 as well
;Instead of 6*18 (108) checks, you get only 15+15 (30) checks

jtb: .db 0,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160


;a Slot,HL returns 
sethladdr:
	push de
	ld de,0
	ld e,a
	ld hl,jtb
	add hl,de
	ld e,(hl)
	ld d,$74
	ex de,hl
	pop de
ret

OOcamX: .db 0,0 ;my own ocam
gensho:
 .db 0,0,0,0,0,0,0,0

sptable:
.db 0,0,0,0,0,0,0,0,0,0

shotgc: .db 0,0
maxshot: .db 0
InitShots:
	ld hl,$7400
	clearshots:
	ld a,0
	ld (hl),a
	inc hl
	ld a,l
	cp 0
	jp nz,clearshots
ret
	
RunShots:
	ld de,(OOcamX)
	ld hl,(CamX)
	ld (OOcamX),hl
	xor a
	sbc hl,de
	ld a,l
	ld (gensho+3),a ;Store the camera offset
	ld a,0
	ld (gensho+2),a ;Store the camera offset

	ld hl,$7400
	looprshot:
		ld a,(hl)
		cp 0
		jp z,notashotrs
		ld (gensho),hl ;save adress
		inc hl
		ld c,(hl) ;Sprite ID
		inc hl
		inc hl
		ld d,(hl) ;X high
		inc hl ;Jump over Xlow
		inc hl
		ld e,(hl) ;Y high
		call UpdateSprite
		ld hl,(gensho)
		ld de,sptable
		ld bc,10
		ldir 
		ld hl,(sptable+2)
		ld (gensho+4),hl ;saving current X
		ld de,(gensho+2)
		xor a
		sbc hl,de ;Sum the camera offset, so shots follow it cheaply
		ld de,(sptable+6)
		add hl,de
		ld (sptable+2),hl ;Add X
		ld a,h
		sbc a,4 ;Pulls it back so 4-4
		and $f8
		cp $f8 ;It's on the 250-4 range
		jp z,shotdead 
		ld hl,(sptable+4)
		ld de,(sptable+8)
		add hl,de
		ld (sptable+4),hl ;Add Y
		ld a,h
		sbc a,4 ;Pulls it back so 4-4
		and $f8
		cp $f8 ;It's on the 250-4 range
		jp z,shotdead 

		ld de,(gensho)
		ld hl,sptable
		ld c,10
		ldir
		ld hl,(gensho) ;load adress
	notashotrs:
	ld de,10
	add hl,de
	;ld a,l
	;cp $A0
	ld a,(maxshot)
	add a,10
	cp l
	jp nz,looprshot
	
	ld hl,shotgc+1
	inc (hl)
	jp nz,noshotgc
		dec hl
		ld a,(hl)
		cp 1
		jp nz,noshotgc
			ld a,0
			ld (hl),a ;Clear GC
			ld hl,$7496
			ld de,$000a
			shotgcfind:
				ld a,(hl)
				cp 0
				jp nz,foundmaxshot
				xor a
				sbc hl,de
				ld a,l
				cp 0
			jp nz,shotgcfind
			foundmaxshot:
			ld a,l
			ld (maxshot),a
	noshotgc:
ret

shotdead:
	ld hl,(gensho)
	ld a,0
	ld (hl),a
	inc hl
	ld c,(hl)
	call RemoveSprite
	ld a,1
	ld (shotgc),a
ret

; b shottype+target ,de X/Y (high)
; A returns shotID
AddShotv:
	ld hl,$73F6
	ld (gensho),de
	ld c,$ff
	findfreeshot:
		inc c ;Just getting the shot id in a simple way
		ld a,l
		cp $A0
		ret z ;Conveniently returns $A0 when no free shot is found
		ld de,10
		add hl,de
		ld a,(hl)
		cp 0
	jp nz,findfreeshot
	
	ld a,(maxshot)
	cp l
	jp nc,notbiggershot
		ld a,l
		add a,10
		ld (maxshot),a ;Higher
	notbiggershot:
	
	ld a,c
	ld (gensho+3),a ;Saving the shot id
	ld a,0
	ld (hl),b ;Type
	inc hl
	inc hl ;Skip sprite ID for now
	ld de,(gensho)
	ld (hl),a ;Clean the low
	inc hl
	ld (hl),d ;X
	inc hl
	ld (hl),a ;And here too
	inc hl
	ld (hl),e ;Y
	ld (gensho),hl ;save this train for later
	ld a,b
	and $7f ;remove target
	dec a ;Frame 0 = shot 1
	;push de
	;pop bc
	ld b,0
	ld c,a
	ld hl,$0300
	ld de,$ffff
	;HL Type/Subtype,DE X,Y,B Flip X,Y ,C Frame
	;A returns The Object ID
	call CreateSprite
	ld c,a
	
	ld hl,(gensho)
	ld de,4 ;Jumps the vectors
	xor a
	sbc hl,de ;Go back to the sprite ID map
	ld (hl),c ;Finally save the address to the sprite
	ld a,(gensho+3)	;and the shot id is delivered
ret

;A shotID, BC pos
SetShotPos:
	push hl
	call sethladdr
	ld a,l
	add a,2 ;Points it to X/Y
	ld l,a
	ld a,0
	ld (hl),a ;Clean LX
	inc hl
	ld (hl),b ;X
	inc hl
	ld (hl),a ;Clean LY
	inc hl
	ld (hl),c ;Y
	pop hl
ret

;A shotID, BC VX, DE VY
SetShotVector:
	push hl
	call sethladdr
	ld a,l
	add a,6 ;Points it to VX/VY
	ld l,a
	ld a,0
	ld (hl),c ;VXL
	inc hl
	ld (hl),b ;VXH
	inc hl
	ld (hl),e ;VYL
	inc hl
	ld (hl),d ;VYH
	pop hl
ret

;Count shots of type
; c shot type, returns on b
CountShot:
	ld hl,$7400
	ld b,0
	shotloop:
		ld a,(maxshot)
		add a,10
		cp l
		;ld a,l
		;cp $80
		ret z ;Done counting
		ld de,10
		ld a,(hl)
		add hl,de
		cp c
	jp nz,shotloop
	inc b
	jp shotloop
ret
;Tests if shot is in the box
;DE x,y, BC WiHe, L Target
;Returns shot ID at A
IsHit:

ret

;A shot ID
KillShot:
	push af
	push hl
	call sethladdr
	ld a,0
	ld (hl),a ;Cleans buffer
	inc hl
	ld c,(hl)
	call RemoveSprite
	pop hl
	pop af
ret
;Changes shot to a reflected type
;A shot ID
ReflectShot:
	push af
	push hl
	push de
	call sethladdr
	ld a,$ff
	ld (hl),a ;Set it as reflected
	ld de,$6
	add hl,de ;Skip to the vectors
	ld a,(hl)
	xor $ff
	ld (hl),a
	inc hl
	ld a,(hl) 
	xor $ff
	ld (hl),a ;Flips X Vector
	inc hl
	ld de,-$500
	ld (hl),e
	inc hl
	ld (hl),d ;Sets shot to go up in a fixed rate
	pop de
	pop hl
	pop af
ret

;HL Local X/Y,DE Bounding Box
;C Shot side
;A Returns shot ID
TestShot:
	ld (gensho),hl ;X/Y
	ld (gensho+2),de ;Bounds
	ld hl,$7400
	ld (gensho+4),hl ;address boy
	ld b,0 ;Actual sprite ID counter
Tshotl:
	ld hl,(gensho+4)
	ld a,(hl)
	cp 0
	jp z,SpReject ;Free slot
	cp $ff
	jp z,SpReject ;Reflected
	and $80
	cp c ;Not the right field
	jp nz,SpReject

		ld de,3
		add hl,de ; Jump target-id/sprite and the low X
		ld a,(gensho+1) ;X bounding box
		ld d,a
		ld a,(hl) ;X High shot
		sbc a,d ;Distance from left bound (will overflow if distance smaller and it's a good thing)
		ld d,a
		ld a,(gensho+3) ;Bounding box size X
		cp d ; subtract position from size, will carry if either the other sub overflowed, or if its too big
		jp c,SpReject
		inc hl
		inc hl ;Go to High y
		ld a,(gensho) ;Y bounding box
		ld d,a
		ld a,(hl) ;Y High shot
		sbc a,d ; Same trick as above 
		ld d,a
		ld a,(gensho+2) ;Bounding box size Y
		cp d ; Same trick as above
		jp nc,Tsfoundshot ;Save a few bytes
	SpReject:
	ld hl,(gensho+4)
	ld de,10
	add hl,de
	ld (gensho+4),hl
	inc b
	ld a,(maxshot)
	add a,10
	cp l
	jp nz,Tshotl
	ld a,255 ;No shot id
ret
	Tsfoundshot:
	ld a,b
ret
