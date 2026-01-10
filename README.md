# AWS2C

## Introduction

The Adventure Writing System (AWS) is a tool put together by Aristide Torrelli 
to easily write interactive fiction games.

Aristide describes his tool here: 
https://web.archive.org/web/20240417220214/http://www.aristidetorrelli.it/aws3/Avventure.html

(Unfortunately, the page is no longer active, hence the link to archive.org)

I thought it would have been fun to put together a converter that creates
a standard C source from the files produced by the AWS tool.

A detailed english description of the AWS format is available in the 
AWS_description.md file.

## Install

To compile the tool, you will need `gcc` and the `make` utility installed in
your system. If you use a Unix operating system, most probably it is the case.
If you use Windows, you may find Cygwin helpful.

Just download the source archive and type `make`. That will create the `aws2c` 
executable.

## Basic usage

Once you have the executable `aws2c` in your path, you use it as in the
following example:

`aws2c adventure.aws adventure.c`

where `adventure.aws` contains the game in the AWS format and the `adventure.c`
is the generated file. To compile it, you will need the `aws.h`, `inout.c`,
`inout.h` and `systemd.h` files, provided with the tool, as well as 
`loadsave.c` and `loadsave.h` if you need to provide the save/load facility:

`gcc adventure.c inout.c loadsave.c -o adventure`

to obtain an executable called `adventure`. `aws2c` also creates a file called
`config.h` that contains some configuration settings for the project.

An example of a file that can be renamed as `systemd.h` and used for compiling
games to modern Unix terminals is `modern.h`.

For the moment `aws2c` does not cover all the full range of actions, decisions
and functions offered by Aristide's AWS 3.2 system. If the tool encounters
something it does not recognise, it will give an error and the resulting C file
may not be compilable. Other actions are just ignored but a warning message
is generated while running `aws2c`.

## Command line options

Since one of the design goals of the utility is to obtain files that can be
compiled for old Commodore machines (C64, C128, Plus4, etc...), options for
translating UTF-8 characters are available. The program help should be relatively self-explanatory:

~~~~
$ ./aws2c -h
Adventure Writing System to C compiler, version 1.9.7, September 2018 - January 2026
Davide Bucci

Usage: ./aws2c [options] inputfile.aws outputfile

then compile (along with file inout.c) using your favourite C compiler.

Available options:
 -h  this help
 -u  convert UTF-8 characters into standard ASCII chars.
        è -> e   é -> e
 -r  same as -u, but keep accents as separate chars.
        è -> e'  è -> e`
 -s  same as -u, but only employs the single accent '.
        é -> e'  è -> e'
 -p  UTF-8 conversion replaces chars that cannot be converted with spaces.
 -c  compress text with Huffman algorithm.
 -d  employ 6 directions instead of 10.
 -m  employ hardcoded messages instead of an array.
 -n  do not clear screen every time a new room is shown.
 -v  or --version print version and exit
 -w  don't check for size and weight of objects (counter 119, 120
     and 124 are not used).
 -k  don't output header.
 -kk strip empty messages and automatic counters.
 -5  use 5-bit compression for the dictionary.
 -3  use 3-byte hash code for dictionary.
 -l  don't take into account light/dark situations.
 -f  <filename> specify the name of the file to be used for the
     configuration. Default is config.h.
     NOTE: if this option is used, you should compile the files
     with the macro CONFIG_FILENAME defined to the configuration
     file name, within ". For example, for gcc
     gcc -DCONFIG_FILENAME=\"test.h\" ...
 -R  <res_file> <map_file> write all messages and description in
     a separate resource file, along with a map file. This option
     requires -m and -c.
 --verbose write plenty of things.
~~~~

If you have a machine that only has the accent ' available such as a Commodore 64, it makes sense to use the `-s` option to create the file to be compiled.
The `-m` option deserves some discussion. The default way of storing messages in the generated C code is an array of structures. This allows a certain flexibility for example to show a message with a code calculated on the fly during the game. With the `-m` option this is not possible as there will be a bunch of variables called `message1`, `message2` etc. This saves space at the expense of a (rarely used) flexibility. If a calculated message code is exploited in the AWS file you are trying to convert with the `-m` option, you will get errors during the compilation of the generated file.

