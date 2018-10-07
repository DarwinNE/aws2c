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
`inout.h` and `modern.h` files, provided with the tool:

`gcc adventure.c inout.c -o adventure`

to obtain an executable called `adventure`.

For the moment AWS2C does not cover all the full range of actions, decisions
and functions offered by Aristide's AWS 3.2 system. If the tool encounters
something it does not recognise, it will give an error and the resulting C file
may not be compilable. Other actions are just ignored but a warning message
is generated while running `aws2c`.