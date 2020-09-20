; Small 40 column console driver for the Commodore VIC20.
; Requirements: at least 8KB of RAM expansion.
;
; Author: D. Bucci
; Some of the code is guided by Mike's advices from the Denial forum:
; http://sleepingelephant.com/ipw-web/bulletin/bb/viewtopic.php?f=2&t=9257

.export _initGraphic
.export _normalText
.export _putc40ch
.export _puts40ch
.export _gets40ch
.export _clrscr
.export _cgetc40ch
.export _negative
.export _positive

.import popax

.importzp ptr1, ptr2, ptr3, ptr4
.importzp tmp1, tmp2, tmp3

    setp=ptr1
    scrp=ptr2
    stri=ptr3
    

    buffer=ptr4
    maxlen=tmp1
    ptrstr=tmp2
    chartmp=tmp3

    startm=$1100
    
    foreground = 1      ; Foreground color
    background = 8      ; Background and border color
    
    cursor = '-'
.segment "LOADHI"
.byte 00, $A0
.segment "RODATA"

    GETIN =  $FFE4      ; Get a key from the keyboard
    VIC_DEFAULT=$EDE4   ; Table of the default values of VIC registers

; Original character set from Mike's MG BROWSE.
TCharset:
 .byte   0,   0,   0,   0,   0,   0,   0,   0
 .byte   0,  34,  34,  34,  34,   0,  34,   0
 .byte   0,  85,  85,   0,   0,   0,   0,   0
 .byte   0,  85, 119,  85,  85, 119,  85,   0
 .byte   0,  34,  51, 102,  51, 102,  34,   0
 .byte   0,  85,  17,  34,  34,  68,  85,   0
 .byte   0, 102, 102,  51, 102, 102,  51,   0
 .byte  34,  34,   0,   0,   0,   0,   0,   0
 .byte   0,  17,  34,  34,  34,  34,  17,   0
 .byte   0,  68,  34,  34,  34,  34,  68,   0
 .byte   0,   0,   0,  85,  34,  85,   0,   0
 .byte   0,   0,   0,  34, 119,  34,   0,   0
 .byte   0,   0,   0,   0,   0,   0,  34,  68
 .byte   0,   0,   0,   0, 119,   0,   0,   0
 .byte   0,   0,   0,   0,   0,   0,  34,   0
 .byte   0,  17,  17,  34,  34,  68,  68,   0
 .byte   0, 119,  85,  85,  85,  85, 119,   0
 .byte   0,  17,  17,  17,  17,  17,  17,   0
 .byte   0, 119,  17, 119,  68,  68, 119,   0
 .byte   0, 119,  17, 119,  17,  17, 119,   0
 .byte   0,  85,  85, 119,  17,  17,  17,   0
 .byte   0, 119,  68, 119,  17,  17, 119,   0
 .byte   0, 119,  68, 119,  85,  85, 119,   0
 .byte   0, 119,  17,  17,  17,  17,  17,   0
 .byte   0, 119,  85, 119,  85,  85, 119,   0
 .byte   0, 119,  85, 119,  17,  17, 119,   0
 .byte   0,   0,   0,  34,   0,   0,  34,   0
 .byte   0,   0,   0,  34,   0,   0,  34,  68
 .byte   0,   0,  17,  34,  68,  34,  17,   0
 .byte   0,   0,   0, 119,   0, 119,   0,   0
 .byte   0,   0,  68,  34,  17,  34,  68,   0
 .byte   0,  34,  85,  17,  34,   0,  34,   0
 .byte   0,  51,  85,  85,  85,  68,  51,   0
 .byte   0,  34,  85,  85, 119,  85,  85,   0
 .byte   0, 102,  85, 102,  85,  85, 102,   0
 .byte   0,  51,  68,  68,  68,  68,  51,   0
 .byte   0, 102,  85,  85,  85,  85, 102,   0
 .byte   0, 119,  68, 102,  68,  68, 119,   0
 .byte   0, 119,  68, 102,  68,  68,  68,   0
 .byte   0,  51,  68,  68,  85,  85,  51,   0
 .byte   0,  85,  85, 119,  85,  85,  85,   0
 .byte   0, 119,  34,  34,  34,  34, 119,   0
 .byte   0,  51,  17,  17,  17,  85,  34,   0
 .byte   0,  85,  85, 102,  85,  85,  85,   0
 .byte   0,  68,  68,  68,  68,  68, 119,   0
 .byte   0,  85, 119,  85,  85,  85,  85,   0
 .byte   0, 102,  85,  85,  85,  85,  85,   0
 .byte   0,  34,  85,  85,  85,  85,  34,   0
 .byte   0, 102,  85,  85, 102,  68,  68,   0
 .byte   0,  34,  85,  85,  85, 102,  51,   0
 .byte   0, 102,  85,  85, 102,  85,  85,   0
 .byte   0,  51,  68,  34,  17,  17, 102,   0
 .byte   0, 119,  34,  34,  34,  34,  34,   0
 .byte   0,  85,  85,  85,  85,  85,  51,   0
 .byte   0,  85,  85,  85,  85,  34,  34,   0
 .byte   0,  85,  85,  85,  85, 119,  85,   0
 .byte   0,  85,  85,  34,  85,  85,  85,   0
 .byte   0,  85,  85,  85,  34,  34,  34,   0
 .byte   0, 119,  17,  34,  34,  68, 119,   0
 .byte   0,  51,  34,  34,  34,  34,  51,   0
 .byte   0,  68,  68,  34,  34,  17,  17,   0
 .byte   0, 102,  34,  34,  34,  34, 102,   0
 .byte  34,  85,   0,   0,   0,   0,   0,   0
 .byte   0,   0,   0,   0,   0,   0,   0, 255
 .byte  34,  17,   0,   0,   0,   0,   0,   0
 .byte   0,   0,   0,  51,  85,  85,  51,   0
 .byte   0,  68,  68, 102,  85,  85, 102,   0
 .byte   0,   0,   0,  51,  68,  68,  51,   0
 .byte   0,  17,  17,  51,  85,  85,  51,   0
 .byte   0,   0,   0,  34,  85, 102,  51,   0
 .byte   0,  17,  34, 119,  34,  34,  34,   0
 .byte   0,   0,   0,  51,  85,  51,  17, 102
 .byte   0,  68,  68, 102,  85,  85,  85,   0
 .byte   0,  34,   0,  34,  34,  34,  34,   0
 .byte   0,  34,   0,  34,  34,  34,  34,  68
 .byte   0,  68,  68,  85, 102,  85,  85,   0
 .byte   0,  34,  34,  34,  34,  34,  34,   0
 .byte   0,   0,   0,  85, 119,  85,  85,   0
 .byte   0,   0,   0, 102,  85,  85,  85,   0
 .byte   0,   0,   0,  34,  85,  85,  34,   0
 .byte   0,   0,   0, 102,  85, 102,  68,  68
 .byte   0,   0,   0,  51,  85,  51,  17,  17
 .byte   0,   0,   0, 102,  85,  68,  68,   0
 .byte   0,   0,   0,  51, 102,  51, 102,   0
 .byte   0,  34,  34, 119,  34,  34,  51,   0
 .byte   0,   0,   0,  85,  85,  85,  51,   0
 .byte   0,   0,   0,  85,  85,  34,  34,   0
 .byte   0,   0,   0,  85,  85, 119,  85,   0
 .byte   0,   0,   0,  85,  34,  34,  85,   0
 .byte   0,   0,   0,  85,  85,  51,  17, 102
 .byte   0,   0,   0, 119,  17,  34, 119,   0
 .byte   0,  51,  34,  68,  34,  34,  51,   0
 .byte   0,  34,  34,   0,  34,  34,  34,   0
 .byte   0, 102,  34,  17,  34,  34, 102,   0
 .byte   0,   0,   0,  85, 170,   0,   0,   0
 .byte   0,   0,   0,   0,   0,   0,   0,   0

