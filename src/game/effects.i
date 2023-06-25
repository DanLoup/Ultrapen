;Calling dibs on 7600
;0 Timer, 1 XHi,2 Xlo,3 Y,4 spriteID (could store the flip here on a high bit),5-6 pointer, 7 Frame counter/curframe
tmpeff: .ds 10,0

InitEffects:
	ld hl,$7600
	ld a,0
	cleareff:
		ld (hl),a
		inc hl
		cp l
	jp nz,cleareff
ret

;I should just use this table raw instead of using weird memory pointers
enepop_eff: .db 0,3,$20,6
penhit_eff: .db 1,3,$40,12
penslide_eff: .db 2,3,$40,12
death1_eff: .db 3,3,$40,120
death2_eff: .db 4,3,$40,120

;AddEffect BC,X (ex),D y, HL pointer to effect type, E is flip (add 128 here)
;Effect structure: 0 sprite gfx id, 1 num aniframes, 2 frame speed (multiplied by 16 for performance/code size reasons), 3 total frames 
;A returns effect ID or FF in case of no slot free
AddEffectd:

	push hl
	push bc
	push de
	ld (tmpeff),hl
	ld (tmpeff+2),de
	ld iy,$7600-8
	ld de,$8
	ld a,$ff
	addefl1:
	add iy,de
	ld a,(iy+0)
	cp 0
	jp z,addefoundsl
	ld a,IYL
	cp $60
	jp nz,addefl1
	ld a,$ff
	pop de
	pop bc
	pop hl
	ret
addefoundsl:
	ld ix,(tmpeff)
	ld a,(ix+3)
	ld (iy+0),a ;Copy timer
	ld (iy+2),c ;X High/low
	ld (iy+1),b
	ld de,(tmpeff+2)
	ld (iy+3),d ;Y
	ld a,e
	and $80
	ld e,a
	ld (iy+4),e ;flip
	ld hl,(tmpeff)
	ld (iy+5),l ;Pointer to effect structure
	ld (iy+6),h
	ld a,(ix+2)
	ld (iy+7),a ;Copy number of frames
	ld h,7
	ld l,(ix+0)
	ld de,0
	ld bc,0
	;HL Type/Subtype,DE X,Y,B Flip X,Y ,C Frame
	;A returns The Object ID
	call CreateSprite
	cp $ff
	jp nz,nocanceleffect
	ld a,0
	ld (iy+0),a
	ld a,$ff
	pop de
	pop bc
	pop hl
	ret
	nocanceleffect:
	or (iy+4) ;Add flip
	ld (iy+4),a
	xor a
	ld a,(iy+0)
	pop de
	pop bc
	pop hl
ret



RunEffects:
	ld hl,$7600 
	effloop:
		ld (tmpeff+6),hl
		ld a,(hl)
		cp 0
		jp z,notthiseff
		dec a
		ld (hl),a ;Count down timer
		cp 0
		jp z,RemoveEffectSprite
		inc l
		ld d,(hl)
		inc l
		ld e,(hl)
		inc l
		ld b,(hl)
		inc l
		ld c,(hl) ;This is ugly, but its both faster AND smaller on source than IX/IY
		ld (tmpeff+4),bc
		inc l
		ld a,c
		and $7f
		ld c,a ;Filter out the flip bit
		call UpdateSpriteGlobal ;C Object ID, DE Screen/X B, Y
		ld a,(hl)
		ld ixl,a
		inc hl
		ld a,(hl)
		ld IXH,a
		inc hl
		ld a,(hl)
		sub $10
		ld (hl),a
		and $f0 ;Lower 4bits will screw this plan
		jp nz,noefframe
			ld a,(hl)
			and $0f ;Filter the current frame out
			inc a ;Add 1 to it
			ld b,(ix+1)
			cp b ;Test if it's the maximum frame
			jp nz,noresfframe
				ld a,0 ;And loop it if it is
			noresfframe:
			or (ix+2) ; up the frame counter
			ld (hl),a ;And rewrite it!
			and $0f
			ld d,a
			ld e,0
			ld a,(tmpeff+4)
			cp 128
			jp c,notflipeff
				ld e,1
			notflipeff:
			and $7f
			ld c,a ;And remove flip from byte
			call UpdateSpriteFrame ;And finally update the frame
		noefframe:
	notthiseff:
		ld hl,(tmpeff+6)
		ld de,8
		add hl,de
		ld a,l
		cp $60
	jp nz,effloop

	ld a,(explomode)
	cp 0
	call nz,runexplosion

ret


RemoveEffectSprite:
	ld de,4
	add hl,de
	ld a,(hl)
	and $7f
	ld c,a
	call RemoveSprite
jp notthiseff


;l is effect ID * 8
;bc is position X/XH
;d is y
SetEffectPos:
	push hl
	push bc
	push de
	ld h,$76
	inc l
	ld (hl),c
	inc l
	ld (hl),b
	inc l
	ld (hl),d
	pop de
	pop bc
	pop hl
ret
;Calling dibs on 7680 - 0-6 sprite IDs 10-x xv,xc,yv,yc x 6
explomode: .db 0 ; 0 counter

expvt: .db $21,$53, $22,$65, $11,$74 ,$14,$52 ,$25,$64 ,$22,$73, $15,$50 ,$5,$43, $25,$70 , $9,$55  ;20 bytes

; hl/X,c/Y 
Explode:
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
	ld ix,$7680
	floop1explo:
		ld hl,$0703
		ld de,(tmpeff+4)
		ld bc,0
		ld a,IYL
		cp 5
		jp nc,nochangetofr2
			ld l,4
		nochangetofr2:
		call CreateSprite
		ld (ix+0),a
		inc ix
		dec IYL
	jp nz,floop1explo
	ld IYL,10
	ld hl,$7690
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
	ld ix,$7680
	ld hl,$7690
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
			call UpdateSpriteFrame ;C Object ID,DE frame,flip
		noframeup:
		protpenexplo:
		inc IXL
		ld a,$8A
		cp IXL
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
