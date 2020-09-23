# Dawn patrol

This game is an English translation of "Pattuglia all'alba", adapted by A. Torrelli from a type-in BASIC program.

## Compile the game on a modern system

All the game is contained in the `dawn_patrol.aws` file. I am supposing that you have compiled `aws2c` and you have the executable ready in the parent directory. You may generate a C source that can be compiled on a modern UTF-8 compliant terminal with the following command:

~~~~
../aws2c -d -n dawn_patrol.aws dawn_patrol.c
~~~~

Option `-d` means that the game only uses 6 directions (N, S, E, W, UP, DOWN) and not NE, NW, etc. This allows to work with smaller arrays and save some space.

Option `-n` means that the game does not clean the screen each time a new room is entered.

You should have generated `dawn_patrol.c` in the current directory. It can be compiled with `inout.c` and the different headers present in the example. They are link to the files provided in the parent directory as they can be shared for all games. Of course, if you have some special needs and you do want to customize the input and the output of your game, for example using a window instead of the standard C console, you can customize `inout.c` as much as you want (but be careful not breaking the word-wrapping mechanism and similar things).

## Modern systems

You can compile the game and play it in a modern console with the following command:

~~~~
gcc dawn_patrol.c inout.c loadsave.c -o patrol
~~~~

If everything went fine, you should obtain the executable file `patrol` that you can run and play in your console:

`./patrol`

Type `stop` to quit the game.

The game employs ANSI control codes for changing the text colours. It is something that is supported on every decent terminal program and so that should not create issues. However, let's have a peek at the corresponding section of the `systemd.h` file:

~~~~
#else /* Definitions for modern ANSI terminals */

    #define BUFFERSIZE 256
    #define B_SIZE 255


    #define SHIFTPETSCII 

    #define waitscreen()

    // The number of columns of the screen
    #define NCOL 80

    #define waitkey() getchar()
    #define inputtxt() printf("\033[1m\x1b[32m\33[40m")
    #define evidence1() printf("\033[1m\x1b[31m\33[40m")
    #define evidence2() printf("\033[0m\x1b[93m\33[40m")
    #define evidence3() printf("\033[0m\x1b[95m\33[40m")
    #define cls()

    #define normaltxt() printf("\033[0m\x1b[36m\33[40m")
    #define tab() printf("\t")
    #define wait1s()    {unsigned int retTime = time(0) + 1;while (time(0) < \
        retTime);}
#define init_term() {\
        printf( "This terminal does not support ANSI codes."\
        "\033[80D"\
        "You'll see garbage chars on the screen. If you use MS-DOS, add "\
        "\033[80D"\
        "DEVICE=DOS\\ANSI.SYS to your CONFIG.SYS file"\
        "\033[80D"\
        "It's supported since MS-DOS 2.0, so no excuses."\
"\033[80D                                                               ");\
        normaltxt();printf("\n\n");}

    #define leave() printf("\033[0m\n\n")
~~~~
`BUFFERSIZE` and `B_SIZE` are used respectively for the size of the input and decompression buffer. `NCOL` is the number of columns to be used for the word-wrapping mechanism. You have then a few macros containing ANSI control codes. `init_term()` in particular shows an error message that is erased if the terminal does not support ANSI.


## Compile the game for vintage computers

When using vintage computers such as the VIC-20, we need to face two issues:

* There's no UTF-8 support at all. That is not a big deal for English games, but can become a problem with other languages where accents play a role.

* The memory is severely constrained.

To solve the first issue, `aws2c` contains a techniques that allows to translate UTF-8 characters into combinations of ASCII characters. It works very well with languages such as Italian, where you can accept substitutions such as `Perché` -> `Perche'`. Very often, there is a single accent available in the character set that is `'`.

A reasonable choice for obtaining a C source that can be fit for vintage system is therefore:

~~~~
../aws2c -d -c -m -n -s dawn_patrol.aws dawn_patrol_no_UTF8.c.c
~~~~

Where the `-d` and `-n` options are as above and there are some additional options employed:

* `-c` indicates that the text should be compressed. `aws2c` employs a Huffman compression algorithm that can shrink text by 40% on average (it greatly depends on the language, though). The benefits of text compressions are reduced for small games as the program must include additional code for the decompression as well as a binary tree that contains the code employed.

* `-m` indicates that messages are hardcoded instead of stored in an array. In other words, you have things such as `message122` in the generated code at the place of `message[122]`. This reduces the memory occupation at the expense of a (minor) lack of flexibility that may be requested by some games.

* `-s` indicates that UTF-8 characters must be replaced by sequences employing only a single accent `'`.

Then, you can target your vintage system with your favourite compiler. 

### Commodore 64

For Commodore 8-bit computers one needs to generate 6502 assembly; I like the Cc65 compiler in particular. If we start by the C64, we may generate a `.prg` file that can be played in an emulator such as VICE:

