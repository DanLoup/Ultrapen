.memorymap
defaultslot 0
slotsize $400
slot 0 $8000
.endme

.rombankmap
bankstotal 1
banksize $400
banks 1
.endro

.bank 0 slot 0
.org $0
.db $FE ,$07,$80, $00,$04, $07,$80 ;bin header
jp boot
jp LoadFile ;This is 800A 
