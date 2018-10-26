# AWS2C

The Adventure Writing System (AWS) is a tool put together by Aristide Torrelli 
to easily write interactive fiction games.

Aristide describes his tool here: http://www.aristidetorrelli.it/aws3/Avventure.html

I thought it would have been fun to put together a converter that creates
a standard C source from the files produced by the AWS tool.

To compile the tool, you will need `gcc` and the `make` utility installed in
your system. If you use a Unix operating system, most probably it is the case.
If you use Windows, you may find Cygwin helpful.

Once you have the executable `aws2c` in your path, you use it as in the
following example:

`aws2c adventure.aws adventure.c`

where `adventure.aws` contains the game in the AWS format and the `adventure.c`
is the generated file. To compile it, you will need the `aws.h`, `inout.c`,
`inout.h` and `systemdef.h` files, provided with the tool:

`gcc adventure.c inout.c -o adventure`

to obtain an executable called `adventure`.

An example of a file that can be renamed as `systemdef.h` and used for compiling
games to modern Unix terminals is `modern.h`.

For the moment AWS2C does not cover all the full range of actions, decisions
and functions offered by Aristide's AWS 3.2 system. If the tool encounters
something it does not recognise, it will give an error and the resulting C file
may not be compilable. Other actions are just ignored but a warning message
is generated while running `aws2c`.

Since one of the design goals of the utility was to obtain files that can be
compiled for old Commodore machines (C64, C128, Plus4, etc...), options for
translating UTF-8 characters are available. The program help should explain them:

~~~~
$ ./aws2c -h
Adventure Writing System to C compiler, version 1.0
Davide Bucci 2018

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
 -c  compress text with Huffman algorithm.
 -d  employ 6 directions instead of 10.
 -m  employ hardcoded messages instead of an array.
~~~~

If you have a machine that only has the accent ' available such as a Commodore 64, it makes sense to use the `-s` option to create the file to be compiled.
The `-m` option deserves a little discussion. The default way of storing messages in the generated C code is an array of structures. This allows a certain flexibility for example to show a message with a code calculated on the fly during the game. With the `-m` option this is not possible as there will be a bunch of variables called `message1`, `message2` etc. This saves space at the expense of a (rarely used) flexibility. If a calculated message code is exploited in the AWS file you are trying to convert with the `-m` option, you will get errors during the compilation of the generated file.