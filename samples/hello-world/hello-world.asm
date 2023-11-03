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

	ldx #hello_world_string
	jsr print_string

	jmp hang

; input: string address in X register
print_string:
	.loop:
		; load the next character
		lda 0, x

		; if it's zero (a null terminator), we're done
		cpx #0
		beq .done

		; print the character
		%blaze_pch()

		; advance the pointer
		inx

		; continue the loop
		jmp .loop

	.done:
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
	db "Hello, world!\n", 0

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