; Wanted VIC config:
; C:9000  0e 22 14 19  00 cc 57 ea  ff ff 00 00  00 00 00 08
;          |  |  | |       |                               +-> Black screen
;          |  |  | |       +-> Screen address (video and chargen ad $1000)
;          |  |  | +-> 8x16 characters, 12 lines of text (will become 24)
;          |  |  +-> 20 columns that will become 40!
;          |  +-> Distance from origin to the first row
;          +-> Distance from origin to the first column
; PAL system at startup:
; C:9000  0c 26 16 2e  00 c0 57 ea  ff ff 00 00  00 00 00 1b
;
; Difference:
;         02 FC FE EB  00 0C
;
; Results with NTSC:
; C:9000  08 15 14 19  00 cc 57 ea  ff ff 00 00  00 00 00 08
 
Offset:
    .byte $02, $FC, $FE, $EB, $00, $0C


; Store some information. Those may be in page 0 to spare a few bytes.
.segment "BSS"
currLine:
    .byte 0
currCol:
    .byte 0
currMask:
    .byte 0
isNeg:
    .byte 0

; See _puts40ch, it is required for the non-self modifying code.
;savey:
;    .byte 0

; You can use the CODE segment, alternatively. 
.segment "HIMEM"

 
; Set the screen in a 160x172 pixels graphic mode.
; The video memory starts at $1000 and the chargen memory at $1000.
; There's a trick to access to chargen, i.e. to avoid the first 16 characters
; so that the bitmap is mapped at $1100.
_initGraphic:
    ldy #$05