~~~~
cl65 -t c64 -D C64 -Or dawn_patrol_no_UTF8.c.c inout.c  loadsave.c -o patrol-64.prg
~~~~

The option `-D C64` tells the compiler to define a preprocessor symbol `C64`. It is important, as it targets the appropriate C64 definitions in the `systemd.h` file. Let's have a look at the appropriate section of the file:

~~~~
#if defined(C64)

    #include<conio.h>

    #define BUFFERSIZE 40
    #define B_SIZE 40

    #define SHIFTPETSCII \
        if((c>=0x41 && c<=0x5A)||(c>=0x61 && c<=0x7A)) c^=0x20

    #define waitscreen()

    /* The number of columns of the screen */
    #define NCOL 40
    /* The number of available rows of the screen. If undefined, it is
       not checked
    */
    #define NROW 21
    extern unsigned char rowc;

    #define green       "\x1E"
    #define red         "\x1C"
    #define cyan        "\x9F"
    #define blue        "\x1F"
    #define yellow      "\x9E"
    #define pink        "\x96"


    /* Macro to wait for a key */
    #define waitkey() cgetc(); rowc=0

    /* Define the style of the input text */
    #define inputtxt() printf(green)

    /* Define the style of the first evidenced text */
    #define evidence1() printf(red)

    /* Define the style of the second evidenced text */
    #define evidence2() printf(yellow)

    /* Define the style of the third evidenced text */
    #define evidence3() printf(pink)

    /* Define the style of the normal text */
    #define normaltxt() printf(cyan)

    /* Clear the screen */
    #define cls() clrscr();zeror()

    /* Write a tabulation (it can be adapted to screen width). */
    #define tab() printf("    ")

    /* Wait for one second */
    #define wait1s()    {}
    #define PTRBRD 53280U
    #define PTRCLR 53281U
    /* Init the terminal */
    #define POKE(addr,val)     (*(unsigned char*) (addr) = (val))
    
    // Restore default VIC-II config (lower case)
    // This is useful if there is loader that goes in a graphic mode.

    #define init_term() {\
        *(char*)PTRBRD = 0x00;\
        *(char*)PTRCLR = 0x00;\
        clrscr();\
        normaltxt();\
        POKE(56578U, 63);\
        POKE(56576U, 151);\
        POKE(53272U, 23);\
        POKE(53265U, 27);\
    }

    /* Prepare the terminal to leave the program execution. */
    #define leave() asm("jmp $FCE2")
~~~~

`SHIFTPETSCII` is a macro that converts ASCII chars that result from the compression into PETSCII. If a string is stored directly in a C source, Cc65 automatically does that for the C64 target. However, if you store compressed text, the decompression algorithm restore ASCII characters and a correction is mandatory.

Then you have some macros that define the different colours via control codes and perform some initializations. Here is the place to put some platform-specific code, the more blatant example is the jump to the reset vector that is to be called when the game is quit (the BASIC is scrambled, anyway).

The idea is to generate code as much as standard and portable as possible, so that it can be compiled for many different platforms.

### VIC-20 + 16K

If you want to play the game on a VIC-20 with the 16KB expansion, you can compile with:

~~~~
cl65 -t vic20 -C vic20-16k_exp.cfg -D VIC20 -Or dawn_patrol_no_UTF8.c.c inout.c  loadsave.c -o patrol-VIC+16.prg
~~~~

The file `vic20-16k_exp.cfg` is a configuration file for Cc65 and is provided in this example.

### Commodore 128

If you have a C128, a very pleasant thing that offers is the 80-column mode:

~~~~
cl65 -t c128 -D C128 -Or dawn_patrol_no_UTF8.c.c inout.c  loadsave.c -o patrol-128.prg
~~~~

A disadvantage of the C128 target with respect to the C64 is that you can use much less memory! Apparently this is normal, I discussed with the Cc65 guys here on GitHub: https://github.com/cc65/cc65/issues/772

### Sinclair ZX Spectrum

To compile for the ZX Spectrum, we need to use a different compiler as the processor here is the Z80. I choose Z88dk that offers a library with a pleasant support of ANSI codes. Another pleasant surprise was to find that the standard C input/output routines worked by default with a 64-column display using an appropriate font. It's very useful for games such as text adventures where you have to write plenty of text! Of course, this comes with a price and you can count on less than 30KB available for your C program on a 48K machine (that's not too bad).

To compile the program:
~~~~
zcc +zx -clib=ansi -lndos -create-app dawn_patrol_no_UTF8.c.c  loadsave.c inout.c -o ZXpatrol
~~~~

You obtain a `ZXpatrol.tap` that you can run in an emulator. You can also create a `.wav` file with the nice `appmake` routine:

~~~~
appmake +zx -b ZXpatrol.tap --audio --fast --dumb
~~~~

I notice that I could obtain audio files working much better when I connect the real hardware to my laptop by filtering the generated file with a low-pass filter cutting at approximatively 6 or 7 kHz.