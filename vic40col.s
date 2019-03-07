; Small 40 column console driver for the Commodore VIC20.
; Requirements: at least 8KB of RAM expansion.
;
; Author: D. Bucci
; Some of the code is guided by Mike's advices from the Denial forum:
; http://sleepingelephant.com/ipw-web/bulletin/bb/viewtopic.php?f=2&t=9257

.export _initGraphic
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
    
    foreground = 3      ; Foreground color
    background = 8      ; Background and border color
    
    cursor = '-'

.segment "HIMEM"

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

; Store some information. Those may be in page 0 to spare a few bytes.

currLine:
    .byte 0
currCol:
    .byte 0
currMask:
    .byte 0
isNeg:
    .byte 0
 
; Set the screen in a 160x172 pixels graphic mode.
; The video memory starts at $1000 and the chargen memory at $1000.
; There's a trick to access to chargen, i.e. to avoid the first 16 characters
; so that the bitmap is mapped at $1100.
_initGraphic:
    ldy #$05
On_02:
    clc
    lda VIC_DEFAULT,Y   ; Read default values from the KERNAL rom
    adc Offset,Y
    sta $9000,Y
    dey
    bpl On_02
    lda #background
    sta $900F           ; Screen background
    jsr _clrscr
    rts

; Wanted config:
; C:9000  0d 26 14 19  00 cc 57 ea  ff ff 00 00  00 00 00 08
;          |  |  | |       |                               +-> Black screen
;          |  |  | |       +-> Screen address (video and chargen ad $1000)
;          |  |  | +-> 8x16 characters, 12 lines of text (will become 24)
;          |  |  +-> 20 columns that will become 40!
;          +-> Distance from border to the first column
; PAL system at startup:
; C:9000  0c 26 16 2e  00 c0 57 ea  ff ff 00 00  00 00 00 1b
;
; Difference:
;         01 00 FE EB  00 0C
;
; Results with NTSC:
; C:9000  07 19 14 19  00 cc 57 ea  ff ff 00 00  00 00 00 08
 
Offset:
    .byte $02, $00, $FE, $EB, $00, $0C

; Clear the screen.
; KNOWN ISSUES: the organization of the characters is currently row-wise.
; This greatly complicates the problem of the scrolling, that is currently
; done in a crappy way.
_clrscr:
    ldx #00
    stx currLine
    stx currCol
@lig:
    txa
    clc                 ; We don't use the first 16 characters as they
    adc #$10            ; will occupy in chargen the same memory as video
    sta $1000,x
    lda #foreground
    sta $9400,x
    inx
    cpx #240
    bne @lig
    lda #0
    ldx #0
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


savey:
    .byte 0

; Write a zero-terminated string pointed in A/X on the screen.
_puts40ch:
    sta stri
    stx stri+1
    ldy #0
@loop:
    lda (stri),y
    beq @ex
    sty savey
    jsr _putc40ch
    ldy savey
    iny
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
    jsr shiftpetascii
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
    lda currCol
    and #$FE
    sta scrp            ; Multiply the column number times 8
    lda currLine
    and #$01
    ora scrp
    sta scrp
    lda #0
    asl scrp
    rol A
    asl scrp
    rol A
    asl scrp
    rol A
    sta scrp+1
    lda currLine        ; Add (currLine & $FE)*160, 160=128+32
    ror
    clc
    adc scrp+1          ; (1) Add (currLine & $FE)*128
    sta scrp+1
    lda currLine
    and #$1E
    clc
    asl
    asl
    asl
    asl
    bcc @noadd0
    inc scrp+1
    inc scrp+1
@noadd0:
    asl
    bcc @noadd1
    inc scrp+1
    clc
@noadd1:
    adc scrp
    sta scrp
    bcc @noadd2
    inc scrp+1
    clc
@noadd2:
    lda #.HIBYTE(startm)    ; Add $1100
    adc scrp+1
    sta scrp+1
    rts

l0=ptr1
l1=ptr2
l2=ptr3

; Scroll text one line up.
; KNOWN ISSUES: basically this code is a mess and it is crap.
; One must better organize the characters on the screen.
ScrollUp:
    lda #.LOBYTE(startm)
    sta l0
    lda #.HIBYTE(startm)
    sta l0+1
    lda #.LOBYTE(startm+8)
    sta l1
    lda #.HIBYTE(startm+8)
    sta l1+1
    lda #.LOBYTE(startm+320)
    sta l2
    lda #.HIBYTE(startm+320)
    sta l2+1
    ldy #0
    ldx #0
@loop:
    lda (l1),y
    sta (l0),y
    lda (l2),y
    sta (l1),y
    iny
    tya
    and #7
    bne @loop
    cpy #0
    beq @st
    tya
    clc
    adc #8
    tay
    bne @loop
@st:
    inx
    cpx #15
    beq @ls
    inc l0+1
    inc l1+1
    inc l2+1
    bne @loop
@ls:
    lda #.LOBYTE(startm+8+16*20*11)
    sta l0
    lda #.HIBYTE(startm+8+16*20*11)
    sta l0+1
    lda #.LOBYTE(startm+8+16*20*11+160)
    sta l1
    lda #.HIBYTE(startm+8+16*20*11+160)
    sta l1+1
    ldy #0
@loop1:
    lda #0
    sta (l0),y
    sta (l1),y
    iny
    tya
    and #7
    bne @loop1
    cpy #152
    beq @st1
    tya
    clc
    adc #8
    tay
    bne @loop1
@st1:
    rts

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
    rts
@cond1:
    cmp #$5A
    bcc @fire
    beq @fire
    jmp @test2
@cond2:
    cmp #$7A
    bcc @fire
    beq @fire
    rts
@fire:
    eor #$20
    rts

