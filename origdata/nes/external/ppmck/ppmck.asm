;	.list
	.include	"define.inc"

	.ifndef	MAKE_NES
MAKE_NES	.equ	0
	.endif

	.ifndef	START_SONG
START_SONG	.equ	1
	.endif

	.if MAKE_NES
; INES header setup
	.inesprg	2	; 16k PRG bank
	.ineschr	1	; 8k CHR bank
	.inesmir	0	; Vertical map mirroring
	.inesmap	0	; Mapper
	.endif

	.bank	0
	.org	$8000
	.code
        
;NSF HEADER
	db	"NESM",$1A	;
	db	1		;Ver.
	db	TOTAL_SONGS	;Number of Songs
	db	START_SONG	;Start Song No.
	dw	LOAD		;Load
	dw	INIT		;Init
	dw	PLAY		;Play
  .org	$800E
	TITLE
;	db	"Title",$0
  .org	$802E
	COMPOSER
;	db	"Composer",$0
  .org	$804E
	MAKER
;	db	"Maker",$0
  .org	$806E
	dw	16666		;1000000 / (freq of NTSC) sec
	.if	(ALLOW_BANK_SWITCH)
	BANKSWITCH_INIT_MACRO
	.else
	db	0,0,0,0,0,0,0,0 ;Bankswitch Init Values
	.endif
	dw	20000		;1000000 / (freq of PAL)  sec
	db	%00
;                ||
;                |+-------------- PAL/NTSC
;                +--------------- dual PAL/NTSC tune or not
;	db	$02

__VRC6	=	%00000001
__VRC7	=	%00000010
__FDS	=	%00000100
__MMC5	=	%00001000
__N106	=	%00010000
__FME7	=	%00100000
	db	SOUND_GENERATOR
	db	0,0,0,0
LOAD:
INIT:
	jsr	sound_init
	rts
PLAY:
	jsr	sound_driver_start
	rts
;-------------------------------------------------------------------------------
	.include	"ppmck/sounddrv.h"
	.include	"ppmck/internal.h"
	.include	"ppmck/dpcm.h"
	.if	SOUND_GENERATOR & __FDS
	.include	"ppmck/fds.h"
	.endif
	.if	SOUND_GENERATOR & __VRC7
	.include	"ppmck/vrc7.h"
	.endif
	.if	SOUND_GENERATOR & __VRC6
	.include	"ppmck/vrc6.h"
	.endif
	.if	SOUND_GENERATOR & __N106
	.include	"ppmck/n106.h"
	.endif
	.if	SOUND_GENERATOR & __MMC5
	.include	"ppmck/mmc5.h"
	.endif
	.if	SOUND_GENERATOR & __FME7
	.include	"ppmck/fme7.h"
	.endif
	.include	"ppmck/freqdata.h"
	.include	"effect.h"

;-------------------------------------------------------------------------------
	.if MAKE_NES

songno    = $0100
pad_click = $0101
pad_press = $0102
nmi_flag  = $0103 ;$ff when play needed
wantinit  = $0104 ;$ff when init
OldSong    = $0105

 .bank 0
NMI:
	bit	wantinit
	bmi	song_init_1
	bit	nmi_flag
	bmi	do_rti		; 処理落ち
	dec	nmi_flag
IRQ:
do_rti:
	rti
	

song_init_1:
	ldx	#$ff
	txs
	inx
	jmp	song_init


RESET:				; このあたりのXレジスタの使い方はQuietustさんの方法を参考にしました
	sei
	cld
	ldx	#$00
	stx	$2000
	stx	$2001

	stx	pad_click
	stx	pad_press

	ldx #0
	dex
	txs		;#$ff
	
	lda #0
	ldx #0
.ram:	sta	<$0000,x
	sta	$100,x
	sta	$200,x
	sta	$300,x
	sta	$400,x
	sta	$500,x
	sta	$600,x
	sta	$700,x
	sta	$800,x
	sta	$900,x
	inx
	bne	.ram

	lda	#START_SONG-1
	sta	songno

	LDX #$02                ; warm up
WarmUp:
	BIT $2002
	BPL WarmUp
	DEX
	BNE WarmUp

; palette stuff
      	LDA #$3F
	STA $2006
	LDA #$00
	STA $2006
        TAX

LoadPal:                        ; load palette
        LDA palette, x
        STA $2007
        INX
        CPX #$20
        BNE LoadPal

	LDA #$20
	STA $2006
	LDA #$00
	STA $2006
	
	LDY #$04                ; clear nametables
ClearName:
	LDX #$00
	LDA #$00
