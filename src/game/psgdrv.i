.MACRO writepsg
	push af
		ld a,\1
		out ($a0),a
		ld a,\2
		out ($a1),a
	pop af
.ENDM


song_addr: .dw 0
music_trpos: .dw 0
music_beat_pos: .dw 0
music_beat_counter: .dw 0
reset_song: .db 0

;0-1 music pointer, 2 current wait, 3-7 instrument controller
;i didn't actually used the "instrument controller" for anything, did i?
;How about this
;3-4 current instrument pointer, 5-6 loop pointers, 7 SCC channel?

chvars: .dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 
music_bpm: .db 0,0
music_notes: 
.dw $0000,$1939,$17cf,$1679,$1536,$1405,$12e5,$11d6,$10d5,$0fe4,$0eff,$0e28,$0d5c,$0c9c,$0be7,$0b3c,
.dw $0a9b,$0a02,$0972,$08eb,$086a,$07f2,$077f,$0714,$06ae,$064e,$05f3,$059e,$054d,$0501,$04b9,$0475,
.dw $0435,$03f9,$03bf,$038a,$0357,$0327,$02f9,$02cf,$02a6,$0280,$025c,$023a,$021a,$01fc,$01df,$01c5,
.dw $01ab,$0193,$017c,$0167,$0153,$0140,$012e,$011d,$010d,$00fe,$00ef,$00e2,$00d5,$00c9,$00be,$00b3,
.dw $00a9,$00a0,$0097,$008e,$0086,$007f,$0077,$0071,$006a,$0064,$005f,$0059,$0054,$0050,$004b,$0047,
.dw $0043,$003f,$003b,$0038,$0035,$0032,$002f,$002c,$002a,$0028,$0025,$0023,$0021,$001f,$001d,$001c,
.dw $001a,$0019,$0017,$0016,$0015,$0014,$0012,$0011,$0010,$000f,$000e,$000e,$000d,$000c,$000b,$000b,
.dw $000a,$000a,$0009,$0008,$0008,$0007,$0007,$0007,$0006,$0006,$0005,$0005,$0005,$0005,$0004,$0004

boot_music_driver:
	ld hl,($840e)
	ld (song_addr),hl ;sets the music pointer
	ld c,6
	ld ix,(song_addr) 
	ld de,3
	add ix,de 
	ld iy,chvars
	ld hl,(song_addr)
	ld de,15
	add hl,de
	mus_set_loop:
		push hl
		ld e,(ix+0)
		ld d,(ix+1)
		add hl,de
		inc ix
		inc ix
		ld (iy+0),l
		ld (iy+1),h
		ld a,0
		ld (iy+3),a
		pop hl
		ld a,1
		ld (iy+2),a
		ld de,8
		add iy,de
		dec c
	jp nz,mus_set_loop
	ld hl,(song_addr)
	ld de,14
	add hl,de
	ld (music_trpos),hl

	ld ix,(song_addr) ;Music pointer
	ld a,(ix+1)
	ld (music_bpm),a
	ld a,(ix+2)
	ld (music_bpm+1),a
	writepsg 7,8+16+32
	writepsg 0,123
	writepsg 1,3

	ld a,0
	ld b,0
	call load_instrument
	ld a,0
	ld b,1
	call load_instrument
	ld a,0
	ld b,2
	call load_instrument

	ld a,255
	ld (music_beat_counter+1),a
	
	ld c,48
	ld hl,sfx_cur
	clear_loopa:
	ld (hl),a
	inc hl
	dec c
	jp nz,clear_loopa


	ld b,$d ;Envelope format
	ld l,$a ;Triangles
	call load8_psg 

ret

beat: .db 1
inst: .db 0
note: .db 0
musisize: .db 32
fxct: .db 0
chaen: .db 0


