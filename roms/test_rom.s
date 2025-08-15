SECTION "Header", ROM0[$0100]
    nop
    jp start

SECTION "Main", ROM0[$0150]
start:
    ld a, $12        ; A = 0x12
    ld b, $01        ; B = 0x01
    inc c
    add a, b
    sub b
    ld hl, $C000
    ld [hl], a
	inc b
	jp next

SECTION "Next", ROM0[$0160]
next:
    dec b
    jr nz, next
    jp final

SECTION "Final", ROM0[$0170]
final:
    ld a, $34
    halt

