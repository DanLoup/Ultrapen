;Disk memory map
;BB00 Decoded fat of the current file
;BA00 Drive type (0 WD,1 Microsol, 2 Toshiba)
;BA01-BA0A Memory registers (case 0), Registers (case 1 & 2)
;BA0B-C side 0/1

Drivetable: .db $f8,$01,$00 ,$b8,$0d,$09, $88,$0d,$09, ,$00

;BA10-BA1F temp registers 

;HL = filename
ReadFS:
	di
	ld b,8 ;First thing, we need the FAT/file tables etc, so we read the sectors and stuff
	xor a ;Resets carry AND sets A to zero, how convenient
	ld c,$f9 ;I have no freaking clue on what a media descriptor is, but will guess its that number on the header
	ld de,0 ;The first ones
	ld hl,$9000 ;4KB after the disk loader, so we have 12KB on this bank
	xor a
	call $4010 ;Drive
	di
	ld a,($9015)
	ld (disktype),a ;Disk type
	ld a,2
	ld (clustersize),a ;Assembles the media descriptor/2 combo i use on the actual file reading
	ld a,($9010) ;Number of FAT copies
	ld b,a
	ld a,($9016) ;Fat size (in sectors)
	ld c,a
	ld a,0
	fatcalcloop:
		add a,c
		dec b	
	jp nz,fatcalcloop
	ld (seccount),a ;Saving the basic sector count, as actual file accesses are by sector, not stuff
	rla
	add a,$92 ;Memory adress + header sector
	ld h,a
	ld l,0 ;And this sets HL into the file table adress
	ld ($8d00),hl ;That is conveniently stored here

	ld hl,($9011) ;File table size (in file count)
	ld b,4
	ftdivloop:
		rr h
		rr l 
		dec b
	jp nz,ftdivloop
	ld a,(seccount) ;Getting partial number
	add a,l
	sub 3 ; The cluster id thing starts with 2, so with this -4 offsef (-1 from the header that was not added), this gets the correct value for the routine below
	ld (seccount),a
	ld de,filenamec ;Filename
	ld hl,($8d00)	;File table
	ld c,11
	FileCounter:
		dec c
		jp z,foundfile
		ld a,(de)
		ld b,(hl)
		inc de
		inc hl
		cp b
		jp z, FileCounter
		ld de,filenamec ;Filename
		ld a,l
		and $F0 ;Remove the lower bits so we get the start of the next file
		ld l,a
		ld bc,32
		add hl,bc
		ld c,11
		ld a,($8d01) ;high byte to compare
		add a,2
		cp h ;If file 400 then stuff
	jp nz,FileCounter
	foundfile:
		ld a,l
		and $F0 ;Again, remove bits
		add a,$1a ;Add to the pointer to the first cluster in file
		ld l,a
		ld e,(hl)
		inc hl
		ld d,(hl)
		ex de,hl ;and freaking finally, hl contains the first fat cluster of the file and $1202 contains the first sector of the clustering
ret


LoadFile:
	ld bc,12
	ld de,filenamec
	ldir
	call SetDiskmode
	call ReadFS
	call GetClusterList
	ld bc,Clusters
	ld hl,$8400
	filerloop:
	push hl
	ld a,(bc) ;Number of sectors of this block
	inc bc
	cp $ff
	jp z,endread
	or a
	rlca ;multiply number of sectors by 2 because fat16 seems to work with sector pairs
	ld (disktype+1),a ;Writes on the pair thing the number of sectors
	ld a,(bc) ;Cluster high
	ld d,a
	inc bc
	ld a,(bc) ;Cluster low
	inc bc
	push bc
	ld e,a ;Cluster ID
	rlc d
	rl e ;Cluster need to be vicided
	ld a,(seccount) ;Now that's complicated
	add a,e
	ld e,a
	ld a,d
	adc 0
	ld d,a ;Add high bit to the math

	ld bc,(disktype) ;sector count, disk
	xor a;Clears A and carry
	call $4010 ;Calls the bios thing
	di
	pop bc
	pop hl
	ld de,$400
	add hl,de
	jp filerloop
	endread:
	pop hl
	ld a,0
	out ($99),a
	ld a,64
	out ($99),a
	call SetMemmode
	call stopmotor
