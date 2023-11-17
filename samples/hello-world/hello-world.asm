lorom

; put-character (from low 8-bits of accumulator)
macro blaze_pch()
	wdm #$80
endmacro

org $008000
main:
	; switch to native mode
	clc
	xce

	; reset status flags
	rep #$ff
	sep #$04

	; set up the stack pointer
	lda #$01ff
	tcs

	ldx #hello_world_string
	jsr print_string

	jmp hang

; input: string address in X register
; must be in native mode with 16-bit index registers
print_string:
	.init:
		; save processor status
		php
		; use 8-bit memory and accumulator
		sep #$20

	.loop:
		; load the next character
		lda 0, x

		; if it's zero (a null terminator), we're done
		cmp #0
		beq .done

		; print the character
		%blaze_pch()

		; advance the pointer
		inx

		; continue the loop
		jmp .loop

	.done:
		; restore processor status
		plp
		; return to caller
		rts

native_cop:
native_brk:
native_abort:
native_nmi:
native_irq:

emu_cop:
emu_abort:
emu_nmi:
emu_irq_brk:

hang:
	jmp hang

hello_world_string:
	; $0A = newline
	db "Hello, world!", $0A, 0

rom_header:
	org $00ffc0

	; PC = $00ffc0
	.title:
		db "HELLO WORLD SAMPLE"
		padbyte $20 ; ASCII $20 = space
		pad $00ffd5

	; PC = $00ffd5
	.mapping_mode:
		db $20 ; LoROM

	; PC = $00ffd6
	.type:
		db 0 ; ROM only

	; PC = $00ffd7
	.size:
		db 15 ; 32 KiB -> log2(32) -> 5

	; PC = $00ffd8
	.sram_size:
		db 0 ; 1 KiB -> log2(1) -> 0, but we actually don't want/need SRAM

	; PC = $00ffd9
	.destination_code:
		db 1 ; USA

	; PC = $00ffda
	.fixed_value:
		db $33 ; must always be $33

	; PC = $00ffdb
	.version
		db 0

	; the checksum values are supposed to be updated by the assembler (asar)

	; PC = $00ffdc
	.checksum_complement:
		db 0

	; PC = $00ffde
	.checksum:
		db 0

native_vectors:
	org $00ffe4

	; PC = $00ffe4
	.cop:
		dw native_cop

	; PC = $00ffe6
	.brk:
		dw native_brk

	; PC = $00ffe8
	.abort
		dw native_abort

	; PC = $00ffea
	.nmi:
		dw native_nmi

	; PC = $00ffec
	.reserved:
		dw hang

	; PC = $00ffee
	.irq:
		dw native_irq

emulation_vectors:
	org $00fff4

	; PC = $00fff4
	.cop:
		dw emu_cop

	; PC = $00fff6
	.reserved:
		dw hang

	; PC = $00fff8
	.abort:
		dw emu_abort

	; PC = $00fffa
	.nmi:
		dw emu_nmi

	; PC = $00fffc
	.reset:
		dw main

	; PC = $00fffe
	.irq_brk:
		dw emu_irq_brk
