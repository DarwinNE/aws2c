# La piramide di Iunnuh

This is an example about how to obtain a complete interactive fiction game
starting with an adventure file produced with Aristide Torrelli's AWS system.

The game used as an example here is an Italian game called "La piramide di 
Iunnuh." To compile it you will need the following things:

- the executable of the tool `aws2c`
- the file `aws.h` that describes the data structures employed in the AWS system
- files `inout.c` and `inout.h` that contain the parser and the input/output
routines
- the file `systemdef.h` that contains macros to change colour on the terminal 
you are using and other important settings.
- the files `loadsave.h` and `loadsave.c` that contain routines useful to load
and save the game.

You can use the provided `aws.h`, `inout.c`, `inout.h` and `systemdef.h` files, 
or you can  customise them to your needs if you want to.

To play the game, let us suppose that you have a Unix operating system with the
gcc and the make tools available on the command line and that you just
successfully compiled `aws2c` in the parent directory.

To obtain the source code of the game, type: 

`../aws2c Innuh.AWS innuh.c`

If everything goes as expected, you should have obtained the source file
`iunnuh.c` containing the game to be compiled. To obtain an executable,
type:

`gcc innuh.c inout.c loadsave.c -o innuh`

and enjoy the game by typing `./iunnuh`