ret
	
	;HL give this routine the first address, table stored at F500->up?
	GetClusterList:
		ld bc,Clusters
		ld a,1
		ld (cl_Count),a ;This holds the current number of clusters to be loaded in series
		ld (cl_Ini),hl ;this holds not a memory address, but a cluster address
		ld (cl_Follow),hl ;a
		jp firstloop
	ClusterLoop:

		push hl 
		ld de,(cl_Follow) ;This loads the previous value
		ex de,hl ;And push it to HL, while pushing the current to DE
		xor a
		sbc hl,de ;does previous - current
		ld a,l
		cp $ff ;If the current value is 1 bigger than the previous, we increase the counter
		jp z,nextaccess 
		ld a,(cl_Count) ;if not, we write the initial position and size to the table
		ld (bc),a
		inc bc
		ld a,(cl_Ini+1)
		ld (bc),a
		inc bc
		ld a,(cl_Ini)
		ld (bc),a
		inc bc
		ld a,1
		ld (cl_Count),a ;The counter resets
		ld (cl_Ini),de ;New value, stored directly from the hl/de swap
		pop hl
		jp firstloop

		nextaccess:
		ld hl,cl_Count
		inc (hl) ;adds a sector to the count streak
		ld (cl_Follow),de ;previous value is now current value
		pop hl
		
		firstloop:
		ld d,h
		ld e,l ;with this, we have the actual initial address stored by the push/pops
		add hl,de
		add hl,de; adress * 3
		xor a
		rr h
		rr l ;divide by 2, and get the nibble 1 or 2 on the carry, quite fancy
		jp c,imparaddr ;here we select if we par or impar
		ld de,$9200 ;Fat seems to be always here (unless the MSX-Dos program gets extended, but you shouln't be using those weird disks anyway)
		add hl,de ;finishes assembling adress
		ld e,(hl) ;lower byte
		inc hl
		ld a,(hl) ;higher byte
		and $0f ;(but just the lower half)
		ld d,a
		ex de,hl ;HL now contains the next jump address
		ld a,l
		and $f0 ;if the lower byte is higher than f0 (all possible endings)
		cp $f0
		jp c,ClusterLoop
		ld a,h
		and $0f ;and the higher is 0f, we end loading right here
		cp $0f
		jp nz,ClusterLoop
		jp endfa

	imparaddr:
		ld de,$9200
		add hl,de 
		ld a,(hl) ;first byte
		inc hl
		and $f0 ;get high nibble
		ld e,a 
		ld a,(hl)
		and $0f ;get low nibble
		or e ;and combine both.. on the wrong orientation
		rrca
		rrca
		rrca
		rrca ;But after 4 "pacmanings" they're just in place
		ld e,a ;And now that's the correct one
		xor a

		ld a,(hl)
		srl a
		srl a
		srl a
		srl a ;And here we get the highest bit, but divide it by 16
		ld d,a

		inc hl ;On impar acesses, you jump twice, so you get 1-2 pattern
		ex de,hl
		ld a,l
		and $f0
		cp $f0 ;Again if higher than f0
		jp c,ClusterLoop
		ld a,h
		and $0f
		cp $0f ;and 0f, its end of the file
		jp nz,ClusterLoop

		endfa:
		ld a,(cl_Count) ;and here, we finally write the final sectors
		ld (bc),a
		inc bc
		ld a,(cl_Ini+1)
		ld (bc),a
		inc bc
		ld a,(cl_Ini)
		ld (bc),a
		inc bc
		ld a,$ff
		ld (bc),a
		inc bc
		ld (bc),a
		inc bc
		ld (bc),a
	ret

	cl_Count: .ds 2,1
	cl_Ini: .ds 2,1
	cl_Follow: .ds 2,1
	
stopmotor:
	ld a,0
	ld ($7ffd),a
	ld ($7ffc),a
	out ($d4),a ;Just attempts to stop the motor from spinning on all the portar documented drives
ret 