run_music_driver:
	ld a,8+16+32
	ld (chaen),a
	ld de,(music_bpm)
	ld hl,(music_beat_counter)
	add hl,de
	ld (music_beat_counter),hl

	jp nc, no_beat
	ld a,0
	ld ix,chvars
	call runchannel
	ld a,1
	ld ix,chvars + 8
	call runchannel
	ld a,2
	ld ix,chvars + 16
	call runchannel
	no_beat:

	ld a,0
	call run_ramp
	ld a,1
	call run_ramp
	ld a,2
	call run_ramp

;;	ld a,(fxct)
;	inc a
;	ld (fxct),a


	ld a,(chaen)
	ld l,a
	ld b,7
	call load8_psg

	ld a,(reset_song)
	cp 1
	call z,boot_music_driver ;loop the music
	ld a,0
	ld (reset_song),a
ret
; L value, B port
load8_psg:
	ld a,b
	out ($a0),a
	ld a,l
	out ($a1),a
ret
; HL value, B port
load16_psg:
	ld a,b
	out ($a0),a
	ld a,l
	out ($a1),a
	inc b
	ld a,b
	out ($a0),a
	ld a,h
	out ($a1),a
ret

;A note,HL returns note value
get_note:
	push de
	add a
	ld e,a
	ld d,0
	ld hl,music_notes
	add hl,de
	ld e,(hl)
	inc hl
	ld d,(hl)
	ex de,hl

	pop de
ret

;A channel nb ,ix channel pointer

;0-1 music pointer, 2 counter for the current note to end
;3 Using it just to disable the channel
;4/5 ramp address, 6 is offering the loop values but not being used
;the b value is used to determine the current note

runchannel:
	ld iyl,a
	ld a,(ix+3)
	cp 1
	jp z,noproc_channel ;Music ended on this channel, channel disabled

	ld a,(ix+2)
	cp 0
	jp z,neg_rc ;this is only called when the note end, to load the next note
	dec a
	ld (ix+2),a
	jp nz,noproc_channel
	neg_rc:
	ld l,(ix+0)
	ld h,(ix+1)
	ld b,(hl)
	inc hl
	ld c,(hl)
	inc hl
	ld (ix+0),l
	ld (ix+1),h
	ld (ix+2),c

	ld a,b
	and 128
	jp nz,cmd_note
	cont_music:
	ld a,iyl
	ld c,b
	ld l,(ix+4)
	ld h,(ix+5)
	ld d,(ix+6)
	call set_channel_sound

noproc_channel:
ret

cmd_note:
	cmd_read_loop:
	ld a,b
	cp 128
	jp nz,no_stopnote
		ld (ix+2),c ;Restore the time because it is a muted note
		push ix
		ld a,iyl
		rlca
		rlca
		rlca
		rlca ;Multiply by 16
		ld ix,sfx_cur
		add a,ixl
		ld ixl,a
		ld a,255
		ld (ix + 13),a ;Just tosses the loop point to an unreachable point
		pop ix
		ret
	no_stopnote:


	ld a,b
	cp 131
	jp nz,nodisable_channel
		ld a,1
		ld (ix+3),a
		ret
	nodisable_channel:
	ld a,b
	cp 132
	jp nz,no_reboot
		;call 5
		ld a,1
		ld (reset_song),a
		ret
	no_reboot:

	ld a,b
	cp 129
	jp nz,nochange_inst
		push bc
		ld a,c ; c is the instrument
		ld b,iyl ; channel
		call load_instrument
		pop bc
	nochange_inst:

	next_cmd:
	ld l,(ix+0)
	ld h,(ix+1)
	ld b,(hl)
	inc hl
	ld c,(hl)
	inc hl
	ld (ix+0),l
	ld (ix+1),h
	ld (ix+2),c

	ld a,b

	and 128
	jp nz,cmd_read_loop
	jp cont_music
ret 