PPULoop:
	STA $2007
	DEX
	BNE PPULoop

	DEY
	BNE ClearName


        JSR DrawScreen          ; draw initial nametable

song_init:
;       ----------------------------------------------------
	ldx	#$00			; X = 0 indicates NTSC
	stx	nmi_flag
	stx	wantinit
	stx	$4015
	lda	songno
	jsr	INIT

        JSR InitSprites
	JSR ChangeSprite
        JSR Vblank              ; turn on screen

mainloop:
	bit	nmi_flag	; wait NMI
	bpl	mainloop
	;JSR Vblank
        JSR UpdateSprites
	;JSR DrawScreen
        JSR ControllerTest      ; check for user input
        ;jsr PlayAddy            ; play the music
        jsr PLAY

	inc	nmi_flag

	jmp	mainloop

;       ----------------------------------------------------

DrawScreen:

        LDA #LOW(pic)              ; load low byte of first picture
        STA $10
        LDA #HIGH(pic)              ; load high byte of first picture
        STA $11

   	LDA #$20                ; set to beginning of first nametable
    	STA $2006
    	LDA #$00
    	STA $2006

        LDY #$00
        LDX #$04

NameLoop:                       ; loop to draw entire nametable
        LDA [$10],y
        STA $2007
        INY
        BNE NameLoop
        INC $11
        DEX
        BNE NameLoop

        RTS

;       ----------------------------------------------------

InitSprites:
      LDA #$ff
      LDX #$00
ClearSprites:
      STA $500, x
      INX
      BNE ClearSprites

      LDA #$00
      STA $2003                 ; set the low byte (00) of the RAM address
      LDA #$05
      STA $4014                 ; set the high byte (05) of the RAM address

LoadSprites:
      LDX #$00
LoadSpritesLoop:
      LDA sprites, x            ; load data from address
      STA $0500, x              ; store into RAM address
      INX
      CPX #4
      BNE LoadSpritesLoop
      RTS

sprites:
           ;vert tile attr horiz
        .db $10, $3E, $00, $10  ; sprite

;       ----------------------------------------------------

UpdateSprites:
        LDA #$00
        STA $2003
        LDA #$05
        STA $4014
        RTS

;       ----------------------------------------------------

ChangeSprite:
        LDX songno
        LDA Line,x
        STA $0500
        RTS

Line:
        .db $36,$3E,$46,$4E,$56,$5E,$66,$6E,$76,$7E,$86,$8E,$96,$9E,$A6

;       ----------------------------------------------------

Vblank:                         ; turn on the screen and start the party
	BIT $2002
	BPL Vblank

        LDX #$00
        STX $2005
        STX $2005

	LDA #%10001000
	STA $2000
        LDA #%00011110
	STA $2001

        RTS

;       ----------------------------------------------------

ControllerTest:

        LDA pad_press
	STA pad_click

	ldy	#$08
	ldx	#$01
	stx	$4016
	dex
	stx	$4016
.nextbit:
	lda	$4016	; A B Select Start Up Down Left Right
	ror	a	; bit0 into C
	txa		;
	ror	a	; C into bit7
	tax		; X=A=C<<7|X>>1
	dey
	bne	.nextbit

	sta	pad_press
	eor	pad_click
	and	pad_press
	sta	pad_click

check_pad:
	lda	pad_click
	tax			; X = pad_click
	beq	EndDrawChk

CheckUp:
	and #%00010000
	BEQ CheckDown

        DEC songno          ; decrement song number here
        BPL EndDrawChk
	inc songno

CheckDown:
	txa
	and	#%00100000
	beq	EndDrawChk

	INC songno          ; increment song number here
        LDA #15
	CMP songno
	BNE EndDrawChk
	dec songno

EndDrawChk:
	LDA songno          ; has song number changed? if so, load next song
	CMP OldSong
	BEQ CheckOver
	STA OldSong
        jmp song_init_1;

CheckOver:

        RTS

;       ----------------------------------------------------
palette:                        ; palette data
        db $0F,$10,$20,$30,$0F,$10,$20,$30,$0F,$10,$20,$30,$0F,$10,$20,$30
        db $0F,$10,$20,$30,$0F,$10,$20,$30,$0F,$10,$20,$30,$0F,$10,$20,$30

pic:
        .INCBIN "screen.nam"

;       ----------------------------------------------------

 .bank 3
	.ORG $fffa              ; vectors
	.DW NMI
	.DW RESET
	.DW IRQ

;       ----------------------------------------------------

 .bank 4
	; NROM should have 8KB CHR-ROM

        .INCBIN "geo.chr"

	.endif