@On_02:
    clc
    lda VIC_DEFAULT,Y   ; Read default values from the KERNAL rom
    adc Offset,Y
    sta $9000,Y
    dey
    bpl @On_02
    lda #background
    sta $900F           ; Screen background
    jsr _clrscr
    rts

_normalText:
    ldy #$F
@On_02:
    lda VIC_DEFAULT,Y   ; Read default values from the KERNAL rom
    sta $9000,Y
    dey
    bpl @On_02
    jsr $e55f
    rts

; Clear the screen.
_clrscr:
    ldx #0
    stx currLine
    stx currCol
    lda #foreground
@lig:                   ; In a first loop, we set up the foreground colour
    sta $9400,x
    inx
    cpx #240
    bne @lig
    
    lda #$10            ; Now we write the characters on screen. Column wise
    pha
    ldx #0
@loop:
    ldy #20
@lig2:
    sta $1000,x
    clc
    adc #12
    inx
    dey
    bne @lig2
    pla
    clc
    adc #1
    pha
    cpx #240
    bne @loop
    pla
    lda #0
    tax
@lig1:
    sta startm +$000,x
    sta startm +$100,x
    sta startm +$200,x
    sta startm +$300,x
    sta startm +$400,x
    sta startm +$500,x
    sta startm +$600,x
    sta startm +$700,x
    sta startm +$800,x
    sta startm +$900,x
    sta startm +$A00,x
    sta startm +$B00,x
    sta startm +$C00,x
    sta startm +$D00,x
    sta startm +$E00,x
    inx
    bne @lig1
    rts


; Write a zero-terminated string pointed in A/X on the screen.
; This is self-modifying code. It will not run from ROM.
; I leave here the original code, as comment, it requires register Y, so
; it requires the savey location.
_puts40ch:
    ;sta stri
    ;stx stri+1
    sta @loop+1 ; **** modifies the address in the lda op below
    stx @loop+2 ; ****
    ;ldy #0
    ldx #0      ; ****
@loop:
    ;lda (stri),y
    lda $ffff,x  ; Code will be modified here ****
    beq @ex
    ;sty savey
    jsr _putc40ch
    ;ldy savey
    ;iny
    inx
    bne @loop
@ex:
    rts

; Get a keystroke and return it in the A register
_cgetc40ch:
@loop:
    jsr GETIN           ; Last instruction is a TXA, so Z flag is up to date
    beq @loop
    rts

; Input a string in the provided buffer.
; Requires the buffer len in A (<256 bytes)
;
; KNOWN issue: the input can not be larger than the screen width and
; the delete goes crazy. Reaching the maximum length entails a return.
_gets40ch:
    sta maxlen          ; Store the length of the buffer (in A as __fastcall__)
    dec maxlen          ; Buffer must be able to contain final '\0'
    jsr popax           ; Get the address of the buffer
    sta buffer
    stx buffer+1
    ldy #0
@cursor:
    sty ptrstr          ; Store the index
    lda #cursor
    jsr _putc40ch
    dec currCol
@skip:
    jsr _cgetc40ch      ; Get a key
    ldy ptrstr          ; Get the index
    cmp #$14            ; Check if character is backspace
    beq @backsp
    cmp #$9D            ; Check if it is left arrow
    beq @backsp
    cmp #$D
    beq @cont
    cmp #$20
    bcc @skip
    cmp #$DF
    bcs @skip
@cont:
    sta (buffer),y      ; Store the character in the buffer
    sta chartmp
    lda #cursor
    jsr _putc40ch
    dec currCol
    lda chartmp
    jsr _putc40ch       ; Print the character
    ldy ptrstr          ; Check if we still have some storage area
    iny
    lda chartmp
    cmp #$D             ; Check if it is '\r'
    beq @ex             ; If yes, exit
    cpy maxlen
    bne @cursor
@ex:
    lda #0
    sta (buffer),y
    rts

@backsp:
    cpy #0              ; If the string is empty, it can not be deleted
    beq @skip
    dey                 ; Remove a character from the buffer
    sty ptrstr
    dec currCol         ; Move back one column
    lda (buffer),y      ; Erase the old character
    jsr _putc40ch
    lda #cursor         ; Erase the cursor
    jsr _putc40ch
    dec currCol
    dec currCol         ; Move back two columns
    ldy ptrstr
    jmp @cursor

; Show text in negative
_negative:
    lda #$FF
    sta isNeg
    rts

; Show normal text (positive)
_positive:
    lda #$00
    sta isNeg
    rts

; Output one character, contained in A. Don't use X.
_putc40ch:
    cmp #13             ; Check if it's a return
    beq return
    ;jsr shiftpetascii
    
    
    shiftpetascii:
    cmp #$80
    bcc @normal
    sbc #96