;I know i already wrote it on the instrument editor.. but can't i just expand the SFX engine?
;It does ADSR with its ramp, you can do arpeggios with extra commands..
;The minimum instrument size there is 15 bytes
;That's probably bigger than many instruments using the ramp system
;The S part is the only problem, because it holding the note
;Could just do a "hold in this place until release" number
;so can we add more bytes?
;No need, just use another bit for the triangle mode
;numblocks: ivol/rampsize,ipitch,vpitch(signed),vvol & audio (1) & noise (2)  is what we have
;so we can add 4 there, or a 0 = psg, 1 = noise, 2 = noise/psg, 3 = triangle
;also for SCC, we can have a separate table, or use the upper bits for numblocks to choose a different ramp
;or we can do that when it's an SCC channel, the 0-7 of the vvol select the ramp

;IX channel regs, C channel number
run_cha_insts:
	ld a,(ix+3)
	cp 0
	ret z

	ld l,a
	dec a
	ld (ix+3),a
	ld a,8
	add c
	ld b,a
	call load8_psg
ret


;SFX format
;byte numramps
;ipitch,ivol,vpitch,vvol (both pitchs are shifted 16 up)
;Dá pra usar bits do ivol para programar efeitos
;Tipo bit 128 = liga noise, 64 liga square (assim dá pra misturar os dois)
;poderia usar 32 e 16 para escolher "bounce", tipo fazer o vetor inverter quando passa do bounds
;Outra coisa é que o vvol deveria ser tipo *4 pra permitir fades com menos de 1 quadro
;hm, preciso enfiar o tamanho da rampa em algum lugar
;talvez os bits de cima do ivol é o tamanho
;e os dois bits de cima do vvol escolhem o noise
;Pelas minhas contas, todos os efeitos do jogo provavelmente cabem em tipo 500 bytes
;Desempacotar isso tipo colocar uns 2 bytes a mais iria subir o tamanho mais do que 
;só fazer as continhas

;Okay, como diabos isso aqui vira engine de instrumento então?
;A forma mais facil seria só triplicar a função pra cada canal de audio, mas isso parece baka mitair



;0 ramps left,1-2 pointer , 3 cramp left,4 ivol, 5 vvol,6 ipitch, 7 vpitch, 8 chamodes ,10-11 note (in PSG scale)
;9,14 & 15 are free 12-13 are loop point and loop to
;Probably should integrate the "set_channel_instrument" to this to avoid all the extra math
;However that screws with the whole concept of setting a ramplist pointer

sfx_cur: .ds 48,0
;A channel, C pitch, D loop val
;HL ramplist pointer (!) (economiza CPU, economiza memoria, e permite coisas tipo embutir efeitos no binario)
set_channel_sound:
	push ix
	rlca
	rlca
	rlca
	rlca ;Multiply by 16
	ld ix,sfx_cur
	add a,ixl
	ld ixl,a
	ld a,0
	ld (ix + 0),a ;Starting ramp
	adc a,ixh
	ld ixh,a
	ld a,(hl)
	add a,2 ;target need to be a bit far
	ld (ix + 15),a ;Number of ramps left
	inc hl 
	ld a,l
	ld (ix + 1),a ;pointer to the actual ramp
	ld a,h
	ld (ix + 2),a ;same pointer to actual ramp
	
	
	ld a,0
	ld (ix + 3),a ;ramp loop values
	ld a,d
	and 15
	ld (ix + 12),a ;Ramp init
	ld a,d
	and 240
	rrca
	rrca
	rrca
	rrca
	ld (ix + 13),a ;Ramp loop point

	ld a,c
	ld (ix + 14),a
	call get_note
	ld (ix + 10),h
	ld (ix + 11),l
	pop ix 

ret
;A = channel
c_cha: .db 0

