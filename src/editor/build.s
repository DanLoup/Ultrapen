.MEMORYMAP
SLOTSIZE $2000
DEFAULTSLOT 0
SLOT 0 $9aaf
.ENDME
.ROMBANKSIZE $2000
.ROMBANKS 1
.ORG $0
;upen
;met
;met_gfx
ld c,$ff
loop:
dec c
jp nz,loop
ret