@normal:
    cmp #$41
    bcs @cond1
@test2:
    cmp #$61
    bcs @cond2
    bcc @exitpetascii   ; rts
@cond1:
    cmp #$5A
    bcc @fire
    beq @fire
    jmp @test2
@cond2:
    cmp #$7A
    bcc @fire
    beq @fire
    bne @exitpetascii   ; rts
@fire:
    eor #$20
    ;rts
    
@exitpetascii:
    sta setp            ; At first, we need to calculate the offset in the
    lda #0              ; character generator memory. Takes a*8 (keeps track of
    asl setp            ; the higher bits). Store in setp and setp+1
    rol A
    asl setp
    rol A
    asl setp
    rol A
    sta setp+1
    clc
    lda setp            ; Then add the address of TCharset
    adc #.LOBYTE(TCharset-32*8)
    sta setp
    lda setp+1
    adc #.HIBYTE(TCharset-32*8)
    sta setp+1
    jsr calcptr         ; Calculate the address of the screen memory
    lda currCol
    and #1              ; Check if it is a left or right character
    bne right
    lda #$F0            ; Left character
    sta currMask
    bne chcpy           ; Branch always
right:
    lda #$0F            ; Right character
    sta currMask
chcpy:
    ldy #0
@charloop:              ; Copy 8 bytes, with the appropriate mask
    lda (setp),y
    bit isNeg
    bpl @normal
    eor #$FF
@normal:
    and currMask
    eor (scrp),y
    sta (scrp),y
    iny
    cpy #$08
    bne @charloop
    inc currCol
    lda currCol
    cmp #40
    bne nores
return:
    inc currLine
    lda currLine
    cmp #24
    bne @noscroll
    dec currLine
    jsr ScrollUp
@noscroll:
    lda #0
    sta currCol
nores:
    rts

; Calculate the address of the screen pointer from the value of currCol
; and currLine. Store the result in scrp and scrp+1 (LO and HI)
; Don't use X.
calcptr:
    lda #0
    sta scrp+1
    lda currCol     ; ((currCol&0xFE)*6+currLine>>1)<<4+(currLine&1)*8+startm
    and #$FE
    asl             ; 6=4+2
    sta scrp
    asl
    adc scrp
    sta scrp
    lda currLine
    clc
    ror
    clc
    adc scrp
    asl
    rol scrp+1
    asl
    rol scrp+1
    asl
    rol scrp+1
    asl
    rol scrp+1
    sta scrp
    lda currLine
    and #1
    asl
    asl
    asl             ; Carry is clear here
    adc scrp
    sta scrp
    bcc @nohi
    inc scrp+1
@nohi:
    lda #.HIBYTE(startm)    ; Add $1100
    clc
    adc scrp+1
    sta scrp+1
    rts

; Scroll text one line up.
ScrollUp:
    ldy #0
@loop:
    lda startm+8,y
    sta startm,y
    lda startm+8+1*192,y
    sta startm+1*192,y
    lda startm+8+2*192,y
    sta startm+2*192,y
    lda startm+8+3*192,y
    sta startm+3*192,y
    lda startm+8+4*192,y
    sta startm+4*192,y
    lda startm+8+5*192,y
    sta startm+5*192,y
    lda startm+8+6*192,y
    sta startm+6*192,y
    lda startm+8+7*192,y
    sta startm+7*192,y
    lda startm+8+8*192,y
    sta startm+8*192,y
    lda startm+8+9*192,y
    sta startm+9*192,y
    lda startm+8+10*192,y
    sta startm+10*192,y
    lda startm+8+11*192,y
    sta startm+11*192,y
    lda startm+8+12*192,y
    sta startm+12*192,y
    lda startm+8+13*192,y
    sta startm+13*192,y
    lda startm+8+14*192,y
    sta startm+14*192,y
    lda startm+8+15*192,y
    sta startm+15*192,y
    lda startm+8+16*192,y
    sta startm+16*192,y
    lda startm+8+17*192,y
    sta startm+17*192,y
    lda startm+8+18*192,y
    sta startm+18*192,y
    lda startm+8+19*192,y
    sta startm+19*192,y
    iny
    cpy #192-8
    bne @loop
    lda #.LOBYTE(startm+8*23)
    sta scrp
    lda #.HIBYTE(startm+8*23)
    sta scrp+1
@nextch:
    lda #0
    ldy #7
@loop1:
    sta (scrp),y
    dey
    bpl @loop1
    lda scrp
    clc
    adc #192
    sta scrp
    bcc @noc
    inc scrp+1
@noc:
    lda scrp+1
    cmp #$20
    bne @nextch
    rts