run_ramp:
	ld (c_cha),a
	ld iy,sfx_cur
	rlca
	rlca
	rlca
	rlca
	add iyl
	ld iyl,a
	ld a,0
	adc iyh
	ld iyh,a	
	ld b,(iy + 0)
	ld a,(iy + 15)
	cp b
	ret z ;no ramps left
	ld a,(iy+3)
	cp 0
	jp nz,no_setupramp
		ld b,(iy + 0)
		ld a,(iy + 15)
		cp b
		jp z,end_sfx ;No more ramps to process so just return
		ld a,(iy+0)
		rlca
		rlca
		ld b,a
		ld a,(iy+1)
		add b
		ld ixl,a
		ld a,(iy+2)
		adc 0
		ld ixh,a 

		ld a,(ix + 7)
		and 128
		jp z,no_dual
			ld a,(ix + 0)
			and $0f
			add a
			inc a ;Add 1 to it, so 0 is not a glitchy useless number
			ld (iy+3),a
			jp dual_end
		no_dual:
			ld a,(ix + 0)
			and $0f
			inc a ;Add 1 to it, so 0 is not a glitchy useless number
			ld (iy+3),a
		dual_end:
		ld a,(ix + 0) ;Number left
		and $f0 
		ld (iy+4),a ; Ivol (multiplied by 16, will be divided on the output)
		ld a,(ix + 1)
		ld (iy+6),a ;i pitch
		ld a,(ix + 2)
		ld (iy+7),a ;v pitch
		ld a,(ix + 3)
		and 252 ;remove the first two bits that program the channel enable
		ld (iy + 5),a ;v vol
		ld a,(ix + 3)
		and 3 
		ld (iy+8),a ;chmode

		ld a,(iy+0)
		inc a
		ld (iy+0),a
		ld b,(iy + 13)
		inc b
		cp b
		jp nz,no_loop_point
			ld a,(iy + 12)
			ld (iy + 0),a
		no_loop_point:
	no_setupramp:

		;Set audio regs
		ld a,(iy+8) ;Intrument type
		cp 0
		call z,psg_square

		ld a,(iy+8) ;Intrument type
		cp 1
		call z,psg_noise

		ld a,(iy+8) ;Intrument type
		cp 2
		call z,psg_triangle


		;--- Ramp calculators ----
		;cramp 0 left ,1 ivol,2 vvol,3 ipitch,4 vpitch,5 chamodes
		ld a,(iy+4)
		ld b,a
		ld a,(iy+5)
		add b
		ld (iy+4),a ; Volume ramp math

		ld a,(iy+6)
		ld b,a
		ld a,(iy+7)
		add b
		ld (iy+6),a ;Pitch ramp math

		ld a,(iy+3)
		dec a
		ld (iy+3),a ;ramp size counter
ret
	end_sfx:
		ld l,0
		ld a,(c_cha)
		add 8
		ld b,a
		call load8_psg ;Mute the current channel to end the ramp
	ret



psg_square:

	ld a,(c_cha)
	cp 0
	jp nz,no_ch1
	ld a,(chaen)
	and $ff-1 ;limpa bit 1
	or 8 ;liga bit 4
	ld (chaen),a
	no_ch1:
	ld a,(c_cha)
	cp 1
	jp nz,no_ch2
	ld a,(chaen)
	and $ff-2 ;limpa bit 2
	or 16 ;liga bit 8
	ld (chaen),a
	no_ch2:
	ld a,(c_cha)
	cp 2
	jp nz,no_ch3
	ld a,(chaen)
	and $ff-4 ;limpa bit 2
	or 32 ;liga bit 8
	ld (chaen),a
	no_ch3:

	call psg_cfg
	ld a,(iy+6) ;pitch bend
	rlca
	rlca
	ld b,a
	and $3
	ld h,a
	ld a,b
	and $ff-3
	ld l,a
	ld d,(iy+10) ;contains the actual note
	ld e,(iy+11)
	add hl,de
	ld a,(c_cha)
	add a
	ld b,a ;Frequency loaded from the channel
	call load16_psg ;Frequency loaded
ret 