Option `-w` allows to save some bytes if the size and weight of objects do not play an important role in the game.

Option `-5` employs a 5-bit coding ('A' -> 1, 'B' -> 2, etc. up to 'Z') for the dictionary. This can be used for games with moderately large dictionaries (at least 150 words) to save space, as there is a certain overhead due to the packing code that needs to be included. The option has the side effect that the dictionary can not be immediately seen with an ASCII editor from the compiled file. That can be interesting for some applications, or when using game passwords.

By default, the code takes into account the presence of absence of light (markers 121 and 122). This can be disabled by using the `-l` option.

### Export messages as resources in paged memory systems and cc65

Option `-R` allows to store all messages in an external resource file. They can then be processed with a Python script written by <https://github.com/hkzlab> and produce a resource file that can be dynamically loaded from a disk. This is useful for targets that have a paged memory (Apple II enhanced with 128KB, for instance):

<https://codeberg.org/hkzlab/Iunnuh2_Apple2e>

An example is available here: <https://github.com/DarwinNE/aws2c/tree/master/Iunnuh2/build/Apple2e>

Note the <resources.h> and <resources.c> files. They require a file produced by <https://github.com/DarwinNE/aws2c/blob/master/utils/retro/res_packer.py> from the resource file obtained by `aws2c`.


The code is modified to expect the strings in a file named `TEXT.DAT`. The file is generated by the python script `strconv.py` in the `utils` directory.

This script takes a text file with the following format as input:
```
<name 0> <comma separated hex array>
<name 1> <comma separated hex array>
...
<name n> <comma separated hex array>
```

The output is a binary file built as follows:
```
.---.---.---.---.  <--- HEADER
|offset | size  |  Header entry for array 0
'---.---'---.---'
|offset | size  |  Header entry for array 1
'---.---'---.---'
|offset | size  |  Header entry for array 2
'---.---'---.---'
 ...............
'---.---'---.---'
|offset | size  |  Header entry for array n 
'---.---'---.---'
| .. PADDING .. |  Padding to have data start at a multiple of 0x100 (256 bytes boundary)
'---.---.---.---'  <--- DATA (array data begins here)
|   |   |   |   |
'---.---.---.---'
|   |   |   |   |
'---.---.---.---'
|   |   |   |   |
'---.---.---.---'
 ...............
|   |   |   |   |
'---.---.---.---'
| .. PADDING .. | Padding to have data end at the end of a 256 bytes page
'---.---.---.---'
```

The header contains an `offset` and `size` entry for each array. The values are **unsigned little endian, 16 bit**. So, every array has a 4 bytes entry in the header.
The `size` is simply the length of the array, `offset` is the offset at which the array starts in the data file **relative to the beginning of the file**.
Both the header and data sections are padded to reach a multiple of 256 bytes.


## Some examples

This tool comes with some examples. In each sub-folder, you find a readme file with all the instructions.

### Dawn patrol

This game is an English translation of "Pattuglia all'alba", adapted by A. Torrelli from a type-in BASIC program.

### La piramide di Iunnuh

This game was published as a type-in BASIC program for the TI99/4A by Aristide 
Torrelli (MCmicrocomputer 33, sett. 1984) and then ported to the ZX Spectrum by 
Manlio Severi (MCmicrocomputer 35, nov. 1984). It was one of the very fist 
Italian adventure games and it had a certain success at the time.

It was originally written in Basic, but Aristide decided in 2014 to write a version in AWS that I am using here as an example.

By the way, basing myself from the original published BASIC version of the game, 
I converted it in Java in 2011 and in C in 2018. I also translated the game in 
English. I did this before developing `aws2c` and you can find the result of my 
efforts here: 
http://davbucci.chez-alice.fr/index.php?argument=varie/innuh/innuh.inc&language=English

### La piramide di Iunnuh 2

This is a much longer and deep adventure game that A. Torrelli did in 2014 for 
the thirtieth anniversary of his first game. I translated it in English and it
is another example of how you can convert an adventure into an executable.

### 
