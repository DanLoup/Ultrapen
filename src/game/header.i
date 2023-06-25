.memorymap
defaultslot 0
slotsize $5000
slot 0 $0000
.endme

.rombankmap
bankstotal 1
banksize $5000
banks 1
.endro

.org $5
ret
.org $50
.include "api.i"

.bank 0 slot 0
.org $38
jp vblank
.org $100