psg_noise:
	ld a,(c_cha)
	cp 0
	jp nz,no_ch1_noise
	ld a,(chaen)
	and $ff-8 ;limpa bit 1
	or 1 ;liga bit 4
	ld (chaen),a
	no_ch1_noise:
	ld a,(c_cha)
	cp 1
	jp nz,no_ch2_noise
	ld a,(chaen)
	and $ff-16 ;limpa bit 2
	or 2 ;liga bit 8
	ld (chaen),a
	no_ch2_noise:
	ld a,(c_cha)
	cp 2
	jp nz,no_ch3_noise
	ld a,(chaen)
	and $ff-32 ;limpa bit 2
	or 4 ;liga bit 8
	ld (chaen),a
	no_ch3_noise:
	call psg_cfg
	ld a,(iy + 14)
	inc a
	and 31
	ld l,a
	ld b,6
	call load8_psg
ret

old_tri: .db 0

psg_triangle:
	;You have to turn off the channel to make triangle wave work, or you get choppy triangle wave
	ld a,(c_cha)
	cp 0
	jp nz,no_ch1_tri
	ld a,(chaen)
	or 1+8 ;Nopes CH1
	ld (chaen),a
	no_ch1_tri:
	ld a,(c_cha)
	cp 1
	jp nz,no_ch2_tri
	ld a,(chaen)
	or 16+2 ;Nopes CH2
	ld (chaen),a
	no_ch2_tri:
	ld a,(c_cha)
	cp 2
	jp nz,no_ch3_tri
	ld a,(chaen)
	or 32+4 ;Nopes CH3
	ld (chaen),a
	no_ch3_tri:

	ld a,(c_cha)
	add 8
	ld b,a
	ld l,16 
	call load8_psg ;Envelope time


	ld a,(iy+6) ;pitch bend
	rlca
	rlca
	ld b,a
	and $3
	ld h,a
	ld a,b
	and $ff-3
	ld l,a
	ld d,(iy+10) ;contains the actual note
	ld e,(iy+11)
	add hl,de

	xor a
	ld a,h
	rrca
	rrca
	rrca
	rrca
	and $f0
	ld h,a
	xor a 
	ld a,l
	rrca
	rrca
	rrca
	rrca
	and $0f 

	add a,h
	ld h,0
	ld l,a
	ld b,$0b
	;;ld hl,$0038 
	call load16_psg ;Real frequency loaded

ret 

psg_cfg:
	ld a,(iy+4)
	and $f0
	rrca
	rrca
	rrca 
	rrca ;Divide by 16
	ld l,a
	ld a,(c_cha)
	add 8
	ld b,a	;Set volume of the current channel
	call load8_psg
ret 







;This read the instrument from the instrument list in memory
;4-5 current instrument pointer, 6 loop vals (packed), 7 SCC channel?
;A instrument ID,B channel

load_instrument:
	push hl
	push ix
	push iy
	ld hl, ($840c) ;huh, the Z80 can do that
	add a 
	add l 
	ld l,a
	ld a,h ;Calculate the address for the instruments
	adc a,0
	ld h,a
	ld a,(hl)
	inc hl
	ld ixl,a
	ld a,(hl)
	ld ixh,a ;IX now points to the instrument data

	ld a,b
	rlca
	rlca
	rlca
	ld iy,chvars 
	add iyl
	ld iyl,a
	ld a,0
	adc a,iyh
	ld iyh,a ;and with this channel var is set

	ld a,(ix + 0)
	ld (iy+7),a ;copy SCC magical number
	ld a,(ix+1)
	ld (iy+6),a ;loop init/point
	push ix
	pop hl
	ld a,2
	add l
	ld (iy+4),a
	ld a,0
	adc a,h
	ld (iy+5),a ;and copy the rest of the ramp
	pop iy
	pop ix
	pop hl
ret


sfx_shoot: 
;numblocks: ivol/rampsize,ipitch,vpitch(signed),vvol & audio (1) & noise (2) 
.db 2
.db $f3,$40,$f0,($f8) 
.db $ff,$10,$2,($f0) 


gen_piano: 
;numblocks: ivol/rampsize,ipitch,vpitch(signed),vvol & audio (1) & noise (2) 
.db 2
.db $08,$0,$0,($20) + 1
.db $ff,$0,$0,($f0) + 1
